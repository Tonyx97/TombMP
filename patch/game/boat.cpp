#include <specific/standard.h>
#include <specific/stypes.h>
#include <specific/init.h>
#include <specific/input.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "sphere.h"
#include "effect2.h"
#include "physics.h"
#include "boat.h"
#include "game.h"

#include <mp/game/level.h>

#define	IN_ACCELERATE		IN_ACTION
#define	IN_REVERSE			IN_JUMP
#define	IN_DISMOUNT			IN_ROLL
#define	IN_TURBO			(IN_DASH|IN_DUCK)
#define	IN_TURNL			(IN_LEFT|IN_STEPL)
#define	IN_TURNR			(IN_RIGHT|IN_STEPR)
#define BOAT_GETONLW_ANIM	0
#define BOAT_GETONRW_ANIM	8
#define BOAT_GETONJ_ANIM	6
#define BOAT_GETON_START	1
#define BOAT_FALL_ANIM		15
#define BOAT_DEATH_ANIM		18
#define BOAT_UNDO_TURN		(ONE_DEGREE / 4)
#define BOAT_TURN			(ONE_DEGREE / 8)
#define BOAT_MAX_TURN		(ONE_DEGREE * 4)
#define BOAT_MAX_SPEED		110
#define BOAT_SLOW_SPEED		(BOAT_MAX_SPEED / 3)
#define BOAT_FAST_SPEED		(BOAT_MAX_SPEED + 75)
#define BOAT_MIN_SPEED		20
#define BOAT_ACCELERATION	5
#define BOAT_BRAKE			5
#define BOAT_SLOWDOWN		1
#define BOAT_REVERSE		-2
#define BOAT_MAX_BACK		-20
#define BOAT_MAX_KICK		-80
#define BOAT_SLIP			10
#define BOAT_SIDE_SLIP		30
#define BOAT_FRONT			750
#define BOAT_SIDE			300
#define BOAT_RADIUS			500
#define BOAT_SNOW			500
#define BOAT_MAX_HEIGHT		(STEP_L)
#define GETOFF_DIST			(1024)
#define BOAT_WAKE			700
#define BOAT_SOUND_CEILING	(WALL_L * 5)
#define BOAT_TIP			(BOAT_FRONT + 250)
#define NUM_WAKE_SPRITES	32
#define WAKE_SIZE 			32
#define WAKE_SPEED 			4
#define SKIDOO_HIT_LEFT		11
#define SKIDOO_HIT_RIGHT	12
#define SKIDOO_HIT_FRONT	13
#define SKIDOO_HIT_BACK		14

enum boat_anims
{
	BOAT_GETON,
	BOAT_STILL,
	BOAT_MOVING,
	BOAT_JUMPR,
	BOAT_JUMPL,
	BOAT_HIT,
	BOAT_FALL,
	BOAT_TURNR,
	BOAT_DEATH,
	BOAT_TURNL
};

struct WAKE_PTS
{
	int32_t x[2];
	int32_t y;
	int32_t z[2];
	int16_t xvel[2];
	int16_t zvel[2];
	uint8_t life;
	uint8_t pad[3];
};

WAKE_PTS WakePtsBoat[NUM_WAKE_SPRITES][2];
uint8_t CurrentStartWakeBoat = 0;
uint8_t WakeShadeBoat = 0;

void BoatSplash(ITEM_INFO* item, long fallspeed, long water);
void TriggerBoatMist(long x, long y, long z, long speed, int16_t angle, long snow);

int CanGetOff(int direction)
{
	auto v = &items[lara.skidoo];

	int angle = (direction < 0 ? v->pos.y_rot - 0x4000 : v->pos.y_rot + 0x4000);

	int x = v->pos.x_pos + (GETOFF_DIST * phd_sin(angle) >> W2V_SHIFT),
		y = v->pos.y_pos,
		z = v->pos.z_pos + (GETOFF_DIST * phd_cos(angle) >> W2V_SHIFT);

	auto room_number = v->room_number;
	auto floor = GetFloor(x, y, z, &room_number);

	int height = GetHeight(floor, x, y, z);

	if ((height - v->pos.y_pos) < -WALL_L / 2)
		return 0;

	if (height_type == BIG_SLOPE || height_type == DIAGONAL)
		return 0;

	int ceiling = GetCeiling(floor, x, y, z);

	return (ceiling - v->pos.y_pos <= -LARA_HITE && height - ceiling >= LARA_HITE);
}

void DoWake(ITEM_INFO* v, int16_t xoff, int16_t zoff, int16_t rotate)
{
	int32_t xv[2], zv[2];

	if (WakePtsBoat[CurrentStartWakeBoat][rotate].life)
		return;

	int c = phd_cos(v->pos.y_rot),
		s = phd_sin(v->pos.y_rot),
		x = v->pos.x_pos + (((zoff * s) + (xoff * c)) >> W2V_SHIFT),
		z = v->pos.z_pos + (((zoff * c) - (xoff * s)) >> W2V_SHIFT);

	auto room_number = v->room_number;
	auto floor = GetFloor(x, v->pos.y_pos + 128, z, &room_number);

	int h = GetWaterHeight(x, v->pos.y_pos + 128, z, room_number);

	int16_t angle1, angle2;

	if (v->speed < 0)
	{
		if (!rotate)
		{
			angle1 = v->pos.y_rot - (ONE_DEGREE * 20);
			angle2 = v->pos.y_rot - (ONE_DEGREE * 40);
		}
		else
		{
			angle1 = v->pos.y_rot + (ONE_DEGREE * 20);
			angle2 = v->pos.y_rot + (ONE_DEGREE * 40);
		}
	}
	else
	{
		if (!rotate)
		{
			angle1 = v->pos.y_rot - (ONE_DEGREE * 160);
			angle2 = v->pos.y_rot - (ONE_DEGREE * 140);
		}
		else
		{
			angle1 = v->pos.y_rot + (ONE_DEGREE * 160);
			angle2 = v->pos.y_rot + (ONE_DEGREE * 140);
		}
	}

	xv[0] = (WAKE_SPEED * phd_sin(angle1)) >> W2V_SHIFT;
	zv[0] = (WAKE_SPEED * phd_cos(angle1)) >> W2V_SHIFT;
	xv[1] = ((WAKE_SPEED + 6) * phd_sin(angle2)) >> W2V_SHIFT;
	zv[1] = ((WAKE_SPEED + 6) * phd_cos(angle2)) >> W2V_SHIFT;

	WakePtsBoat[CurrentStartWakeBoat][rotate].y = v->pos.y_pos + 32;
	WakePtsBoat[CurrentStartWakeBoat][rotate].life = 0x40;

	for (int i = 0; i < 2; ++i)
	{
		WakePtsBoat[CurrentStartWakeBoat][rotate].x[i] = x;
		WakePtsBoat[CurrentStartWakeBoat][rotate].z[i] = z;
		WakePtsBoat[CurrentStartWakeBoat][rotate].xvel[i] = xv[i];
		WakePtsBoat[CurrentStartWakeBoat][rotate].zvel[i] = zv[i];
	}

	if (rotate == 1)
	{
		++CurrentStartWakeBoat;
		CurrentStartWakeBoat &= (NUM_WAKE_SPRITES - 1);
	}
}

void UpdateBoatWakeFX()
{
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < NUM_WAKE_SPRITES; ++j)
			if (WakePtsBoat[j][i].life)
			{
				--WakePtsBoat[j][i].life;
				WakePtsBoat[j][i].x[0] += WakePtsBoat[j][i].xvel[0];
				WakePtsBoat[j][i].z[0] += WakePtsBoat[j][i].zvel[0];
				WakePtsBoat[j][i].x[1] += WakePtsBoat[j][i].xvel[1];
				WakePtsBoat[j][i].z[1] += WakePtsBoat[j][i].zvel[1];
			}
}

int DoShift(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x = pos->x >> WALL_SHIFT,
		z = pos->z >> WALL_SHIFT,
		x_old = old->x >> WALL_SHIFT,
		z_old = old->z >> WALL_SHIFT,
		shift_x = pos->x & (WALL_L - 1),
		shift_z = pos->z & (WALL_L - 1);

	if (x == x_old)
	{
		if (z == z_old)
		{
			skidoo->pos.z_pos += (old->z - pos->z);
			skidoo->pos.x_pos += (old->x - pos->x);
		}
		else if (z > z_old)
		{
			skidoo->pos.z_pos -= shift_z + 1;

			return (pos->x - skidoo->pos.x_pos);
		}
		else
		{
			skidoo->pos.z_pos += WALL_L - shift_z;

			return (skidoo->pos.x_pos - pos->x);
		}
	}
	else if (z == z_old)
	{
		if (x > x_old)
		{
			skidoo->pos.x_pos -= shift_x + 1;

			return (skidoo->pos.z_pos - pos->z);
		}
		else
		{
			skidoo->pos.x_pos += WALL_L - shift_x;

			return (pos->z - skidoo->pos.z_pos);
		}
	}
	else
	{
		x = z = 0;

		auto room_number = skidoo->room_number;
		auto floor = GetFloor(old->x, pos->y, pos->z, &room_number);

		int height = GetHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_L)
			z = (pos->z > old->z ? -shift_z - 1 : WALL_L - shift_z);

		room_number = skidoo->room_number;
		floor = GetFloor(pos->x, pos->y, old->z, &room_number);
		height = GetHeight(floor, pos->x, pos->y, old->z);

		if (height < old->y - STEP_L)
			x = (pos->x > old->x ? -shift_x - 1 : WALL_L - shift_x);

		if (x && z)
		{
			skidoo->pos.z_pos += z;
			skidoo->pos.x_pos += x;
		}
		else if (z)
		{
			skidoo->pos.z_pos += z;

			return (z > 0 ? skidoo->pos.x_pos - pos->x : pos->x - skidoo->pos.x_pos);
		}
		else if (x)
		{
			skidoo->pos.x_pos += x;

			return (x > 0 ? pos->z - skidoo->pos.z_pos : skidoo->pos.z_pos - pos->z);
		}
		else
		{
			skidoo->pos.z_pos += (old->z - pos->z);
			skidoo->pos.x_pos += (old->x - pos->x);
		}
	}

	return 0;
}

int GetCollisionAnim(ITEM_INFO* skidoo, PHD_VECTOR* moved)
{
	moved->x = skidoo->pos.x_pos - moved->x;
	moved->z = skidoo->pos.z_pos - moved->z;

	if (moved->x || moved->z)
	{
		int c = phd_cos(skidoo->pos.y_rot),
			s = phd_sin(skidoo->pos.y_rot),
			front = (moved->z * c + moved->x * s) >> W2V_SHIFT,
			side = (-moved->z * s + moved->x * c) >> W2V_SHIFT;

		if (ABS(front) > ABS(side)) return (front > 0 ? SKIDOO_HIT_BACK : SKIDOO_HIT_FRONT);
		else						return (side > 0 ? SKIDOO_HIT_LEFT : SKIDOO_HIT_RIGHT);
	}

	return 0;
}

int32_t TestWaterHeight(ITEM_INFO* item, int32_t z_off, int32_t x_off, PHD_VECTOR* pos)
{
	int c = phd_cos(item->pos.y_rot),
		s = phd_sin(item->pos.y_rot);

	pos->x = item->pos.x_pos + ((z_off * s + x_off * c) >> W2V_SHIFT);
	pos->y = item->pos.y_pos - (z_off * phd_sin(item->pos.x_rot) >> W2V_SHIFT) + (x_off * phd_sin(item->pos.z_rot) >> W2V_SHIFT);
	pos->z = item->pos.z_pos + ((z_off * c - x_off * s) >> W2V_SHIFT);

	auto room_number = item->room_number;

	GetFloor(pos->x, pos->y, pos->z, &room_number);

	int height = GetWaterHeight(pos->x, pos->y, pos->z, room_number);
	if (height == NO_HEIGHT)
	{
		auto floor = GetFloor(pos->x, pos->y, pos->z, &room_number);

		height = GetHeight(floor, pos->x, pos->y, pos->z);

		if (height == NO_HEIGHT)
			return height;
	}

	return height - 5;
}

void DoBoatShift(int boat_number)
{
	auto boat = &items[boat_number];
	auto item_number = room[boat->room_number].item_number;

	while (item_number != NO_ITEM)
	{
		auto item = &items[item_number];

		if (item->object_number == BOAT && item_number != boat_number && lara.skidoo != item_number)
		{
			int x = item->pos.x_pos - boat->pos.x_pos,
				z = item->pos.z_pos - boat->pos.z_pos,
				distance = x * x + z * z;

			if (distance < SQUARE(BOAT_RADIUS * 2))
			{
				boat->pos.x_pos = item->pos.x_pos - x * SQUARE(BOAT_RADIUS * 2) / distance;
				boat->pos.z_pos = item->pos.z_pos - z * SQUARE(BOAT_RADIUS * 2) / distance;
			}

			return;
		}

		item_number = item->next_item;
	}
}

int DoBoatDynamics(int height, int fallspeed, int* y)
{
	if (height > *y)
	{
		*y += fallspeed;

		if (*y > height)
		{
			*y = height;
			fallspeed = 0;
		}
		else fallspeed += GRAVITY;
	}
	else
	{
		fallspeed += ((height - *y - fallspeed) >> 3);

		if (fallspeed < -20)
			fallspeed = -20;

		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

int BoatDynamics(int16_t boat_number)
{
	PHD_VECTOR moved, fl, fr, br, bl, f;
	PHD_VECTOR fl_old, fr_old, bl_old, br_old, f_old;

	auto boat = &items[boat_number];
	auto binfo = (BOAT_INFO*)boat->data;

	boat->pos.z_rot -= binfo->tilt_angle;

	int hfl_old = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl_old),
		hfr_old = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr_old),
		hbl_old = TestWaterHeight(boat, -BOAT_FRONT, -BOAT_SIDE, &bl_old),
		hbr_old = TestWaterHeight(boat, -BOAT_FRONT, BOAT_SIDE, &br_old),
		hf_old = TestWaterHeight(boat, BOAT_TIP, 0, &f_old);

	PHD_VECTOR old { boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos };

	if (bl_old.y > hbl_old) bl_old.y = hbl_old;
	if (br_old.y > hbr_old) br_old.y = hbr_old;
	if (fl_old.y > hfl_old) fl_old.y = hfl_old;
	if (fr_old.y > hfr_old) fr_old.y = hfr_old;
	if (f_old.y > hf_old)   f_old.y = hf_old;

	boat->pos.y_rot += binfo->boat_turn + binfo->extra_rotation;
	binfo->tilt_angle = binfo->boat_turn * 6;

	boat->pos.z_pos += boat->speed * phd_cos(boat->pos.y_rot) >> W2V_SHIFT;
	boat->pos.x_pos += boat->speed * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;

	binfo->prop_rot += (boat->speed >= 0 ? (boat->speed * (ONE_DEGREE * 3)) + (ONE_DEGREE << 1) : ONE_DEGREE * 33);
	
	int slip = BOAT_SIDE_SLIP * phd_sin(boat->pos.z_rot) >> W2V_SHIFT;

	if (!slip && boat->pos.z_rot)
		slip = (boat->pos.z_rot > 0) ? 1 : -1;

	boat->pos.z_pos -= slip * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;
	boat->pos.x_pos += slip * phd_cos(boat->pos.y_rot) >> W2V_SHIFT;

	slip = BOAT_SLIP * phd_sin(boat->pos.x_rot) >> W2V_SHIFT;

	if (!slip && boat->pos.x_rot)
		slip = (boat->pos.x_rot > 0) ? 1 : -1;

	boat->pos.z_pos -= slip * phd_cos(boat->pos.y_rot) >> W2V_SHIFT;
	boat->pos.x_pos -= slip * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;

	moved.x = boat->pos.x_pos;
	moved.z = boat->pos.z_pos;

	DoBoatShift(boat_number);

	int rot = 0,
		hbl = TestWaterHeight(boat, -BOAT_FRONT, -BOAT_SIDE, &bl);

	if (hbl < bl_old.y - STEP_L / 2)
		rot = DoShift(boat, &bl, &bl_old);

	int hbr = TestWaterHeight(boat, -BOAT_FRONT, BOAT_SIDE, &br);
	if (hbr < br_old.y - STEP_L / 2)
		rot += DoShift(boat, &br, &br_old);

	int hfl = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl);
	if (hfl < fl_old.y - STEP_L / 2)
		rot += DoShift(boat, &fl, &fl_old);

	int hfr = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr);
	if (hfr < fr_old.y - STEP_L / 2)
		rot += DoShift(boat, &fr, &fr_old);

	if (!slip && TestWaterHeight(boat, BOAT_TIP, 0, &f) < f_old.y - STEP_L / 2)
		DoShift(boat, &f, &f_old);

	auto room_number = boat->room_number;
	auto floor = GetFloor(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, &room_number);

	int height = GetWaterHeight(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, room_number);

	if (height == NO_HEIGHT)
		height = GetHeight(floor, boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos);

	if (height < boat->pos.y_pos - STEP_L / 2)
		DoShift(boat, (PHD_VECTOR*)&boat->pos, &old);

	binfo->extra_rotation = rot;

	int collide = GetCollisionAnim(boat, &moved);

	if (slip || collide)
	{
		int newspeed = ((boat->pos.z_pos - old.z) * phd_cos(boat->pos.y_rot) + (boat->pos.x_pos - old.x) * phd_sin(boat->pos.y_rot)) >> W2V_SHIFT;

		if (lara.skidoo == boat_number && boat->speed > BOAT_MAX_SPEED + BOAT_ACCELERATION && newspeed < boat->speed - 10)
		{
			lara_item->hit_points -= boat->speed;
			lara_item->hit_status = 1;

			g_audio->play_sound(31, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

			newspeed >>= 1;
			boat->speed >>= 1;
		}

		if (slip)
		{
			if (boat->speed <= BOAT_MAX_SPEED + 10)
				boat->speed = newspeed;
		}
		else
		{
			if (boat->speed > 0 && newspeed < boat->speed)		boat->speed = newspeed;
			else if (boat->speed < 0 && newspeed > boat->speed) boat->speed = newspeed;
		}

		if (boat->speed < BOAT_MAX_BACK)
			boat->speed = BOAT_MAX_BACK;
	}

	return collide;
}

void InitialiseBoat(int16_t item_number)
{
	auto boat = &items[item_number];

	boat->data = (BOAT_INFO*)game_malloc(sizeof(BOAT_INFO), 0);

	auto binfo = (BOAT_INFO*)boat->data;

	binfo->boat_turn = 0;
	binfo->left_fallspeed = binfo->right_fallspeed = 0;
	binfo->tilt_angle = 0;
	binfo->extra_rotation = 0;
	binfo->water = 0;
	binfo->pitch = 0;

	for (int i = 0; i < NUM_WAKE_SPRITES; ++i)
	{
		WakePtsBoat[i][0].life = 0;
		WakePtsBoat[i][1].life = 0;
	}
}

int BoatCheckGeton(int16_t item_number, COLL_INFO* coll)
{
	if (lara.gun_status != LG_ARMLESS)
		return 0;

	auto boat = &items[item_number];

	int dist = ((lara_item->pos.z_pos - boat->pos.z_pos) * phd_cos(-boat->pos.y_rot) -
			    (lara_item->pos.x_pos - boat->pos.x_pos) * phd_sin(-boat->pos.y_rot)) >> W2V_SHIFT;

	if (dist > 512)
		return 0;

	int16_t rot = boat->pos.y_rot - lara_item->pos.y_rot,
		    geton = 0;

	if (lara.water_status == LARA_SURFACE || lara.water_status == LARA_WADE)
	{
		if (!(input & IN_ACTION) || lara_item->gravity_status || boat->speed)
			return 0;

		if (rot > 0x2000 && rot < 0x6000)		 geton = 1;
		else if (rot > -0x6000 && rot < -0x2000) geton = 2;
	}
	else if (lara.water_status == LARA_ABOVEWATER)
	{
		if (lara_item->fallspeed > 0)
		{
			if (lara_item->pos.y_pos + 512 > boat->pos.y_pos)
				geton = 3;
		}
		else if (lara_item->fallspeed == 0)
		{
			if (rot > -0x6000 && rot < 0x6000)
				geton = (lara_item->pos.x_pos == boat->pos.x_pos && lara_item->pos.y_pos == boat->pos.y_pos && lara_item->pos.z_pos == boat->pos.z_pos ? 4 : 3);
		}
	}

	if (!geton || !TestBoundsCollide(boat, lara_item, coll->radius) || !TestCollision(boat, lara_item))
		return 0;

	return geton;
}

void BoatCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
		return;

	int geton = BoatCheckGeton(item_number, coll);

	if (!geton)
	{
		coll->enable_baddie_push = 1;
		ObjectCollision(item_number, l, coll);
		return;
	}

	auto v = &items[item_number];

	if (auto entity = g_level->get_entity_by_item(v))
		if (!g_level->is_entity_streamed(entity))
		{
			ObjectCollision(item_number, l, coll);

			return g_level->request_entity_ownership(entity, true);
		}

	lara.skidoo = item_number;
	lara.water_status = LARA_ABOVEWATER;

	if (geton == 2)		 l->anim_number = objects[boat_anim_obj].anim_index + BOAT_GETONLW_ANIM;
	else if (geton == 1) l->anim_number = objects[boat_anim_obj].anim_index + BOAT_GETONRW_ANIM;
	else if (geton == 3) l->anim_number = objects[boat_anim_obj].anim_index + BOAT_GETONJ_ANIM;
	else				 l->anim_number = objects[boat_anim_obj].anim_index + BOAT_GETON_START;

	l->pos.x_pos = v->pos.x_pos;
	l->pos.y_pos = v->pos.y_pos - 5;
	l->pos.z_pos = v->pos.z_pos;
	l->pos.y_rot = v->pos.y_rot;
	l->pos.x_rot = l->pos.z_rot = 0;
	l->gravity_status = 0;
	l->speed = 0;
	l->fallspeed = 0;
	l->frame_number = anims[l->anim_number].frame_base;
	l->current_anim_state = l->goal_anim_state = BOAT_GETON;

	if (l->room_number != v->room_number)
		ItemNewRoom(lara.item_number, v->room_number);

	AnimateItem(l);

	if (items[item_number].status != ACTIVE)
	{
		AddActiveItem(item_number);
		items[item_number].status = ACTIVE;
	}
}

int BoatUserControl(ITEM_INFO* boat)
{
	auto binfo = (BOAT_INFO*)boat->data;

	int no_turn = 1;

	if (boat->pos.y_pos >= binfo->water - STEP_L / 2 && binfo->water != NO_HEIGHT)
	{
		if ((!(input & IN_DISMOUNT) && !(input & IN_LOOK)) || boat->speed)
		{
			if (((input & IN_TURNL) && !(input & IN_REVERSE)) || ((input & IN_TURNR) && (input & IN_REVERSE)))
			{
				if (binfo->boat_turn > 0)
					binfo->boat_turn -= BOAT_UNDO_TURN;
				else
				{
					binfo->boat_turn -= BOAT_TURN;

					if (binfo->boat_turn < -BOAT_MAX_TURN)
						binfo->boat_turn = -BOAT_MAX_TURN;
				}

				no_turn = 0;
			}
			else if (((input & IN_TURNR) && !(input & IN_REVERSE)) || ((input & IN_TURNL) && (input & IN_REVERSE)))
			{
				if (binfo->boat_turn < 0)
					binfo->boat_turn += BOAT_UNDO_TURN;
				else
				{
					binfo->boat_turn += BOAT_TURN;
					if (binfo->boat_turn > BOAT_MAX_TURN)
						binfo->boat_turn = BOAT_MAX_TURN;
				}
				no_turn = 0;
			}

			if (input & IN_REVERSE)
			{
				if (boat->speed > 0)				  boat->speed -= BOAT_BRAKE;
				else if (boat->speed > BOAT_MAX_BACK) boat->speed += BOAT_REVERSE;
			}
			else if (input & IN_ACCELERATE)
			{
				int max_speed = ((input & IN_TURBO) ? BOAT_FAST_SPEED : (input & IN_SLOW) ? BOAT_SLOW_SPEED : BOAT_MAX_SPEED);

				if (boat->speed < max_speed)					  boat->speed += BOAT_ACCELERATION / 2 + BOAT_ACCELERATION * boat->speed / (2 * max_speed);
				else if (boat->speed > max_speed + BOAT_SLOWDOWN) boat->speed -= BOAT_SLOWDOWN;
			}
			else if (boat->speed >= 0 && boat->speed < BOAT_MIN_SPEED && (input & (IN_TURNL | IN_TURNR)))
			{
				if (boat->speed == 0 && !(input & IN_DISMOUNT))
					boat->speed = BOAT_MIN_SPEED;
			}
			else if (boat->speed > BOAT_SLOWDOWN)
				boat->speed -= BOAT_SLOWDOWN;
			else boat->speed = 0;
		}
		else
		{
			if (boat->speed >= 0 && boat->speed < BOAT_MIN_SPEED && (input & (IN_TURNL | IN_TURNR)))
			{
				if (boat->speed == 0 && !(input & IN_DISMOUNT))
					boat->speed = BOAT_MIN_SPEED;
			}
			else if (boat->speed > BOAT_SLOWDOWN)
				boat->speed -= BOAT_SLOWDOWN;
			else boat->speed = 0;

			if ((input & IN_LOOK) && boat->speed == 0)
				LookUpDown();
		}
	}

	return no_turn;
}

void BoatAnimation(ITEM_INFO* boat, int collide)
{
	auto binfo = (BOAT_INFO*)boat->data;

	if (lara_item->hit_points <= 0)
	{
		if (lara_item->current_anim_state != BOAT_DEATH)
		{
			lara_item->anim_number = objects[boat_anim_obj].anim_index + BOAT_DEATH_ANIM;
			lara_item->frame_number = anims[lara_item->anim_number].frame_base;
			lara_item->current_anim_state = lara_item->goal_anim_state = BOAT_DEATH;
		}
	}
	else if (boat->pos.y_pos < binfo->water - STEP_L / 2 && boat->fallspeed > 0)
	{
		if (lara_item->current_anim_state != BOAT_FALL)
		{
			lara_item->anim_number = objects[boat_anim_obj].anim_index + BOAT_FALL_ANIM;
			lara_item->frame_number = anims[lara_item->anim_number].frame_base;
			lara_item->current_anim_state = lara_item->goal_anim_state = BOAT_FALL;
		}
	}
	else if (collide)
	{
		if (lara_item->current_anim_state != BOAT_HIT)
		{
			lara_item->anim_number = (int16_t)(objects[boat_anim_obj].anim_index + collide);
			lara_item->frame_number = anims[lara_item->anim_number].frame_base;
			lara_item->current_anim_state = lara_item->goal_anim_state = BOAT_HIT;
		}
	}
	else
	{
		switch (lara_item->current_anim_state)
		{
		case BOAT_STILL:
		{
			if (input & IN_DISMOUNT)
			{
				if (boat->speed == 0)
				{
					if ((input & IN_TURNR) && CanGetOff(boat->pos.y_rot + 0x4000))	  lara_item->goal_anim_state = BOAT_JUMPR;
					else if (input & IN_TURNL && CanGetOff(boat->pos.y_rot - 0x4000)) lara_item->goal_anim_state = BOAT_JUMPL;
				}
			}

			if (boat->speed > 0)
				lara_item->goal_anim_state = BOAT_MOVING;

			break;
		}
		case BOAT_MOVING:
		{
			if (boat->speed <= 0)	   lara_item->goal_anim_state = BOAT_STILL;
			else if (input & IN_TURNR) lara_item->goal_anim_state = BOAT_TURNR;
			else if (input & IN_TURNL) lara_item->goal_anim_state = BOAT_TURNL;

			break;
		}
		case BOAT_FALL:
			lara_item->goal_anim_state = BOAT_MOVING;
			break;
		case BOAT_TURNR:
		{
			if (boat->speed <= 0)		  lara_item->goal_anim_state = BOAT_STILL;
			else if (!(input & IN_TURNR)) lara_item->goal_anim_state = BOAT_MOVING;

			break;
		}
		case BOAT_TURNL:
		{
			if (boat->speed <= 0)		  lara_item->goal_anim_state = BOAT_STILL;
			else if (!(input & IN_TURNL)) lara_item->goal_anim_state = BOAT_MOVING;
		}
		}
	}
}
import prof;
void BoatControl(int16_t item_number)
{
	auto boat = &items[item_number];
	auto binfo = (BOAT_INFO*)boat->data;

	PHD_VECTOR fl, fr;

	int collide = BoatDynamics(item_number),
		hfl = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl),
		hfr = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr);

	auto room_number = boat->room_number;
	auto floor = GetFloor(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, &room_number);

	int height = GetHeight(floor, boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos),
		ceiling = GetCeiling(floor, boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos);

	if (lara.skidoo == item_number)
	{
		TestTriggers(trigger_index, 0);
		TestTriggers(trigger_index, 1);
	}

	int water = binfo->water = GetWaterHeight(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, room_number);

	bool no_turn = true,
		 drive = false;

	if (lara.skidoo == item_number && lara_item->hit_points > 0)
	{
		switch (lara_item->current_anim_state)
		{
		case BOAT_GETON:
		case BOAT_JUMPR:
		case BOAT_JUMPL:
			break;
		default:
			drive = true;
			no_turn = BoatUserControl(boat);
		}
	}
	else boat->speed -= (boat->speed > BOAT_SLOWDOWN ? BOAT_SLOWDOWN : 0);

	if (no_turn)
	{
		if (binfo->boat_turn < -BOAT_UNDO_TURN)		binfo->boat_turn += BOAT_UNDO_TURN;
		else if (binfo->boat_turn > BOAT_UNDO_TURN) binfo->boat_turn -= BOAT_UNDO_TURN;
		else										binfo->boat_turn = 0;
	}

	boat->floor = height - 5;

	if (binfo->water == NO_HEIGHT) binfo->water = height;
	else						   binfo->water -= 5;

	binfo->left_fallspeed = DoBoatDynamics(hfl, binfo->left_fallspeed, (int*)&fl.y);
	binfo->right_fallspeed = DoBoatDynamics(hfr, binfo->right_fallspeed, (int*)&fr.y);

	int ofs = boat->fallspeed;

	boat->fallspeed = DoBoatDynamics(binfo->water, boat->fallspeed, (int*)&boat->pos.y_pos);

	if (ofs - boat->fallspeed > 32 && boat->fallspeed == 0 && water != NO_HEIGHT)
		BoatSplash(boat, ofs - boat->fallspeed, water);

	height = fl.y + fr.y;
	height = (height < 0 ? -(abs(height) >> 1) : height >> 1);

	int x_rot = phd_atan(BOAT_FRONT, boat->pos.y_pos - height),
		z_rot = phd_atan(BOAT_SIDE, height - fl.y);

	boat->pos.x_rot += (x_rot - boat->pos.x_rot) >> 1;
	boat->pos.z_rot += (z_rot - boat->pos.z_rot) >> 1;

	if (!x_rot && ABS(boat->pos.x_rot) < 4) boat->pos.x_rot = 0;
	if (!z_rot && ABS(boat->pos.z_rot) < 4) boat->pos.z_rot = 0;

	if (lara.skidoo == item_number)
	{
		BoatAnimation(boat, collide);

		if (room_number != boat->room_number)
		{
			ItemNewRoom(item_number, room_number);
			ItemNewRoom(lara.item_number, room_number);
		}

		boat->pos.z_rot += binfo->tilt_angle;

		lara_item->pos.x_pos = boat->pos.x_pos;
		lara_item->pos.y_pos = boat->pos.y_pos;
		lara_item->pos.z_pos = boat->pos.z_pos;
		lara_item->pos.x_rot = boat->pos.x_rot;
		lara_item->pos.y_rot = boat->pos.y_rot;
		lara_item->pos.z_rot = boat->pos.z_rot;

		AnimateItem(lara_item);

		if (lara_item->hit_points > 0)
		{
			boat->anim_number = objects[BOAT].anim_index + (lara_item->anim_number - objects[boat_anim_obj].anim_index);
			boat->frame_number = anims[boat->anim_number].frame_base + (lara_item->frame_number - anims[lara_item->anim_number].frame_base);
		}

		camera.target_elevation = -20 * ONE_DEGREE;
		camera.target_distance = WALL_L * 2;
	}
	else
	{
		if (room_number != boat->room_number)
			ItemNewRoom(item_number, room_number);

		boat->pos.z_rot += binfo->tilt_angle;
	}

	int pitch = boat->speed;

	binfo->pitch += (pitch - binfo->pitch) >> 2;

	prof::print(RED, "{}", binfo->pitch);
	if (boat->speed > 8)
		g_audio->play_sound(197, { boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos }, 0.25f + (float(binfo->pitch) / 200.f));
	else if (drive)
		g_audio->play_sound(195, { boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos }, 0.25f + (float(binfo->pitch) / 200.f));

	if (lara.skidoo != item_number)
		return;

	if ((lara_item->current_anim_state == BOAT_JUMPR || lara_item->current_anim_state == BOAT_JUMPL) &&
		 lara_item->frame_number == anims[lara_item->anim_number].frame_end)
	{
		lara_item->pos.y_rot += (lara_item->current_anim_state == BOAT_JUMPL ? -0x4000 : 0x4000);
		lara_item->anim_number = 77;
		lara_item->frame_number = anims[lara_item->anim_number].frame_base;
		lara_item->current_anim_state = lara_item->goal_anim_state = AS_FORWARDJUMP;
		lara_item->gravity_status = 1;
		lara_item->fallspeed = -40;
		lara_item->speed = 20;
		lara_item->pos.x_rot = lara_item->pos.z_rot = 0;
		lara.skidoo = NO_ITEM;

		int x = lara_item->pos.x_pos + (360 * phd_sin(lara_item->pos.y_rot) >> W2V_SHIFT),
			y = lara_item->pos.y_pos - 90,
			z = lara_item->pos.z_pos + (360 * phd_cos(lara_item->pos.y_rot) >> W2V_SHIFT);

		auto room_number = lara_item->room_number;

		floor = GetFloor(x, y, z, &room_number);

		if (GetHeight(floor, x, y, z) >= y - STEP_L)
		{
			lara_item->pos.x_pos = x;
			lara_item->pos.z_pos = z;

			if (room_number != lara_item->room_number)
				ItemNewRoom(lara.item_number, room_number);
		}

		lara_item->pos.y_pos = y;

		boat->anim_number = objects[BOAT].anim_index + 0;
		boat->frame_number = anims[boat->anim_number].frame_base;

		if (auto entity = g_level->get_entity_by_item(boat))
			g_level->request_entity_ownership(entity, false);
	}

	room_number = boat->room_number;
	floor = GetFloor(boat->pos.x_pos, boat->pos.y_pos + 128, boat->pos.z_pos, &room_number);

	int h = GetWaterHeight(boat->pos.x_pos, boat->pos.y_pos + 128, boat->pos.z_pos, room_number);

	h = !(h > boat->pos.y_pos + 32 || h == NO_HEIGHT);

	bool nowake = (lara_item->current_anim_state == BOAT_JUMPR || lara_item->current_anim_state == BOAT_JUMPL);

	if (!(wibble & 15) && h && !nowake)
	{
		DoWake(boat, -256 - 128, 0, 0);
		DoWake(boat, 256 + 128, 0, 1);
	}

	if (boat->speed == 0 || !h || nowake)
	{
		if (WakeShadeBoat)
			--WakeShadeBoat;
	}
	else if (WakeShadeBoat < 16)
		++WakeShadeBoat;

	PHD_VECTOR prop { 0, 0, -80 };

	GetJointAbsPosition(boat, &prop, 2);

	room_number = boat->room_number;
	floor = GetFloor(prop.x, prop.y, prop.z, &room_number);
	h = GetWaterHeight(prop.x, prop.y, prop.z, room_number);

	if (boat->speed && h < prop.y && h != NO_HEIGHT)
	{
		TriggerBoatMist(prop.x, prop.y, prop.z, abs(boat->speed), boat->pos.y_rot + 0x8000, 0);

		if ((GetRandomControl() & 1) == 0)
		{
			PHD_3DPOS pos { prop.x + (GetRandomControl() & 63) - 32, prop.y + (GetRandomControl() & 15), prop.z + (GetRandomControl() & 63) - 32 };

			room_number = boat->room_number;

			GetFloor(pos.x_pos, pos.y_pos, pos.z_pos, &room_number);
			CreateBubble(&pos, room_number, 16, 8);
		}
	}
	else
	{
		if (prop.y > GetHeight(floor, prop.x, prop.y, prop.z) && !(room[room_number].flags & UNDERWATER))
		{
			GAME_VECTOR pos { prop.x, prop.y, prop.z };

			int cnt = (GetRandomControl() & 3) + 3;

			for (; cnt > 0; --cnt)
				TriggerBoatMist(prop.x, prop.y, prop.z, ((GetRandomControl() & 15) + 96) << 4, boat->pos.y_rot + 0x4000 + GetRandomControl(), 1);
		}
	}

	UpdateBoatWakeFX();
}

void DrawBoat(ITEM_INFO* item)
{
	auto b = (BOAT_INFO*)item->data;

	item->data = &b->prop_rot;

	DrawAnimatingItem(item);

	item->data = (void*)b;
}

void TriggerBoatMist(long x, long y, long z, long speed, int16_t angle, long snow)
{
	auto sptr = &spark[GetFreeSpark()];

	int zv = (speed * phd_cos(angle)) >> (W2V_SHIFT + 2),
		xv = (speed * phd_sin(angle)) >> (W2V_SHIFT + 2);

	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;

	if (snow)
	{
		sptr->dR = 255;
		sptr->dG = 255;
		sptr->dB = 255;
	}
	else
	{
		sptr->dR = 64;
		sptr->dG = 64;
		sptr->dB = 64;
	}

	sptr->ColFadeSpeed = 4 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 12 - (snow << 3);
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = xv + ((GetRandomControl() & 127) - 64);
	sptr->Yvel = (speed << 3) + (speed << 2);
	sptr->Zvel = zv + ((GetRandomControl() & 127) - 64);
	sptr->Friction = 3;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	sptr->Def = (PHDSPRITESTRUCT*)objects[EXPLOSION1].mesh_ptr;

	int size = 0;

	if (!snow)
	{
		size = (GetRandomControl() & 7) + (speed >> 1) + 16;

		sptr->Scalar = 4;
		sptr->Gravity = sptr->MaxYvel = 0;
		sptr->Width = sptr->sWidth = size >> 2;
		sptr->Height = sptr->sHeight = size >> 2;
	}
	else
	{
		size = (GetRandomControl() & 7) + 16;

		sptr->Friction = 0;
		sptr->Scalar = 3;
		sptr->Yvel = -sptr->Yvel >> 5;
		sptr->Gravity = (GetRandomControl() & 31) + 32;
		sptr->MaxYvel = 0;
		sptr->Width = sptr->sWidth = size;
		sptr->Height = sptr->sHeight = size;
	}

	sptr->dWidth = size;
	sptr->dHeight = size;
}

void BoatSplash(ITEM_INFO* item, long fallspeed, long water)
{
	splash_setup.x = item->pos.x_pos;
	splash_setup.y = water;
	splash_setup.z = item->pos.z_pos;
	splash_setup.InnerXZoff = 16 << 2;
	splash_setup.InnerXZsize = 12 << 2;
	splash_setup.InnerYsize = -96 << 2;
	splash_setup.InnerXZvel = 0xa0;
	splash_setup.InnerYvel = -fallspeed << 7;
	splash_setup.InnerGravity = 0x80;
	splash_setup.InnerFriction = 7;
	splash_setup.MiddleXZoff = 24 << 2;
	splash_setup.MiddleXZsize = 24 << 2;
	splash_setup.MiddleYsize = -64 << 2;
	splash_setup.MiddleXZvel = 0xe0;
	splash_setup.MiddleYvel = -fallspeed << 6;
	splash_setup.MiddleGravity = 0x48;
	splash_setup.MiddleFriction = 8;
	splash_setup.OuterXZoff = 32 << 2;
	splash_setup.OuterXZsize = 32 << 2;
	splash_setup.OuterXZvel = 0x110;
	splash_setup.OuterFriction = 9;

	SetupSplash(&splash_setup);

	splash_count = 16;
}