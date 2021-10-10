#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "control.h"

#define TARGET_UP_TIME		 200
#define TARGET_DROP_SPEED	(ONE_DEGREE * 2)
#define TARGET_HIT_POINTS	8
#define HIT1_HP				6
#define HIT2_HP				4
#define HIT3_HP				2

enum target_anims
{
	TARGET_RISE,
	TARGET_HIT1,
	TARGET_HIT2,
	TARGET_HIT3
};

void InitialiseTarget(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->active)
		RemoveActiveItem(item_number);

	item->anim_number = objects[item->object_number].anim_index;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
	item->required_anim_state = 0;
	item->pos.x_rot = item->pos.z_rot = 0;
	item->status = NOT_ACTIVE;
	item->active = item->flags = item->timer = item->item_flags[2] = 0;
	item->hit_points = objects[item->object_number].hit_points;
	item->data = nullptr;
}

void TargetControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->status != ACTIVE)
		return;

	if (item->hit_points != DONT_TARGET)
	{
		if (item->hit_status)
			g_audio->play_sound(119, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

		switch (item->current_anim_state)
		{
		case TARGET_RISE:
		{
			if (item->hit_points < HIT1_HP)
			{
				item->hit_points = HIT1_HP;
				item->goal_anim_state = TARGET_HIT1;
			}

			break;
		}
		case TARGET_HIT1:
		{
			if (item->hit_points < HIT2_HP)
			{
				item->anim_number = objects[item->object_number].anim_index + 2;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = TARGET_HIT2;
				item->hit_points = HIT2_HP;
			}

			break;
		}
		case TARGET_HIT2:
		{
			if (item->hit_points < HIT3_HP)
			{
				item->anim_number = objects[item->object_number].anim_index + 3;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = TARGET_HIT3;
				item->hit_points = HIT3_HP;
			}

			break;
		}
		case TARGET_HIT3:
		{
			if (item->hit_points <= 0 && item->hit_points != DONT_TARGET)
			{
				lara.target = nullptr;
				item->hit_points = DONT_TARGET;
				item->item_flags[0] = 10 * (ONE_DEGREE);	// X rot fallspeed.
				item->item_flags[1] = 0;					// Number of bounces.
				item->item_flags[2] = 1;					// Shot it to death!
			}
			break;
		}
		}

		++item->timer;

		if (item->timer > 10 * 30)	// been alive for 10 seconds ?
		{
			lara.target = nullptr;
			item->hit_points = DONT_TARGET;
			item->item_flags[0] = 1 * (ONE_DEGREE);	// X rot fallspeed.
			item->item_flags[1] = 0;					// Number of bounces.
			item->item_flags[2] = 0;					// Shot it to death!
		}
	}

	if (item->hit_points == DONT_TARGET && item->item_flags[2] == 1)	// Shot it to death ?
	{
		item->pos.x_rot += item->item_flags[0];
		item->item_flags[0] += (ONE_DEGREE << 2) >> item->item_flags[1];

		if (item->pos.x_rot > 0x3800)
		{
			if (item->item_flags[1] == 2)
			{
				item->pos.x_rot = 0x3800;

				RemoveActiveItem(item_number);

				return;
			}
			else
			{
				if (item->item_flags[1] == 1)
					g_audio->play_sound(120, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				item->pos.x_rot = 0x3800;
				item->item_flags[0] = -item->item_flags[0] >> 2;
				++item->item_flags[1];
			}
		}
	}
	else if (item->hit_points == DONT_TARGET && item->item_flags[2] == 0)	// Timer ran out ?
	{
		item->pos.x_rot -= item->item_flags[0];
		item->item_flags[0] += (ONE_DEGREE >> 1) >> item->item_flags[1];

		if (item->pos.x_rot < -0x2a00)
		{
			if (item->item_flags[1] == 2)
			{
				item->pos.x_rot = -0x2a00;

				RemoveActiveItem(item_number);

				return;
			}
			else
			{
				g_audio->play_sound(119, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				item->pos.x_rot = -0x2a00;
				item->item_flags[0] = -item->item_flags[0] >> 2;
				++item->item_flags[1];
			}
		}
	}

	AnimateItem(item);
}

void ResetTargets()
{
	for (int i = 0; i < level_items; ++i)
		if (items[i].object_number == TARGETS)
			InitialiseTarget(i);
}