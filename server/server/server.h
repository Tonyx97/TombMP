#pragma once

import utils;

#include <shared/game_net/messages.h>
#include <shared/game_net/bitstream.h>
#include <shared/game_net/structs.h>
#include <shared/game_net/file_Transfer.h>

#include <net/net_player.h>

#include <json.hpp>

using nlohmann::json;

class game_ms;
class resource;
class game_level;

enum resource_action;
enum script_action_result;

enum game_setting
{
	GAME_SETTING_PLAYER_INFO = utils::hash::JENKINS("playerInfo"),
	GAME_SETTING_FRIENDLY_FIRE = utils::hash::JENKINS("friendlyFire"),
	GAME_SETTING_FLIP_MAP_SYNC = utils::hash::JENKINS("flipMapSync"),
};

class server : public bit_streaming
{
private:

	struct
	{
		std::unordered_set<std::string> startup_resources;

		std::string ip,
					name,
					gamemode,
					pass;
		
		int ticks;
	} info;

	gns::server::game_settings game_settings {};

	json users_db;

	std::unordered_set<std::string> logged_in_users;

	std::fstream users_db_file;

	file_transfer* ft = nullptr;

	std::unordered_map<PLAYER_ID, net_player*> net_players;

	game_ms* ms = nullptr;

	uint64_t tick = 0;

	bool initialized = false;

public:

	static constexpr auto USERS_DB = "users_db.json";
	static constexpr auto USERS_DB_ARRAY = "users";

	server();
	~server();

	bool init_settings();
	bool init_game_settings();
	bool init_user_database();
	bool init_masterserver_connection();
	bool init();
	bool load_resources(bool startup = false);
	bool set_user_flags(const std::string& user, uint64_t flags);
	bool register_user(const std::string& user, const std::string& pass);
	bool verify_user(const std::string& user, const std::string& pass, uint64_t& flags, bool& invalid_pass);
	bool is_game_setting_enabled(game_setting id);
	bool verify_game_ownership(net_player* player);
	bool is_user_logged_in(const std::string& user) const		{ return logged_in_users.contains(user); }

	void set_user_logged_in(const std::string& user)			{ logged_in_users.insert(user); }
	void set_user_logged_out(const std::string& user)			{ logged_in_users.erase(user); }
	void set_game_setting_enabled(game_setting id, bool enabled);
	void update_users_db();
	void remove_player(PLAYER_ID id);
	void remove_player(net_player* player);
	void dispatch_packets();

	bool is_initialized() const									{ return initialized; }

	int get_system_ping(const SLNet::SystemAddress& addr) const;
	int get_ticks() const										{ return info.ticks; }
	int get_update_rate() const									{ return int((1000.f / float(get_ticks())) * 1000.f); }

	SLNet::BitStream* create_all_resources_sync_status_packet();

	net_player* add_player(SLNet::Packet* p);
	net_player* get_net_player(PLAYER_ID id);
	net_player* get_net_player(const SLNet::SystemAddress sys_address);

	file_transfer* get_ft() const								{ return ft; }

	decltype(game_settings)& get_game_settings()				{ return game_settings; }

	const decltype(net_players)& get_net_players() const		{ return net_players; }

	static void on_resource_action(resource* rsrc, resource_action action, script_action_result res, bool done);
	static void script_error_callback(class script* s, const std::string& err);
	
	static constexpr auto SETTINGS_FILE()						{ return "settings.json"; }
	static constexpr auto GAME_SETTINGS_FILE()					{ return "game_settings.json"; }
	static constexpr int MS_UPDATE_MODIFIER()					{ return 1; }
	static constexpr int MAX_PLAYERS()							{ return GNS_GLOBALS::MAX_PLAYERS; }
};

inline std::unique_ptr<server> g_server;