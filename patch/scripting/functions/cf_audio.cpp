#include <shared/defs.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/globals.h>

#include <sol/sol.hpp>

#include <audio/audio_system.h>

#include "cf_defs.h"

void cf_audio::register_functions(sol::state* vm)
{
	vm->set_function("playGameSound", [&](int id, int x, int y, int z, float pitch, bool sync)
	{
		return g_audio->play_sound(id, { x, y, z }, pitch, sync);
	});

	vm->set_function("loadAudio", [&](sol::this_state s, const char* path)
	{
		return g_audio->load_audio(resource_system::RESOURCES_PATH + script::get_global_string(s, scripting::globals::RESOURCE_NAME) + '\\' + path);
	});

	vm->set_function("unloadAudio", [&](sol::this_state s, const char* path)
	{
		return g_audio->unload_audio(resource_system::RESOURCES_PATH + script::get_global_string(s, scripting::globals::RESOURCE_NAME) + '\\' + path);
	});

	vm->set_function("playAudio", [&](sol::this_state s, const char* file, int x, int y, int z, float pitch)
	{
		return g_audio->create_sound(script::get_global_script(s), resource_system::RESOURCES_PATH + script::get_global_string(s, scripting::globals::RESOURCE_NAME) + '\\' + file, { x, y, z }, pitch);
	});

	vm->set_function("isAudio", [&](audio* a)
	{
		return g_audio->has_audio(a);
	});
}