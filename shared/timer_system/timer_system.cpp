#include <shared/defs.h>
#include <shared/scripting/script.h>

#include "timer_system.h"

timer::timer(const sol::function& fn, const std::vector<sol::object>& args, int interval, int times) : fn(fn), args(args), interval(interval), times(times), times_remaining(times)
{
	obj_type = OBJ_TYPE_TIMER;
	last = std::chrono::steady_clock::now();
}

void timer::reset()
{
	last = std::chrono::steady_clock::now();
	times_remaining = times;
	interval_left = interval;
}

bool timer::update()
{
	auto curr = std::chrono::steady_clock::now();

	if ((interval_left = int(std::chrono::duration_cast<std::chrono::milliseconds>(curr - last).count())) >= interval)
	{
		last = curr;

		--times_remaining;

		fn(sol::as_args(args));
	}

	return (times_remaining == 0);
}

timer* timer_system::add_timer(script* s, const sol::function& fn, const std::vector<sol::object>& args, int interval, int times)
{
	if (!s || interval < timer::INTERVAL_MIN() || times < 1)
		return nullptr;

	auto t = s->add_obj(new timer(fn, args, interval, times));

	timers.insert(t);

	return t;
}

timer_system::timer_it timer_system::destroy_timer(timer* t, bool* destroyed)
{
	if (auto it = timers.find(t); it != timers.end())
	{
		delete t;

		if (destroyed)
			*destroyed = true;

		return timers.erase(it);
	}

	if (destroyed)
		*destroyed = false;

	return timers.end();
}

void timer_system::update()
{
	for (auto it = timers.begin(); it != timers.end(); ++it)
		if (auto t = *it; t->update())
			if ((it = destroy_timer(t)) == timers.end())
				break;
}