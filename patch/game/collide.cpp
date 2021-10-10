import prof;

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "control.h"
#include "inventry.h"
#include "sphere.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#include <scripting/events.h>

#include <mp/game/player.h>
#include <mp/game/level.h>

#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>

#define LEFTRIGHTRAD	250
#define TARGET_DIST		(WALL_L * 4)
#define MOVE_SPEED		16
#define MOVE_ANG		2 * ONE_DEGREE

int32_t xfront, zfront;

void GetCollisionInfo(COLL_INFO* coll, int32_t xpos, int32_t ypos, int32_t zpos, int16_t room_number, int32_t objheight)
{
	coll->coll_type = 0;
	coll->shift.x = coll->shift.y = coll->shift.z = 0;
	coll->quadrant = (uint16_t)(coll->facing + 0x2000) / 0x4000;

	int angadd = (abs(lara_item->pos.y_rot - coll->facing) > 0x7000 ? 0x3000 : 0x4000);

	int xleft2 = (phd_sin(coll->facing - angadd) * LEFTRIGHTRAD) >> W2V_SHIFT,
		zleft2 = (phd_cos(coll->facing - angadd) * LEFTRIGHTRAD) >> W2V_SHIFT,
		xright2 = (phd_sin(coll->facing + angadd) * LEFTRIGHTRAD) >> W2V_SHIFT,
		zright2 = (phd_cos(coll->facing + angadd) * LEFTRIGHTRAD) >> W2V_SHIFT,
		xleft, zleft,
		xright, zright;

	bool hit_left = false,
		 hit_right = false;

	int x = xpos,
		y = ypos - objheight,
		z = zpos,
		ytop = y - 160;

	auto troom_number = room_number;
	auto floor = GetFloor(x, ytop, z, &troom_number);

	int height = GetHeight(floor, x, ytop, z);
	if (height != NO_HEIGHT)
		height -= ypos;

	int ceiling = GetCeiling(floor, x, ytop, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->mid_floor = height;
	coll->mid_ceiling = ceiling;
	coll->mid_type = height_type;
	coll->trigger = trigger_index;

	auto tilt = GetTiltType(floor, x, lara_item->pos.y_pos, z);

	coll->tilt_z = tilt >> 8;
	coll->tilt_x = (signed char)tilt;

	switch (coll->quadrant)
	{
	case 0:
		xfront = (phd_sin(coll->facing) * coll->radius) >> (W2V_SHIFT);
		zfront = coll->radius;
		xleft = -(coll->radius);
		zleft = coll->radius;
		xright = coll->radius;
		zright = coll->radius;
		break;
	case 1:
		xfront = coll->radius;
		zfront = (phd_cos(coll->facing) * coll->radius) >> (W2V_SHIFT);
		xleft = coll->radius;
		zleft = coll->radius;
		xright = coll->radius;
		zright = -(coll->radius);
		break;
	case 2:
		xfront = (phd_sin(coll->facing) * coll->radius) >> (W2V_SHIFT);
		zfront = -coll->radius;
		xleft = coll->radius;
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = -(coll->radius);
		break;
	case 3:
		xfront = -(coll->radius);
		zfront = (phd_cos(coll->facing) * coll->radius) >> (W2V_SHIFT);
		xleft = -(coll->radius);
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = coll->radius;
		break;
	default:
		xleft = zleft = 0;
		xright = zright = 0;
		xfront = zfront = 0;
		break;
	}

	x = xpos + xfront;
	z = zpos + zfront;
	floor = GetFloor(x, ytop, z, &troom_number);

	if ((height = GetHeight(floor, x, ytop, z)) != NO_HEIGHT)
		height -= ypos;

	if ((ceiling = GetCeiling(floor, x, ytop, z)) != NO_HEIGHT)
		ceiling -= y;

	coll->front_floor = height;
	coll->front_ceiling = ceiling;
	coll->front_type = height_type;

	floor = GetFloor(x + xfront, ytop, z + zfront, &troom_number);
	height = GetHeight(floor, x + xfront, ytop, z + zfront);

	if (height != NO_HEIGHT)
		height -= ypos;

	if (coll->slopes_are_walls && (coll->front_type == BIG_SLOPE || coll->front_type == DIAGONAL) && coll->front_floor < coll->mid_floor && height < coll->front_floor)
		coll->front_floor = -32767;
	else if (coll->slopes_are_pits && (coll->front_type == BIG_SLOPE || coll->front_type == DIAGONAL) && coll->front_floor > coll->mid_floor)
		coll->front_floor = 512;
	else if (coll->lava_is_pit && coll->front_floor > 0 && trigger_index && (*(trigger_index) & DATA_TYPE) == LAVA_TYPE)
		coll->front_floor = 512;

	x = xpos + xleft;
	z = zpos + zleft;
	
	auto lrroom_number = room_number;

	floor = GetFloor(x, ytop, z, &lrroom_number);

	if ((height = GetHeight(floor, x, ytop, z)) != NO_HEIGHT)
		height -= ypos;

	if ((ceiling = GetCeiling(floor, x, ytop, z)) != NO_HEIGHT)
		ceiling -= y;

	coll->left_floor = height;
	coll->left_ceiling = ceiling;
	coll->left_type = height_type;

	if (coll->slopes_are_walls == 1 && (coll->left_type == BIG_SLOPE || coll->left_type == DIAGONAL) && coll->left_floor < 0) coll->left_floor = -32767;
	else if (coll->slopes_are_pits && (coll->left_type == BIG_SLOPE || coll->left_type == DIAGONAL) && coll->left_floor > 0)  coll->left_floor = 512;
	else if (coll->lava_is_pit && coll->left_floor > 0 && trigger_index && (*(trigger_index)&DATA_TYPE) == LAVA_TYPE)		  coll->left_floor = 512;

	x = xpos + xleft;
	z = zpos + zleft;
	floor = GetFloor(x, ytop, z, &troom_number);

	if ((height = GetHeight(floor, x, ytop, z)) != NO_HEIGHT)
		height -= ypos;

	if ((ceiling = GetCeiling(floor, x, ytop, z)) != NO_HEIGHT)
		ceiling -= y;

	coll->left_floor2 = height;
	coll->left_ceiling2 = ceiling;
	coll->left_type2 = height_type;

	if (coll->slopes_are_walls == 1 && (coll->left_type2 == BIG_SLOPE || coll->left_type2 == DIAGONAL) && coll->left_floor2 < 0) coll->left_floor2 = -32767;
	else if (coll->slopes_are_pits && (coll->left_type2 == BIG_SLOPE || coll->left_type2 == DIAGONAL) && coll->left_floor2 > 0)  coll->left_floor2 = 512;
	else if (coll->lava_is_pit && coll->left_floor2 > 0 && trigger_index && (*(trigger_index)&DATA_TYPE) == LAVA_TYPE)			 coll->left_floor2 = 512;

	x = xpos + xright;
	z = zpos + zright;
	lrroom_number = room_number;
	floor = GetFloor(x, ytop, z, &lrroom_number);

	if ((height = GetHeight(floor, x, ytop, z)) != NO_HEIGHT)
		height -= ypos;

	if ((ceiling = GetCeiling(floor, x, ytop, z)) != NO_HEIGHT)
		ceiling -= y;

	coll->right_floor = height;
	coll->right_ceiling = ceiling;
	coll->right_type = height_type;

	if (coll->slopes_are_walls == 1 && (coll->right_type == BIG_SLOPE || coll->right_type == DIAGONAL) && coll->right_floor < 0) coll->right_floor = -32767;
	else if (coll->slopes_are_pits && (coll->right_type == BIG_SLOPE || coll->right_type == DIAGONAL) && coll->right_floor > 0)  coll->right_floor = 512;
	else if (coll->lava_is_pit && coll->right_floor > 0 && trigger_index && (*(trigger_index)&DATA_TYPE) == LAVA_TYPE)			 coll->right_floor = 512;

	x = xpos + xright;
	z = zpos + zright;
	floor = GetFloor(x, ytop, z, &troom_number);

	if ((height = GetHeight(floor, x, ytop, z)) != NO_HEIGHT)
		height -= ypos;

	if ((ceiling = GetCeiling(floor, x, ytop, z)) != NO_HEIGHT)
		ceiling -= y;

	coll->right_floor2 = height;
	coll->right_ceiling2 = ceiling;
	coll->right_type2 = height_type;

	if (coll->slopes_are_walls == 1 && (coll->right_type2 == BIG_SLOPE || coll->right_type2 == DIAGONAL) && coll->right_floor2 < 0) coll->right_floor2 = -32767;
	else if (coll->slopes_are_pits && (coll->right_type2 == BIG_SLOPE || coll->right_type2 == DIAGONAL) && coll->right_floor2 > 0)  coll->right_floor2 = 512;
	else if (coll->lava_is_pit && coll->right_floor2 > 0 && trigger_index && (*(trigger_index)&DATA_TYPE) == LAVA_TYPE)				coll->right_floor2 = 512;

	x = xpos + xleft2;
	z = zpos + zleft2;
	lrroom_number = room_number;
	floor = GetFloor(x, ytop, z, &lrroom_number);

	if ((height = GetHeight(floor, x, ytop, z)) != NO_HEIGHT)
		height -= ypos;

	if ((ceiling = GetCeiling(floor, x, ytop, z)) != NO_HEIGHT)
		ceiling -= y;

	if (height > coll->bad_pos || height < coll->bad_neg || ceiling > coll->bad_ceiling)
		hit_left = true;

	x = xpos + xright2;
	z = zpos + zright2;
	lrroom_number = room_number;
	floor = GetFloor(x, ytop, z, &lrroom_number);

	if ((height = GetHeight(floor, x, ytop, z)) != NO_HEIGHT)
		height -= ypos;

	if ((ceiling = GetCeiling(floor, x, ytop, z)) != NO_HEIGHT)
		ceiling -= y;

	if (height > coll->bad_pos || height < coll->bad_neg || ceiling > coll->bad_ceiling)
		hit_right = true;

	if (CollideStaticObjects(coll, xpos, ypos, zpos, troom_number, objheight))
	{
		x = xpos + coll->shift.x;
		z = zpos + coll->shift.z;
		floor = GetFloor(x, ypos, z, &troom_number);
		if (GetHeight(floor, x, ypos, z) < ypos - (WALL_L / 2))
		{
			coll->shift.x = -coll->shift.x;
			coll->shift.z = -coll->shift.z;
		}
		else if (GetCeiling(floor, x, ypos, z) > ypos - objheight)
		{
			coll->shift.x = -coll->shift.x;
			coll->shift.z = -coll->shift.z;
		}
	}
	
	if (coll->mid_floor == NO_HEIGHT)
	{
		coll->shift.x = coll->old.x - xpos;
		coll->shift.y = coll->old.y - ypos;
		coll->shift.z = coll->old.z - zpos;
		coll->coll_type = COLL_FRONT;

		return;
	}

	if (coll->mid_floor - coll->mid_ceiling <= 0)
	{
		coll->shift.x = coll->old.x - xpos;
		coll->shift.y = coll->old.y - ypos;
		coll->shift.z = coll->old.z - zpos;
		coll->coll_type = COLL_CLAMP;

		return;
	}

	if (coll->mid_ceiling >= 0)
	{
		coll->shift.y = coll->mid_ceiling;
		coll->coll_type = COLL_TOP;
		coll->hit_ceiling = 1;
	}

	if (hit_left && hit_right)
	{
		coll->shift.x = coll->old.x - xpos;
		coll->shift.z = coll->old.z - zpos;
		coll->coll_type = COLL_FRONT;

		return;
	}

	if (coll->front_floor > coll->bad_pos || coll->front_floor < coll->bad_neg || coll->front_ceiling > coll->bad_ceiling)
	{
		if (coll->front_type == DIAGONAL || coll->front_type == SPLIT_TRI)
		{
			coll->shift.x = coll->old.x - xpos;
			coll->shift.z = coll->old.z - zpos;
		}
		else
		{
			switch (coll->quadrant)
			{
			case 0:
			case 2:
				coll->shift.x = coll->old.x - xpos;
				coll->shift.z = FindGridShift(zpos + zfront, zpos);
				break;
			case 1:
			case 3:
				coll->shift.x = FindGridShift(xpos + xfront, xpos);
				coll->shift.z = coll->old.z - zpos;
				break;
			}
		}

		coll->coll_type = COLL_FRONT;

		return;
	}

	if (coll->front_ceiling >= coll->bad_ceiling)
	{
		coll->shift.x = coll->old.x - xpos;
		coll->shift.y = coll->old.y - ypos;
		coll->shift.z = coll->old.z - zpos;
		coll->coll_type = COLL_TOPFRONT;

		return;
	}

	if (coll->left_floor > coll->bad_pos || coll->left_floor < coll->bad_neg || coll->left_ceiling > coll->bad_ceiling)
	{
		if (coll->left_type == SPLIT_TRI)
		{
			coll->shift.x = coll->old.x - xpos;
			coll->shift.z = coll->old.z - zpos;
		}
		else
		{
			switch (coll->quadrant)
			{
			case 0:
			case 2: coll->shift.x = FindGridShift(xpos + xleft, xpos + xfront); break;
			case 1:
			case 3: coll->shift.z = FindGridShift(zpos + zleft, zpos + zfront); break;
			}
		}

		coll->coll_type = COLL_LEFT;

		return;
	}

	if (coll->right_floor > coll->bad_pos || coll->right_floor < coll->bad_neg || coll->right_ceiling > coll->bad_ceiling)
	{
		if (coll->right_type == SPLIT_TRI)
		{
			coll->shift.x = coll->old.x - xpos;
			coll->shift.z = coll->old.z - zpos;
		}
		else
		{
			switch (coll->quadrant)
			{
			case 0:
			case 2: coll->shift.x = FindGridShift(xpos + xright, xpos + xfront); break;
			case 1:
			case 3: coll->shift.z = FindGridShift(zpos + zright, zpos + zfront); break;
			}
		}

		coll->coll_type = COLL_RIGHT;
	}
}

int32_t FindGridShift(int32_t src, int32_t dst)
{
	int srcw = src >> WALL_SHIFT,
		dstw = dst >> WALL_SHIFT;

	if (srcw == dstw)
		return 0;

	src &= WALL_L - 1;

	return (dstw > srcw ? WALL_L - (src - 1) : -(src + 1));
}

int32_t FindGridShift2(int32_t src, int32_t dst)
{
	int srcw = src >> WALL_SHIFT,
		dstw = dst >> WALL_SHIFT;

	if (srcw == dstw)
		return 0;

	src &= WALL_L - 1;

	return (dstw > srcw ? WALL_L - src - 1 : -src + 1);
}

int CollideStaticObjects(COLL_INFO* coll, int32_t x, int32_t y, int32_t z, int16_t room_number, int32_t hite)
{
	coll->hit_static = 0;

	int inxmin = x - coll->radius,
		inxmax = x + coll->radius,
		inymin = y - hite,
		inymax = y,
		inzmin = z - coll->radius,
		inzmax = z + coll->radius;

	GetNearByRooms(x, y, z, coll->radius + 50, hite + 50, room_number);

	for (int i = 0; i < number_draw_rooms; ++i)
	{
		auto r = &room[rooms_to_draw[i]];
		auto mesh = r->mesh;

		for (int j = r->num_meshes; j > 0; --j, ++mesh)
		{
			auto sinfo = &static_objects[mesh->static_number];

			if ((sinfo->flags & 1))
				continue;

			int ymin = mesh->y + sinfo->y_minc,
				ymax = mesh->y + sinfo->y_maxc,
				xmin, xmax, zmin, zmax;

			switch (mesh->y_rot)
			{
			default:
				xmin = mesh->x + sinfo->x_minc;
				xmax = mesh->x + sinfo->x_maxc;
				zmin = mesh->z + sinfo->z_minc;
				zmax = mesh->z + sinfo->z_maxc;
				break;
			case 16384:
				xmin = mesh->x + sinfo->z_minc;
				xmax = mesh->x + sinfo->z_maxc;
				zmin = mesh->z - sinfo->x_maxc;
				zmax = mesh->z - sinfo->x_minc;
				break;
			case -32768:
				xmin = mesh->x - sinfo->x_maxc;
				xmax = mesh->x - sinfo->x_minc;
				zmin = mesh->z - sinfo->z_maxc;
				zmax = mesh->z - sinfo->z_minc;
				break;
			case -16384:
				xmin = mesh->x - sinfo->z_maxc;
				xmax = mesh->x - sinfo->z_minc;
				zmin = mesh->z + sinfo->x_minc;
				zmax = mesh->z + sinfo->x_maxc;
				break;
			}

			if (inxmax <= xmin || inxmin >= xmax ||
				inymax <= ymin || inymin >= ymax ||
				inzmax <= zmin || inzmin >= zmax)
				continue;

			PHD_VECTOR shifter {};

			int shl = inxmax - xmin,
				shr = xmax - inxmin;

			shifter.x = (shl < shr ? -shl : shr);

			shl = inzmax - zmin;
			shr = zmax - inzmin;
			shifter.z = (shl < shr ? -shl : shr);
			
			switch (coll->quadrant)
			{
			case NORTH:
			{
				if (shifter.x > coll->radius || shifter.x < -coll->radius)
				{
					coll->shift.z = shifter.z;
					coll->shift.x = coll->old.x - x;
					coll->coll_type = COLL_FRONT;
				}
				else if (shifter.x > 0 && shifter.x <= coll->radius)
				{
					coll->shift.x = shifter.x;
					coll->shift.z = 0;
					coll->coll_type = COLL_LEFT;
				}
				else if (shifter.x < 0 && shifter.x >= -coll->radius)
				{
					coll->shift.x = shifter.x;
					coll->shift.z = 0;
					coll->coll_type = COLL_RIGHT;
				}

				break;
			}
			case EAST:
			{
				if (shifter.z > coll->radius || shifter.z < -coll->radius)
				{
					coll->shift.x = shifter.x;
					coll->shift.z = coll->old.z - z;
					coll->coll_type = COLL_FRONT;
				}
				else if (shifter.z > 0 && shifter.z <= coll->radius)
				{
					coll->shift.z = shifter.z;
					coll->shift.x = 0;
					coll->coll_type = COLL_RIGHT;
				}
				else if (shifter.z < 0 && shifter.z >= -coll->radius)
				{
					coll->shift.z = shifter.z;
					coll->shift.x = 0;
					coll->coll_type = COLL_LEFT;
				}

				break;
			}
			case SOUTH:
			{
				if (shifter.x > coll->radius || shifter.x < -coll->radius)
				{
					coll->shift.z = shifter.z;
					coll->shift.x = coll->old.x - x;
					coll->coll_type = COLL_FRONT;
				}
				else if (shifter.x > 0 && shifter.x <= coll->radius)
				{
					coll->shift.x = shifter.x;
					coll->shift.z = 0;
					coll->coll_type = COLL_RIGHT;
				}
				else if (shifter.x < 0 && shifter.x >= -coll->radius)
				{
					coll->shift.x = shifter.x;
					coll->shift.z = 0;
					coll->coll_type = COLL_LEFT;
				}

				break;
			}
			case WEST:
			{
				if (shifter.z > coll->radius || shifter.z < -coll->radius)
				{
					coll->shift.x = shifter.x;
					coll->shift.z = coll->old.z - z;
					coll->coll_type = COLL_FRONT;
				}
				else if (shifter.z > 0 && shifter.z <= coll->radius)
				{
					coll->shift.z = shifter.z;
					coll->shift.x = 0;
					coll->coll_type = COLL_LEFT;
				}
				else if (shifter.z < 0 && shifter.z >= -coll->radius)
				{
					coll->shift.z = shifter.z;
					coll->shift.x = 0;
					coll->coll_type = COLL_RIGHT;
				}

				break;
			}
			}

			return (coll->hit_static = 1);
		}
	}

	return 0;
}

void GetNearByRooms(int32_t x, int32_t y, int32_t z, int32_t r, int32_t h, int16_t room_number)
{
	rooms_to_draw[0] = room_number;
	number_draw_rooms = 1;

	GetNewRoom(x + r, y, z + r, room_number);
	GetNewRoom(x - r, y, z + r, room_number);
	GetNewRoom(x + r, y, z - r, room_number);
	GetNewRoom(x - r, y, z - r, room_number);
	GetNewRoom(x + r, y - h, z + r, room_number);
	GetNewRoom(x - r, y - h, z + r, room_number);
	GetNewRoom(x + r, y - h, z - r, room_number);
	GetNewRoom(x - r, y - h, z - r, room_number);
}

void GetNewRoom(int32_t x, int32_t y, int32_t z, int16_t room_number)
{
	GetFloor(x, y, z, &room_number);

	int i = 0;

	for (; i < number_draw_rooms; ++i)
		if (rooms_to_draw[i] == room_number)
			break;

	if (i == number_draw_rooms)
		rooms_to_draw[number_draw_rooms++] = room_number;
}

void ShiftItem(ITEM_INFO* item, COLL_INFO* coll)
{
	item->pos.x_pos += coll->shift.x;
	item->pos.y_pos += coll->shift.y;
	item->pos.z_pos += coll->shift.z;
	coll->shift.x = coll->shift.y = coll->shift.z = 0;
}

void UpdateLaraRoomTeleport(ITEM_INFO* item, int height)
{
	int x = item->pos.x_pos,
		y = item->pos.y_pos + height,
		z = item->pos.z_pos;

	auto room_number = item->room_number;
	auto floor = FindFloor(x, y, z, &room_number);

	item->floor = GetHeight(floor, x, y, z);

	if (item->room_number != room_number)
		ItemNewRoom(lara.item_number, room_number);
}

void UpdateLaraRoom(ITEM_INFO* item, int height)
{
	int x = item->pos.x_pos,
		y = item->pos.y_pos + height,
		z = item->pos.z_pos;

	auto room_number = item->room_number;
	auto floor = GetFloor(x, y, z, &room_number);

	item->floor = GetHeight(floor, x, y, z);

	if (item->room_number != room_number)
		ItemNewRoom(lara.item_number, room_number);
}

int16_t GetTiltType(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z)
{
	while (floor->pit_room != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;

		auto r = &room[floor->pit_room];

		floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
	}

	auto index = floor->index;

	if (y + 512 < ((int32_t)floor->floor << 8))
		return 0;

	if (floor->index)
	{
		auto data = &floor_data[floor->index];

		auto type = *(data) & DATA_TYPE;

		if (type == TILT_TYPE)
			return (*(data + 1));

		if (type == SPLIT1 || type == SPLIT2 || type == NOCOLF1T || type == NOCOLF2T || type == NOCOLF1B || type == NOCOLF2B)
		{
			int dx = x & 1023,
				dz = z & 1023;

			int16_t tilts = *(data + 1),
				   t0 = tilts & 15,
				   t1 = (tilts >> 4) & 15,
				   t2 = (tilts >> 8) & 15,
				   t3 = (tilts >> 12) & 15,
				   xoff, yoff;

			if ((type & DATA_TYPE) == SPLIT1 || (type & DATA_TYPE) == NOCOLF1T || (type & DATA_TYPE) == NOCOLF1B)
			{
				if (dx <= (1024 - dz))
				{
					xoff = t2 - t1;
					yoff = t0 - t1;
				}
				else
				{
					xoff = t3 - t0;
					yoff = t3 - t2;
				}
			}
			else
			{
				if (dx <= dz)
				{
					xoff = t2 - t1;
					yoff = t3 - t2;
				}
				else
				{
					xoff = t3 - t0;
					yoff = t0 - t1;
				}
			}

			return ((xoff << 8) | (yoff & 0xff));
		}
	}

	return 0;
}

void LaraBaddieCollision(ITEM_INFO* laraitem, COLL_INFO* coll)
{
	lara.hit_direction = -1;

	laraitem->hit_status = 0;

	if (laraitem->hit_points <= 0)
		return;

	int16_t roomies[20];

	roomies[0] = laraitem->room_number;

	int16_t numroom = 1,
		  * door = room[laraitem->room_number].door;

	if (door)
		for (int i = (int)*(door++); i > 0; --i, door += 16)
			roomies[numroom++] = *(door);

	for (int i = 0; i < numroom; ++i)
	{
		auto item_num = room[roomies[i]].item_number;

		while (item_num != NO_ITEM)
		{
			auto item = &items[item_num];

			if (item->collidable && item->status != INVISIBLE && item_num != lara.item_number)
			{
				if (auto object = &objects[item->object_number]; object->collision)
				{
					int x = laraitem->pos.x_pos - item->pos.x_pos,
						y = laraitem->pos.y_pos - item->pos.y_pos,
						z = laraitem->pos.z_pos - item->pos.z_pos;

					if (x > -TARGET_DIST && x < TARGET_DIST &&
						z > -TARGET_DIST && z < TARGET_DIST &&
						y > -TARGET_DIST && y < TARGET_DIST)
						(*object->collision)(item_num, laraitem, coll);
				}
			}

			item_num = item->next_item;
		}
	}

	if (lara.hit_direction == -1)
		lara.hit_frame = 0;

	Inventory_Chosen = -1;
}

void CreatureCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (!TestBoundsCollide(item, laraitem, coll->radius) || !TestCollision(item, laraitem) || lara.water_status == LARA_UNDERWATER)
		return;

	if (coll->enable_baddie_push)
		ItemPushLara(item, laraitem, coll, coll->enable_spaz, 0);
	else if (coll->enable_spaz)
	{
		int x = laraitem->pos.x_pos - item->pos.x_pos,
			z = laraitem->pos.z_pos - item->pos.z_pos,
			c = phd_cos(item->pos.y_rot),
			s = phd_sin(item->pos.y_rot);

		auto bounds = GetBestFrame(item);

		int rx = (*(bounds + 0) + *(bounds + 1)) / 2,
			rz = (*(bounds + 4) + *(bounds + 5)) / 2;

		x -= (c * rx + s * rz) >> W2V_SHIFT;
		z -= (c * rz - s * rx) >> W2V_SHIFT;

		if (bounds[3] - bounds[2] > STEP_L)
		{
			PHD_ANGLE hitang = laraitem->pos.y_rot - (32768 + phd_atan(z, x));

			lara.hit_direction = (uint16_t)(hitang + 0x2000) / 0x4000;

			if (++lara.hit_frame > 30)
				lara.hit_frame = 30;
		}
	}
}

void ObjectCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (!TestBoundsCollide(item, laraitem, coll->radius) || !TestCollision(item, laraitem))
		return;

	if (coll->enable_baddie_push)
		ItemPushLara(item, laraitem, coll, 0, 1);
}

void ObjectCollisionSub(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (!TestBoundsCollide(item, laraitem, coll->radius) || !TestCollision(item, laraitem))
		return;

	ItemPushLara(item, laraitem, coll, 0, 0);
}

void AIPickupCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll) {}

void DoorCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (!TestBoundsCollide(item, laraitem, coll->radius) || !TestCollision(item, laraitem))
		return;

	if (coll->enable_baddie_push)
		ItemPushLara(item, laraitem, coll, item->current_anim_state != item->goal_anim_state ? coll->enable_spaz : 0, 1);
}

void TrapCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (item->status == ACTIVE)
	{
		if (!TestBoundsCollide(item, laraitem, coll->radius))
			return;

		TestCollision(item, laraitem);

		if (item->object_number == FAN && item->current_anim_state == 1)
			ObjectCollision(item_num, laraitem, coll);
	}
	else if (item->status != INVISIBLE)
		ObjectCollision(item_num, laraitem, coll);
}

void ItemPushLara(ITEM_INFO* item, ITEM_INFO* laraitem, COLL_INFO* coll, int spazon, int bigpush)
{
	if (!g_resource->trigger_event(events::physics::ON_PLAYER_ITEM_COLLIDE, game_level::LOCALPLAYER(), item, coll))
		return;

	int x = laraitem->pos.x_pos - item->pos.x_pos,
		z = laraitem->pos.z_pos - item->pos.z_pos,
		c = phd_cos(item->pos.y_rot),
		s = phd_sin(item->pos.y_rot),
		rx = (c * x - s * z) >> W2V_SHIFT,
		rz = (c * z + s * x) >> W2V_SHIFT;

	auto object = &objects[item->object_number];
	auto bounds = GetBestFrame(item);

	int minx = *(bounds + 0),
		maxx = *(bounds + 1),
		minz = *(bounds + 4),
		maxz = *(bounds + 5);

	if (bigpush)
	{
		minx -= coll->radius;
		maxx += coll->radius;
		minz -= coll->radius;
		maxz += coll->radius;
	}

	if (abs(x) > 0x1200 || abs(z) > 0x1200)
		return;

	if (rx >= minx && rx <= maxx && rz >= minz && rz <= maxz)
	{
		int l = rx - minx,
			r = maxx - rx,
			t = maxz - rz,
			b = rz - minz;

		if (l <= r && l <= t && l <= b)		 rx -= l;
		else if (r <= l && r <= t && r <= b) rx += r;
		else if (t <= l && t <= r && t <= b) rz += t;
		else								 rz -= b;

		int ax = (c * rx + s * rz) >> W2V_SHIFT,
			az = (c * rz - s * rx) >> W2V_SHIFT;

		laraitem->pos.x_pos = item->pos.x_pos + ax;
		laraitem->pos.z_pos = item->pos.z_pos + az;

		rx = (*(bounds + 0) + *(bounds + 1)) / 2;
		rz = (*(bounds + 4) + *(bounds + 5)) / 2;
		x -= (c * rx + s * rz) >> W2V_SHIFT;
		z -= (c * rz - s * rx) >> W2V_SHIFT;

		if (spazon && bounds[3] - bounds[2] > STEP_L)
		{
			PHD_ANGLE hitang = laraitem->pos.y_rot - (32768 + phd_atan(z, x));

			lara.hit_direction = (uint16_t)(hitang + 0x2000) / 0x4000;

			if (!lara.hit_frame)
				g_audio->play_sound(31, { laraitem->pos.x_pos, laraitem->pos.y_pos, laraitem->pos.z_pos });

			if (++lara.hit_frame > 34)
				lara.hit_frame = 34;
		}

		coll->bad_pos = NO_BAD_POS;
		coll->bad_neg = -STEPUP_HEIGHT;
		coll->bad_ceiling = 0;

		PHD_ANGLE hitang = coll->facing;

		coll->facing = phd_atan((laraitem->pos.z_pos - coll->old.z), (laraitem->pos.x_pos - coll->old.x));

		GetCollisionInfo(coll, laraitem->pos.x_pos, laraitem->pos.y_pos, laraitem->pos.z_pos, laraitem->room_number, LARA_HITE);
		coll->facing = hitang;

		if (coll->coll_type != COLL_NONE)
		{
			laraitem->pos.x_pos = coll->old.x;
			laraitem->pos.z_pos = coll->old.z;
		}
		else
		{
			coll->old.x = laraitem->pos.x_pos;
			coll->old.y = laraitem->pos.y_pos;
			coll->old.z = laraitem->pos.z_pos;

			UpdateLaraRoom(laraitem, -10);
		}
	}
}

int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* laraitem, int32_t radius)
{
	auto object = &objects[item->object_number];
	auto bounds = GetBestFrame(item);
	auto larabounds = GetBestFrame(laraitem);

	if (item->pos.y_pos + *(bounds + 3) <= laraitem->pos.y_pos + *(larabounds + 2) ||
		item->pos.y_pos + *(bounds + 2) >= laraitem->pos.y_pos + *(larabounds + 3))
		return 0;

	int c = phd_cos(item->pos.y_rot),
		s = phd_sin(item->pos.y_rot),
		x = laraitem->pos.x_pos - item->pos.x_pos,
		z = laraitem->pos.z_pos - item->pos.z_pos,
		rx = (c * x - s * z) >> W2V_SHIFT,
		rz = (c * z + s * x) >> W2V_SHIFT,
		minx = *(bounds + 0) - radius,
		maxx = *(bounds + 1) + radius,
		minz = *(bounds + 4) - radius,
		maxz = *(bounds + 5) + radius;

	return (rx >= minx && rx <= maxx && rz >= minz && rz <= maxz);
}

int TestLaraPosition(int16_t* bounds, ITEM_INFO* item, ITEM_INFO* laraitem, PHD_ANGLE override_y_rot)
{
	PHD_ANGLE real_y_rot = override_y_rot == -1 ? item->pos.y_rot : override_y_rot,
			  xrotrel = laraitem->pos.x_rot - item->pos.x_rot,
			  yrotrel = laraitem->pos.y_rot - real_y_rot,
			  zrotrel = laraitem->pos.z_rot - item->pos.z_rot;

	if (xrotrel < *(bounds + 6) || xrotrel > *(bounds + 7) || yrotrel < *(bounds + 8) || yrotrel > *(bounds + 9) || zrotrel < *(bounds + 10) || zrotrel > *(bounds + 11))
		return 0;

	phd_PushUnitMatrix();
	phd_RotYXZ(real_y_rot, item->pos.x_rot, item->pos.z_rot);

	int x = laraitem->pos.x_pos - item->pos.x_pos,
		y = laraitem->pos.y_pos - item->pos.y_pos,
		z = laraitem->pos.z_pos - item->pos.z_pos;

	auto mptr = phd_mxptr;

	int rx = (*(mptr + M00) * x + *(mptr + M10) * y + *(mptr + M20) * z) >> W2V_SHIFT,
		ry = (*(mptr + M01) * x + *(mptr + M11) * y + *(mptr + M21) * z) >> W2V_SHIFT,
		rz = (*(mptr + M02) * x + *(mptr + M12) * y + *(mptr + M22) * z) >> W2V_SHIFT;

	phd_PopMatrix();

	if (rx < (int32_t)*(bounds + 0) ||
		rx > (int32_t)*(bounds + 1) ||
		ry < (int32_t)*(bounds + 2) ||
		ry > (int32_t)*(bounds + 3) ||
		rz < (int32_t)*(bounds + 4) ||
		rz > (int32_t)*(bounds + 5))
		return 0;

	return 1;
}

void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* laraitem)
{
	laraitem->pos.x_rot = item->pos.x_rot;
	laraitem->pos.y_rot = item->pos.y_rot;
	laraitem->pos.z_rot = item->pos.z_rot;

	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	auto mptr = phd_mxptr;

	int x = item->pos.x_pos + ((*(mptr + M00) * vec->x + *(mptr + M01) * vec->y + *(mptr + M02) * vec->z) >> W2V_SHIFT),
		y = item->pos.y_pos + ((*(mptr + M10) * vec->x + *(mptr + M11) * vec->y + *(mptr + M12) * vec->z) >> W2V_SHIFT),
		z = item->pos.z_pos + ((*(mptr + M20) * vec->x + *(mptr + M21) * vec->y + *(mptr + M22) * vec->z) >> W2V_SHIFT);

	phd_PopMatrix();

	laraitem->pos.x_pos = x;
	laraitem->pos.y_pos = y;
	laraitem->pos.z_pos = z;
}

int MoveLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* laraitem)
{
	PHD_3DPOS dest;

	phd_PushUnitMatrix();
	{
		phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

		auto mptr = phd_mxptr;

		dest =
		{
			item->pos.x_pos + ((*(mptr + M00) * vec->x + *(mptr + M01) * vec->y + *(mptr + M02) * vec->z) >> W2V_SHIFT),
			item->pos.y_pos + ((*(mptr + M10) * vec->x + *(mptr + M11) * vec->y + *(mptr + M12) * vec->z) >> W2V_SHIFT),
			item->pos.z_pos + ((*(mptr + M20) * vec->x + *(mptr + M21) * vec->y + *(mptr + M22) * vec->z) >> W2V_SHIFT),
			item->pos.x_rot,
			item->pos.y_rot,
			item->pos.z_rot
		};
	}
	phd_PopMatrix();

	if (item->object_number == FLARE_ITEM)
	{
		// stop Lara teleporting when picking up flare back against a wall

		auto room_number = laraitem->room_number;
		auto floor = GetFloor(dest.x_pos, dest.y_pos, dest.z_pos, &room_number);
		auto height = GetHeight(floor, dest.x_pos, dest.y_pos, dest.z_pos);

		if (ABS(height - laraitem->pos.y_pos) > STEP_L * 2)
			return 0;
		else
		{
			int x = dest.x_pos - laraitem->pos.x_pos,
				y = dest.y_pos - laraitem->pos.y_pos,
				z = dest.z_pos - laraitem->pos.z_pos,
				dist = phd_sqrt(x * x + y * y + z * z);

			if (dist < 128)
				return 1;
		}
	}

	return Move3DPosTo3DPos(&laraitem->pos, &dest, MOVE_SPEED, MOVE_ANG);
}

int Move3DPosTo3DPos(PHD_3DPOS* srcpos, PHD_3DPOS* destpos, int velocity, PHD_ANGLE angadd)
{
	int x = destpos->x_pos - srcpos->x_pos,
		y = destpos->y_pos - srcpos->y_pos,
		z = destpos->z_pos - srcpos->z_pos,
		dist = phd_sqrt(x * x + y * y + z * z);

	if (velocity >= dist)
	{
		srcpos->x_pos = destpos->x_pos;
		srcpos->y_pos = destpos->y_pos;
		srcpos->z_pos = destpos->z_pos;
	}
	else
	{
		srcpos->x_pos += (x * velocity) / dist;
		srcpos->y_pos += (y * velocity) / dist;
		srcpos->z_pos += (z * velocity) / dist;
	}

	PHD_ANGLE angdif = destpos->x_rot - srcpos->x_rot;

	if (angdif > angadd)		srcpos->x_rot += angadd;
	else if (angdif < -angadd)	srcpos->x_rot -= angadd;
	else						srcpos->x_rot = destpos->x_rot;

	angdif = destpos->y_rot - srcpos->y_rot;

	if (angdif > angadd)		srcpos->y_rot += angadd;
	else if (angdif < -angadd)	srcpos->y_rot -= angadd;
	else						srcpos->y_rot = destpos->y_rot;

	angdif = destpos->z_rot - srcpos->z_rot;

	if (angdif > angadd)			srcpos->z_rot += angadd;
	else if (angdif < -angadd)	srcpos->z_rot -= angadd;
	else						srcpos->z_rot = destpos->z_rot;

	return (srcpos->x_pos == destpos->x_pos &&
			srcpos->y_pos == destpos->y_pos &&
			srcpos->z_pos == destpos->z_pos &&
			srcpos->x_rot == destpos->x_rot &&
			srcpos->y_rot == destpos->y_rot &&
			srcpos->z_rot == destpos->z_rot);
}