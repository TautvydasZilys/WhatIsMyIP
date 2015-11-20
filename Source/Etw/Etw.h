#pragma once

namespace Etw
{

#include "EtwProviderGenerated.h"

struct EtwScopedEvent
{
private:
	const char* m_Category;
	const char* m_Name;

public:
	inline EtwScopedEvent(const char* category, const char* name) :
		m_Category(category),
		m_Name(name)
	{
		EventWriteScopedEvent(0, m_Category, m_Name);
	}

	inline ~EtwScopedEvent()
	{
		EventWriteScopedEvent(1, m_Category, m_Name);
	}
};

struct EtwRefCountedScopedEvent
{
private:
	std::shared_ptr<EtwScopedEvent> m_Event;

public:
	inline EtwRefCountedScopedEvent(const char* category, const char* name) :
		m_Event(std::make_shared<EtwScopedEvent>(category, name))
	{
	}
};

inline void EtwSingleEvent(const char* category, const char* name)
{
	EventWriteSingleEvent(category, name);
}

}