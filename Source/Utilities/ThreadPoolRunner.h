#pragma once

#include "EventHandler.h"
#include "HString.h"

namespace Utilities
{

class ThreadPoolRunner
{
private:
	static WRL::ComPtr<ABI::Windows::System::Threading::IThreadPoolStatics> s_ThreadPool;

	static void Create()
	{
		auto hr = Windows::Foundation::GetActivationFactory(Utilities::HStringReference(L"Windows.System.Threading.ThreadPool"), &s_ThreadPool);
		
		if (FAILED(hr))
			__fastfail(hr);
	}

	static void Destroy()
	{
		s_ThreadPool = nullptr;
	}

public:
	template <typename Function>
	static HRESULT RunAsync(Function&& function)
	{
		Assert(s_ThreadPool != nullptr);

		WRL::ComPtr<ABI::Windows::Foundation::IAsyncAction> runAction;
		return s_ThreadPool->RunAsync(EventHandlerFactory<ABI::Windows::System::Threading::IWorkItemHandler>::Make(
			[function](ABI::Windows::Foundation::IAsyncAction*) -> HRESULT
			{
				function();
				return S_OK;
			}).Get(), &runAction);
	}

	struct ScopedSingleton
	{
	public:
		ScopedSingleton()
		{
			ThreadPoolRunner::Create();
		}

		~ScopedSingleton()
		{
			ThreadPoolRunner::Destroy();
		}
	};
};

}