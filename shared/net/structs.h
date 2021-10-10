#pragma once

template <int max_size>
struct ns_string
{
	char str[max_size] = { 0 };

	ns_string(const char* v)				{ set(v); }

	void set(const char* v)					{ strcpy_s(str, v); }
	void set(const std::string& v)			{ strcpy_s(str, v.c_str()); }

	char* operator * ()						{ return str; }

	std::string get_str()					{ return std::string(str); }

	bool valid() const						{ auto len = strlen(str); return (len > 0 && len < max_size); }
	bool empty() const						{ return (len() == 0); }

	size_t len() const						{ return strlen(str); }
};

template <typename T, int max_size>
struct ns_vector
{
	T data[max_size] {};
	int count = 0;

	void add(const T& val)  { if (count < max_size) data[count++] = val; }

	auto begin()			{ return data; }
	auto end()				{ return data + count; }
};

struct ns_server_info
{
	ns_string<4096> players;

	ns_string<128> name;

	ns_string<64> gamemode,
				  map,
				  ip;

	int players_count,
		max_players;
};