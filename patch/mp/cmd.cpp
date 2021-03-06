#include <shared/defs.h>
#include <shared/scripting/script.h>
#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>

#include <scripting/debugger.h>

#include <specific/standard.h>
#include <game/traps.h>

#include "cmd.h"
#include "client.h"

namespace chat_cmd
{
	std::unordered_map<std::string, std::function<cmd_fn_t>> g_cmds;

	std::string last_error;

	void register_commands()
	{
		// debug commands
		g_cmds.insert({ "debug", toggle_debug });
		g_cmds.insert({ "netstat", toggle_netstats });

		// player commands
		g_cmds.insert({ "register", register_user });
		g_cmds.insert({ "login", login });
		g_cmds.insert({ "logout", logout });
		g_cmds.insert({ "name", name });
		g_cmds.insert({ "set_flags", set_flags });

		// resource commands
		g_cmds.insert({ "start", start_resource });
		g_cmds.insert({ "stop", stop_resource });
		g_cmds.insert({ "restart", restart_resource });
		g_cmds.insert({ "refresh", refresh });
	}

	void destroy_all_commands()
	{
		g_cmds.clear();
	}

	cmd_exec_result execute_command(const std::string& cmd, cmd_params params)
	{
		if (auto it = g_cmds.find(cmd); it != g_cmds.end())
			return it->second(params);

		return (g_resource->call_command(cmd, params) ? CMD_EXEC_OK : CMD_EXEC_INVALID_CMD);
	}

	cmd_exec_result toggle_netstats(cmd_params params)
	{
		if (params.size() != 1)
			return CMD_EXEC_INVALID_PARAMS;

		if (int level = std::stoi(params[0]); level >= 0 && level <= 3)
			g_client->set_net_stats(level - 1);
		else return CMD_EXEC_INVALID_PARAMS;

		return CMD_EXEC_OK;
	}

	cmd_exec_result toggle_debug(cmd_params)
	{
		g_debugger->toggle();

		return CMD_EXEC_OK;
	}

	cmd_exec_result register_user(cmd_params params)
	{
		if (params.size() != 2)
			return CMD_EXEC_INVALID_PARAMS;

		gns::net_player::register_login info {};

		info.user = params[0].c_str();
		info.pass = params[1].c_str();

		if (!g_client->send_packet(ID_NET_PLAYER_REGISTER, info))
			return set_last_error("register failed");

		return CMD_EXEC_OK;
	}

	cmd_exec_result login(cmd_params params)
	{
		if (params.size() != 2)
			return CMD_EXEC_INVALID_PARAMS;

		gns::net_player::register_login info {};

		info.user = params[0].c_str();
		info.pass = params[1].c_str();

		if (!g_client->send_packet(ID_NET_PLAYER_LOGIN, info))
			return set_last_error("login failed");

		return CMD_EXEC_OK;
	}

	cmd_exec_result logout(cmd_params params)
	{
		if (params.size() != 0)
			return CMD_EXEC_INVALID_PARAMS;

		if (!g_client->send_packet(ID_NET_PLAYER_LOGOUT))
			return set_last_error("logout failed");

		return CMD_EXEC_OK;
	}

	cmd_exec_result set_flags(cmd_params params)
	{
		if (params.size() != 2)
			return CMD_EXEC_INVALID_PARAMS;

		if (uint64_t flags = std::stoi(params[1]); flags != 0)
		{
			gns::net_player::net_flags info {};

			info.user = params[0].c_str();
			info.flags = flags;

			if (!g_client->send_packet(ID_NET_PLAYER_NET_FLAGS, info))
				return set_last_error("logout failed");
		}

		return CMD_EXEC_OK;
	}

	cmd_exec_result name(cmd_params params)
	{
		if (params.size() != 1)
			return CMD_EXEC_INVALID_PARAMS;

		if (!g_client->set_name(params[0]))
			return set_last_error("Name is too short or too big");

		return CMD_EXEC_OK;
	}

	cmd_exec_result start_resource(cmd_params params)
	{
		if (params.size() != 1)
			return CMD_EXEC_INVALID_PARAMS;

		gns::resource::action info {};

		info.name = params[0].c_str();
		info.action = RESOURCE_ACTION_START;

		if (!g_client->send_packet(ID_RESOURCE_ACTION, info))
			return set_last_error("Start resource failed");

		return CMD_EXEC_OK;
	}

	cmd_exec_result stop_resource(cmd_params params)
	{
		if (params.size() != 1)
			return CMD_EXEC_INVALID_PARAMS;

		gns::resource::action info {};

		info.name = params[0].c_str();
		info.action = RESOURCE_ACTION_STOP;

		if (!g_client->send_packet(ID_RESOURCE_ACTION, info))
			return set_last_error("Stop resource failed");

		return CMD_EXEC_OK;
	}

	cmd_exec_result restart_resource(cmd_params params)
	{
		if (params.size() != 1)
			return CMD_EXEC_INVALID_PARAMS;

		gns::resource::action info {};

		info.name = params[0].c_str();
		info.action = RESOURCE_ACTION_RESTART;

		if (!g_client->send_packet(ID_RESOURCE_ACTION, info))
			return set_last_error("Restart resource failed");

		return CMD_EXEC_OK;
	}

	cmd_exec_result refresh(cmd_params params)
	{
		if (params.size() != 0)
			return CMD_EXEC_INVALID_PARAMS;

		if (!g_client->send_packet(ID_RESOURCE_REFRESH_ALL))
			return set_last_error("Refresh failed");

		return CMD_EXEC_OK;
	}

	cmd_exec_result set_last_error(const std::string& v)
	{
		last_error = v;
		return CMD_EXEC_LAST_ERR;
	}

	const std::string& get_last_error()
	{
		return last_error;
	}
}