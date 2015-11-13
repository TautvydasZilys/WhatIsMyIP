#pragma once

namespace Utilities
{

class HStringBuilder;

}

namespace Networking
{

namespace IPInformation
{

HRESULT GetAllNetworkAdapters(std::set<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>>* networkAdapters);
HRESULT FillConnectionProfileInformation(ABI::Windows::Networking::Connectivity::IConnectionProfile* connectionProfile, Utilities::HStringBuilder& builder);

}

}