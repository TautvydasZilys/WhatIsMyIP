#include "PrecompiledHeader.h"
#include "ConnectionProfileInformation.h"
#include "ConnectionProperties.h"
#include "Etw\Etw.h"
#include "IPInformation.h"
#include "NetworkEnumNames.h"
#include "PlugNPlay\PlugNPlayObjectRegistry.h"
#include "Utilities\EventHandler.h"
#include "Utilities\SynchronousOperation.h"
#include "Utilities\ThreadPoolRunner.h"
#include "Utilities\Vector.h"

using namespace ABI::Windows::Devices::Enumeration::Pnp;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Networking;
using namespace ABI::Windows::Networking::Connectivity;

using namespace Networking;
using namespace Networking::IPInformation;

static HRESULT GetDefaultNetworkAdapterName(HSTRING* outName)
{
	const wchar_t kDefaultName[] = L"Unknown Network Adapter";
	return WindowsCreateString(kDefaultName, ARRAYSIZE(kDefaultName) - 1, outName);
}

static HRESULT GetNetworkAdapterName(INetworkAdapter* networkAdapter, HSTRING* outName)
{
	GUID adapterId;
	auto hr = networkAdapter->get_NetworkAdapterId(&adapterId);
	ReturnIfFailed(hr);

	const int kBufferLength = 40;
	wchar_t adapterIdStr[kBufferLength];
	auto adapterIdStrLength = StringFromGUID2(adapterId, adapterIdStr, kBufferLength);
	Assert(adapterIdStrLength != 0);
	
	hr = PlugNPlay::PlugNPlayObjectRegistry::Lookup(adapterIdStr, outName);

	if (FAILED(hr))
		return GetDefaultNetworkAdapterName(outName);

	return S_OK;
}

HRESULT Networking::IPInformation::FillConnectionProfileInformation(HSTRING address, IConnectionProfile* connectionProfile, ConnectionProfileInformation* profileInfo)
{
	Etw::EtwScopedEvent convertEvent("IPInformation", "Fill connection profile information");
	HRESULT hr;

	// Getting data plan if connection is metered is __extremely__ slow
	// It involves a cross process call to RuntimeBroker.exe, which then calls into svchost.exe to query some database
	// On Lumia 920, this takes around 700 ms
	// In an attempt to hide the cost, let's query it on another thread while this thread queries other attributes
	WRL::ComPtr<IDataPlanStatus> dataPlanStatus;
	Utilities::HandleHolder dataPlanStatusAcquiredEvent = CreateEventExW(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

	Utilities::ThreadPoolRunner::RunAsync([connectionProfile, &dataPlanStatus, &dataPlanStatusAcquiredEvent]
	{
		Etw::EtwScopedEvent getDataPlanStatusEvent("IPInformation", "Get data plan status");
		connectionProfile->GetDataPlanStatus(&dataPlanStatus);

		auto setEventResult = SetEvent(dataPlanStatusAcquiredEvent);
		Assert(setEventResult != FALSE);
	});

	profileInfo->address = address;

	hr = connectionProfile->GetNetworkConnectivityLevel(&profileInfo->connectivityLevel);
	ReturnIfFailed(hr);

	// ConnectionCost
	{
		WRL::ComPtr<IConnectionCost> connectionCost;
		hr = connectionProfile->GetConnectionCost(&connectionCost);
		ReturnIfFailed(hr);

		hr = connectionCost->get_NetworkCostType(&profileInfo->networkCostType);
		ReturnIfFailed(hr);

		hr = connectionCost->get_Roaming(&profileInfo->isRoaming);
		ReturnIfFailed(hr);
	}

	// NetworkSecurity
	{
		WRL::ComPtr<INetworkSecuritySettings> securitySettings;
		hr = connectionProfile->get_NetworkSecuritySettings(&securitySettings);
		ReturnIfFailed(hr);

		hr = securitySettings->get_NetworkAuthenticationType(&profileInfo->authenticationType);
		ReturnIfFailed(hr);

		hr = securitySettings->get_NetworkEncryptionType(&profileInfo->encryptionType);
		ReturnIfFailed(hr);
	}

	// IConnectionProfile2
	WRL::ComPtr<IConnectionProfile2> connectionProfile2;
	hr = connectionProfile->QueryInterface(__uuidof(IConnectionProfile2), &connectionProfile2);
	ReturnIfFailed(hr);

	hr = connectionProfile2->get_IsWwanConnectionProfile(&profileInfo->isWWanConnection);
	ReturnIfFailed(hr);

	hr = connectionProfile2->get_IsWlanConnectionProfile(&profileInfo->isWLanConnection);
	ReturnIfFailed(hr);

	if (profileInfo->isWWanConnection)
	{
		WRL::ComPtr<IWwanConnectionProfileDetails> wwanConnectionDetails;
		hr = connectionProfile2->get_WwanConnectionProfileDetails(&wwanConnectionDetails);
		ReturnIfFailed(hr);

		hr = wwanConnectionDetails->get_HomeProviderId(&profileInfo->wwanHomeProviderId);
		ReturnIfFailed(hr);

		hr = wwanConnectionDetails->get_AccessPointName(&profileInfo->wwanAccessPointName);
		ReturnIfFailed(hr);

		hr = wwanConnectionDetails->GetNetworkRegistrationState(&profileInfo->wwanNetworkRegistrationState);
		ReturnIfFailed(hr);

		hr = wwanConnectionDetails->GetCurrentDataClass(&profileInfo->wwanDataClass);
		ReturnIfFailed(hr);
	}

	if (profileInfo->isWLanConnection)
	{
		WRL::ComPtr<IWlanConnectionProfileDetails> wlanConnectionDetails;
		hr = connectionProfile2->get_WlanConnectionProfileDetails(&wlanConnectionDetails);
		ReturnIfFailed(hr);

		hr = wlanConnectionDetails->GetConnectedSsid(&profileInfo->wlanSSID);
		ReturnIfFailed(hr);
	}

	WRL::ComPtr<IReference<uint8_t>> signalStrength;

	hr = connectionProfile2->GetSignalBars(&signalStrength);
	ReturnIfFailed(hr);

	if (signalStrength != nullptr)
	{
		hr = signalStrength->get_Value(&profileInfo->signalStrength);
		ReturnIfFailed(hr);

		profileInfo->hasSignalStrength = true;
	}
	else
	{
		profileInfo->hasSignalStrength = false;
	}

	// NetworkAdapter
	{
		WRL::ComPtr<INetworkAdapter> networkAdapter;
		hr = connectionProfile->get_NetworkAdapter(&networkAdapter);
		ReturnIfFailed(hr);

		hr = networkAdapter->get_IanaInterfaceType(&profileInfo->interfaceType);
		ReturnIfFailed(hr);

		WRL::ComPtr<INetworkItem> networkItem;
		hr = networkAdapter->get_NetworkItem(&networkItem);
		ReturnIfFailed(hr);

		hr = networkItem->GetNetworkTypes(&profileInfo->networkType);
		ReturnIfFailed(hr);

		hr = GetNetworkAdapterName(networkAdapter.Get(), &profileInfo->name);
		ReturnIfFailed(hr);
	}

	// DataPlan
	{
		{
			Etw::EtwScopedEvent waitForDataPlanStatusAcquisitionEvent("IPInformation", "Waiting for data plan status acquisition");
			auto waitResult = WaitForSingleObjectEx(dataPlanStatusAcquiredEvent, INFINITE, FALSE);
			Assert(waitResult == WAIT_OBJECT_0);
		}

		WRL::ComPtr<IReference<uint32_t>> megabytesLimit;
		hr = dataPlanStatus->get_DataLimitInMegabytes(&megabytesLimit);
		ReturnIfFailed(hr);

		if (megabytesLimit != nullptr)
		{
			profileInfo->hasLimit = true;
			hr = megabytesLimit->get_Value(&profileInfo->megabytesLimit);
		}
		else
		{
			profileInfo->hasLimit = false;
		}

		WRL::ComPtr<IDataPlanUsage> dataPlanUsage;
		hr = dataPlanStatus->get_DataPlanUsage(&dataPlanUsage);
		ReturnIfFailed(hr);

		if (dataPlanUsage != nullptr)
		{
			hr = dataPlanUsage->get_MegabytesUsed(&profileInfo->megabytesUsed);
			ReturnIfFailed(hr);
		}
	}

	return S_OK;
}

void Networking::IPInformation::ConvertConnectionProfileInformationToConnectionProperties(const ConnectionProfileInformation& profileInfo, ConnectionProperties& connectionProperties)
{
	Etw::EtwScopedEvent convertEvent("IPInformation", "Convert connection profile information to connection properties");

	uint32_t length;
	auto& properties = connectionProperties.properties;

	connectionProperties.name = profileInfo.name.GetRawBuffer(&length);

	properties[L"Address"] = profileInfo.address.GetRawBuffer(&length);
	properties[L"Connectivity level"] = Networking::NetworkEnumNames::GetConnectivityLevelName(profileInfo.connectivityLevel);
	properties[L"Network cost type"] = Networking::NetworkEnumNames::GetNetworkCostTypeName(profileInfo.networkCostType);
	properties[L"Is roaming"] = profileInfo.isRoaming ? L"Yes" : L"No";

	if (profileInfo.networkCostType == NetworkCostType_Fixed || profileInfo.networkCostType == NetworkCostType_Variable)
	{
		properties[L"Data used"] = std::to_wstring(profileInfo.megabytesUsed) + L" MB";
		properties[L"Data limit"] = profileInfo.hasLimit ? std::to_wstring(profileInfo.megabytesLimit) + L" MB" : L"Unlimited";
	}

	properties[L"Authentication type"] = Networking::NetworkEnumNames::GetNetworkAuthenticationTypeName(profileInfo.authenticationType);
	properties[L"Encryption type"] = Networking::NetworkEnumNames::GetNetworkEncryptionTypeName(profileInfo.encryptionType);
	properties[L"Interface type"] = Networking::NetworkEnumNames::GetIANAInterfaceTypeName(profileInfo.interfaceType);
	properties[L"Network type"] = Networking::NetworkEnumNames::GetNetworkTypeName(profileInfo.networkType);
	
	if (profileInfo.hasSignalStrength)
	{
		properties[L"Signal strength"] = std::to_wstring(static_cast<uint32_t>(profileInfo.signalStrength));
	}

	if (profileInfo.isWWanConnection)
	{
		properties[L"Home provider"] = profileInfo.wwanHomeProviderId.GetRawBuffer(&length);
		properties[L"Access point"] = profileInfo.wwanAccessPointName.GetRawBuffer(&length);
		properties[L"Registration state"] = Networking::NetworkEnumNames::GetWWanRegistrationStateName(profileInfo.wwanNetworkRegistrationState);
		properties[L"Data class"] = Networking::NetworkEnumNames::GetWWanDataClassName(profileInfo.wwanDataClass);
	}

	if (profileInfo.isWLanConnection)
	{
		properties[L"SSID"] = profileInfo.wlanSSID.GetRawBuffer(&length);
	}
}

HRESULT Networking::IPInformation::GetAllNetworkAdapters(std::vector<std::pair<WRL::ComPtr<INetworkAdapter>, Utilities::HString>>* networkAdapters)
{
	Etw::EtwScopedEvent convertEvent("IPInformation", "Get all network adapters");
	HRESULT hr;

	WRL::ComPtr<INetworkInformationStatics> networkInformation;
	hr = Windows::Foundation::GetActivationFactory(Utilities::HStringReference(L"Windows.Networking.Connectivity.NetworkInformation"), &networkInformation);
	FastFailIfFailed(hr);

	WRL::ComPtr<IVectorView<HostName*>> hostNames;
	hr = networkInformation->GetHostNames(&hostNames);
	ReturnIfFailed(hr);

	uint32_t hostNameCount;
	hr = hostNames->get_Size(&hostNameCount);
	ReturnIfFailed(hr);

	for (uint32_t i = 0; i < hostNameCount; i++)
	{
		WRL::ComPtr<IHostName> hostName;
		hr = hostNames->GetAt(i, &hostName);
		ContinueIfFailed(hr);

		WRL::ComPtr<IIPInformation> ipInformation;
		hr = hostName->get_IPInformation(&ipInformation);
		ContinueIfFailed(hr);

		if (ipInformation == nullptr)
			continue;

		Utilities::HString address;
		hr = hostName->get_CanonicalName(&address);
		ContinueIfFailed(hr);

		WRL::ComPtr<INetworkAdapter> networkAdapter;
		hr = ipInformation->get_NetworkAdapter(&networkAdapter);
		ContinueIfFailed(hr);

		networkAdapters->emplace_back(std::move(networkAdapter), std::move(address));
	}

	return S_OK;
}
