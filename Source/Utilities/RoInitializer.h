#pragma once

namespace Utilities
{

struct RoInitializer
{
	inline RoInitializer()
	{
		auto hr = RoInitialize(RO_INIT_MULTITHREADED);
		Assert(SUCCEEDED(hr));
	}

	inline ~RoInitializer()
	{
		RoUninitialize();
	}
};

}