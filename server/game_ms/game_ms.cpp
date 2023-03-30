import prof;

#include <shared/defs.h>
#include <shared/net/net.h>

#include "game_ms.h"

using namespace tr;

game_ms::game_ms()
{
	net::client::max_timeout = 1000;
}

game_ms::~game_ms()
{
	net::client::destroy();
}

bool game_ms::connect()
{
	prof::printt(YELLOW, "Connecting to MasterServer...");

#ifdef _DEBUG
	if (!net::client::connect("127.0.0.1", net::MASTER_SERVER_PORT))
#else
	if (!net::client::connect("65.20.98.86", net::MASTER_SERVER_PORT))
#endif
	{
		prof::printt(RED, "Can't establish connection with the MasterServer");
		return false;
	}

	prof::printt(GREEN, "MasterServer connection OK");
	
	if (!net::NET_OK(net::client::send_packet(PID_MS_CLIENT_TYPE, TYPE())))
		return false;

	if (!net::NET_OK(net::client::send_packet(PID_SERVER_VER, VERSION())))
		return false;

	prof::printt(YELLOW, "Checking version...");

	if (net::packet_data pd {}; net::NET_OK(net::client::rcv_packet(pd)))
	{
		if (pd.pi.pid != PID_OK)
			prof::printt(RED, "{}", pd.as_ptr<char>());
		else prof::printt(GREEN, "Version OK");
	}
	else return false;

	return true;
}

bool game_ms::send_info(const std::string& ip, const std::string& name, const std::string& gamemode, const std::string& map, const std::string& players, int players_count, int max_players)
{
	ns_server_info data
	{
		.players = "No Players",
		.name = name.c_str(),
		.gamemode = gamemode.c_str(),
		.map = map.c_str(),
		.ip = "none",
		.players_count = players_count,
		.max_players = max_players
	};

	if (!players.empty())
		data.players = players.c_str();

	if (!ip.empty())
		data.ip = ip.c_str();

	return (net::NET_OK(net::client::send_packet(PID_SERVER_INFO, data)) ? true : connect());
}

bool game_ms::verify_game_ownership(uint64_t steam_id)
{
	if (!net::NET_OK(net::client::send_packet(PID_VERIFY_GAME_OWNERSHIP, steam_id)))
		return false;

	net::packet_data pd {};

	if (!net::NET_OK(net::client::rcv_packet(pd)))
		return false;
	
	return pd.read<bool>();
}