import utils;
import prof;

#include <shared/defs.h>
#include <shared/game_net/structs.h>

#include <json.hpp>

#include "resource_system.h"
#include "resource.h"
#include "script.h"
#include "events.h"

using nlohmann::json;

resource::~resource()
{
	destroy_all_scripts();
}

bool resource::for_each_client_file(const std::function<bool(const file_info&)>& fn)
{
	if (client_files.empty())
		return true;

	for (const auto& [name, info] : client_files)
		if (fn(info))
			return true;

	return false;
}

void resource::clear_all_files()
{
	server_files.clear();
	client_files.clear();
}

void resource::destroy_all_scripts()
{
	for (const auto& [name, script] : scripts)
		delete script;

	scripts.clear();
}

bool resource::is_script_loaded(script* s)
{
	if (!running)
		return false;

	return scripts.contains(s->get_lookup_name());
}

bool resource::create_from_server(bool autostart)
{
	if (auto refreshed_res = refresh(); refreshed_res != RESOURCE_REFRESH_OK)
		return false;

	if (autostart && start() != SCRIPT_ACTION_OK)
	{
		prof::printt(DARK_RED, last_error.c_str());

		stop();

		return false;
	}

	return true;
}

bool resource::create_from_client(const std::vector<std::string>& file_list)
{
	destroy_all_scripts();

	// remove files that are no longer used

	for_each_client_file([&](const file_info& info)
	{
		for (const auto& subpath : file_list)
			if (subpath == info.name)
				return false;

		std::filesystem::remove(path + info.name);

		return false;
	});

	clear_all_files();

	for (const auto& subpath : file_list)
	{
		if (const bool is_script = (std::filesystem::path(subpath).extension() == script::FILE_EXT))
		{
			if (add_script(subpath, SCRIPT_TYPE_CLIENT) != SCRIPT_OK)
				return false;
		}
		else add_client_file(subpath);
	}

	return true;
}

bool resource::add_server_file(const std::string& name)
{
	auto hash = utils::file::hash_file_simple(path + name);
	if (!hash)
		return false;

	const auto lookup = get_lookup(folder, name);

	server_files.insert({ lookup,
	{
		.rsrc = folder,
		.name = name,
		.hash = hash
	} });

	return true;
}

bool resource::add_client_file(const std::string& name)
{
	auto hash = utils::file::hash_file_simple(path + name);
	if (!hash)
		return false;

	const auto lookup = get_lookup(folder, name);

	client_files.insert({ lookup,
	{
		.rsrc = folder,
		.name = name,
		.hash = hash
	} });

	return true;
}

resource_refresh resource::refresh()
{
	auto set_error = [&](const std::string& err, resource_refresh id)
	{
		prof::print(DARK_RED, "Failed loading '{}' ({})", folder.c_str(), err.c_str());
		last_error = err;
		return id;
	};

	if (is_running())
		return set_error("Resource already running", RESOURCE_REFRESH_ALREADY_RUNNING);

	auto meta_file = std::ifstream(path + META_FILE_NAME);
	if (!meta_file)
		return set_error("Resource has no meta file", RESOURCE_REFRESH_NO_META);

	json meta;

	meta_file >> meta;

	if (auto val = meta["author"]; val.is_string())			author = val;
	if (auto val = meta["name"]; val.is_string())			name = val;
	if (auto val = meta["version"]; val.is_string())		version = val;
	if (auto val = meta["description"]; val.is_string())	description = val;

	auto scripts = meta["scripts"],
		 files = meta["files"],
		 exports = meta["exports"];

	destroy_all_scripts();
	clear_all_files();

	for (const auto& script : scripts)
	{
		auto src = script["src"],
			 type = script["type"];

		if (!src.is_string() || !type.is_string())
			return set_error("Resource has an invalid script list", RESOURCE_REFRESH_INVALID_SCRIPT);

		if (add_script(src, script_type(utils::hash::JENKINS(std::string(type)))) == SCRIPT_FAILED)
			return set_error("Resource could not add a script", RESOURCE_REFRESH_SCRIPT_FAILED);
	}

	for (const std::string& file : files)
		if (!verify_path_file_name(file) || !add_client_file(file))
			return set_error("Resource has an invalid file list", RESOURCE_REFRESH_INVALID_FILE);

	for (const auto& exp : exports)
	{
		auto fn = exp["fn"],
			 type = exp["type"];

		if (!fn.is_string() || !type.is_string())
			return set_error("Resource has an invalid exported functions list", RESOURCE_REFRESH_INVALID_EXPORTED_FN);

		// TODO
	}

	return RESOURCE_REFRESH_OK;
}

script_action_result resource::start()
{
	for (const auto& [name, script] : scripts)
	{
		if (script->is_running())
			return SCRIPT_ACTION_ALREADY_RUNNING;

		if (script->start() != SCRIPT_ACTION_OK)
		{
			if (!last_error.empty())
				prof::printt(DARK_RED, last_error.c_str());

			stop();

			return SCRIPT_ACTION_SCRIPT_ERROR;
		}
	}

	g_resource->trigger_event(events::resource::ON_START, this);

	status = RESOURCE_STATUS_STARTED;
	running = true;

	return SCRIPT_ACTION_OK;
}

script_action_result resource::stop()
{
	g_resource->trigger_event(events::resource::ON_STOP, this);
	g_resource->clear_resource(this, false);

	for (const auto& [name, script] : scripts)
		script->stop();

	const bool was_running = running;

	status = RESOURCE_STATUS_STOPPED;
	running = false;

	return (was_running ? SCRIPT_ACTION_OK : SCRIPT_ACTION_NOT_RUNNING);
}

script_load_result resource::add_script(const std::string& subpath, script_type type)
{
	bool load = false;

	if (as_server)
	{
		if (type == SCRIPT_TYPE_SHARED)
		{
			if (!add_server_file(subpath)) return SCRIPT_FAILED;
			if (!add_client_file(subpath)) return SCRIPT_FAILED;

			load = true;
		}
		else if (type == SCRIPT_TYPE_SERVER)
		{
			if (!add_server_file(subpath))
				return SCRIPT_FAILED;

			load = true;
		}
		else if (!add_client_file(subpath))
			return SCRIPT_FAILED;
	}
	else if (!(load = add_client_file(subpath)))
		return SCRIPT_FAILED;

	if (!load)
		return SCRIPT_INVALID_PLATFORM;

	const auto lookup_name = get_lookup(folder, subpath);

	if (scripts.contains(lookup_name))
		return SCRIPT_EXISTS;

	scripts.insert({ lookup_name, new script(this, subpath, lookup_name, type) });

	return SCRIPT_OK;
}

script* resource::get_script(const std::string& lookup)
{
	auto it = scripts.find(lookup);
	return (it != scripts.end() ? it->second : nullptr);
}

file_info* resource::get_server_file(const std::string& name)
{
	auto it = server_files.find(name);
	return (it != server_files.end() ? &it->second : nullptr);
}

file_info* resource::get_client_file(const std::string& name)
{
	auto it = client_files.find(name);
	return (it != client_files.end() ? &it->second : nullptr);
}

void resource::print()
{
	if (as_server)
	{
		prof::print(YELLOW, "Author: {}", author.c_str());
		prof::print(YELLOW, "Folder Name: {}", folder.c_str());
		prof::print(YELLOW, "Name: {}", name.c_str());
		prof::print(YELLOW, "Version: {}", version.c_str());
		prof::print(YELLOW, "Description: {}", description.c_str());
		prof::print(YELLOW, "Server Files:");

		for (const auto& [name, info] : server_files)
			prof::print(YELLOW, "\t- '{}' {:#x}", name.c_str(), info.hash);
	}

	prof::print(YELLOW, "Client Files:");
	
	for (const auto& [name, info] : client_files)
		prof::print(YELLOW, "\t- '{}' {:#x}", name.c_str(), info.hash);
}

bool resource::verify_resource_name(const std::string& name)
{
	return (std::regex_match(name, std::regex("[a-zA-Z0-9\\_]+")) && name.length() < GNS_GLOBALS::MAX_RESOURCE_NAME_LENGTH);
}

bool resource::verify_file_name(const std::string& name)
{
	return std::regex_match(name, std::regex("[a-zA-Z0-9\\_]+\\.[a-zA-Z0-9\\_]+"));
}

bool resource::verify_path_file_name(const std::string& name)
{
	if (auto last_blackslash = name.find_last_of('\\'); last_blackslash != std::string::npos)
		return verify_file_name(name.substr(last_blackslash + 1));

	return verify_file_name(name);
}