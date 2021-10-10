#include <shared/defs.h>

#include "shf_defs.h"

#include "../resource.h"
#include "../resource_system.h"

void shf_resource::register_functions(sol::state* vm)
{
	vm->set_function("cancelEvent", [&]() { g_resource->cancel_event(); });

	vm->set_function("addEvent", [&](sol::this_state s, const char* name, const sol::function& fn, sol::variadic_args va)
	{
		return g_resource->add_event(name, fn, script::get_global_script(s), (va.size() == 1 && va[0].is<bool>() ? va[0].as<bool>() : false));
	});

	vm->set_function("removeEvent", [&](sol::this_state s, const char* name)
	{
		return g_resource->remove_event(name, script::get_global_script(s));
	});

	vm->set_function("triggerEvent", [&](sol::this_state s, const char* name, sol::variadic_args va)
	{
		return g_resource->trigger_non_remote_event(name, va);
	});
}