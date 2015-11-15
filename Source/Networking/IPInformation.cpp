#include "PrecompiledHeader.h"
#include "ConnectionProperties.h"
#include "IPInformation.h"
#include "NetworkEnumNames.h"
#include "Utilities\EventHandler.h"

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Networking;
using namespace ABI::Windows::Networking::Connectivity;

using namespace Networking;
using namespace Networking::IPInformation;

struct ConnectionProfileInformation
{
	WRL::HString name;
	WRL::HString address;
	NetworkConnectivityLevel connectivityLevel;

	NetworkCostType networkCostType;
	boolean isRoaming;

	uint32_t megabytesUsed;
	uint32_t megabytesLimit;
	bool hasLimit;

	NetworkAuthenticationType authenticationType;
	NetworkEncryptionType encryptionType;

	uint32_t interfaceType;
	NetworkTypes networkType;

	bool hasSignalStrength;
	uint8_t signalStrength;

	boolean isWWanConnection;
	WRL::HString wwanHomeProviderId;
	WRL::HString wwanAccessPointName;
	WwanNetworkRegistrationState wwanNetworkRegistrationState;
	WwanDataClass wwanDataClass;

	boolean isWLanConnection;
	WRL::HString wlanSSID;
};

static HRESULT GatherProfileInformation(HSTRING address, IConnectionProfile* connectionProfile, ConnectionProfileInformation* profileInfo)
{
	auto hr = profileInfo->address.Set(address);
	ReturnIfFailed(hr);

	hr = connectionProfile->get_ProfileName(profileInfo->name.GetAddressOf());
	ReturnIfFailed(hr);

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

	// DataPlan
	{
		WRL::ComPtr<IDataPlanStatus> dataPlanStatus;
		hr = connectionProfile->GetDataPlanStatus(&dataPlanStatus);
		ReturnIfFailed(hr);

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

		hr = wwanConnectionDetails->get_HomeProviderId(profileInfo->wwanHomeProviderId.GetAddressOf());
		ReturnIfFailed(hr);

		hr = wwanConnectionDetails->get_AccessPointName(profileInfo->wwanAccessPointName.GetAddressOf());
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

		hr = wlanConnectionDetails->GetConnectedSsid(profileInfo->wlanSSID.GetAddressOf());
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

	return S_OK;
}

static void ConvertConnectionProfileInformationToConnectionProperties(const ConnectionProfileInformation& profileInfo, ConnectionProperties& connectionProperties)
{
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

HRESULT Networking::IPInformation::GetAllNetworkAdapters(std::vector<std::pair<WRL::ComPtr<INetworkAdapter>, WRL::HString>>* networkAdapters)
{
	HRESULT hr;

	WRL::ComPtr<INetworkInformationStatics> networkInformation;
	hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.Networking.Connectivity.NetworkInformation").Get(), &networkInformation);
	ReturnIfFailed(hr);

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

		WRL::HString address;
		hr = hostName->get_CanonicalName(address.GetAddressOf());
		ContinueIfFailed(hr);

		WRL::ComPtr<INetworkAdapter> networkAdapter;
		hr = ipInformation->get_NetworkAdapter(&networkAdapter);
		ContinueIfFailed(hr);

		networkAdapters->emplace_back(std::move(networkAdapter), std::move(address));
	}

	return S_OK;
}

HRESULT Networking::IPInformation::FillConnectionProfileInformation(HSTRING address, IConnectionProfile* connectionProfile, ConnectionProperties& connectionProperties)
{
	ConnectionProfileInformation profileInfo;
	ZeroMemory(&profileInfo, sizeof(profileInfo));
	auto hr = GatherProfileInformation(address, connectionProfile, &profileInfo);
	ReturnIfFailed(hr);

	ConvertConnectionProfileInformationToConnectionProperties(profileInfo, connectionProperties);
	return S_OK;
}

HRESULT Networking::IPInformation::SubscribeToOnNetworkStatusChanged(ABI::Windows::Networking::Connectivity::INetworkStatusChangedEventHandler* eventHandler, EventRegistrationToken* eventToken)
{
	WRL::ComPtr<INetworkInformationStatics> networkInformation;
	auto hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.Networking.Connectivity.NetworkInformation").Get(), &networkInformation);
	ReturnIfFailed(hr);

	return networkInformation->add_NetworkStatusChanged(eventHandler, eventToken);
}

HRESULT Networking::IPInformation::UnsubscribeFromOnNetworkStatusChanged(EventRegistrationToken eventToken)
{
	WRL::ComPtr<INetworkInformationStatics> networkInformation;
	auto hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.Networking.Connectivity.NetworkInformation").Get(), &networkInformation);
	ReturnIfFailed(hr);

	return networkInformation->remove_NetworkStatusChanged(eventToken);
}