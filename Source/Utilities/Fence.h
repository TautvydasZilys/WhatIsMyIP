#pragma once

#include "Utilities\HandleHolder.h"

namespace Utilities
{

class Fence
{
private:
	uint32_t m_Height;
	HandleHolder m_Event;

public:
	inline Fence(uint32_t height) :
		m_Height(height)
	{
		m_Event = CreateEventExW(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		Assert(m_Event != INVALID_HANDLE_VALUE);
	}

	Fence(const Fence&) = delete;
	Fence& operator=(const Fence&) = delete;

	inline void Advance()
	{
		if (InterlockedDecrement(&m_Height) == 0)
		{
			auto setEventResult = SetEvent(m_Event);
			Assert(setEventResult != FALSE);
		}
	}

	inline void Synchronize()
	{
		auto waitResult = WaitForSingleObjectEx(m_Event, INFINITE, FALSE);
		Assert(waitResult == WAIT_OBJECT_0);
	}
};

}