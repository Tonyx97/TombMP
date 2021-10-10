#include <shared/defs.h>

#include <sol/sol.hpp>

#include <mp/client.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include <game/game.h>

#include "cf_defs.h"

void cf_level::register_functions(sol::state* vm)
{
	vm->set_function("setLavaDeathType",				[&](int type)	  { lava_type = type; });
	vm->set_function("enableLaraBreath",				[&](bool enabled) { enable_lara_breath = enabled; });
	vm->set_function("enableColdExposure",				[&](bool enabled) { enable_cold_exposure = enabled; });
	vm->set_function("enableArtifactPickupLevelFinish",	[&](bool enabled) { artifact_pickup_finish = enabled; });
	vm->set_function("enablePropellerInstaDeath",		[&](bool enabled) { enable_propeller_insta_death = enabled; });
	vm->set_function("enableIslandSpikesSound",			[&](bool enabled) { enable_island_spikes_sound = enabled; });
	vm->set_function("enableDeadlySwamp",				[&](bool enabled) { enable_deadly_swamp = enabled; });
	vm->set_function("enableRapidsFireType",			[&](bool enabled) { enable_rapids_fire_type = enabled; });
	vm->set_function("setFishShoalType",				[&](int type)	  { fish_shoal_type = type; });
	vm->set_function("setFlamersFriendly",				[&](bool enabled) { enable_flamer_friendly = enabled; });
	vm->set_function("enableFuseBox",					[&](bool enabled) { enable_fusebox = enabled; });
	vm->set_function("enableSmashObj1Destruction",		[&](bool enabled) { enable_smash1_destruction = enabled; });
	vm->set_function("setPunksFriendly",				[&](bool enabled) { enable_punks_friendly = enabled; });
	vm->set_function("enableAIPatrolDestruction",		[&](bool enabled) { enable_killable_ai_patrol = enabled; });
	vm->set_function("enableFootprints",				[&](bool enabled) { enable_footprints = enabled; });
	vm->set_function("enableEngineExtendedFeatures",	[&](bool enabled)
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

		enable_engine_extended_features = enabled;
	});
}