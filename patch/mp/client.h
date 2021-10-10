#pragma once

#include <shared/game_net/bitstream.h>
#include <shared/game_net/structs.h>
#include <shared/game_net/file_transfer.h>

class resource;
class game_level;

enum resource_action;
enum script_action_result;

enum conn_err
{
	CONN_ERR_OK = 20,
	CONN_ERR_CONN_LOST,
	CONN_ERR_INVALID_PASSWORD,
	CONN_ERR_BANNED,
	CONN_ERR_ATTEMPT_FAILED,
};

class client : public bit_streaming
{
private:

	struct
	{
		struct file_info_data
		{
			std::vector<char> data;

			FILE_HASH hash;

			int size,
				bytes_rcved;
		};

		std::unordered_map<std::string, file_info_data> received_files_data;
		std::unordered_map<std::string, std::vector<std::string>> rsrcs_file_list;

		bool downloading = false;
	} ft;

	gns::server::game_settings game_settings {};

	std::unordered_map<std::string, std::vector<std::string>> resources_to_load;

	std::string nickname,
				used_ip,
				used_pass,
				net_stats;

	SLNet::SystemAddress sys_address = SLNet::UNASSIGNED_SYSTEM_ADDRESS,
						 sv_sys_address = SLNet::UNASSIGNED_SYSTEM_ADDRESS;

	conn_err last_err = CONN_ERR_OK;

	PLAYER_ID id = 0;

	SYNC_ID sync_id = INITIAL_SYNC_ID;

	int net_stat_level = -1;

	std::atomic_bool connected = false,
					 ready = false,
					 game_initialized = false,
					 wait_to_load = true;

public:

	client();
	~client();

	bool init();
	bool connect();
	bool dispatch_packets(int sleep_time = -1);

	void join(bool spawn_ready = false);
	void add_resource_script(const std::string& rsrc_name, const std::string& script);
	void sync_resource_all();
	void sync_resource_all_status();
	void set_ready_status(bool v)								{ ready = v; }
	void set_sync_id(SYNC_ID v)									{ sync_id = v; }
	void initialize_game()										{ game_initialized = true; }
	void clear_rsrcs_file_list()								{ ft.rsrcs_file_list.clear(); }
	void begin_download()										{ ft.downloading = wait_to_load = true; }
	void end_download()											{ ft.downloading = wait_to_load = false; }
	void cancel_wait()											{ wait_to_load = false; }
	void set_net_stats(int level)								{ net_stat_level = level; }
	void set_last_error(conn_err err)							{ last_err = err; }

	void sync_with_players();
	void sync_level_entities(const std::vector<std::pair<int, int16_t>>& level_entities);
	void sync_spawned_entities();

	bool add_file_data(const std::string& filename, std::vector<char>& data, int total_size, FILE_HASH hash);
	bool save_file(const std::string& filename);
	void save_resource_file(const std::string& rsrc, const std::string& file);
	bool set_name(const std::string& val);
	bool is_downloading() const									{ return ft.downloading; }
	
	bool is_game_initialized() const							{ return game_initialized; }
	bool is_connected() const									{ return connected; }
	bool is_ready() const										{ return ready; }
	bool can_load() const										{ return !wait_to_load; }
	bool has_id(PLAYER_ID v) const								{ return v == id; }
	bool has_sync_id(SYNC_ID v) const							{ return v == sync_id; }

	int get_net_stat() const									{ return net_stat_level; }
	int get_ping();

	conn_err get_last_err() const								{ return last_err; }

	SYNC_ID get_sync_id() const									{ return sync_id; }

	uint64_t get_id() const										{ return id; }

	const std::string& get_name() const							{ return nickname; }
	const std::string& get_net_stat_str() const					{ return net_stats; }

	const std::vector<std::string>& get_rsrc_file_list(const std::string& rsrc);
	
	decltype(game_settings)& get_game_settings()				{ return game_settings; }

	static void on_resource_action(resource* rsrc, resource_action action, script_action_result res, bool done);
};

inline std::unique_ptr<client> g_client;