#include <shared/defs.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/globals.h>

#include <sol/sol.hpp>

#include <audio/audio_system.h>

#include "cf_defs.h"

#include <specific/file.h>
#include <game/laraanim.h>

void cf_anim::register_functions(sol::state* vm)
{
	vm->set_function("loadAnimation", [&](const char* filename)
	{
		return load_animation(filename);
	});

	vm->set_function("getAnimFrameBase", [&](int16_t anim, int16_t offset) -> int16_t
	{
		return GF(anim, offset);
	});
}