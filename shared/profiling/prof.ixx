module;

#include <shared/defs.h>

export module prof;

import utils;

#define FORMAT(t, a) std::vformat(t, std::make_format_args(a...))
#define FORMATV(t, ...) std::vformat(t, std::make_format_args(__VA_ARGS__))

export
{
	enum eColor : unsigned short
	{
		BLACK = 0x0,
		DARK_BLUE = 0x1,
		DARK_GREEN = 0x2,
		DARK_CYAN = 0x3,
		DARK_RED = 0x4,
		DARK_PURPLE = 0x5,
		DARK_YELLOW = 0x6,
		GREY = 0x7,
		DARK_GREY = 0x8,
		BLUE = 0x9,
		GREEN = 0xA,
		CYAN = 0xB,
		RED = 0xC,
		PURPLE = 0xD,
		YELLOW = 0xE,
		WHITE = 0xF,
	};

	namespace prof
	{
#if defined(_DEBUG) || (!defined(GAME_CLIENT) || defined(_LD))
		HANDLE console_handle = nullptr;
		HWND console_hwnd = nullptr;
		bool console_allocated = false;

		void open_console(const char* name)
		{
			if (!console_allocated)
			{
				FILE* temp;

				AllocConsole();
				freopen_s(&temp, "CONOUT$", "w", stdout);
				freopen_s(&temp, "CONIN$", "r", stdin);
				SetConsoleTitleA(name);
				SetWindowPos(console_hwnd, 0, (GetSystemMetrics(SM_CXSCREEN) - 1000) / 2, (GetSystemMetrics(SM_CYSCREEN) - 650) / 2, 1000, 650, 0);
				console_allocated = true;
			}

			ShowWindow(console_hwnd, SW_SHOW);
		}

		void init(const char* name)
		{
			if (console_allocated)
				return;

			open_console(name);

			console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
			console_hwnd = GetConsoleWindow();
		}

		void close_console(bool free_console)
		{
			if (!console_allocated)
				return;

			ShowWindow(console_hwnd, SW_HIDE);

			if (free_console)
				FreeConsole();

			console_allocated = false;
		}

		bool dbg_file_internal(const char* name, const char* text)
		{
			if (auto debug = std::ofstream(name, std::ios::app))
				debug << "***** [] *****" << std::endl << text << std::endl << std::endl;

			return false;
		}

		bool dbg_mbox_internal(uint32_t type, const char* msg_box_text)
		{
			MessageBoxA(nullptr, msg_box_text, "TombMP", type);
			return false;
		}

		void print_internal(bool nl, bool show_time, eColor color, const std::string& text)
		{
			SetConsoleTextAttribute(console_handle, color);

			auto time_str = utils::time::get_str_time(':');

			if (show_time)
			{
				if (nl) printf_s("[%s] %s\n", time_str.c_str(), text.c_str());
				else	printf_s("[%s] %s", time_str.c_str(), text.c_str());
			}
			else
			{
				if (nl) printf_s("%s\n", text.c_str());
				else	printf_s("%s", text.c_str());
			}
		}

		void print_info_internal(bool nl, const char* filename, int line, eColor color, const char* text)
		{
			std::string file_str(filename);

			auto real_pos = file_str.substr(0, file_str.find_last_of('\\') - 1).find_last_of('\\') + 1;
			auto fixed_filename = file_str.substr(real_pos, file_str.length() - real_pos);

			SetConsoleTextAttribute(console_handle, WHITE);		printf_s("[Line ");
			SetConsoleTextAttribute(console_handle, CYAN);		printf_s("%i", line);
			SetConsoleTextAttribute(console_handle, WHITE);		printf_s(": ");
			SetConsoleTextAttribute(console_handle, PURPLE);	printf_s("%s", fixed_filename.c_str());
			SetConsoleTextAttribute(console_handle, WHITE);		printf_s("] ");
			SetConsoleTextAttribute(console_handle, color);

			print_internal(nl, false, color, text);
		}

		void critical_error_internal(const std::string& text)
		{
			MessageBoxA(nullptr, text.c_str(), "TombMP", MB_OK | MB_ICONERROR);

			exit(EXIT_FAILURE);
		}

		template <typename... A>
		void print(bool nl, eColor color, const char* text, A... args)
		{
			print_internal(nl, false, color, FORMAT(text, args));
		}

		template <typename... A>
		void print(eColor color, const char* text, A... args)
		{
			print_internal(true, false, color, FORMAT(text, args));
		}

		template <typename... A>
		void printt(eColor color, const char* text, A... args)
		{
			print_internal(true, true, color, FORMAT(text, args));
		}

		template <typename... A>
		void print_nl(eColor color, const char* text, A... args)
		{
			print_internal(false, false, color, FORMAT(text, args));
		}

		template <typename... A>
		void print_info(bool nl, const char* filename, int line, eColor color, const char* text, A... args)
		{
			print_info_internal(nl, filename, line, color, FORMAT(text, args));
		}

		template <typename... A>
		void critical_error(const char* text, A... args)
		{
			critical_error_internal(FORMAT(text, args));
		}
#else
		void open_console(const char* name) {}
		void init(const char* name) {}
		void close_console(bool free_console) {}
		bool dbg_file_internal(const char* name, const char* text) { return true; }
		bool dbg_mbox_internal(uint32_t type, const char* msg_box_text) { return true; }
		void print_internal(bool nl, bool show_time, eColor color, const std::string& text) {}
		void print_info_internal(bool nl, const char* filename, int line, eColor color, const char* text) {}
		void critical_error_internal(const std::string& text) {}

		template <typename... A> void print(bool nl, eColor color, const char* text, A... args) {}
		template <typename... A> void print(eColor color, const char* text, A... args) {}
		template <typename... A> void printt(eColor color, const char* text, A... args) {}
		template <typename... A> void print_nl(eColor color, const char* text, A... args) {}
		template <typename... A> void print_info(bool nl, const char* filename, int line, eColor color, const char* text, A... args) {}
		template <typename... A> void critical_error(const char* text, A... args) {}
#endif
	}

	struct TimeProfiling
	{
		std::chrono::high_resolution_clock::time_point m_start;
		uint64_t cycles = 0;
		char name[128] = { 0 };

		TimeProfiling(const char* name)
		{
			strcpy_s(this->name, name);
			m_start = std::chrono::high_resolution_clock::now();
			cycles = __rdtsc();
		}

		~TimeProfiling()
		{
			const auto cycles_passed = __rdtsc() - cycles;
			const auto time_passed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_start).count();
			prof::print(true, YELLOW, "%s: %.3f ms | %i mcs | %i cycles", name, static_cast<double>(time_passed) / 1000.f, time_passed, cycles_passed);
		}
	};
}