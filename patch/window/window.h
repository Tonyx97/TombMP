#pragma once

#include <d3dtypes.h>

union win_key_info
{
	uint32_t info;

	struct
	{
		uint32_t repeat_count : 16;
		uint32_t scan_code : 8;
		uint32_t extended : 1;
		uint32_t reserved : 4;
		uint32_t ctx_code : 1;
		uint32_t previous_state : 1;
		uint32_t transition_state : 1;
	};
};

class window
{
private:

	WNDCLASSA wnd_class {};

	HWND win32_hwnd = nullptr;

	void* instance = nullptr;

	int sx = -1,
		sy = -1,
		aspect_x = 4,
		aspect_y = 3,
		fov = 80,
		filter = D3DFILTER_LINEAR,
		shade_mode = D3DSHADE_GOURAUD,
		fill_mode = D3DFILL_SOLID;

	bool fullscreen = false,
		 perspective = true,
		 dither = true,
		 should_close = false;

	static void dispatch_key(int key, bool pressed, win_key_info ki);
	static long __stdcall wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void destroy();

public:

	window()								{}
	~window()								{ destroy(); }

	bool init(void* instance, const char* name);
	bool reg_window_class();
	bool wants_to_close()					{ return should_close; }
	bool is_fullscreen() const				{ return fullscreen; }
	bool is_perspective_enabled() const		{ return perspective; }
	bool is_dither_enabled() const			{ return dither; }

	void show();
	void set_close()						{ should_close = true; }
	void set_fov(int v)						{ fov = v; }
	
	int get_fov() const						{ return fov; }
	int get_filter() const					{ return filter; }
	int get_shade_mode() const				{ return shade_mode; }
	int get_fill_mode() const				{ return fill_mode; }

	void* get_instance()					{ return instance; }
	HWND get_win32_handle()					{ return win32_hwnd; }

	std::tuple<int, int> get_resolution()	{ return { sx, sy }; }
	std::tuple<int, int> get_aspect()		{ return { aspect_x, aspect_y }; }

	void poll_events();

	static constexpr auto WINDOW_CLASS()	{ return "TRIIIOC"; }
};

inline std::unique_ptr<window> g_window;