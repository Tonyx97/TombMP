#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "control.h"
#include "effect2.h"

#include <specific/fn_stubs.h>

#define COMPY_DAMAGE 90
#define COMPY_RUN_TURN (ONE_DEGREE*10)
#define COMPY_STOP_TURN (ONE_DEGREE*3)
#define COMPY_UPSET_SPEED 15
#define COMPY_HIT_RANGE SQUARE(WALL_L/3)
#define COMPY_ATTACK_ANGLE 0x3000
#define COMPY_JUMP_CHANCE 0x1000
#define COMPY_ATTACK_CHANCE 0x1F
#define COMPY_TOUCH 0x0004
#define COMPY_DIE_ANIM 6
#define COMPY_HIT_FLAG 1

enum compy_anims
{
	COMPY_STOP,
	COMPY_RUN,
	COMPY_JUMP,
	COMPY_ATTACK,
	COMPY_DEATH
};

BITE_INFO compy_hit = { 0, 0, 0, 2 };

void InitialiseCompy(int16_t item_number)
{
	InitialiseCreature(item_number);
	compys_attack_lara = 0;
}

void CompyControl(int16_t item_number)
{
	int16_t compys_carcass, linknum;

	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto compy = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   neck = 0,
		   angle = 0;

	if (!item->item_flags[1])
	{
		linknum = room[item->room_number].item_number;

		if (items[linknum].object_number == ANIMATING6)
			item->item_flags[1] = linknum;
		else
		{
			for (; linknum != NO_ITEM; linknum = items[linknum].next_item)
			{
				if (items[linknum].object_number == ANIMATING6)
				{
					item->item_flags[1] = linknum;
					break;
				}
			}
		}
	}

	compys_carcass = item->item_flags[1];

	auto enemy = compy->enemy;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != COMPY_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + COMPY_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = COMPY_DEATH;
		}
	}
	else if (enemy)
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);

		if (compy->mood == BORED_MOOD && compys_carcass)
		{
			int dx = items[compys_carcass].pos.x_pos - item->pos.x_pos,
				dz = items[compys_carcass].pos.z_pos - item->pos.z_pos;

			info.distance = (dx * dx) + (dz * dz);
			info.angle = phd_atan(dz, dx) - item->pos.y_rot;
			info.ahead = (info.angle > -FRONT_ARC && info.angle < FRONT_ARC);
		}

		int random = ((item_number & 0x7) * 0x200) - 0x700;

		if (!compy_scared_timer && !compys_attack_lara && ((info.enemy_facing < COMPY_ATTACK_ANGLE && info.enemy_facing > -COMPY_ATTACK_ANGLE && lara_item->speed > COMPY_UPSET_SPEED) || lara_item->current_anim_state == AS_ROLL || item->hit_status))
		{
			item->item_flags[0] = (random + 0x700) >> 7;
			compy_scared_timer = 280;
		}
		else if (compy_scared_timer)
		{
			if (item->item_flags[0] > 0)
				--item->item_flags[0];
			else
			{
				compy->mood = ESCAPE_MOOD;
				--compy_scared_timer;
			}

			if (GetRandomControl() < COMPY_ATTACK_CHANCE && item->timer > 180)
				compys_attack_lara = 1;
		}
		else if (info.zone_number != info.enemy_zone)
			compy->mood = BORED_MOOD;
		else compy->mood = ATTACK_MOOD;

		switch (compy->mood)
		{
		case ATTACK_MOOD:
		{
			compy->target.x = enemy->pos.x_pos + (WALL_L * phd_sin(item->pos.y_rot + random) >> W2V_SHIFT);
			compy->target.y = enemy->pos.y_pos;
			compy->target.z = enemy->pos.z_pos + (WALL_L * phd_cos(item->pos.y_rot + random) >> W2V_SHIFT);
			break;
		}
		case STALK_MOOD:
		case ESCAPE_MOOD:
		{
			compy->target.x = item->pos.x_pos + (WALL_L * phd_sin(info.angle + 0x8000 + random) >> W2V_SHIFT);
			compy->target.z = item->pos.z_pos + (WALL_L * phd_cos(info.angle + 0x8000 + random) >> W2V_SHIFT);

			auto room_number = item->room_number;
			auto floor = GetFloor(compy->target.x, item->pos.y_pos, compy->target.z, &room_number);

			if (abs(boxes[floor->box].height - item->pos.y_pos) > STEP_L)
			{
				compy->mood = BORED_MOOD;
				item->item_flags[0] = compy_scared_timer;
			}

			break;
		}
		case BORED_MOOD:
		{
			if (compys_carcass)
			{
				compy->target.x = items[compys_carcass].pos.x_pos;
				compy->target.z = items[compys_carcass].pos.z_pos;
			}
		}
		}

		angle = CreatureTurn(item, compy->maximum_turn);

		if (info.ahead)
			head = info.angle;

		neck = -(info.angle / 4);

		++item->timer;

		if (item->hit_status && item->timer > 200 && GetRandomControl() < COMPY_ATTACK_CHANCE * 100)
			compys_attack_lara = 1;

		switch (item->current_anim_state)
		{
		case COMPY_STOP:
		{
			compy->maximum_turn = COMPY_STOP_TURN;
			compy->flags &= ~COMPY_HIT_FLAG;

			if (compy->mood == ATTACK_MOOD)
			{
				if (info.ahead && info.distance < COMPY_HIT_RANGE * 4)
				{
					if (compys_attack_lara)
						item->goal_anim_state = (GetRandomControl() < 0x4000 ? COMPY_ATTACK : COMPY_JUMP);
					else item->goal_anim_state = COMPY_STOP;
				}
				else if (info.distance > COMPY_HIT_RANGE * (9 - 4 * compys_attack_lara))
					item->goal_anim_state = COMPY_RUN;
			}
			else if (compy->mood == BORED_MOOD)
			{
				if (info.ahead && info.distance < COMPY_HIT_RANGE * 3 && compys_carcass)
					item->goal_anim_state = (GetRandomControl() < 0x4000 ? COMPY_ATTACK : COMPY_JUMP);
				else if (info.distance > COMPY_HIT_RANGE * 3)
					item->goal_anim_state = COMPY_RUN;
			}
			else item->goal_anim_state = (GetRandomControl() < COMPY_JUMP_CHANCE ? COMPY_JUMP : COMPY_RUN);

			break;
		}
		case COMPY_RUN:
		{
			compy->flags &= ~COMPY_HIT_FLAG;
			compy->maximum_turn = COMPY_RUN_TURN;

			if (info.angle < COMPY_ATTACK_ANGLE && info.angle > -COMPY_ATTACK_ANGLE && info.distance < COMPY_HIT_RANGE * (9 - 4 * compys_attack_lara))
				item->goal_anim_state = COMPY_STOP;

			break;
		}
		case COMPY_ATTACK:
		case COMPY_JUMP:
		{
			compy->maximum_turn = COMPY_RUN_TURN;

			if (!(compy->flags & COMPY_HIT_FLAG) && (item->touch_bits & COMPY_TOUCH) && compys_attack_lara)
			{
				compy->flags |= COMPY_HIT_FLAG;

				lara_item->hit_points -= COMPY_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &compy_hit, DoBloodSplat);
			}
			else if (!(compy->flags & COMPY_HIT_FLAG) && info.distance < COMPY_HIT_RANGE && info.ahead && compys_carcass && compy->mood != ATTACK_MOOD)
			{
				compy->flags |= COMPY_HIT_FLAG;
				CreatureEffect(item, &compy_hit, DoBloodSplat);
			}
		}
		}
	}

	int16_t tilt = angle >> 1;

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, neck);
	CreatureAnimation(item_number, angle, tilt);
}

void CarcassControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->status != ACTIVE)
		return;

	auto old_room = item->room_number;

	item->pos.y_pos += item->fallspeed;

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	auto h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	if (item->pos.y_pos >= h)
	{
		item->pos.y_pos = h;
		item->fallspeed = 0;
		item->pos.z_rot = 0x4000;

		return;
	}

	item->pos.z_rot += item->fallspeed;
	item->fallspeed += (room[room_number].flags & UNDERWATER) ? 1 : 8;

	auto maxfs = (room[old_room].flags & UNDERWATER) ? 64 : 512;

	if (item->fallspeed > maxfs)
		item->fallspeed = maxfs;

	if ((room[room_number].flags & UNDERWATER) && !(room[old_room].flags & UNDERWATER))
	{
		splash_setup.x = item->pos.x_pos;
		splash_setup.y = room[room_number].y;
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerXZoff = 16;
		splash_setup.InnerXZsize = 16;
		splash_setup.InnerYsize = -96;
		splash_setup.InnerXZvel = 0xa0;
		splash_setup.InnerYvel = -item->fallspeed * 72;
		splash_setup.InnerGravity = 0x80;
		splash_setup.InnerFriction = 7;
		splash_setup.MiddleXZoff = 24;
		splash_setup.MiddleXZsize = 32;
		splash_setup.MiddleYsize = -64;
		splash_setup.MiddleXZvel = 0xe0;
		splash_setup.MiddleYvel = -item->fallspeed * 36;
		splash_setup.MiddleGravity = 0x48;
		splash_setup.MiddleFriction = 8;
		splash_setup.OuterXZoff = 32;
		splash_setup.OuterXZsize = 32;
		splash_setup.OuterXZvel = 0x110;
		splash_setup.OuterFriction = 9;

		SetupSplash(&splash_setup);

		item->fallspeed = 16;
		item->pos.y_pos = room[room_number].y + 1;
	}

	carcass_item = ((room[item->room_number].flags & UNDERWATER) ? item_number : -1);
}