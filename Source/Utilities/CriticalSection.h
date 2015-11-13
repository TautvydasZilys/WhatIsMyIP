#pragma once

namespace Utilities
{

class CriticalSection
{
	CRITICAL_SECTION m_CriticalSection;

public:
	inline CriticalSection()
	{
		InitializeCriticalSectionEx(&m_CriticalSection, 4000, 0);
	}

	CriticalSection(const CriticalSection&) = delete;
	CriticalSection& operator=(const CriticalSection&) = delete;

	inline ~CriticalSection()
	{
		DeleteCriticalSection(&m_CriticalSection);
	}

	inline void Enter()
	{
		EnterCriticalSection(&m_CriticalSection);
	}

	inline void Leave()
	{
		LeaveCriticalSection(&m_CriticalSection);
	}

	class Lock
	{
	private:
		CriticalSection& m_CriticalSection;

	public:
		inline Lock(CriticalSection& criticalSection) :
			m_CriticalSection(criticalSection)
		{
			m_CriticalSection.Enter();
		}

		Lock(const Lock&) = delete;
		Lock& operator=(const Lock&) = delete;

		inline ~Lock()
		{
			m_CriticalSection.Leave();
		}
	};
};

}