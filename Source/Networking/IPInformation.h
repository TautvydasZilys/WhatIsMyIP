#pragma once

namespace Networking
{

struct ConnectionProperties;

namespace IPInformation
{

HRESULT GetAllNetworkAdapters(std::vector<std::pair<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>, WRL::HString>>* networkAdapters);
HRESULT FillConnectionProfileInformation(HSTRING address, ABI::Windows::Networking::Connectivity::IConnectionProfile* connectionProfile, ConnectionProperties& connectionProperties);

}

}