#pragma once

class argument_parser
{
private:

	std::unordered_map<std::string, std::string> args;

public:

	argument_parser(const std::string& str);

	std::string get_arg(const std::string& arg);

	bool get_registry_value_int(void* data, int size, const std::string& name);

	template <typename T>
	T get_registry_value(const std::string& name)
	{
		T value;

		DWORD size = sizeof(T);

		if (get_registry_value_int(&value, size, name))
			return value;

		return {};
	}
};

inline std::unique_ptr<argument_parser> g_arg_parser;