#include "objects.h"
#include "lara.h"

#define CROC_BITE_DAMAGE	100
#define CROC_TOUCH			(0x3fc)
#define CROC_TURN			(ONE_DEGREE * 3)
#define CROC_FLOAT_SPEED	(WALL_L / 32)
#define CROC_DIE_ANIM		4

BITE_INFO croc_bite = { 5, -21, 467, 9 };

enum croc_anims
{
	CROC_EMPTY,
	CROC_SWIM,
	CROC_ATTACK,
	CROC_DEATH
};

void CrocControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto croc = (CREATURE_INFO*)item->data;
	if (!croc)
		return;

	int16_t head = 0,
		   angle = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != CROC_DEATH)
		{
			item->anim_number = objects[CROCODILE].anim_index + CROC_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = CROC_DEATH;
			item->hit_points = DONT_TARGET;
		}

		CreatureFloat(item_number);

		return;
	}

	AI_INFO info;

	CreatureAIInfo(item, &info);

	if (info.ahead)
		head = info.angle;

	GetCreatureMood(item, &info, VIOLENT);

	if (!(room[lara_item->room_number].flags & UNDERWATER) && lara.skidoo == NO_ITEM)
		croc->mood = BORED_MOOD;

	CreatureMood(item, &info, VIOLENT);
	CreatureTurn(item, CROC_TURN);

	switch (item->current_anim_state)
	{
	case CROC_SWIM:
	{
		if (info.bite && item->touch_bits)
			item->goal_anim_state = CROC_ATTACK;
		break;
	}
	case CROC_ATTACK:
	{
		if (item->frame_number == anims[item->anim_number].frame_base)
			item->required_anim_state = 0;

		if (info.bite && item->touch_bits)
		{
			if (!item->required_anim_state)
			{
				CreatureEffect(item, &croc_bite, DoBloodSplat);

				lara_item->hit_points -= CROC_BITE_DAMAGE;
				lara_item->hit_status = 1;

				item->required_anim_state = CROC_SWIM;
			}
		}
		else item->goal_anim_state = CROC_SWIM;

		break;
	}
	}

	CreatureJoint(item, 0, head);
	CreatureUnderwater(item, WALL_L / 4);
	CreatureAnimation(item_number, angle, 0);
}