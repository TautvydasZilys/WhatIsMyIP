#pragma once

namespace Utilities
{

struct RoInitializer
{
	inline RoInitializer()
	{
		auto hr = RoInitialize(RO_INIT_MULTITHREADED);
		FastFailIfFailed(hr);
	}

	inline ~RoInitializer()
	{
		RoUninitialize();
	}
};

}