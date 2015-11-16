#include "PrecompiledHeader.h"
#include "NetworkEnumNames.h"

using namespace ABI::Windows::Networking::Connectivity;

const std::initializer_list<wchar_t*> kIANAInterfaceTypeNames =
{
	L"Unknown",
	L"Other",
	L"BBN Report 1822",
	L"BBN Report 1822",
	L"BBN Report 1822",
	L"X.25",
	L"Ethernet",
	L"IEEE802.3",
	L"Token Bus",
	L"Token Ring",
	L"ISO88026",
	L"Star LAN",
	L"Proteon 10Mbit",
	L"Proteon 80Mbit",
	L"HyperChannel",
	L"FDDI",
	L"LAP B",
	L"SDLC",
	L"T1/E1 Carrier",
	L"E1 Carrier",
	L"Basic ISDN",
	L"Primary ISDN",
	L"Point-to-Point Serial protocol",
	L"Point-to-Point Protocol",
	L"Software loopback",
	L"EON",
	L"Ethernet 3Mbit",
	L"NSIP",
	L"SLIP",
	L"Ultra",
	L"DS3/E3 Interface",
	L"SMDS Interface",
	L"Frame Relay",
	L"RS-232",
	L"Parallel Printer",
	L"ARC network",
	L"ARC network plus",
	L"ATM",
	L"MIOX25",
	L"SONET or SDH",
	L"X.25 packet level",
	L"802.2 LLC",
	L"Local Talk",
	L"SMDS DXI",
	L"Frame Relay DCE",
	L"V.35",
	L"HSSI",
	L"HIPPI",
	L"Generic Modem",
	L"AAL5 over ATM",
	L"Sonet Path",
	L"Sonet VT",
	L"SMDS Inter-Carrier Interface",
	L"Proprietary virtual/internal interface",
	L"Proprietary multi-link multiplexing",
	L"100BaseVG",
	L"Fibre Channel",
	L"HIPPI interfaces",
	L"Interconnet over FR",
	L"ATM Emulated LAN for 802.3",
	L"ATM Emulated LAN for 802.5",
	L"ATM Emulated circuit",
	L"FastEther",
	L"ISDN and X.25",
	L"CCITT V.11/X.21",
	L"CCITT V.36",
	L"CCITT G703 at 64Kbps",
	L"CCITT G703 at 2Mbps",
	L"SNA QLLC",
	L"FastEtherFX",
	L"Channel",
	L"IEEE 802.11 wireless",
	L"IBM System 360/370 OEMI Channel",
	L"IBM Enterprise Systems Connection",
	L"Data Link Switching",
	L"ISDN S/T interface",
	L"ISDN U interface",
	L"Link Access Protocol D",
	L"IP Switching Objects",
	L"Remote Source Route Bridging",
	L"ATM Logical Port",
	L"Digital Signal Level 0",
	L"group of ds0s on the same ds1",
	L"Bisynchronous Protocol",
	L"Asynchronous Protocol",
	L"Combat Net Radio",
	L"ISO 802.5r DTR",
	L"Enhanced Pos Loc Report Sys",
	L"Appletalk Remote Access",
	L"Proprietary Connectionless Proto.",
	L"CCITT-ITU X.29 PAD Protocol",
	L"CCITT-ITU X.3 PAD Facility",
	L"Multiproto Interconnect over FR",
	L"CCITT-ITU X213",
	L"Asymmetric Digital Subscriber Loop",
	L"Rate-Adapt. Digital Subscriber Loop",
	L"Symmetric Digital Subscriber Loop",
	L"Very H-Speed Digital Subscriber Loop",
	L"ISO 802.5 CRFP",
	L"Myricom Myrinet",
	L"Voice recEive and transMit (E&M)",
	L"Voice Foreign Exchange Office",
	L"Voice Foreign Exchange Station",
	L"Voice encapsulation",
	L"Voice over IP encapsulation",
	L"ATM DXI",
	L"ATM FUNI",
	L"ATM IMA",
	L"PPP Multilink Bundle",
	L"IBM ipOverCdlc",
	L"IBM Common Link Access to Workstn",
	L"IBM stackToStack",
	L"IBM VIPA",
	L"IBM multi-protocol channel support",
	L"IBM ipOverAtm",
	L"ISO 802.5j Fiber Token Ring",
	L"IBM twinaxial data link control",
	L"Gigabit Ethernet",
	L"HDLC",
	L"LAP F",
	L"V.37",
	L"Multi-Link Protocol",
	L"X25 Hunt Group",
	L"Transp HDLC",
	L"Interleave channel",
	L"Fast channel",
	L"IP (for APPN HPR in IP networks)",
	L"CATV Mac Layer",
	L"CATV Downstream interface",
	L"CATV Upstream interface",
	L"Avalon Parallel Processor",
	L"Tunnel type encapsulation",
	L"coffee pot",
	L"Circiut Emulation Service",
	L"ATM Sub Interface",
	L"Layer 2 Virtual LAN using 802.1Q",
	L"Layer 3 Virtual LAN - IP",
	L"Layer 3 Virtual LAN - IPX",
	L"IP over Power Lines",
	L"Multimedia Mail over IP",
	L"Dynamic synchronous Transfer Mode",
	L"Data Communications Network",
	L"IP Forwarding Interface",
	L"Multi-rate Symmetric DSL",
	L"IEEE1394 (Firewire) High Performance Serial Bus",
	L"HIPPI-6400",
	L"DVB-RCC MAC Layer",
	L"DVB-RCC Downstream Channel",
	L"DVB-RCC Upstream Channel",
	L"ATM Virtual Interface",
	L"MPLS Tunnel Virtual Interface",
	L"Spatial Reuse Protocol",
	L"Voice over ATM",
	L"Voice Over Frame Relay",
	L"Digital Subscriber Loop over ISDN",
	L"Avici Composite Link Interface",
	L"SS7 Signaling Link",
	L"Prop. P2P wireless interface",
	L"Frame forward Interface",
	L"Multiprotocol over ATM AAL5",
	L"USB Interface",
	L"IEEE 802.3ad Link Aggregate",
	L"BGP Policy Accounting",
	L"FRF.16 Multilik Frame Relay",
	L"H323 Gatekeeper",
	L"H323 Voice and Video Proxy",
	L"MPLS",
	L"Multi-frequency signaling link",
	L"High Bit-Rate DSL, 2nd gen.",
	L"Multirate HDSL2",
	L"Facility Data Link (4Kbps) on a DS1",
	L"Packet over SONET/SDH Interface",
	L"DVB-ASI Input",
	L"DVB-ASI Output",
	L"Power Line Communications",
	L"Non-Facility Associated Signaling",
	L"TROO8",
	L"Remote Digital Terminal",
	L"Integrated Digital Terminal",
	L"ISUP",
	L"Cisco proprietary Maclayer",
	L"Cisco proprietary Downstream",
	L"Cisco proprietary Upstream",
	L"HIPERLAN Type 2 Radio Interface",
	L"WMAN",
	L"SONET Overhead Channel",
	L"Digital Wrapper Overhead",
	L"ATM adaptation layer 2",
	L"MAC layer over radio links",
	L"ATM over radio links",
	L"Inter-Machine Trunks",
	L"Multiple Virtual Lines DSL",
	L"Long Reach DSL",
	L"Frame Relay DLCI End Point",
	L"ATM VCI End Point",
	L"Optical Channel",
	L"Optical Transport",
	L"Proprietary ATM",
	L"Voice Over Cable Interface",
	L"Infiniband",
	L"TE Link",
	L"Q.2931",
	L"Virtual Trunk Group",
	L"SIP Trunk Group",
	L"SIP Signaling",
	L"CATV Upstream Channel",
	L"Acorn Econet",
	L"FSAN 155Mb Symetrical PON interface",
	L"FSAN 622Mb Symetrical PON interface",
	L"Transparent bridge interface",
	L"Interface common to multiple lines",
	L"voice E&M Feature Group D",
	L"voice FGD Exchange Access North American",
	L"voice Direct Inward Dialing",
	L"MPEG transport interface",
	L"6to4 interface",
	L"GTP (GPRS Tunneling Protocol)",
	L"Paradyne EtherLoop 1",
	L"Paradyne EtherLoop 2",
	L"Optical Channel Group",
	L"HomePNA ITU-T G.989",
	L"Generic Framing Procedure (GFP)",
	L"Layer 2 Virtual LAN using Cisco ISL",
	L"Acteleis proprietary MetaLOOP High Speed Link",
	L"FCIP Link",
	L"Resilient Packet Ring Interface Type",
	L"RF Qam Interface",
	L"Link Management Protocol",
	L"VectaStar",
	L"Interface Asymmetric Digital Subscriber Loop",
	L"ADSL Version 2",
	L"MACSecControlled",
	L"MACSecUncontrolled",
	L"Avici Optical Ethernet Aggregate",
	L"atmbond",
	L"voice FGD Operator Services",
	L"MultiMedia over Coax Alliance",
	L"IEEE 802.16 WMAN interface",
	L"ADSL Version 2 Plus",
	L"DVB-RCS MAC Layer",
	L"DVB Satellite TDM",
	L"DVB-RCS TDMA",
	L"LAPS based on ITU-T X.86/Y.1323",
	L"3GPP WWAN",
	L"3GPP2 WWAN",
	L"Voice P-phone EBS physical interface",
	L"Pseudowire interface type",
	L"Internal LAN on a bridge (IEEE 802.1ap)",
	L"Provider Instance Port on a bridge (IEEE 802.1ah PBB)",
	L"Alcatel-Lucent Ethernet Link",
	L"Gigabit-capable passive optical networks (ITU-T G.948)",
	L"VDS1 line Version 2",
	L"WLAN Profile Interface",
	L"WLAN BSS Interface",
	L"WTP Virtual Radio Interface",
	L"Bitsport",
	L"DOCSIS CATV Upstream RF Port",
	L"CATV downstream RF port",
	L"VMware Virtual Network Interface",
	L"IEEE 802.15.4 WPAN interface",
	L"OTN Optical Data Unit",
	L"OTN Optical channel Transport Unit",
	L"VPLS Forwarding Instance Interface Type",
	L"G.998.1 bonded interface",
	L"G.998.2 bonded interface",
	L"G.998.3 bonded interface",
	L"Ethernet Passive Optical Networks",
	L"EPON Optical Network Unit",
	L"EPON physical User to Network interface",
	L"The emulation of a point-to-point link over the EPON layer",
	L"GPON Optical Network Unit",
	L"GPON physical User to Network interface",
	L"VMware NIC Team",
	L"Reserved",
	L"Reserved",
	L"Reserved",
	L"Reserved",
	L"CATV Downstream OFDM interface",
	L"CATV Upstream OFDMA interface",
	L"G.fast port",
	L"SDCI (IO-Link)"
};

const wchar_t* Networking::NetworkEnumNames::GetIANAInterfaceTypeName(uint32_t interfaceType)
{
	if (interfaceType < kIANAInterfaceTypeNames.size())
		return *(kIANAInterfaceTypeNames.begin() + interfaceType);

	return L"Unknown";
}

const wchar_t* Networking::NetworkEnumNames::GetConnectivityLevelName(NetworkConnectivityLevel connectivityLevel)
{
	switch (connectivityLevel)
	{
	case NetworkConnectivityLevel_None:
		return L"None";

	case NetworkConnectivityLevel_LocalAccess:
		return L"Local access";

	case NetworkConnectivityLevel_ConstrainedInternetAccess:
		return L"Constrained internet access";

	case NetworkConnectivityLevel_InternetAccess:
		return L"Internet access";

	default:
		return L"Unknown";
	}
}

const wchar_t* Networking::NetworkEnumNames::GetNetworkCostTypeName(NetworkCostType costType)
{
	switch (costType)
	{
	case NetworkCostType_Fixed:
		return L"Fixed";

	case NetworkCostType_Variable:
		return L"Variable";

	case NetworkCostType_Unrestricted:
		return L"Unrestricted";

	case NetworkCostType_Unknown:
	default:
		return L"Unknown";
	}
}

const wchar_t* Networking::NetworkEnumNames::GetNetworkAuthenticationTypeName(NetworkAuthenticationType authenticationType)
{
	switch (authenticationType)
	{
	case NetworkAuthenticationType_None:
		return L"None";

	case NetworkAuthenticationType_Unknown:
	default:
		return L"Unknown";

	case NetworkAuthenticationType_Open80211:
		return L"IEEE 802.11 Open";

	case NetworkAuthenticationType_SharedKey80211:
		return L"IEEE 802.11 Shared Key";

	case NetworkAuthenticationType_Wpa:
		return L"WPA";

	case NetworkAuthenticationType_WpaPsk:
		return L"WPA-PSK";

	case NetworkAuthenticationType_WpaNone:
		return L"WPA-None";

	case NetworkAuthenticationType_Rsna:
		return L"RSNA";

	case NetworkAuthenticationType_RsnaPsk:
		return L"RSNA-PSK";

	case NetworkAuthenticationType_Ihv:
		return L"IHV";
	}
}

const wchar_t* Networking::NetworkEnumNames::GetNetworkEncryptionTypeName(NetworkEncryptionType encryptionType)
{
	switch (encryptionType)
	{
	case NetworkEncryptionType_None:
		return L"None";

	case NetworkEncryptionType_Unknown:
	default:
		return L"Unknown";

	case NetworkEncryptionType_Wep:
		return L"WEP";

	case NetworkEncryptionType_Wep40:
		return L"WEP40";

	case NetworkEncryptionType_Wep104:
		return L"WEP104";

	case NetworkEncryptionType_Tkip:
		return L"TKIP";

	case NetworkEncryptionType_Ccmp:
		return L"CCMP";

	case NetworkEncryptionType_WpaUseGroup:
		return L"WPA";

	case NetworkEncryptionType_RsnUseGroup:
		return L"RSN";

	case NetworkEncryptionType_Ihv:
		return L"IHV";
	}
}

const wchar_t* Networking::NetworkEnumNames::GetNetworkTypeName(NetworkTypes networkType)
{
	switch (networkType)
	{
	case NetworkTypes_None:
		return L"None";

	case NetworkTypes_Internet:
		return L"Internet";

	case NetworkTypes_PrivateNetwork:
		return L"Private network";

	default:
		return L"Unknown";
	}
}

const wchar_t* Networking::NetworkEnumNames::GetWWanRegistrationStateName(WwanNetworkRegistrationState wwanNetworkRegistrationState)
{
	switch (wwanNetworkRegistrationState)
	{
	case WwanNetworkRegistrationState_None:
		return L"None";

	case WwanNetworkRegistrationState_Deregistered:
		return L"Deregistered";

	case WwanNetworkRegistrationState_Searching:
		return L"Searching";

	case WwanNetworkRegistrationState_Home:
		return L"Home";

	case WwanNetworkRegistrationState_Roaming:
		return L"Roaming";

	case WwanNetworkRegistrationState_Partner:
		return L"Partner";

	case WwanNetworkRegistrationState_Denied:
		return L"Denied";

	default:
		return L"Unknown";
	}
}

const wchar_t* Networking::NetworkEnumNames::GetWWanDataClassName(WwanDataClass wwanDataClass)
{
	switch (wwanDataClass)
	{
	case WwanDataClass_None:
		return L"None";

	case WwanDataClass_Gprs:
		return L"GPRS";

	case WwanDataClass_Edge:
		return L"EDGE";

	case WwanDataClass_Umts:
		return L"UMTS";

	case WwanDataClass_Hsdpa:
		return L"HSDPA";

	case WwanDataClass_Hsupa:
		return L"HSUPA";

	case WwanDataClass_LteAdvanced:
		return L"LTE Advanced";

	case WwanDataClass_Cdma1xRtt:
		return L"CDMA 1xRTT";

	case WwanDataClass_Cdma1xEvdo:
		return L"CDMA 1xEV-DO";

	case WwanDataClass_Cdma1xEvdoRevA:
		return L"CDMA 1xEV-DO RevA";

	case WwanDataClass_Cdma1xEvdv:
		return L"CDMA 1xEV-DV";

	case WwanDataClass_Cdma3xRtt:
		return L"CDMA 3xRTT";

	case WwanDataClass_Cdma1xEvdoRevB:
		return L"CDMA 1xEV-DO RevB";

	case WwanDataClass_CdmaUmb:
		return L"CDMA UMB";

	case WwanDataClass_Custom:
		return L"Custom";

	default:
		return L"Unknown";
	}
}
