import prof;
import utils;

#include <shared/defs.h>
#include <shared/game_net/structs.h>

#include "script.h"
#include "resource.h"
#include "resource_system.h"

resource_system::~resource_system()
{
	for (const auto& [name, rsrc] : resources)
		clear_resource(rsrc, true);
}

void resource_system::set_action_callback(const on_resource_callback_t& on_rsrc_action)
{
	on_resource_action = on_rsrc_action;
}

void resource_system::clear_resource(resource* rsrc, bool destroy)
{
	clear_resource_commands(rsrc);
	clear_resource_events(rsrc);
	clear_resource_binds(rsrc);

	if (destroy)
		delete rsrc;
}

resource* resource_system::load_server_resource(const std::string& path, const std::string& folder, bool autostart)
{
	if (auto r = get_resource(folder))
		return r;

	if (!resource::verify_resource_name(folder))
		return nullptr;

	auto rsrc = new resource(path, folder);

	rsrc->mark_as_server();

	if (!rsrc->create_from_server(autostart))
	{
		prof::print(DARK_GREEN, "Resource '{}' could not be parsed or loaded", folder.c_str());
		delete rsrc;
		return nullptr;
	}

	resources.insert({ folder, rsrc });

	return rsrc;
}

resource* resource_system::load_client_resource(const std::string& folder, const std::vector<std::string>& file_list)
{
	if (auto r = get_resource(folder))
	{
		if (r->get_status() == RESOURCE_STATUS_STOPPED)
			r->create_from_client(file_list);

		return r;
	}

	if (!resource::verify_resource_name(folder))
		return nullptr;

	auto rsrc = new resource(RESOURCES_PATH + folder + '\\', folder);

	if (!rsrc->create_from_client(file_list))
		prof::print(DARK_GREEN, "Resource '{}' could not be loaded", folder.c_str());

	resources.insert({ folder, rsrc });

	return rsrc;
}

bool resource_system::get_list_of_files(const std::function<void(const std::string&, const std::string&, FILE_HASH)>& file_cb)
{
	if (!std::filesystem::is_directory(resource_system::RESOURCES_PATH))
		std::filesystem::create_directory(resource_system::RESOURCES_PATH);

	for (const auto& dp : std::filesystem::directory_iterator(resource_system::RESOURCES_PATH))
	{
		if (!dp.is_directory())
			continue;

		const auto& dpp = dp.path();

		auto dfolder = dpp.filename().string(),
			 dpath = dpp.string();

		for (const auto& fp : std::filesystem::recursive_directory_iterator(dpath))
		{
			if (!fp.is_regular_file())
				continue;
			
			const auto& fpp = fp.path();

			auto filename = fpp.filename().string(),
				 path = fpp.parent_path().string() + '\\',
				 sub_path = path.substr(dpath.length() + 1);

			file_cb(dfolder, sub_path + filename, utils::file::hash_file_simple(path + filename));
		}
	}

	return true;
}

bool resource_system::trigger_remote_event(const std::string& event_name, const std::vector<std::any>& va)
{
	const auto args = sol::as_args(va);

	if (auto it = events.find(event_name); it != events.end())
		for (const auto& [rsrc, script_events] : it->second)
			for (const auto& [s, event_info] : script_events)
				if (event_info.allow_remote_trigger)
					event_info.fn(args);

	return true;
}

bool resource_system::trigger_non_remote_event(const std::string& event_name, const sol::variadic_args& va)
{
	if (auto it = events.find(event_name); it != events.end())
		for (const auto& [rsrc, script_events] : it->second)
			for (const auto& [s, event_info] : script_events)
				event_info.fn(va);

	return true;
}

bool resource_system::add_event(const std::string& name, const sol::function& fn, script* s, bool allow_remote_trigger)
{
	auto rsrc_owner = s->get_owner();
	if (!rsrc_owner)
		return false;

	if (auto it_event = events.find(name); it_event != events.end())
	{
		auto& rsrc_event_info = it_event->second;

		if (auto it_rsrc_event = rsrc_event_info.find(rsrc_owner); it_rsrc_event != rsrc_event_info.end())
		{
			auto& rsrc_script_events = it_rsrc_event->second;

			for (const auto& [event_script, event_info] : rsrc_script_events)
				if (event_script == s)
					return false;

			rsrc_script_events.push_back({ s, { name, fn, allow_remote_trigger } });
		}
		else rsrc_event_info[rsrc_owner].push_back({ s, { name, fn, allow_remote_trigger } });
	}
	else events[name][rsrc_owner].push_back({ s, { name, fn, allow_remote_trigger } });

	return true;
}

bool resource_system::remove_event(const std::string& name, script* s)
{
	auto rsrc_owner = s->get_owner();
	if (!rsrc_owner)
		return false;

	if (auto it_event = events.find(name); it_event != events.end())
	{
		auto& rsrc_events = it_event->second;

		if (auto it_rsrc_event = rsrc_events.find(rsrc_owner); it_rsrc_event != rsrc_events.end())
		{
			auto& rsrc_script_events = it_rsrc_event->second;

			for (auto it = rsrc_script_events.begin(); it != rsrc_script_events.end(); ++it)
				if (it->first == s)
				{
					rsrc_script_events.erase(it);

					if (rsrc_script_events.empty())
						rsrc_events.erase(rsrc_owner);

					if (rsrc_events.empty())
						events.erase(name);

					return true;
				}
		}
	}

	return false;
}

bool resource_system::add_command(const std::string& name, script* s, const sol::function& fn)
{
	auto rsrc_owner = s->get_owner();
	if (!rsrc_owner)
		return false;

	if (auto it_event = commands.find(name); it_event != commands.end())
	{
		auto& rsrc_cmds = it_event->second;

		if (auto it_rsrc_cmd = rsrc_cmds.find(rsrc_owner); it_rsrc_cmd != rsrc_cmds.end())
		{
			auto& scripts_cmds = it_rsrc_cmd->second;

			for (const auto& [cmd_script, cmd_fn] : scripts_cmds)
				if (cmd_script == s)
					return false;

			scripts_cmds.push_back({ s, fn });
		}
		else rsrc_cmds[rsrc_owner].push_back({ s, fn });
	}
	else commands[name][rsrc_owner].push_back({ s, fn });

	return true;
}

bool resource_system::remove_command(const std::string& name, script* s)
{
	auto rsrc_owner = s->get_owner();
	if (!rsrc_owner)
		return false;

	if (auto it_event = commands.find(name); it_event != commands.end())
	{
		auto& rsrc_cmds = it_event->second;

		if (auto it_rsrc_cmd = rsrc_cmds.find(rsrc_owner); it_rsrc_cmd != rsrc_cmds.end())
		{
			auto& scripts_cmds = it_rsrc_cmd->second;

			for (auto it = scripts_cmds.begin(); it != scripts_cmds.end(); ++it)
				if (it->first == s)
				{
					scripts_cmds.erase(it);

					if (scripts_cmds.empty())
						rsrc_cmds.erase(rsrc_owner);

					if (rsrc_cmds.empty())
						commands.erase(name);

					return true;
				}
		}
	}

	return false;
}

bool resource_system::add_bind(uint32_t key, const char* state, script* s, const sol::function& fn)
{
	auto rsrc_owner = s->get_owner();
	if (!rsrc_owner)
		return false;

	auto m_key = std::make_pair(key, !strcmp(state, "down"));

	if (auto it_bind = binds.find(m_key); it_bind != binds.end())
	{
		auto& rsrc_binds = it_bind->second;

		if (auto it_rsrc_bind = rsrc_binds.find(rsrc_owner); it_rsrc_bind != rsrc_binds.end())
		{
			auto& scripts_binds = it_rsrc_bind->second;

			for (const auto& [bind_script, bind_fn] : scripts_binds)
				if (bind_script == s)
					return false;

			scripts_binds.push_back({ s, fn });
		}
		else rsrc_binds[rsrc_owner].push_back({ s, fn });
	}
	else binds[m_key][rsrc_owner].push_back({ s, fn });

	return true;
}

bool resource_system::remove_bind(uint32_t key, const char* state, script* s)
{
	auto rsrc_owner = s->get_owner();
	if (!rsrc_owner)
		return false;

	auto m_key = std::make_pair(key, !strcmp(state, "down"));

	if (auto it_bind = binds.find(m_key); it_bind != binds.end())
	{
		auto& rsrc_binds = it_bind->second;

		if (auto it_rsrc_bind = rsrc_binds.find(rsrc_owner); it_rsrc_bind != rsrc_binds.end())
		{
			auto& scripts_binds = it_rsrc_bind->second;

			for (auto it = scripts_binds.begin(); it != scripts_binds.end(); ++it)
				if (it->first == s)
				{
					scripts_binds.erase(it);

					if (scripts_binds.empty())
						rsrc_binds.erase(rsrc_owner);

					if (rsrc_binds.empty())
						binds.erase(m_key);

					return true;
				}
		}
	}

	return false;
}

bool resource_system::call_command(const std::string& cmd, const std::vector<std::string>& args)
{
	if (auto it = commands.find(cmd); it != commands.end())
	{
		for (const auto& [rsrc, scripts] : it->second)
			for (auto& [s, cmd_fn] : scripts)
				cmd_fn(sol::as_args(args));

		return true;
	}

	return false;
}

bool resource_system::call_bind(uint32_t key, bool state)
{
	if (auto it = binds.find({ key, state }); it != binds.end())
	{
		for (const auto& [rsrc, scripts] : it->second)
			for (auto& [s, bind_fn] : scripts)
				bind_fn(key, state);

		return true;
	}

	return false;
}

void resource_system::clear_resource_events(resource* rsrc)
{
	for (auto& [_, rsrc_events] : events)
		rsrc_events.erase(rsrc);
}

void resource_system::clear_resource_commands(resource* rsrc)
{
	for (auto& [_, rsrc_cmds] : commands)
		rsrc_cmds.erase(rsrc);
}

void resource_system::clear_resource_binds(resource* rsrc)
{
	for (auto& [_, rsrc_binds] : binds)
		rsrc_binds.erase(rsrc);
}

script_action_result resource_system::start_resource(const std::string& name)
{
	if (auto r = get_resource(name))
	{
		if (r->is_running())
			return SCRIPT_ACTION_ALREADY_RUNNING;

		if (on_resource_action)
			on_resource_action(r, RESOURCE_ACTION_START, SCRIPT_ACTION_OK, false);

		if (is_server && r->refresh() != RESOURCE_REFRESH_OK)
		{
			last_error = r->get_last_error();
			return SCRIPT_ACTION_FAIL;
		}

		auto res = r->start();

		last_error = r->get_last_error();

		if (on_resource_action)
			on_resource_action(r, RESOURCE_ACTION_START, res, true);

		return res;
	}

	return SCRIPT_ACTION_NOT_FOUND;
}

script_action_result resource_system::stop_resource(const std::string& name)
{
	if (auto r = get_resource(name))
	{
		if (!r->is_running())
			return SCRIPT_ACTION_NOT_RUNNING;

		if (on_resource_action)
			on_resource_action(r, RESOURCE_ACTION_STOP, SCRIPT_ACTION_OK, false);

		auto res = r->stop();

		last_error = r->get_last_error();

		if (on_resource_action)
			on_resource_action(r, RESOURCE_ACTION_STOP, res, true);

		return res;
	}

	return SCRIPT_ACTION_NOT_FOUND;
}

script_action_result resource_system::restart_resource(const std::string& name)
{
	if (auto r = get_resource(name))
	{
		if (auto stop_res = stop_resource(name); stop_res != SCRIPT_ACTION_OK)
			return stop_res;

		return start_resource(name);
	}

	return SCRIPT_ACTION_NOT_FOUND;
}

void resource_system::for_each_resource(const std::function<void(resource*)>& fn)
{
	for (const auto& [name, rsrc] : resources)
		fn(rsrc);
}

void resource_system::restart_all()
{
	for (const auto& [name, rsrc] : resources)
		restart_resource(name);
}

resource* resource_system::get_resource(const std::string& name)
{
	auto it = resources.find(name);
	return (it != resources.end() ? it->second : nullptr);
}