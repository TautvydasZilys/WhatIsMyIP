#pragma once

#include "HandleHolder.h"
#include "ThreadPoolRunner.h"

namespace Utilities
{

#define AsyncCreationSingletonImpl(T) \
	std::atomic<Utilities::AsyncCreationSingleton<T>::CreationState> Utilities::AsyncCreationSingleton<T>::s_CreationState = Utilities::AsyncCreationSingleton<T>::CreationState::kNotCreated; \
	T* Utilities::AsyncCreationSingleton<T>::s_Instance = nullptr; \
	Utilities::HandleHolder Utilities::AsyncCreationSingleton<T>::s_CreatedEvent

template <typename T>
class AsyncCreationSingleton
{
private:
	enum class CreationState
	{
		kNotCreated = 0,
		kCreating,
		kCreated,
		kDestroyed
	};

	static std::atomic<CreationState> s_CreationState;
	static T* s_Instance;
	static HandleHolder s_CreatedEvent;

	static void CreateAsync()
	{
		Assert(s_CreationState == CreationState::kNotCreated);
		s_CreatedEvent = CreateEventExW(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

		auto hr = ThreadPoolRunner::RunAsync([]
		{
			CreationState notCreated = CreationState::kNotCreated;
			if (s_CreationState.compare_exchange_strong(notCreated, CreationState::kCreating, std::memory_order_seq_cst))
			{
				auto hr = T::Create(&s_Instance);
				FastFailIfFailed(hr);

				s_CreationState = CreationState::kCreated;
				
				auto setEventResult = SetEvent(s_CreatedEvent);
				Assert(setEventResult != FALSE);
			}
		});
		FastFailIfFailed(hr);
	}

	static void Destroy()
	{
		Assert(s_CreationState != CreationState::kDestroyed);

		for (;;)
		{
			CreationState notCreated = CreationState::kNotCreated;
			if (s_CreationState.compare_exchange_strong(notCreated, CreationState::kDestroyed, std::memory_order_seq_cst))
				break;

			CreationState created = CreationState::kCreated;
			if (s_CreationState.compare_exchange_strong(created, CreationState::kDestroyed, std::memory_order_seq_cst))
			{
				s_Instance->Destroy();
				s_Instance = nullptr;
				break;
			}
		}
	}

protected:
	static T* GetInstance()
	{
		if (s_CreationState != CreationState::kCreated)
		{
			auto waitResult = WaitForSingleObjectEx(s_CreatedEvent, INFINITE, FALSE);
			Assert(waitResult == WAIT_OBJECT_0);
		}

		return s_Instance;
	}

public:
	struct ScopedSingleton
	{
	public:
		ScopedSingleton()
		{
			AsyncCreationSingleton<T>::CreateAsync();
		}

		~ScopedSingleton()
		{
			AsyncCreationSingleton<T>::Destroy();
		}
	};
};

}