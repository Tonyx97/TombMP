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
	vm->set_function("loadAnimation", [&](sol::this_state s, const char* filename)
	{
		return load_animation(resource_system::RESOURCES_PATH + script::get_global_string(s, scripting::globals::RESOURCE_NAME) + '\\' + filename);
	});

	vm->set_function("getAnimFrameBase", [&](int16_t anim, int16_t offset) -> int16_t
	{
		return GF(anim, offset);
	});

	vm->set_function("setAnimNextAnim", [&](int16_t id, int16_t next_id, int16_t next_offset) -> int16_t
	{
		if (id >= 0 && id < number_anims + number_custom_anims)
		{
			auto anim = &anims[id];

			anim->jump_anim_num = next_id;
			anim->jump_frame_num = GF(next_id, next_offset);

			return true;
		}

		return false;
	});
}