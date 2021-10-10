#pragma once

import utils;

#include "script.h"

#include <shared/game_net/file_info.h>

class resource;

enum resource_action;

using on_resource_callback_t = std::function<void(resource*, resource_action, script_action_result, bool)>;
using setup_fn_t = std::function<void(sol::state*)>;
using err_callback_t = std::function<void(script*, const std::string&)>;

struct uint32_t_bool_pair_hash
{
	template <typename Tx, typename Ty>
	size_t operator() (const std::pair<Tx, Ty>& p) const
	{
		size_t h = 0;

		utils::hash::hash_combine(h, std::hash<Tx>{}(p.first), std::hash<Ty>{}(p.second));

		return h;
	}
};

struct script_event_info
{
	std::string name;

	sol::function fn;

	bool allow_remote_trigger;
};

class resource_system
{
private:

	setup_fn_t setup_fn;
	err_callback_t err_callback;

	std::unordered_map<std::string, resource*> resources;
	std::unordered_map<std::string, std::unordered_map<resource*, std::vector<std::pair<script*, script_event_info>>>> events;
	std::unordered_map<std::string, std::unordered_map<resource*, std::vector<std::pair<script*, sol::function>>>> commands;
	std::unordered_map<std::pair<uint32_t, bool>, std::unordered_map<resource*, std::vector<std::pair<script*, sol::function>>>, uint32_t_bool_pair_hash> binds;

	on_resource_callback_t on_resource_action;

	std::string last_error;

	std::stack<int> cancelled_events;

	int current_event_id = -1;

	bool is_server = false;

public:

	static constexpr auto RESOURCES_PATH = "resources\\";
	static constexpr auto MAX_RESOURCES = 512;

	resource_system()									{}
	~resource_system();

	void register_as_server()							{ is_server = true; }
	void register_as_client()							{ is_server = false; }
	void cancel_event()									{ cancelled_events.push(current_event_id); }
	void set_script_setup_fn(const setup_fn_t& fn)		{ setup_fn = fn; }
	void set_err_callback_fn(const err_callback_t& fn)	{ err_callback = fn; }
	void set_action_callback(const on_resource_callback_t& on_rsrc_action);
	void clear_resource(resource* rsrc, bool destroy);

	int get_resources_count() const						{ return int(resources.size()); }

	// server functions

	resource* load_server_resource(const std::string& path, const std::string& folder, bool autostart = true);

	// client functions

	resource* load_client_resource(const std::string& folder, const std::vector<std::string>& file_list);

	bool get_list_of_files(const std::function<void(const std::string&, const std::string&, FILE_HASH)>& file_cb);

	// shared functions

	template <typename T>
	void update_global(const std::string& name, const T& value)
	{
		for (const auto& [rsrc_name, rsrc] : resources)
			for (const auto& [script_name, script] : rsrc->get_scripts())
				script->update_global(name, value);
	}

	// remote and non-remote events

	bool trigger_remote_event(const std::string& event_name, const std::vector<std::any>& va);
	bool trigger_non_remote_event(const std::string& event_name, const sol::variadic_args& va);

	// engine events

	template <typename... A>
	bool trigger_event(const std::string& event_name, A&&... args)
	{
		++current_event_id;

		if (auto it = events.find(event_name); it != events.end())
			for (const auto& [rsrc, script_events] : it->second)
				for (const auto& [s, event_info] : script_events)
					event_info.fn(args...);

		if (!cancelled_events.empty() && cancelled_events.top() == current_event_id--)
		{
			cancelled_events.pop();
			return false;
		}

		return true;
	}

	template <typename T>
	T exec_string(const std::string& script)
	{
		sol::state vm;

		if (sol::function_result fr = vm.do_string(script); fr.valid())
			return fr.get<T>();

		return {};
	}

	bool add_event(const std::string& name, const sol::function& fn, script* s, bool allow_remote_trigger);
	bool remove_event(const std::string& name, script* s);

	bool add_command(const std::string& name, script* s, const sol::function& fn);
	bool remove_command(const std::string& name, script* s);

	bool add_bind(uint32_t key, const char* state, script* s, const sol::function& fn);
	bool remove_bind(uint32_t key, const char* state, script* s);

	bool call_command(const std::string& cmd, const std::vector<std::string>& args);
	bool call_bind(uint32_t key, bool state);

	void clear_resource_events(resource* rsrc);
	void clear_resource_commands(resource* rsrc);
	void clear_resource_binds(resource* rsrc);

	script_action_result start_resource(const std::string& name);
	script_action_result stop_resource(const std::string& name);
	script_action_result restart_resource(const std::string& name);

	void for_each_resource(const std::function<void(resource*)>& fn);
	void restart_all();

	resource* get_resource(const std::string& name);

	const setup_fn_t& get_script_setup_fn() const		{ return setup_fn; }
	const err_callback_t& get_err_callback() const		{ return err_callback; }

	const std::string& get_last_error() const			{ return last_error; }
};

inline std::unique_ptr<resource_system> g_resource;