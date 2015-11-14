#include "PrecompiledHeader.h"
#include "IPInformation.h"
#include "NetworkEnumNames.h"
#include "Utilities\EventHandler.h"
#include "Utilities\HStringBuilder.h"

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Networking;
using namespace ABI::Windows::Networking::Connectivity;

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

static HRESULT GatherProfileInformation(IConnectionProfile* connectionProfile, ConnectionProfileInformation* profileInfo)
{
	auto hr = connectionProfile->get_ProfileName(profileInfo->name.GetAddressOf());
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

static void AppendProfileInformation(const ConnectionProfileInformation& profileInfo, Utilities::HStringBuilder& builder)
{
	builder += profileInfo.name.Get();

	builder += L"\r\n\t";	
	builder += L"Address: ";
	builder += profileInfo.address.Get();

	builder += L"\r\n\t";
	builder += L"Connectivity level: ";
	builder += Networking::NetworkEnumNames::GetConnectivityLevelName(profileInfo.connectivityLevel);

	builder += L"\r\n\t";
	builder += L"Network cost type: ";
	builder += Networking::NetworkEnumNames::GetNetworkCostTypeName(profileInfo.networkCostType);

	builder += L"\r\n\t";

	if (profileInfo.isRoaming)
	{
		builder += L"Is roaming: Yes";
	}
	else
	{
		builder += L"Is roaming: No";
	}

	if (profileInfo.networkCostType == NetworkCostType_Fixed || profileInfo.networkCostType == NetworkCostType_Variable)
	{
		builder += L"\r\n\t";
		builder += L"Data used: ";
		builder += profileInfo.megabytesUsed;
		builder += L" MB";
		
		builder += L"\r\n\t";
		builder += L"Data limit: ";

		if (profileInfo.hasLimit)
		{
			builder += profileInfo.megabytesLimit;
			builder += L" MB";
		}
		else
		{
			builder += L"Unlimited";
		}
	}

	builder += L"\r\n\t";
	builder += L"Authentication type: ";
	builder += Networking::NetworkEnumNames::GetNetworkAuthenticationTypeName(profileInfo.authenticationType);

	builder += L"\r\n\t";
	builder += L"Encryption type: ";
	builder += Networking::NetworkEnumNames::GetNetworkEncryptionTypeName(profileInfo.encryptionType);

	builder += L"\r\n\t";
	builder += L"Interface type: ";
	builder += Networking::NetworkEnumNames::GetIANAInterfaceTypeName(profileInfo.interfaceType);

	builder += L"\r\n\t";
	builder += L"Network type: ";
	builder += Networking::NetworkEnumNames::GetNetworkTypeName(profileInfo.networkType);

	if (profileInfo.hasSignalStrength)
	{
		builder += L"\r\n\t";
		builder += L"Signal strength: ";
		builder += profileInfo.signalStrength;
	}

	if (profileInfo.isWWanConnection)
	{
		builder += L"\r\n\t";
		builder += L"Home provider: ";
		builder += profileInfo.wwanHomeProviderId.Get();

		builder += L"\r\n\t";
		builder += L"Access point: ";
		builder += profileInfo.wwanAccessPointName.Get();

		builder += L"\r\n\t";
		builder += L"Network registration state: ";
		builder += Networking::NetworkEnumNames::GetWWanRegistrationStateName(profileInfo.wwanNetworkRegistrationState);

		builder += L"\r\n\t";
		builder += L"Data class: ";
		builder += Networking::NetworkEnumNames::GetWWanDataClassName(profileInfo.wwanDataClass);		
	}

	if (profileInfo.isWLanConnection)
	{
		builder += L"\r\n\t";
		builder += L"SSID: ";
		builder += profileInfo.wlanSSID.Get();
	}

	builder += L"\r\n\r\n";	
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

HRESULT Networking::IPInformation::FillConnectionProfileInformation(HSTRING address, IConnectionProfile* connectionProfile, Utilities::HStringBuilder& builder)
{
	ConnectionProfileInformation profileInfo;
	ZeroMemory(&profileInfo, sizeof(profileInfo));
	auto hr = GatherProfileInformation(connectionProfile, &profileInfo);
	ReturnIfFailed(hr);

	hr = profileInfo.address.Set(address);
	ReturnIfFailed(hr);

	AppendProfileInformation(profileInfo, builder);
	return S_OK;
}