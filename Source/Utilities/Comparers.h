#pragma once

#include "HString.h"

namespace Utilities
{

struct HStringLess
{
	inline bool operator()(HSTRING left, HSTRING right) const
	{
		int32_t lexicalComparisonResult;
		auto hr = WindowsCompareStringOrdinal(left, right, &lexicalComparisonResult);
		Assert(SUCCEEDED(hr));
		return lexicalComparisonResult < 0;
	}

	inline bool operator()(const Utilities::HString& left, const Utilities::HString& right) const
	{
		return operator()(static_cast<HSTRING>(left), static_cast<HSTRING>(right));
	}
};

}