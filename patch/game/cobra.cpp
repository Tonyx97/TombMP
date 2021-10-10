#include <math.h>

#include "lara.h"
#include "game.h"

#define COBRA_DAMAGE 80

#define COBRA_TURN (ONE_DEGREE*10)

#define COBRA_FORGET_RADIUS SQUARE(WALL_L * 3)
#define COBRA_OUTER_RADIUS	SQUARE(WALL_L * 3 / 2)
#define COBRA_INNER_RADIUS	SQUARE(WALL_L)

#define RATTLER_FORGET_RADIUS	SQUARE(WALL_L * 5 / 2)
#define RATTLER_OUTER_RADIUS	SQUARE(WALL_L * 5 / 4)
#define RATTLER_INNER_RADIUS	SQUARE(WALL_L * 2 / 3)

#define COBRA_UPSET_SPEED	15
#define COBRA_HIT_RANGE		(STEP_L*2)

#define COBRA_TOUCH 0x2000

#define COBRA_DIE_ANIM	4
#define COBRA_DOWN_ANIM 2

enum cobra_anims
{
	COBRA_RISE,
	COBRA_WAIT,
	COBRA_STRIKE,
	COBRA_DOWN,
	COBRA_DEATH
};

void InitialiseCobra(int16_t item_number);
void CobraControl(int16_t item_number);

BITE_INFO cobra_hit = { 0, 0, 0, 13 };

void InitialiseCobra(int16_t item_number)
{
	InitialiseCreature(item_number);

	auto item = &items[item_number];

	item->anim_number = objects[item->object_number].anim_index + COBRA_DOWN_ANIM;
	item->frame_number = anims[item->anim_number].frame_base + 45;
	item->current_anim_state = item->goal_anim_state = COBRA_DOWN;
	item->item_flags[2] = item->hit_points;
	item->hit_points = DONT_TARGET;
}

void CobraControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	if (!item->data)
		return;

	auto cobra = (CREATURE_INFO*)item->data;

	int16_t angle = 0;

	if (item->hit_points <= 0 && item->hit_points != DONT_TARGET)
	{
		if (item->current_anim_state != COBRA_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + COBRA_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = COBRA_DEATH;
		}
	}
	else
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);

		info.angle += 0xC00;

		cobra->target.x = lara_item->pos.x_pos;
		cobra->target.z = lara_item->pos.z_pos;

		angle = CreatureTurn(item, cobra->maximum_turn);

		if (abs(info.angle) < COBRA_TURN)
			item->pos.y_rot += info.angle;
		else if (info.angle < 0)
			item->pos.y_rot -= COBRA_TURN;
		else item->pos.y_rot += COBRA_TURN;

		switch (item->current_anim_state)
		{
		case COBRA_WAIT:
		{
			cobra->flags = 0;

			if (info.distance > RATTLER_FORGET_RADIUS)
				item->goal_anim_state = COBRA_DOWN;
			else if ((lara_item->hit_points > 0) && ((info.ahead && info.distance < RATTLER_INNER_RADIUS) || item->hit_status || (lara_item->speed > COBRA_UPSET_SPEED)))
				item->goal_anim_state = COBRA_STRIKE;

			break;
		}
		case COBRA_DOWN:
		{
			cobra->flags = 0;

			if (item->hit_points != DONT_TARGET)
			{
				item->item_flags[2] = item->hit_points;
				item->hit_points = DONT_TARGET;
			}

			if (info.distance < RATTLER_OUTER_RADIUS && lara_item->hit_points > 0)
			{
				item->goal_anim_state = COBRA_RISE;
				item->hit_points = item->item_flags[2];
			}

			break;
		}
		case COBRA_STRIKE:
		{
			if (cobra->flags != 1 && (item->touch_bits & COBRA_TOUCH))
			{
				cobra->flags = 1;
				lara_item->hit_points -= COBRA_DAMAGE;
				lara_item->hit_status = 1;
				lara.poisoned = 0x100;

				CreatureEffect(item, &cobra_hit, DoBloodSplat);
			}

			break;
		}
		case COBRA_RISE:
			item->hit_points = item->item_flags[2];
			break;
		}
	}

	CreatureAnimation(item_number, angle, 0);
}