#include <specific/standard.h>
#include <specific/global.h>
#include <specific/input.h>
#include <specific/init.h>
#include <specific/fn_stubs.h>

#include "objects.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "sphere.h"
#include "lot.h"
#include "effect2.h"
#include "missile.h"
#include "game.h"

#define LIFT_WAIT_TIME	90
#define	LOW_RUMBLE		20
#define	BIG_RUMBLE		100
#define FINAL_COUNTDOWN (30 * 5)
#define LIFT_HEIGHT		(STEP_L*5)

struct DOORPOS_DATA
{
	FLOOR_INFO* floor;
	FLOOR_INFO data;
	int16_t block;
};

struct DOOR_DATA
{
	DOORPOS_DATA d1, d1flip, d2, d2flip;
};

struct LIFT_INFO
{
	int start_height;
	int wait_time;
};

enum copter_anim
{
	COPTER_EMPTY,
	COPTER_SPIN,
	COPTER_TAKEOFF
};

int16_t final_boss[5];

PHD_VECTOR PolePos = { 0, 0, -208 };
PHD_VECTOR PolePosR = { 0, 0, 0 };

int16_t PoleBounds[12] =
{
	-256,						   256,
	   0,						     0,
	-512,						   512,
	-10 * ONE_DEGREE,  10 * ONE_DEGREE,
	-30 * ONE_DEGREE,  30 * ONE_DEGREE,
	-10 * ONE_DEGREE,  10 * ONE_DEGREE
};

void EarthQuake(int16_t item_number)
{
	auto item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->item_flags[1] == 0)
		item->item_flags[1] = BIG_RUMBLE;

	if (item->item_flags[2] == 0 && abs(item->item_flags[0] - item->item_flags[1]) < 16)
	{
		if (item->item_flags[1] == LOW_RUMBLE)
		{
			item->item_flags[1] = BIG_RUMBLE;
			item->item_flags[2] = (GetRandomControl() & 127) + 90;

		}
		else
		{
			item->item_flags[1] = LOW_RUMBLE;
			item->item_flags[2] = (GetRandomControl() & 127) + 30;
		}
	}

	if (item->item_flags[2])
		--item->item_flags[2];

	if (item->item_flags[0] > item->item_flags[1])
		item->item_flags[0] -= (GetRandomControl() & 7) + 2;
	else item->item_flags[0] += (GetRandomControl() & 7) + 2;

	int pitch = (0x10000 + (item->item_flags[0] << 8)) << 8;

	g_audio->play_sound(107);

	camera.bounce = -item->item_flags[0];

	if (int want = GetRandomControl(); want < 0x400)
	{
		want = (want < 0x200) ? FLAME_EMITTER : FALLING_CEILING1;

		for (auto earth_item = room[item->room_number].item_number; earth_item != NO_ITEM; earth_item = items[earth_item].next_item)
		{
			item = &items[earth_item];

			if (item->object_number != want)
				continue;

			if (item->status == ACTIVE || item->status == DEACTIVATED)
				continue;

			AddActiveItem(earth_item);

			item->status = ACTIVE;
			item->flags = CODE_BITS;
			item->timer = 0;

			return;
		}
	}
}

void ControlCutShotgun(int16_t item_number)
{
	auto item = &items[item_number];

	item->status = INVISIBLE;
}

void MiniCopterControl(int16_t item_number)
{
	auto item = &items[item_number];

	item->pos.z_pos += 100;

	PHD_3DPOS pos { lara_item->pos.x_pos + ((item->pos.x_pos - lara_item->pos.x_pos) >> 2),
					lara_item->pos.y_pos + ((item->pos.y_pos - lara_item->pos.y_pos) >> 2),
					lara_item->pos.z_pos + ((item->pos.z_pos - lara_item->pos.z_pos) >> 2) };

	g_audio->play_sound(297, { pos.x_pos, pos.y_pos, pos.z_pos });

	if (ABS(item->pos.z_pos - lara_item->pos.z_pos) > WALL_L * 30)
		KillItem(item_number);

	AnimateItem(item);

	auto room_number = item->room_number;

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);
}

void InitialiseWindow(int16_t item_number)
{
	auto item = &items[item_number];

	item->flags = 0;
	item->mesh_bits = 0x1;

	auto r = &room[item->room_number];
	auto floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];

	if (boxes[floor->box].overlap_index & BLOCKABLE)
		boxes[floor->box].overlap_index |= BLOCKED;
}

void SmashWindow(int16_t item_number)
{
	auto item = &items[item_number];

	auto r = &room[item->room_number];
	auto floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];

	if (boxes[floor->box].overlap_index & BLOCKABLE)
		boxes[floor->box].overlap_index &= (~BLOCKED);

	if (item->object_number == SMASH_WINDOW)
		g_audio->play_sound(214, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	else
	{
		g_audio->play_sound(105, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		g_audio->play_sound(106, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}

	item->mesh_bits = 0x0fffe;
	item->collidable = 0;

	ExplodingDeath(item_number, 0x0fefe, 0);

	item->flags |= ONESHOT;

	if (item->status == ACTIVE)
		RemoveActiveItem(item_number);

	item->status = DEACTIVATED;
}

void WindowControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->flags & ONESHOT)
		return;

	if (lara.skidoo == NO_ITEM)
	{
		if (!item->touch_bits)
			return;

		item->touch_bits = 0;

		if (ABS(lara_item->speed * phd_cos((int16_t)(lara_item->pos.y_rot - item->pos.y_rot)) >> W2V_SHIFT) < 50)
			return;
	}
	else if (!ItemNearLara(lara_item, &item->pos, WALL_L / 2))
		return;

	SmashWindow(item_number);
}

void ShutThatDoor(DOORPOS_DATA* d)
{
	if (auto floor = d->floor)
	{
		floor->index = 0;
		floor->box = NO_BOX;
		floor->floor = floor->ceiling = NO_HEIGHT >> 8;
		floor->pit_room = floor->sky_room = NO_ROOM;

		if (d->block != NO_BOX)
			boxes[d->block].overlap_index |= BLOCKED;
	}
}

void OpenThatDoor(DOORPOS_DATA* d)
{
	if (auto floor = d->floor)
	{
		memcpy(floor, &d->data, sizeof(FLOOR_INFO));

		if (d->block != NO_BOX)
			boxes[d->block].overlap_index &= (~BLOCKED);
	}
}

void InitialiseDoor(int16_t item_number)
{
	auto item = &items[item_number];
	auto door = (DOOR_DATA*)game_malloc(sizeof(DOOR_DATA), EXTRA_DOOR_STUFF);

	item->data = door;

	int dx = 0,
		dy = 0;

	if (item->pos.y_rot == 0)			 --dx;
	else if (item->pos.y_rot == -0x8000) ++dx;
	else if (item->pos.y_rot == 0x4000)  --dy;
	else							 	 ++dy;

	auto r = &room[item->room_number];

	door->d1.floor = &r->floor[(((item->pos.z_pos - r->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - r->x) >> WALL_SHIFT) + dy) * r->x_size];

	auto room_number = GetDoor(door->d1.floor);

	int16_t box_number;

	if (room_number == NO_ROOM)
		box_number = door->d1.floor->box;
	else
	{
		auto b = &room[room_number];

		box_number = b->floor[(((item->pos.z_pos - b->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - b->x) >> WALL_SHIFT) + dy) * b->x_size].box;
	}

	door->d1.block = (boxes[box_number].overlap_index & BLOCKABLE) ? box_number : NO_BOX;

	memcpy(&door->d1.data, door->d1.floor, sizeof(FLOOR_INFO));

	if (r->flipped_room != -1)
	{
		r = &room[r->flipped_room];

		door->d1flip.floor = &r->floor[(((item->pos.z_pos - r->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - r->x) >> WALL_SHIFT) + dy) * r->x_size];
		room_number = GetDoor(door->d1flip.floor);

		if (room_number == NO_ROOM)
			box_number = door->d1flip.floor->box;
		else
		{
			auto b = &room[room_number];

			box_number = b->floor[(((item->pos.z_pos - b->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - b->x) >> WALL_SHIFT) + dy) * b->x_size].box;
		}

		door->d1flip.block = (boxes[box_number].overlap_index & BLOCKABLE) ? box_number : NO_BOX;

		memcpy(&door->d1flip.data, door->d1flip.floor, sizeof(FLOOR_INFO));
	}
	else door->d1flip.floor = nullptr;

	auto two_room = GetDoor(door->d1.floor);

	if (item->object_number != DOOR_TYPE9)
	{
		ShutThatDoor(&door->d1);
		ShutThatDoor(&door->d1flip);
	}

	if (two_room == NO_ROOM)
	{
		door->d2.floor = nullptr;
		door->d2flip.floor = nullptr;
	}
	else
	{
		r = &room[two_room];

		door->d2.floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
		room_number = GetDoor(door->d2.floor);

		if (room_number == NO_ROOM)
			box_number = door->d2.floor->box;
		else
		{
			auto b = &room[room_number];
			box_number = b->floor[((item->pos.z_pos - b->z) >> WALL_SHIFT) + ((item->pos.x_pos - b->x) >> WALL_SHIFT) * b->x_size].box;
		}

		door->d2.block = (boxes[box_number].overlap_index & BLOCKABLE) ? box_number : NO_BOX;

		memcpy(&door->d2.data, door->d2.floor, sizeof(FLOOR_INFO));

		if (r->flipped_room != -1)
		{
			r = &room[r->flipped_room];

			door->d2flip.floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
			room_number = GetDoor(door->d2flip.floor);

			if (room_number == NO_ROOM)
				box_number = door->d2flip.floor->box;
			else
			{
				auto b = &room[room_number];
				box_number = b->floor[((item->pos.z_pos - b->z) >> WALL_SHIFT) + ((item->pos.x_pos - b->x) >> WALL_SHIFT) * b->x_size].box;
			}
			door->d2flip.block = (boxes[box_number].overlap_index & BLOCKABLE) ? box_number : NO_BOX;

			memcpy(&door->d2flip.data, door->d2flip.floor, sizeof(FLOOR_INFO));
		}
		else door->d2flip.floor = nullptr;

		if (item->object_number != DOOR_TYPE9)
		{
			ShutThatDoor(&door->d2);
			ShutThatDoor(&door->d2flip);
		}

		room_number = item->room_number;

		ItemNewRoom(item_number, two_room);

		item->room_number = room_number;
	}
}

void DoorControl(int16_t item_number)
{
	auto item = &items[item_number];
	auto door = (DOOR_DATA*)item->data;

	if (TriggerActive(item))
	{
		if (item->current_anim_state == DOOR_CLOSED)
			item->goal_anim_state = DOOR_OPEN;
		else
		{
			if (item->object_number != DOOR_TYPE9)
			{
				OpenThatDoor(&door->d1);
				OpenThatDoor(&door->d2);
				OpenThatDoor(&door->d1flip);
				OpenThatDoor(&door->d2flip);
			}
		}
	}
	else
	{
		if (item->current_anim_state == DOOR_OPEN)
			item->goal_anim_state = DOOR_CLOSED;
		else
		{
			if (item->object_number != DOOR_TYPE9)
			{
				ShutThatDoor(&door->d1);
				ShutThatDoor(&door->d2);
				ShutThatDoor(&door->d1flip);
				ShutThatDoor(&door->d2flip);
			}
		}
	}

	AnimateItem(item);
}

int OnDrawBridge(ITEM_INFO* item, int32_t x, int32_t y)
{
	int ix = item->pos.z_pos >> WALL_SHIFT,
		iy = item->pos.x_pos >> WALL_SHIFT;

	x >>= WALL_SHIFT;
	y >>= WALL_SHIFT;

	if (item->pos.y_rot == 0 && y == iy && (x == ix - 1 || x == ix - 2))			return 1;
	else if (item->pos.y_rot == -0x8000 && y == iy && (x == ix + 1 || x == ix + 2)) return 1;
	else if (item->pos.y_rot == 0x4000 && x == ix && (y == iy - 1 || y == iy - 2))  return 1;
	else if (item->pos.y_rot == -0x4000 && x == ix && (y == iy + 1 || y == iy + 2)) return 1;

	return 0;
}

void DrawBridgeFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (item->current_anim_state == DOOR_OPEN)
	{
		if (!OnDrawBridge(item, z, x))
			return;

		if (y <= item->pos.y_pos)
		{
			OnObject = 1;

			*height = item->pos.y_pos;

			if (item == lara_item)
				lara_item->item_flags[0] = 1;
		}
	}
}

void DrawBridgeCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (item->current_anim_state == DOOR_OPEN)
	{
		if (!OnDrawBridge(item, z, x))
			return;

		if (y > item->pos.y_pos)
			*height = item->pos.y_pos + STEP_L;
	}
}

void DrawBridgeCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	if (auto item = &items[item_num]; item->current_anim_state == DOOR_CLOSED)
		DoorCollision(item_num, laraitem, coll);
}

void InitialiseLift(int16_t item_number)
{
	auto lift = (LIFT_INFO*)game_malloc(sizeof(LIFT_INFO), 0);

	items[item_number].data = lift;

	lift->start_height = items[item_number].pos.y_pos;
	lift->wait_time = 0;
}

void LiftControl(int16_t item_number)
{
	auto item = &items[item_number];
	auto lift = (LIFT_INFO*)item->data;

	if (TriggerActive(item))
	{
		if (item->pos.y_pos < lift->start_height + STEP_L * 22 - 16)
		{
			if (lift->wait_time < LIFT_WAIT_TIME)
			{
				item->goal_anim_state = DOOR_OPEN;
				++lift->wait_time;
			}
			else
			{
				item->goal_anim_state = DOOR_CLOSED;
				item->pos.y_pos += 16;
			}
		}
		else
		{
			item->goal_anim_state = DOOR_OPEN;
			lift->wait_time = 0;
		}
	}
	else
	{
		if (item->pos.y_pos > lift->start_height + 16)
		{
			if (lift->wait_time < LIFT_WAIT_TIME)
			{
				item->goal_anim_state = DOOR_OPEN;
				++lift->wait_time;
			}
			else
			{
				item->goal_anim_state = DOOR_CLOSED;
				item->pos.y_pos -= 16;
			}
		}
		else
		{
			item->goal_anim_state = DOOR_OPEN;
			lift->wait_time = 0;
		}
	}

	AnimateItem(item);

	auto room_number = item->room_number;

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);
}

void LiftFloorCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* floor, int32_t* ceiling)
{
	x >>= WALL_SHIFT;
	z >>= WALL_SHIFT;

	int ix = item->pos.x_pos >> WALL_SHIFT,
		iz = item->pos.z_pos >> WALL_SHIFT,
		lx = lara_item->pos.x_pos >> WALL_SHIFT,
		lz = lara_item->pos.z_pos >> WALL_SHIFT,
		point_in_shaft = ((x == ix || x + 1 == ix) && (z == iz || z - 1 == iz)),
		lift_floor = item->pos.y_pos,
		lift_ceiling = item->pos.y_pos - LIFT_HEIGHT;

	*floor = 0x7fff;
	*ceiling = -0x7fff;

	if ((lx == ix || lx + 1 == ix) && (lz == iz || lz - 1 == iz))
	{
		if (item->current_anim_state == DOOR_CLOSED && lara_item->pos.y_pos < lift_floor + STEP_L && lara_item->pos.y_pos > lift_ceiling + STEP_L)
		{
			if (point_in_shaft)
			{
				*floor = lift_floor;
				*ceiling = lift_ceiling + STEP_L;
			}
			else
			{
				*floor = NO_HEIGHT;
				*ceiling = 0x7fff;
			}
		}
		else if (point_in_shaft)
		{
			if (lara_item->pos.y_pos < lift_ceiling + STEP_L)
				*floor = lift_ceiling;
			else if (lara_item->pos.y_pos < lift_floor + STEP_L)
			{
				*floor = lift_floor;
				*ceiling = lift_ceiling + STEP_L;
			}
			else *ceiling = lift_floor + STEP_L;
		}
	}
	else if (point_in_shaft)
	{
		if (y <= lift_ceiling)
			*floor = lift_ceiling;
		else if (y < lift_floor + STEP_L)
		{
			if (item->current_anim_state == DOOR_CLOSED)
			{
				*floor = NO_HEIGHT;
				*ceiling = 0x7fff;
			}
			else
			{
				*floor = lift_floor;
				*ceiling = lift_ceiling + STEP_L;
			}
		}
		else *ceiling = lift_floor + STEP_L;
	}
}

void LiftFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	int32_t floor, ceiling;

	LiftFloorCeiling(item, x, y, z, &floor, &ceiling);

	if (floor < *height)
	{
		OnObject = 1;
		*height = floor;
	}
}

void LiftCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	int32_t floor, ceiling;

	LiftFloorCeiling(item, x, y, z, &floor, &ceiling);

	if (ceiling > *height)
		*height = ceiling;
}

void BridgeFlatFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (y <= item->pos.y_pos)
	{
		*height = item->pos.y_pos;
		height_type = WALL;

		OnObject = 1;

		if (item == lara_item)
			lara_item->item_flags[0] = 1;
	}
}

void BridgeFlatCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (y > item->pos.y_pos)
		*height = item->pos.y_pos + STEP_L;
}

int32_t GetOffset(ITEM_INFO* item, int32_t x, int32_t z)
{
	if (item->pos.y_rot == 0)			 return ((WALL_L - x) & (WALL_L - 1));
	else if (item->pos.y_rot == -0x8000) return (x & (WALL_L - 1));
	else if (item->pos.y_rot == 0x4000)  return (z & (WALL_L - 1));

	return ((WALL_L - z) & (WALL_L - 1));
}

void BridgeTilt1Floor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (int level = item->pos.y_pos + (GetOffset(item, x, z) >> 2); y <= level)
	{
		*height = level;
		height_type = WALL;

		OnObject = 1;

		if (item == lara_item)
			lara_item->item_flags[0] = 1;
	}
}

void BridgeTilt1Ceiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (int level = item->pos.y_pos + (GetOffset(item, x, z) >> 2); y > level)
		*height = level + STEP_L;
}

void BridgeTilt2Floor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (int level = item->pos.y_pos + (GetOffset(item, x, z) >> 1); y <= level)
	{
		*height = level;
		height_type = WALL;

		OnObject = 1;

		if (item == lara_item)
			lara_item->item_flags[0] = 1;
	}
}

void BridgeTilt2Ceiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (int level = item->pos.y_pos + (GetOffset(item, x, z) >> 1); y > level)
		*height = level + STEP_L;
}

void CopterControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->current_anim_state == COPTER_SPIN && (item->flags & ONESHOT))
		item->goal_anim_state = COPTER_TAKEOFF;

	AnimateItem(item);

	auto room_number = item->room_number;

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);

	auto bounds = GetBoundsAccurate(item);

	PHD_VECTOR pos;

	pos.x = (bounds[0] + bounds[1]) >> 1;
	pos.y = (bounds[2] + bounds[3]) >> 1;
	pos.z = (bounds[4] + bounds[5]) >> 1;
	pos.x = lara_item->pos.x_pos + ((pos.x - lara_item->pos.x_pos) >> 2);
	pos.y = lara_item->pos.y_pos + ((pos.y - lara_item->pos.y_pos) >> 2);
	pos.z = lara_item->pos.z_pos + ((pos.z - lara_item->pos.z_pos) >> 2);

	g_audio->play_sound(297, { pos.x, pos.y, pos.z });

	if (item->status == DEACTIVATED)
		KillItem(item_number);
}

void GeneralControl(int16_t item_number)
{
	auto item = &items[item_number];

	item->goal_anim_state = (TriggerActive(item) ? DOOR_OPEN : DOOR_CLOSED);

	AnimateItem(item);

	auto room_number = item->room_number;

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);

	if (item->status == DEACTIVATED)
	{
		RemoveActiveItem(item_number);
		item->flags |= ONESHOT;
	}
}

void DetonatorControl(int16_t item_number)
{
	auto item = &items[item_number];

	AnimateItem(item);

	if (item->frame_number - anims[item->anim_number].frame_base == 80)
	{
		camera.bounce = -150;
		g_audio->play_sound(105);
	}

	if (item->status == DEACTIVATED)
		RemoveActiveItem(item_number);
}

void ControlAnimating_1_4(short item_number)
{
	if (auto item = &items[item_number]; TriggerActive(item))
		AnimateItem(item);
}

void PoleCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll)
{
	if (!enable_engine_extended_features)
		return;

	ITEM_INFO* item = &items[item_num];

	if ((input & IN_ACTION) &&
		lara.gun_status == LG_ARMLESS &&
		l->current_anim_state == AS_STOP &&
		l->anim_number == BREATH_A)
	{
		int16_t roty = item->pos.y_rot;

		item->pos.y_rot = l->pos.y_rot;

		if (TestLaraPosition(PoleBounds, item, l))
		{
			if (MoveLaraPosition(&PolePos, item, l))
			{
				l->anim_number = STAT2POLE_A;
				l->frame_number = STAT2POLE_F;
				l->current_anim_state = AS_POLESTAT;
				lara.gun_status = LG_HANDSBUSY;
			}
		}
		else lara.gun_status = LG_ARMLESS;

		item->pos.y_rot = roty;
	}
	else if ((input & IN_ACTION) &&
			lara.gun_status == LG_ARMLESS &&
			l->gravity_status &&
			l->fallspeed > 0 &&
			(l->current_anim_state == AS_REACH ||
			 l->current_anim_state == AS_UPJUMP))
	{
		if (TestBoundsCollide(item, l, LARA_RAD) && TestCollision(item, l))
		{
			auto roty = item->pos.y_rot;

			item->pos.y_rot = l->pos.y_rot;

			if (l->current_anim_state == AS_REACH)
			{
				PolePosR.y = l->pos.y_pos - item->pos.y_pos + 10;

				l->anim_number = REACH2POLE_A;
				l->frame_number = REACH2POLE_F;
			}
			else
			{
				PolePosR.y = l->pos.y_pos - item->pos.y_pos + 66;

				l->anim_number = JUMP2POLE_A;
				l->frame_number = JUMP2POLE_F;
			}

			AlignLaraPosition(&PolePosR, item, l);
			
			l->gravity_status = 0;
			l->fallspeed = 0;
			l->current_anim_state = AS_POLESTAT;

			lara.gun_status = LG_HANDSBUSY;

			item->pos.y_rot = roty;
		}
	}
	else if ((l->current_anim_state < AS_POLESTAT || l->current_anim_state > AS_POLEDOWN) && l->current_anim_state != AS_BACKJUMP)
		ObjectCollision(item_num, l, coll);
}