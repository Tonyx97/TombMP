#pragma once

enum eKeyID
{
	KEY_UNKNOWN = -1,
	KEY_LMOUSE = VK_LBUTTON,
	KEY_RMOUSE = VK_RBUTTON,
	KEY_MMOUSE = VK_MBUTTON,
	KEY_MOUSEX4 = VK_XBUTTON1,
	KEY_MOUSEX5 = VK_XBUTTON2,
	KEY_SPACE = VK_SPACE,
	KEY_COMMA = VK_OEM_COMMA,
	KEY_MINUS = VK_OEM_MINUS,
	KEY_PERIOD = VK_OEM_PERIOD,
	KEY_SLASH = VK_OEM_102,
	KEY_SEMICOLON = VK_OEM_1,
	KEY_0 = 0x30,
	KEY_1 = 0x31,
	KEY_2 = 0x32,
	KEY_3 = 0x33,
	KEY_4 = 0x34,
	KEY_5 = 0x35,
	KEY_6 = 0x36,
	KEY_7 = 0x37,
	KEY_8 = 0x38,
	KEY_9 = 0x39,
	KEY_A = 0x41,
	KEY_B = 0x42,
	KEY_C = 0x43,
	KEY_D = 0x44,
	KEY_E = 0x45,
	KEY_F = 0x46,
	KEY_G = 0x47,
	KEY_H = 0x48,
	KEY_I = 0x49,
	KEY_J = 0x4A,
	KEY_K = 0x4B,
	KEY_L = 0x4C,
	KEY_M = 0x4D,
	KEY_N = 0x4E,
	KEY_O = 0x4F,
	KEY_P = 0x50,
	KEY_Q = 0x51,
	KEY_R = 0x52,
	KEY_S = 0x53,
	KEY_T = 0x54,
	KEY_U = 0x55,
	KEY_V = 0x56,
	KEY_W = 0x57,
	KEY_X = 0x58,
	KEY_Y = 0x59,
	KEY_Z = 0x5A,
	KEY_NUM_0 = VK_NUMPAD0,
	KEY_NUM_1 = VK_NUMPAD1,
	KEY_NUM_2 = VK_NUMPAD2,
	KEY_NUM_3 = VK_NUMPAD3,
	KEY_NUM_4 = VK_NUMPAD4,
	KEY_NUM_5 = VK_NUMPAD5,
	KEY_NUM_6 = VK_NUMPAD6,
	KEY_NUM_7 = VK_NUMPAD7,
	KEY_NUM_8 = VK_NUMPAD8,
	KEY_NUM_9 = VK_NUMPAD9,
	KEY_NUM_ADD = VK_ADD,
	KEY_NUM_DECIMAL = VK_DECIMAL,
	KEY_NUM_DIVIDE = VK_DIVIDE,
	KEY_NUM_MULTIPLY = VK_MULTIPLY,
	KEY_NUM_SUBTRACT = VK_SUBTRACT,
	KEY_CAPSLOCK = VK_CAPITAL,
	KEY_DELETE = VK_DELETE,
	KEY_DOWN = VK_DOWN,
	KEY_END = VK_END,
	KEY_ENTER = VK_RETURN,
	KEY_ESCAPE = VK_ESCAPE,
	KEY_F1 = VK_F1,
	KEY_F2 = VK_F2,
	KEY_F3 = VK_F3,
	KEY_F4 = VK_F4,
	KEY_F5 = VK_F5,
	KEY_F6 = VK_F6,
	KEY_F7 = VK_F7,
	KEY_F8 = VK_F8,
	KEY_F9 = VK_F9,
	KEY_F10 = VK_F10,
	KEY_F11 = VK_F11,
	KEY_F12 = VK_F12,
	KEY_HOME = VK_HOME,
	KEY_INSERT = VK_INSERT,
	KEY_LEFT = VK_LEFT,
	KEY_ALT = VK_MENU,
	KEY_SHIFT = VK_SHIFT,
	KEY_CONTROL = VK_CONTROL,
	KEY_LEFT_ALT = VK_LMENU,
	KEY_LEFT_CONTROL = VK_LCONTROL,
	KEY_LEFT_SHIFT = VK_LSHIFT,
	KEY_LEFT_SUPER = VK_LWIN,
	KEY_MENU = VK_MENU,
	KEY_NUM_LOCK = VK_NUMLOCK,
	KEY_PAGE_DOWN = VK_NEXT,
	KEY_PAGE_UP = VK_PRIOR,
	KEY_PAUSE = VK_PAUSE,
	KEY_PRINT_SCREEN = VK_PRINT,
	KEY_RIGHT = VK_RIGHT,
	KEY_RIGHT_ALT = VK_RMENU,
	KEY_RIGHT_CONTROL = VK_RCONTROL,
	KEY_RIGHT_SHIFT = VK_RSHIFT,
	KEY_RIGHT_SUPER = VK_RWIN,
	KEY_SCROLL_LOCK = VK_SCROLL,
	KEY_TAB = VK_TAB,
	KEY_UP = VK_UP,
	KEY_MAX = 512,
};

enum eKeyHash
{
	KEY_HASH_UNKNOWN = -1,
	KEY_HASH_LMOUSE = 0x9e6f3518,
	KEY_HASH_RMOUSE = 0x9de9a3c2,
	KEY_HASH_MMOUSE = 0x22597116,
	KEY_HASH_MOUSEX4 = 0x455c94fe,
	KEY_HASH_MOUSEX5 = 0xd107ab76,
	KEY_HASH_LARROW = 0x83629de4,
	KEY_HASH_RARROW = 0xc36b1e2,
	KEY_HASH_UARROW = 0x154ca516,
	KEY_HASH_DARROW = 0xc229daf5,
	KEY_HASH_0 = 0x3a87bef7,
	KEY_HASH_1 = 0x4a205d10,
	KEY_HASH_2 = 0xcff66792,
	KEY_HASH_3 = 0x9b97fdca,
	KEY_HASH_4 = 0x49c5cc,
	KEY_HASH_5 = 0x2826947d,
	KEY_HASH_6 = 0x3243278e,
	KEY_HASH_7 = 0xf6edaff4,
	KEY_HASH_8 = 0x27bf105a,
	KEY_HASH_9 = 0xf2eb258b,
	KEY_HASH_A = 0x55f63e9c,
	KEY_HASH_B = 0xc31517c4,
	KEY_HASH_C = 0x945e393b,
	KEY_HASH_D = 0x6709dd5f,
	KEY_HASH_E = 0x92ff3429,
	KEY_HASH_F = 0x6d6be80f,
	KEY_HASH_G = 0x574b3aae,
	KEY_HASH_H = 0x6dc0e5f5,
	KEY_HASH_I = 0x56623620,
	KEY_HASH_J = 0x41f70c4e,
	KEY_HASH_K = 0xe7345582,
	KEY_HASH_L = 0x7358ecc1,
	KEY_HASH_M = 0xf45a364,
	KEY_HASH_N = 0xfc31fc95,
	KEY_HASH_O = 0x5ff74286,
	KEY_HASH_P = 0x61484448,
	KEY_HASH_Q = 0x81c6043b,
	KEY_HASH_R = 0xd4f0294a,
	KEY_HASH_S = 0x18e6b032,
	KEY_HASH_T = 0x7a7f722a,
	KEY_HASH_U = 0x4afc922d,
	KEY_HASH_V = 0x1c8133ef,
	KEY_HASH_W = 0x5da5b557,
	KEY_HASH_X = 0xc80904b0,
	KEY_HASH_Y = 0xfe55f4a1,
	KEY_HASH_Z = 0xe188e7e,
	KEY_HASH_NUM_0 = 0x7ffe40c8,
	KEY_HASH_NUM_1 = 0x23a47275,
	KEY_HASH_NUM_2 = 0xa3a97149,
	KEY_HASH_NUM_3 = 0x22956e07,
	KEY_HASH_NUM_4 = 0xe3d96f70,
	KEY_HASH_NUM_5 = 0xd88e5796,
	KEY_HASH_NUM_6 = 0xca553a04,
	KEY_HASH_NUM_7 = 0xd222c88f,
	KEY_HASH_NUM_8 = 0xe5816e2c,
	KEY_HASH_NUM_9 = 0x7bca99c0,
	KEY_HASH_NUM_MUL = 0xd41f882d,
	KEY_HASH_NUM_ADD = 0xabdfedb4,
	//KEY_HASH_NUM_SEP = 0xd4fed1c3,
	KEY_HASH_NUM_SUB = 0xd43e9bb5,
	KEY_HASH_NUM_DIV = 0x813bba62,
	KEY_HASH_NUM_DEC = 0x38060f1f,
	KEY_HASH_NUM_ENTER = 0x4ff00921,
	KEY_HASH_F1 = 0xe69012c1,
	KEY_HASH_F2 = 0xc3e3a395,
	KEY_HASH_F3 = 0x51abbdff,
	KEY_HASH_F4 = 0x38d498d9,
	KEY_HASH_F5 = 0xa7cbf58e,
	KEY_HASH_F6 = 0x281c7055,
	KEY_HASH_F7 = 0x7409070d,
	KEY_HASH_F8 = 0x9112bb9c,
	KEY_HASH_F9 = 0xe6a06562,
	KEY_HASH_F10 = 0x2801b18,
	KEY_HASH_F11 = 0x192e40b8,
	KEY_HASH_F12 = 0x22b9cb8b,
	KEY_HASH_ESCAPE = 0x2bf53fc4,
	KEY_HASH_BACKSPACE = 0x48a63d90,
	KEY_HASH_TAB = 0x1a84773a,
	KEY_HASH_LALT = 0xaab51a4c,
	KEY_HASH_RALT = 0xf351305,
	KEY_HASH_ENTER = 0x8eaeb98d,
	KEY_HASH_SPACE = 0x4bb67749,
	KEY_HASH_PGUP = 0x6e2510a5,
	KEY_HASH_PGDN = 0xe52be18e,
	KEY_HASH_END = 0x34a7ac65,
	KEY_HASH_HOME = 0xbd6a0bec,
	KEY_HASH_INSERT = 0x7de214f0,
	KEY_HASH_DELETE = 0x69b09287,
	KEY_HASH_LSHIFT = 0x202256af,
	KEY_HASH_RSHIFT = 0x97d4d266,
	KEY_HASH_LCTRL = 0x878a7dca,
	KEY_HASH_RCTRL = 0x70627971,
	KEY_HASH_LBRACKET = 0xbe1794b9,
	KEY_HASH_RBRACKET = 0x53163d8c,
	KEY_HASH_PAUSE = 0x78d4fff8,
	KEY_HASH_CAPSLOCK = 0xe3f9a801,
	KEY_HASH_SCROLL = 0x7535fcbb,
	KEY_HASH_SEMICOLON = 0xa8d98f29,
	KEY_HASH_COMMA = 0x8053cea2,
	KEY_HASH_DASH = 0xee8029d5,
	KEY_HASH_DOT = 0x5682f945,
	KEY_HASH_SLASH = 0xc140cd17,
	KEY_HASH_SHARP = 0x4d39f313,
	KEY_HASH_EQUAL = 0x329a1e5c,
	KEY_HASH_BACKSLASH = 0x3c2f10de,
};

class keycode
{
private:

	struct input_state
	{
		bool down = false,
			 pressed = false,
			 released = false;
	} keys_input[512] {};

	std::unordered_map<uint32_t, input_state*> named_keys_input;

	std::vector<bool*> pressed_keys,
					   released_keys;

	float mouse_wheel = 0.f;

	bool named_keys_enabled = false;

public:

	keycode();
	~keycode() {}

	void debug();

	void clear_per_frame_states();
	void set_key_input(uint32_t key, bool state);
	void set_named_keys_feature_enabled(bool val)	{ named_keys_enabled = val; }
	void set_mouse_wheel_value(float val)			{ mouse_wheel = val; }

	bool is_key_down(uint32_t key) const;
	bool is_key_pressed(uint32_t key) const;
	bool is_key_released(uint32_t key) const;

	bool is_key_down(const char* key);
	bool is_key_pressed(const char* key);
	bool is_key_released(const char* key);

	uint32_t get_key_hash_from_key_id(uint32_t key);

	float get_mouse_wheel_value() const				{ return mouse_wheel; }
};

inline std::unique_ptr<keycode> g_keycode;