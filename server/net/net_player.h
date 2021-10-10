#pragma once

#include <shared/game_net/messages.h>
#include <shared/game_net/bitstream.h>
#include <shared/game_net/structs.h>

class server;
class resource;
class game_player;

enum net_player_flags : unsigned __int64
{
	NET_PLAYER_FLAG_NONE		= 0,
	NET_PLAYER_FLAG_OWNER		= (1 << 1),
	NET_PLAYER_FLAG_ADMIN		= (1 << 2),
	NET_PLAYER_FLAG_MOD			= (1 << 3),
	NET_PLAYER_FLAG_USER		= (1 << 4),
};

class net_player
{
public:

private:

	struct
	{
		std::unordered_map<std::string, std::unordered_map<std::string, gns::resource::info>> rsrc_files_to_check;
	} ft;

	SLNet::SystemAddress sys_address;

	std::string id_str,
				name,
				user_db_name;

	game_player* player = nullptr;

	uint64_t steam_id = 0;

	PLAYER_ID id = 0;

	uint64_t flags = 0;

	bool fully_joined = false,
		 game_purchased = false;

public:

	net_player();
	~net_player();
	
	void set_sys_address(const SLNet::SystemAddress& v)			{ sys_address = v; }
	void set_id_str(const std::string& v)						{ id_str = v; }
	void set_name(const std::string& v)							{ name = v; }
	void set_id(PLAYER_ID v)									{ id = v; }
	void set_player(game_player* v)								{ player = v; }
	void set_steam_id(uint64_t v)								{ steam_id = v; }
	void set_fully_joined()										{ fully_joined = true; }
	void verify_game_purchase()									{ game_purchased = true; }
	void sync_all_resources();
	void sync_all_resources_status();
	void add_file_to_check(gns::resource::info& info);
	void compare_and_send_files();
	void send_notification(const std::string& text, uint32_t color);

	bool login(const std::string& user, const std::string& pass, bool& invalid_pass);
	bool logout();
	bool has_fully_joined() const								{ return fully_joined; }
	bool has_purchased_game() const								{ return game_purchased; }
	bool is_logged_in() const									{ return flags != NET_PLAYER_FLAG_NONE; }
	bool is_owner() const										{ return (flags & NET_PLAYER_FLAG_OWNER); }
	bool is_admin() const										{ return (flags & NET_PLAYER_FLAG_ADMIN); }
	bool is_moderator() const									{ return (flags & NET_PLAYER_FLAG_MOD); }
	bool has_resource_permissions() const						{ return (is_admin() || is_owner()); }

	int get_ping() const;
	
	uint64_t get_steam_id() const								{ return steam_id; }
	uint64_t get_flags() const									{ return flags; }

	PLAYER_ID get_id()											{ return id; }

	game_player* get_player() const								{ return player; }

	const SLNet::SystemAddress& get_sys_address() const			{ return sys_address; }
	
	const std::string& get_id_str() const						{ return id_str; }
	const std::string& get_name() const							{ return name; }
};