#include <specific/standard.h>
#include <specific/global.h>
#include <specific/fn_stubs.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "effect2.h"
#include "control.h"

enum laser_anims
{
	LASER_ROTATE
};

void ControlArea51Laser(int16_t item_number)
{
	auto item = &items[item_number];

	item->current_anim_state = LASER_ROTATE;

	int rad = (GetRandomControl() & 1) + 8,
		r = (GetRandomControl() & 3) + 24,
		g = (GetRandomControl() & 3),
		b = (GetRandomControl() & 1);

	TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos - 64, item->pos.z_pos, rad, r, g, b);

	item->mesh_bits = 0xffffffff - (GetRandomControl() & 20);

	int destx = (item->item_flags[1] & 255) << 9,
		destz = ((item->item_flags[1] >> 8) & 255) << 9,
		dx = destx - item->pos.x_pos,
		dz = destz - item->pos.z_pos;

	if (abs(dx) < 768 && abs(dz) < 768)
	{
		dx = ((item->item_flags[0] & 255) << 9) + ((item->item_flags[2] * -2560 * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
		dz = (((item->item_flags[0] >> 8) & 255) << 9) + ((item->item_flags[2] * -2560 * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);

		item->item_flags[1] = ((dx >> 9) & 255) | (((dz >> 9) & 255) << 8);
		item->shadeB = 32;
	}

	if (item->item_flags[2] == 1)
	{
		if (item->shadeB)
		{
			if (item->item_flags[3] == 0)
			{
				if (--item->shadeB == 0)
					item->item_flags[2] = -item->item_flags[2];
			}
			else if (item->item_flags[3] <= 4)
				item->item_flags[3] = 0;
			else item->item_flags[3] -= item->item_flags[3] >> 2;
		}
		else
		{
			item->item_flags[3] += 5;
			if (item->item_flags[3] > 512)
				item->item_flags[3] = 512;
		}
	}
	else
	{
		if (item->shadeB)
		{
			if (item->item_flags[3] == 0)
			{
				if (--item->shadeB == 0)
					item->item_flags[2] = -item->item_flags[2];
			}
			else if (item->item_flags[3] >= -4)
				item->item_flags[3] = 0;
			else item->item_flags[3] -= item->item_flags[3] >> 2;
		}
		else
		{
			item->item_flags[3] -= 5;
			if (item->item_flags[3] < -512)
				item->item_flags[3] = -512;
		}
	}

	item->pos.z_pos += (item->item_flags[3] * phd_cos(item->pos.y_rot)) >> (W2V_SHIFT + 2);
	item->pos.x_pos += (item->item_flags[3] * phd_sin(item->pos.y_rot)) >> (W2V_SHIFT + 2);

	auto room_number = item->room_number;

	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);

	if (TestBoundsCollide(item, lara_item, 64))
	{
		lara_item->hit_points -= 25;
		DoLotsOfBloodD(lara_item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, lara_item->pos.z_pos, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, lara_item->room_number, 3);
	}

	AnimateItem(item);
}

void InitialiseArea51Laser(int16_t item_number)
{
	auto item = &items[item_number];

	int dx = item->pos.x_pos + ((2560 * phd_sin(item->pos.y_rot)) >> W2V_SHIFT),
		dz = item->pos.z_pos + ((2560 * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);

	item->item_flags[0] = ((item->pos.x_pos >> 9) & 255) | (((item->pos.z_pos >> 9) & 255) << 8);	// setup X & Z origin.
	item->item_flags[1] = ((dx >> 9) & 255) | (((dz >> 9) & 255) << 8);								// setup X & Z destination.
	item->item_flags[2] = 1;																		// next destination. 1 = with y_rot, -1 = against y_rot.
	item->item_flags[3] = 0;																		// speed, because, get this, you CAN'T dynamically change the 'speed' field of animating items!!! Ha,Ha...
	item->shadeB = 0;																				// clear stop timer.
}