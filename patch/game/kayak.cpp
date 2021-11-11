#include <specific/standard.h>
#include <specific/stypes.h>
#include <specific/input.h>
#include <specific/global.h>

#include <specific/fn_stubs.h>
#include <specific/init.h>

#include "objects.h"
#include "laraanim.h"
#include "laraflar.h"
#include "control.h"
#include "camera.h"
#include "effect2.h"
#include "kayak.h"
#include "physics.h"
#include "gameflow.h"
#include "game.h"

#include <mp/game/level.h>

#define KAYAK_COLLIDE			64
#define GETOFF_DIST 			768
#define KAYAK_TO_BADDIE_RADIUS	256
#define MAX_SPEED				0x380000
#define KAYAK_FRICTION			0x8000
#define KAYAK_ROT_FRIC			0x50000
#define KAYAK_DFLECT_ROT		0x80000
#define KAYAK_FWD_VEL			0x180000
#define KAYAK_FWD_ROT			0x800000
#define KAYAK_LR_VEL			0x100000
#define KAYAK_LR_ROT			0xc00000
#define KAYAK_MAX_LR			0xc00000
#define KAYAK_TURN_ROT			0x200000
#define KAYAK_MAX_TURN			0x1000000
#define KAYAK_TURN_BRAKE		0x8000
#define KAYAK_HARD_ROT			0x1000000
#define KAYAK_MAX_STAT			0x1000000
#define BOAT_SLIP				50
#define BOAT_SIDE_SLIP			50
#define HIT_BACK				1
#define HIT_FRONT				2
#define HIT_LEFT				3
#define HIT_RIGHT				4
#define KAYAK_BACK_A			2
#define KAYAK_CLIMBIN_A			3
#define KAYAK_CLIMBIN_F			GF(KAYAK_CLIMBIN_A, 0)
#define KAYAK_CLIMBIN2_A		4
#define KAYAK_DEATHIN_A			5
#define KAYAK_FORWARD_A			8
#define KAYAK_2FORWARD_A		9
#define KAYAK_JUMPOUT1_A		14
#define KAYAK_POSE_A			16
#define KAYAK_POSE_F			GF(KAYAK_POSE_A, 0)
#define KAYAK_JUMPOUT2_A		24
#define KAYAK_DROWN_A			25
#define KAYAK_TURNL_A			26
#define KAYAK_TURNR_A			27
#define KAYAK_CLIMBINR_A		28
#define KAYAK_CLIMBINR_F		GF(KAYAK_CLIMBINR_A, 0)
#define KAYAK_JUMPOUTR_A		32
#define KAYAK_DRAW_SHIFT		32
#define LARA_LEG_BITS			((1 << HIPS) | (1 << THIGH_L) | (1 << CALF_L) | (1 << FOOT_L) | (1 << THIGH_R) | (1 << CALF_R) | (1 << FOOT_R))
#define NUM_WAKE_SPRITES		32
#define WAKE_SIZE 				32
#define WAKE_SPEED 				4
#define KAYAK_X					128
#define KAYAK_Z					128
#define SKIDOO_MAX_KICK			-80
#define SKIDOO_MIN_BOUNCE		((MAX_SPEED / 2) >> 8)
#define TARGET_DIST				(WALL_L * 2)

enum
{
	KS_BACK,
	KS_POSE,
	KS_LEFT,
	KS_RIGHT,
	KS_CLIMBIN,
	KS_DEATHIN,
	KS_FORWARD,
	KS_ROLL,
	KS_DROWNIN,
	KS_JUMPOUT,
	KS_TURNL,
	KS_TURNR,
	KS_CLIMBINR,
	KS_CLIMBOUTL,
	KS_CLIMBOUTR,
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

WAKE_PTS WakePtsKayak[NUM_WAKE_SPRITES][2];

uint8_t CurrentStartWake = 0,
	  WakeShade = 0;

void KayakSplash(ITEM_INFO* item, long fallspeed, long water);

void DoWakeKayak(ITEM_INFO* v, int16_t xoff, int16_t zoff, int16_t rotate)
{
	if (WakePtsKayak[CurrentStartWake][rotate].life)
		return;

	int c = phd_cos(v->pos.y_rot),
		s = phd_sin(v->pos.y_rot),
		x = v->pos.x_pos + (((zoff * s) + (xoff * c)) >> W2V_SHIFT),
		z = v->pos.z_pos + (((zoff * c) - (xoff * s)) >> W2V_SHIFT);

	auto room_number = v->room_number;
	auto floor = GetFloor(x, v->pos.y_pos, z, &room_number);

	if (GetWaterHeight(x, v->pos.y_pos, z, room_number) != NO_HEIGHT)
	{
		int16_t angle1, angle2;

		if (v->speed < 0)
		{
			if (!rotate)
			{
				angle1 = v->pos.y_rot - (ONE_DEGREE * 10);
				angle2 = v->pos.y_rot - (ONE_DEGREE * 30);
			}
			else
			{
				angle1 = v->pos.y_rot + (ONE_DEGREE * 10);
				angle2 = v->pos.y_rot + (ONE_DEGREE * 30);
			}
		}
		else
		{
			if (!rotate)
			{
				angle1 = v->pos.y_rot - (ONE_DEGREE * 170);
				angle2 = v->pos.y_rot - (ONE_DEGREE * 150);
			}
			else
			{
				angle1 = v->pos.y_rot + (ONE_DEGREE * 170);
				angle2 = v->pos.y_rot + (ONE_DEGREE * 150);
			}
		}

		int32_t xv[2] { (WAKE_SPEED * phd_sin(angle1)) >> W2V_SHIFT, ((WAKE_SPEED + 2) * phd_sin(angle2)) >> W2V_SHIFT },
			   zv[2] { (WAKE_SPEED * phd_cos(angle1)) >> W2V_SHIFT, ((WAKE_SPEED + 2) * phd_cos(angle2)) >> W2V_SHIFT };

		WakePtsKayak[CurrentStartWake][rotate].y = v->pos.y_pos + KAYAK_DRAW_SHIFT;
		WakePtsKayak[CurrentStartWake][rotate].life = 0x40;

		for (int i = 0; i < 2; ++i)
		{
			WakePtsKayak[CurrentStartWake][rotate].x[i] = x;
			WakePtsKayak[CurrentStartWake][rotate].z[i] = z;
			WakePtsKayak[CurrentStartWake][rotate].xvel[i] = xv[i];
			WakePtsKayak[CurrentStartWake][rotate].zvel[i] = zv[i];
		}

		if (rotate == 1)
		{
			++CurrentStartWake;
			CurrentStartWake &= (NUM_WAKE_SPRITES - 1);
		}
	}
}

void DoRipple(ITEM_INFO* v, int16_t xoff, int16_t zoff)
{
	int c = phd_cos(v->pos.y_rot),
		s = phd_sin(v->pos.y_rot),
		x = v->pos.x_pos + (((zoff * s) + (xoff * c)) >> W2V_SHIFT),
		z = v->pos.z_pos + (((zoff * c) - (xoff * s)) >> W2V_SHIFT);

	auto room_number = v->room_number;
	auto floor = GetFloor(x, v->pos.y_pos, z, &room_number);

	if (GetWaterHeight(x, v->pos.y_pos, z, room_number) != NO_HEIGHT)
		SetupRipple(x, v->pos.y_pos, z, -2 - (GetRandomControl() & 1), 0)->init = 0;
}

void UpdateWakeFXKayak()
{
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < NUM_WAKE_SPRITES; ++j)
		{
			if (WakePtsKayak[j][i].life)
			{
				--WakePtsKayak[j][i].life;
				WakePtsKayak[j][i].x[0] += WakePtsKayak[j][i].xvel[0];
				WakePtsKayak[j][i].z[0] += WakePtsKayak[j][i].zvel[0];
				WakePtsKayak[j][i].x[1] += WakePtsKayak[j][i].xvel[1];
				WakePtsKayak[j][i].z[1] += WakePtsKayak[j][i].zvel[1];
			}
		}
	}
}

void KayakSplash(ITEM_INFO* item, long fallspeed, long water)
{
	splash_setup.x = item->pos.x_pos;
	splash_setup.y = water;
	splash_setup.z = item->pos.z_pos;
	splash_setup.InnerXZoff = 16 << 3;
	splash_setup.InnerXZsize = 12 << 2;
	splash_setup.InnerYsize = -96 << 2;
	splash_setup.InnerXZvel = 0xa0;
	splash_setup.InnerYvel = -fallspeed << 5;
	splash_setup.InnerGravity = 0x80;
	splash_setup.InnerFriction = 7;
	splash_setup.MiddleXZoff = 24 << 3;
	splash_setup.MiddleXZsize = 24 << 2;
	splash_setup.MiddleYsize = -64 << 2;
	splash_setup.MiddleXZvel = 0xe0;
	splash_setup.MiddleYvel = -fallspeed << 4;
	splash_setup.MiddleGravity = 0x48;
	splash_setup.MiddleFriction = 8;
	splash_setup.OuterXZoff = 32 << 3;
	splash_setup.OuterXZsize = 32 << 2;
	splash_setup.OuterXZvel = 0x110;
	splash_setup.OuterFriction = -9;

	SetupSplash(&splash_setup);

	splash_count = 16;
}

void TriggerRapidsMist(long x, long y, long z)
{
	auto sptr = &spark[GetFreeSpark()];

	int xsize = (GetRandomControl() & 7) + 16;

	sptr->On = 1;
	sptr->sR = 128;
	sptr->sG = 128;
	sptr->sB = 128;
	sptr->dR = 192;
	sptr->dG = 192;
	sptr->dB = 192;
	sptr->ColFadeSpeed = 2;
	sptr->FadeToBlack = 4;
	sptr->sLife = sptr->Life = 6 + (GetRandomControl() & 3);
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 3;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	sptr->Def = (PHDSPRITESTRUCT*)objects[EXPLOSION1].mesh_ptr;
	sptr->Scalar = 4;
	sptr->Gravity = 0;
	sptr->MaxYvel = 0;
	sptr->Width = sptr->sWidth = xsize >> 1;
	sptr->dWidth = xsize;
	sptr->Height = sptr->sHeight = xsize >> 1;
	sptr->dHeight = xsize;
}

int GetInKayak(int16_t item_number, COLL_INFO* coll)
{
	auto l = lara_item;

	if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
		return 0;

	auto v = &items[item_number];

	int x = l->pos.x_pos - v->pos.x_pos,
		z = l->pos.z_pos - v->pos.z_pos,
		dist = (x * x) + (z * z);

	if (dist > (130000))
		return 0;

	auto room_number = v->room_number;
	auto floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	if (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) > -32000)
	{
		int16_t ang = phd_atan(v->pos.z_pos - l->pos.z_pos, v->pos.x_pos - l->pos.x_pos) - v->pos.y_rot;

		uint16_t tempang = l->pos.y_rot - v->pos.y_rot;

		if ((ang > -(ONE_DEGREE * 45)) && (ang < (ONE_DEGREE * 135)))
		{
			if (tempang > (45 * ONE_DEGREE) && tempang < (135 * ONE_DEGREE))
				return -1;
		}
		else if (tempang > (225 * ONE_DEGREE) && tempang < (315 * ONE_DEGREE))
			return 1;
	}

	return 0;
}

int32_t GetCollisionAnim(ITEM_INFO* v, int xdiff, int zdiff)
{
	if ((xdiff = v->pos.x_pos - xdiff) || (zdiff = v->pos.z_pos - zdiff))
	{
		int c = phd_cos(v->pos.y_rot),
			s = phd_sin(v->pos.y_rot),
			front = ((zdiff * c) + (xdiff * s)) >> W2V_SHIFT,
			side = ((-zdiff * s) + (xdiff * c)) >> W2V_SHIFT;

		if (ABS(front) > ABS(side))
			return (front > 0 ? HIT_BACK : HIT_FRONT);
		else return (side > 0 ? HIT_LEFT : HIT_RIGHT);
	}

	return 0;
}

int DoDynamics(int32_t height, int fallspeed, int32_t* y)
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
		int kick = (height - *y) << 2;

		if (kick < SKIDOO_MAX_KICK)
			kick = SKIDOO_MAX_KICK;

		fallspeed += ((kick - fallspeed) >> 3);

		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

void DoCurrentKayak(ITEM_INFO* item)
{
	auto r = &room[item->room_number];

	if (!lara.current_active)
	{
		int absvel = abs(lara.current_xvel),
			shifter = 2;

		if (absvel > 16)	 shifter = 4;
		else if (absvel > 8) shifter = 3;

		lara.current_xvel -= lara.current_xvel >> shifter;

		if (abs(lara.current_xvel) < 4)
			lara.current_xvel = 0;

		if ((absvel = abs(lara.current_zvel)) > 16)
			shifter = 4;
		else if (absvel > 8)
			shifter = 3;
		else shifter = 2;

		lara.current_zvel -= lara.current_zvel >> shifter;

		if (abs(lara.current_zvel) < 4)
			lara.current_zvel = 0;

		if (lara.current_xvel == 0 && lara.current_zvel == 0)
			return;
	}
	else
	{
		int sinkval = lara.current_active - 1;

		PHD_VECTOR target { camera.fixed[sinkval].x, camera.fixed[sinkval].y, camera.fixed[sinkval].z };

		int angle = ((m_atan2(target.x, target.z, lara_item->pos.x_pos, lara_item->pos.z_pos) - 0x4000) >> 4) & 4095,
			dx = target.x - lara_item->pos.x_pos,
			dz = target.z - lara_item->pos.z_pos,
			speed = camera.fixed[sinkval].data;

		dx = (m_sin(angle << 1) * speed) >> 2;
		dz = (m_cos(angle << 1) * speed) >> 2;

		lara.current_xvel += (dx - lara.current_xvel) >> 4;
		lara.current_zvel += (dz - lara.current_zvel) >> 4;
	}

	item->pos.x_pos += lara.current_xvel >> 8;
	item->pos.z_pos += lara.current_zvel >> 8;

	lara.current_active = 0;
}

int TestHeight(ITEM_INFO* item, int x, int y, PHD_VECTOR* in_pos)
{
	ITEM_INFO* k; // edi@1
	int v5; // ebx@1
	int v6; // eax@1
	int v7; // ebp@1
	int v8; // ST20_4@1
	PHD_VECTOR* pos; // esi@1
	int v10; // eax@1
	unsigned __int16 v11; // cx@1
	int v12; // edx@1
	signed int v14; // ebx@1
	int pos_z; // ebx@1
	signed int pos_y; // edx@1
	int result; // eax@1
	int v19; // ecx@2
	FLOOR_INFO* v20; // eax@2

	k = item;
	v5 = phd_cos(item->pos.y_rot);      // ry
	v6 = phd_sin(k->pos.y_rot);
	v7 = x;
	v8 = v6;
	pos = in_pos;
	in_pos->x = k->pos.x_pos + ((x * v5 + y * v6) >> 14);
	v10 = phd_sin(k->pos.z_rot);
	v11 = k->pos.x_rot;
	in_pos = (PHD_VECTOR*)(k->pos.y_pos + (v7 * v10 >> 14));
	v12 = phd_sin(v11);
	auto room_number = (int16_t)(x * v8);
	v14 = y * v5 - x * v8;
	pos->y = (int32_t)&in_pos->x - (y * v12 >> 14);
	pos_z = k->pos.z_pos + (v14 >> 14);
	pos_y = pos->y;
	pos->z = pos_z;
	auto room = k->room_number;

	GetFloor(pos->x, pos_y, pos_z, &room);

	result = GetWaterHeight(pos->x, pos->y, pos->z, room);

	if (result != -32512 || (v19 = pos->z, room = k->room_number, v20 = GetFloor(pos->x, pos->y, v19, &room), result = GetHeight(v20, pos->x, pos->y, pos->z), result != -32512))
		result -= 5;

	return result;
}

int CanGetOut(ITEM_INFO* v, int direction)
{
	PHD_VECTOR pos;
	return (v->pos.y_pos - TestHeight(v, (direction < 0) ? -GETOFF_DIST : GETOFF_DIST, 0, &pos) <= 0);
}

int DoShiftKayak(ITEM_INFO* v, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x = pos->x >> WALL_SHIFT,
		z = pos->z >> WALL_SHIFT,
		x_old = old->x >> WALL_SHIFT,
		z_old = old->z >> WALL_SHIFT,
		shift_x = pos->x & (WALL_L - 1),
		shift_z = pos->z & (WALL_L - 1);

	if (x == x_old)
	{
		old->x = 0;

		if (z == z_old)
		{
			v->pos.z_pos += (old->z - pos->z);
			v->pos.x_pos += (old->x - pos->x);
		}

		else if (z > z_old)
		{
			v->pos.z_pos -= shift_z + 1;
			return (pos->x - v->pos.x_pos);
		}

		else
		{
			v->pos.z_pos += WALL_L - shift_z;
			return (v->pos.x_pos - pos->x);
		}
	}

	else if (z == z_old)
	{
		old->z = 0;

		if (x > x_old)
		{
			v->pos.x_pos -= shift_x + 1;
			return (v->pos.z_pos - pos->z);
		}

		else
		{
			v->pos.x_pos += WALL_L - shift_x;
			return (pos->z - v->pos.z_pos);
		}
	}

	else
	{
		x = z = 0;

		auto room_number = v->room_number;
		auto floor = GetFloor(old->x, pos->y, pos->z, &room_number);

		if (GetHeight(floor, old->x, pos->y, pos->z) < old->y - STEP_L)
		{
			if (pos->z > old->z)
				z = -shift_z - 1;
			else
				z = WALL_L - shift_z;
		}

		room_number = v->room_number;
		floor = GetFloor(pos->x, pos->y, old->z, &room_number);

		if (GetHeight(floor, pos->x, pos->y, old->z) < old->y - STEP_L)
			x = (pos->x > old->x ? -shift_x - 1 : WALL_L - shift_x);

		if (x && z)
		{
			v->pos.z_pos += z;
			v->pos.x_pos += x;
		}
		else if (z)
		{
			v->pos.z_pos += z;
			return (z > 0 ? v->pos.x_pos - pos->x : pos->x - v->pos.x_pos);
		}
		else if (x)
		{
			v->pos.x_pos += x;
			return (x > 0 ? pos->z - v->pos.z_pos : v->pos.z_pos - pos->z);
		}
		else
		{
			v->pos.z_pos += (old->z - pos->z);
			v->pos.x_pos += (old->x - pos->x);
		}
	}

	return 0;
}

void KayakToBackground(ITEM_INFO* v, KAYAKINFO* Kayak)
{
	int32_t h, slip = 0, rot = 0;
	PHD_VECTOR pos;
	int32_t height[8];
	PHD_VECTOR oldpos[9];

	int32_t x, z;
	int32_t fh, lh, rh;
	PHD_VECTOR fpos, lpos, rpos;

	Kayak->OldPos = v->pos;

	height[0] = TestHeight(v, 0, 1024, &oldpos[0]);
	height[1] = TestHeight(v, -96, 512, &oldpos[1]);
	height[2] = TestHeight(v, 96, 512, &oldpos[2]);
	height[3] = TestHeight(v, -128, 128, &oldpos[3]);
	height[4] = TestHeight(v, 128, 128, &oldpos[4]);
	height[5] = TestHeight(v, -128, -320, &oldpos[5]);
	height[6] = TestHeight(v, 128, -320, &oldpos[6]);
	height[7] = TestHeight(v, 0, -640, &oldpos[7]);

	for (int i = 0; i < 8; ++i)
		if (oldpos[i].y > height[i])
			oldpos[i].y = height[i];

	oldpos[8].x = v->pos.x_pos;
	oldpos[8].y = v->pos.y_pos;
	oldpos[8].z = v->pos.z_pos;

	fh = TestHeight(v, 0, 1024, &fpos);
	lh = TestHeight(v, -KAYAK_X, KAYAK_Z, &lpos);
	rh = TestHeight(v, KAYAK_X, KAYAK_Z, &rpos);

	v->pos.y_rot += (Kayak->Rot >> 16);

	v->pos.x_pos += (v->speed * phd_sin(v->pos.y_rot)) >> W2V_SHIFT;
	v->pos.z_pos += (v->speed * phd_cos(v->pos.y_rot)) >> W2V_SHIFT;

	DoCurrentKayak(v);

	Kayak->FallSpeedL = DoDynamics(lh, Kayak->FallSpeedL, &lpos.y);
	Kayak->FallSpeedR = DoDynamics(rh, Kayak->FallSpeedR, &rpos.y);
	Kayak->FallSpeedF = DoDynamics(fh, Kayak->FallSpeedF, &fpos.y);

	v->fallspeed = DoDynamics(Kayak->Water, v->fallspeed, &v->pos.y_pos);

	h = (lpos.y + rpos.y) >> 1;
	x = phd_atan(1024, v->pos.y_pos - fpos.y);
	z = phd_atan(KAYAK_X, h - lpos.y);

	v->pos.x_rot = x;
	v->pos.z_rot = z;

	int oldx = v->pos.x_pos,
		oldz = v->pos.z_pos;

	if ((h = TestHeight(v, 0, -640, &pos)) < (oldpos[7].y - KAYAK_COLLIDE))		rot = DoShiftKayak(v, &pos, &oldpos[7]);
	if ((h = TestHeight(v, 128, -320, &pos)) < (oldpos[6].y - KAYAK_COLLIDE))	rot += DoShiftKayak(v, &pos, &oldpos[6]);
	if ((h = TestHeight(v, -128, -320, &pos)) < (oldpos[5].y - KAYAK_COLLIDE))	rot += DoShiftKayak(v, &pos, &oldpos[5]);
	if ((h = TestHeight(v, 128, 128, &pos)) < (oldpos[4].y - KAYAK_COLLIDE))	rot += DoShiftKayak(v, &pos, &oldpos[4]);
	if ((h = TestHeight(v, -128, 128, &pos)) < (oldpos[3].y - KAYAK_COLLIDE))	rot += DoShiftKayak(v, &pos, &oldpos[3]);
	if ((h = TestHeight(v, 96, 512, &pos)) < (oldpos[2].y - KAYAK_COLLIDE))		rot += DoShiftKayak(v, &pos, &oldpos[2]);
	if ((h = TestHeight(v, -96, 512, &pos)) < (oldpos[1].y - KAYAK_COLLIDE))	rot += DoShiftKayak(v, &pos, &oldpos[1]);
	if ((h = TestHeight(v, 0, 1024, &pos)) < (oldpos[0].y - KAYAK_COLLIDE))		rot += DoShiftKayak(v, &pos, &oldpos[0]);

	v->pos.y_rot += rot;

	auto room_number = v->room_number;
	auto floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	h = GetWaterHeight(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, room_number);

	if (h == NO_HEIGHT)
		h = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);

	if (h < (v->pos.y_pos - KAYAK_COLLIDE))
		DoShiftKayak(v, (PHD_VECTOR*)&v->pos, &oldpos[8]);

	room_number = v->room_number;
	floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
	h = GetWaterHeight(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, room_number);

	if (h == NO_HEIGHT)
		h = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);

	if (h == NO_HEIGHT)
	{
		GAME_VECTOR kpos { Kayak->OldPos.x_pos, Kayak->OldPos.y_pos, Kayak->OldPos.z_pos, v->room_number };

		CameraCollisionBounds(&kpos, 256, 0);

		v->pos.x_pos = kpos.x;
		v->pos.y_pos = kpos.y;
		v->pos.z_pos = kpos.z;
		v->room_number = kpos.room_number;
	}

	if (slip || GetCollisionAnim(v, oldx, oldz))
	{
		int newspeed = (((v->pos.z_pos - oldpos[8].z) * phd_cos(v->pos.y_rot) + (v->pos.x_pos - oldpos[8].x) * phd_sin(v->pos.y_rot)) >> W2V_SHIFT) << 8;

		if (slip)
		{
			if (Kayak->Vel <= MAX_SPEED)
				Kayak->Vel = newspeed;
		}
		else
		{
			if (Kayak->Vel > 0 && newspeed < Kayak->Vel)	  Kayak->Vel = newspeed;
			else if (Kayak->Vel < 0 && newspeed > Kayak->Vel) Kayak->Vel = newspeed;
		}

		if (Kayak->Vel < -MAX_SPEED)
			Kayak->Vel = -MAX_SPEED;
	}
}

void KayakUserInput(ITEM_INFO* v, ITEM_INFO* l, KAYAKINFO* Kayak)
{
	if (l->hit_points <= 0 && l->current_anim_state != KS_DEATHIN)
	{
		l->anim_number = objects[kayak_anim_obj].anim_index + KAYAK_DEATHIN_A;
		l->frame_number = anims[l->anim_number].frame_base;
		l->current_anim_state = l->goal_anim_state = KS_DEATHIN;
	}

	int16_t frame = l->frame_number - anims[l->anim_number].frame_base;

	static char lr;

	bool getting_off = false;

	switch (l->current_anim_state)
	{
	case KS_POSE:
	{
		if ((input & IN_ROLL) && !lara.current_active && !lara.current_xvel && !lara.current_zvel)
		{
			if ((input & IN_LEFT) && CanGetOut(v, -1))
			{
				l->goal_anim_state = KS_JUMPOUT;
				l->required_anim_state = KS_CLIMBOUTL;
			}

			else if ((input & IN_RIGHT) && CanGetOut(v, 1))
			{
				l->goal_anim_state = KS_JUMPOUT;
				l->required_anim_state = KS_CLIMBOUTR;
			}
		}
		else if (input & IN_FORWARD)
		{
			l->goal_anim_state = KS_RIGHT;
			Kayak->Turn = 0;
			Kayak->Forward = 1;
		}
		else if (input & IN_BACK)
			l->goal_anim_state = KS_BACK;
		else if (input & IN_LEFT)
		{
			l->goal_anim_state = KS_LEFT;

			Kayak->Turn = !Kayak->Vel;
			Kayak->Forward = 0;
		}

		else if (input & IN_RIGHT)
		{
			l->goal_anim_state = KS_RIGHT;

			Kayak->Turn = !Kayak->Vel;
			Kayak->Forward = 0;
		}
		else if ((input & IN_STEPL) && (Kayak->Vel || lara.current_xvel || lara.current_zvel))
			l->goal_anim_state = KS_TURNL;
		else if ((input & IN_STEPR) && (Kayak->Vel || lara.current_xvel || lara.current_zvel))
			l->goal_anim_state = KS_TURNR;

		break;
	}
	case KS_LEFT:
	{
		if (Kayak->Forward)
		{
			if (!frame)
				lr = 0;

			if (frame == 2 && !(lr & 0x80))
				++lr;
			else if (frame > 2)
				lr &= ~0x80;

			if (input & IN_FORWARD)
			{
				if (input & IN_LEFT)
				{
					if ((lr & ~0x80) >= 2)
						l->goal_anim_state = KS_RIGHT;
				}
				else l->goal_anim_state = KS_RIGHT;
			}
			else l->goal_anim_state = KS_POSE;
		}
		else if (!(input & IN_LEFT))
			l->goal_anim_state = KS_POSE;

		if (frame == 7)
		{
			if (Kayak->Forward)
			{
				if ((Kayak->Rot -= KAYAK_FWD_ROT) < -KAYAK_MAX_TURN)
					Kayak->Rot = -KAYAK_MAX_TURN;

				Kayak->Vel += KAYAK_FWD_VEL;
			}

			else if (Kayak->Turn)
			{
				if ((Kayak->Rot -= KAYAK_HARD_ROT) < -KAYAK_MAX_STAT)
					Kayak->Rot = -KAYAK_MAX_STAT;
			}
			else
			{
				if ((Kayak->Rot -= KAYAK_LR_ROT) < -KAYAK_MAX_LR)
					Kayak->Rot = -KAYAK_MAX_LR;

				Kayak->Vel += KAYAK_LR_VEL;
			}
		}

		if (frame > 6 && frame < 24 && (frame & 1))
			DoRipple(v, -384, -64);

		break;
	}
	case KS_RIGHT:
	{
		if (Kayak->Forward)
		{
			if (!frame)
				lr = 0;

			if (frame == 2 && !(lr & 0x80))
				++lr;
			else if (frame > 2)
				lr &= ~0x80;

			if (input & IN_FORWARD)
			{
				if (input & IN_RIGHT)
				{
					if ((lr & ~0x80) >= 2)
						l->goal_anim_state = KS_LEFT;
				}
				else l->goal_anim_state = KS_LEFT;
			}
			else l->goal_anim_state = KS_POSE;
		}
		else if (!(input & IN_RIGHT))
			l->goal_anim_state = KS_POSE;

		if (frame == 7)
		{
			if (Kayak->Forward)
			{
				if ((Kayak->Rot += KAYAK_FWD_ROT) > KAYAK_MAX_TURN)
					Kayak->Rot = KAYAK_MAX_TURN;

				Kayak->Vel += KAYAK_FWD_VEL;
			}
			else if (Kayak->Turn)
			{
				if ((Kayak->Rot += KAYAK_HARD_ROT) > KAYAK_MAX_STAT)
					Kayak->Rot = KAYAK_MAX_STAT;
			}
			else
			{
				if ((Kayak->Rot += KAYAK_LR_ROT) > KAYAK_MAX_LR)
					Kayak->Rot = KAYAK_MAX_LR;

				Kayak->Vel += KAYAK_LR_VEL;
			}
		}

		if ((frame > 6) && (frame < 24) && (frame & 1))
			DoRipple(v, 384, -64);

		break;
	}
	case KS_BACK:
	{
		if (!(input & IN_BACK))
			l->goal_anim_state = KS_POSE;

		if (l->anim_number - objects[kayak_anim_obj].anim_index == KAYAK_BACK_A)
		{
			if (frame == 8)
			{
				Kayak->Rot += KAYAK_FWD_ROT;
				Kayak->Vel -= KAYAK_FWD_VEL;
			}

			if (frame == 31)
			{
				Kayak->Rot -= KAYAK_FWD_ROT;
				Kayak->Vel -= KAYAK_FWD_VEL;
			}

			if (frame < 15 && (frame & 1))						DoRipple(v, 384, -128);
			else if (frame >= 20 && frame <= 34 && (frame & 1)) DoRipple(v, -384, -128);
		}

		break;
	}
	case KS_TURNL:
	{
		if (!(input & IN_STEPL) || (!Kayak->Vel && !lara.current_xvel && !lara.current_zvel))
			l->goal_anim_state = KS_POSE;
		else if ((l->anim_number - objects[kayak_anim_obj].anim_index) == KAYAK_TURNL_A)
		{
			if (Kayak->Vel >= 0)
			{
				if ((Kayak->Rot -= KAYAK_TURN_ROT) < -KAYAK_MAX_TURN)
					Kayak->Rot = -KAYAK_MAX_TURN;

				if ((Kayak->Vel += -KAYAK_TURN_BRAKE) < 0)
					Kayak->Vel = 0;
			}

			if (Kayak->Vel < 0)
			{
				Kayak->Rot += KAYAK_TURN_ROT;

				if ((Kayak->Vel += KAYAK_TURN_BRAKE) > 0)
					Kayak->Vel = 0;
			}

			if (!(wibble & 3))
				DoRipple(v, -256, -256);
		}

		break;
	}
	case KS_TURNR:
	{
		if (!(input & IN_STEPR) || (!Kayak->Vel && !lara.current_xvel && !lara.current_zvel))
			l->goal_anim_state = KS_POSE;

		else if ((l->anim_number - objects[kayak_anim_obj].anim_index) == KAYAK_TURNR_A)
		{
			if (Kayak->Vel >= 0)
			{
				if ((Kayak->Rot += KAYAK_TURN_ROT) > KAYAK_MAX_TURN)
					Kayak->Rot = KAYAK_MAX_TURN;

				if ((Kayak->Vel += -KAYAK_TURN_BRAKE) < 0)
					Kayak->Vel = 0;
			}

			if (Kayak->Vel < 0)
			{
				Kayak->Rot -= KAYAK_TURN_ROT;

				if ((Kayak->Vel += KAYAK_TURN_BRAKE) > 0)
					Kayak->Vel = 0;
			}

			if (!(wibble & 3))
				DoRipple(v, 256, -256);
		}

		break;
	}
	case KS_CLIMBIN:
	{
		if (l->anim_number == objects[kayak_anim_obj].anim_index + KAYAK_CLIMBIN2_A && frame == 24 && !(Kayak->Flags & 0x80))
		{
			auto tmp = lara.mesh_ptrs[HAND_R];

			lara.mesh_ptrs[HAND_R] = objects[kayak_anim_obj].mesh_ptr[HAND_R];
			objects[kayak_anim_obj].mesh_ptr[HAND_R] = tmp;

			l->mesh_bits &= ~LARA_LEG_BITS;

			Kayak->Flags |= 0x80;
		}

		break;
	}
	case KS_JUMPOUT:
	{
		if (l->anim_number == objects[kayak_anim_obj].anim_index + KAYAK_JUMPOUT1_A && frame == 27 && Kayak->Flags & 0x80)
		{
			auto tmp = lara.mesh_ptrs[HAND_R];

			lara.mesh_ptrs[HAND_R] = objects[kayak_anim_obj].mesh_ptr[HAND_R];
			objects[kayak_anim_obj].mesh_ptr[HAND_R] = tmp;

			l->mesh_bits |= LARA_LEG_BITS;

			Kayak->Flags &= ~0x80;
		}

		l->goal_anim_state = l->required_anim_state;

		break;
	}
	case KS_CLIMBOUTL:
	{
		if (l->anim_number == objects[kayak_anim_obj].anim_index + KAYAK_JUMPOUT2_A && frame == 83)
		{
			PHD_VECTOR vec { 0, 350, 500 };

			get_lara_bone_pos(lara_item, &vec, HIPS);

			l->pos.x_pos = vec.x;
			l->pos.y_pos = vec.y;
			l->pos.z_pos = vec.z;
			l->pos.x_rot = 0;
			l->pos.y_rot = v->pos.y_rot - 0x4000;
			l->pos.z_rot = 0;
			l->anim_number = FASTFALL_A;
			l->frame_number = FASTFALL_F;
			l->current_anim_state = l->goal_anim_state = AS_FASTFALL;
			l->fallspeed = 0;
			l->gravity_status = 1;

			lara.gun_status = LG_ARMLESS;
			lara.skidoo = NO_ITEM;
			
			getting_off = true;
		}

		break;
	}
	case KS_CLIMBOUTR:
	{
		if (l->anim_number == objects[kayak_anim_obj].anim_index + KAYAK_JUMPOUTR_A && frame == 83)
		{
			PHD_VECTOR vec { 0, 350, 500 };

			get_lara_bone_pos(lara_item, &vec, HIPS);

			l->pos.x_pos = vec.x;
			l->pos.y_pos = vec.y;
			l->pos.z_pos = vec.z;
			l->pos.x_rot = 0;
			l->pos.y_rot = v->pos.y_rot + 0x4000;
			l->pos.z_rot = 0;
			l->anim_number = FASTFALL_A;
			l->frame_number = FASTFALL_F;
			l->current_anim_state = l->goal_anim_state = AS_FASTFALL;
			l->fallspeed = 0;
			l->gravity_status = 1;

			lara.gun_status = LG_ARMLESS;
			lara.skidoo = NO_ITEM;

			getting_off = true;
		}
	}
	}

	if (Kayak->Vel > 0)
	{
		if ((Kayak->Vel -= KAYAK_FRICTION) < 0)
			Kayak->Vel = 0;
	}
	else if (Kayak->Vel < 0 && (Kayak->Vel += KAYAK_FRICTION) > 0)
		Kayak->Vel = 0;

	if (Kayak->Vel > MAX_SPEED)		  Kayak->Vel = MAX_SPEED;
	else if (Kayak->Vel < -MAX_SPEED) Kayak->Vel = -MAX_SPEED;

	v->speed = (Kayak->Vel >> 16);

	if (Kayak->Rot >= 0)
	{
		if ((Kayak->Rot -= KAYAK_ROT_FRIC) < 0)
			Kayak->Rot = 0;
	}
	else if (Kayak->Rot < 0 && (Kayak->Rot += KAYAK_ROT_FRIC) > 0)
		Kayak->Rot = 0;

	if (getting_off)
		if (auto entity = g_level->get_entity_by_item(v))
			g_level->request_entity_ownership(entity, false);
}

void KayakToBaddieCollision(ITEM_INFO* v)
{
	int16_t roomies[20],
		    numroom = 1;

	roomies[0] = v->room_number;

	if (auto door = room[v->room_number].door)
	{
		for (int i = (int)*(door++); i > 0; --i)
		{
			roomies[numroom++] = *(door);
			door += 16;
		}
	}

	for (int i = 0; i < numroom; ++i)
	{
		auto item_num = room[roomies[i]].item_number;

		while (item_num != NO_ITEM)
		{
			auto item = &items[item_num];

			if (item->collidable && item->status != INVISIBLE)
			{
				auto object = &objects[item->object_number];

				if (object->collision &&
					(item->object_number == SPIKES ||
					 item->object_number == DARTS ||
					 item->object_number == TEETH_TRAP ||
					 (item->object_number == BLADE &&
					  item->current_anim_state != 1) ||
					 (item->object_number == ICICLES &&
					  item->current_anim_state != 3)))
				{
					int x, y, z;

					x = v->pos.x_pos - item->pos.x_pos;
					y = v->pos.y_pos - item->pos.y_pos;
					z = v->pos.z_pos - item->pos.z_pos;

					if (x > -TARGET_DIST && x < TARGET_DIST &&
						z > -TARGET_DIST && z < TARGET_DIST &&
						y > -TARGET_DIST && y < TARGET_DIST)
					{
						if (TestBoundsCollide(item, v, KAYAK_TO_BADDIE_RADIUS))
						{
							DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - STEP_L, lara_item->pos.z_pos, v->speed, v->pos.y_rot, lara_item->room_number, 3);

							lara_item->hit_points -= 5;
						}
					}
				}
			}

			item_num = item->next_item;
		}
	}
}

void LaraRapidsDrown()
{
	auto l = lara_item;

	l->anim_number = objects[kayak_anim_obj].anim_index + KAYAK_DROWN_A;
	l->frame_number = anims[l->anim_number].frame_base;
	l->current_anim_state = EXTRA_RAPIDSDROWN;
	l->goal_anim_state = EXTRA_RAPIDSDROWN;
	l->hit_points = 0;
	l->fallspeed = 0;
	l->gravity_status = 0;
	l->speed = 0;

	AnimateItem(l);

	lara.extra_anim = 1;
	lara.gun_status = LG_HANDSBUSY;
	lara.gun_type = LG_UNARMED;
	lara.hit_direction = -1;
}

void KayakInitialise(int16_t item_number)
{
	auto v = &items[item_number];

	v->data = game_malloc(sizeof(KAYAKINFO), 0);

	auto Kayak = (KAYAKINFO*)v->data;

	Kayak->Vel = Kayak->Rot = Kayak->Flags = 0;
	Kayak->FallSpeedF = Kayak->FallSpeedL = Kayak->FallSpeedR = 0;
	Kayak->OldPos = v->pos;

	for (int i = 0; i < NUM_WAKE_SPRITES; ++i)
		WakePtsKayak[i][0].life = WakePtsKayak[i][1].life = 0;
}

void KayakDraw(ITEM_INFO* v)
{
	v->pos.y_pos += KAYAK_DRAW_SHIFT;

	DrawAnimatingItem(v);

	v->pos.y_pos -= KAYAK_DRAW_SHIFT;
}

void KayakCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
		return;

	if (int geton = GetInKayak(item_number, coll))
	{
		auto v = &items[item_number];

		if (auto entity = g_level->get_entity_by_item(v))
			if (!g_level->is_entity_streamed(entity))
			{
				coll->enable_baddie_push = 1;

				ObjectCollision(item_number, l, coll);

				return g_level->request_entity_ownership(entity, true);
			}

		lara.skidoo = item_number;

		if (lara.gun_type == LG_FLARE)
		{
			create_flare(0);
			undraw_flare_meshes();

			lara.flare_control_left = 0;
			lara.request_gun_type = lara.gun_type = LG_ARMLESS;
		}

		l->anim_number = objects[kayak_anim_obj].anim_index + (geton > 0 ? KAYAK_CLIMBIN_A : KAYAK_CLIMBINR_A);
		l->frame_number = anims[l->anim_number].frame_base;
		l->current_anim_state = l->goal_anim_state = KS_CLIMBIN;
		l->pos.x_pos = v->pos.x_pos;
		l->pos.y_pos = v->pos.y_pos;
		l->pos.z_pos = v->pos.z_pos;
		l->pos.y_rot = v->pos.y_rot;
		l->pos.x_rot = l->pos.z_rot = 0;
		l->gravity_status = 0;
		l->speed = 0;
		l->fallspeed = 0;

		lara.water_status = LARA_ABOVEWATER;

		if (l->room_number != v->room_number)
			ItemNewRoom(lara.item_number, v->room_number);

		AnimateItem(l);

		auto Kayak = (KAYAKINFO*)v->data;

		Kayak->Water = v->pos.y_pos;
		Kayak->Flags = 0;
	}
	else
	{
		coll->enable_baddie_push = 1;

		ObjectCollision(item_number, l, coll);
	}
}

int KayakControl()
{
	auto l = lara_item;
	auto v = &items[lara.skidoo];
	auto Kayak = (KAYAKINFO*)v->data;

	if (input & IN_LOOK)
		LookUpDown();

	int ofs = v->fallspeed,
		water;

	KayakUserInput(v, l, Kayak);
	KayakToBackground(v, Kayak);

	auto room_number = v->room_number;
	auto floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	TestTriggers(trigger_index, 0);

	if ((Kayak->Water = water = GetWaterHeight(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, room_number)) == NO_HEIGHT)
	{
		Kayak->Water = water = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
		Kayak->TrueWater = 0;
	}
	else
	{
		Kayak->Water -= 5;
		Kayak->TrueWater = 1;
	}

	if (ofs - v->fallspeed > 128 && v->fallspeed == 0 && water != NO_HEIGHT)
	{
		if (int damage = (ofs - v->fallspeed);damage > 160)
			l->hit_points -= (damage - 160) << 3;

		KayakSplash(v, ofs - v->fallspeed, water);
	}

	if (lara.skidoo != NO_ITEM)
	{
		if (v->room_number != room_number)
		{
			ItemNewRoom(lara.skidoo, room_number);
			ItemNewRoom(lara.item_number, room_number);
		}

		l->pos.x_pos = v->pos.x_pos;
		l->pos.y_pos = v->pos.y_pos + KAYAK_DRAW_SHIFT;
		l->pos.z_pos = v->pos.z_pos;
		l->pos.x_rot = v->pos.x_rot;
		l->pos.y_rot = v->pos.y_rot;
		l->pos.z_rot = v->pos.z_rot >> 1;

		AnimateItem(l);

		v->anim_number = objects[KAYAK].anim_index + (l->anim_number - objects[kayak_anim_obj].anim_index);
		v->frame_number = anims[v->anim_number].frame_base + (l->frame_number - anims[l->anim_number].frame_base);

		camera.target_elevation = -30 * ONE_DEGREE;
		camera.target_distance = WALL_L * 2;
	}

	if (!(wibble & 15) && Kayak->TrueWater)
	{
		DoWakeKayak(v, -128, 0, 0);
		DoWakeKayak(v, 128, 0, 1);
	}

	if (v->speed == 0 && !lara.current_xvel && !lara.current_zvel)
	{
		if (WakeShade)
			--WakeShade;
	}
	else if (WakeShade < 16)
		++WakeShade;

	UpdateWakeFXKayak();

	KayakToBaddieCollision(v);

	return (lara.skidoo != NO_ITEM) ? 1 : 0;
}