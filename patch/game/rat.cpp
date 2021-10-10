#include "objects.h"
#include "lara.h"
#include "control.h"

#define MOUSE_DAMAGE 	20
#define MOUSE_TOUCH		(0x7f)

#define MOUSE_WAIT1_CHANCE 0x500
#define MOUSE_WAIT2_CHANCE (MOUSE_WAIT1_CHANCE + 0x500)

#define MOUSE_ATTACK_RANGE SQUARE(WALL_L / 3)

#define MOUSE_DIE_ANIM 9

#define MOUSE_RUN_TURN (ONE_DEGREE * 6)

enum mouse_anims
{
	MOUSE_EMPTY,
	MOUSE_RUN,
	MOUSE_STOP,
	MOUSE_WAIT1,
	MOUSE_WAIT2,
	MOUSE_ATTACK,
	MOUSE_DEATH
};

BITE_INFO mouse_bite = { 0, 0, 57, 2 };

void MouseControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto mouse = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   angle = 0;

	if (!mouse)
		return;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != MOUSE_DEATH)
		{
			item->anim_number = objects[SMALL_RAT].anim_index + MOUSE_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = MOUSE_DEATH;
		}
	}
	else
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, MOUSE_RUN_TURN);

		switch (item->current_anim_state)
		{
		case MOUSE_WAIT2:
		{
			if (mouse->mood == BORED_MOOD || mouse->mood == STALK_MOOD)
			{
				int random = GetRandomControl();

				if (random < MOUSE_WAIT1_CHANCE)		item->required_anim_state = MOUSE_WAIT1;
				else if (random > MOUSE_WAIT2_CHANCE)	item->required_anim_state = MOUSE_RUN;
			}
			else if (info.distance < MOUSE_ATTACK_RANGE)
				item->required_anim_state = MOUSE_ATTACK;
			else item->required_anim_state = MOUSE_RUN;

			if (item->required_anim_state)
				item->goal_anim_state = MOUSE_STOP;

			break;
		}
		case MOUSE_STOP:
		{
			mouse->maximum_turn = 0;

			if (item->required_anim_state)
				item->goal_anim_state = item->required_anim_state;

			break;
		}
		case MOUSE_RUN:
		{
			mouse->maximum_turn = MOUSE_RUN_TURN;

			if (mouse->mood == BORED_MOOD || mouse->mood == STALK_MOOD)
			{
				if (int random = GetRandomControl(); random < MOUSE_WAIT1_CHANCE)
				{
					item->required_anim_state = MOUSE_WAIT1;
					item->goal_anim_state = MOUSE_STOP;
				}
				else if (random < MOUSE_WAIT2_CHANCE)
					item->goal_anim_state = MOUSE_STOP;
			}
			else if (info.ahead && info.distance < MOUSE_ATTACK_RANGE)
				item->goal_anim_state = MOUSE_STOP;

			break;
		}
		case MOUSE_ATTACK:
		{
			if (!item->required_anim_state && (item->touch_bits & MOUSE_TOUCH))
			{
				CreatureEffect(item, &mouse_bite, DoBloodSplat);
				lara_item->hit_points -= MOUSE_DAMAGE;
				lara_item->hit_status = 1;
				item->required_anim_state = MOUSE_STOP;
			}

			break;
		}
		case MOUSE_WAIT1:
			if (GetRandomControl() < MOUSE_WAIT1_CHANCE)
				item->goal_anim_state = MOUSE_STOP;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(item_number, angle, 0);
}