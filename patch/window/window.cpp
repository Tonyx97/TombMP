#include <specific/standard.h>

#include <keycode/keycode.h>

#include "window.h"
#include "callbacks.h"

void window::dispatch_key(int key, bool pressed, win_key_info ki)
{
	//PRINT(DBG_RED, "[%s] %i (%i | %i | %i | %i | %i | %i)", pressed ? "WM_KEYDOWN" : "WM_KEYUP", key, ki.repeat_count, ki.scan_code, ki.extended, ki.ctx_code, ki.previous_state, ki.transition_state);

	switch (key)
	{
	case VK_SHIFT:   return kb_cb(ki.scan_code == 42 ? VK_RSHIFT : VK_LSHIFT, pressed, key);
	case VK_CONTROL: return kb_cb(ki.extended ? VK_RCONTROL : VK_LCONTROL, pressed, key);
	case VK_MENU:	 return kb_cb(ki.extended ? VK_RMENU : VK_LMENU, pressed, key);
	}

	kb_cb(key, pressed, 0);
}

long __stdcall window::wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == 0x112 && wParam == 0xf100)
		return 0;

	win_key_info ki = { (uint32_t)lParam };

	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		int button = 0;

		bool pressed = (msg == WM_LBUTTONDOWN ||
						msg == WM_RBUTTONDOWN ||
						msg == WM_MBUTTONDOWN ||
						msg == WM_XBUTTONDOWN);

		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) button = VK_LBUTTON;
		if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) button = VK_RBUTTON;
		if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) button = VK_MBUTTON;
		if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
		
		kb_cb(button, pressed, 0);

		break;
	}
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		dispatch_key(int(wParam), true, ki);
		break;
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		dispatch_key(int(wParam), false, ki);
		break;
	}
	case WM_MOUSEWHEEL:
		scroll_cb(float(GET_WHEEL_DELTA_WPARAM(wParam)) / float(WHEEL_DELTA));
		break;
	case WM_MOUSEHWHEEL:
		scroll_cb(-float(GET_WHEEL_DELTA_WPARAM(wParam)) / float(WHEEL_DELTA));
		break;
	case WM_CHAR:
		//PRINT(DBG_RED, "[WM_CHAR] %i %i", wParam, ki.repeat_count, ki.scan_code, ki.extended, ki.ctx_code, ki.previous_state, ki.transition_state);
		chars_cb(static_cast<wchar_t>(wParam), 0);
		break;
	case WM_CREATE:
		ShowCursor(false);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	}

	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

bool window::init(void* instance, const char* name)
{
	this->instance = instance;

	if (auto voodoo_file = std::ifstream("dgVoodoo.conf"))
	{
		std::string line;

		bool checking_res = false;

		while (std::getline(voodoo_file, line))
		{
			if (checking_res)
			{
				if (line.find("Resolution") == 0)
				{
					std::regex sx_rgx("h:(\\d+)"),
							   sy_rgx("v:(\\d+)");

					std::smatch sm;

					if (std::regex_search(line, sm, sx_rgx)) sx = std::stoi(sm[1]);
					if (std::regex_search(line, sm, sy_rgx)) sy = std::stoi(sm[1]);
				}
			}
			else if (line.find("GameRealFSMode") == 1)
			{
				std::regex fs_rgx("GameRealFSMode(\\s)*=(\\s)*(true|false)");

				std::smatch sm;

				if (std::regex_search(line, sm, fs_rgx))
					fullscreen = !sm[3].compare("true");
			}
			else if (line.find("Aspect") == 1)
			{
				std::regex asp_rgx("Aspect(\\s)*=(\\s)*(\\d+):(\\d+)");

				std::smatch sm;

				if (std::regex_search(line, sm, asp_rgx))
				{
					aspect_x = std::stoi(sm[3].str());
					aspect_y = std::stoi(sm[4].str());
				}
			}
			else if (line.find("FOV") == 1)
			{
				std::regex fov_rgx("FOV(\\s)*=(\\s)*(\\d+)");

				std::smatch sm;

				if (std::regex_search(line, sm, fov_rgx))
					fov = std::stoi(sm[3].str());
			}
			else if (line.find("internal3D") != std::string::npos)
				checking_res = true;
		}
	}

	if (sx == -1 || sx == 0 ||
		sy == -1 || sy == 0)
		return false;

	SetProcessDPIAware();

	int width = sx,
		height = sy;

    DWORD style = WS_POPUP;

    if (!fullscreen)
    {
		RECT rect { 0, 0, sx, sy };

		AdjustWindowRect(&rect, style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, false);

		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
    }

	if (!reg_window_class())
		return false;

	return (win32_hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		WINDOW_CLASS(),
		name,
        style,
		GetSystemMetrics(SM_CXSCREEN) / 2 - sx / 2,
		GetSystemMetrics(SM_CYSCREEN) / 2 - sy / 2,
		width, height,
		nullptr,
		nullptr,
		(HINSTANCE)instance,
		nullptr));
}

void window::destroy()
{
}

bool window::reg_window_class()
{
	wnd_class.hIcon = LoadIcon((HINSTANCE)instance, "GAME_ICON");
	wnd_class.lpszMenuName = nullptr;
	wnd_class.lpszClassName = WINDOW_CLASS();
	wnd_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wnd_class.hInstance = (HINSTANCE)instance;
	wnd_class.style = CS_VREDRAW | CS_HREDRAW;
	wnd_class.lpfnWndProc = (WNDPROC)wnd_proc;
	wnd_class.cbClsExtra = 0;
	wnd_class.cbWndExtra = 0;

	return RegisterClass(&wnd_class);
}

void window::show()
{
	ShowWindow(win32_hwnd, SW_SHOW);
}

void window::poll_events()
{
	g_keycode->clear_per_frame_states();

	MSG msg;

	while (!should_close && PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_TIMER)
			continue;

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		should_close = (msg.message == WM_QUIT);
	}
}