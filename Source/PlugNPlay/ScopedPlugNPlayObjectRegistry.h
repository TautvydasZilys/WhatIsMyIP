#pragma once
#include "PlugNPlayObjectRegistry.h"

namespace PlugNPlay
{

struct ScopedPlugNPlayObjectRegistry
{
	inline ScopedPlugNPlayObjectRegistry()
	{
		auto hr = PlugNPlayObjectRegistry::Create();
		FastFailIfFailed(hr);
	}

	inline ~ScopedPlugNPlayObjectRegistry()
	{
		PlugNPlayObjectRegistry::Destroy();
	}
};

}