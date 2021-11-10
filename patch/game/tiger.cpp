#include "objects.h"
#include "lara.h"
#include "control.h"

#define TIGER_BITE_DAMAGE		90
#define TIGER_TOUCH				(0x7fdc000)
#define TIGER_DIE_ANIM			11
#define TIGER_WALK_TURN			(3 * ONE_DEGREE)
#define TIGER_RUN_TURN			(6 * ONE_DEGREE)
#define TIGER_ATTACK1_RANGE		SQUARE(WALL_L / 3)
#define TIGER_ATTACK2_RANGE		SQUARE(WALL_L * 3 / 2)
#define TIGER_ATTACK3_RANGE		SQUARE(WALL_L)
#define TIGER_ROAR_CHANCE		0x60
#define TIGER_WALK_CHANCE		(TIGER_ROAR_CHANCE + 0x400)

enum tiger_anims
{
	TIGER_EMPTY,
	TIGER_STOP,
	TIGER_WALK,
	TIGER_RUN,
	TIGER_WAIT,
	TIGER_ROAR,
	TIGER_ATTACK1,
	TIGER_ATTACK2,
	TIGER_ATTACK3,
	TIGER_DEATH
};

BITE_INFO tiger_bite = { 19, -13, 3, 26 };

void TigerControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto tiger = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		    angle = 0,
		    tilt = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != TIGER_DEATH)
		{
			item->anim_number = objects[TIGER].anim_index + TIGER_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = TIGER_DEATH;
		}
	}
	else
	{
		if (!tiger)
			return;

		AI_INFO info;

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);

		if (info.ahead)
			head = info.angle;

		if (tiger->alerted && info.zone_number != info.enemy_zone)
			tiger->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, tiger->maximum_turn);

		switch (item->current_anim_state)
		{
		case TIGER_STOP:
		{
			tiger->maximum_turn = 0;
			tiger->flags = 0;

			if (tiger->mood == ESCAPE_MOOD)
			{
				if (lara.target != item && info.ahead)
					item->goal_anim_state = TIGER_STOP;
				else item->goal_anim_state = TIGER_RUN;
			}
			else if (tiger->mood == BORED_MOOD)
			{
				if (int random = (int16_t)GetRandomControl(); random < TIGER_ROAR_CHANCE)
					item->goal_anim_state = TIGER_ROAR;
				else if (random < TIGER_WALK_CHANCE)
					item->goal_anim_state = TIGER_WALK;
			}
			else if (info.bite && info.distance < TIGER_ATTACK1_RANGE)
				item->goal_anim_state = TIGER_ATTACK1;
			else if (info.bite && info.distance < TIGER_ATTACK3_RANGE)
			{
				tiger->maximum_turn = TIGER_WALK_TURN;
				item->goal_anim_state = TIGER_ATTACK3;
			}
			else if (item->required_anim_state)
				item->goal_anim_state = item->required_anim_state;
			else if (tiger->mood != ATTACK_MOOD && GetRandomControl() < TIGER_ROAR_CHANCE)
				item->goal_anim_state = TIGER_ROAR;
			else item->goal_anim_state = TIGER_RUN;

			break;
		}
		case TIGER_WALK:
		{
			tiger->maximum_turn = TIGER_WALK_TURN;

			if (tiger->mood == ESCAPE_MOOD || tiger->mood == ATTACK_MOOD)
				item->goal_anim_state = TIGER_RUN;
			else if (GetRandomControl() < TIGER_ROAR_CHANCE)
			{
				item->goal_anim_state = TIGER_STOP;
				item->required_anim_state = TIGER_ROAR;
			}

			break;
		}
		case TIGER_RUN:
		{
			tiger->maximum_turn = TIGER_RUN_TURN;

			if (tiger->mood == BORED_MOOD)
				item->goal_anim_state = TIGER_STOP;
			else if (tiger->flags && info.ahead)
				item->goal_anim_state = TIGER_STOP;
			else if (info.bite && info.distance < TIGER_ATTACK2_RANGE)
				item->goal_anim_state = (lara_item->speed == 0 ? TIGER_STOP : TIGER_ATTACK2);
			else if (tiger->mood != ATTACK_MOOD && GetRandomControl() < TIGER_ROAR_CHANCE)
			{
				item->required_anim_state = TIGER_ROAR;
				item->goal_anim_state = TIGER_STOP;
			}
			else if (tiger->mood == ESCAPE_MOOD && lara.target != item && info.ahead)
				item->goal_anim_state = TIGER_STOP;

			tiger->flags = 0;

			break;
		}
		case TIGER_ATTACK1:
		case TIGER_ATTACK2:
		case TIGER_ATTACK3:
		{
			auto enemy = tiger->enemy;

			if (enemy == lara_item)
			{
				if (!tiger->flags && (item->touch_bits & TIGER_TOUCH))
				{
					lara_item->hit_status = 1;
					lara_item->hit_points -= TIGER_BITE_DAMAGE;

					CreatureEffect(item, &tiger_bite, DoBloodSplat);

					tiger->flags = 1;
				}
			}
			else
			{
				if (!tiger->flags && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < TIGER_ATTACK2_RANGE &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= TIGER_ATTACK2_RANGE &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < TIGER_ATTACK2_RANGE)
					{
						enemy->hit_points -= TIGER_BITE_DAMAGE >> 1;
						enemy->hit_status = 1;
						tiger->flags = 1;

						CreatureEffect(item, &tiger_bite, DoBloodSplat);
					}
				}
			}
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(item_number, angle, tilt);
}