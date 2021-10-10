import utils;

#include <shared/defs.h>
#include <shared/scripting/script.h>

#include <keycode/keycode.h>
#include <ui/ui.h>

#include "debugger.h"

#undef min
#undef max

void debugger::update()
{
	if (!enabled)
		return;

	if (g_keycode->is_key_down(KEY_CONTROL))
	{
		if (g_keycode->is_key_down(KEY_PAGE_UP))		scroll_up();
		else if (g_keycode->is_key_down(KEY_PAGE_DOWN)) scroll_down();
		else if (g_keycode->is_key_down(KEY_DOWN))		scrolling = false;
	}

	auto [sx, sy] = g_ui->get_screen_size();

	g_ui->begin_window("##debugger_wnd", { sx / 2.f - SIZE_X / 2.f, sy - SIZE_Y - 50.f }, { SIZE_X, SIZE_Y }, { 0.f, 0.f, 0.f, 0.25f });

	const auto count = msgs_count();

	if (count > 256)
		base = count - 256;

	for (int i = base; i < count; ++i)
		ImGui::TextWrapped(messages[i].c_str());

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
}

void debugger::scroll_up()
{
	scrolling = true;
	scroll_dir = -1;
}

void debugger::scroll_down()
{
	scrolling = true;
	scroll_dir = 1;
}

void debugger::error_callback(script* s, const std::string& err)
{
	g_debugger->add_client_msg(err.substr(0, err.find('\n')));
}