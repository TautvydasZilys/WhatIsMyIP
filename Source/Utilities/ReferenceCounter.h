#pragma once

namespace Utilities
{

class ReferenceCounter
{
public:
	template <typename T, typename std::enable_if<std::is_base_of<IUnknown, typename std::remove_pointer<T>::type>::value, bool>::type = true>
	inline static void AddRef(T& item)
	{
		if (item != nullptr)
			item->AddRef();
	}

	template <typename T, typename std::enable_if<std::is_base_of<IUnknown, typename std::remove_pointer<T>::type>::value, bool>::type = true>
	inline static void Release(T& item)
	{
		if (item != nullptr)
		{
			item->Release();
			item = nullptr;
		}
	}

	template <typename T, typename std::enable_if<std::is_same<HSTRING, T>::value, bool>::type = true>
	inline static void AddRef(T& item)
	{
		auto hr = WindowsDuplicateString(item, &item);
		Assert(SUCCEEDED(hr));
	}

	template <typename T, typename std::enable_if<std::is_same<HSTRING, T>::value, bool>::type = true>
	inline static void Release(T& item)
	{
		auto hr = WindowsDeleteString(item);
		Assert(SUCCEEDED(hr));

		item = nullptr;
	}

	template <typename T, typename std::enable_if<!std::is_base_of<IUnknown, typename std::remove_pointer<T>::type>::value && !std::is_same<HSTRING, T>::value>::type = true>
	inline static void AddRef(T& item)
	{
	}

	template <typename T, typename std::enable_if<!std::is_base_of<IUnknown, typename std::remove_pointer<T>::type>::value && !std::is_same<HSTRING, T>::value, bool>::type = true>
	inline static void Release(T& item)
	{
	}
};

}