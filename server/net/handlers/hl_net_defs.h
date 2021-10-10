#pragma once

class net_player;

namespace net_player_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_player_steam_id(net_player*);
	void on_player_join(net_player*);
	void on_player_quit(net_player*, bool timeout);
	void on_player_name(net_player*);
	void on_player_chat(net_player*);
	void on_player_register(net_player*);
	void on_player_login(net_player*);
	void on_player_logout(net_player*);
	void on_player_net_flags(net_player*);
	void on_player_resource_sync(net_player*, const std::string&);
}

namespace resource_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_resource_sync_all(net_player*);
	void on_resource_sync_all_status(net_player*);
	void on_resource_start(net_player*, const std::string&);
	void on_resource_stop(net_player*, const std::string&);
	void on_resource_restart(net_player*, const std::string&);
	void on_resource_refresh(net_player*);
}

namespace server_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_trigger_server_event(net_player*);
}