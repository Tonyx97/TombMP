#include <specific/standard.h>
#include <specific/input.h>

#include "lara.h"
#include "laraanim.h"
#include "control.h"
#include "laramisc.h"
#include "moveblok.h"

#include <mp/client.h>
#include <mp/game/entity_base.h>
#include <mp/game/level.h>

#define MAXOFF_MB	300
#define MAXOFF_Z	(LARA_RAD + 80)

int16_t MovingBlockBounds[12] =
{
				-MAXOFF_MB,		  MAXOFF_MB,	// X axis Limits
						 0,				  0,	// Y axis Limits
	-WALL_L / 2 - MAXOFF_Z,		-WALL_L / 2,	// Z axis Limits
		  -10 * ONE_DEGREE,	10 * ONE_DEGREE,	// X Rot Limits
		  -30 * ONE_DEGREE,	30 * ONE_DEGREE,	// Y Rot Limits
		  -10 * ONE_DEGREE,	10 * ONE_DEGREE		// Z Rot Limits
};

void InitialiseMovingBlock(int16_t item_num)
{
	auto item = &items[item_num];

	ClearMovableBlockSplitters(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

	if (item->status != INVISIBLE)
		AlterFloorHeight(item, -1024);
}

void ClearMovableBlockSplitters(int32_t x, int32_t y, int32_t z, int16_t room_number)
{
	auto floor = GetFloor(x, y, z, &room_number);

	boxes[floor->box].overlap_index &= ~BLOCKED;

	auto height = boxes[floor->box].height,
		 base_room_number = room_number;

	floor = GetFloor(x + WALL_L, y, z, &room_number);

	if (floor->box != NO_BOX && boxes[floor->box].height == height && (boxes[floor->box].overlap_index & BLOCKABLE) && (boxes[floor->box].overlap_index & BLOCKED))
		ClearMovableBlockSplitters(x + WALL_L, y, z, room_number);

	room_number = base_room_number;
	floor = GetFloor(x - WALL_L, y, z, &room_number);

	if (floor->box != NO_BOX && boxes[floor->box].height == height && (boxes[floor->box].overlap_index & BLOCKABLE) && (boxes[floor->box].overlap_index & BLOCKED))
		ClearMovableBlockSplitters(x - WALL_L, y, z, room_number);

	room_number = base_room_number;
	floor = GetFloor(x, y, z + WALL_L, &room_number);

	if (floor->box != NO_BOX && boxes[floor->box].height == height && (boxes[floor->box].overlap_index & BLOCKABLE) && (boxes[floor->box].overlap_index & BLOCKED))
		ClearMovableBlockSplitters(x, y, z + WALL_L, room_number);

	room_number = base_room_number;
	floor = GetFloor(x, y, z - WALL_L, &room_number);

	if (floor->box != NO_BOX && boxes[floor->box].height == height && (boxes[floor->box].overlap_index & BLOCKABLE) && (boxes[floor->box].overlap_index & BLOCKED))
		ClearMovableBlockSplitters(x, y, z - WALL_L, room_number);
}

void MovableBlock(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->flags & ONESHOT)
	{
		AlterFloorHeight(item, 1024);
		KillItem(item_number);
		return;
	}

	int old_x = item->pos.x_pos,
		old_y = item->pos.y_pos,
		old_z = item->pos.z_pos,
		old_room = item->room_number;

	AnimateItem(item);

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	int height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->pos.y_pos < height)
		item->gravity_status = 1;
	else if (item->gravity_status)
	{
		item->gravity_status = 0;
		item->pos.y_pos = height;
		item->status = DEACTIVATED;

		floor_shake_effect(item);

		g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	if (item->status == DEACTIVATED)
	{
		item->status = NOT_ACTIVE;

		RemoveActiveItem(item_number);
		AlterFloorHeight(old_x, old_y, old_z, old_room, 1024);
		AlterFloorHeight(item, -1024);
		AdjustStopperFlag(item, item->item_flags[0] + 0x8000, 0);

		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
		TestTriggers(trigger_index, 1);

		if (auto entity = g_level->get_entity_by_item(item); g_level->is_entity_streamed(entity))
		{
			entity->sync();

			g_level->request_entity_ownership(entity, false);
		}
	}
}

void MovableBlockCollision(int16_t item_num, ITEM_INFO* lara_item, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (item->active == ACTIVE)
		return;

	auto entity = g_level->get_entity_by_item(item);

	if (!(input & IN_ACTION))
		return g_level->request_entity_ownership(entity, false);

	if (lara_item->gravity_status || lara_item->pos.y_pos != item->pos.y_pos)
		return;

	auto quadrant = (uint16_t)(lara_item->pos.y_rot + 0x2000) / 0x4000;

	if (lara_item->current_anim_state == AS_STOP)
	{
		if (lara.gun_status != LG_ARMLESS)
			return;

		PHD_ANGLE new_angle = 0;

		switch (quadrant)
		{
		case NORTH: new_angle = 0;		break;
		case EAST:  new_angle = 16384;  break;
		case SOUTH: new_angle = -32768; break;
		case WEST:  new_angle = -16384; break;
		}

		if (!TestLaraPosition(MovingBlockBounds, item, lara_item, new_angle))
			return;

		if (entity)
		{
			g_level->request_entity_ownership(entity, true);

			if (!g_level->is_entity_streamed(entity))
				return;
		}

		auto room_number = lara_item->room_number;
		auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (room_number != item->room_number)
			return;

		switch (quadrant)
		{
		case NORTH:
			lara_item->pos.z_pos &= -WALL_L;
			lara_item->pos.z_pos += WALL_L - LARA_RAD;
			break;
		case SOUTH:
			lara_item->pos.z_pos &= -WALL_L;
			lara_item->pos.z_pos += LARA_RAD;
			break;
		case EAST:
			lara_item->pos.x_pos &= -WALL_L;
			lara_item->pos.x_pos += WALL_L - LARA_RAD;
			break;
		case WEST:
			lara_item->pos.x_pos &= -WALL_L;
			lara_item->pos.x_pos += LARA_RAD;
			break;
		}

		lara_item->pos.y_rot = new_angle;
		lara_item->goal_anim_state = AS_PPREADY;

		AnimateLara(lara_item);

		if (lara_item->current_anim_state == AS_PPREADY)
			lara.gun_status = LG_HANDSBUSY;
	}
	else if (lara_item->current_anim_state == AS_PPREADY)
	{
		if (lara_item->frame_number != PPREADY_F || !TestLaraPosition(MovingBlockBounds, item, lara_item, lara_item->pos.y_rot))
			return;

		if (input & IN_FORWARD)
		{
			if (!TestBlockPush(item, 1024, quadrant))
				return;

			lara_item->goal_anim_state = AS_PUSHBLOCK;
			item->goal_anim_state = PUSH;
		}
		else if (input & IN_BACK)
		{
			if (!TestBlockPull(item, 1024, quadrant))
				return;

			lara_item->goal_anim_state = AS_PULLBLOCK;
			item->goal_anim_state = PULL;
		}
		else return;

		item->pos.y_rot = lara_item->pos.y_rot;

		AddActiveItem(item_num);
		//AlterFloorHeight(item, 1024);
		AdjustStopperFlag(item, item->item_flags[0], 1);

		AnimateItem(item);
		AnimateLara(lara_item);

		lara.head_x_rot = lara.head_y_rot = 0;
		lara.torso_x_rot = lara.torso_y_rot = 0;
	}
}

int TestBlockMovable(ITEM_INFO* item, int blokhite)
{
	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (floor->floor == NO_HEIGHT / 256)
		return 1;

	if (floor->floor * 256 != item->pos.y_pos - blokhite)
		return 0;

	return 1;
}

int TestBlockPush(ITEM_INFO* item, int blokhite, uint16_t quadrant)
{
	if (!TestBlockMovable(item, blokhite))
		return 0;

	int x = item->pos.x_pos,
		y = item->pos.y_pos,
		z = item->pos.z_pos;

	switch (quadrant)
	{
	case NORTH: z += WALL_L; break;
	case EAST:  x += WALL_L; break;
	case SOUTH: z -= WALL_L; break;
	case WEST:  x -= WALL_L; break;
	}

	auto room_num = item->room_number;
	auto floor = GetFloor(x, y, z, &room_num);
	auto r = &room[room_num];

	if (r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper)
		return 0;

	COLL_INFO scoll;

	scoll.quadrant = quadrant;
	scoll.radius = 500;

	if (CollideStaticObjects(&scoll, x, y, z, room_num, 1000) || ((int32_t)floor->floor << 8) != y)
		return 0;

	GetHeight(floor, x, y, z);

	if (height_type != WALL)
		return 0;

	int cmax = y - (blokhite - 100);

	floor = GetFloor(x, cmax, z, &room_num);

	if (GetCeiling(floor, x, cmax, z) > cmax)
		return 0;

	item->item_flags[0] = lara_item->pos.y_rot;

	return 1;
}

int TestBlockPull(ITEM_INFO* item, int blokhite, uint16_t quadrant)
{
	if (!TestBlockMovable(item, blokhite))
		return 0;

	int zadd = 0,
		xadd = 0;

	switch (quadrant)
	{
	case NORTH: zadd = -WALL_L; break;
	case EAST:  xadd = -WALL_L; break;
	case SOUTH: zadd = WALL_L; break;
	case WEST:  xadd = WALL_L; break;
	}

	int x = item->pos.x_pos + xadd,
		y = item->pos.y_pos,
		z = item->pos.z_pos + zadd;

	auto room_num = item->room_number;
	auto floor = GetFloor(x, y, z, &room_num);
	auto r = &room[room_num];

	if (r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper)
		return 0;

	COLL_INFO scoll;

	scoll.quadrant = quadrant;
	scoll.radius = 500;

	if (CollideStaticObjects(&scoll, x, y, z, room_num, 1000) || ((int32_t)floor->floor << 8) != y)
		return 0;

	int cmax = y - blokhite;

	floor = GetFloor(x, cmax, z, &room_num);

	if (((int32_t)floor->ceiling << 8) > cmax)
		return 0;

	x += xadd;
	z += zadd;
	room_num = item->room_number;
	floor = GetFloor(x, y, z, &room_num);

	if (((int32_t)floor->floor << 8) != y)
		return 0;

	cmax = y - LARA_HITE;
	floor = GetFloor(x, cmax, z, &room_num);

	if (((int32_t)floor->ceiling << 8) > cmax)
		return 0;

	x = lara_item->pos.x_pos + xadd;
	y = lara_item->pos.y_pos;
	z = lara_item->pos.z_pos + zadd;
	room_num = lara_item->room_number;
	floor = GetFloor(x, y, z, &room_num);
	scoll.quadrant = (quadrant + 2) & 3;
	scoll.radius = LARA_RAD;

	if (CollideStaticObjects(&scoll, x, y, z, room_num, LARA_HITE))
		return 0;

	item->item_flags[0] = lara_item->pos.y_rot + 0x8000;

	return 1;
}

void AlterFloorHeight(ITEM_INFO* item, int height)
{
	AlterFloorHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, height);
}

void AlterFloorHeight(int x, int y, int z, int16_t room_id, int height)
{
	auto floor = GetFloor(x, y, z, &room_id);
	auto ceiling = GetFloor(x, y + height - WALL_L, z, &room_id);

	if (floor->floor == NO_HEIGHT / 256)
		floor->floor = ceiling->ceiling + height / 256;
	else
	{
		floor->floor += height / 256;

		if (floor->floor == ceiling->ceiling)
			floor->floor = NO_HEIGHT / 256;
	}

	if (boxes[floor->box].overlap_index & BLOCKABLE)
	{
		if (height < 0)
			boxes[floor->box].overlap_index |= BLOCKED;
		else boxes[floor->box].overlap_index &= ~BLOCKED;
	}
}

void DrawMovableBlock(ITEM_INFO* item)
{
	if (item->status != ACTIVE)
		DrawAnimatingItem(item);
	else DrawUnclippedItem(item);
}

void DrawUnclippedItem(ITEM_INFO* item)
{
	int left = phd_left,
		right = phd_right,
		top = phd_top,
		bottom = phd_bottom;

	phd_left = phd_top = 0;
	phd_bottom = phd_winymax;
	phd_right = phd_winxmax;

	DrawAnimatingItem(item);

	phd_left = left;
	phd_right = right;
	phd_top = top;
	phd_bottom = bottom;
}