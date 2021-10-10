#pragma once

#include <shared/thread_safe/vector.h>

class client_base;

class ms
{
private:

	thread_safe::vector<client_base*> clients;

	std::string game_hash;

	std::mutex game_hash_mtx;

public:

	ms();
	~ms();

	bool init();
	bool run();
	bool create_client(const std::string& ip, UINT_PTR s);
	bool verify_game_ownership(uint64_t steam_id);

	std::string get_game_hash();
	std::string get_servers_list();

	std::tuple<int, int> check_and_clean_connections();
};