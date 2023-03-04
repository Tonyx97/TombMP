import prof;

#include <shared/defs.h>
#include <shared/net/net.h>

#include <ms/ms.h>

#include "clients.h"

using namespace tr;

/*
* CLIENT CLIENT
*/

client_sv::client_sv(ms* sv, SOCKET s, const std::string& ip)
{
	this->sv = sv;
	this->ip = ip;

	si.s = s;

	server_type = false;

	future = std::async(std::launch::async, &client_sv::dispatcher, this);
}

client_sv::~client_sv()
{
}

void client_sv::dispatcher()
{
	prof::print(YELLOW, "------------ (CLIENT IN) {:#x} ------------", si.s);

	net_result last_net_result = NETR_OK;

	bool running = true;

	do {
		net::packet_data pd {};

		if (auto res = net::rcv_packet_test(si, pd, MAX_TIMEOUT_MS()); net::NET_OK(res))
		{
			switch (auto pid = pd.pi.pid)
			{
			case PID_CLIENT_VER:
			{
				version = pd.read<int>();

				if (version != VERSION())
				{
					net::send_packet(si, PID_FAIL, "The client is outdated, please update it");

					running = false;
				}
				else net::send_packet(si, PID_OK, sv->get_game_hash());

				break;
			}
			case PID_SERVERS_LIST:
			{
				if (auto servers_list = sv->get_servers_list(); !servers_list.empty())
					net::send_packet(si, PID_SERVERS_LIST, servers_list);
				else net::send_packet(si, PID_NONE);
				
				break;
			}
			}
		}
		else last_net_result = NETR_FAIL;

		std::this_thread::yield();
	} while (net::NET_OK(last_net_result) && running);

	auto old_socket = si.s;

	net::close_socket(si.s);

	prof::print(YELLOW, "------------ (CLIENT OUT) {:#x} ------------", old_socket);
}

bool client_sv::is_waiting_exit()
{
	return (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
}

bool client_sv::compare_ip(const std::string& other)
{
	return !ip.compare(other);
}

/*
* SERVER CLIENT
*/

server_sv::server_sv(ms* sv, SOCKET s, const std::string& ip)
{
	this->sv = sv;
	this->ip = ip;

#ifndef _DEBUG
	if (!ip.compare("127.0.0.1"))
		public_ip = "51.77.201.205";
	else public_ip = ip;
#else
	public_ip = ip;
#endif

	si.s = s;

	server_type = true;

	future = std::async(std::launch::async, &server_sv::dispatcher, this);
}

server_sv::~server_sv()
{
}

void server_sv::dispatcher()
{
	prof::print(YELLOW, "------------ (SERVER IN) {:#x} {} ------------", si.s, ip);

	net_result last_net_result = NETR_OK;

	bool running = true;
	
	do {
		net::packet_data pd {};

		if (auto res = net::rcv_packet(si, pd, MAX_TIMEOUT_MS()); net::NET_OK(res))
		{
			switch (auto pid = pd.pi.pid)
			{
			case PID_SERVER_VER:
			{
				version = pd.read<int>();
				
				if (version != VERSION())
				{
					net::send_packet(si, PID_FAIL, "The server is outdated, please update it");

					running = false;
				}
				else net::send_packet(si, PID_OK);

				prof::print(CYAN, "Server version: {}", version);
				
				break;
			}
			case PID_SERVER_INFO:
			{
				if (auto data = pd.read_struct<ns_server_info>())
				{
					lock();

					if (std::string new_ip = *data->ip; new_ip.find("none") == -1)
					{
						public_ip = *data->ip;

						prof::print(YELLOW, "------------ PUBLIC IP {} ------------", public_ip);
					}
					else prof::print(YELLOW, "------------ PUBLIC IP {} ------------", new_ip);

					name = *data->name;
					gamemode = *data->gamemode;
					map = *data->map;
					players_list = *data->players;
					players_count = data->players_count;
					max_players = data->max_players;
					sharing_enabled = true;

					unlock();
				}
				else prof::print(RED, "Error receiving server info");

				break;
			}
			case PID_VERIFY_GAME_OWNERSHIP:
			{
				net::send_packet(si, PID_VERIFY_GAME_OWNERSHIP, sv->verify_game_ownership(pd.read<uint64_t>()));

				break;
			}
			}
		}
		else last_net_result = NETR_FAIL;

		std::this_thread::yield();
	} while (net::NET_OK(last_net_result) && running);

	auto old_socket = si.s;

	net::close_socket(si.s);

	prof::print(YELLOW, "------------ (SERVER OUT) {:#x} ------------", old_socket);
}

void server_sv::lock()
{
	mtx.lock();
}

void server_sv::unlock()
{
	mtx.unlock();
}

std::string server_sv::get_all_info()
{
	if (!sharing_enabled)
		return "__EMPTY__";

	std::stringstream info;

	lock();

	info << public_ip << ";" << name << ";" << gamemode << ";" << players_count << ";" << max_players << ";" << map << ";" << players_list;

	unlock();

	return info.str();
}

bool server_sv::is_waiting_exit()
{
	return (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
}

bool server_sv::compare_ip(const std::string& other)
{
	return !ip.compare(other);
}