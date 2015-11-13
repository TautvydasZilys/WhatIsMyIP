#include "PrecompiledHeader.h"
#include "IPInformation.h"
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

	return S_OK;
}

static void AppendProfileInformation(const ConnectionProfileInformation& profileInfo, Utilities::HStringBuilder& builder)
{
	builder += profileInfo.name.Get();
	builder += L"\r\n\t";
	
	builder += L"Address: ";
	builder += profileInfo.address.Get();
	builder += L"\r\n\t";

	switch (profileInfo.connectivityLevel)
	{
	case NetworkConnectivityLevel_None:
		builder += L"Connectivity level: None";
		break;

	case NetworkConnectivityLevel_LocalAccess:
		builder += L"Connectivity level: Local access";
		break;

	case NetworkConnectivityLevel_ConstrainedInternetAccess:
		builder += L"Connectivity level: Constrained internet access";
		break;

	case NetworkConnectivityLevel_InternetAccess:
		builder += L"Connectivity level: Internet access";
		break;

	default:
		builder += L"Connectivity level: Unknown";
		break;
	}

	builder += L"\r\n\t";

	switch (profileInfo.networkCostType)
	{
	case NetworkCostType_Fixed:
		builder += L"Network cost type: Fixed";
		break;

	case NetworkCostType_Variable:
		builder += L"Network cost type: Variable";
		break;

	case NetworkCostType_Unrestricted:
		builder += L"Network cost type: Unrestricted";
		break;

	case NetworkCostType_Unknown:
	default:
		builder += L"Network cost type: Unknown";
		break;
	}

	builder += L"\r\n\t";

	if (profileInfo.isRoaming)
	{
		builder += L"Is roaming: Yes";
	}
	else
	{
		builder += L"Is roaming: No";
	}

	builder += L"\r\n\t";

	if (profileInfo.networkCostType == NetworkCostType_Fixed || profileInfo.networkCostType == NetworkCostType_Variable)
	{
		builder += L"Data used: ";
		builder += profileInfo.megabytesUsed;
		builder += L" MB\r\n\t";

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

		builder += L"\r\n\t";
	}

	builder += L"Authentication type: ";

	switch (profileInfo.authenticationType)
	{
	case NetworkAuthenticationType_None:
		builder += L"None";
		break;

	case NetworkAuthenticationType_Unknown:
		builder += L"Unknown";
		break;

	case NetworkAuthenticationType_Open80211:
		builder += L"Open80211";
		break;

	case NetworkAuthenticationType_SharedKey80211:
		builder += L"SharedKey80211";
		break;

	case NetworkAuthenticationType_Wpa:
		builder += L"Wpa";
		break;

	case NetworkAuthenticationType_WpaPsk:
		builder += L"WpaPsk";
		break;

	case NetworkAuthenticationType_WpaNone:
		builder += L"WpaNone";
		break;

	case NetworkAuthenticationType_Rsna:
		builder += L"Rsna";
		break;

	case NetworkAuthenticationType_RsnaPsk:
		builder += L"RsnaPsk";
		break;

	case NetworkAuthenticationType_Ihv:
		builder += L"Ihv";
		break;

	default:
		builder += L"Unknown";
		break;
	}

	builder += L"\r\n\t";
	builder += L"Encryption type: ";

	switch (profileInfo.encryptionType)
	{
	case NetworkEncryptionType_None:
		builder += L"None";
		break;

	case NetworkEncryptionType_Unknown:
		builder += L"Unknown";
		break;

	case NetworkEncryptionType_Wep:
		builder += L"Wep";
		break;

	case NetworkEncryptionType_Wep40:
		builder += L"Wep40";
		break;

	case NetworkEncryptionType_Wep104:
		builder += L"Wep104";
		break;

	case NetworkEncryptionType_Tkip:
		builder += L"Tkip";
		break;

	case NetworkEncryptionType_Ccmp:
		builder += L"Ccmp";
		break;

	case NetworkEncryptionType_WpaUseGroup:
		builder += L"WpaUseGroup";
		break;

	case NetworkEncryptionType_RsnUseGroup:
		builder += L"RsnUseGroup";
		break;

	case NetworkEncryptionType_Ihv:
		builder += L"Ihv";
		break;

	default:
		builder += L"Unknown";
		break;
	}

	builder += L"\r\n\t";
	builder += L"Interface type: ";

	switch (profileInfo.interfaceType)
	{
	case 1:
		builder += L"Other";
		break;

	case 6:
		builder += L"Ethernet";
		break;

	case 9:
		builder += L"Token ring";
		break;

	case 23:
		builder += L"PPP";
		break;

	case 24:
		builder += L"Software loopback";
		break;

	case 37:
		builder += L"ATM";
		break;

	case 71:
		builder += L"IEEE 802.11 wireless";
		break;

	case 131:
		builder += L"Tunnel type encapsulation";
		break;

	case 144:
		builder += L"IEEE 1394 (Firewire) high performance serial bus";
		break;
	}

	builder += L"\r\n\t";
	builder += L"Network type: ";

	switch (profileInfo.networkType)
	{
	case NetworkTypes_None:
		builder += L"None";
		break;

	case NetworkTypes_Internet:
		builder += L"Internet";
		break;
	
	case NetworkTypes_PrivateNetwork:
		builder += L"Private network";
		break;

	default:
		builder += L"Unknown";
		break;
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

		WRL::HString name;
		hr = hostName->get_CanonicalName(name.GetAddressOf());
		ContinueIfFailed(hr);

		WRL::ComPtr<INetworkAdapter> networkAdapter;
		hr = ipInformation->get_NetworkAdapter(&networkAdapter);
		ContinueIfFailed(hr);

		networkAdapters->emplace_back(std::move(networkAdapter), std::move(name));
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