import prof;

#include <specific/standard.h>
#include <specific/global.h>

#include <mp/chat.h>
#include <keycode/keycode.h>

void move_cb(int x, int y)
{
	//g_engine->update_window_position(x, y);
}

void resize_cb(int width, int height)
{
	prof::print(CYAN, "[1] Resize to ({}, {})", width, height);
	setup_screen_size(width, height);
}

void framebuffer_cb(int width, int height)
{
	prof::print(CYAN, "[2] Resize to ({}, {})", width, height);
	setup_screen_size(width, height);
}

void kb_cb(int scancode, int action, int mods)
{
	g_keycode->set_key_input(scancode, action != 0);
	g_chat->key_input(scancode, action != 0);

	if (mods != 0)
		g_keycode->set_key_input(mods, action != 0);

	//PRINT(DBG_CYAN, "[kb_cb] %i %i %i", scancode, action, mods);
}

void chars_cb(unsigned int codepoint, int mods)
{
	if (codepoint == 8)
		g_chat->remove_char();
	else if (codepoint == 127)
		g_chat->remove_word();
	else if (codepoint == 22)
		g_chat->paste_text();
	else if (codepoint > 0x1F)
		g_chat->add_char(codepoint);

	//PRINT(DBG_CYAN, "[chars_cb] %i %i", codepoint, mods);
}

void cursor_pos_cb(double x, double y)
{
	auto x_float = static_cast<float>(x),
		 y_float = static_cast<float>(y);

	//g_engine->set_cursor_pos({ x_float, y_float });
}

void focus_cb(int focus)
{
}

void mouse_btn_cb(int button, int action, int mods)
{
	if (button > KEY_MAX)
		return;

	g_keycode->set_key_input(button, action != 0);
}

void scroll_cb(double y)
{
	g_keycode->set_mouse_wheel_value(static_cast<float>(y));
}