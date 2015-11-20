#pragma once

#include "HandleHolder.h"
#include "TemplateHelpers\ArgumentTraits.h"

namespace Utilities
{

template <typename T>
class SynchronousOperation : 
	public WRL::RuntimeClass<
		WRL::RuntimeClassFlags<WRL::WinRtClassicComMix>, 
		ABI::Windows::Foundation::IAsyncOperationCompletedHandler<T>,
		WRL::FtmBase>
{
public:
	typedef typename std::remove_pointer<typename TemplateHelpers::ArgumentTraits<decltype(&ABI::Windows::Foundation::IAsyncOperation<T>::GetResults)>::Arg1>::type ResultType;

private:
	HRESULT m_Hr;
	ResultType m_Result;
	HandleHolder m_Event;

	virtual HRESULT STDMETHODCALLTYPE Invoke(ABI::Windows::Foundation::IAsyncOperation<T>* asyncOperation, ABI::Windows::Foundation::AsyncStatus status) override
	{
		if (status == ABI::Windows::Foundation::AsyncStatus::Completed)
		{
			m_Hr = asyncOperation->GetResults(&m_Result);			
		}
		else
		{
			WRL::ComPtr<ABI::Windows::Foundation::IAsyncInfo> asyncInfo;
			auto hr = asyncOperation->QueryInterface(__uuidof(ABI::Windows::Foundation::IAsyncInfo), &asyncInfo);
			if (SUCCEEDED(hr))
			{
				hr = asyncInfo->get_ErrorCode(&m_Hr);
				ReturnIfFailed(hr);
			}
		}

		SetEvent(m_Event);
		return S_OK;
	}

public:
	SynchronousOperation()
	{
		m_Event = CreateEventExW(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		Assert(m_Event != INVALID_HANDLE_VALUE);
	}

	static inline HRESULT Perform(ABI::Windows::Foundation::IAsyncOperation<T>* asyncOperation, ResultType* result)
	{
		auto operation = WRL::Make<SynchronousOperation<T>>();
		auto hr = asyncOperation->put_Completed(operation.Get());

		auto waitResult = WaitForSingleObjectEx(operation->m_Event, INFINITE, FALSE);
		Assert(waitResult == WAIT_OBJECT_0);
		
		if (SUCCEEDED(operation->m_Hr))
			*result = operation->m_Result;
		
		return operation->m_Hr;
	}
};

template <typename T>
static inline HRESULT PerformSynchronousOperation(ABI::Windows::Foundation::IAsyncOperation<T>* asyncOperation, typename SynchronousOperation<T>::ResultType* result)
{
	return SynchronousOperation<T>::Perform(asyncOperation, result);
}

}