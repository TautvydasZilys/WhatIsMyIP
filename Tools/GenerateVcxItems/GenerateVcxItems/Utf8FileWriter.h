#pragma once

template <typename Derived>
class WriterBase
{
private:
	friend class WriteHelper;

	template <typename T>
	struct WriteHelper;

	inline bool WriteWideString(const wchar_t* wstr, size_t wstrLength)
	{
		std::vector<char> utf8Str;
		utf8Str.resize(4 * wstrLength);

		auto nameLength = static_cast<size_t>(WideCharToMultiByte(CP_UTF8, 0, wstr, wstrLength, &utf8Str[0], utf8Str.size(), nullptr, nullptr));

		if (nameLength == 0)
			return false;

		utf8Str.resize(nameLength);
		return WriteImpl(utf8Str);
	}

	template <>
	struct WriteHelper<std::vector<char>>
	{
		static inline bool Write(Derived* writer, const std::vector<char>& data)
		{
			return writer->WriteBytes(data.data(), data.size());
		}
	};

	template <size_t length>
	struct WriteHelper<const char[length]>
	{
		static inline bool Write(Derived* writer, const char (&str)[length])
		{
			return writer->WriteBytes(str, length - 1);
		}
	};

	template <size_t length>
	struct WriteHelper<const uint8_t[length]>
	{
		static inline bool Write(Derived* writer, const uint8_t (&str)[length])
		{
			return writer->WriteBytes(str, length - 1);
		}
	};

	template <>
	struct WriteHelper<const char*>
	{
		static inline bool Write(Derived* writer, const char* str)
		{
			return writer->WriteBytes(str, strlen(str));
		}
	};

	template <>
	struct WriteHelper<std::wstring>
	{
		static inline bool Write(Derived* writer, const std::wstring& str)
		{
			return writer->WriteWideString(str.c_str(), str.length());
		}
	};

	template <size_t length>
	struct WriteHelper<const wchar_t[length]>
	{
		static inline bool Write(Derived* writer, const wchar_t (&str)[length])
		{
			return writer->WriteWideString(str, length - 1);
		}
	};

	template <>
	struct WriteHelper<const wchar_t*>
	{
		static inline bool Write(Derived* writer, const wchar_t* str)
		{
			return writer->WriteWideString(str, wcslen(str));
		}
	};

	template <typename T>
	bool WriteImpl(const T& data)
	{
		return WriteHelper<T>::Write(static_cast<Derived*>(this), data);
	}

public:
	inline bool Write()
	{
		return true;
	}

	template <typename Arg1, typename... Args>
	inline bool Write(const Arg1& arg, const Args&... data)
	{
		return WriteImpl(arg) && Write(data...);
	}
};

class Utf8FileWriter :
	public WriterBase<Utf8FileWriter>
{
private:
	HANDLE m_FileHandle;
	bool m_IsValid;

	Utf8FileWriter(const Utf8FileWriter&) = delete;
	Utf8FileWriter& operator=(const Utf8FileWriter&) = delete;

	inline void Release()
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_FileHandle);
			m_FileHandle = INVALID_HANDLE_VALUE;
		}
	}

public:
	inline bool WriteBytes(const void* data, DWORD size)
	{
		DWORD bytesWritten;
		auto result = WriteFile(m_FileHandle, data, size, &bytesWritten, nullptr);
		return result != FALSE && size == bytesWritten;
	}

	inline bool IsValid() const
	{
		return m_IsValid;
	}

	inline Utf8FileWriter(const wchar_t* path)
	{
		m_FileHandle = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			const uint8_t utf8bom[] = { 0xEF, 0xBB, 0xBF, 0x00 };
			m_IsValid = Write(utf8bom);
		}
		else
		{
			m_IsValid = false;
		}
	}

	inline ~Utf8FileWriter()
	{
		Release();
	}
};
