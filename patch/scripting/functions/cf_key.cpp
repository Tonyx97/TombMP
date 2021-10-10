#include <shared/defs.h>

#include <sol/sol.hpp>

#include <keycode/keycode.h>

#include <shared/scripting/resource_system.h>

#include <mp/cmd.h>

#include "cf_defs.h"

void cf_key::register_functions(sol::state* vm)
{
	vm->set_function("isKeyDown",		[&](int key) { return g_keycode->is_key_down(key); });
	vm->set_function("isKeyPressed",	[&](int key) { return g_keycode->is_key_pressed(key); });
	vm->set_function("isKeyReleased",	[&](int key) { return g_keycode->is_key_released(key); });
	vm->set_function("getMouseWheel",	[&]()		 { return g_keycode->get_mouse_wheel_value(); });

	vm->set_function("addCommand", [&](sol::this_state s, const char* cmd, const sol::function& fn)
	{
		return g_resource->add_command(cmd, script::get_global_script(s), fn);
	});

	vm->set_function("removeCommand", [&](sol::this_state s, const char* cmd)
	{
		return g_resource->remove_command(cmd, script::get_global_script(s));
	});

	vm->set_function("bindKey", [&](sol::this_state s, uint32_t key, const char* state, const sol::function& fn)
	{
		return g_resource->add_bind(key, state, script::get_global_script(s), fn);
	});

	vm->set_function("unbindKey", [&](sol::this_state s, uint32_t key, const char* state)
	{
		return g_resource->remove_bind(key, state, script::get_global_script(s));
	});
}