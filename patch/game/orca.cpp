#include "lara.h"
#include "control.h"
#include "effect2.h"
#include "sphere.h"

#define ORCA_BITE			400
#define ORCA_TOUCH			0x3400
#define ORCA_FAST_RANGE		SQUARE(WALL_L * 3 / 2)
#define ORCA_ATTACK1_RANGE	SQUARE(WALL_L * 3 / 4)
#define ORCA_ATTACK2_RANGE	SQUARE(WALL_L * 4 / 3)
#define ORCA_FAST_TURN		(ONE_DEGREE * 2)
#define ORCA_SLOW_TURN		(ONE_DEGREE * 2)
#define ORCA_ATTACK1_CHANCE 0x800
#define ORCA_DIE_ANIM		4
#define ORCA_KILL_ANIM		19

enum orca_anim
{
	ORCA_SLOW,
	ORCA_FAST,
	ORCA_JUMP,
	ORCA_SPLASH,
	ORCA_SLOW_BUTT,
	ORCA_FAST_BUTT,
	ORCA_BREACH,
	ORCA_ROLL180
};

void OrcaControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto orca = (CREATURE_INFO*)item->data;

	bool lara_alive = (lara_item->hit_points > 0);

	item->hit_points = DONT_TARGET;

	AI_INFO info;

	CreatureAIInfo(item, &info);
	GetCreatureMood(item, &info, VIOLENT);

	if (!(room[lara_item->room_number].flags & UNDERWATER) && lara.skidoo == NO_ITEM)
		orca->mood = BORED_MOOD;

	CreatureMood(item, &info, VIOLENT);

	int16_t angle = CreatureTurn(item, orca->maximum_turn);

	switch (item->current_anim_state)
	{
	case ORCA_SLOW:
	{
		orca->flags = 0;
		orca->maximum_turn = ORCA_SLOW_TURN;

		if (orca->mood != BORED_MOOD)
		{
			if (info.ahead && info.distance < ORCA_ATTACK1_RANGE)
				item->goal_anim_state = ORCA_SLOW_BUTT;
			else if (orca->mood == ESCAPE_MOOD)
				item->goal_anim_state = ORCA_FAST;
			else if (info.distance > ORCA_FAST_RANGE)
			{
				if (info.angle < 0x5000 && info.angle >-0x5000)
					item->goal_anim_state = ((GetRandomControl() & 0x3F) ? ORCA_FAST : ORCA_BREACH);
				else item->goal_anim_state = ORCA_ROLL180;
			}
		}
		else item->goal_anim_state = ((GetRandomControl() & 0xFF) ? ORCA_SLOW : ORCA_JUMP);

		break;
	}
	case ORCA_FAST:
	{
		orca->flags = 0;
		orca->maximum_turn = ORCA_FAST_TURN;

		if (orca->mood == BORED_MOOD)
			item->goal_anim_state = ((GetRandomControl() & 0xFF) ? ORCA_SLOW : ORCA_JUMP);
		else if (orca->mood == ESCAPE_MOOD)
			break;
		else if (info.ahead && info.distance < ORCA_FAST_RANGE && info.zone_number == info.enemy_zone)
			item->goal_anim_state = ORCA_SLOW;
		else if (info.distance > ORCA_FAST_RANGE && !(GetRandomControl() & 0x7F))
			item->goal_anim_state = ORCA_JUMP;
		else if (info.distance > ORCA_FAST_RANGE && !info.ahead)
			item->goal_anim_state = ORCA_SLOW;

		break;
	}
	case ORCA_ROLL180:
	{
		orca->maximum_turn = 0;

		if (item->frame_number == anims[item->anim_number].frame_base + 59)
		{
			item->pos.y_rot += (item->pos.y_rot < 0 ? 0x8000 : -0x8000);
			item->pos.x_rot = -item->pos.x_rot;
		}
	}
	}

	CreatureAnimation(item_number, angle, 0);
	CreatureUnderwater(item, WALL_L / 5);

	if (wibble & 4)
	{
		PHD_VECTOR pos { -32, 16, -300 };
		GetJointAbsPosition(item, &pos, 5);

		auto room_number = item->room_number;
		auto floor = GetFloor(pos.x, pos.y, pos.z, &room_number);

		if (int wh = GetWaterHeight(pos.x, pos.y, pos.z, room_number); wh != NO_HEIGHT)
			if (pos.y < wh)
				auto r = SetupRipple(pos.x, wh, pos.z, -2 - (GetRandomControl() & 1), 0)->init = 0;
	}
}