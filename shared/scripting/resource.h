#pragma once

#include <shared/game_net/file_info.h>

class script;

enum script_type;
enum script_load_result;
enum script_action_result;

enum resource_action
{
	RESOURCE_ACTION_START,
	RESOURCE_ACTION_STOP,
	RESOURCE_ACTION_RESTART,
};

enum resource_status
{
	RESOURCE_STATUS_STARTED,
	RESOURCE_STATUS_STOPPED,
};

enum resource_refresh
{
	RESOURCE_REFRESH_ALREADY_RUNNING,
	RESOURCE_REFRESH_NO_META,
	RESOURCE_REFRESH_INVALID_SCRIPT,
	RESOURCE_REFRESH_SCRIPT_FAILED,
	RESOURCE_REFRESH_INVALID_FILE,
	RESOURCE_REFRESH_INVALID_EXPORTED_FN,
	RESOURCE_REFRESH_OK,
};

class resource
{
private:

	std::string path,
				folder,
				author,
				name,
				version,
				description;

	std::unordered_map<std::string, script*> scripts;

	std::unordered_map<std::string, file_info> server_files,
											   client_files;

	std::string last_error;

	int status = RESOURCE_STATUS_STOPPED;

	bool as_server = false,
		 running = false;

public:

	static constexpr auto META_FILE_NAME = "meta.json";

	resource(const std::string& path, const std::string& folder) : path(path), folder(folder) {}
	~resource();

	void set_path(const std::string& v)						{ path = v; }
	void set_author(const std::string& v)					{ author = v; }
	void set_name(const std::string& v)						{ name = v; }
	void set_version(const std::string& v)					{ version = v; }
	void set_description(const std::string& v)				{ description = v; }
	void mark_as_server()									{ as_server = true; }
	void set_last_error(const std::string& err)				{ last_error = err; }

	bool for_each_client_file(const std::function<bool(const file_info&)>& fn);
	bool is_running() const									{ return running; }
	bool has_script(const std::string& name) const			{ return scripts.contains(name); }

	const std::string& get_name() const						{ return name; }
	const std::string& get_path() const						{ return path; }
	const std::string& get_folder() const					{ return folder; }
	
	template <typename T>
	void for_each_script(const T& fn)
	{
		for (const auto& [name, s] : scripts)
			fn(s);
	}

	void clear_all_files();
	void destroy_all_scripts();

	bool is_script_loaded(script* s);
	bool create_from_server(bool autostart = true);
	bool create_from_client(const std::vector<std::string>& file_list);
	bool add_server_file(const std::string& name);
	bool add_client_file(const std::string& name);

	int get_status() const									{ return status; }

	resource_refresh refresh();

	script_action_result start();
	script_action_result stop();

	script_load_result add_script(const std::string& subpath, script_type type);

	script* get_script(const std::string& lookup);

	file_info* get_server_file(const std::string& name);
	file_info* get_client_file(const std::string& name);

	std::string get_filename(const std::string& subpath)	{ return folder + '\\' + subpath; }

	const std::string& get_last_error() const				{ return last_error; }

	const decltype(scripts)& get_scripts() const			{ return scripts; }

	void print();

	static bool verify_resource_name(const std::string& name);
	static bool verify_file_name(const std::string& name);
	static bool verify_path_file_name(const std::string& name);

	static std::string get_lookup(const std::string& folder, const std::string& name)	{ return folder + '\\' + name; }
};