#pragma once

namespace Utilities
{

class HString
{
private:
	HSTRING m_String;

	void Release()
	{
		if (m_String != nullptr)
		{
			auto hr = WindowsDeleteString(m_String);
			Assert(SUCCEEDED(hr));

			m_String = nullptr;
		}
	}

public:
	inline HString() :
		m_String(nullptr)
	{
	}

	inline HString(std::nullptr_t) :
		m_String(nullptr)
	{
	}

	inline HString(const wchar_t* str)
	{
		auto hr = WindowsCreateString(str, static_cast<uint32_t>(wcslen(str)), &m_String);
		Assert(SUCCEEDED(hr));
	}

	inline HString(const wchar_t* str, uint32_t length)
	{
		auto hr = WindowsCreateString(str, length, &m_String);
		Assert(SUCCEEDED(hr));
	}

	template <uint32_t length>
	inline HString(const wchar_t (&str)[length])
	{
		auto hr = WindowsCreateString(str, length - 1, &m_String);
		Assert(SUCCEEDED(hr));
	}

	inline HString(HSTRING str)
	{
		auto hr = WindowsDuplicateString(str, &m_String);
		Assert(SUCCEEDED(hr));
	}

	inline HString(const HString& other)
	{
		auto hr = WindowsDuplicateString(other.m_String, &m_String);
		Assert(SUCCEEDED(hr));
	}

	inline HString(HString&& other) :
		m_String(other.Detach())
	{
	}

	inline ~HString()
	{
		Release();
	}

	inline HString& operator=(const HString& other)
	{
		Release();
		
		auto hr = WindowsDuplicateString(other.m_String, &m_String);
		Assert(SUCCEEDED(hr));

		return *this;
	}

	inline HString& operator=(HString&& other)
	{
		Release();
		m_String = other.Detach();
		return *this;
	}

	inline operator HSTRING() const
	{
		return m_String;
	}

	inline HSTRING* operator&()
	{
		Release();
		return &m_String;
	}

	inline void Attach(HSTRING str)
	{
		Release();
		m_String = str;
	}

	inline HSTRING Detach()
	{
		auto str = m_String;
		m_String = nullptr;
		return str;
	}

	inline const wchar_t* GetRawBuffer(uint32_t* length) const
	{
		return WindowsGetStringRawBuffer(m_String, length);
	}

	inline const wchar_t* GetRawBuffer() const
	{
		uint32_t length;
		return GetRawBuffer(&length);
	}
};

class HStringReference
{
private:
	HSTRING m_String;
	HSTRING_HEADER m_Header;

public:
	inline HStringReference(const wchar_t* str)
	{
		auto hr = WindowsCreateStringReference(str, static_cast<uint32_t>(wcslen(str)), &m_Header, &m_String);
		Assert(SUCCEEDED(hr));
	}

	inline HStringReference(const wchar_t* str, uint32_t length)
	{
		auto hr = WindowsCreateStringReference(str, length, &m_Header, &m_String);
		Assert(SUCCEEDED(hr));
	}

	template <uint32_t length>
	inline HStringReference(const wchar_t(&str)[length])
	{
		auto hr = WindowsCreateStringReference(str, length - 1, &m_Header, &m_String);
		Assert(SUCCEEDED(hr));
	}

	inline operator HSTRING() const
	{
		return m_String;
	}
};

}