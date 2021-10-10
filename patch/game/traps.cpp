#include <specific/standard.h>
#include <specific/global.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "effect2.h"
#include "sphere.h"
#include "traps.h"
#include "kayak.h"
#include "game.h"

#include <specific/init.h>
#include <specific/fn_stubs.h>

BITE_INFO teeth1a = { -23, 0, -1718, 0 },
		  teeth1b = { 71, 0,- 1718, 1 },
		  teeth2a = { -23, 10, -1718, 0 },
		  teeth2b = { 71, 10, -1718, 1 },
		  teeth3a = { -23, -10, -1718, 0 },
		  teeth3b = { 71, -10, -1718, 1 };

void TriggerPendulumFlame(int16_t item_number);

void PropellerControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item) && !(item->flags & ONESHOT))
	{
		item->goal_anim_state = PROP_ON;

		if (item->touch_bits & 6)
		{
			if (enable_propeller_insta_death)
			{
				lara_item->hit_points = -1;
				DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - (STEP_L * 2), lara_item->pos.z_pos, (int16_t)(GetRandomControl() >> 10), (int16_t)(item->pos.y_rot + 0x4000), lara_item->room_number, 5);
			}
			else lara_item->hit_points -= PROP_DAMAGE;

			lara_item->hit_status = 1;

			DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - (STEP_L * 2), lara_item->pos.z_pos, (int16_t)(GetRandomControl() >> 10), (int16_t)(item->pos.y_rot + 0x4000), lara_item->room_number, 3);

			if (item->object_number == SAW)
				g_audio->play_sound(207, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		}
		else if (item->object_number == SAW) g_audio->play_sound(206, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		else if (item->object_number == FAN) g_audio->play_sound(215, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		else								 g_audio->play_sound(217, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}
	else if (item->goal_anim_state != PROP_OFF)
	{
		if (item->object_number == FAN)
			g_audio->play_sound(216, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

		item->goal_anim_state = PROP_OFF;
	}

	AnimateItem(item);

	if (item->status == DEACTIVATED)
	{
		RemoveActiveItem(item_number);

		if (item->object_number != SAW)
			item->collidable = 0;
	}
}

void ControlSpikeWall(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item) && item->status != DEACTIVATED)
	{
		int z = item->pos.z_pos + (SPIKE_SPEED * phd_cos(item->pos.y_rot) >> WALL_SHIFT),
			x = item->pos.x_pos + (SPIKE_SPEED * phd_sin(item->pos.y_rot) >> WALL_SHIFT);

		auto room_number = item->room_number;
		auto floor = GetFloor(x, item->pos.y_pos, z, &room_number);

		if (GetHeight(floor, x, item->pos.y_pos, z) != item->pos.y_pos)
		{
			item->status = DEACTIVATED;

			g_audio->stop_sound(147);
		}
		else
		{
			item->pos.z_pos = z;
			item->pos.x_pos = x;

			if (room_number != item->room_number)
				ItemNewRoom(item_number, room_number);

			g_audio->play_sound(147, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		}
	}

	if (item->touch_bits)
	{
		lara_item->hit_points -= SPIKEWALL_DAMAGE;
		lara_item->hit_status = 1;

		DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - (WALL_L / 2), lara_item->pos.z_pos, SPIKE_SPEED, item->pos.y_rot, lara_item->room_number, 3);

		item->touch_bits = 0;

		g_audio->play_sound(56, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}
}

void ControlCeilingSpikes(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item) && item->status != DEACTIVATED)
	{
		int y = item->pos.y_pos + ((item->item_flags[0] == 1) ? 10 : 5);

		auto room_number = item->room_number;
		auto floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);

		if (GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos) < y + WALL_L)
		{
			item->status = DEACTIVATED;

			g_audio->stop_sound(147);
		}
		else
		{
			item->pos.y_pos = y;

			if (room_number != item->room_number)
				ItemNewRoom(item_number, room_number);

			g_audio->play_sound(147, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		}
	}

	if (item->touch_bits)
	{
		lara_item->hit_points -= SPIKEWALL_DAMAGE;
		lara_item->hit_status = 1;

		DoLotsOfBlood(lara_item->pos.x_pos, item->pos.y_pos + WALL_L * 3 / 4, lara_item->pos.z_pos, SPIKE_SPEED, item->pos.y_rot, lara_item->room_number, 3);

		item->touch_bits = 0;

		g_audio->play_sound(56, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}

	if (TriggerActive(item) && item->status != DEACTIVATED && item->item_flags[0] == 1)
		AnimateItem(item);
}

void HookControl(int16_t item_number)
{
	static int clonked = 0;

	auto item = &items[item_number];

	if (item->touch_bits && !clonked)
	{
		lara_item->hit_points -= HOOK_DAMAGE;
		lara_item->hit_status = 1;

		DoLotsOfBlood(
			lara_item->pos.x_pos,
			lara_item->pos.y_pos - (WALL_L / 2),
			lara_item->pos.z_pos,
			lara_item->speed,
			lara_item->pos.y_rot,
			lara_item->room_number,
			3);
	}
	else clonked = 0;

	AnimateItem(item);
}

void SpinningBlade(int16_t item_number)
{
	auto item = &items[item_number];

	bool spinning = false;

	if (item->current_anim_state == SPIN_ROLL)
	{
		if (item->goal_anim_state != SPIN_STOP)
		{
			int z = item->pos.z_pos + (WALL_L * 3 / 2 * phd_cos(item->pos.y_rot) >> W2V_SHIFT),
				x = item->pos.x_pos + (WALL_L * 3 / 2 * phd_sin(item->pos.y_rot) >> W2V_SHIFT);

			auto room_number = item->room_number;
			auto floor = GetFloor(x, item->pos.y_pos, z, &room_number);

			if (GetHeight(floor, x, item->pos.y_pos, z) == NO_HEIGHT)
				item->goal_anim_state = SPIN_STOP;
		}

		spinning = true;

		if (item->touch_bits)
		{
			lara_item->hit_status = 1;
			lara_item->hit_points -= SPINBLADE_DAMAGE;

			DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - STEP_L * 2, lara_item->pos.z_pos, (int16_t)(item->speed * 2), lara_item->pos.y_rot, lara_item->room_number, 2);
		}

	}
	else if (TriggerActive(item))
		item->goal_anim_state = SPIN_ROLL;

	AnimateItem(item);

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	item->floor = item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);

	if (spinning && item->current_anim_state == SPIN_STOP)
		item->pos.y_rot += -0x8000;
}

void IcicleControl(int16_t item_number)
{
	auto item = &items[item_number];

	switch (item->current_anim_state)
	{
	case ICICLE_BREAK:
		item->goal_anim_state = ICICLE_FALL;
		break;
	case ICICLE_FALL:
	{
		if (item->gravity_status == 0)
		{
			item->fallspeed = 50;
			item->gravity_status = 1;
		}

		if (item->touch_bits)
		{
			lara_item->hit_points -= ICICLE_DAMAGE;
			lara_item->hit_status = 1;
		}

		break;
	}
	case ICICLE_LAND:
		item->gravity_status = 0;
	}

	AnimateItem(item);

	if (item->status == DEACTIVATED)
	{
		RemoveActiveItem(item_number);
		return;
	}

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->current_anim_state == ICICLE_FALL && item->pos.y_pos >= item->floor)
	{
		item->goal_anim_state = ICICLE_LAND;
		item->pos.y_pos = item->floor;
		item->fallspeed = 0;
		item->gravity_status = 0;
		item->mesh_bits = 0x2b;

		g_audio->play_sound(69, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}
}

void InitialiseBlade(int16_t item_number)
{
	auto item = &items[item_number];

	item->anim_number = objects[BLADE].anim_index + 2;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = STATUE_STOP;
}

void BladeControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item) && item->current_anim_state == STATUE_STOP)
		item->goal_anim_state = STATUE_CUT;
	else item->goal_anim_state = STATUE_STOP;

	if ((item->touch_bits & 0x2) && item->current_anim_state == STATUE_CUT)
	{
		lara_item->hit_status = 1;
		lara_item->hit_points -= BLADE_DAMAGE;

		DoLotsOfBlood(lara_item->pos.x_pos, item->pos.y_pos - STEP_L, lara_item->pos.z_pos, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number, 2);
	}

	AnimateItem(item);
}

void InitialiseKillerStatue(int16_t item_number)
{
	auto item = &items[item_number];

	item->anim_number = objects[item->object_number].anim_index + 3;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = STATUE_STOP;
}

void SpringBoardControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->current_anim_state == BOARD_OFF && lara_item->pos.y_pos == item->pos.y_pos &&
		(lara_item->pos.x_pos >> WALL_SHIFT) == (item->pos.x_pos >> WALL_SHIFT) &&
		(lara_item->pos.z_pos >> WALL_SHIFT) == (item->pos.z_pos >> WALL_SHIFT))
	{
		if (lara_item->hit_points <= 0)
			return;

		if (lara_item->current_anim_state == AS_BACK || lara_item->current_anim_state == AS_FASTBACK)
			lara_item->speed = -lara_item->speed;

		lara_item->fallspeed = -240;
		lara_item->gravity_status = 1;
		lara_item->anim_number = FALLDOWN_A;
		lara_item->frame_number = FALLDOWN_F;
		lara_item->current_anim_state = AS_FORWARDJUMP;
		lara_item->goal_anim_state = AS_FORWARDJUMP;

		item->goal_anim_state = BOARD_ON;
	}

	AnimateItem(item);
}

void InitialiseRollingBall(int16_t item_number)
{
	GAME_VECTOR* old;

	auto item = &items[item_number];

	item->data = old = (GAME_VECTOR*)game_malloc(sizeof(GAME_VECTOR), ROLLINGBALL_STUFF);

	old->x = item->pos.x_pos;
	old->y = item->pos.y_pos;
	old->z = item->pos.z_pos;
	old->room_number = item->room_number;
}

void RollingBallControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->status == ACTIVE)
	{
		if (item->goal_anim_state == TRAP_WORKING)
		{
			AnimateItem(item);
			return;
		}

		if (item->pos.y_pos < item->floor)
		{
			if (!item->gravity_status)
			{
				item->gravity_status = 1;
				item->fallspeed = -10;
			}
		}
		else if (item->current_anim_state == TRAP_SET)
			item->goal_anim_state = TRAP_ACTIVATE;

		int oldx = item->pos.x_pos,
			oldz = item->pos.z_pos;

		AnimateItem(item);

		auto room_number = item->room_number;
		auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (item->room_number != room_number)
			ItemNewRoom(item_number, room_number);

		item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		TestTriggers(trigger_index, 1);

		if (item->pos.y_pos >= item->floor - STEP_L)
		{
			item->gravity_status = 0;
			item->fallspeed = 0;
			item->pos.y_pos = item->floor;

			g_audio->play_sound(147, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			int dist = phd_sqrt(SQUARE(camera.mike_pos.x - item->pos.x_pos) + SQUARE(camera.mike_pos.z - item->pos.z_pos));
			if (dist < ROLL_SHAKE_RANGE)
				camera.bounce = -40 * (ROLL_SHAKE_RANGE - dist) / ROLL_SHAKE_RANGE;
		}

		int dist, ydist;

		if (item->object_number == ROLLING_BALL)
		{
			dist = (128 * 3) - 64;
			ydist = (STEP_L * 3) + 64;
		}
		else if (item->object_number == BIG_ROLLING_BALL)
		{
			dist = (128 * 9) - 64;
			ydist = (STEP_L * 8) + 64;
		}
		else
		{
			dist = WALL_L;
			ydist = WALL_L;
		}

		int x = item->pos.x_pos + (dist * phd_sin(item->pos.y_rot) >> W2V_SHIFT),
			z = item->pos.z_pos + (dist * phd_cos(item->pos.y_rot) >> W2V_SHIFT);

		floor = GetFloor(x, item->pos.y_pos, z, &room_number);

		room_number = item->room_number;
		floor = GetFloor(x, item->pos.y_pos - ydist, z, &room_number);

		if (GetHeight(floor, x, item->pos.y_pos, z) < item->pos.y_pos ||
			GetCeiling(floor, x, item->pos.y_pos - ydist, z) >(item->pos.y_pos - ydist))
		{
			if (item->object_number == OILDRUMS)
			{
				g_audio->stop_sound(147);

				item->goal_anim_state = TRAP_WORKING;
			}
			else
			{
				g_audio->stop_sound(147);

				item->status = DEACTIVATED;
			}

			item->pos.y_pos = item->floor;
			item->pos.x_pos = oldx;
			item->pos.z_pos = oldz;
			item->speed = item->fallspeed = 0;
			item->touch_bits = 0;
		}
	}
	else if (item->status == DEACTIVATED)
	{
		if (!TriggerActive(item))
		{
			auto old = (GAME_VECTOR*)item->data;

			item->status = NOT_ACTIVE;
			item->pos.x_pos = old->x;
			item->pos.y_pos = old->y;
			item->pos.z_pos = old->z;

			if (item->room_number != old->room_number)
			{
				RemoveDrawnItem(item_number);

				auto r = &room[old->room_number];

				item->next_item = r->item_number;
				item->room_number = old->room_number;

				r->item_number = item_number;
			}

			item->current_anim_state = item->goal_anim_state = TRAP_SET;
			item->anim_number = objects[item->object_number].anim_index;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
			item->required_anim_state = 0;

			RemoveActiveItem(item_number);
		}
	}
}

void RollingBallCollision(int16_t item_num, ITEM_INFO* litem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (item->status == ACTIVE)
	{
		if (!TestBoundsCollide(item, litem, coll->radius) || !TestCollision(item, litem))
			return;

		if (litem->gravity_status)
		{
			if (coll->enable_baddie_push)
				ItemPushLara(item, litem, coll, coll->enable_spaz, 1);

			litem->hit_points -= ROLLINGBALL_DAMAGE_AIR;

			int x = litem->pos.x_pos - item->pos.x_pos,
				y = (litem->pos.y_pos - 350) - (item->pos.y_pos - WALL_L / 2),
				z = litem->pos.z_pos - item->pos.z_pos;

			auto d = (int16_t)phd_sqrt(x * x + y * y + z * z);
			if (d < WALL_L / 2)
				d = WALL_L / 2;

			x = item->pos.x_pos + (x * WALL_L / 2) / d;
			y = item->pos.y_pos - WALL_L / 2 + (y * WALL_L / 2) / d;
			z = item->pos.z_pos + (z * WALL_L / 2) / d;

			DoBloodSplat(x, y, z, item->speed, item->pos.y_rot, item->room_number);
		}
		else
		{
			litem->hit_status = 1;

			if (litem->hit_points > 0)
			{
				litem->hit_points = -1;
				litem->pos.y_rot = item->pos.y_rot;
				litem->pos.x_rot = litem->pos.z_rot = 0;

				litem->anim_number = RBALL_DEATH_A;
				litem->frame_number = RBALL_DEATH_F;
				litem->current_anim_state = AS_SPECIAL;
				litem->goal_anim_state = AS_SPECIAL;

				camera.flags = FOLLOW_CENTRE;
				camera.target_angle = 170 * ONE_DEGREE;
				camera.target_elevation = -25 * ONE_DEGREE;

				for (int i = 0; i < 15; ++i)
				{
					int x = litem->pos.x_pos + (GetRandomControl() - 16384) / 256,
						z = litem->pos.z_pos + (GetRandomControl() - 16384) / 256,
						y = litem->pos.y_pos - GetRandomControl() / 64;

					auto d = (int16_t)((GetRandomControl() - 16384) / 8 + item->pos.y_rot);

					DoBloodSplat(x, y, z, (int16_t)(item->speed * 2), d, item->room_number);
				}
			}
		}
	}
	else if (item->status != INVISIBLE)
		ObjectCollision(item_num, litem, coll);
}

void SpikeControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (enable_island_spikes_sound)
	{
		if (item->frame_number == anims[item->anim_number].frame_base)
			g_audio->play_sound(259, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

		AnimateItem(item);
	}
}

void SpikeCollision(int16_t item_num, ITEM_INFO* litem, COLL_INFO* coll)
{
	auto item = &items[item_num];

	if (litem->hit_points < 0 || !TestBoundsCollide(item, litem, coll->radius) || !TestCollision(item, litem))
		return;

	int num = GetRandomControl() / 24576;

	if (litem->gravity_status)
	{
		if (litem->fallspeed > 6)
		{
			litem->hit_points = -1;

			num = 20;
		}
	}
	else if (litem->speed < 30)
		return;

	litem->hit_points -= SPIKE_DAMAGE;

	for (; num > 0; --num)
	{
		int x = litem->pos.x_pos + (GetRandomControl() - 16384) / 256,
			z = litem->pos.z_pos + (GetRandomControl() - 16384) / 256,
			y = litem->pos.y_pos - GetRandomControl() / 64;

		DoBloodSplat(x, y, z, 20, (int16_t)GetRandomControl(), item->room_number);
	}

	if (litem->hit_points <= 0)
	{
		litem->anim_number = SPIKE_DEATH_A;
		litem->frame_number = SPIKE_DEATH_F;
		litem->current_anim_state = AS_DEATH;
		litem->goal_anim_state = AS_DEATH;
		litem->pos.y_pos = item->pos.y_pos;
		litem->gravity_status = 0;
	}
}

void TrapDoorControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item))
	{
		if (item->current_anim_state == DOOR_CLOSED)
			item->goal_anim_state = DOOR_OPEN;
	}
	else if (item->current_anim_state == DOOR_OPEN)
		item->goal_anim_state = DOOR_CLOSED;

	AnimateItem(item);
}

void TrapDoorFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (!OnTrapDoor(item, x, z) || y > item->pos.y_pos)
		return;

	if (item->current_anim_state == DOOR_CLOSED && item->pos.y_pos < *height)
	{
		*height = item->pos.y_pos;

		OnObject = 1;
		height_type = WALL;

		if (item == lara_item)
			lara_item->item_flags[0] = 1;
	}
}

void TrapDoorCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	if (!OnTrapDoor(item, x, z) || y <= item->pos.y_pos)
		return;

	if (item->current_anim_state == DOOR_CLOSED && item->pos.y_pos > *height)
	{
		*height = item->pos.y_pos + STEP_L;

		CeilingObject = item;
	}
}

int OnTrapDoor(ITEM_INFO* item, int32_t x, int32_t z)
{
	x >>= WALL_SHIFT;
	z >>= WALL_SHIFT;

	int tx = item->pos.x_pos >> WALL_SHIFT,
		tz = item->pos.z_pos >> WALL_SHIFT;

	if ((item->pos.y_rot == 0 && x == tx && (z == tz || z == tz + 1)) ||
		(item->pos.y_rot == -0x8000 && x == tx && (z == tz || z == tz - 1)) ||
		(item->pos.y_rot == 0x4000 && z == tz && (x == tx || x == tx + 1)) ||
		(item->pos.y_rot == -0x4000 && z == tz && (x == tx || x == tx - 1)))
		return 1;

	return 0;
}

void Pendulum(int16_t item_number)
{
	auto item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->touch_bits)
	{
		lara_item->hit_points -= 50;
		lara_item->hit_status = 1;

		if (item->item_flags[0] == 0)
		{
			int x = lara_item->pos.x_pos + (GetRandomControl() - 16384) / 256,
				z = lara_item->pos.z_pos + (GetRandomControl() - 16384) / 256,
				y = lara_item->pos.y_pos - GetRandomControl() / 44;

			auto d = (int16_t)((GetRandomControl() - 16384) / 8 + lara_item->pos.y_rot);

			DoBloodSplat(x, y, z, lara_item->speed, d, lara_item->room_number);
		}
		else LaraBurn();
	}

	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &item->room_number);

	item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->item_flags[0])
	{
		TriggerPendulumFlame(item_number);

		PHD_VECTOR pos { 0, -512, 0 };

		GetJointAbsPosition(item, &pos, 5);

		TriggerDynamicLight(pos.x, pos.y, pos.z, 11, 24 + (GetRandomControl() & 7), 12 + (GetRandomControl() & 3), 0);
	}

	AnimateItem(item);
}

void FallingBlock(int16_t item_number)
{
	int32_t origin;

	auto item = &items[item_number];

	origin = (item->object_number == FALLING_PLANK ? WALL_L : STEP_L * 2);

	switch (item->current_anim_state)
	{
	case TRAP_SET:
	{
		if (lara_item->pos.y_pos == item->pos.y_pos - origin)
			item->goal_anim_state = TRAP_ACTIVATE;
		else
		{
			item->status = NOT_ACTIVE;

			RemoveActiveItem(item_number);

			return;
		}

		break;
	}
	case TRAP_ACTIVATE:
		item->goal_anim_state = TRAP_WORKING;
		break;
	case TRAP_WORKING:
	{
		if (item->goal_anim_state != TRAP_FINISHED)
			item->gravity_status = 1;
	}
	}

	AnimateItem(item);

	if (item->status == DEACTIVATED)
	{
		RemoveActiveItem(item_number);
		return;
	}

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->current_anim_state == TRAP_WORKING && item->pos.y_pos >= item->floor)
	{
		item->goal_anim_state = TRAP_FINISHED;
		item->pos.y_pos = item->floor;
		item->fallspeed = 0;
		item->gravity_status = 0;
	}
}

void FallingBlockFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	int origin = (item->object_number == FALLING_PLANK ? WALL_L : STEP_L * 2);

	if (y <= item->pos.y_pos - origin && (item->current_anim_state == TRAP_SET || item->current_anim_state == TRAP_ACTIVATE))
	{
		*height = item->pos.y_pos - origin;

		OnObject = 1;
	}
}

void FallingBlockCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height)
{
	int origin = (item->object_number == FALLING_PLANK ? WALL_L : STEP_L * 2);

	if (y > item->pos.y_pos - origin && (item->current_anim_state == TRAP_SET || item->current_anim_state == TRAP_ACTIVATE))
		*height = item->pos.y_pos + STEP_L - origin;
}

void TeethTrap(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item))
	{
		item->goal_anim_state = TT_NASTY;

		if (item->touch_bits && item->current_anim_state == TT_NASTY)
		{
			lara_item->hit_points -= TEETHTRAP_DAMAGE;
			lara_item->hit_status = 1;

			BaddieBiteEffect(item, &teeth1a);
			BaddieBiteEffect(item, &teeth1b);
			BaddieBiteEffect(item, &teeth2a);
			BaddieBiteEffect(item, &teeth2b);
			BaddieBiteEffect(item, &teeth3a);
			BaddieBiteEffect(item, &teeth3b);
		}
	}
	else item->goal_anim_state = TT_NICE;

	AnimateItem(item);
}

void FallingCeiling(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->current_anim_state == TRAP_SET)
	{
		item->goal_anim_state = TRAP_ACTIVATE;
		item->gravity_status = 1;
	}
	else if (item->current_anim_state == TRAP_ACTIVATE)
	{
		if (item->touch_bits)
		{
			lara_item->hit_points -= FALLING_CEILING_DAMAGE;
			lara_item->hit_status = 1;
		}
	}

	AnimateItem(item);

	if (item->status == DEACTIVATED)
	{
		RemoveActiveItem(item_number);
		return;
	}

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);

	if (item->current_anim_state == TRAP_ACTIVATE && item->pos.y_pos >= item->floor)
	{
		item->goal_anim_state = TRAP_WORKING;
		item->pos.y_pos = item->floor;
		item->fallspeed = 0;
		item->gravity_status = 0;
	}
}

void DartEmitterControl(int16_t item_num)
{
	auto item = &items[item_num];

	if (item->active)
	{
		if (item->timer <= 0)
			item->timer = 24;
		else
		{
			--item->timer;
			return;
		}
	}

	auto dart_num = CreateItem();

	if (dart_num != NO_ITEM)
	{
		auto dart = &items[dart_num];

		dart->object_number = DARTS;
		dart->room_number = item->room_number;

		int x = 0,
			z = 0;

		switch (item->pos.y_rot)
		{
		case 0: z = WALL_L / 2;		  break;
		case 16384: x = WALL_L / 2;   break;
		case -32768: z = -WALL_L / 2; break;
		case -16384: x = -WALL_L / 2; break;
		}

		dart->pos.x_pos = item->pos.x_pos + x;
		dart->pos.y_pos = item->pos.y_pos - WALL_L / 2;
		dart->pos.z_pos = item->pos.z_pos + z;

		InitialiseItem(dart_num);

		dart->pos.x_rot = 0;
		dart->pos.y_rot = item->pos.y_rot + 0x8000;
		dart->speed = DART_SPEED;

		int xand = 0,
			zand = 0;

		if (x)
			xand = abs(x << 1) - 1;
		else zand = abs(z << 1) - 1;

		for (int i = 0; i < 5; ++i)
		{
			int rnd = -GetRandomControl();

			TriggerDartSmoke(dart->pos.x_pos, dart->pos.y_pos, dart->pos.z_pos, (x < 0) ? -(rnd & xand) : (rnd & xand), (z < 0) ? -(rnd & zand) : (rnd & zand), 0);
		}

		AddActiveItem(dart_num);

		dart->status = ACTIVE;

		g_audio->play_sound(247, { dart->pos.x_pos, dart->pos.y_pos, dart->pos.z_pos });
	}
}

void DartsControl(int16_t item_num)
{
	auto item = &items[item_num];

	if (item->touch_bits)
	{
		lara_item->hit_points -= 25;
		lara_item->hit_status = 1;

		lara.poisoned += 160;

		DoBloodSplat(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number);
		
		return KillItem(item_num);
	}

	int ox = item->pos.x_pos,
		oz = item->pos.z_pos,
		speed = (item->speed * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;

	item->pos.z_pos += (speed * phd_cos(item->pos.y_rot)) >> W2V_SHIFT;
	item->pos.x_pos += (speed * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;
	item->pos.y_pos += -((item->speed * phd_sin(item->pos.x_rot)) >> W2V_SHIFT);

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_num, room_number);

	item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->pos.y_pos >= item->floor)
	{
		for (int i = 0; i < 4; ++i)
			TriggerDartSmoke(ox, item->pos.y_pos, oz, 0, 0, 1);

		KillItem(item_num);
	}
}

void SideFlameEmitterControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item))
	{
		if (!item->data)
		{
			auto fx_number = CreateEffect(item->room_number);

			if (fx_number != NO_ITEM)
			{
				auto fx = &effects[fx_number];

				fx->pos.x_pos = item->pos.x_pos;
				fx->pos.y_pos = item->pos.y_pos;
				fx->pos.z_pos = item->pos.z_pos;
				fx->pos.y_rot = item->pos.y_rot;
				fx->frame_number = SIDE_FIRE;
				fx->object_number = FLAME;
				fx->counter = fx->flag1 = fx->flag2 = 0;
			}

			item->data = (void*)(fx_number + 1);
		}
	}
	else if (item->data)
	{
		KillEffect((int16_t)((int)(item->data) - 1));

		item->data = nullptr;
	}
}

void FlameEmitterControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item))
	{
		if (!item->data)
		{
			auto fx_number = CreateEffect(item->room_number);

			if (fx_number != NO_ITEM)
			{
				auto fx = &effects[fx_number];

				fx->pos.x_pos = item->pos.x_pos;
				fx->pos.y_pos = item->pos.y_pos;
				fx->pos.z_pos = item->pos.z_pos;
				fx->pos.y_rot = item->pos.y_rot;
				fx->frame_number = BIG_FIRE;
				fx->object_number = FLAME;
				fx->counter = 0;
			}

			item->data = (void*)(fx_number + 1);
		}
	}
	else if (item->data)
	{
		KillEffect((int16_t)((int)(item->data) - 1));

		item->data = nullptr;
	}
}

void FlameEmitter2Control(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item))
	{
		if (!item->data)
		{
			auto fx_number = CreateEffect(item->room_number);

			if (fx_number != NO_ITEM)
			{
				auto fx = &effects[fx_number];

				fx->pos.x_pos = item->pos.x_pos;
				fx->pos.y_pos = item->pos.y_pos;
				fx->pos.z_pos = item->pos.z_pos;
				fx->pos.y_rot = item->pos.y_rot;
				fx->frame_number = SMALL_FIRE;
				fx->object_number = FLAME;
				fx->counter = 0;
			}

			item->data = (void*)(fx_number + 1);
		}
	}
	else if (item->data)
	{
		KillEffect((int16_t)((int)(item->data) - 1));

		item->data = nullptr;
	}
}

void FlameEmitter3Control(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item))
	{
		if (!item->data)
		{
			auto fx_number = CreateEffect(item->room_number);

			if (fx_number != NO_ITEM)
			{
				auto fx = &effects[fx_number];

				fx->pos.x_pos = item->pos.x_pos;
				fx->pos.y_pos = item->pos.y_pos;
				fx->pos.z_pos = item->pos.z_pos;
				fx->frame_number = JET_FIRE;
				fx->object_number = FLAME;
				fx->counter = fx->flag1 = 0;
				fx->flag2 = GetRandomControl() & 63;
			}

			item->data = (void*)(fx_number + 1);
		}
	}
	else if (item->data)
	{
		KillEffect((int16_t)((int)(item->data) - 1));

		item->data = nullptr;
	}
}

void SideFlameDetection(FX_INFO* fx, long length)
{
	int dx = lara_item->pos.x_pos - fx->pos.x_pos,
		dz = lara_item->pos.z_pos - fx->pos.z_pos;

	if (dx < -0x5000 || dx > 0x5000 || dz < -0x5000 || dz > 0x5000)
		return;

	int x, z, xs, zs, xe, ze;

	switch (fx->pos.y_rot)
	{
	case 0:
		x = fx->pos.x_pos;
		z = fx->pos.z_pos + 512;
		xs = -FIRE_WIDTH;
		xe = FIRE_WIDTH;
		ze = 0;
		zs = -length;
		break;
	case 16384:
		x = fx->pos.x_pos + 512;
		z = fx->pos.z_pos;
		xe = 0;
		xs = -length;
		zs = -FIRE_WIDTH;
		ze = FIRE_WIDTH;
		break;
	case -32768:
		x = fx->pos.x_pos;
		z = fx->pos.z_pos - 512;
		xs = -FIRE_WIDTH;
		xe = FIRE_WIDTH;
		zs = 0;
		ze = length;
		break;
	case -16384:
		x = fx->pos.x_pos - 512;
		z = fx->pos.z_pos;
		xs = 0;
		xe = length;
		zs = -FIRE_WIDTH;
		ze = FIRE_WIDTH;
		break;
	default:
		return;
	}

	auto bounds = GetBoundsAccurate(lara_item);

	if (lara_item->pos.x_pos < x + xs || lara_item->pos.x_pos > x + xe ||
		lara_item->pos.z_pos < z + zs || lara_item->pos.z_pos > z + ze ||
		lara_item->pos.y_pos + bounds[2] > fx->pos.y_pos + 128 || lara_item->pos.y_pos + bounds[3] < fx->pos.y_pos - 384)
		return;

	if (fx->flag1 < 18)
	{
		lara_item->hit_points -= FLAME_TOONEAR_DAMAGE;
		lara_item->hit_status = 1;
	}
	else LaraBurn();
}

void FlameControl(int16_t fx_number)
{
	static constexpr unsigned char xzoffs[16][2] =
	{
		{ 9, 9 },
		{ 24, 9 },
		{ 40, 9 },
		{ 55, 9 },
		{ 9, 24 },
		{ 24, 24 },
		{ 40, 24 },
		{ 55, 24 },
		{ 9, 40 },
		{ 24, 40 },
		{ 40, 40 },
		{ 55, 40 },
		{ 9, 55 },
		{ 24, 55 },
		{ 40, 55 },
		{ 55, 55 }
	};

	auto fx = &effects[fx_number];

	int distance = GetRandomControl(),
		ox = 0,
		oz = 0,
		od = 0;
	
	if (fx->frame_number == BIG_FIRE)
	{
		if ((wibble & 12) == 0)
		{
			TriggerFireFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1, 0);
			TriggerFireSmoke(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1, 0);
		}

		TriggerStaticFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, (GetRandomControl() & 15) + 96);
	}
	else if (fx->frame_number == SMALL_FIRE)
	{
		if (fx->counter < 0)
		{
			draw_lara_fire(lara_item, lara.burn_red, lara.burn_green, lara.burn_blue);

			fx->pos.x_pos = lara_item->pos.x_pos;
			fx->pos.y_pos = lara_item->pos.y_pos;
			fx->pos.z_pos = lara_item->pos.z_pos;

			if (lara_item->room_number != fx->room_number)
				EffectNewRoom(fx_number, lara_item->room_number);

			auto destroy_local_fire = [&]()
			{
				fx->counter = 0;

				KillEffect(fx_number);

				g_audio->stop_sound(150);

				lara.burn = 0;
			};

			int y = GetWaterHeight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->room_number);

			if (y != NO_HEIGHT && fx->pos.y_pos > y)
			{
				if ((room[fx->room_number].flags & SWAMP) && enable_deadly_swamp)
				{
					g_audio->play_sound(150, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });

					lara_item->hit_points -= FLAME_ONFIRE_DAMAGE;
					lara_item->hit_status = 1;
				}
				else destroy_local_fire();
			}
			else
			{
				if (!lara.burn)
					destroy_local_fire();
				else
				{
					g_audio->play_sound(150, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });

					lara_item->hit_points -= FLAME_ONFIRE_DAMAGE;
					lara_item->hit_status = 1;
				}
			}

			return;
		}

		int r = ((fx->pos.y_rot >> 4) & 4095) << 1,
			z = (m_cos(r) * 288) >> 12,
			x = (m_sin(r) * 288) >> 12,
			g = (GetRandomControl() & 15) + 32;

		TriggerStaticFlame(fx->pos.x_pos + x, fx->pos.y_pos - 192, fx->pos.z_pos + z, g);

		if ((wibble & 24) == 0) TriggerFireFlame(fx->pos.x_pos + x, fx->pos.y_pos - 224, fx->pos.z_pos + z, -1, 1);
		if ((wibble & 24) == 0) TriggerFireSmoke(fx->pos.x_pos + x, fx->pos.y_pos, fx->pos.z_pos + z, -1, 1);
	}
	else if (fx->frame_number == JET_FIRE)
	{
		if (fx->flag1 == 0)
		{
			fx->flag1 = (GetRandomControl() & 3) + 8;

			int x = GetRandomControl() & 63;

			if (fx->flag2 == x)
				x = (x + 13) & 63;

			fx->flag2 = (int16_t)x;
		}
		else --fx->flag1;

		int x = (xzoffs[fx->flag2 & 7][0] << 4) - 512,
			z = (xzoffs[fx->flag2 & 7][1] << 4) - 512;

		if ((wibble & 4) == 0)
			TriggerFireFlame(fx->pos.x_pos + x, fx->pos.y_pos, fx->pos.z_pos + z, -1, 2);

		x = (xzoffs[(fx->flag2 >> 3) + 8][0] << 4) - 512;
		z = (xzoffs[(fx->flag2 >> 3) + 8][1] << 4) - 512;

		if (wibble & 4)
			TriggerFireFlame(fx->pos.x_pos + x, fx->pos.y_pos, fx->pos.z_pos + z, -1, 2);
	}
	else
	{
		int r = ((fx->pos.y_rot >> 4) & 4095) << 1;

		oz = (m_cos(r) * (512 + (distance & 255))) >> 12;
		ox = (m_sin(r) * (512 + (distance & 255))) >> 12;

		if (fx->flag2)
		{
			if (wibble & 4)
				TriggerSideFlame(fx->pos.x_pos + ox, fx->pos.y_pos, fx->pos.z_pos + oz, (r + 4096) & 8191, ((GetRandomControl() & 7) == 0), 1);
			
			--fx->flag2;
		}
		else
		{
			if (fx->flag1)
			{
				if (wibble & 4)
				{
					if (fx->flag1 > 112)	 TriggerSideFlame(fx->pos.x_pos + ox, fx->pos.y_pos, fx->pos.z_pos + oz, (r + 4096) & 8190, (129 - fx->flag1) >> 1, 0);
					else if (fx->flag1 < 18) TriggerSideFlame(fx->pos.x_pos + ox, fx->pos.y_pos, fx->pos.z_pos + oz, (r + 4096) & 8190, (fx->flag1 >> 1) + 1, 0);
					else					 TriggerSideFlame(fx->pos.x_pos + ox, fx->pos.y_pos, fx->pos.z_pos + oz, (r + 4096) & 8190, 9, 0);
				}

				fx->flag1 -= 2;
			}
			else
			{
				fx->flag1 = 128;
				fx->flag2 = (enable_rapids_fire_type ? 30 * 4 : 30 * 2);
			}
		}
	}

	int x = fx->pos.x_pos + ((distance & 0xf) << 5),
		y = fx->pos.y_pos + ((distance & 0xf0) << 1),
		z = fx->pos.z_pos + ((distance & 0xf00) >> 3);

	if (fx->frame_number == SIDE_FIRE)
	{
		if (fx->flag2)			 od = 0;
		else if (fx->flag1 < 18) od = 2048;
		else if (fx->flag1 < 64) od = 2048;
		else					 od = (128 - fx->flag1) << 5;

		int r = (((fx->pos.y_rot >> 4) + 2048) & 4095) << 1;

		oz = (m_cos(r) * od) >> 12;
		ox = (m_sin(r) * od) >> 12;

		TriggerDynamicLight(x + ox, y, z + oz, (fx->flag2 == 0) ? 13 : 6, 24 + (distance & 7), 12 + ((distance >> 4) & 3), 0);
	}
	else TriggerDynamicLight(x, y, z, 16 - (fx->frame_number << 2), 24 + (distance & 7), 12 + ((distance >> 4) & 3), 0);

	g_audio->play_sound(150, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });

	if (fx->counter)
		--fx->counter;
	else if (fx->frame_number == SIDE_FIRE)
	{
		if (!lara.burn && od)
			SideFlameDetection(fx, od);
	}
	else if (fx->frame_number != SMALL_FIRE)
	{
		PHD_3DPOS pos { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos };

		if (ItemNearLara(lara_item, &pos, 600))
		{
			x = lara_item->pos.x_pos - pos.x_pos;
			z = lara_item->pos.z_pos - pos.z_pos;
			distance = x * x + z * z;

			lara_item->hit_points -= FLAME_TOONEAR_DAMAGE;
			lara_item->hit_status = 1;

			if (distance < SQUARE(450))
			{
				fx->counter = 100;

				LaraBurn();
			}
		}
	}
}

void LaraBurn()
{
	if (lara.burn || !lara_item)
		return;

	if (auto fx_number = CreateEffect(lara_item->room_number); fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		fx->frame_number = SMALL_FIRE;
		fx->object_number = FLAME;
		fx->counter = -1;
		fx->item = lara.item_number;

		lara.burn = 1;
	}
}

void draw_lara_fire(ITEM_INFO* item, int r, int g, int b)
{
	PHD_VECTOR base_pos { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos };

	for (int i = 0; i < MAX_LARA_MESHES; ++i)
	{
		if ((wibble & 12) == 0)
		{
			PHD_VECTOR bone_pos {};

			GetJointAbsPosition(item, &bone_pos, i);

			TriggerFireFlame(bone_pos.x, bone_pos.y, bone_pos.z, -1, r, g, b);
		}
	}

	TriggerDynamicLight(base_pos.x, base_pos.y, base_pos.z, 13, r & 7, g & 7, b & 7);
}

void LavaBurn(ITEM_INFO* item)
{
	if (item->hit_points < 0 || lara.water_status == LARA_CHEAT)
		return;
	
	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, 32000, item->pos.z_pos, &room_number);

	if (item->floor != GetHeight(floor, item->pos.x_pos, 32000, item->pos.z_pos))
		return;

	if (lava_type == LAVA_DEATH_RAPIDS)
		return LaraRapidsDrown();

	item->hit_status = 1;
	item->hit_points = -1;

	if (lava_type == LAVA_DEATH_FIRE)
		return LaraBurn();

	lara.electric = 1;
}

void TriggerPendulumFlame(int16_t item_number)
{
	int dx = lara_item->pos.x_pos - items[item_number].pos.x_pos,
		dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 7) + 32;

	sptr->On = 1;
	sptr->sR = 48 + (GetRandomControl() & 31);
	sptr->sG = sptr->sR >> 1;
	sptr->sB = 0;
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 28;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = ((GetRandomControl() & 31) - 16);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 31) - 16);
	sptr->Xvel = ((GetRandomControl() & 63) - 32);
	sptr->Yvel = -(GetRandomControl() & 15) - 16;
	sptr->Zvel = ((GetRandomControl() & 63) - 32);
	sptr->Friction = 4;
	sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;

	if (GetRandomControl() & 1)
	{
		sptr->Flags |= SP_ROTATE;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = (GetRandomControl() & 31) - 16;
	}

	sptr->FxObj = (uint8_t)item_number;
	sptr->NodeNumber = SPN_PENDULUMFLAME;
	sptr->Gravity = -(GetRandomControl() & 31) - 16;
	sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
	sptr->Def = (uint8_t)objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;
	sptr->Width = sptr->sWidth = (uint8_t)size;
	sptr->Height = sptr->sHeight = (uint8_t)size;
	sptr->dWidth = (uint8_t)(size >> 2);
	sptr->dHeight = (uint8_t)(size >> 2);
}