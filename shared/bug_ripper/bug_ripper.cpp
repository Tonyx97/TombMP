import utils;
import prof;

#include <shared/defs.h>

#include "bug_ripper.h"

#include <DbgHelp.h>

#ifdef GAME_CLIENT
#include <window/window.h>
#endif

#define IDD_CRASH_DIALOG            114
#define IDC_SEND_DESC				9701
#define IDC_SEND_QUESTION			9702
#define IDC_INFO_EDIT				9703
#define IDC_CRASH_HEAD				9704

#pragma comment (lib, "dbghelp.lib")
#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

INT_PTR __stdcall crash_wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
	switch (msg)
	{
	case WM_COMMAND:
	{
		switch (auto param = LOWORD(w_param))
		{
		case IDOK:
		case IDCANCEL:
			g_bug_ripper->close_crash_wnd(); return 1;
		}
	}
	}

	return 0;
}

long __stdcall global_veh(_EXCEPTION_POINTERS* ep)
{
	return g_bug_ripper->show_and_dump_crash(ep);
}

bool bug_ripper::init()
{
#ifdef _DEBUG
	return true;
#else
	
	if (AddVectoredExceptionHandler(1, global_veh))
	{
		game_base = uintptr_t(GetModuleHandle(nullptr));

		utils::mem::for_each_module(GetCurrentProcessId(), [&](uintptr_t base_addr, uint32_t size, const char* name)
		{
			if (base_addr == game_base)
			{
				game_size = size;
				return true;
			}

			return false;
		}, {});

		return true;
	}

	return false;
#endif
}

bool bug_ripper::destroy()
{
#ifdef _DEBUG
	return true;
#else
	return RemoveVectoredExceptionHandler(global_veh);
#endif
}

long bug_ripper::show_and_dump_crash(_EXCEPTION_POINTERS* ep)
{
	uintptr_t eip = uintptr_t(ep->ExceptionRecord->ExceptionAddress);

	if (eip < game_base || eip >= game_base + game_size)
		return EXCEPTION_CONTINUE_SEARCH;

	char mod_name[256] { 0 };

	if (auto mod_base = get_module_info_if_valid(eip, mod_name))
	{
		auto dialog = CreateDialogW(nullptr, MAKEINTRESOURCEW(IDD_CRASH_DIALOG), nullptr, crash_wnd_proc);

#ifdef GAME_CLIENT
		SendMessageW(dialog, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon((HINSTANCE)g_window->get_instance(), "GAME_ICON"));
#endif
		ShowCursor(true);

		RECT rect;

		GetWindowRect(dialog, &rect);

		const int width = rect.right - rect.left;
		const int height = rect.bottom - rect.top;

		SetWindowText(dialog, "CE Bug Ripper");
		SetWindowPos(dialog, HWND_TOP, (GetSystemMetrics(SM_CXSCREEN) - width) / 2, (GetSystemMetrics(SM_CYSCREEN) - height) / 2, width, height, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		BringWindowToTop(dialog);

		std::string buffer = "The game crashed, we need to gather information about this crash.\r\nPlease send us report so we can fix the issue as soon as possible.\r\nMore Information about this crash in game's logs.\r\n\r\n";

		std::ofstream log("CRASH DUMP " + utils::time::get_str_date('-', '.', ' ') + ".log", std::ios::trunc);

		log << "- Exception Code: " << std::hex << ep->ExceptionRecord->ExceptionCode << std::endl;
		log << "- GP Registers:" << std::endl;
		log << "\tEip: " << mod_name << " + 0x" << std::hex << eip << std::endl;
		log << "\tEbp: 0x" << std::hex << ep->ContextRecord->Ebp << std::endl;
		log << "\tEax: 0x" << std::hex << ep->ContextRecord->Eax << std::endl;
		log << "\tEbx: 0x" << std::hex << ep->ContextRecord->Ebx << std::endl;
		log << "\tEcx: 0x" << std::hex << ep->ContextRecord->Ecx << std::endl;
		log << "\tEdx: 0x" << std::hex << ep->ContextRecord->Edx << std::endl;
		log << "\tEsp: 0x" << std::hex << ep->ContextRecord->Esp << std::endl;
		log << "\tEsi: 0x" << std::hex << ep->ContextRecord->Esi << std::endl;
		log << "\tEdi: 0x" << std::hex << ep->ContextRecord->Edi << std::endl;
		log << "- Debug Registers:" << std::endl;
		log << "\tContextFlags: 0x" << std::hex << ep->ContextRecord->ContextFlags << std::endl;
		log << "\tDR0: 0x" << std::hex << ep->ContextRecord->Dr0 << std::endl;
		log << "\tDR1: 0x" << std::hex << ep->ContextRecord->Dr1 << std::endl;
		log << "\tDR2: 0x" << std::hex << ep->ContextRecord->Dr2 << std::endl;
		log << "\tDR3: 0x" << std::hex << ep->ContextRecord->Dr3 << std::endl;
		log << "\tDR6: 0x" << std::hex << ep->ContextRecord->Dr6 << std::endl;
		log << "\tDR7: 0x" << std::hex << ep->ContextRecord->Dr7 << std::endl;
		log << "- Segment Registers & CPU flags:" << std::endl;
		log << "\tSegCs: 0x" << std::hex << ep->ContextRecord->SegCs << std::endl;
		log << "\tSegDs: 0x" << std::hex << ep->ContextRecord->SegDs << std::endl;
		log << "\tSegEs: 0x" << std::hex << ep->ContextRecord->SegEs << std::endl;
		log << "\tSegFs: 0x" << std::hex << ep->ContextRecord->SegFs << std::endl;
		log << "\tSegGs: 0x" << std::hex << ep->ContextRecord->SegGs << std::endl;
		log << "\tSegSs: 0x" << std::hex << ep->ContextRecord->SegSs << std::endl;
		log << "\tEFlags: 0x" << std::hex << ep->ContextRecord->EFlags << std::endl;
		log << "- Stack: " << std::hex << ep->ContextRecord->Esp << std::endl;

		for (int i = 0; i < 0x300; i += 0x4)
		{
			auto read_val = *(uintptr_t*)(ep->ContextRecord->Esp + i);

			if (read_val >= game_base && read_val < game_base + game_size)
				log << "\t*0x" << std::hex << ep->ContextRecord->Esp + i << ": 0x" << std::hex << read_val - game_base << std::endl;
		}

		log.close();

		buffer += std::format("Exception Address: {} + {:#x}", mod_name, eip);

		SetWindowText(GetDlgItem(dialog, IDC_INFO_EDIT), buffer.c_str());

		while (!can_close_crash_wnd)
		{
			if (MSG msg; PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) && GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		DestroyWindow(dialog);

		std::exit(0);
	}

	prof::print(RED, "Continuing exception search...");

	return EXCEPTION_CONTINUE_SEARCH;
}

uintptr_t bug_ripper::get_module_info_if_valid(uintptr_t& addr, char* module_name)
{
	uintptr_t mod_base = 0;
	
	utils::mem::for_each_module(GetCurrentProcessId(), [&addr, module_name, &mod_base](uintptr_t base_addr, uint32_t size, const char* name)
	{
		if (addr >= base_addr && addr < base_addr + size)
		{
			strcpy(module_name, name);
			mod_base = base_addr;
			addr -= base_addr;
			return true;
		}

		return false;
	}, {});

	return mod_base;
}