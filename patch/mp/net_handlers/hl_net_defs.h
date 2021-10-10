#pragma once

namespace file_handlers
{
	void handle_packet(uint16_t);

	void on_begin_transfer();
	void on_end_transfer();
	void on_file_list();
	void on_transfer();
}

namespace resource_handlers
{
	void handle_packet(uint16_t);

	void on_resource_sync_all();
	void on_resource_sync_all_status();
	void on_resource_stop(const std::string&);
}

namespace net_player_handlers
{
	void handle_packet(uint16_t);

	void on_player_connect();
	void on_player_name();
	void on_player_join();
	void on_player_quit();
	void on_player_chat();
	void on_player_notification();
	void on_player_script_error();
}

namespace server_handlers
{
	void handle_packet(uint16_t);

	void on_game_settings();
	void on_trigger_client_event();
}