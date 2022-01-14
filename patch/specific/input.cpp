import prof;

#include "standard.h"
#include "global.h"
#include "input.h"
#include "frontend.h"
#include "init.h"
#include "output.h"

#include <game/moveblok.h>
#include <game/lot.h>
#include <game/control.h>
#include <game/invfunc.h>
#include <game/laramisc.h>
#include <game/camera.h>
#include <game/lara.h>
#include <game/objects.h>
#include <game/gameflow.h>
#include <game/laraanim.h>
#include <game/game.h>
#include <game/effect2.h>

#include <main.h>

#include <argument_parser/argument_parser.h>
#include <window/window.h>
#include <keycode/keycode.h>
#include <ui/ui.h>
#include <mp/chat.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

int g_key_map[IN_MAX_ID];

int get_general_input()
{
	int linput = 0;
	
	if (g_keycode->is_key_down(g_key_map[IN_FORWARD_ID]))	linput |= IN_FORWARD;
	if (g_keycode->is_key_down(g_key_map[IN_BACK_ID]))		linput |= IN_BACK;
	if (g_keycode->is_key_down(g_key_map[IN_LEFT_ID]))		linput |= IN_LEFT;
	if (g_keycode->is_key_down(g_key_map[IN_RIGHT_ID]))		linput |= IN_RIGHT;
	if (g_keycode->is_key_down(g_key_map[IN_SLOW_ID]))		linput |= IN_SLOW;
	if (g_keycode->is_key_down(g_key_map[IN_JUMP_ID]))		linput |= IN_JUMP;
	if (g_keycode->is_key_down(g_key_map[IN_ACTION_ID]))	linput |= IN_ACTION;
	if (g_keycode->is_key_down(g_key_map[IN_DRAW_ID]))		linput |= IN_DRAW;
	if (g_keycode->is_key_down(g_key_map[IN_FLARE_ID]))		linput |= IN_FLARE;
	if (g_keycode->is_key_down(g_key_map[IN_LOOK_ID]))		linput |= IN_LOOK;
	if (g_keycode->is_key_down(g_key_map[IN_ROLL_ID]))		linput |= IN_ROLL;
	if (g_keycode->is_key_down(g_key_map[IN_DUCK_ID]))		linput |= IN_DUCK;

	if (linput & IN_SLOW)
	{
		if (!(linput & (IN_FORWARD | IN_BACK)))
		{
			if (linput & IN_LEFT)		linput = (linput & ~IN_LEFT) | IN_STEPL;
			else if (linput & IN_RIGHT) linput = (linput & ~IN_RIGHT) | IN_STEPR;
		}
	}

	if (linput & IN_FORWARD && linput & IN_BACK)									linput |= IN_ROLL;
	if (g_keycode->is_key_pressed(g_key_map[IN_SELECT_ID]) || (linput & IN_ACTION))	linput |= IN_SELECT;
	if (g_keycode->is_key_pressed(g_key_map[IN_DESELECT_ID]) && lara.spawned)		linput |= (IN_DESELECT | IN_OPTION);

	if ((linput & (IN_LEFT | IN_RIGHT)) == (IN_LEFT | IN_RIGHT))
		linput -= (IN_LEFT | IN_RIGHT);

	if (g_keycode->is_key_down(g_key_map[IN_DUCK_ID])) linput |= IN_DUCK;
	if (g_keycode->is_key_down(g_key_map[IN_DASH_ID])) linput |= IN_DASH;

	// weapon select keys

	if (g_keycode->is_key_down(KEY_1) && Inv_RequestItem(GUN_OPTION))			lara.request_gun_type = LG_PISTOLS;
	else if (g_keycode->is_key_down(KEY_2) && Inv_RequestItem(SHOTGUN_OPTION))	lara.request_gun_type = LG_SHOTGUN;
	else if (g_keycode->is_key_down(KEY_3) && Inv_RequestItem(MAGNUM_OPTION))	lara.request_gun_type = LG_MAGNUMS;
	else if (g_keycode->is_key_down(KEY_4) && Inv_RequestItem(UZI_OPTION))		lara.request_gun_type = LG_UZIS;
	else if (g_keycode->is_key_down(KEY_5) && Inv_RequestItem(HARPOON_OPTION))	lara.request_gun_type = LG_HARPOON;
	else if (g_keycode->is_key_down(KEY_6) && Inv_RequestItem(M16_OPTION))		lara.request_gun_type = LG_M16;
	else if (g_keycode->is_key_down(KEY_7) && Inv_RequestItem(ROCKET_OPTION))	lara.request_gun_type = LG_ROCKET;
	else if (g_keycode->is_key_down(KEY_8) && Inv_RequestItem(GRENADE_OPTION))  lara.request_gun_type = LG_GRENADE;

	// medi pack select keys

	if (g_keycode->is_key_down(g_key_map[IN_SMALL_MEDKIT]) && Inv_RequestItem(MEDI_OPTION))		    UseItem(MEDI_OPTION);
	else if (g_keycode->is_key_down(g_key_map[IN_LARGE_MEDKIT]) && Inv_RequestItem(BIGMEDI_OPTION)) UseItem(BIGMEDI_OPTION);

	//if (g_keycode->is_key_down(KEY_L)) level_complete = 1;

#ifdef _DEBUG
	if (g_keycode->is_key_down(KEY_F1))
	{
		float x = lara_item->pos.x_pos / 449, y = lara_item->pos.y_pos / 449, z = lara_item->pos.z_pos / 449;
		printf_s("{ %i, %i, %i %i } -> %.2f %.2f %.2f\n", lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, lara_item->room_number, x, y, z);
	}

	if (g_keycode->is_key_down(KEY_F2))
	{
		printf_s("%i %i %i\n", lara_item->current_anim_state, lara.left_arm.frame_number, lara.right_arm.frame_number);

		//lara_item->current_anim_state = AS_STOP;
		//lara_item->goal_anim_state = AS_STOP;
		//lara_item->anim_number = 1714;
		//lara_item->frame_number = GF(1714, 0);
		//lara_item->anim_number = 642;
		//lara_item->frame_number = GF(642, 0);

		for (int i = 0; i < level_items; ++i)
			if (items[i].object_number == 211 || items[i].object_number == 208 || items[i].object_number == 207 || items[i].object_number == 225 || 
				items[i].object_number == 226 || items[i].object_number == 229 || items[i].object_number == 230)
				lara_item->pos = items[i].pos;
	}

	if (g_keycode->is_key_down(KEY_O) && lara_item)
	{
		PHD_VECTOR pelvis;
		pelvis.x = pelvis.y = pelvis.z = 0;
		get_lara_bone_pos(lara_item, &pelvis, HEAD);
		DoBloodSplat(pelvis.x, pelvis.y, pelvis.z, 10, 0, lara_item->room_number);
	}

	if (lara_item)
	{
		//printf_s("%i\n", );
		//lara_item->il.sun.y = rand() % 65565;
		//lara_item->il.sun.x = rand() % 65565;
		//lara_item->il.sun.z = rand() % 65565;
		//lara_item->il.bulb.x = rand() % 256;
		//lara_item->il.bulb.y = rand() % 256;
		//lara_item->il.bulb.z = rand() % 256;
	}

	if (g_keycode->is_key_down(KEY_I))
	{
		// lara creation
		/*auto item_number = CreateItem();
		if (item_number != NO_ITEM)
		{
			auto item = &items[item_number];

			item->object_number = 0;
			item->pos.x_pos = lara_item->pos.x_pos;
			item->pos.y_pos = lara_item->pos.y_pos;
			item->pos.z_pos = lara_item->pos.z_pos;
			item->room_number = lara_item->room_number;

			auto old_item_num = lara.item_number;
			auto old_item = lara_item;
			InitialiseItem(item_number);
			lara_item = old_item;
			lara.item_number = old_item_num;

			item->speed = 0;
			item->item_flags[0] = 0;

			AddActiveItem(item_number);

			lara_item2 = item;

			printf_s("%i\n", item_number);
		}

		Sleep(250);*/
	}

	if (g_keycode->is_key_down(KEY_K) && lara_item)
	{
		// tonyboss
		//lara_item->pos = { 41883, -4864, 52538, 0, 0, 0 };
		//lara_item->room_number = 24;

		// sumersible
		//lara_item->pos = { 59192, -2298, 23565, 0, 0, 0 };
		//lara_item->room_number = 103;

		// boat
		//lara_item->pos = { 25700, -5120, 11826, 0, 0, 0 };
		//lara_item->room_number = 138;

		// detention camp
		//lara_item->pos = { 36379, -3328, 42761, 0, 0, 0 };
		//lara_item->room_number = 30;

		// madubu blades
		
		//lara_item->pos = { 46912, -12032, 18790, 0, 0, 0 };
		//lara_item->room_number = 9;

		camera.target = { 46912, -12032, 18790, 0, 0 };

		//CalculateCamera();
	}

	if (g_keycode->is_key_down(KEY_J) && lara_item)
	{
		CreatureDie(lara.item_number, true, true, true);
		//Sleep(100);

		//for (int i = 0; i < 2; ++i)
		//	TriggerExplosionSparks(old_x, old_y, old_z, 3, -1, 0, item->room_number);

		// boat 2

		//lara_item->pos = { 26108, -6400, 11365, 0, 0, 0 };
		//lara_item->room_number = 39;

		//lara_item->speed = 100;
		//lara_item->fallspeed = -100;

		/*g_level->for_each_player([&](game_player* p)
		{
				if (p != game_level::LOCALPLAYER())
				{
					camera.item = p->get_item();
					prof::print(BLUE, "new item {:#x}", uint32_t(camera.item));
				}
		});

		CalculateCamera();*/
	}

	/*if (g_keycode->is_key_down(KEY_U) && lara_item)
	{
		lara_item->current_anim_state = AS_ALL4S;
		lara_item->goal_anim_state = AS_ALL4S;
		lara_item->anim_number = 297;
		lara_item->frame_number = GF(297, 0);

		Sleep(100);
	}*/

	/*if (lara_item)
		printf("%i %i\n", lara_item->current_anim_state, lara_item->goal_anim_state);*/

		/*if (rawkey(DIK_I))
		{
			auto create_pickup = [&](int16_t id, int16_t linked_pickup = NO_ITEM) -> int16_t
			{
				auto carried_item = CreateItem();
				if (carried_item != NO_ITEM)
				{
					auto item = &items[carried_item];

					item->object_number = id;
					item->pos.x_pos = -1;
					item->pos.y_pos = -1;
					item->pos.z_pos = -1;
					item->room_number = lara_item->room_number;
					item->shade = int16_t(0x4210 | 0x8000);

					InitialiseItem(carried_item);

					item->carried_item = linked_pickup;

					//AddActiveItem(carried_item);

					return carried_item;
				}

				return NO_ITEM;
			};

			auto p1 = create_pickup(ROCKET_AMMO_ITEM);

			auto item_number = CreateItem();
			if (item_number != NO_ITEM)
			{
				auto item = &items[item_number];

				item->object_number = RAPTOR;
				item->pos.x_pos = lara_item->pos.x_pos;
				item->pos.y_pos = lara_item->pos.y_pos;
				item->pos.z_pos = lara_item->pos.z_pos;
				item->room_number = lara_item->room_number;
				item->shade = int16_t(0x4210 | 0x8000);

				InitialiseItem(item_number);

				item->carried_item = p1;
				item->hit_points = 1;

				AddActiveItem(item_number);

				Sleep(200);
			}
		}*/

	if (g_keycode->is_key_down(KEY_O))
		{
			auto biggun = CreateItem();
			if (biggun != NO_ITEM)
			{
				auto item = &items[biggun];

				item->object_number = BIGGUN;
				item->pos.x_pos = lara_item->pos.x_pos;
				item->pos.y_pos = lara_item->pos.y_pos;
				item->pos.z_pos = lara_item->pos.z_pos;
				item->room_number = lara_item->room_number;
				item->shade = int16_t(0x4210 | 0x8000);

				InitialiseItem(biggun);

				item->carried_item = -1;
				item->hit_points = 1;

				AddActiveItem(biggun);

				Sleep(200);
			}
		}

	if (g_keycode->is_key_down(KEY_P))
	{
		auto item_number = CreateItem();
		if (item_number != NO_ITEM)
		{
			auto item = &items[item_number];

			item->object_number = COBRA;
			item->pos.x_pos = lara_item->pos.x_pos + 1024;
			item->pos.y_pos = lara_item->pos.y_pos;
			item->pos.z_pos = lara_item->pos.z_pos;
			item->room_number = lara_item->room_number;
			item->shade = int16_t(0x4210 | 0x8000);

			InitialiseItem(item_number);

			item->carried_item = -1;
			item->flags &= ~(ONESHOT | KILLED_ITEM | INVISIBLE);
			item->ai_bits = MODIFY;
			item->data = 0;
			item->status = ACTIVE;
			item->mesh_bits = 0xffffffff;
			item->hit_points = 1;
			item->collidable = 1;

			AddActiveItem(item_number);
			ItemNewRoom(item_number, item->room_number);
			EnableBaddieAI(item_number, 1);

			//Sleep(200);
		}
	}

	static int i = 15;
	
	if (g_keycode->is_key_down(KEY_G))
	{
		lara_item->anim_number = 312;
		lara_item->frame_number = GF(312, 0);
	}

	if (g_keycode->is_key_down(KEY_H) && --i <= 0)
	{
		lara_item->fallspeed = -240;
		lara_item->gravity_status = 1;
		lara_item->goal_anim_state = AS_FORWARDJUMP;
		lara_item->current_anim_state = AS_FORWARDJUMP;
		lara_item->anim_number = FALLDOWN_A;
		lara_item->frame_number = FALLDOWN_F;

		i = 15;
	}

	if (g_keycode->is_key_down(KEY_F) && --i <= 0) { FlipMap(); i = 15; }
	if (g_keycode->is_key_down(KEY_R))
	{
		camera.type = TARGET_CAMERA;
		camera.item = nullptr;
		camera.pos = { 50035, -1024, 45275, 2 };
		camera.target = { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.y_pos, 2 };
	}

	if (g_keycode->is_key_down(KEY_C)) linput |= IN_C;
	if (g_keycode->is_key_down(KEY_D)) linput |= IN_D;
#endif

	return linput;
}

void init_key_map()
{
	g_key_map[IN_FORWARD_ID] = g_arg_parser->get_registry_value<int>("Forward");
	g_key_map[IN_BACK_ID] = g_arg_parser->get_registry_value<int>("Back");
	g_key_map[IN_LEFT_ID] = g_arg_parser->get_registry_value<int>("Left");
	g_key_map[IN_RIGHT_ID] = g_arg_parser->get_registry_value<int>("Right");
	g_key_map[IN_JUMP_ID] = g_arg_parser->get_registry_value<int>("Jump");
	g_key_map[IN_DRAW_ID] = g_arg_parser->get_registry_value<int>("Draw Guns");
	g_key_map[IN_ACTION_ID] = g_arg_parser->get_registry_value<int>("Action");
	g_key_map[IN_SLOW_ID] = g_arg_parser->get_registry_value<int>("Walk");
	g_key_map[IN_OPTION_ID] = g_arg_parser->get_registry_value<int>("Option");
	g_key_map[IN_LOOK_ID] = g_arg_parser->get_registry_value<int>("Look");
	g_key_map[IN_ROLL_ID] = g_arg_parser->get_registry_value<int>("Roll");
	g_key_map[IN_FLARE_ID] = g_arg_parser->get_registry_value<int>("Flare");
	g_key_map[IN_SELECT_ID] = g_arg_parser->get_registry_value<int>("Select");
	g_key_map[IN_DESELECT_ID] = g_arg_parser->get_registry_value<int>("Deselect");
	g_key_map[IN_DUCK_ID] = g_arg_parser->get_registry_value<int>("Duck");
	g_key_map[IN_DASH_ID] = g_arg_parser->get_registry_value<int>("Sprint");
	g_key_map[IN_SMALL_MEDKIT] = g_arg_parser->get_registry_value<int>("SmallMedKit");
	g_key_map[IN_LARGE_MEDKIT] = g_arg_parser->get_registry_value<int>("LargeMedKit");
	g_key_map[IN_C_ID] = KEY_C;
	g_key_map[IN_D_ID] = KEY_D;
}

bool update_input(bool inventory)
{
	g_window->poll_events();

	if (inventory)
	{
		if (!lara.frozen)
		{
			inv_input = input = 0;

			if (g_keycode->is_key_down(g_key_map[IN_FORWARD_ID]))							inv_input |= IN_FORWARD;
			if (g_keycode->is_key_down(g_key_map[IN_BACK_ID]))								inv_input |= IN_BACK;
			if (g_keycode->is_key_down(g_key_map[IN_LEFT_ID]))								inv_input |= IN_LEFT;
			if (g_keycode->is_key_down(g_key_map[IN_RIGHT_ID]))								inv_input |= IN_RIGHT;
			if (g_keycode->is_key_pressed(g_key_map[IN_SELECT_ID]) ||
				g_keycode->is_key_pressed(g_key_map[IN_ACTION_ID]))							inv_input |= IN_SELECT;
			if (g_keycode->is_key_pressed(g_key_map[IN_DESELECT_ID]) && lara.spawned)		inv_input |= (IN_DESELECT | IN_OPTION);
		}
	}
	else
	{
		if (g_keycode->is_key_pressed(KEY_T))
			g_chat->begin_typing();

		if (g_chat->is_typing())
			input = 0;
		else if (!lara.frozen)
			input = get_general_input();
		else input = 0;

		if (bool escaped = g_keycode->is_key_pressed(KEY_ESCAPE); escaped || g_keycode->is_key_pressed(KEY_ENTER))
			g_chat->end_typing(!escaped);
		else if (g_keycode->is_key_down(KEY_PAGE_UP))
			g_chat->scroll_up();
		else if (g_keycode->is_key_down(KEY_PAGE_DOWN))
			g_chat->scroll_down();
	}

	return (game_closedown = g_window->wants_to_close());
}