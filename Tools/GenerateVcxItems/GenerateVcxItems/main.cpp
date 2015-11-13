#include "PrecompiledHeader.h"
#include "PathHelpers.h"
#include "Utf8FileWriter.h"

static std::wstring FindProjectRoot()
{
	const size_t kBufferLength = MAX_PATH + 1;
	wchar_t currentDirectory[kBufferLength];
	
	if (GetCurrentDirectoryW(kBufferLength, currentDirectory) == 0)
		return std::wstring();

	std::wstring searchDirectory(currentDirectory);
	while (!PathHelpers::FileExists(PathHelpers::CombinePaths(searchDirectory, L"projectroot")) && !searchDirectory.empty())
		PathHelpers::ChopOffLastPathComponent(searchDirectory);

	return searchDirectory;
}

static std::vector<std::wstring> EnumerateFiles(const std::wstring& path, const std::initializer_list<wchar_t*>& extensions)
{
	std::vector<std::wstring> files;
	auto searchQuery = PathHelpers::CombinePaths(path, L"*.*");

	WIN32_FIND_DATAW findData;
	auto findHandle = FindFirstFileExW(searchQuery.c_str(), FindExInfoBasic, &findData, FindExSearchNameMatch, nullptr, 0);
	if (findHandle == INVALID_HANDLE_VALUE)
		return files;

	do
	{
		const wchar_t* fileName = findData.cFileName;

		if (fileName[0] == '.' && (fileName[1] == L'\0' || (fileName[1] == L'.' && fileName[2] == L'\0')))
			continue;

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			for (auto& file : EnumerateFiles(PathHelpers::CombinePaths(path, fileName), extensions))
				files.push_back(std::move(file));
		}
		else if (std::any_of(extensions.begin(), extensions.end(), [fileName](const wchar_t* extension) { return PathHelpers::HasFileExtension(fileName, extension); }))
		{
			files.push_back(PathHelpers::CombinePaths(path, fileName));
		}
	}
	while (FindNextFileW(findHandle, &findData) != FALSE);

	FindClose(findHandle);
	return files;
}

static std::wstring GetLastErrorString()
{
	auto lastError = GetLastError();

	std::wstring message;
	message.resize(1024);
	auto length = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, lastError, 0, &message[0], message.length(), nullptr);

	if (length == 0)
	{
		message = L"Unknown error.";
	}
	else
	{
		message.resize(length);
	}

	return message;
}

static std::wstring CreateGuid()
{
	GUID guid;
	if (FAILED(CoCreateGuid(&guid)))
		return std::wstring();

	std::wstring guidStr;
	guidStr.resize(40);

	auto length = static_cast<size_t>(StringFromGUID2(guid, &guidStr[0], 40));
	guidStr.resize(length - 1);
	return guidStr;
}

static std::wstring CreateGuidWithoutBraces()
{
	auto guid = CreateGuid();
	return guid.substr(1, guid.length() - 2);
}

static const char* DetermineMSBuildFileType(const std::wstring& filePath)
{
	if (PathHelpers::HasFileExtension(filePath, L".h"))
		return "ClInclude";

	if (PathHelpers::HasFileExtension(filePath, L".cpp"))
		return "ClCompile";

	return "None";
}

#define WriteFileChecked(...) do \
	{ \
		if (!fileWriter.Write(__VA_ARGS__)) \
		{ \
			auto error = GetLastErrorString(); \
			std::wcout << "Error: failed to write to file: " << error << std::endl; \
			return false; \
		} \
	} \
	while (false)

static bool GenerateVcxItemsFile(const std::wstring& targetPath, const std::vector<std::wstring>& files)
{
	Utf8FileWriter fileWriter(targetPath.c_str());
	
	if (!fileWriter.IsValid())
	{
		auto error = GetLastErrorString();
		std::wcout << "Error: failed to open \"" << targetPath << "\" for writing: " << error << std::endl;
		return false;
	}

	WriteFileChecked(R"*(<?xml version="1.0" encoding="utf-8"?>)*" "\r\n");
	WriteFileChecked(R"*(<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">)*" "\r\n");
	WriteFileChecked(R"*(  <PropertyGroup Label="Globals">)*" "\r\n");
	WriteFileChecked(R"*(    <MSBuildAllProjects>$(MSBuildAllProjects);$(MSBuildThisFileFullPath)</MSBuildAllProjects>)*" "\r\n");
	WriteFileChecked(R"*(    <CodeSharingProject>)*", CreateGuidWithoutBraces(), "</CodeSharingProject>\r\n");
	WriteFileChecked(R"*(    <ItemsProjectGuid>)*", CreateGuid(), "</ItemsProjectGuid>\r\n");
	WriteFileChecked(R"*(    <HasSharedItems>true</HasSharedItems>)*" "\r\n");
	WriteFileChecked(R"*(  </PropertyGroup>)*" "\r\n");
	WriteFileChecked(R"*(  <PropertyGroup Condition="'$(Configuration)'=='Debug'" Label="Configuration">)*" "\r\n");
	WriteFileChecked(R"*(    <ConfigurationType>Application</ConfigurationType>)*" "\r\n");
	WriteFileChecked(R"*(    <UseDebugLibraries>true</UseDebugLibraries>)*" "\r\n");
	WriteFileChecked(R"*(    <PlatformToolset>v120</PlatformToolset>)*" "\r\n");
	WriteFileChecked(R"*(  </PropertyGroup>)*" "\r\n");
	WriteFileChecked(R"*(  <PropertyGroup Condition="'$(Configuration)'=='Release'" Label="Configuration">)*" "\r\n");
	WriteFileChecked(R"*(    <ConfigurationType>Application</ConfigurationType>)*" "\r\n");
	WriteFileChecked(R"*(    <UseDebugLibraries>false</UseDebugLibraries>)*" "\r\n");
	WriteFileChecked(R"*(    <WholeProgramOptimization>true</WholeProgramOptimization>)*" "\r\n");
	WriteFileChecked(R"*(    <PlatformToolset>v120</PlatformToolset>)*" "\r\n");
	WriteFileChecked(R"*(  </PropertyGroup>)*" "\r\n");
	WriteFileChecked(R"*(  <ItemDefinitionGroup>)*" "\r\n");
	WriteFileChecked(R"*(    <ClCompile>)*" "\r\n");
	WriteFileChecked(R"*(      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>)*" "\r\n");
	WriteFileChecked(R"*(      <PrecompiledHeaderFile>PrecompiledHeader.h</PrecompiledHeaderFile>)*" "\r\n");
	WriteFileChecked(R"*(      <AdditionalIncludeDirectories>$(SourceDir);%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>)*" "\r\n");
	WriteFileChecked(R"*(      <CompileAsWinRT>false</CompileAsWinRT>)*" "\r\n");
	WriteFileChecked(R"*(      <TreatWarningAsError>true</TreatWarningAsError>)*" "\r\n");
	WriteFileChecked(R"*(      <CallingConvention>StdCall</CallingConvention>)*" "\r\n");
	WriteFileChecked(R"*(    </ClCompile>)*" "\r\n");
	WriteFileChecked(R"*(    <Link>)*" "\r\n");
	WriteFileChecked(R"*(      <GenerateWindowsMetadata>false</GenerateWindowsMetadata>)*" "\r\n");
	WriteFileChecked(R"*(    </Link>)*" "\r\n");
	WriteFileChecked(R"*(  </ItemDefinitionGroup>)*" "\r\n");

	WriteFileChecked("  <ItemGroup>\r\n");

	for (const auto& file : files)
	{
		auto fileType = DetermineMSBuildFileType(file);

		if (PathHelpers::GetFileName(file.c_str()) == L"PrecompiledHeader.cpp")
		{
			WriteFileChecked("    <", fileType, " Include=\"$(SourceDir)", file, "\">\r\n");
			WriteFileChecked("      <PrecompiledHeader>Create</PrecompiledHeader>\r\n");
			WriteFileChecked("    </", fileType, ">\r\n");
		}
		else
		{
			WriteFileChecked("    <", fileType, " Include=\"$(SourceDir)", file, "\" />\r\n");
		}
	}

	WriteFileChecked("  </ItemGroup>\r\n");

	WriteFileChecked("  <ItemGroup>\r\n");
	WriteFileChecked(R"(    <ProjectCapability Include="SourceItemsFromImports" />)" "\r\n");
	WriteFileChecked("  </ItemGroup>\r\n");
	WriteFileChecked("</Project>");

	return true;
}

typedef std::map<std::wstring, std::vector<std::wstring>> FilterGroups;

static bool WriteVcxItemsFiltersFile(const std::wstring& targetPath, const FilterGroups& filterGroups)
{
	Utf8FileWriter fileWriter(targetPath.c_str());

	if (!fileWriter.IsValid())
	{
		auto error = GetLastErrorString();
		std::wcout << "Error: failed to open \"" << targetPath << "\" for writing: " << error << std::endl;
		return false;
	}

	WriteFileChecked(R"(<?xml version="1.0" encoding="utf-8"?>)" "\r\n");
	WriteFileChecked(R"(<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">)" "\r\n");
	WriteFileChecked("  <ItemGroup>\r\n");

	for (const auto& filterGroup : filterGroups)
	{	
		if (filterGroup.first.empty())
			continue;

		WriteFileChecked("    <Filter Include=\"", filterGroup.first, "\">\r\n");
		WriteFileChecked("      <UniqueIdentifier>", CreateGuid(), "</UniqueIdentifier>\r\n");
		WriteFileChecked("    </Filter>\r\n");
	}

	WriteFileChecked("  </ItemGroup>\r\n");
	WriteFileChecked("  <ItemGroup>\r\n");

	for (const auto& filterGroup : filterGroups)
	{
		for (const auto& file : filterGroup.second)
		{
			auto fileType = DetermineMSBuildFileType(file);

			if (filterGroup.first.empty())
			{
				WriteFileChecked("    <", fileType, " Include=\"$(SourceDir)", file, "\" />\r\n");
			}
			else
			{
				WriteFileChecked("    <", fileType, " Include=\"$(SourceDir)", file, "\">\r\n");
				WriteFileChecked("      <Filter>", filterGroup.first, "</Filter>\r\n");
				WriteFileChecked("    </", fileType, ">\r\n");
			}
		}
	}

	WriteFileChecked("  </ItemGroup>\r\n");
	WriteFileChecked("</Project>");

	return true;
}

static bool GenerateVcxItemsFiltersFile(const std::wstring& targetPath, const std::vector<std::wstring>& files)
{
	FilterGroups filterGroups;

	for (const auto& file : files)
		filterGroups.emplace(PathHelpers::GetDirectoryName(file.c_str()), std::vector<std::wstring>()).first->second.push_back(file);

	WriteVcxItemsFiltersFile(targetPath, filterGroups);
	return true;
}

int wmain(int argc, wchar_t* argv[])
{
	if (argc != 2)
	{
		std::wcout << L"Usage: " << PathHelpers::GetFileName(argv[0]) << L" <vcxitems file name>" << std::endl;
		return -1;
	}

	auto projectRoot = FindProjectRoot();

	if (projectRoot.empty())
	{
		std::wcout << "Error: failed to find project root" << std::endl;
		return -1;
	}

	auto sourceDirectory = PathHelpers::CombinePaths(projectRoot, L"Source");
	auto sourceFiles = EnumerateFiles(sourceDirectory, { L".cpp", L".h" });
	std::transform(sourceFiles.begin(), sourceFiles.end(), sourceFiles.begin(), [&sourceDirectory](const std::wstring& path) { return path.substr(sourceDirectory.length() + 1); });	

	auto vcxItemsPath = PathHelpers::CombinePaths(projectRoot, L"Projects", argv[1]);

	if (!GenerateVcxItemsFile(vcxItemsPath, sourceFiles))
		return -1;

	if (!GenerateVcxItemsFiltersFile(vcxItemsPath + L".filters", sourceFiles))
		return -1;

	return 0;
}