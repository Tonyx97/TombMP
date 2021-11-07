import prof;

#include <shared/defs.h>

#include <shared/scripting/resource_system.h>
#include <shared/timer_system/timer_system.h>
#include <shared/bug_ripper/bug_ripper.h>

#include <scripting/global.h>
#include <game_ms/game_ms.h>
#include <server/server.h>
#include <game/level.h>

int main()
{
	auto err = [&](int id)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		return id;
	};

	prof::init("TombMP Server");

	utils::mt.seed(__rdtsc());

	g_bug_ripper = std::make_unique<bug_ripper>();
	g_resource = std::make_unique<resource_system>();
	g_server = std::make_unique<server>();
	g_level = std::make_unique<game_level>();
	g_timer = std::make_unique<timer_system>();

	if (!g_bug_ripper->init())
		return 0;

	g_level->register_entity_type(ENTITY_LEVEL_TYPE_BLOCK, MOVABLE_BLOCK, MOVABLE_BLOCK2, MOVABLE_BLOCK3, MOVABLE_BLOCK4);
	
	g_level->register_entity_type(ENTITY_LEVEL_TYPE_AI, TRIBEAXE, DOG, SMALL_RAT, WHALE, DIVER, CROW, TIGER, VULTURE, LIZARD_MAN,
								  TONY, TRIBEBOSS, LON_BOSS, WILLARD_BOSS, STHPAC_MERCENARY, OILRED, WHITE_SOLDIER, HUSKIE, BURNT_MUTANT, MUTANT1, MUTANT2, MUTANT3,
								  LON_MERCENARY1, LON_MERCENARY2, PUNK1, PUNK2, SECURITY_GUARD, MP1, MP2, BOB, SWAT_GUN, CIVVIE, COBRA,
								  SHIVA, MONKEY, TREX, RAPTOR, COMPY);

	g_level->register_entity_type(ENTITY_LEVEL_TYPE_INTERACTIVE, SMALL_SWITCH, AIRLOCK_SWITCH, PUSH_SWITCH, SWITCH_TYPE1, SWITCH_TYPE2,
								  KEY_HOLE1, KEY_HOLE2, KEY_HOLE3, KEY_HOLE4, PUZZLE_HOLE1, PUZZLE_HOLE2, PUZZLE_HOLE3, PUZZLE_HOLE4,
								  PUZZLE_DONE1, PUZZLE_DONE2, PUZZLE_DONE3, PUZZLE_DONE4);

	g_level->register_entity_type(ENTITY_LEVEL_TYPE_DOOR, DOOR_TYPE1, DOOR_TYPE2, DOOR_TYPE3, DOOR_TYPE4, DOOR_TYPE5, DOOR_TYPE6, DOOR_TYPE7, DOOR_TYPE8,
								  DOOR_TYPE9, TRAPDOOR, TRAPDOOR2);
	
	g_level->register_entity_type(ENTITY_LEVEL_TYPE_TRAP, MINI_COPTER, DEATH_SLIDE, SPIKE_WALL, CEILING_SPIKES, HOOK, SAW, FAN,
								  SMALL_FAN, SPINNING_BLADE, ICICLES, BLADE, SPRING_BOARD, FALLING_BLOCK, FALLING_BLOCK2, FALLING_PLANK,
								  PENDULUM, TEETH_TRAP, AVALANCHE, OILDRUMS, ROLLING_BALL, BIG_ROLLING_BALL, SPIKES, FALLING_CEILING1,
								  FLAME_EMITTER, FLAME_EMITTER2, FLAME_EMITTER3, SIDE_FLAME_EMITTER);
	
	g_level->register_entity_type(ENTITY_LEVEL_TYPE_ANIMATING, ANIMATING1, ANIMATING2, ANIMATING3, ANIMATING4, ANIMATING5, ANIMATING6);

	g_level->register_entity_type(ENTITY_LEVEL_TYPE_SPECIAL_FX, SPECIAL_FX1, SPECIAL_FX2, SPECIAL_FX3);

	g_level->register_entity_type(ENTITY_LEVEL_TYPE_PICKUP, PICKUP_ITEM1, PICKUP_ITEM2, KEY_ITEM1, KEY_ITEM2, KEY_ITEM3, KEY_ITEM4,
								  PUZZLE_ITEM1, PUZZLE_ITEM2, PUZZLE_ITEM3, PUZZLE_ITEM4, GUN_ITEM, SHOTGUN_ITEM, HARPOON_ITEM, ROCKET_GUN_ITEM,
								  GRENADE_GUN_ITEM, M16_ITEM, MAGNUM_ITEM, UZI_ITEM, FLAREBOX_ITEM, GUN_AMMO_ITEM, SG_AMMO_ITEM, MAG_AMMO_ITEM,
								  UZI_AMMO_ITEM, HARPOON_AMMO_ITEM, M16_AMMO_ITEM, ROCKET_AMMO_ITEM, GRENADE_AMMO_ITEM, MEDI_ITEM, BIGMEDI_ITEM,
								  ICON_PICKUP1_ITEM, ICON_PICKUP2_ITEM, ICON_PICKUP3_ITEM, ICON_PICKUP4_ITEM, SAVEGAME_CRYSTAL_ITEM);

	g_level->register_entity_type(ENTITY_LEVEL_TYPE_VEHICLE, QUADBIKE, KAYAK, BOAT, MINECART, BIGGUN, UPV);

	g_resource->register_as_server();
	g_resource->set_script_setup_fn(scripting::init_functions_and_globals);
	g_resource->set_action_callback(server::on_resource_action);
	g_resource->set_err_callback_fn(server::script_error_callback);

	if (!g_server->init())
		return err(1);

	if (!g_server->load_resources(true))
		return err(2);

	while (!GetAsyncKeyState(VK_F12))
		g_server->dispatch_packets();

	g_resource.reset();
	g_timer.reset();
	g_level.reset();
	g_server.reset();
	g_bug_ripper.reset();

	return 0;
}