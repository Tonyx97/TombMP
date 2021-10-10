import prof;

#include "objects.h"
#include "laraanim.h"
#include "invfunc.h"
#include "laramisc.h"
#include "laraflar.h"
#include "health.h"
#include "game.h"
#include "effect2.h"

#include <specific/standard.h>
#include <specific/input.h>
#include <specific/fn_stubs.h>

#include <scripting/events.h>

#include <shared/scripting/resource_system.h>

#include <mp/client.h>
#include <mp/game/entity_base.h>
#include <mp/game/level.h>

#define MAXOFF		220
#define MAXOFF_KH	200
#define MAXOFF_PH	200

extern int16_t inv_keys_objects;

enum switch_states
{
	SS_OFF,
	SS_ON,
	SS_LINK
};

int16_t PickUpBounds[12] =
{
	-256, 256,
	-100, 100,
	-256, 256,
	-10 * ONE_DEGREE, 10 * ONE_DEGREE,
	0, 0,
	0, 0
};

PHD_VECTOR PickUpPosition = { 0, 0, -100 };

int16_t PickUpBoundsUW[12] =
{
	-512, 512,
	-512, 512,
	-512, 512,
	-45 * ONE_DEGREE, 45 * ONE_DEGREE,
	-45 * ONE_DEGREE, 45 * ONE_DEGREE,
	-45 * ONE_DEGREE, 45 * ONE_DEGREE
};

PHD_VECTOR PickUpPositionUW = { 0, -200, -350 };

int16_t Switch1Bounds[12] =
{
	-MAXOFF, MAXOFF,					// X axis Limits
	0, 0,								// Y axis Limits
	WALL_L / 2 - MAXOFF, WALL_L / 2,	// Z axis Limits
	-10 * ONE_DEGREE, 10 * ONE_DEGREE,	// X Rot Limits
	-30 * ONE_DEGREE, 30 * ONE_DEGREE,	// Y Rot Limits
	-10 * ONE_DEGREE, 10 * ONE_DEGREE	// Z Rot Limits
};

PHD_VECTOR DetonatorPosition	= { 0, 0, 0 };
PHD_VECTOR SmallSwitchPosition	= { 0, 0, WALL_L / 2 - LARA_RAD - 50 };
PHD_VECTOR PushSwitchPosition	= { 0, 0, WALL_L / 2 - LARA_RAD - 120 };
PHD_VECTOR AirlockPosition		= { 0, 0, WALL_L / 2 - LARA_RAD - 200 };

int16_t Switch2Bounds[12] =
{
	-WALL_L, WALL_L,					// X axis Limits
	-WALL_L, WALL_L,					// Y axis Limits
	-WALL_L, WALL_L / 2,       			// Z axis Limits
	-80 * ONE_DEGREE, 80 * ONE_DEGREE,	// X Rot Limits
	-80 * ONE_DEGREE, 80 * ONE_DEGREE,	// Y Rot Limits
	-80 * ONE_DEGREE, 80 * ONE_DEGREE	// Z Rot Limits
};

PHD_VECTOR Switch2Position = { 0, 0, 108 };	// X, Y, Z relative to Switch

int16_t KeyHoleBounds[12] =
{
	-MAXOFF_KH, MAXOFF_KH,				// X axis Limits
	0, 0,								// Y axis Limits
	WALL_L / 2 - MAXOFF_KH, WALL_L / 2,	// Z axis Limits
	-10 * ONE_DEGREE, 10 * ONE_DEGREE,	// X Rot Limits
	-30 * ONE_DEGREE, 30 * ONE_DEGREE,	// Y Rot Limits
	-10 * ONE_DEGREE, 10 * ONE_DEGREE	// Z Rot Limits
};

PHD_VECTOR KeyHolePosition = { 0, 0, WALL_L / 2 - LARA_RAD - 50 };

int16_t PuzzleHoleBounds[12] =
{
	-MAXOFF_PH, MAXOFF_PH,				// X axis Limits
	0,0,								// Y axis Limits
	WALL_L / 2 - MAXOFF_PH, WALL_L / 2,	// Z axis Limits
	-10 * ONE_DEGREE, 10 * ONE_DEGREE,	// X Rot Limits
	-30 * ONE_DEGREE, 30 * ONE_DEGREE,	// Y Rot Limits
	-10 * ONE_DEGREE, 10 * ONE_DEGREE	// Z Rot Limits
};

PHD_VECTOR PuzzleHolePosition = { 0, 0, WALL_L / 2 - LARA_RAD - 85 };  // Ideal Pos of Lara rel to PuzzleHole

int pup_x, pup_y, pup_z;

void PickUpCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	auto x_rot = item->pos.x_rot,
		 y_rot = item->pos.y_rot,
		 z_rot = item->pos.z_rot;

	item->pos.y_rot = laraitem->pos.y_rot;
	item->pos.z_rot = 0;

	if (lara.water_status == LARA_ABOVEWATER || lara.water_status == LARA_WADE)
	{
		item->pos.x_rot = 0;

		if (ItemNearLara(item, &laraitem->pos, 0x200))
			if (!g_resource->trigger_event(events::pickup::ON_PICKUP_COLLISION, item))
				return;

		if (!TestLaraPosition(PickUpBounds, item, laraitem))
		{
			item->pos.x_rot = x_rot;
			item->pos.y_rot = y_rot;
			item->pos.z_rot = z_rot;

			return;
		}

		if (laraitem->current_anim_state == AS_PICKUP)
		{
			if ((laraitem->frame_number == PICKUP_F || laraitem->frame_number == DUCKPICKUP_F || laraitem->frame_number == ALL4SPICKUP_F) && item->object_number != FLARE_ITEM)
			{
				AddDisplayPickup(item->object_number);
				Inv_AddItem(item->object_number);

				if (item->object_number == ICON_PICKUP1_ITEM ||
					item->object_number == ICON_PICKUP2_ITEM ||
					item->object_number == ICON_PICKUP3_ITEM ||
					item->object_number == ICON_PICKUP4_ITEM)
				{
					if (artifact_pickup_finish)
						return FinishLevel();
				}

				KillItem(item_num);
			}
			else
			{
				item->pos.x_rot = x_rot;
				item->pos.y_rot = y_rot;
				item->pos.z_rot = z_rot;
			}

			return;
		}
		else if (laraitem->current_anim_state == AS_FLAREPICKUP)
		{
			if (((laraitem->anim_number == DUCKPICKUPF_A && laraitem->frame_number == DUCKPICKUPF_F) ||
				laraitem->frame_number == PICKUPF_F) && item->object_number == FLARE_ITEM && lara.gun_type != LG_FLARE)
			{
				lara.request_gun_type = LG_FLARE;
				lara.gun_type = LG_FLARE;

				InitialiseNewWeapon();

				lara.gun_status = LG_SPECIAL;
				lara.flare_age = (int32_t)(item->data) & 0x7fff;

				KillItem(item_num);
			}

			return;
		}
		else if ((input & IN_ACTION) && laraitem->current_anim_state == AS_ALL4S)
			laraitem->goal_anim_state = AS_DUCK;
		else if ((input & IN_ACTION) &&
				 !laraitem->gravity_status &&
				 ((laraitem->current_anim_state == AS_STOP && laraitem->anim_number == BREATH_A) ||
				 (laraitem->current_anim_state == AS_DUCK && laraitem->anim_number == DUCKBREATHE_A)) &&
				 lara.gun_status == LG_ARMLESS && !(lara.gun_type == LG_FLARE && item->object_number == FLARE_ITEM))
		{
			if (item->object_number == FLARE_ITEM)
			{
				laraitem->goal_anim_state = AS_FLAREPICKUP;

				do AnimateLara(laraitem);
				while (laraitem->current_anim_state != AS_FLAREPICKUP);
			}
			else
			{
				AlignLaraPosition(&PickUpPosition, item, laraitem);

				laraitem->goal_anim_state = AS_PICKUP;

				AnimateLara(laraitem);
			}

			laraitem->goal_anim_state = (lara.is_ducked ? AS_DUCK : AS_STOP);
			
			lara.gun_status = LG_HANDSBUSY;

			item->pos.x_rot = x_rot;
			item->pos.y_rot = y_rot;
			item->pos.z_rot = z_rot;

			return;
		}
	}
	else if (lara.water_status == LARA_UNDERWATER)
	{
		item->pos.x_rot = -25 * ONE_DEGREE;

		if (ItemNearLara(item, &laraitem->pos, 0x200))
			if (!g_resource->trigger_event(events::pickup::ON_PICKUP_COLLISION, item))
				return;

		if (!TestLaraPosition(PickUpBoundsUW, item, laraitem))
		{
			item->pos.x_rot = x_rot;
			item->pos.y_rot = y_rot;
			item->pos.z_rot = z_rot;

			return;
		}

		if (laraitem->current_anim_state == AS_PICKUP)
		{
			if (laraitem->frame_number == PICKUP_UW_F && item->object_number != FLARE_ITEM)
			{
				AddDisplayPickup(item->object_number);
				Inv_AddItem(item->object_number);

				KillItem(item_num);
			}

			return;
		}
		else if (laraitem->current_anim_state == AS_FLAREPICKUP)
		{
			if (laraitem->frame_number == PICKUPF_UW_F && item->object_number == FLARE_ITEM && lara.gun_type != LG_FLARE)
			{
				lara.request_gun_type = LG_FLARE;
				lara.gun_type = LG_FLARE;

				InitialiseNewWeapon();

				lara.gun_status = LG_SPECIAL;
				lara.flare_age = (int32_t)(item->data) & 0x7fff;

				draw_flare_meshes();
				KillItem(item_num);
			}

			return;
		}
		else if (input & IN_ACTION && laraitem->current_anim_state == AS_TREAD && lara.gun_status == LG_ARMLESS && !(lara.gun_type == LG_FLARE && item->object_number == FLARE_ITEM))
		{
			if (!MoveLaraPosition(&PickUpPositionUW, item, laraitem))
				return;

			if (item->object_number == FLARE_ITEM)
			{
				laraitem->fallspeed = 0;
				laraitem->current_anim_state = AS_FLAREPICKUP;
				laraitem->anim_number = PICKUPF_UW_A;
				laraitem->frame_number = anims[PICKUPF_UW_A].frame_base;
			}
			else
			{
				laraitem->goal_anim_state = AS_PICKUP;

				do AnimateLara(laraitem);
				while (laraitem->current_anim_state != AS_PICKUP);
			}

			laraitem->goal_anim_state = AS_TREAD;

			return;
		}
	}

	item->pos.x_rot = x_rot;
	item->pos.y_rot = y_rot;
	item->pos.z_rot = z_rot;
}

void SwitchCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (!(input & IN_ACTION) || item->status != NOT_ACTIVE || (item->flags & ONESHOT) || lara.gun_status != LG_ARMLESS || laraitem->gravity_status)
		return;

	auto entity = g_level->get_entity_by_item(item);

	if (entity)
	{
		g_level->request_entity_ownership(entity, true, 5);

		if (!g_level->is_entity_streamed(entity))
			return;
	}

	if (laraitem->current_anim_state == AS_STOP && laraitem->anim_number == BREATH_A)
	{
		if (!TestLaraPosition(Switch1Bounds, item, laraitem))
			return;

		lara.torso_x_rot = lara.torso_y_rot = lara.head_x_rot = lara.head_y_rot = 0;

		laraitem->pos.y_rot = item->pos.y_rot;

		if (item->object_number == SMALL_SWITCH)		AlignLaraPosition(&SmallSwitchPosition, item, laraitem);
		else if (item->object_number == PUSH_SWITCH)	AlignLaraPosition(&PushSwitchPosition, item, laraitem);
		else if (item->object_number == AIRLOCK_SWITCH) AlignLaraPosition(&AirlockPosition, item, laraitem);

		if (item->current_anim_state == SS_ON)
		{
			laraitem->current_anim_state = AS_SWITCHON;

			if (item->object_number == SMALL_SWITCH)
				laraitem->anim_number = objects[LARA].anim_index + 195;
			else if (item->object_number == PUSH_SWITCH)
				laraitem->anim_number = objects[LARA].anim_index + 197;
			else if (item->object_number == AIRLOCK_SWITCH)
			{
				laraitem->anim_number = objects[LARA_EXTRA].anim_index;
				laraitem->frame_number = anims[laraitem->anim_number].frame_base;
				laraitem->current_anim_state = EXTRA_BREATH;
				laraitem->goal_anim_state = EXTRA_AIRLOCK;

				AnimateItem(laraitem);

				lara.extra_anim = 1;
			}
			else laraitem->anim_number = objects[LARA].anim_index + 63;

			item->goal_anim_state = SS_OFF;
		}
		else
		{
			laraitem->current_anim_state = AS_SWITCHOFF;

			if (item->object_number == SMALL_SWITCH)
				laraitem->anim_number = objects[LARA].anim_index + 196;
			else if (item->object_number == PUSH_SWITCH)
				laraitem->anim_number = objects[LARA].anim_index + 197;
			else if (item->object_number == AIRLOCK_SWITCH)
			{
				laraitem->anim_number = objects[LARA_EXTRA].anim_index;
				laraitem->frame_number = anims[laraitem->anim_number].frame_base;
				laraitem->current_anim_state = EXTRA_BREATH;
				laraitem->goal_anim_state = EXTRA_AIRLOCK;

				AnimateItem(laraitem);

				lara.extra_anim = 1;
			}
			else laraitem->anim_number = objects[LARA].anim_index + 64;

			item->goal_anim_state = SS_ON;
		}

		if (!lara.extra_anim)
		{
			laraitem->frame_number = anims[laraitem->anim_number].frame_base;
			laraitem->goal_anim_state = AS_STOP;
		}

		lara.gun_status = LG_HANDSBUSY;

		item->status = ACTIVE;

		AddActiveItem(item_num);
		AnimateItem(item);
	}
}

void SwitchCollision2(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (!(input & IN_ACTION) || item->status != NOT_ACTIVE || lara.water_status != LARA_UNDERWATER || lara.gun_status != LG_ARMLESS)
		return;

	auto entity = g_level->get_entity_by_item(item);

	if (entity)
	{
		g_level->request_entity_ownership(entity, true, 5);

		if (!g_level->is_entity_streamed(entity))
			return;
	}

	if (laraitem->current_anim_state == AS_TREAD)
	{
		if (!TestLaraPosition(Switch2Bounds, item, laraitem))
			return;

		if (item->current_anim_state == SS_ON || item->current_anim_state == SS_OFF)
		{
			if (MoveLaraPosition(&Switch2Position, item, laraitem))
			{
				laraitem->fallspeed = 0;
				laraitem->goal_anim_state = AS_SWITCHON;

				do AnimateLara(laraitem);
				while (laraitem->current_anim_state != AS_SWITCHON);

				laraitem->goal_anim_state = AS_TREAD;
				lara.gun_status = LG_HANDSBUSY;

				item->status = ACTIVE;
				item->goal_anim_state = (item->current_anim_state== SS_ON ? SS_OFF : SS_ON);

				AddActiveItem(item_num);
				AnimateItem(item);
			}
		}
	}
}

void DetonatorCollision(int16_t item_number, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	if (lara.extra_anim)
		return;

	auto item = &items[item_number];

	auto x_rot = item->pos.x_rot,
		 y_rot = item->pos.y_rot,
		 z_rot = item->pos.z_rot;

	item->pos.y_rot = laraitem->pos.y_rot;
	item->pos.z_rot = 0;
	item->pos.x_rot = 0;

	if (item->status != DEACTIVATED && (input & IN_ACTION) && lara.gun_status == LG_ARMLESS && !laraitem->gravity_status && laraitem->current_anim_state == AS_STOP)
	{
		if (item->object_number == DETONATOR && !TestLaraPosition(PickUpBounds, item, laraitem))
			goto normal_collision;
		
		if (Inventory_Chosen == -1)
		{
			init_inventory(INV_KEYS_MODE);
			return;
		}

		if (Inventory_Chosen != KEY_OPTION2)
			goto normal_collision;

		Inv_RemoveItem(KEY_OPTION2);
		AlignLaraPosition(&DetonatorPosition, item, laraitem);

		laraitem->anim_number = objects[LARA_EXTRA].anim_index;
		laraitem->frame_number = anims[laraitem->anim_number].frame_base;
		laraitem->current_anim_state = EXTRA_BREATH;

		if (item->object_number == DETONATOR)
			laraitem->goal_anim_state = EXTRA_PLUNGER;
		
		AnimateItem(laraitem);

		lara.extra_anim = 1;
		lara.gun_status = LG_HANDSBUSY;

		if (item->object_number == DETONATOR)
		{
			item->status = ACTIVE;

			AddActiveItem(item_number);
		}

		return;
	}

normal_collision:
	item->pos.x_rot = x_rot;
	item->pos.y_rot = y_rot;
	item->pos.z_rot = z_rot;

	ObjectCollision(item_number, laraitem, coll);
}

void KeyHoleCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	if (laraitem->current_anim_state == AS_STOP && laraitem->anim_number == BREATH_A)
	{
		if ((Inventory_Chosen == -1 && !(input & IN_ACTION)) || lara.gun_status != LG_ARMLESS || laraitem->gravity_status)
			return;

		auto item = &items[item_num];

		auto entity = g_level->get_entity_by_item(item);

		if (entity)
		{
			g_level->request_entity_ownership(entity, true, 5);

			if (!g_level->is_entity_streamed(entity))
				return;
		}

		if (!TestLaraPosition(KeyHoleBounds, item, laraitem))
			return;

		bool correct = false;

		if (item->status != NOT_ACTIVE)
		{
			if (laraitem->pos.x_pos != pup_x ||
				laraitem->pos.y_pos != pup_y ||
				laraitem->pos.z_pos != pup_z)
			{
				pup_x = laraitem->pos.x_pos;
				pup_y = laraitem->pos.y_pos;
				pup_z = laraitem->pos.z_pos;

				g_audio->play_sound(2, { laraitem->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
			}

			return;
		}

		if (Inventory_Chosen == -1)
		{
			init_inventory(INV_KEYS_MODE);

			if (Inventory_Chosen == -1 && inv_keys_objects)
				return;

			if (Inventory_Chosen != -1)
				pup_y = laraitem->pos.y_pos - 1;

			return;
		}
		else pup_y = laraitem->pos.y_pos - 1;

		switch (item->object_number)
		{
		case KEY_HOLE1:
			if (Inventory_Chosen == KEY_OPTION1) correct = Inv_RemoveItem(KEY_OPTION1);
			break;
		case KEY_HOLE2:
			if (Inventory_Chosen == KEY_OPTION2) correct = Inv_RemoveItem(KEY_OPTION2);
			break;
		case KEY_HOLE3:
			if (Inventory_Chosen == KEY_OPTION3) correct = Inv_RemoveItem(KEY_OPTION3);
			break;
		case KEY_HOLE4:
			if (Inventory_Chosen == KEY_OPTION4) correct = Inv_RemoveItem(KEY_OPTION4);
			break;
		}

		Inventory_Chosen = -1;

		if (!correct)
		{
			if (laraitem->pos.x_pos != pup_x ||
				laraitem->pos.y_pos != pup_y ||
				laraitem->pos.z_pos != pup_z)
			{
				g_audio->play_sound(2, { laraitem->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				pup_x = laraitem->pos.x_pos;
				pup_y = laraitem->pos.y_pos;
				pup_z = laraitem->pos.z_pos;
			}

			return;
		}

		AlignLaraPosition(&KeyHolePosition, item, laraitem);

		laraitem->goal_anim_state = AS_USEKEY;

		do AnimateLara(laraitem);
		while (laraitem->current_anim_state != AS_USEKEY);

		laraitem->goal_anim_state = AS_STOP;

		lara.gun_status = LG_HANDSBUSY;

		item->status = ACTIVE;

		pup_x = laraitem->pos.x_pos;
		pup_y = laraitem->pos.y_pos;
		pup_z = laraitem->pos.z_pos;
	}
}

void PuzzleHoleCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	auto request_control = [&]()
	{
		if (auto entity = g_level->get_entity_by_item(item))
		{
			g_level->request_entity_ownership(entity, true, 5);

			if (!g_level->is_entity_streamed(entity))
				return false;
		}

		return true;
	};

	if (laraitem->current_anim_state == AS_STOP && laraitem->anim_number == BREATH_A)
	{
		if ((Inventory_Chosen == -1 && !(input & IN_ACTION)) || lara.gun_status != LG_ARMLESS || laraitem->gravity_status)
			return;

		if (!TestLaraPosition(PuzzleHoleBounds, item, laraitem))
			return;

		if (!request_control())
			return;

		if (item->status != NOT_ACTIVE)
		{
			if (laraitem->pos.x_pos != pup_x ||
				laraitem->pos.y_pos != pup_y ||
				laraitem->pos.z_pos != pup_z)
			{
				pup_x = laraitem->pos.x_pos;
				pup_y = laraitem->pos.y_pos;
				pup_z = laraitem->pos.z_pos;

				g_audio->play_sound(2, { laraitem->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
			}

			return;
		}

		if (Inventory_Chosen == -1)
		{
			init_inventory(INV_KEYS_MODE);

			if (Inventory_Chosen == -1 && inv_keys_objects)
				return;

			if (Inventory_Chosen != -1)
				pup_y = laraitem->pos.y_pos - 1;

			return;
		}
		else pup_y = laraitem->pos.y_pos - 1;

		bool correct = false;

		switch (item->object_number)
		{
		case PUZZLE_HOLE1:
			if (Inventory_Chosen == PUZZLE_OPTION1) correct = Inv_RemoveItem(PUZZLE_OPTION1);
			break;
		case PUZZLE_HOLE2:
			if (Inventory_Chosen == PUZZLE_OPTION2) correct = Inv_RemoveItem(PUZZLE_OPTION2);
			break;
		case PUZZLE_HOLE3:
			if (Inventory_Chosen == PUZZLE_OPTION3) correct = Inv_RemoveItem(PUZZLE_OPTION3);
			break;
		case PUZZLE_HOLE4:
			if (Inventory_Chosen == PUZZLE_OPTION4) correct = Inv_RemoveItem(PUZZLE_OPTION4);
			break;
		}

		Inventory_Chosen = -1;

		if (!correct)
		{
			if (laraitem->pos.x_pos != pup_x ||
				laraitem->pos.y_pos != pup_y ||
				laraitem->pos.z_pos != pup_z)
			{
				g_audio->play_sound(2, { laraitem->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				pup_x = laraitem->pos.x_pos;
				pup_y = laraitem->pos.y_pos;
				pup_z = laraitem->pos.z_pos;
			}

			return;
		}

		AlignLaraPosition(&PuzzleHolePosition, item, laraitem);

		laraitem->goal_anim_state = AS_USEPUZZLE;

		do AnimateLara(laraitem);
		while (laraitem->current_anim_state != AS_USEPUZZLE);

		laraitem->goal_anim_state = AS_STOP;
		lara.gun_status = LG_HANDSBUSY;

		item->status = ACTIVE;

		pup_x = laraitem->pos.x_pos;
		pup_y = laraitem->pos.y_pos;
		pup_z = laraitem->pos.z_pos;
	}
	else if (laraitem->current_anim_state == AS_USEPUZZLE)
	{
		if (!TestLaraPosition(PuzzleHoleBounds, item, laraitem))
			return;

		if (!request_control())
			return;

		if (laraitem->frame_number == USEPUZZLE_F)
		{
			switch (item->object_number)
			{
			case PUZZLE_HOLE1: item->object_number = PUZZLE_DONE1; break;
			case PUZZLE_HOLE2: item->object_number = PUZZLE_DONE2; break;
			case PUZZLE_HOLE3: item->object_number = PUZZLE_DONE3; break;
			case PUZZLE_HOLE4: item->object_number = PUZZLE_DONE4; break;
			}

			item->anim_number = objects[item->object_number].anim_index;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
			item->required_anim_state = 0;

			AddActiveItem(item_num);

			item->flags = CODE_BITS;
			item->status = ACTIVE;

			AnimateItem(item);

			/*if (auto entity = g_level->get_entity_by_item(item))
				entity->sync(true);*/
		}
	}
}

void SwitchControl(int16_t item_number)
{
	auto item = &items[item_number];

	item->flags |= CODE_BITS;

	if (!TriggerActive(item) && !(item->flags & ONESHOT))
	{
		item->goal_anim_state = SS_ON;
		item->timer = 0;
	}

	AnimateItem(item);
}

int SwitchTrigger(int16_t item_num, int16_t timer)
{
	auto item = &items[item_num];

	if (item->status == DEACTIVATED)
	{
		if (item->current_anim_state == SS_OFF && timer > 0)
		{
			item->timer = timer;
			item->status = ACTIVE;

			if (timer != 1)
				item->timer *= 30;
		}
		else
		{
			RemoveActiveItem(item_num);

			item->status = NOT_ACTIVE;

			if (item->item_flags[0])
				item->flags |= ONESHOT;
		}

		return 1;
	}

	return (item->flags & ONESHOT);
}

int KeyTrigger(int16_t item_num)
{
	if (auto item = &items[item_num]; item->status == ACTIVE && lara.gun_status != LG_HANDSBUSY)
	{
		item->status = DEACTIVATED;
		return 1;
	}

	return 0;
}

int PickupTrigger(int16_t item_num)
{
	if (auto item = &items[item_num]; item->status == INVISIBLE)
	{
		item->status = DEACTIVATED;
		return 1;
	}

	return 0;
}

void BossDropIcon(int16_t item_number)
{
	auto item = &items[item_number];
	auto pickup_number = item->carried_item;

	while (pickup_number != NO_ITEM)
	{
		auto pickup = &items[pickup_number];

		pickup->pos.x_pos = item->pos.x_pos;
		pickup->pos.y_pos = item->pos.y_pos;
		pickup->pos.z_pos = item->pos.z_pos;

		ItemNewRoom(pickup_number, item->room_number);
		AddActiveItem(pickup_number);

		item->status = ACTIVE;

		pickup_number = pickup->carried_item;
	}
}

void AnimatingPickUp(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->status == INVISIBLE || (item->flags & KILLED_ITEM))
		return;

	if (item->object_number == SAVEGAME_CRYSTAL_ITEM && item->item_flags[1] == 0)
	{
		item->item_flags[1] = 1;
		item->item_flags[2] = item->pos.y_pos;
	}

	item->pos.y_rot += 0x400;
	++item->item_flags[0];
	item->item_flags[0] &= 63;

	int col = abs(m_sin(item->item_flags[0] << 7) >> 7);
	if (col > 31)
		col = 31;

	if (item->object_number == SAVEGAME_CRYSTAL_ITEM)
	{
		int dx = abs(m_sin(item->item_flags[0] << 7) >> 4);

		item->pos.y_pos = item->item_flags[2] - dx - 64;

		TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 8, 0, col >> 2, col);

		dx = abs(item->pos.x_pos - lara_item->pos.x_pos);

		int dy = abs(item->pos.y_pos - lara_item->pos.y_pos),
			dz = abs(item->pos.z_pos - lara_item->pos.z_pos);

		if (dx < 0x100 && dy < 0x400 && dz < 0x100)
		{
			if (g_resource->trigger_event(events::pickup::ON_SAVECRYSTAL_PICKUP, item))
			{
				g_audio->play_sound(212, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				KillItem(item_number);
			}
		}
	}
	else TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 8, 0, col, col >> 1);
}