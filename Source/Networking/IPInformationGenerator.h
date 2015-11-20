#pragma once

#include "ConnectionProfileInformation.h"
#include "ConnectionProperties.h"
#include "Etw\Etw.h"
#include "IPInformation.h"
#include "Utilities\CriticalSection.h"
#include "Utilities\HString.h"

inline bool operator<(const GUID& left, const GUID& right)
{
	return memcmp(&left, &right, sizeof(GUID)) < 0;
}

namespace Networking
{

template <typename CallbackType>
class IPInformationGenerator :
	public WRL::RuntimeClass<
		WRL::RuntimeClassFlags<WRL::ClassicCom>,
		ABI::Windows::Foundation::IAsyncOperationCompletedHandler<ABI::Windows::Networking::Connectivity::ConnectionProfile*>,
		WRL::FtmBase>
{
private:
	CallbackType m_Callback;

	std::vector<ConnectionProfileInformation> m_ConnectionsInformation;
	std::map<GUID, Utilities::HString> m_NetworkAdapterAddresses;
	Utilities::CriticalSection m_ConnectionProfilesCriticalSection;

	uint32_t m_PendingAsyncOperationCount;
	bool m_Started;

	inline void FillNetworkAdapterAddresses(const std::vector<std::pair<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>, Utilities::HString>>& adapterInfos)
	{
		Etw::EtwScopedEvent fillAdapterAddressesEvent("IPInformationGenerator", "Fill network adapter addresses");

		for (auto& adapterInfo : adapterInfos)
		{
			GUID adapterId;
			auto hr = adapterInfo.first->get_NetworkAdapterId(&adapterId);
			ContinueIfFailed(hr);

			m_NetworkAdapterAddresses.emplace(adapterId, adapterInfo.second);
		}
	}

	inline void FireFindConnectionProfilesTasks(const std::vector<std::pair<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>, Utilities::HString>>& adapterInfos)
	{
		Etw::EtwScopedEvent fireTasksEvent("IPInformationGenerator", "Fire find connection profiles tasks");
		m_PendingAsyncOperationCount = static_cast<uint32_t>(adapterInfos.size());

		if (m_PendingAsyncOperationCount == 0)
		{
			CompleteOperation();
			return;
		}

		for (auto& adapterInfo : adapterInfos)
		{
			WRL::ComPtr<ABI::Windows::Foundation::IAsyncOperation<ABI::Windows::Networking::Connectivity::ConnectionProfile*>> getProfileOperation;
			auto hr = adapterInfo.first->GetConnectedProfileAsync(&getProfileOperation);
			if (FAILED(hr))
			{
				DecrementPendingOperationCount();
				continue;
			}

			hr = getProfileOperation->put_Completed(this);
			if (FAILED(hr))
			{
				DecrementPendingOperationCount();
				continue;
			}
		}
	}

	void CompleteOperation()
	{
		std::vector<ConnectionProperties> connectionProperties(m_ConnectionsInformation.size());

		for (size_t i = 0; i < m_ConnectionsInformation.size(); i++)
			IPInformation::ConvertConnectionProfileInformationToConnectionProperties(m_ConnectionsInformation[i], connectionProperties[i]);

		std::sort(connectionProperties.begin(), connectionProperties.end(), [](const ConnectionProperties& left, const ConnectionProperties& right) { return left.name < right.name; });

		auto hr = m_Callback(connectionProperties);
		Assert(SUCCEEDED(hr));
	}

	inline void DecrementPendingOperationCount()
	{
		auto newCount =	InterlockedDecrement(&m_PendingAsyncOperationCount);

		if (newCount == 0)
			CompleteOperation();
	}

	inline HSTRING GetConnectionProfileAddress(ABI::Windows::Networking::Connectivity::IConnectionProfile* profile)
	{
		Etw::EtwScopedEvent getAddressEvent("IPInformationGenerator", "Get IConnectionProfile address");

		WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter> adapter;
		auto hr = profile->get_NetworkAdapter(&adapter);
		if (FAILED(hr))
			return nullptr;

		GUID adapterId;
		hr = adapter->get_NetworkAdapterId(&adapterId);
		if (FAILED(hr))
			return nullptr;

		return m_NetworkAdapterAddresses[adapterId];
	}

public:
	IPInformationGenerator(CallbackType&& callback) :
		m_Callback(std::forward<CallbackType>(callback)),
		m_PendingAsyncOperationCount(0),
		m_Started(false)
	{
	}

	inline HRESULT Perform()
	{
		if (m_Started)
			return E_NOT_VALID_STATE;

		m_Started = true;

		std::vector<std::pair<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>, Utilities::HString>> adapters;
		auto hr = IPInformation::GetAllNetworkAdapters(&adapters);
		ReturnIfFailed(hr);

		FillNetworkAdapterAddresses(adapters);
		FireFindConnectionProfilesTasks(adapters);

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Invoke(ABI::Windows::Foundation::IAsyncOperation<ABI::Windows::Networking::Connectivity::ConnectionProfile*>* asyncOperation, ABI::Windows::Foundation::AsyncStatus asyncStatus) override
	{
		if (asyncStatus == Completed)
		{			
			WRL::ComPtr<ABI::Windows::Networking::Connectivity::IConnectionProfile> connectionProfile;
			auto hr = asyncOperation->GetResults(&connectionProfile);

			if (SUCCEEDED(hr) && connectionProfile != nullptr)
			{
				ConnectionProfileInformation connectionInfo;
				auto connectionProfileAddress = GetConnectionProfileAddress(connectionProfile.Get());
				IPInformation::FillConnectionProfileInformation(connectionProfileAddress, connectionProfile.Get(), &connectionInfo);

				Utilities::CriticalSection::Lock lock(m_ConnectionProfilesCriticalSection);
				m_ConnectionsInformation.push_back(std::move(connectionInfo));
			}
		}

		DecrementPendingOperationCount();
		return S_OK;
	}
};

template <typename CallbackType>
inline HRESULT GenerateIPInformationAsync(CallbackType&& callbackType)
{
	auto generator = WRL::Make<IPInformationGenerator<CallbackType>>(std::forward<CallbackType>(callbackType));
	return generator->Perform();
}

}