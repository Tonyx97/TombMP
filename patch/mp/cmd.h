#pragma once

#include <sol/sol.hpp>

enum cmd_exec_result
{
	CMD_EXEC_OK,
	CMD_EXEC_INVALID_CMD,
	CMD_EXEC_INVALID_PARAMS,
	CMD_EXEC_LAST_ERR,
};

namespace chat_cmd
{
	using cmd_params = const std::vector<std::string>&;
	using cmd_fn_t = cmd_exec_result(cmd_params);

	void register_commands();
	void destroy_all_commands();

	cmd_exec_result execute_command(const std::string& cmd, cmd_params params);

	cmd_exec_result toggle_netstats(cmd_params);
	cmd_exec_result toggle_debug(cmd_params);
	cmd_exec_result register_user(cmd_params);
	cmd_exec_result login(cmd_params);
	cmd_exec_result logout(cmd_params);
	cmd_exec_result set_flags(cmd_params);
	cmd_exec_result name(cmd_params);
	cmd_exec_result start_resource(cmd_params);
	cmd_exec_result stop_resource(cmd_params);
	cmd_exec_result restart_resource(cmd_params);
	cmd_exec_result refresh(cmd_params);

	cmd_exec_result set_last_error(const std::string& v);
	const std::string& get_last_error();
}