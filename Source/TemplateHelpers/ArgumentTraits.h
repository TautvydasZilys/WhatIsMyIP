#pragma once

namespace TemplateHelpers
{
	template <typename MethodType>
	struct ArgumentTraits;
}

#ifndef _M_IX86

#define CALLINGCONVENTION
#include "ArgumentTraitsImpl.h"

#else

#define CALLINGCONVENTION __cdecl
#include "ArgumentTraitsImpl.h"
#undef CALLINGCONVENTION

#define CALLINGCONVENTION __stdcall
#include "ArgumentTraitsImpl.h"
#undef CALLINGCONVENTION

#define CALLINGCONVENTION __fastcall
#include "ArgumentTraitsImpl.h"
#undef CALLINGCONVENTION

#endif