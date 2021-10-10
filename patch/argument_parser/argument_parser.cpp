import prof;

#include <shared/defs.h>

#include "argument_parser.h"

argument_parser::argument_parser(const std::string& str)
{
	prof::print(GREEN, "'{}'", str.c_str());

	std::regex arguments_rgx("\\-(\\w+) ((\\w|\\.)+) ");
	std::string arguments = str;
	std::smatch sm;

	while (std::regex_search(arguments, sm, arguments_rgx))
	{
		prof::print(BLUE, "'{}' '{}'", sm[1].str().c_str(), sm[2].str().c_str());

		args.insert({ sm[1].str(), sm[2].str() });

		arguments = sm.suffix().str();
	}
}

std::string argument_parser::get_arg(const std::string& arg)
{
	auto it = args.find(arg);
	return (it != args.end() ? it->second : "");
}

bool argument_parser::get_registry_value_int(void* data, int size, const std::string& name)
{
	HKEY key = nullptr;

	if (RegCreateKey(HKEY_CURRENT_USER, "SOFTWARE\\TombMP\\game", &key) != ERROR_SUCCESS)
		return false;

	const bool ok = (RegQueryValueExA(key, name.c_str(), nullptr, nullptr, (BYTE*)data, (DWORD*)&size) == ERROR_SUCCESS);

	RegCloseKey(key);

	return ok;
}