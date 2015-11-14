#pragma once

namespace Networking
{

namespace NetworkEnumNames
{

const wchar_t* GetIANAInterfaceTypeName(uint32_t interfaceType);
const wchar_t* GetConnectivityLevelName(ABI::Windows::Networking::Connectivity::NetworkConnectivityLevel connectivityLevel);
const wchar_t* GetNetworkCostTypeName(ABI::Windows::Networking::Connectivity::NetworkCostType costType);
const wchar_t* GetNetworkAuthenticationTypeName(ABI::Windows::Networking::Connectivity::NetworkAuthenticationType authenticationType);
const wchar_t* GetNetworkEncryptionTypeName(ABI::Windows::Networking::Connectivity::NetworkEncryptionType encryptionType);
const wchar_t* GetNetworkTypeName(ABI::Windows::Networking::Connectivity::NetworkTypes networkTypes);
const wchar_t* GetWWanRegistrationStateName(ABI::Windows::Networking::Connectivity::WwanNetworkRegistrationState wwanNetworkRegistrationState);
const wchar_t* GetWWanDataClassName(ABI::Windows::Networking::Connectivity::WwanDataClass wwanDataClass);

}

}