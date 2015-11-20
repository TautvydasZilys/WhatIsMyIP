#include "PrecompiledHeader.h"
#include "Etw\Etw.h"
#include "PlugNPlayObjectRegistry.h"
#include "Utilities\EventHandler.h"
#include "Utilities\Fence.h"
#include "Utilities\HandleHolder.h"
#include "Utilities\Vector.h"

using namespace ABI::Windows::Devices::Enumeration::Pnp;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace PlugNPlay;

PlugNPlayObjectRegistry* PlugNPlayObjectRegistry::s_Instance;

const wchar_t kDEVPKEY_Device_InstanceId[] = L"{78c34fc8-104a-4aca-9ea4-524d52996e57} 256";
const wchar_t kDEVPKEY_Device_DeviceDesc[] = L"{A45C254E-DF1C-4EFD-8020-67D146A850E0} 2";
const wchar_t kDEVPKEY_Device_FriendlyName[] = L"{A45C254E-DF1C-4EFD-8020-67D146A850E0} 14";

HRESULT PlugNPlayObjectRegistry::Create()
{
	Etw::EtwScopedEvent creationEvent("PlugNPlayObjectRegistry", "Create registry");

	auto instance = WRL::Make<PlugNPlayObjectRegistry>();
	
	auto hr = instance->Initialize();
	if (FAILED(hr))
	{
		instance->Cleanup();
		ReturnIfFailed(hr);
	}

	s_Instance = instance.Detach();
	return S_OK;
}

void PlugNPlayObjectRegistry::Destroy()
{
	auto instance = static_cast<PlugNPlayObjectRegistry*>(InterlockedExchangePointer(reinterpret_cast<void**>(&s_Instance), nullptr));

	if (instance != nullptr)
	{
		instance->Cleanup();
		instance->Release();
	}
}

PlugNPlayObjectRegistry::PlugNPlayObjectRegistry() :
	m_IsPnpObjectWatcherRunning(false),
	m_IsInitialEnumerationCompleted(false),
	m_DirtyStatus(DirtyStatus::kNotDirty)
{
	m_AddedEventToken.value = 0;
	m_UpdatedEventToken.value = 0;
	m_RemovedEventToken.value = 0;
}

HRESULT PlugNPlayObjectRegistry::Initialize()
{
	m_InitialEnumerationCompletedEvent = CreateEventExW(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

	auto hr = Windows::Foundation::GetActivationFactory(Utilities::HStringReference(L"Windows.Devices.Enumeration.Pnp.PnpObject"), &m_PnpObjectStatics);
	ReturnIfFailed(hr);

	auto interfaceInstanceIdProperties = WRL::Make<Utilities::Vector<HSTRING>>();
	interfaceInstanceIdProperties->Append(Utilities::HStringReference(kDEVPKEY_Device_InstanceId));

	hr = m_PnpObjectStatics->CreateWatcher(PnpObjectType_DeviceInterface, interfaceInstanceIdProperties.Get(), &m_PnpObjectWatcher);
	ReturnIfFailed(hr);

	WRL::ComPtr<PlugNPlayObjectRegistry> _this = this;

	hr = m_PnpObjectWatcher->add_Added(Utilities::EventHandlerFactory<ITypedEventHandler<PnpObjectWatcher*, PnpObject*>>::Make([_this](IPnpObjectWatcher*, IPnpObject* pnpObject)
	{
		_this->OnObjectAdded(pnpObject);
		return S_OK;
	}).Get(), &m_AddedEventToken);
	ReturnIfFailed(hr);

	hr = m_PnpObjectWatcher->add_Updated(Utilities::EventHandlerFactory<ITypedEventHandler<PnpObjectWatcher*, PnpObjectUpdate*>>::Make([_this](IPnpObjectWatcher*, IPnpObjectUpdate* pnpObject)
	{
		_this->OnObjectUpdated(pnpObject);
		return S_OK;
	}).Get(), &m_UpdatedEventToken);
	ReturnIfFailed(hr);

	hr = m_PnpObjectWatcher->add_Removed(Utilities::EventHandlerFactory<ITypedEventHandler<PnpObjectWatcher*, PnpObjectUpdate*>>::Make([_this](IPnpObjectWatcher*, IPnpObjectUpdate* pnpObject)
	{
		_this->OnObjectRemoved(pnpObject);
		return S_OK;
	}).Get(), &m_RemovedEventToken);
	ReturnIfFailed(hr);

	hr = m_PnpObjectWatcher->add_EnumerationCompleted(Utilities::EventHandlerFactory<ITypedEventHandler<PnpObjectWatcher*, IInspectable*>>::Make([_this](IPnpObjectWatcher*, IInspectable*)
	{
		_this->OnInitialEnumerationCompleted();
		return S_OK;
	}).Get(), &m_InitialEnumerationCompletedToken);
	ReturnIfFailed(hr);

	hr = m_PnpObjectWatcher->Start();
	ReturnIfFailed(hr);

	m_IsPnpObjectWatcherRunning = true;
	return S_OK;
}

void PlugNPlayObjectRegistry::Cleanup()
{
	Etw::EtwScopedEvent cleanupEvent("PlugNPlayObjectRegistry", "Cleanup registry");

	HRESULT hr;

	if (m_PnpObjectWatcher != nullptr)
	{
		if (m_RemovedEventToken.value != 0)
		{
			hr = m_PnpObjectWatcher->remove_Removed(m_RemovedEventToken);
			Assert(SUCCEEDED(hr));

			m_RemovedEventToken.value = 0;
		}

		if (m_UpdatedEventToken.value != 0)
		{
			hr = m_PnpObjectWatcher->remove_Updated(m_UpdatedEventToken);
			Assert(SUCCEEDED(hr));

			m_UpdatedEventToken.value = 0;
		}

		if (m_AddedEventToken.value != 0)
		{
			hr = m_PnpObjectWatcher->remove_Added(m_AddedEventToken);
			Assert(SUCCEEDED(hr));

			m_AddedEventToken.value = 0;
		}

		Utilities::ReaderWriterLock::WriterLock lock(m_RegistryLock);

		if (m_IsPnpObjectWatcherRunning)
		{
			hr = m_PnpObjectWatcher->Stop();
			Assert(SUCCEEDED(hr));

			m_IsPnpObjectWatcherRunning = false;
		}

		m_PnpObjectWatcher = nullptr;
	}
}

HRESULT PlugNPlayObjectRegistry::Lookup(const wchar_t* interfaceInstanceIdSubstring, HSTRING* outName)
{
	Etw::EtwScopedEvent lookupEvent("PlugNPlayObjectRegistry", "Lookup interface instance id substring");

	Assert(s_Instance != nullptr);
	return s_Instance->LookupImpl(interfaceInstanceIdSubstring, outName);
}

static inline HRESULT UnboxString(IInspectable* boxed, HSTRING* outUnboxed)
{
	if (boxed == nullptr)
	{
		*outUnboxed = nullptr;
		return S_OK;
	}

	WRL::ComPtr<IPropertyValue> propertyValue;
	auto hr = boxed->QueryInterface(__uuidof(IPropertyValue), &propertyValue);
	ReturnIfFailed(hr);

	return propertyValue->GetString(outUnboxed);
}

void PlugNPlayObjectRegistry::RebuildRegistry()
{
	Utilities::ReaderWriterLock::WriterLock registryLock(m_RegistryLock);
	Utilities::CriticalSection::Lock objectLock(m_ObjectCriticalSection);

	Etw::EtwScopedEvent rebuildEVent("PlugNPlayObjectRegistry", "Rebuild registry");

	m_DirtyStatus = DirtyStatus::kNotDirty;

	m_InterfaceInstanceIds.clear();
	m_InstanceIdIndices.clear();
	m_DeviceNames.clear();

	Utilities::Fence allDevicesCreatedFence(static_cast<uint32_t>(m_PnpObjects.size()));

	m_InstanceIdIndices.reserve(m_PnpObjects.size());
	m_DeviceNames.resize(m_PnpObjects.size());

	auto deviceNameProperties = WRL::Make<Utilities::Vector<HSTRING>>();
	deviceNameProperties->Append(Utilities::HStringReference(kDEVPKEY_Device_FriendlyName));
	deviceNameProperties->Append(Utilities::HStringReference(kDEVPKEY_Device_DeviceDesc)); // Fallback in case friendly name is not available

	for (auto& pnpObject : m_PnpObjects)
	{
		WRL::ComPtr<IMapView<HSTRING, IInspectable*>> objectProperties;
		auto hr = pnpObject.second->get_Properties(&objectProperties);
		AdvanceFenceAndContinueIfFailed(hr, allDevicesCreatedFence);

		WRL::ComPtr<IInspectable> instanceIdInspectable;
		hr = objectProperties->Lookup(Utilities::HStringReference(L"System.Devices.DeviceInstanceId"), &instanceIdInspectable);
		AdvanceFenceAndContinueIfFailed(hr, allDevicesCreatedFence);

		Utilities::HString deviceId;
		hr = UnboxString(instanceIdInspectable.Get(), &deviceId);
		if (FAILED(hr) || deviceId == nullptr)
		{
			allDevicesCreatedFence.Advance();
			continue;
		}

		WRL::ComPtr<IAsyncOperation<PnpObject*>> createDeviceAsyncOperation;
		hr = m_PnpObjectStatics->CreateFromIdAsync(PnpObjectType_Device, deviceId, deviceNameProperties.Get(), &createDeviceAsyncOperation);
		AdvanceFenceAndContinueIfFailed(hr, allDevicesCreatedFence);

		uint32_t deviceIndex = static_cast<uint32_t>(m_InstanceIdIndices.size());
		hr = createDeviceAsyncOperation->put_Completed(Utilities::EventHandlerFactory<IAsyncOperationCompletedHandler<PnpObject*>>::Make([this, &allDevicesCreatedFence, deviceIndex](IAsyncOperation<PnpObject*>* operation, AsyncStatus asyncStatus)
		{
			Assert(asyncStatus == Completed);

			if (asyncStatus == Completed)
			{
				WRL::ComPtr<IPnpObject> device;
				auto hr = operation->GetResults(&device);
				AdvanceFenceAndReturnSuccessIfFailed(hr, allDevicesCreatedFence);

				WRL::ComPtr<IMapView<HSTRING, IInspectable*>> deviceProperties;
				hr = device->get_Properties(&deviceProperties);
				AdvanceFenceAndReturnSuccessIfFailed(hr, allDevicesCreatedFence);
				
				WRL::ComPtr<IInspectable> nameInspectable;
				hr = deviceProperties->Lookup(Utilities::HStringReference(kDEVPKEY_Device_FriendlyName), &nameInspectable);

				if (FAILED(hr) || nameInspectable == nullptr)
					hr = deviceProperties->Lookup(Utilities::HStringReference(kDEVPKEY_Device_DeviceDesc), &nameInspectable);

				AdvanceFenceAndReturnSuccessIfFailed(hr, allDevicesCreatedFence);

				Utilities::HString name;
				hr = UnboxString(nameInspectable.Get(), &name);
				AdvanceFenceAndReturnSuccessIfFailed(hr, allDevicesCreatedFence);

				m_DeviceNames[deviceIndex] = std::move(name);
			}

			allDevicesCreatedFence.Advance();
			return S_OK;
		}).Get());
		ContinueIfFailed(hr);
			
		uint32_t idLength;
		const wchar_t* deviceIdCharacters = pnpObject.first.GetRawBuffer(&idLength);

		m_InstanceIdIndices.push_back(m_InterfaceInstanceIds.size());
		m_InterfaceInstanceIds.insert(m_InterfaceInstanceIds.end(), deviceIdCharacters, deviceIdCharacters + idLength);
		m_InterfaceInstanceIds.push_back(0);
	}

	{
		Etw::EtwScopedEvent waitForFenceEvent("PlugNPlayObjectRegistry", "Synchronize allDevicesCreatedFence");
		allDevicesCreatedFence.Synchronize();
	}
}

void PlugNPlayObjectRegistry::RebuildRegistryIfNeeded()
{
	do
	{
		auto dirty = DirtyStatus::kDirty;
		if (m_DirtyStatus.compare_exchange_strong(dirty, DirtyStatus::kRebuilding, std::memory_order_seq_cst))
			RebuildRegistry();
	}
	while (m_DirtyStatus == DirtyStatus::kRebuilding);
}

void PlugNPlayObjectRegistry::OnObjectAdded(IPnpObject* pnpObject)
{
	Utilities::HString objectId;
	auto hr = pnpObject->get_Id(&objectId);
	Assert(SUCCEEDED(hr));
	if (FAILED(hr)) return;

	Utilities::CriticalSection::Lock lock(m_ObjectCriticalSection);
	m_PnpObjects.emplace(std::move(objectId), pnpObject);

	if (m_IsInitialEnumerationCompleted)
		m_DirtyStatus = DirtyStatus::kDirty;
}

void PlugNPlayObjectRegistry::OnObjectUpdated(IPnpObjectUpdate* pnpObjectUpdate)
{
	Utilities::CriticalSection::Lock lock(m_ObjectCriticalSection);

	if (m_IsInitialEnumerationCompleted)
		m_DirtyStatus = DirtyStatus::kDirty;
}

void PlugNPlayObjectRegistry::OnObjectRemoved(IPnpObjectUpdate* pnpObjectUpdate)
{
	Utilities::HString objectId;
	auto hr = pnpObjectUpdate->get_Id(&objectId);
	Assert(SUCCEEDED(hr));
	if (FAILED(hr)) return;

	Utilities::CriticalSection::Lock lock(m_ObjectCriticalSection);
	m_PnpObjects.erase(objectId);

	if (m_IsInitialEnumerationCompleted)
		m_DirtyStatus = DirtyStatus::kDirty;
}

void PlugNPlayObjectRegistry::OnInitialEnumerationCompleted()
{
	m_IsInitialEnumerationCompleted = true;
	RebuildRegistry();
	SetEvent(m_InitialEnumerationCompletedEvent);
}

HRESULT PlugNPlayObjectRegistry::LookupImpl(const wchar_t* interfaceInstanceIdSubstring, HSTRING* outName)
{
	Assert(s_Instance != nullptr);

	auto waitResult = WaitForSingleObjectEx(s_Instance->m_InitialEnumerationCompletedEvent, INFINITE, FALSE);
	Assert(waitResult == WAIT_OBJECT_0);

	RebuildRegistryIfNeeded();
	Utilities::ReaderWriterLock::ReaderLock lock(m_RegistryLock);

	bool found;
	size_t foundIndex;

	for (size_t i = 0; i < m_InterfaceInstanceIds.size(); i++)
	{
		size_t index = i;
		size_t j = 0;

		while (interfaceInstanceIdSubstring[j] != L'\0' && interfaceInstanceIdSubstring[j] == m_InterfaceInstanceIds[index])
		{
			index++;
			j++;
		}

		if (interfaceInstanceIdSubstring[j] == '\0')
		{
			found = true;
			foundIndex = i;
			break;
		}
	}

	if (!found)
		return E_FAIL;

	auto it = std::upper_bound(m_InstanceIdIndices.begin(), m_InstanceIdIndices.end(), foundIndex);
	if (it == m_InstanceIdIndices.begin())
		return E_FAIL;

	auto deviceName = m_DeviceNames[it - m_InstanceIdIndices.begin() - 1];
	*outName = deviceName.Detach();
	return S_OK;
}