#include <shared/defs.h>

#include <sol/sol.hpp>

#include <mp/client.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include <game/game.h>
#include <game/control.h>
#include <game/minecart.h>

#include "cf_defs.h"

import prof;

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

	vm->set_function("getFloor", [&](int x, int y, int z, int16_t room)
	{
		return GetFloor(x, y, z, &room);
	});

	vm->set_function("getFloorIndex", [&](FLOOR_INFO* floor) -> int16_t
	{
		if (!floor)
			return -1;

		return floor->index;
	});

	vm->set_function("isFloorSecretTrigger", [&](FLOOR_INFO* floor)
	{
		if (!floor)
			return false;

		auto dummy_data = &floor_data[floor->index];

		int16_t* data = nullptr,
				 type;

		do {
			type = *(dummy_data++);

			switch (type & DATA_TYPE)
			{
			case SPLIT1:
			case SPLIT2:
			case NOCOLF1T:
			case NOCOLF1B:
			case NOCOLF2T:
			case NOCOLF2B:
			case TILT_TYPE:
			case SPLIT3:
			case SPLIT4:
			case NOCOLC1T:
			case NOCOLC1B:
			case NOCOLC2T:
			case NOCOLC2B:
			case ROOF_TYPE:
			case DOOR_TYPE:
				++dummy_data;
				break;
			case LAVA_TYPE:
				data = dummy_data - 1;
				break;
			case CLIMB_TYPE:
			case MONKEY_TYPE:
			case MINEL_TYPE:
			case MINER_TYPE:
			{
				if (!data)
					data = dummy_data - 1;

				break;
			}
			case TRIGGER_TYPE:
			{
				if (!data)
					data = dummy_data - 1;

				++dummy_data;

				int16_t trigger;

				do {
					trigger = *(dummy_data++);

					if (TRIG_BITS(trigger) != TO_OBJECT)
					{
						if (TRIG_BITS(trigger) == TO_CAMERA)
							trigger = *(dummy_data++);

						continue;
					}
				} while (!(trigger & END_BIT));

				break;
			}
			}
		} while (!(type & END_BIT));

		if (!data)
			return false;

		if ((*data & DATA_TYPE) == LAVA_TYPE)
		{
			if (*data & END_BIT) return false;
			++data;
		}

		if ((*data & DATA_TYPE) == CLIMB_TYPE)
		{
			if (*data & END_BIT) return false;
			++data;
		}

		if ((*data & DATA_TYPE) == MONKEY_TYPE)
		{
			if (*data & END_BIT) return false;
			++data;
		}

		if ((*data & DATA_TYPE) == MINEL_TYPE)
		{
			if (*data & END_BIT) return false;
			++data;
		}

		if ((*data & DATA_TYPE) == MINER_TYPE)
		{
			if (*data & END_BIT) return false;
			++data;
		}

		type = (*(data++) >> 8) & 0x3f;

		++data;	// skip flags

		switch (type)
		{
		case SWITCH:	++data; break;
		case KEY:		++data; break;
		case PICKUP:	++data; break;
		case PAD:
		case ANTIPAD:
		case COMBAT:
		case HEAVY:
		case DUMMY:		break;
		}

		int16_t trigger;

		do {
			trigger = *(data++);

			switch (TRIG_BITS(trigger))
			{
			case TO_OBJECT:
			case TO_TARGET:
			case TO_SINK:
			case TO_FLIPMAP:
			case TO_FLIPON:
			case TO_FLIPOFF:
			case TO_FLIPEFFECT:
			case TO_FINISH:
			case TO_CD:
			case TO_BODYBAG:	break;
			case TO_CAMERA:		trigger = *(data++); break;
			case TO_SECRET:		return true;
			}
		} while (!(trigger & END_BIT));

		return false;
	});
}