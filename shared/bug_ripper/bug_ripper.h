#pragma once

class bug_ripper
{
private:

	uintptr_t game_base = 0;

	uint32_t game_size = 0;

	bool can_close_crash_wnd = false;

public:

	bug_ripper()			{}
	~bug_ripper()			{}

	bool init();
	bool destroy();

	void close_crash_wnd()	{ can_close_crash_wnd = true; }

	long show_and_dump_crash(struct _EXCEPTION_POINTERS* ep);

	uintptr_t get_module_info_if_valid(uintptr_t& addr, char* module_name);
};

inline std::unique_ptr<bug_ripper> g_bug_ripper;