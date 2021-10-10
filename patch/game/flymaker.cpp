#include "objects.h"
#include "lara.h"
#include "lot.h"
#include "types.h"

#define LARA_TOO_FAR SQUARE(WALL_L * 12)

int16_t FlyItem[3] = { NO_ITEM, NO_ITEM, NO_ITEM };

void FlyEmitterControl(int16_t item_num)
{
	auto item = &items[item_num];

	if (!(item->active) || item->hit_points <= 0 || item->timer <= 0)
		return;

	if (FlyItem[0] == NO_ITEM)
	{
		for (int target_number = 0, i = 0; target_number < level_items; ++target_number)
		{
			auto target = &items[target_number];

			if (target->object_number == MUTANT1 && target->ai_bits == MODIFY)
			{
				FlyItem[i++] = target_number;
				KillItem(target_number);
			}

			if (i > 2)
				return;
		}

		return;
	}

	if (items[FlyItem[0]].status == ACTIVE && items[FlyItem[1]].status == ACTIVE && items[FlyItem[2]].status == ACTIVE)
		return;

	int x = lara_item->pos.x_pos - item->pos.x_pos,
		z = lara_item->pos.z_pos - item->pos.z_pos;

	if (z > 32000 || z < -32000 || x>32000 || x < -32000)
		return;

	if (((z * z) + (x * x)) > LARA_TOO_FAR)
		return;

	if (item->item_flags[0] <= 0)
	{
		item->item_flags[0] = 255;
		item->timer -= 30;
	}
	else
	{
		--item->item_flags[0];
		return;
	}

	int fly_num = 0;

	for (int i = 0; i < 3; ++i, ++fly_num)
		if (!items[FlyItem[i]].data && item[FlyItem[i]].status != ACTIVE)
			break;

	if (fly_num > 2)
		return;

	auto fly = &items[FlyItem[fly_num]];

	fly->pos = item->pos;
	fly->anim_number = objects[fly->object_number].anim_index;
	fly->frame_number = anims[fly->anim_number].frame_base;
	fly->current_anim_state = fly->goal_anim_state = anims[fly->anim_number].current_anim_state;
	fly->required_anim_state = 0;
	fly->flags &= ~(ONESHOT | KILLED_ITEM | INVISIBLE);
	fly->ai_bits = MODIFY;
	fly->data = 0;
	fly->status = ACTIVE;
	fly->mesh_bits = 0xffffffff;
	fly->hit_points = objects[fly->object_number].hit_points;
	fly->collidable = 1;

	if (fly->active)
		RemoveActiveItem(FlyItem[fly_num]);

	AddActiveItem(FlyItem[fly_num]);
	ItemNewRoom(FlyItem[fly_num], item->room_number);
	EnableBaddieAI(FlyItem[fly_num], 1);
}