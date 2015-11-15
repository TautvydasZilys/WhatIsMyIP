#pragma once

#include "ConnectionProperties.h"
#include "IPInformation.h"
#include "Utilities\CriticalSection.h"

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

	std::vector<WRL::ComPtr<ABI::Windows::Networking::Connectivity::IConnectionProfile>> m_ConnectionProfiles;
	std::map<GUID, WRL::HString> m_NetworkAdapterAddresses;
	Utilities::CriticalSection m_ConnectionProfilesCriticalSection;

	uint32_t m_PendingAsyncOperationCount;
	bool m_Started;

	inline void FillNetworkAdapterAddresses(const std::vector<std::pair<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>, WRL::HString>>& adapterInfos)
	{
		for (auto& adapterInfo : adapterInfos)
		{
			GUID adapterId;
			auto hr = adapterInfo.first->get_NetworkAdapterId(&adapterId);
			ContinueIfFailed(hr);

			WRL::HString address;
			hr = address.Set(adapterInfo.second.Get());
			ContinueIfFailed(hr);

			m_NetworkAdapterAddresses.emplace(adapterId, std::move(address));
		}
	}

	inline void FireFindConnectionProfilesTasks(const std::vector<std::pair<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>, WRL::HString>>& adapterInfos)
	{
		m_PendingAsyncOperationCount = static_cast<uint32_t>(adapterInfos.size());

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
		HRESULT hr;
		std::vector<ConnectionProperties> connectionProperties;
		connectionProperties.reserve(m_ConnectionProfiles.size());

		for (const auto& profile : m_ConnectionProfiles)
		{
			WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter> adapter;
			hr = profile->get_NetworkAdapter(&adapter);
			ContinueIfFailed(hr);

			GUID adapterId;
			hr = adapter->get_NetworkAdapterId(&adapterId);
			ContinueIfFailed(hr);

			ConnectionProperties properties;
			hr = IPInformation::FillConnectionProfileInformation(m_NetworkAdapterAddresses[adapterId].Get(), profile.Get(), properties);
			ContinueIfFailed(hr);

			connectionProperties.push_back(std::move(properties));
		}

		hr = m_Callback(connectionProperties);
		Assert(SUCCEEDED(hr));
	}

	inline void DecrementPendingOperationCount()
	{
		auto newCount =	InterlockedDecrement(&m_PendingAsyncOperationCount);

		if (newCount == 0)
			CompleteOperation();
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

		std::vector<std::pair<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>, WRL::HString>> adapters;
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
				Utilities::CriticalSection::Lock lock(m_ConnectionProfilesCriticalSection);
				m_ConnectionProfiles.push_back(std::move(connectionProfile));
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