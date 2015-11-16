#pragma once

namespace Utilities
{

class HandleHolder
{
private:
	HANDLE m_Handle;

public:
	inline HandleHolder() :
		m_Handle(INVALID_HANDLE_VALUE)
	{
	}

	inline HandleHolder(HANDLE handle) :
		m_Handle(handle)
	{
	}

	HandleHolder(const HandleHolder&) = delete;
	HandleHolder& operator=(const HandleHolder&) = delete;

	inline HandleHolder(HandleHolder&& other) :
		m_Handle(other.m_Handle)
	{
		other.m_Handle = INVALID_HANDLE_VALUE;
	}

	inline HandleHolder& operator=(HandleHolder&& other)
	{
		std::swap(m_Handle, other.m_Handle);
		return *this;
	}

	inline ~HandleHolder()
	{
		if (m_Handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_Handle);
			m_Handle = INVALID_HANDLE_VALUE;
		}
	}

	inline operator HANDLE() const
	{
		return m_Handle;
	}
};

}