#pragma once

namespace Utilities
{

class HStringBuilder
{
private:
	HSTRING_BUFFER m_Buffer;
	wchar_t* m_Characters;
	uint32_t m_BufferSize;
	uint32_t m_Length;

	template <typename T>
	struct AppendHelper
	{
		template <typename = typename std::enable_if<std::is_integral<T>::value>::type>
		inline static HStringBuilder& Append(HStringBuilder* builder, T number)
		{
			wchar_t buffer[20];
			auto length = swprintf_s(buffer, L"%u", number);
			builder->Append(buffer, static_cast<uint32_t>(length));
			return *builder;
		}
	};

	template<>
	struct AppendHelper<HSTRING>
	{
		inline static HStringBuilder& Append(HStringBuilder* builder, HSTRING str)
		{
			uint32_t length;
			const wchar_t* characters = WindowsGetStringRawBuffer(str, &length);
			builder->Append(characters, length);
			return *builder;
		}
	};

	template <uint32_t length>
	struct AppendHelper<const wchar_t[length]>
	{
		inline static HStringBuilder& Append(HStringBuilder* builder, const wchar_t(&str)[length])
		{
			builder->Append(str, length - 1);
			return *builder;
		}
	};

	template <>
	struct AppendHelper<const wchar_t*>
	{
		inline static HStringBuilder& Append(HStringBuilder* builder, const wchar_t* str)
		{
			builder->Append(str, wcslen(str));
			return *builder;
		}
	};

public:
	inline HStringBuilder() :
		m_Buffer(nullptr),
		m_Characters(nullptr),
		m_BufferSize(0),
		m_Length(0)
	{
	}

	inline HStringBuilder(uint32_t initialReservationSize) :
		m_Buffer(nullptr),
		m_Characters(nullptr),
		m_BufferSize(0),
		m_Length(0)
	{
		Reserve(initialReservationSize);
	}

	inline void Reserve(uint32_t size)
	{
		if (m_BufferSize >= size)
			return;

		HRESULT hr;
		auto oldBuffer = m_Buffer;
		auto oldCharacters = m_Characters;

		hr = WindowsPreallocateStringBuffer(size, &m_Characters, &m_Buffer);
		Assert(SUCCEEDED(hr));

		if (m_Buffer != nullptr)
		{
			memcpy(m_Characters, oldCharacters, m_Length * sizeof(wchar_t));
			hr = WindowsDeleteStringBuffer(oldBuffer);
			Assert(SUCCEEDED(hr));
		}

		m_BufferSize = size;
	}

	inline void Append(const wchar_t* str, uint32_t length)
	{
		Reserve(m_Length + length);
		memcpy(m_Characters + m_Length, str, length * sizeof(wchar_t));
		m_Length += length;
	}

	template <typename T>
	inline HStringBuilder& operator+=(const T& str)
	{
		return AppendHelper<T>::Append(this, str);
	}

	HRESULT Promote(HSTRING* str)
	{
		if (m_Buffer == nullptr)
			return WindowsCreateString(nullptr, 0, str);

		auto hr = WindowsPromoteStringBuffer(m_Buffer, str);
		m_Buffer = nullptr;
		m_Characters = nullptr;
		m_BufferSize = 0;
		m_Length = 0;
		return hr;
	}
};

}