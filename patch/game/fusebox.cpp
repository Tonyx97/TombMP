#include "objects.h"
#include "items.h"
#include "control.h"
#include "effect2.h"
#include "gameflow.h"
#include "game.h"

void InitialiseFusebox(int16_t item_number) {}

void ControlFusebox(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->hit_points <= 0 && item->anim_number == objects[item->object_number].anim_index)
	{
		int x = (item->pos.x_pos + (420 * phd_sin(item->pos.y_rot + 0x8000))) >> W2V_SHIFT,
			z = (item->pos.z_pos + (420 * phd_cos(item->pos.y_rot + 0x8000))) >> W2V_SHIFT;

		item->anim_number = objects[item->object_number].anim_index + 1;
		item->frame_number = anims[item->anim_number].frame_base;

		TriggerExplosionSparks(x, item->pos.y_pos - (STEP_L * 3), z, 2, 0, 0, item->room_number);
		TriggerExplosionSmoke(x, item->pos.y_pos - (STEP_L * 3), z, 0);

		if (enable_fusebox)
		{
			auto room_number = item->room_number;
			auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
			auto height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

			TestTriggers(trigger_index, 1);

			auto target_item = &items[0];

			for (int i = 0; i < level_items; ++i, ++target_item)
			{
				if (target_item->object_number == LON_BOSS)
				{
					target_item->item_flags[2] = 1;
					break;
				}
			}
		}
	}

	AnimateItem(item);
}