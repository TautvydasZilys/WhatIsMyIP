#pragma once

namespace PathHelpers
{

struct PathView
{
	const wchar_t* path;
	const size_t length;

	PathView(const std::wstring& path) :
		path(path.c_str()),
		length(path.length())
	{
	}

	template <size_t length>
	PathView(const wchar_t(&path)[length]) :
		path(path),
		length(length - 1)
	{
	}

	template <typename T, typename = std::enable_if<std::is_pointer<T>::value>::type>
	PathView(T path) :
		path(path),
		length(wcslen(path))
	{
	}
};

inline std::wstring CombinePaths(const PathView left, const PathView right)
{
	std::wstring combinedPath;
	combinedPath.reserve(left.length + right.length + 1);
	combinedPath.append(left.path, left.length);

	if (left.path[left.length - 1] != '\\')
		combinedPath += L'\\';

	combinedPath.append(right.path, right.length);
	return combinedPath;
}

template <typename T>
inline std::wstring CombinePaths(const T& path)
{
	return path;
}

template <typename TLeft, typename TRight, typename... T>
inline std::wstring CombinePaths(const TLeft& left, const TRight& right, const T&... rest)
{
	auto combined = CombinePaths(PathView(left), PathView(right));
	return CombinePaths(combined, rest...);
}

inline bool FileExists(const std::wstring& path)
{
	DWORD fileAttributes = GetFileAttributesW(path.c_str());
	return fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

inline void ChopOffLastPathComponent(std::wstring& path)
{
	size_t index = path.size() - 1;

	while (index != static_cast<size_t>(-1) && path[index] != '\\')
		index--;

	if (index == static_cast<size_t>(-1))
	{
		path.resize(0);
	}
	else
	{
		path.resize(index);
	}
}

inline std::wstring GetDirectoryName(const wchar_t* path)
{
	size_t index = 0;
	size_t lastBackslash = 0;

	while (path[index] != L'\0')
	{
		if (path[index] == '\\')
			lastBackslash = index;

		index++;
	}

	return std::wstring(path, lastBackslash);
}

inline std::wstring GetFileName(const wchar_t* path)
{
	size_t index = 0;
	size_t lastBackslash = -1;

	while (path[index] != L'\0')
	{
		if (path[index] == '\\')
			lastBackslash = index;

		index++;
	}

	return path + lastBackslash + 1;
}

inline bool HasFileExtension(const PathView fileName, const PathView extension)
{
	if (extension.length > fileName.length)
		return false;

	return _wcsicmp(fileName.path + fileName.length - extension.length, extension.path) == 0;
}

}