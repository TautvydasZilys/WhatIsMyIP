// Intentional no pragma once
// #pragma once

#ifndef CALLINGCONVENTION
#error CALLINGCONVENTION must be defined
#endif

namespace TemplateHelpers
{

	template <typename DelegateType, typename ReturnType>
	struct ArgumentTraits<ReturnType(CALLINGCONVENTION DelegateType::*)()>
	{
		static const size_t ArgumentCount = 0;
	};

	template <typename DelegateType, typename ReturnType, typename Arg1Type>
	struct ArgumentTraits<ReturnType(CALLINGCONVENTION DelegateType::*)(Arg1Type)>
	{
		static const size_t ArgumentCount = 1;
		typedef Arg1Type Arg1;
	};

	template <typename DelegateType, typename ReturnType, typename Arg1Type, typename Arg2Type>
	struct ArgumentTraits<ReturnType(CALLINGCONVENTION DelegateType::*)(Arg1Type, Arg2Type)>
	{
		static const size_t ArgumentCount = 2;
		typedef Arg1Type Arg1;
		typedef Arg2Type Arg2;
	};
};
