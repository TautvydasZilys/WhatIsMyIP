#pragma once

namespace Networking
{

struct ConnectionProperties
{
	std::wstring name;
	std::map<const wchar_t*, std::wstring> properties;

	inline ConnectionProperties()
	{
	}

	ConnectionProperties(ConnectionProperties&) = delete;
	ConnectionProperties& operator=(ConnectionProperties&) = delete;

	inline ConnectionProperties(ConnectionProperties&& other) :
		name(std::move(other.name)),
		properties(std::move(other.properties))
	{		
	}

	ConnectionProperties& operator=(ConnectionProperties&& other)
	{
		name = std::move(other.name);
		properties = std::move(other.properties);
	}	
};

}