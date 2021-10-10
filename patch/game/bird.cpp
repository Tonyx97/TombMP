#include "objects.h"
#include "lara.h"

#define BIRD_DAMAGE			20
#define BIRD_TURN			(ONE_DEGREE * 3)
#define BIRD_ATTACK_RANGE	SQUARE(WALL_L / 2)
#define BIRD_TAKEOFF_CHANCE 0x100
#define BIRD_DIE_ANIM		8
#define CROW_DIE_ANIM		1
#define BIRD_START_ANIM		5
#define CROW_START_ANIM		14

enum bird_anims
{
	BIRD_EMPTY,
	BIRD_FLY,
	BIRD_STOP,
	BIRD_GLIDE,
	BIRD_FALL,
	BIRD_DEATH,
	BIRD_ATTACK,
	BIRD_EAT
};

BITE_INFO bird_bite = { 15, 46, 21, 6 },
		  crow_bite = { 2, 10, 60, 14 };

void InitialiseVulture(int16_t item_number)
{
	InitialiseCreature(item_number);

	if (auto item = &items[item_number]; item->object_number == CROW)
	{
		item->anim_number = objects[CROW].anim_index + CROW_START_ANIM;
		item->frame_number = anims[item->anim_number].frame_base;
		item->current_anim_state = item->goal_anim_state = BIRD_EAT;
	}
	else
	{
		item->anim_number = objects[VULTURE].anim_index + BIRD_START_ANIM;
		item->frame_number = anims[item->anim_number].frame_base;
		item->current_anim_state = item->goal_anim_state = BIRD_STOP;
	}
}

void VultureControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto bird = (CREATURE_INFO*)item->data;

	int16_t angle = 0;

	if (!bird)
		return;

	if (item->hit_points <= 0)
	{
		switch (item->current_anim_state)
		{
		case BIRD_FALL:
		{
			if (item->pos.y_pos > item->floor)
			{
				item->pos.y_pos = item->floor;
				item->gravity_status = 0;
				item->fallspeed = 0;
				item->goal_anim_state = BIRD_DEATH;
			}

			break;
		}
		case BIRD_DEATH:
			item->pos.y_pos = item->floor;
			break;
		default:
		{
			item->anim_number = (item->object_number == CROW ? objects[CROW].anim_index + CROW_DIE_ANIM : objects[VULTURE].anim_index + BIRD_DIE_ANIM);
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BIRD_FALL;
			item->gravity_status = 1;
			item->speed = 0;
			break;
		}
		}

		item->pos.x_rot = 0;
	}
	else
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, BIRD_TURN);

		switch (item->current_anim_state)
		{
		case BIRD_EAT:
		{
			item->pos.y_pos = item->floor;

			if (bird->mood != BORED_MOOD)
				item->goal_anim_state = BIRD_FLY;

			break;
		}
		case BIRD_STOP:
		{
			item->pos.y_pos = item->floor;

			if (bird->mood != BORED_MOOD)
				item->goal_anim_state = BIRD_FLY;

			break;
		}
		case BIRD_FLY:
		{
			bird->flags = 0;

			if (item->required_anim_state)
				item->goal_anim_state = item->required_anim_state;
			if (bird->mood == BORED_MOOD)
				item->goal_anim_state = BIRD_STOP;
			else if (info.ahead && info.distance < BIRD_ATTACK_RANGE)
				item->goal_anim_state = BIRD_ATTACK;
			else item->goal_anim_state = BIRD_GLIDE;

			break;
		}
		case BIRD_GLIDE:
		{
			if (bird->mood == BORED_MOOD)
			{
				item->required_anim_state = BIRD_STOP;
				item->goal_anim_state = BIRD_FLY;
			}
			else if (info.ahead && info.distance < BIRD_ATTACK_RANGE)
				item->goal_anim_state = BIRD_ATTACK;

			break;
		}
		case BIRD_ATTACK:
		{
			if (!bird->flags && item->touch_bits)
			{
				lara_item->hit_points -= BIRD_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &(item->object_number == CROW ? crow_bite : bird_bite), DoBloodSplat);

				bird->flags = 1;
			}

			break;
		}
		}
	}

	CreatureAnimation(item_number, angle, 0);
}