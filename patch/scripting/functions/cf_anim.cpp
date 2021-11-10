#include <shared/defs.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/globals.h>

#include <sol/sol.hpp>

#include <audio/audio_system.h>

#include "cf_defs.h"

#include <specific/file.h>

#include <game/game.h>
#include <game/anim.h>
#include <game/laraanim.h>

void cf_anim::register_functions(sol::state* vm)
{
	vm->set_function("loadAnimation", [&](sol::this_state s, const char* filename)
	{
		return load_animation(resource_system::RESOURCES_PATH + script::get_global_string(s, scripting::globals::RESOURCE_NAME) + '\\' + filename);
	});

	vm->set_function("unloadAnimation", [&](int16_t id)
	{
		return unload_animation(id);
	});

	vm->set_function("getAnimFrameBase", [&](int16_t id, const sol::variadic_args& args) -> int16_t
	{
		if (is_valid_anim(id) && is_custom_anim_loaded(id))
		{
			int16_t offset = 0;

			if (args.size() == 1) offset = args[0].as<int16_t>();

			return GF(id, offset);
		}

		return -1;
	});

	vm->set_function("setAnimNextAnim", [&](int16_t id, int16_t next_id, const sol::variadic_args& args) -> int16_t
	{
		if (is_valid_anim(id) && is_custom_anim_loaded(id) && is_custom_anim_loaded(next_id))
		{
			int16_t next_offset = 0;

			if (args.size() == 1) next_offset = args[0].as<int16_t>();

			auto anim = &anims[id];

			anim->jump_anim_num = next_id;
			anim->jump_frame_num = GF(next_id, next_offset);

			return true;
		}

		return false;
	});

	vm->set_function("setEngineExtendedAnimID", [&](int extended_anim_id, int16_t id, const sol::variadic_args& args)
	{
		if (extended_anim_id >= 0 && extended_anim_id < MAX_LARA_EXTENDED_ANIMS && is_valid_anim(id) && is_custom_anim_loaded(id))
		{
			int16_t frame_offset = 0,
					end_id = -1;

			if (args.size() == 1) frame_offset = args[0].as<int16_t>();
			if (args.size() == 2) end_id = args[1].as<int16_t>();

			g_extended_anim_info[extended_anim_id] = { .id = id, .end_id = end_id, .frame = int16_t(GF(id, frame_offset)) };
		}
	});

	vm->set_function("setEngineExtendedVehicleAnimIDs", [&](bool enabled)
	{
		if (enabled)
		{
			biggun_anim_obj = BIGGUN_ANIM;
			sub_anim_obj = SUB_ANIM;
			quadbike_anim_obj = QUADBIKE_ANIM;
			boat_anim_obj = BOAT_ANIM;
			minecart_anim_obj = MINECART_ANIM;
			kayak_anim_obj = KAYAK_ANIM;
		}
		else
		{
			biggun_anim_obj = VEHICLE_ANIM;
			sub_anim_obj = VEHICLE_ANIM;
			quadbike_anim_obj = VEHICLE_ANIM;
			boat_anim_obj = VEHICLE_ANIM;
			minecart_anim_obj = VEHICLE_ANIM;
			kayak_anim_obj = VEHICLE_ANIM;
		}
	});
}