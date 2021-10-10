#include "objects.h"
#include "lara.h"
#include "control.h"
#include "lasers.h"

#define SIGN_BIT 0x80000000

extern int los_rooms[], number_los_rooms;

bool LaraOnLOS(GAME_VECTOR* start, GAME_VECTOR* target)
{
	int dx = target->x - start->x,
		dy = target->y - start->y,
		dz = target->z - start->z;

	auto item = lara_item;
	auto bounds = GetBoundsAccurate(item);
	auto direction = (uint16_t)(item->pos.y_rot + 0x2000) / 0x4000;

	int16_t* zextent,
		  * xextent;

	if (direction & 1)
	{
		zextent = &bounds[0];
		xextent = &bounds[4];
	}
	else
	{
		zextent = &bounds[4];
		xextent = &bounds[0];
	}

	int failure = 0;

	if (ABS(dz) > ABS(dx))
	{
		int distance = item->pos.z_pos + zextent[0] - start->z;

		for (int j = 0; j < 2; ++j)
		{
			if ((distance & SIGN_BIT) == (dz & SIGN_BIT))
			{
				int y = dy * distance / dz;

				if (y > item->pos.y_pos + bounds[2] - start->y && y < item->pos.y_pos + bounds[3] - start->y)
				{
					int x = dx * distance / dz;

					if (x < item->pos.x_pos + xextent[0] - start->x)
						failure |= 0x1;
					else if (x > item->pos.x_pos + xextent[1] - start->x)
						failure |= 0x2;
					else return true;
				}
			}

			distance = item->pos.z_pos + zextent[1] - start->z;
		}

		if (failure == 0x3)
			return true;
	}
	else
	{
		int distance = item->pos.x_pos + xextent[0] - start->x;

		for (int j = 0; j < 2; ++j)
		{
			if ((distance & SIGN_BIT) == (dx & SIGN_BIT))
			{
				int y = dy * distance / dx;

				if (y > item->pos.y_pos + bounds[2] - start->y && y < item->pos.y_pos + bounds[3] - start->y)
				{
					int z = dz * distance / dx;

					if (z < item->pos.z_pos + zextent[0] - start->z)
						failure |= 0x1;
					else if (z > item->pos.z_pos + zextent[1] - start->z)
						failure |= 0x2;
					else return true;
				}
			}

			distance = item->pos.x_pos + xextent[1] - start->x;
		}

		if (failure == 0x3)
			return true;
	}

	return false;
}

void UpdateLaserShades()
{
	auto shptr = &LaserShades[0];

	for (int i = 0; i < 32; ++i)
	{
		auto val = *shptr;

		int rnd = GetRandomDraw();

		if (rnd < 0x400)		rnd = (rnd & 15) + 16;
		else if (rnd < 0x1000)	rnd &= 7;
		else if (!(rnd & 0x70)) rnd &= 3;
		else					rnd = 0;

		if (rnd)
		{
			val += rnd;
			if (val > 127)
				val = 127;
		}
		else
		{
			if (val > 16)
				val -= val >> 3;
			else val = 16;
		}

		*shptr++ = val;
	}
}

void LaserSplitterToggle(ITEM_INFO* item)
{
	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (!(boxes[floor->box].overlap_index & BLOCKABLE))
		return;

	bool active = TriggerActive(item);

	if (active == ((boxes[floor->box].overlap_index & BLOCKED) == BLOCKED))
		return;

	int dx, dz;

	switch (item->pos.y_rot)
	{
	case 0:
		dz = -WALL_L;
		dx = 0;
		break;
	case 16384:
		dz = 0;
		dx = -WALL_L;
		break;
	case -32768:
		dz = WALL_L;
		dx = 0;
		break;
	default:
		dz = 0;
		dx = WALL_L;
		break;
	}

	int x = item->pos.x_pos,
		z = item->pos.z_pos;

	while (floor->box != NO_BOX && (boxes[floor->box].overlap_index & BLOCKABLE))
	{
		if (active)
			boxes[floor->box].overlap_index |= BLOCKED;
		else boxes[floor->box].overlap_index &= ~BLOCKED;

		x += dx;
		z += dz;

		floor = GetFloor(x, item->pos.y_pos, z, &room_number);
	}
}

void LaserControl(int16_t item_number)
{
	LaserSplitterToggle(&items[item_number]);
}

void S_DrawLaser(ITEM_INFO* item)
{
	if (!TriggerActive(item))
		return;

	int x,
		y = 0,
		z;

	switch (item->pos.y_rot)
	{
	case 0:
		z = 511;
		x = 0;
		break;
	case 16384:
		z = 0;
		x = 511;
		break;
	case -32768:
		z = -511;
		x = 0;
		break;
	default:
		z = 0;
		x = -511;
		break;
	}

	for (int i = 0; i < item->hit_points; ++i)
	{
		GAME_VECTOR	src  { item->pos.x_pos + x, item->pos.y_pos + y, item->pos.z_pos + z, item->room_number },
					dest { item->pos.x_pos - (x << 5), item->pos.y_pos + y, item->pos.z_pos - (z << 5) };

		LOS(&src, &dest);

		if (LaraOnLOS(&src, &dest))
		{
			if (item->object_number != SECURITY_LASER_ALARM)
			{
				lara_item->hit_points -= (item->object_number == SECURITY_LASER_KILLER ? 0 : 10);

				DoLotsOfBloodD(lara_item->pos.x_pos, item->pos.y_pos + y, lara_item->pos.z_pos, (GetRandomDraw() & 127) + 128, GetRandomDraw() << 1, lara_item->room_number, 1);
			}

			if (item->pos.y_rot == 0 || item->pos.y_rot == -32768)
				dest.z = lara_item->pos.z_pos;
			else dest.x = lara_item->pos.x_pos;

			auto target = &items[0];

			for (int i = 0; i < level_items; ++i, ++target)
				if ((target->object_number == STROBE_LIGHT || target->object_number == ROBOT_SENTRY_GUN) && TriggerActive(target))
					target->really_active = 1;
		}

		if (item->object_number == SECURITY_LASER_ALARM)
			S_DrawLaserBeam(&src, &dest, 16, 0, 16);
		else if (item->object_number == SECURITY_LASER_DEADLY)
			S_DrawLaserBeam(&src, &dest, 0, 0, 16);
		else S_DrawLaserBeam(&src, &dest, 0, 2, 16);

		y -= 256;
	}
}