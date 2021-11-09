import utils;
import prof;

#include <shared/defs.h>

#include <shared/scripting/resource_system.h>
#include <shared/timer_system/timer_system.h>
#include <shared/bug_ripper/bug_ripper.h>

#include <specific/standard.h>
#include <specific/global.h>
#include <specific/directx.h>
#include <specific/texture.h>
#include <specific/drawprimitive.h>
#include <specific/hwrender.h>
#include <specific/time.h>
#include <specific/picture.h>
#include <specific/init.h>
#include <specific/input.h>

#include <3dsystem/hwinsert.h>

#include <game/gameflow.h>

#include <main.h>

#include <argument_parser/argument_parser.h>
#include <window/window.h>
#include <keycode/keycode.h>
#include <scripting/global.h>
#include <scripting/debugger.h>
#include <ui/ui.h>
#include <mp/client.h>
#include <mp/chat.h>
#include <mp/cmd.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#define USE_NET 1

bool game_main();

int __stdcall WinMain(HINSTANCE instance, HINSTANCE prev_instance, char* _cmd_line, int cmd_show)
{
	std::string cmd_line = _cmd_line;

#if !defined(_DEBUG) && !defined(_LD) && !defined(LEVEL_EDITOR)
	if (cmd_line.empty())
		return 0;
#endif
	
	prof::init("TombMP Client");

	// systems initialization

	g_bug_ripper = std::make_unique<bug_ripper>();

	if (!g_bug_ripper->init())
		return 1;

	utils::mt.seed(__rdtsc());

#if defined(_DEBUG)
	prof::print(GREEN, "Real parameters: {}", cmd_line);

	if (cmd_line.empty())
		cmd_line = "-ip 127.0.0.1 -pass pass -nickname Tonyx97 ";
#elif defined(_LD)
	if (cmd_line.empty())
		cmd_line = "-ip 217.182.174.42 -pass pass -nickname Tonyx97 ";
#elif defined(LEVEL_EDITOR)
	cmd_line = "-ip 127.0.0.1 -pass pass -nickname player0 ";
#endif

	g_arg_parser = std::make_unique<argument_parser>(cmd_line);
	g_client = std::make_unique<client>();
	g_level = std::make_unique<game_level>();
	g_window = std::make_unique<window>();
	g_timer = std::make_unique<timer_system>();
	g_ui = std::make_unique<d2d1_ui>();
	g_chat = std::make_unique<chat>();
	g_keycode = std::make_unique<keycode>();
	g_audio = std::make_unique<audio_system>();
	g_resource = std::make_unique<resource_system>();
	g_debugger = std::make_unique<debugger>();

	g_resource->register_as_client();
	g_resource->set_script_setup_fn(scripting::init_functions_and_globals);
	g_resource->set_action_callback(client::on_resource_action);
	g_resource->set_err_callback_fn(debugger::error_callback);

#if USE_NET
	if (!g_client->init())
		return 4;
	
	if (!g_client->connect())
		return 5;

	for (auto i = 0; (i < 20 && !g_client->is_connected()); ++i)
		if (!g_client->dispatch_packets())
			std::this_thread::sleep_for(std::chrono::milliseconds(250));

	if (!g_client->is_connected())
		return g_client->get_last_err();

	g_client->join();
#endif

	if (!g_window->init(instance, "TombMP") ||
		!g_audio->init())
		return 6;

	if (!g_ui->init())
		return 7;

	// game

	auto exit_game = [&]()
	{
		chat_cmd::destroy_all_commands();

		WinFreeDX();
		DXFreeDeviceInfo(&App.DeviceInfo);

		g_resource.reset();
		g_timer.reset();
		g_ui.reset();
		g_chat.reset();
		g_keycode.reset();
		g_audio.reset();
		g_debugger.reset();
		g_level.reset();
		g_client.reset();
		g_window.reset();
		g_arg_parser.reset();
		g_bug_ripper.reset();

		free_game_memory();

		return 0;
	};

	memset(&App, 0, sizeof(WINAPP));

	App.hInstance = instance;
	App.WindowHandle = g_window->get_win32_handle();

	DXGetDeviceInfo(&App.DeviceInfo, App.WindowHandle);

	if (!WinDXInit(&App.DeviceInfo))
		return exit_game();

	App.DeviceInfoPtr = &App.DeviceInfo;

	InitDrawPrimitive(App.lpD3DDevice);

	DXTextureInit(DXTextureList);

	time_init();
	hwr_init();
	hwr_init_state();
	init_key_map();	
	setup_screen_size();

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

	g_window->show();
		
	game_main();
	hwr_destroy();

	return exit_game();
}

bool game_main()
{
	// called by WinMain and only returns when game wants to shut down

	game_sizer = screen_sizer = 1.0;

	S_InitialiseSystem();

	if (!std::filesystem::is_directory("data"))
	{
		prof::print(RED, "Data folder doesn't exist");
		return false;
	}

	if (!GF_LoadScriptFile("data\\gf.dat"))
		S_ExitSystem();

	int option = OPENING_GAME;

	bool ok = true;

	while (ok)
	{
		int level_number = option & 0xff;
		option &= ~0xff;

		switch (option)
		{
		case OPENING_GAME:
		{
			option = START_GAME;
			level_number = 0;

			break;
		}
		case START_GAME:
		{
			if ((option = GF_InterpretSequence(GF_level_sequence_list[LV_GYM])) == EXIT_GAME)
				ok = false;
			else if (option == LEVEL_CHANGE)
				option = START_GAME;

			break;
		}
		default:
			ok = false;
		}
	}

	return true;
}