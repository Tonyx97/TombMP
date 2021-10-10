#pragma once

#include <sol/sol.hpp>

#include <shared/scripting/obj_base.h>

class timer : public obj_base
{
private:

	sol::function fn;

	std::vector<sol::object> args;

	std::chrono::steady_clock::time_point last;

	int interval = 0,
		interval_left = 0,
		times = 0,
		times_remaining = 0;

public:

#ifdef GAME_CLIENT
	static constexpr auto INTERVAL_MIN() { return 50; }
#else
	static constexpr auto INTERVAL_MIN() { return 1; }
#endif

	timer(const sol::function& fn, const std::vector<sol::object>& args, int interval, int times);

	void reset();

	int get_interval() const		{ return interval; }
	int get_interval_left() const	{ return interval_left; }
	int get_times() const			{ return times; }
	int get_times_remaining() const { return times_remaining; }

	bool update();
};

class timer_system
{
private:

	std::unordered_set<timer*> timers;

public:

	using timer_it = decltype(timers)::iterator;

	timer* add_timer(class script* s, const sol::function& fn, const std::vector<sol::object>& args, int interval, int times);

	timer_it destroy_timer(timer* t, bool* destroyed = nullptr);

	bool is_timer(timer* t) const { return timers.contains(t); }

	void update();
};

inline std::unique_ptr<timer_system> g_timer;