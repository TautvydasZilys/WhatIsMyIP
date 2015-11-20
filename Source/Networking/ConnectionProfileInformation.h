#pragma once

#include "Utilities\HString.h"

namespace Networking
{

struct ConnectionProfileInformation
{
	Utilities::HString name;
	Utilities::HString address;
	ABI::Windows::Networking::Connectivity::NetworkConnectivityLevel connectivityLevel;

	ABI::Windows::Networking::Connectivity::NetworkCostType networkCostType;
	boolean isRoaming;

	uint32_t megabytesUsed;
	uint32_t megabytesLimit;
	bool hasLimit;

	ABI::Windows::Networking::Connectivity::NetworkAuthenticationType authenticationType;
	ABI::Windows::Networking::Connectivity::NetworkEncryptionType encryptionType;

	uint32_t interfaceType;
	ABI::Windows::Networking::Connectivity::NetworkTypes networkType;

	bool hasSignalStrength;
	uint8_t signalStrength;

	boolean isWWanConnection;
	Utilities::HString wwanHomeProviderId;
	Utilities::HString wwanAccessPointName;
	ABI::Windows::Networking::Connectivity::WwanNetworkRegistrationState wwanNetworkRegistrationState;
	ABI::Windows::Networking::Connectivity::WwanDataClass wwanDataClass;

	boolean isWLanConnection;
	Utilities::HString wlanSSID;

	inline ConnectionProfileInformation()
	{
		ZeroMemory(this, sizeof(ConnectionProfileInformation));
	}

	// Let's not do copying for now
	ConnectionProfileInformation(const ConnectionProfileInformation&) = delete;
	ConnectionProfileInformation& operator=(const ConnectionProfileInformation&) = delete;

	inline ConnectionProfileInformation(ConnectionProfileInformation&& other)
	{
		memcpy(this, &other, sizeof(ConnectionProfileInformation));
		ZeroMemory(&other, sizeof(ConnectionProfileInformation));
	}

	inline ConnectionProfileInformation& operator=(ConnectionProfileInformation&& other)
	{
		this->~ConnectionProfileInformation();
		memcpy(this, &other, sizeof(ConnectionProfileInformation));
		ZeroMemory(&other, sizeof(ConnectionProfileInformation));
		return *this;
	}
};


}