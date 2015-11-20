#pragma once

#include "Utilities\HString.h"

namespace Networking
{

struct ConnectionProfileInformation;
struct ConnectionProperties;

namespace IPInformation
{

HRESULT GetAllNetworkAdapters(std::vector<std::pair<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>, Utilities::HString>>* networkAdapters);
HRESULT FillConnectionProfileInformation(HSTRING address, ABI::Windows::Networking::Connectivity::IConnectionProfile* connectionProfile, ConnectionProfileInformation* profileInformation);
void ConvertConnectionProfileInformationToConnectionProperties(const ConnectionProfileInformation& profileInformation, ConnectionProperties& connectionProperties);
HRESULT SubscribeToOnNetworkStatusChanged(ABI::Windows::Networking::Connectivity::INetworkStatusChangedEventHandler* eventHandler, EventRegistrationToken* eventToken);
HRESULT UnsubscribeFromOnNetworkStatusChanged(EventRegistrationToken eventToken);

}

}