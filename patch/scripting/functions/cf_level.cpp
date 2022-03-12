#include <shared/defs.h>

#include <sol/sol.hpp>

#include <mp/client.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include <game/game.h>
#include <game/minecart.h>

#include "cf_defs.h"

void cf_level::register_functions(sol::state* vm)
{
	vm->set_function("resetLevelFinished",				[&]()			  { level_complete = false; });
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
	vm->set_function("enableEngineExtendedFeatures",	[&](bool enabled) { enable_engine_extended_features = enabled; });
	vm->set_function("setMinecartTurnModifier",			[&](int v)		  { minecart_turn_extra_blocks = v; });
	vm->set_function("getFloor",						[&](int x, int y, int z, int16_t room)
																		{
																			auto floor = GetFloor(x, y, z, &room);
																			return floor ? floor->floor : 0;
																		});
}