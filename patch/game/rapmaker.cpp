#include <specific/stypes.h>

#include "objects.h"
#include "lara.h"
#include "lot.h"
#include "types.h"

#define MAX_RAPTORS		5
#define LARA_TOO_CLOSE	SQUARE(WALL_L * 4)

int16_t RaptorItem[3] = { NO_ITEM, NO_ITEM, NO_ITEM };

void RaptorEmitterControl(int16_t item_num)
{
	auto item = &items[item_num];

	if (!(item->active) || item->timer <= 0)
		return;

	if (RaptorItem[0] == NO_ITEM)
	{
		for (int target_number = 0, i = 0; target_number < level_items; ++target_number)
		{
			auto target = &items[target_number];

			if (target->object_number == RAPTOR && target->ai_bits == MODIFY)
				RaptorItem[i++] = target_number;

			if (i > 2)
				return;
		}

		return;
	}
	
	if (items[RaptorItem[0]].data != nullptr && items[RaptorItem[1]].data != nullptr && items[RaptorItem[2]].data != nullptr)
		return;

	int z = lara_item->pos.z_pos - item->pos.z_pos,
		x = lara_item->pos.x_pos - item->pos.x_pos,
		distance = (z * z) + (x * x);

	if (distance < LARA_TOO_CLOSE)
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

	int raptor_num = 0;

	for (int i = 0; i < 3; ++i, ++raptor_num)
		if (!items[RaptorItem[i]].data)
			break;

	auto raptor = &items[RaptorItem[raptor_num]];

	raptor->pos = item->pos;
	raptor->anim_number = objects[raptor->object_number].anim_index;
	raptor->frame_number = anims[raptor->anim_number].frame_base;
	raptor->current_anim_state = raptor->goal_anim_state = anims[raptor->anim_number].current_anim_state;
	raptor->required_anim_state = 0;
	raptor->flags &= ~(ONESHOT | KILLED_ITEM | INVISIBLE);
	raptor->data = 0;
	raptor->status = ACTIVE;
	raptor->mesh_bits = 0xffffffff;
	raptor->hit_points = objects[raptor->object_number].hit_points;
	raptor->collidable = 1;

	if (raptor->active)
		RemoveActiveItem(RaptorItem[raptor_num]);

	AddActiveItem(RaptorItem[raptor_num]);
	ItemNewRoom(RaptorItem[raptor_num], item->room_number);
	EnableBaddieAI(RaptorItem[raptor_num], 1);
}

void InitialiseRaptorEmitter(int16_t item_number)
{
	items[item_number].item_flags[0] = (item_number & 0x3) * 96;
}