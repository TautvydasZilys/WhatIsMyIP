#pragma once

#include "ConnectionProperties.h"
#include "Utilities\AsyncCreationSingleton.h"
#include "Utilities\CriticalSection.h"

namespace Networking
{

class IPInformationWatcher :
	public WRL::RuntimeClass<
		WRL::RuntimeClassFlags<WRL::ClassicCom>,
		ABI::Windows::Networking::Connectivity::INetworkStatusChangedEventHandler,
		WRL::FtmBase>,
	public Utilities::AsyncCreationSingleton<IPInformationWatcher>
{
public:
	typedef std::function<void(const std::vector<Networking::ConnectionProperties>& connectionProperties)> IPInformationAvailableCallback;

private:
	WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkInformationStatics> m_NetworkInformation;
	EventRegistrationToken m_NetworkStatusChangedToken;
	uint32_t m_NextRegistrationToken;

	std::vector<std::pair<uint32_t, IPInformationAvailableCallback>> m_Callbacks;
	Utilities::CriticalSection m_CallbacksCriticalSection;
	bool m_HasIPInformation;
	std::vector<Networking::ConnectionProperties> m_ConnectionProperties;

	IPInformationWatcher();
	HRESULT Initialize();
	void Cleanup();

	HRESULT UpdateIPInformation();

	static HRESULT Create(IPInformationWatcher** watcher);
	void Destroy();

	friend WRL::ComPtr<IPInformationWatcher> WRL::Make<IPInformationWatcher>();
	friend class Utilities::AsyncCreationSingleton<IPInformationWatcher>;

public:
	virtual HRESULT STDMETHODCALLTYPE Invoke(IInspectable* sender) override;

	static uint32_t SubscribeToChanges(IPInformationAvailableCallback&& callback);
	static void UnsubscribeFromChanges(uint32_t registrationToken);
	static void ForceUpdateIPInformation();
};

}