import utils;
import prof;

#include <shared/defs.h>
#include <shared/net/net.h>
#include <shared/crypto/sha512.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/resource.h>
#include <shared/timer_system/timer_system.h>

#include <game_ms/game_ms.h>

#include <scripting/events.h>

#include <net/handlers/hl_net_defs.h>

#include <game/handlers/hl_defs.h>
#include <game/level.h>
#include <game/player.h>

#include "server.h"

server::server()
{
	peer = SLNet::RakPeerInterface::GetInstance();

	ms = new game_ms();
	ft = new file_transfer();
}

server::~server()
{
	delete ft;
	delete ms;

	if (!peer)
		return;

	peer->Shutdown(100);

	SLNet::RakPeerInterface::DestroyInstance(peer);

	update_users_db();
}

bool server::init_settings()
{
	prof::printt(YELLOW, "Loading server info...");

	auto file = std::ifstream(SETTINGS_FILE());
	if (!file)
		return false;

	json si;

	file >> si;

	if (auto val = si["ip_address"]; !val.is_string())
		return false;
	else info.ip = val;

	if (auto val = si["server_name"]; !val.is_string())
		return false;
	else info.name = val;

	if (auto val = si["password"]; !val.is_string())
		return false;
	else info.pass = val;

	if (auto val = si["gamemode"]; !val.is_string())
		return false;
	else info.gamemode = val;

	if (auto val = si["refresh_rate"]; !val.is_number_integer())
		return false;
	else info.ticks = val;

	auto startup_resources = si["startup_resources"];

	for (const std::string& rsrc : startup_resources)
		info.startup_resources.insert(rsrc);

	prof::printt(CYAN, "IP: '{}'", info.ip.c_str());
	prof::printt(CYAN, "Server name: '{}'", info.name.c_str());
	prof::printt(CYAN, "Gamemode: '{}'", info.gamemode.c_str());
	prof::printt(CYAN, "Refresh rate: {}", info.ticks);

	for (const auto& rsrc : info.startup_resources)
		prof::printt(CYAN, "Startup resource: '{}'", rsrc.c_str());

	return true;
}

bool server::init_game_settings()
{
	auto file = std::ifstream(GAME_SETTINGS_FILE());
	if (!file)
		return false;

	json gi;

	file >> gi;

	if (auto val = gi["playerInfo"]; val.is_boolean())		game_settings.player_info = val;
	if (auto val = gi["friendlyFire"]; val.is_boolean())	game_settings.friendly_fire = val;
	if (auto val = gi["flipMapSync"]; val.is_boolean())		game_settings.flip_map_sync = val;

	prof::printt(YELLOW, "Initializing server game settings...");
	prof::printt(CYAN, "playerInfo: {}", game_settings.player_info);
	prof::printt(CYAN, "friendlyFire: {}", game_settings.friendly_fire);
	prof::printt(CYAN, "flipMapSync: {}", game_settings.flip_map_sync);

	return true;
}

bool server::init_user_database()
{
	prof::printt(YELLOW, "Loading users database...");

	if (users_db_file = std::fstream(USERS_DB, std::ios::in))
	{
		users_db_file >> users_db;
		users_db_file.close();
	}

	return true;
}

bool server::init_masterserver_connection()
{
	return ms->connect();
}

bool server::init()
{
	if (!peer)
		return false;

	if (!init_masterserver_connection())
		return false;

	if (!init_settings())
		return false;

	if (!init_game_settings())
		return false;

	if (!init_user_database())
		return false;

	if (!info.pass.empty())
		peer->SetIncomingPassword(info.pass.c_str(), static_cast<int>(info.pass.length()));

#ifdef _DEBUG
	peer->SetTimeoutTime(60000, SLNet::UNASSIGNED_SYSTEM_ADDRESS);
#else
	peer->SetTimeoutTime(2500, SLNet::UNASSIGNED_SYSTEM_ADDRESS);
#endif

	SLNet::SocketDescriptor socket_desc;

	socket_desc.port = tr::net::GAME_SERVER_PORT_INT;
	socket_desc.socketFamily = AF_INET;

	if (peer->Startup(MAX_PLAYERS(), &socket_desc, 1) != SLNet::RAKNET_STARTED)
		return false;

	peer->SetMaximumIncomingConnections(MAX_PLAYERS());
	peer->SetOccasionalPing(true);
	peer->SetUnreliableTimeout(2500);
	peer->SetSplitMessageProgressInterval(100);

	return (bs_ready = true);
}

bool server::load_resources(bool startup)
{
	if (!std::filesystem::is_directory(resource_system::RESOURCES_PATH))
		return false;

	int loaded = 0,
		failed = 0;

	for (const auto& p : std::filesystem::directory_iterator(resource_system::RESOURCES_PATH))
	{
		const auto& pp = p.path();

		auto path = pp.string() + '\\',
			 folder = pp.filename().string();

		if (startup && !info.startup_resources.contains(folder))
			continue;

		if (!g_resource->load_server_resource(path, folder, startup))
			++failed;
		else ++loaded;
	}

	prof::printt(CYAN, "{} loaded resources, {} failed", loaded, failed);

	return true;
}

bool server::set_user_flags(const std::string& user, uint64_t flags)
{
	auto& arr = users_db[USERS_DB_ARRAY];

	if (!arr.contains(user))
		return false;

	auto& user_obj = arr[user];

	if (user_obj.size() != 2)
		return false;

	user_obj[1] = flags;

	return true;
}

bool server::register_user(const std::string& user, const std::string& pass)
{
	auto& arr = users_db[USERS_DB_ARRAY];

	if (arr.contains(user))
		return false;
	
	arr[user] = { crypto::sha512(pass), NET_PLAYER_FLAG_USER };

	update_users_db();

	return true;
}

bool server::verify_user(const std::string& user, const std::string& pass, uint64_t& flags, bool& invalid_pass)
{
	const auto& arr = users_db[USERS_DB_ARRAY];

	if (!arr.contains(user))
		return false;

	const auto& user_obj = arr[user];

	if (user_obj.size() != 2)
		return false;

	if (std::string(user_obj.at(0)).compare(crypto::sha512(pass)))
		return !(invalid_pass = true);

	flags = user_obj.at(1);

	update_users_db();

	return true;
}

bool server::is_game_setting_enabled(game_setting id)
{
	switch (id)
	{
	case GAME_SETTING_PLAYER_INFO:		return game_settings.player_info;
	case GAME_SETTING_FRIENDLY_FIRE:	return game_settings.friendly_fire;
	case GAME_SETTING_FLIP_MAP_SYNC:	return game_settings.flip_map_sync;
	}

	return false;
}

bool server::verify_game_ownership(net_player* player)
{
	/*const bool verified = ms->verify_game_ownership(player->get_steam_id());

	g_server->send_packet_broadcast_ex(ID_NET_PLAYER_STEAM_ID, player->get_sys_address(), false, verified);

	if (!verified)
		peer->CloseConnection(player->get_sys_address(), true);
	else player->verify_game_purchase();

	return false;*/
	return true;
}

void server::remove_player(PLAYER_ID id)
{
	if (auto it = net_players.find(id); it != net_players.end())
	{
		auto n_player = it->second;

		if (auto player = n_player->get_player())
			g_level->remove_entity(player);

		delete n_player;

		net_players.erase(it);
	}
}

void server::set_game_setting_enabled(game_setting id, bool enabled)
{
	switch (id)
	{
	case GAME_SETTING_PLAYER_INFO:		game_settings.player_info = enabled;	break;
	case GAME_SETTING_FRIENDLY_FIRE:	game_settings.friendly_fire = enabled;	break;
	case GAME_SETTING_FLIP_MAP_SYNC:	game_settings.flip_map_sync = enabled;  break;
	}

	send_packet_broadcast(ID_SERVER_GAME_SETTINGS, game_settings);
}

void server::update_users_db()
{
	users_db_file.open(USERS_DB, std::ios::out, std::ios::trunc);
	users_db_file << std::setw(4) << users_db << std::endl;
	users_db_file.close();
}

int server::get_system_ping(const SLNet::SystemAddress& addr) const
{
	return peer->GetLastPing(addr);
}

void server::remove_player(net_player* player)
{
	remove_player(player->get_id());
}

SLNet::BitStream* server::create_all_resources_sync_status_packet()
{
	auto rsrcs_count = g_resource->get_resources_count();
	if (rsrcs_count <= 0)
		return nullptr;

	auto bs = g_server->create_packet(ID_RESOURCE_SYNC_ALL_STATUS);

	bs->Write(rsrcs_count);

	g_resource->for_each_resource([&](resource* rsrc)
	{
		bs->Write(gns::resource::action { .name = rsrc->get_folder().c_str(), .action = rsrc->get_status() });
	});

	return bs;
}

net_player* server::add_player(SLNet::Packet* p)
{
	auto n_player = new net_player();

	n_player->set_sys_address(p->systemAddress);
	n_player->set_id(p->guid.g);
	n_player->set_id_str(p->guid.ToString());
	n_player->set_name("** Not Connected **");

	auto player = g_level->add_player(n_player);

	n_player->set_player(player);

	auto id = n_player->get_id();

	net_players.insert({ id, n_player });

	gns::net_player::connect info;

	info.id = id;
	info.sid = player->get_sync_id();

	send_packet_broadcast_ex(ID_NET_PLAYER_CONNECT, p->systemAddress, true, info);

	prof::printt(GREEN, "New connection from {}", p->systemAddress.ToString());

	return n_player;
}

net_player* server::get_net_player(PLAYER_ID id)
{
	auto it = net_players.find(id);
	return (it != net_players.end() ? it->second : nullptr);
}

net_player* server::get_net_player(const SLNet::SystemAddress sys_address)
{
	auto it = std::find_if(net_players.begin(), net_players.end(), [&](const auto& p) { return p.second->get_sys_address() == sys_address; });
	return (it != net_players.end() ? it->second : nullptr);
}

void server::on_resource_action(resource* rsrc, resource_action action, script_action_result res, bool done)
{
	if (!done)
		return;

	switch (action)
	{
	case RESOURCE_ACTION_START:
	{
		if (res == SCRIPT_ACTION_OK || res == SCRIPT_ACTION_SCRIPT_ERROR)
		{
			for (const auto& [id, player] : g_server->get_net_players())
				player->sync_all_resources();
		}

		break;
	}
	case RESOURCE_ACTION_STOP:
	{
		if (res == SCRIPT_ACTION_OK)
		{
			if (auto packet = g_server->create_all_resources_sync_status_packet())
				g_server->send_packet_broadcast(packet);
		}
	}
	}
}

void server::script_error_callback(script* s, const std::string& err)
{
	for (const auto& [id, player] : g_server->get_net_players())
		if (player->has_resource_permissions())
			g_server->send_packet_broadcast_ex(ID_NET_PLAYER_SCRIPT_ERROR, player->get_sys_address(), false, gns::net_player::script_error { err.substr(0, err.find('\n')).c_str() });
}

void server::dispatch_packets()
{
	if (tick++ % (get_ticks() * MS_UPDATE_MODIFIER()) == 0)
	{
		std::string players_list;

		std::for_each(net_players.begin(), net_players.end(), [&](const std::pair<PLAYER_ID, net_player*>& p) { players_list += p.second->get_name() + ";"; });

		ms->send_info(info.ip, info.name, info.gamemode, g_level->get_name(), players_list, static_cast<int>(net_players.size()), MAX_PLAYERS());

		send_packet_broadcast(ID_SYNC_REQUEST_STREAM_INFO);
	}

	// trigger onTick event

	g_resource->trigger_event(events::engine::ON_TICK, tick);

	// dispatch files transferring

	ft->dispatch();

	// update the server timers

	g_timer->update();

	// update streamers sync

	g_level->update_entity_streamers();

	// dispatch packets now

	for (auto p = peer->Receive(); p; peer->DeallocatePacket(p), p = peer->Receive())
	{
		auto is_packet_in_range = [&](auto pid, auto min, auto max)
		{
			return (pid > min && pid < max);
		};

		PLAYER_ID id = p->guid.g;

		auto sys_address = p->systemAddress;

		SLNet::BitStream bs_in(p->data, p->length, false);

		switch (const auto packet_id = get_packet_id(p, &bs_in))
		{
		case ID_NEW_INCOMING_CONNECTION:
		{
			auto player = add_player(p);

			gns::level::load out_info {};

			out_info.name = g_level->get_filename().c_str();
			out_info.restart = false;
			
			send_packet_broadcast_ex(ID_SERVER_GAME_SETTINGS, sys_address, false, game_settings);
			send_packet_broadcast_ex(ID_LEVEL_LOAD, sys_address, false, out_info);

#ifdef LEVEL_EDITOR
			g_resource->set_action_callback(nullptr);
			g_resource->set_err_callback_fn(nullptr);

			g_resource->restart_all();

			g_resource->set_action_callback(server::on_resource_action);
			g_resource->set_err_callback_fn(server::script_error_callback);
#endif

			player->sync_all_resources();

			break;
		}
		default:
		{
			if (packet_id == ID_DISCONNECTION_NOTIFICATION || packet_id == ID_CONNECTION_LOST ||
				is_packet_in_range(packet_id, ID_NET_PLAYER_PACKET_BEGIN, ID_NET_PLAYER_PACKET_END))
			{
				net_player_handlers::handle_packet(get_net_player(id), packet_id);
			}
			else if (is_packet_in_range(packet_id, ID_SERVER_PACKET_BEGIN, ID_SERVER_PACKET_END))
				server_handlers::handle_packet(get_net_player(id), packet_id);
			else if (is_packet_in_range(packet_id, ID_LEVEL_PACKET_BEGIN, ID_LEVEL_PACKET_END))
				level_handlers::handle_packet(get_net_player(id), packet_id);
			else if (is_packet_in_range(packet_id, ID_RESOURCE_PACKET_BEGIN, ID_RESOURCE_PACKET_END))
				resource_handlers::handle_packet(get_net_player(id), packet_id);
			else if (is_packet_in_range(packet_id, ID_PLAYER_PACKET_BEGIN, ID_PLAYER_PACKET_END))
				player_handlers::handle_packet(get_net_player(id), packet_id);
			else if (is_packet_in_range(packet_id, ID_PROJECTILE_PACKET_BEGIN, ID_PROJECTILE_PACKET_END))
				projectile_handlers::handle_packet(get_net_player(id), packet_id);
			else if (is_packet_in_range(packet_id, ID_FX_PACKET_BEGIN, ID_FX_PACKET_END))
				fx_handlers::handle_packet(get_net_player(id), packet_id);
			else if (is_packet_in_range(packet_id, ID_AUDIO_PACKET_BEGIN, ID_AUDIO_PACKET_END))
				audio_handlers::handle_packet(get_net_player(id), packet_id);
			else if (is_packet_in_range(packet_id, ID_ITEM_PACKET_BEGIN, ID_ITEM_PACKET_END))
				entity_handlers::handle_packet(get_net_player(id), packet_id);
			else if (is_packet_in_range(packet_id, ID_SYNC_BEGIN, ID_SYNC_END))
				sync_handlers::handle_packet(get_net_player(id), packet_id);
		}
		}

		reset_bs();
	}

	std::this_thread::sleep_for(std::chrono::microseconds(get_update_rate()));
}