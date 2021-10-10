import utils;

#include <shared/defs.h>

#include <shared/scripting/resource_system.h>

#include "keycode.h"

keycode::keycode()
{
	pressed_keys.reserve(KEY_MAX);
	released_keys.reserve(KEY_MAX);
	named_keys_input.reserve(KEY_MAX);
}

void keycode::debug()
{

}

void keycode::clear_per_frame_states()
{
	for (const auto& pressed_key_state : pressed_keys)	 *pressed_key_state = false;
	for (const auto& released_key_state : released_keys) *released_key_state = false;

	pressed_keys.clear();
	released_keys.clear();

	mouse_wheel = 0.f;
}

void keycode::set_key_input(uint32_t key, bool state)
{
	auto& key_input = keys_input[key];

	if (!key_input.down && state)
	{
		key_input.pressed = true;

		pressed_keys.push_back(&key_input.pressed);
	}
	else if (key_input.down && !state)
	{
		key_input.released = true;

		released_keys.push_back(&key_input.released);
	}

	g_resource->call_bind(key, key_input.down = state);

	if (named_keys_enabled)
		if (const auto key_hash = get_key_hash_from_key_id(key); key_hash != KEY_HASH_UNKNOWN)
			named_keys_input[key_hash] = &key_input;
}

bool keycode::is_key_down(uint32_t key) const
{
	return keys_input[key].down;
}

bool keycode::is_key_pressed(uint32_t key) const
{
	return keys_input[key].pressed;
}

bool keycode::is_key_released(uint32_t key) const
{
	return keys_input[key].released;
}

bool keycode::is_key_down(const char* key)
{
	if (const auto it = named_keys_input.find(utils::hash::JENKINS(key)); it != named_keys_input.end())
		return it->second->down;

	return false;
}

bool keycode::is_key_pressed(const char* key)
{
	if (const auto it = named_keys_input.find(utils::hash::JENKINS(key)); it != named_keys_input.end())
		return it->second->pressed;

	return false;
}

bool keycode::is_key_released(const char* key)
{
	if (const auto it = named_keys_input.find(utils::hash::JENKINS(key)); it != named_keys_input.end())
		return it->second->released;

	return false;
}

uint32_t keycode::get_key_hash_from_key_id(uint32_t key)
{
	switch (key)
	{
	case KEY_LMOUSE:		return KEY_HASH_LMOUSE;
	case KEY_RMOUSE:		return KEY_HASH_RMOUSE;
	case KEY_MMOUSE:		return KEY_HASH_MMOUSE;
	case KEY_MOUSEX4:		return KEY_HASH_MOUSEX4;
	case KEY_MOUSEX5:		return KEY_HASH_MOUSEX5;
	case KEY_LEFT:			return KEY_HASH_LARROW;
	case KEY_RIGHT:			return KEY_HASH_RARROW;
	case KEY_UP:			return KEY_HASH_UARROW;
	case KEY_DOWN:			return KEY_HASH_DARROW;
	case KEY_0:				return KEY_HASH_0;
	case KEY_1:				return KEY_HASH_1;
	case KEY_2:				return KEY_HASH_2;
	case KEY_3:				return KEY_HASH_3;
	case KEY_4:				return KEY_HASH_4;
	case KEY_5:				return KEY_HASH_5;
	case KEY_6:				return KEY_HASH_6;
	case KEY_7:				return KEY_HASH_7;
	case KEY_8:				return KEY_HASH_8;
	case KEY_9:				return KEY_HASH_9;
	case KEY_A:				return KEY_HASH_A;
	case KEY_B:				return KEY_HASH_B;
	case KEY_C:				return KEY_HASH_C;
	case KEY_D:				return KEY_HASH_D;
	case KEY_E:				return KEY_HASH_E;
	case KEY_F:				return KEY_HASH_F;
	case KEY_G:				return KEY_HASH_G;
	case KEY_H:				return KEY_HASH_H;
	case KEY_I:				return KEY_HASH_I;
	case KEY_J:				return KEY_HASH_J;
	case KEY_K:				return KEY_HASH_K;
	case KEY_L:				return KEY_HASH_L;
	case KEY_M:				return KEY_HASH_M;
	case KEY_N:				return KEY_HASH_N;
	case KEY_O:				return KEY_HASH_O;
	case KEY_P:				return KEY_HASH_P;
	case KEY_Q:				return KEY_HASH_Q;
	case KEY_R:				return KEY_HASH_R;
	case KEY_S:				return KEY_HASH_S;
	case KEY_T:				return KEY_HASH_T;
	case KEY_U:				return KEY_HASH_U;
	case KEY_V:				return KEY_HASH_V;
	case KEY_W:				return KEY_HASH_W;
	case KEY_X:				return KEY_HASH_X;
	case KEY_Y:				return KEY_HASH_Y;
	case KEY_Z:				return KEY_HASH_Z;
	case KEY_NUM_0:			return KEY_HASH_NUM_0;
	case KEY_NUM_1:			return KEY_HASH_NUM_1;
	case KEY_NUM_2:			return KEY_HASH_NUM_2;
	case KEY_NUM_3:			return KEY_HASH_NUM_3;
	case KEY_NUM_4:			return KEY_HASH_NUM_4;
	case KEY_NUM_5:			return KEY_HASH_NUM_5;
	case KEY_NUM_6:			return KEY_HASH_NUM_6;
	case KEY_NUM_7:			return KEY_HASH_NUM_7;
	case KEY_NUM_8:			return KEY_HASH_NUM_8;
	case KEY_NUM_9:			return KEY_HASH_NUM_9;
	case KEY_NUM_MULTIPLY:	return KEY_HASH_NUM_MUL;
	case KEY_NUM_ADD:		return KEY_HASH_NUM_ADD;
	case KEY_NUM_DECIMAL:	return KEY_HASH_NUM_DEC;
	case KEY_NUM_SUBTRACT:	return KEY_HASH_NUM_SUB;
	case KEY_NUM_DIVIDE:	return KEY_HASH_NUM_DIV;
	//case KEY_NUM_ENTER:		return KEY_HASH_NUM_ENTER;
	case KEY_F1:			return KEY_HASH_F1;
	case KEY_F2:			return KEY_HASH_F2;
	case KEY_F3:			return KEY_HASH_F3;
	case KEY_F4:			return KEY_HASH_F4;
	case KEY_F5:			return KEY_HASH_F5;
	case KEY_F6:			return KEY_HASH_F6;
	case KEY_F7:			return KEY_HASH_F7;
	case KEY_F8:			return KEY_HASH_F8;
	case KEY_F9:			return KEY_HASH_F9;
	case KEY_F10:			return KEY_HASH_F10;
	case KEY_F11:			return KEY_HASH_F11;
	case KEY_F12:			return KEY_HASH_F12;
	case KEY_ESCAPE:		return KEY_HASH_ESCAPE;
	//case KEY_BACKSPACE:		return KEY_HASH_BACKSPACE;
	case KEY_TAB:			return KEY_HASH_TAB;
	case KEY_LEFT_ALT:		return KEY_HASH_LALT;
	case KEY_RIGHT_ALT:		return KEY_HASH_RALT;
	case KEY_ENTER:			return KEY_HASH_ENTER;
	case KEY_SPACE:			return KEY_HASH_SPACE;
	case KEY_PAGE_UP:		return KEY_HASH_PGUP;
	case KEY_PAGE_DOWN:		return KEY_HASH_PGDN;
	//case KEY_END:			return KEY_HASH_END;
	case KEY_HOME:			return KEY_HASH_HOME;
	case KEY_INSERT:		return KEY_HASH_INSERT;
	//case KEY_DELETE:		return KEY_HASH_DELETE;
	case KEY_LEFT_SHIFT:	return KEY_HASH_LSHIFT;
	case KEY_RIGHT_SHIFT:	return KEY_HASH_RSHIFT;
	case KEY_LEFT_CONTROL:	return KEY_HASH_LCTRL;
	case KEY_RIGHT_CONTROL:	return KEY_HASH_RCTRL;
	//case KEY_LEFT_BRACKET:	return KEY_HASH_LBRACKET;
	//case KEY_RIGHT_BRACKET:	return KEY_HASH_RBRACKET;
	case KEY_PAUSE:			return KEY_HASH_PAUSE;
	//case KEY_CAPSLOCK:		return KEY_HASH_CAPSLOCK;
	case KEY_SCROLL_LOCK:	return KEY_HASH_SCROLL;
	//case KEY_SEMICOLON:		return KEY_HASH_SEMICOLON;
	case KEY_COMMA:			return KEY_HASH_COMMA;
	case KEY_MINUS:			return KEY_HASH_DASH;
	case KEY_PERIOD:		return KEY_HASH_DOT;
	case KEY_SLASH:			return KEY_HASH_SLASH;
	//case KEY_EQUAL:			return KEY_HASH_EQUAL;
	//case KEY_BACKSLASH:		return KEY_HASH_BACKSLASH;
	}

	return KEY_HASH_UNKNOWN;
}