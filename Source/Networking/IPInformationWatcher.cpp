#include "PrecompiledHeader.h"
#include "Etw\Etw.h"
#include "IPInformation.h"
#include "IPInformationGenerator.h"
#include "IPInformationWatcher.h"

using namespace Networking;

AsyncCreationSingletonImpl(IPInformationWatcher);

HRESULT IPInformationWatcher::Create(IPInformationWatcher** watcher)
{
	auto instance = WRL::Make<IPInformationWatcher>();

	auto hr = instance->Initialize();
	if (FAILED(hr))
	{
		instance->Cleanup();
		return hr;
	}

	*watcher = instance.Detach();
	return S_OK;
}

void IPInformationWatcher::Destroy()
{
	Cleanup();
	Release();
}


IPInformationWatcher::IPInformationWatcher() :
	m_NextRegistrationToken(0),
	m_HasIPInformation(false)
{
	m_NetworkStatusChangedToken.value = 0;
}

HRESULT IPInformationWatcher::Initialize()
{
	auto hr = Windows::Foundation::GetActivationFactory(Utilities::HStringReference(L"Windows.Networking.Connectivity.NetworkInformation"), &m_NetworkInformation);
	FastFailIfFailed(hr);

	hr = m_NetworkInformation->add_NetworkStatusChanged(this, &m_NetworkStatusChangedToken);
	ReturnIfFailed(hr);

	hr = UpdateIPInformation();
	ReturnIfFailed(hr);

	return S_OK;
}

void IPInformationWatcher::Cleanup()
{
	HRESULT hr;

	if (m_NetworkInformation != nullptr)
	{
		if (m_NetworkStatusChangedToken.value != 0)
		{
			hr = m_NetworkInformation->remove_NetworkStatusChanged(m_NetworkStatusChangedToken);
			Assert(SUCCEEDED(hr));

			m_NetworkStatusChangedToken.value = 0;
		}

		m_NetworkInformation = nullptr;
	}
}

HRESULT IPInformationWatcher::UpdateIPInformation()
{
	Etw::EtwRefCountedScopedEvent refreshIPInfoEvent("IPInformationWatcher", "Refresh ip information");

	WRL::ComPtr<IPInformationWatcher> _this = this;
	return GenerateIPInformationAsync([_this, refreshIPInfoEvent](std::vector<Networking::ConnectionProperties>&& connectionProperties)
	{
		Utilities::CriticalSection::Lock lock(_this->m_CallbacksCriticalSection);
		_this->m_HasIPInformation = true;
		_this->m_ConnectionProperties = std::move(connectionProperties);

		for (const auto& callback : _this->m_Callbacks)
			callback.second(_this->m_ConnectionProperties);

		return S_OK;
	});
}

HRESULT STDMETHODCALLTYPE IPInformationWatcher::Invoke(IInspectable* sender)
{
	auto hr = UpdateIPInformation();
	FastFailIfFailed(hr);

	return S_OK;
}

uint32_t IPInformationWatcher::SubscribeToChanges(std::function<void(const std::vector<Networking::ConnectionProperties>& connectionProperties)>&& callback)
{
	auto instance = GetInstance();
	uint32_t token;

	{
		Utilities::CriticalSection::Lock lock(instance->m_CallbacksCriticalSection);
		token = instance->m_NextRegistrationToken++;
		instance->m_Callbacks.emplace_back(token, std::move(callback));

		if (instance->m_HasIPInformation)
			instance->m_Callbacks.back().second(instance->m_ConnectionProperties);
	}
	
	return token;
}

void IPInformationWatcher::UnsubscribeFromChanges(uint32_t registrationToken)
{
	auto instance = GetInstance();

	Utilities::CriticalSection::Lock lock(instance->m_CallbacksCriticalSection);
	auto& callbacks = instance->m_Callbacks;

	for (size_t i = 0; i < callbacks.size(); i++)
	{
		if (callbacks[i].first == registrationToken)
		{
			if (i != callbacks.size() - 1)
				callbacks[i] = std::move(callbacks[callbacks.size() - 1]);

			callbacks.pop_back();
			return;
		}
	}

	Assert(false && "Unregistered registration token passed to UnsubscribeFromChanges");
}

void IPInformationWatcher::ForceUpdateIPInformation()
{
	auto hr = GetInstance()->UpdateIPInformation();
	FastFailIfFailed(hr);
}