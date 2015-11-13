#pragma once

namespace Utilities
{

class HStringBuilder;

}

namespace Networking
{

namespace IPInformation
{

HRESULT GetAllNetworkAdapters(std::vector<std::pair<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>, WRL::HString>>* networkAdapters);
HRESULT FillConnectionProfileInformation(HSTRING address, ABI::Windows::Networking::Connectivity::IConnectionProfile* connectionProfile, Utilities::HStringBuilder& builder);

}

}