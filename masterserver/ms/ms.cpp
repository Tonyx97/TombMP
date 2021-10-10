import utils;
import prof;

#include <shared/defs.h>
#include <shared/net/net.h>
#include <shared/crypto/sha512.h>

#include <curl/curl.h>

#include <json.hpp>

#include <clients/clients.h>

#include "ms.h"

using json = nlohmann::json;
using namespace tr;

size_t apis_write_callback(
	const char* in,
	size_t size,
	size_t num,
	std::string* out)
{
	const size_t bytes(size * num);
	out->append(in, bytes);
	return bytes;
}

ms::ms() {}

ms::~ms()
{
	net::server::destroy();
}

bool ms::init()
{
	if (!net::server::connect(net::MASTER_SERVER_PORT))
		return false;

#if !defined(_DEBUG) && !defined(_LD)
	if (auto game_file = utils::file::read_file("TombMP.exe"); !game_file.empty())
	{
		game_hash = crypto::sha512(game_file);

		prof::print(CYAN, "Ready (game_hash: '{}')", game_hash.c_str());
	}
	else
	{
		prof::print(RED, "Game file doesn't exist");
		return false;
	}
#endif

	return true;
}

bool ms::run()
{
	std::atomic_bool running = true;

	auto conn_checker = std::thread([&]()
	{
		int refreshes = 0,
			cleaned = 0,
			active = 0;

		while (true)
		{
			refreshes++;
			std::tie(cleaned, active) = check_and_clean_connections();
			SetConsoleTitle(std::format("TombMP (refreshes {} | cleans {} | conns {})", refreshes, cleaned, active).c_str());
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	});

	while (running)
	{
		if (auto [socket, ip] = net::server::accept_conn(); socket != INVALID_SOCKET)
		{
#ifndef _DEBUG
			/*if (clients.contains_if([&](client_base* cl) { return cl->compare_ip(ip); }))
				net::close_socket({ socket }, false);
			else*/
#endif
				std::async(std::launch::async, &ms::create_client, this, ip, socket);
		}
	}

	conn_checker.join();

	running = false;

	return true;
}

bool ms::create_client(const std::string& ip, UINT_PTR s)
{
	net::packet_data pd {};

	if (!net::NET_OK(net::rcv_packet({ s }, pd, 5000)))
		return false;
	
	if (auto type = pd.read<int>(); type == 1)
		clients.push(new server_sv(this, s, ip));
	else if (type == 2)
		clients.push(new client_sv(this, s, ip));

	return true;
}

bool ms::verify_game_ownership(uint64_t steam_id)
{
	bool verified = false;

	if (auto api = curl_easy_init())
	{
		std::string request;

		auto query = std::format("https://api.steampowered.com/IPlayerService/GetOwnedGames/v0001/?key=04B1B33BD8467924D74081CB8DE0947D&steamid={}&format=json", steam_id);

		curl_easy_setopt(api, CURLOPT_URL, query.c_str());
		curl_easy_setopt(api, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(api, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(api, CURLOPT_WRITEFUNCTION, apis_write_callback);
		curl_easy_setopt(api, CURLOPT_WRITEDATA, &request);

		if (curl_easy_perform(api) == CURLE_OK)
		{
			json j;

			std::stringstream(request) >> j;

			for (auto& response : j)
				for (auto& game : response["games"])
					if (int appid = game["appid"];  appid == 225320)
					{
						verified = true;
						break;
					}
		}
		else std::cout << "Error in curl_easy_perform" << std::endl;

		curl_easy_cleanup(api);
	}

	if (verified)
		std::cout << "Steam ID " << steam_id << " successfully verified" << std::endl;
	else std::cout << "Steam ID " << steam_id << " could not be verified" << std::endl;

	return verified;
}

std::string ms::get_game_hash()
{
	std::lock_guard lock(game_hash_mtx);
	return game_hash;
}

std::string ms::get_servers_list()
{
	std::string list;

	clients.lock();

	for (const auto& c : clients)
		if (c->is_server())
			list += static_cast<server_sv*>(c)->get_all_info() + ":";

	clients.unlock();

	return list;
}

std::tuple<int, int> ms::check_and_clean_connections()
{
	int cleaned_conns = 0,
		active_conns = 0;

	clients.lock();

	auto it = clients.begin();
	while (it != clients.end())
	{
		if ((*it)->is_waiting_exit())
		{
			delete *it;
			it = clients.erase(it);
			++cleaned_conns;
		}
		else
		{
			++it;
			++active_conns;
		}
	}

	clients.unlock();

	return { cleaned_conns, active_conns };
}