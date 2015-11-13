#pragma once
#include "IPInformation.h"
#include "Utilities\CriticalSection.h"

namespace Networking
{

template <typename CallbackType>
class IPInformationGenerator :
	public WRL::RuntimeClass<
		WRL::RuntimeClassFlags<WRL::ClassicCom>,
		ABI::Windows::Foundation::IAsyncOperationCompletedHandler<ABI::Windows::Networking::Connectivity::ConnectionProfile*>>
{
private:
	CallbackType m_Callback;

	std::vector<WRL::ComPtr<ABI::Windows::Networking::Connectivity::IConnectionProfile>> m_ConnectionProfiles;
	Utilities::CriticalSection m_ConnectionProfilesCriticalSection;

	uint32_t m_PendingAsyncOperationCount;
	bool m_Started;

	void CompleteOperation()
	{
		
	}

	void DecrementPendingOperationCount()
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

	HRESULT Perform()
	{
		if (m_Started)
			return E_NOT_VALID_STATE;

		m_Started = true;

		std::set<WRL::ComPtr<ABI::Windows::Networking::Connectivity::INetworkAdapter>> networkAdapters;
		auto hr = IPInformation::GetAllNetworkAdapters(&networkAdapters);
		m_PendingAsyncOperationCount = static_cast<uint32_t>(networkAdapters.size());

		for (auto& adapter : networkAdapters)
		{
			WRL::ComPtr<ABI::Windows::Foundation::IAsyncOperation<ABI::Windows::Networking::Connectivity::ConnectionProfile*>> getProfileOperation;
			hr = adapter->GetConnectedProfileAsync(&getProfileOperation);
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

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Invoke(ABI::Windows::Foundation::IAsyncOperation<ABI::Windows::Networking::Connectivity::ConnectionProfile*>* asyncOperation, ABI::Windows::Foundation::AsyncStatus asyncStatus) override
	{
		if (asyncStatus == Completed)
		{			
			WRL::ComPtr<ABI::Windows::Networking::Connectivity::IConnectionProfile> connectionProfile;
			auto hr = asyncOperation->GetResults(&connectionProfile);

			if (SUCCEEDED(hr))
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
HRESULT GenerateIPInformationAsync(CallbackType&& callbackType)
{
	auto generator = WRL::Make<IPInformationGenerator<CallbackType>>(std::forward<CallbackType>(callbackType));
	return generator->Perform();
}

}