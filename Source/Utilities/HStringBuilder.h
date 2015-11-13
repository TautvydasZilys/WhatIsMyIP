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

	inline HStringBuilder& operator+=(HSTRING str)
	{
		uint32_t length;
		const wchar_t* characters = WindowsGetStringRawBuffer(str, &length);
		Append(characters, length);
		return *this;
	}

	template <uint32_t length>
	inline HStringBuilder& operator+=(const wchar_t(&str)[length])
	{
		Append(str, length - 1);
		return *this;
	}

	inline HStringBuilder& operator+=(uint32_t number)
	{
		wchar_t buffer[10];
		auto length = swprintf_s(buffer, L"%u", number);
		Append(buffer, static_cast<uint32_t>(length));
		return *this;
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