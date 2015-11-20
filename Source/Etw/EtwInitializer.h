#pragma once

#include "Etw.h"

namespace Etw
{

struct EtwInitializer
{
	inline EtwInitializer()
	{
		EventRegisterWhatIsMyIP();
	}

	inline ~EtwInitializer()
	{
		EventUnregisterWhatIsMyIP();
	}
};

}