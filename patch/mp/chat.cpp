import utils;

#include <shared/defs.h>
#include <shared/scripting/script.h>

#include <scripting/debugger.h>
#include <keycode/keycode.h>
#include <ui/ui.h>

#include "client.h"
#include "chat.h"
#include "cmd.h"

#undef min
#undef max

chat::chat()
{
	chat_cmd::register_commands();
}

void chat::key_input(uint32_t key, bool pressed)
{
	if (!enabled || !typing)
		return;

	if (pressed)
	{
		if (!cmd_history.empty())
		{
			auto set_new_message = [&](int index)
			{
				curr_msg = cmd_history[cmd_history_index = std::clamp(cmd_history_index, 0, static_cast<int>(cmd_history.size() - 1))];
			};

			if (key == KEY_UP)		  set_new_message(--cmd_history_index);
			else if (key == KEY_DOWN) set_new_message(++cmd_history_index);
		}
	}
}

void chat::add_char(wchar_t c)
{
	if (!enabled)
		return;

	if (curr_msg.length() < max_length())
		curr_msg += c;
}

void chat::remove_char()
{
	if (!enabled)
		return;

	if (!curr_msg.empty())
		curr_msg.pop_back();
}

void chat::remove_word()
{
	if (!enabled)
		return;

	if (auto last_space = curr_msg.find_last_of(L" _."); last_space == -1)
	{
		if (curr_msg.empty())
			return;
		else curr_msg = L"";
	}
	else curr_msg = curr_msg.substr(0, last_space);
}

void chat::paste_text()
{
	if (!enabled)
		return;

	curr_msg += utils::win::get_clipboard_text();

	if (curr_msg.length() >= max_length())
		curr_msg = curr_msg.substr(0, max_length() - 1);
}

void chat::begin_typing()
{
	if (!enabled || typing)
		return;

	curr_msg = L"";

	typing = true;
}

void chat::end_typing(bool send)
{
	if (!typing || !enabled)
		return;

	std::wstring msg_to_send = curr_msg;

	curr_msg.clear();

	typing = false;

	if (!send)
		return;

	if (msg_to_send.empty())
		return;

	if (*msg_to_send.begin() == L'/')
	{
		cmd_history.push_back(msg_to_send);

		cmd_history_index = static_cast<int>(cmd_history.size());

		std::string cmd,
					params,
					cmd_without_slash = utils::string::convert(msg_to_send).substr(1);

		if (!utils::string::split_left(cmd_without_slash, cmd, params, ' '))
			cmd = cmd_without_slash;

		if (!cmd.compare("clear"))
			chat_list.clear();
		else if (!cmd.compare("cleardbg"))
			g_debugger->clear();
		else
		{
			switch (chat_cmd::execute_command(cmd, utils::string::split(params, ' ')))
			{
			case CMD_EXEC_INVALID_CMD:
			{
				add_chat_msg(L"##ff0000ffInvalid command");
				break;
			}
			case CMD_EXEC_INVALID_PARAMS:
			{
				add_chat_msg(L"##ff0000ffInvalid parameters");
				break;
			}
			case CMD_EXEC_LAST_ERR:
			{
				add_chat_msg(L"##ff0000ff" + utils::string::convert(chat_cmd::get_last_error()));
				break;
			}
			}
		}
	}
	else
	{
		gns::net_player::chat info {};

		info.player_name = g_client->get_name().c_str();
		info.message =  msg_to_send.c_str();

		g_client->send_packet(ID_NET_PLAYER_CHAT, info);
	}
}

void chat::add_chat_msg(const std::wstring& msg)
{
	if (!enabled)
		return;

	std::lock_guard lock(chat_list_mtx);

	chat_list.push_back(utils::string::convert(msg));
}

void chat::add_chat_msg(const std::string& msg)
{
	if (!enabled)
		return;

	std::lock_guard lock(chat_list_mtx);

	chat_list.push_back(msg);
}

void chat::scroll_up()
{
	if (!enabled)
		return;

	scrolling = true;
	scroll_dir = -1;
}

void chat::scroll_down()
{
	if (!enabled)
		return;

	scrolling = true;
	scroll_dir = 1;
}

void chat::update()
{
	if (!enabled)
		return;

	g_ui->begin_window("##chat_wnd", { 5.f, 300.f }, { max_sx + 5.f, 300.f }, { 0.f, 0.f, 0.f, 0.5f });

	for (const auto& msg : chat_list)
		ImGui::TextWrapped(msg.c_str());

	if (scrolling)
	{
		curr = std::lerp(curr, 30.f * float(scroll_dir), 0.1f);

		if (scroll_dir == 0 && std::abs(curr) < 1.f)
			curr = 0.f;

		float new_scroll = ImGui::GetScrollY() + curr;

		ImGui::SetScrollY(new_scroll);

		if (new_scroll >= ImGui::GetScrollMaxY())
		{
			curr = 0.f;
			scrolling = false;
		}
		else scroll_dir = 0;
	}
	else ImGui::SetScrollY(std::lerp(ImGui::GetScrollY(), ImGui::GetScrollMaxY(), 0.25f));
	
	g_ui->end_window();

	if (typing)
	{
		auto sy = g_ui->calc_text_size(!curr_msg.empty() ? utils::string::convert(curr_msg).c_str() : " ", text_size, max_sx).y;

		g_ui->draw_filled_rect(5.f, 630.f, max_sx + 5.f, sy + 5.f, { 0.f, 0.f, 0.f, 0.5f });

		if (!curr_msg.empty())
			g_ui->add_text(curr_msg.c_str(), 10.f, 630.f, text_size, { 1.f, 1.f, 1.f, 1.f }, false, -1, max_sx);
	}
}

size_t chat::max_length() const
{
	return GNS_GLOBALS::MAX_CHAT_MSG_LENGTH - 1;
}