#include <shared/defs.h>
#include <shared/timer_system/timer_system.h>

#include <shared/scripting/script.h>

#include <sol/sol.hpp>

#include "shf_defs.h"

void shf_util::register_functions(sol::state* vm)
{
	vm->set_function("setTimer", [&](sol::this_state s, const sol::function& fn, int interval, int times, sol::variadic_args va)
	{
		std::vector<sol::object> args;

		for (const auto& arg : va)
			args.push_back(arg);

		return g_timer->add_timer(script::get_global_script(s), fn, std::move(args), interval, times);
	});

	vm->set_function("destroyTimer", [&](timer* t)
	{
		if (!t || !t->get_owner())
			return false;

		bool destroyed = false;

		g_timer->destroy_timer(t, &destroyed);

		return destroyed;
	});

	vm->set_function("isTimer",		[&](timer* t) { return g_timer->is_timer(t); });
	vm->set_function("resetTimer",  [&](timer* t) { return t->reset(); });
	
	vm->set_function("getTimerInfo", [&](timer* t) -> std::tuple<int, int, int, int>
	{
		return { t->get_interval_left(), t->get_times_remaining(), t->get_interval(), t->get_times() };
	});
}