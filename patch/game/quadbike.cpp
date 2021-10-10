#include <specific/standard.h>
#include <specific/global.h>
#include <specific/input.h>
#include <specific/output.h>
#include <specific/init.h>

#include <mp/game/player.h>
#include <mp/game/level.h>

#include "objects.h"
#include "laraanim.h"
#include "control.h"
#include "camera.h"
#include "sphere.h"
#include "laraflar.h"
#include "effect2.h"
#include "quadbike.h"
#include "lara1gun.h"
#include "physics.h"
#include "game.h"
#include "missile.h"

#define GF2(a)					(anims[objects[QUADBIKE].anim_index+a].frame_base)
#define QUADBIKE_TURNL_A		3
#define QUADBIKE_TURNL_F		GF2(QUADBIKE_TURNL_A)
#define QUADBIKE_TURNR_A		20
#define QUADBIKE_TURNR_F		GF2(QUADBIKE_TURNR_A)
#define QUADBIKE_FALLSTART_A	6
#define QUADBIKE_FALLSTART_F	GF2(QUADBIKE_FALLSTART_A)
#define QUADBIKE_FALL_A			7
#define QUADBIKE_FALL_F			GF2(QUADBIKE_FALL_A)
#define QUADBIKE_GETONR_A		9
#define QUADBIKE_GETONR_F		GF2(QUADBIKE_GETONR_A)
#define Q_HITB_A				11
#define Q_HITF_A				12
#define Q_HITL_A				14
#define Q_HITR_A				13
#define QUADBIKE_GETONL_A		23
#define QUADBIKE_GETONL_F		GF2(QUADBIKE_GETONL_A)
#define QUADBIKE_FALLSTART2_A	25
#define IN_ACCELERATE			(IN_ACTION)
#define IN_BRAKE				(IN_JUMP)
#define IN_DISMOUNT				(IN_ROLL)
#define IN_HANDBRAKE			(IN_DASH | IN_DUCK)
#define WADE_DEPTH				(STEP_L)
#define SWIM_DEPTH				730
#define REV_PITCH1S				-0x2000
#define REV_PITCH1E				0x6800
#define REV_PITCH2S				-0x2800
#define REV_PITCH2E				0x7000
#define REV_INAIR 				0xa000
#define ACCELERATION_1			0x4000
#define ACCELERATION_2			0x7000
#define MAX_SPEED				0xa000
#define MIN_HANDBRAKE_SPEED		0x3000
#define FRICTION				0x0100
#define BRAKE					0x0280
#define REVERSE_ACC				-0x0300
#define MAX_BACK				-0x3000
#define MAX_REVS				0xa000
#define TERMINAL_FALLSPEED		240
#define SKIDOO_SLIP				100
#define SKIDOO_SLIP_SIDE		50
#define SKIDOO_MAX_KICK			-80
#define QUAD_FRONT				550
#define QUAD_BACK				-550
#define QUAD_SIDE				260
#define QUAD_RADIUS				500
#define QUAD_HEIGHT				512
#define SMAN_SHOT_DAMAGE		10
#define SMAN_LARA_DAMAGE		50
#define DAMAGE_START			140
#define DAMAGE_LENGTH			14
#define SKIDOO_GETON_ANIM		1
#define SKIDOO_GETONL_ANIM		18
#define SKIDOO_FALL_ANIM		8
#define SKIDOO_DEAD_ANIM		15
#define SKIDOO_HIT_LEFT			11
#define SKIDOO_HIT_RIGHT		12
#define SKIDOO_HIT_FRONT		13
#define SKIDOO_HIT_BACK			14
#define GETOFF_DIST				512
#define SKIDOO_UNDO_TURN		(ONE_DEGREE * 2)
#define SKIDOO_TURN				((ONE_DEGREE / 2) + SKIDOO_UNDO_TURN)
#define SKIDOO_MAX_TURN			(ONE_DEGREE * 5)
#define SKIDOO_HTURN			(((ONE_DEGREE * 3) / 4) + SKIDOO_UNDO_TURN)
#define SKIDOO_MAX_HTURN		(ONE_DEGREE * 8)
#define MIN_MOMENTUM_TURN		(ONE_DEGREE * 3)
#define MAX_MOMENTUM_TURN		(ONE_DEGREE + (ONE_DEGREE / 2))
#define SKIDOO_MAX_MOM_TURN		(ONE_DEGREE * 150)
#define SKIDOO_FRONT			550
#define SKIDOO_SIDE				260
#define SKIDOO_RADIUS			500
#define SKIDOO_SNOW				500
#define SKIDOO_MAX_HEIGHT		(STEP_L)
#define SKIDOO_MIN_BOUNCE		((MAX_SPEED / 2) >> 8)
#define SMAN_MIN_TURN			(SKIDOO_MAX_TURN / 3)
#define SMAN_TARGET_ANGLE		(ONE_DEGREE * 15)
#define SMAN_WAIT_RANGE			SQUARE(WALL_L * 4)
#define SMAN_DEATH_ANIM			10
#define TARGET_DIST				(WALL_L * 2)

enum
{
	QS_0,
	QS_DRIVE,
	QS_TURNL, QS_3, QS_4,
	QS_SLOW,
	QS_BRAKE,
	QS_BIKEDEATH,
	QS_FALL,
	QS_GETONR,
	QS_GETOFFR,		// 10
	QS_HITBACK,
	QS_HITFRONT,
	QS_HITLEFT,
	QS_HITRIGHT,
	QS_STOP, QS_16,
	QS_LAND,
	QS_STOPSLOWLY,
	QS_FALLDEATH,
	QS_FALLOFF,		// 20
	QS_WHEELIE,
	QS_TURNR,
	QS_GETONL,
	QS_GETOFFL,
};

BITE_INFO quad_bites[6] =
{
	{ -56, -32, -380, 0 },	// Exhaust left.
	{ 56, -32, -380, 0 },	// Exhaust right.
	{ -8, 180, -48, 3 },	// Back left.
	{ 8, 180, -48, 4 },		// Back right.
	{ 90, 180, -32, 6 },	// Front right.
	{ -90, 180, -32, 7 }	// Front left.
};

uint8_t HandbrakeStarting,
	    CanHandbrakeStart,
	    ExhaustStart;

void TriggerExhaustSmoke(long x, long y, long z, int16_t angle, long speed, long moving);

void QuadbikeExplode(ITEM_INFO* v)
{
	if (room[v->room_number].flags & UNDERWATER)
		TriggerUnderwaterExplosion(v);
	else
	{
		TriggerExplosionSparks(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, 3, -2, 0, v->room_number);

		for (int i = 0; i < 3; ++i)
			TriggerExplosionSparks(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, 3, -1, 0, v->room_number);
	}

	ExplodingDeath(lara.skidoo, 0xfffffffe, 1);
	KillItem(lara.skidoo);

	g_audio->stop_sound(202);
	g_audio->stop_sound(155);

	v->status = DEACTIVATED;

	g_audio->play_sound(105);
	g_audio->play_sound(106);

	lara.skidoo = NO_ITEM;

	if (auto entity = g_level->get_entity_by_item(v))
		g_level->request_entity_ownership(entity, false);
}

int CanGetOffQB(int direction)
{
	auto v = &items[lara.skidoo];

	int16_t angle = (direction < 0 ? v->pos.y_rot - 0x4000 : v->pos.y_rot + 0x4000);

	int x = v->pos.x_pos + (GETOFF_DIST * phd_sin(angle) >> W2V_SHIFT),
		y = v->pos.y_pos,
		z = v->pos.z_pos + (GETOFF_DIST * phd_cos(angle) >> W2V_SHIFT);

	auto room_number = v->room_number;
	auto floor = GetFloor(x, y, z, &room_number);

	int height = GetHeight(floor, x, y, z);

	if (height_type == BIG_SLOPE || height_type == DIAGONAL || height == NO_HEIGHT)
		return 0;

	if (ABS(height - v->pos.y_pos) > WALL_L / 2)
		return 0;

	int ceiling = GetCeiling(floor, x, y, z);

	return (ceiling - v->pos.y_pos <= -LARA_HITE && height - ceiling >= LARA_HITE);
}

int SkidooCheckGetOff()
{
	auto v = &items[lara.skidoo];
	auto l = lara_item;

	if ((l->current_anim_state == QS_GETOFFR || l->current_anim_state == QS_GETOFFL) && l->frame_number == anims[l->anim_number].frame_end)
	{
		l->pos.y_rot += (l->current_anim_state == QS_GETOFFL ? 0x4000 : -0x4000);
		l->anim_number = STOP_A;
		l->frame_number = STOP_F;
		l->current_anim_state = l->goal_anim_state = AS_STOP;
		l->pos.x_pos -= GETOFF_DIST * phd_sin(l->pos.y_rot) >> W2V_SHIFT;
		l->pos.z_pos -= GETOFF_DIST * phd_cos(l->pos.y_rot) >> W2V_SHIFT;
		l->pos.x_rot = l->pos.z_rot = 0;

		lara.skidoo = NO_ITEM;
		lara.gun_status = LG_ARMLESS;

		if (auto entity = g_level->get_entity_by_item(v))
			g_level->request_entity_ownership(entity, false);
	}
	else if (l->frame_number == anims[l->anim_number].frame_end)
	{
		auto bike = (QUADINFO*)v->data;

		if (l->current_anim_state == QS_FALLOFF)
		{
			PHD_VECTOR vec {};

			l->anim_number = FASTFALL_A;
			l->frame_number = FASTFALL_F;
			l->current_anim_state = AS_FASTFALL;

			get_lara_bone_pos(lara_item, &vec, HIPS);

			l->pos.x_pos = vec.x;
			l->pos.y_pos = vec.y;
			l->pos.z_pos = vec.z;
			l->fallspeed = v->fallspeed;
			l->gravity_status = 1;
			l->pos.x_rot = l->pos.z_rot = 0;
			l->hit_points = 0;
			v->flags |= ONESHOT;

			lara.gun_status = LG_ARMLESS;

			return 0;
		}
		else if (l->current_anim_state == QS_FALLDEATH)
		{
			l->goal_anim_state = AS_DEATH;
			l->fallspeed = DAMAGE_START + DAMAGE_LENGTH;
			l->speed = 0;

			bike->Flags |= QF_DEAD;

			return 0;
		}
	}

	return 1;
}

int GetOnQuadBike(int16_t item_number, COLL_INFO* coll)
{
	auto l = lara_item;
	auto v = &items[item_number];

	if (!(input & IN_ACTION) || v->flags & ONESHOT || lara.gun_status != LG_ARMLESS || l->gravity_status || ABS(v->pos.y_pos - l->pos.y_pos) > 256)
		return 0;

	int x = l->pos.x_pos - v->pos.x_pos,
		z = l->pos.z_pos - v->pos.z_pos,
		dist = (x * x) + (z * z);

	if (dist > 170000)
		return 0;

	auto room_number = v->room_number;
	auto floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	if (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) < -32000)
		return 0;
	else
	{
		auto ang = phd_atan(v->pos.z_pos - l->pos.z_pos, v->pos.x_pos - l->pos.x_pos) - v->pos.y_rot;

		if (ang > -ONE_DEGREE * 45 && ang < ONE_DEGREE * 135)
		{
			uint16_t tempang = l->pos.y_rot - v->pos.y_rot;
			return (tempang > 45 * ONE_DEGREE && tempang < 135 * ONE_DEGREE);
		}
		else
		{
			uint16_t tempang = l->pos.y_rot - v->pos.y_rot;
			return (tempang > 225 * ONE_DEGREE && tempang < 315 * ONE_DEGREE);
		}
	}

	return 0;
}

void SkidooBaddieCollision(ITEM_INFO* skidoo)
{
	int16_t roomies[16],
		    numroom = 1;

	roomies[0] = skidoo->room_number;

	if (auto door = room[skidoo->room_number].door)
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

			if (item->collidable && item->status != INVISIBLE && item != lara_item && item != skidoo)
			{
				if (auto object = &objects[item->object_number]; object->collision && (object->intelligent || item->object_number == AVALANCHE))
				{
					int x = skidoo->pos.x_pos - item->pos.x_pos,
						y = skidoo->pos.y_pos - item->pos.y_pos,
						z = skidoo->pos.z_pos - item->pos.z_pos;

					if (x > -TARGET_DIST && x < TARGET_DIST &&
						z > -TARGET_DIST && z < TARGET_DIST &&
						y > -TARGET_DIST && y < TARGET_DIST)
					{
						if (auto player = g_level->get_entity_by_item(item))
						{
							// TODO
						}
						else if (TestBoundsCollide(item, skidoo, SKIDOO_RADIUS))
						{
							if (item->object_number == AVALANCHE)
							{
								if (item->current_anim_state == TRAP_ACTIVATE)
								{
									lara_item->hit_status = 1;
									lara_item->hit_points -= 100;
								}
							}
							else
							{
								DoLotsOfBlood(item->pos.x_pos, skidoo->pos.y_pos - STEP_L, item->pos.z_pos, skidoo->speed, skidoo->pos.y_rot, item->room_number, 3);

								item->hit_points = 0;
							}
						}
					}
				}
			}

			item_num = item->next_item;
		}
	}
}

int GetCollisionAnimQB(ITEM_INFO* v, PHD_VECTOR* moved)
{
	if ((moved->x = v->pos.x_pos - moved->x) || (moved->z = v->pos.z_pos - moved->z))
	{
		int c = phd_cos(v->pos.y_rot),
			s = phd_sin(v->pos.y_rot),
			front = ((moved->z * c) + (moved->x * s)) >> W2V_SHIFT,
			side = ((-moved->z * s) + (moved->x * c)) >> W2V_SHIFT;

		if (ABS(front) > ABS(side))
			return (front > 0 ? SKIDOO_HIT_BACK : SKIDOO_HIT_FRONT);
		else return (side > 0 ? SKIDOO_HIT_LEFT : SKIDOO_HIT_RIGHT);
	}

	return 0;
}

int32_t TestHeightQB(ITEM_INFO* item, int32_t z_off, int32_t x_off, PHD_VECTOR* pos)
{
	pos->y = item->pos.y_pos - (z_off * phd_sin(item->pos.x_rot) >> W2V_SHIFT) + (x_off * phd_sin(item->pos.z_rot) >> W2V_SHIFT);

	int c = phd_cos(item->pos.y_rot),
		s = phd_sin(item->pos.y_rot);

	pos->z = item->pos.z_pos + ((z_off * c - x_off * s) >> W2V_SHIFT);
	pos->x = item->pos.x_pos + ((z_off * s + x_off * c) >> W2V_SHIFT);

	auto room_number = item->room_number;
	auto floor = GetFloor(pos->x, pos->y, pos->z, &room_number);

	c = GetCeiling(floor, pos->x, pos->y, pos->z);

	return (pos->y < c || c == NO_HEIGHT ? NO_HEIGHT : GetHeight(floor, pos->x, pos->y, pos->z));
}

int DoShiftQB(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old)
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

		if (GetHeight(floor, old->x, pos->y, pos->z) < old->y - STEP_L)
			z = (pos->z > old->z ? -shift_z - 1 : WALL_L - shift_z);

		room_number = skidoo->room_number;
		floor = GetFloor(pos->x, pos->y, old->z, &room_number);

		if (GetHeight(floor, pos->x, pos->y, old->z) < old->y - STEP_L)
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

int DoDynamicsQB(int height, int fallspeed, int* y)
{
	if (height > *y)
	{
		*y += fallspeed;

		if (*y > height - SKIDOO_MIN_BOUNCE)
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

int SkidooDynamics(ITEM_INFO* v)
{
	PHD_VECTOR fl_old, fr_old, bl_old, br_old, mtl_old, mbl_old, mtr_old, mbr_old, mml_old, mmr_old;

	auto bike = (QUADINFO*)v->data;

	int hfl_old = TestHeightQB(v, SKIDOO_FRONT, -SKIDOO_SIDE, &fl_old),
		hfr_old = TestHeightQB(v, SKIDOO_FRONT, SKIDOO_SIDE, &fr_old),
		hbl_old = TestHeightQB(v, -SKIDOO_FRONT, -SKIDOO_SIDE, &bl_old),
		hbr_old = TestHeightQB(v, -SKIDOO_FRONT, SKIDOO_SIDE, &br_old),
		hmml_old = TestHeightQB(v, 0, -SKIDOO_SIDE, &mml_old),
		hmmr_old = TestHeightQB(v, 0, SKIDOO_SIDE, &mmr_old),
		hmtl_old = TestHeightQB(v, SKIDOO_FRONT >> 1, -SKIDOO_SIDE, &mtl_old),
		hmtr_old = TestHeightQB(v, SKIDOO_FRONT >> 1, SKIDOO_SIDE, &mtr_old),
		hmbl_old = TestHeightQB(v, -SKIDOO_FRONT >> 1, -SKIDOO_SIDE, &mbl_old),
		hmbr_old = TestHeightQB(v, -SKIDOO_FRONT >> 1, SKIDOO_SIDE, &mbr_old);

	PHD_VECTOR old { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos };

	if (bl_old.y > hbl_old)   bl_old.y = hbl_old;
	if (br_old.y > hbr_old)   br_old.y = hbr_old;
	if (fl_old.y > hfl_old)	  fl_old.y = hfl_old;
	if (fr_old.y > hfr_old)	  fr_old.y = hfr_old;
	if (mbl_old.y > hmbl_old) mbl_old.y = hmbl_old;
	if (mbr_old.y > hmbr_old) mbr_old.y = hmbr_old;
	if (mtl_old.y > hmtl_old) mtl_old.y = hmtl_old;
	if (mtr_old.y > hmtr_old) mtr_old.y = hmtr_old;
	if (mml_old.y > hmml_old) mml_old.y = hmml_old;
	if (mmr_old.y > hmmr_old) mmr_old.y = hmmr_old;

	if (v->pos.y_pos > (v->floor - STEP_L))
	{
		if (bike->skidoo_turn < -SKIDOO_UNDO_TURN)		bike->skidoo_turn += SKIDOO_UNDO_TURN;
		else if (bike->skidoo_turn > SKIDOO_UNDO_TURN)  bike->skidoo_turn -= SKIDOO_UNDO_TURN;
		else											bike->skidoo_turn = 0;

		v->pos.y_rot += bike->skidoo_turn + bike->extra_rotation;

		int16_t momentum = MIN_MOMENTUM_TURN - (((((MIN_MOMENTUM_TURN - MAX_MOMENTUM_TURN) << 8) / MAX_SPEED) * bike->Velocity) >> 8),
			    rot = v->pos.y_rot - bike->momentum_angle;

		if (!(input & IN_ACCELERATE) && bike->Velocity > 0)
			momentum += momentum >> 2;

		if (rot < -MAX_MOMENTUM_TURN)
		{
			if (rot < -SKIDOO_MAX_MOM_TURN)
			{
				rot = -SKIDOO_MAX_MOM_TURN;
				bike->momentum_angle = v->pos.y_rot - rot;
			}
			else bike->momentum_angle -= momentum;
		}
		else if (rot > MAX_MOMENTUM_TURN)
		{
			if (rot > SKIDOO_MAX_MOM_TURN)
			{
				rot = SKIDOO_MAX_MOM_TURN;
				bike->momentum_angle = v->pos.y_rot - rot;
			}
			else bike->momentum_angle += momentum;
		}
		else bike->momentum_angle = v->pos.y_rot;
	}
	else v->pos.y_rot += bike->skidoo_turn + bike->extra_rotation;

	auto room_number = v->room_number;
	auto floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	int height = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos),
		speed = (v->pos.y_pos >= height ? (v->speed * phd_cos(v->pos.x_rot)) >> W2V_SHIFT : v->speed);

	v->pos.z_pos += (speed * phd_cos(bike->momentum_angle)) >> W2V_SHIFT;
	v->pos.x_pos += (speed * phd_sin(bike->momentum_angle)) >> W2V_SHIFT;

	if (int slip = SKIDOO_SLIP * phd_sin(v->pos.x_rot) >> W2V_SHIFT; ABS(slip) > SKIDOO_SLIP / 2)
	{
		slip += (slip > 0 ? -10 : 10);

		v->pos.z_pos -= slip * phd_cos(v->pos.y_rot) >> W2V_SHIFT;
		v->pos.x_pos -= slip * phd_sin(v->pos.y_rot) >> W2V_SHIFT;
	}

	if (int slip = SKIDOO_SLIP_SIDE * phd_sin(v->pos.z_rot) >> W2V_SHIFT; ABS(slip) > SKIDOO_SLIP_SIDE / 2)
	{
		v->pos.z_pos -= slip * phd_sin(v->pos.y_rot) >> W2V_SHIFT;
		v->pos.x_pos += slip * phd_cos(v->pos.y_rot) >> W2V_SHIFT;
	}

	PHD_VECTOR moved { v->pos.x_pos, 0, v->pos.z_pos },
			   fl, fr, br, bl, mtl, mbl, mtr, mbr, mml, mmr;

	if (!(v->flags & ONESHOT))
		SkidooBaddieCollision(v);

	int rot = 0;

	if (TestHeightQB(v, SKIDOO_FRONT, -SKIDOO_SIDE, &fl) < fl_old.y - STEP_L)
		rot = DoShiftQB(v, &fl, &fl_old);

	if (TestHeightQB(v, SKIDOO_FRONT >> 1, -SKIDOO_SIDE, &mtl) < mtl_old.y - STEP_L)
		DoShiftQB(v, &mtl, &mtl_old);

	if (TestHeightQB(v, 0, -SKIDOO_SIDE, &mml) < mml_old.y - STEP_L)
		DoShiftQB(v, &mml, &mml_old);

	if (TestHeightQB(v, -SKIDOO_FRONT >> 1, -SKIDOO_SIDE, &mbl) < mbl_old.y - STEP_L)
		DoShiftQB(v, &mbl, &mbl_old);

	if (TestHeightQB(v, -SKIDOO_FRONT, -SKIDOO_SIDE, &bl) < bl_old.y - STEP_L)
		if (int rotadd = DoShiftQB(v, &bl, &bl_old); (rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
			rot += rotadd;

	if (TestHeightQB(v, SKIDOO_FRONT, SKIDOO_SIDE, &fr) < fr_old.y - STEP_L)
		if (int rotadd = DoShiftQB(v, &fr, &fr_old); (rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
			rot += rotadd;

	if (TestHeightQB(v, SKIDOO_FRONT >> 1, SKIDOO_SIDE, &mtr) < mtr_old.y - STEP_L)
		DoShiftQB(v, &mtr, &mtr_old);

	if (TestHeightQB(v, 0, SKIDOO_SIDE, &mmr) < mmr_old.y - STEP_L)
		DoShiftQB(v, &mmr, &mmr_old);

	if (TestHeightQB(v, -SKIDOO_FRONT >> 1, SKIDOO_SIDE, &mbr) < mbr_old.y - STEP_L)
		DoShiftQB(v, &mbr, &mbr_old);

	if (TestHeightQB(v, -SKIDOO_FRONT, SKIDOO_SIDE, &br) < br_old.y - STEP_L)
		if (int rotadd = DoShiftQB(v, &br, &br_old); (rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
			rot += rotadd;

	room_number = v->room_number;
	floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	height = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);

	if (height < v->pos.y_pos - STEP_L)
		DoShiftQB(v, (PHD_VECTOR*)&v->pos, &old);

	bike->extra_rotation = rot;

	if (GetCollisionAnimQB(v, &moved))
	{
		int newspeed = (((v->pos.z_pos - old.z) * phd_cos(bike->momentum_angle) + (v->pos.x_pos - old.x) * phd_sin(bike->momentum_angle)) >> W2V_SHIFT) << 8;

		if (&items[lara.skidoo] == v && bike->Velocity == MAX_SPEED && newspeed < (bike->Velocity - 10))
		{
			lara_item->hit_points -= (bike->Velocity - newspeed) >> 7;
			lara_item->hit_status = 1;
		}

		if (bike->Velocity > 0 && newspeed < bike->Velocity)	  bike->Velocity = (newspeed < 0) ? 0 : newspeed;
		else if (bike->Velocity < 0 && newspeed > bike->Velocity) bike->Velocity = (newspeed > 0) ? 0 : newspeed;

		if (bike->Velocity < MAX_BACK)
			bike->Velocity = MAX_BACK;

		return true;
	}

	return false;
}

void AnimateQuadBike(ITEM_INFO* v, int collide, int dead)
{
	auto l = lara_item;
	auto bike = (QUADINFO*)v->data;

	if (v->pos.y_pos != v->floor && l->current_anim_state != QS_FALL && l->current_anim_state != QS_LAND && l->current_anim_state != QS_FALLOFF && !dead)
	{
		l->anim_number = (bike->Velocity < 0 ? objects[quadbike_anim_obj].anim_index + QUADBIKE_FALLSTART_A : objects[quadbike_anim_obj].anim_index + QUADBIKE_FALLSTART2_A);
		l->frame_number = anims[l->anim_number].frame_base;
		l->current_anim_state = l->goal_anim_state = QS_FALL;
	}
	else if (collide &&
		l->current_anim_state != QS_HITFRONT &&
		l->current_anim_state != QS_HITBACK &&
		l->current_anim_state != QS_HITLEFT &&
		l->current_anim_state != QS_HITRIGHT &&
		l->current_anim_state != QS_FALLOFF &&
		bike->Velocity > (MAX_SPEED / 3) &&
		!dead)
	{
		if (collide == SKIDOO_HIT_FRONT)
		{
			l->anim_number = objects[quadbike_anim_obj].anim_index + Q_HITF_A;
			l->current_anim_state = l->goal_anim_state = QS_HITFRONT;
		}
		else if (collide == SKIDOO_HIT_BACK)
		{
			l->anim_number = objects[quadbike_anim_obj].anim_index + Q_HITB_A;
			l->current_anim_state = l->goal_anim_state = QS_HITBACK;
		}
		else if (collide == SKIDOO_HIT_LEFT)
		{
			l->anim_number = objects[quadbike_anim_obj].anim_index + Q_HITL_A;
			l->current_anim_state = l->goal_anim_state = QS_HITLEFT;
		}
		else
		{
			l->anim_number = objects[quadbike_anim_obj].anim_index + Q_HITR_A;
			l->current_anim_state = l->goal_anim_state = QS_HITRIGHT;
		}

		l->frame_number = anims[l->anim_number].frame_base;

		g_audio->play_sound(202, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });
	}
	else
	{
		switch (l->current_anim_state)
		{
		case QS_STOP:
		{
			g_audio->play_sound(153, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });

			if (dead)
				l->goal_anim_state = QS_BIKEDEATH;

			else if (input & IN_DISMOUNT && bike->Velocity == 0)
			{
				if ((input & IN_RIGHT) && CanGetOffQB(1))	   l->goal_anim_state = QS_GETOFFR;
				else if ((input & IN_LEFT) && CanGetOffQB(-1)) l->goal_anim_state = QS_GETOFFL;
			}
			else if (input & (IN_ACCELERATE | IN_BRAKE))
				l->goal_anim_state = QS_DRIVE;

			break;
		}
		case QS_DRIVE:
		{
			g_audio->stop_sound(153);

			if (dead)																	  l->goal_anim_state = (bike->Velocity > (MAX_SPEED / 2) ? QS_FALLDEATH : QS_BIKEDEATH);
			else if ((bike->Velocity >> 8) == 0 && !(input & (IN_ACCELERATE | IN_BRAKE))) l->goal_anim_state = QS_STOP;
			else if (input & IN_LEFT && !HandbrakeStarting)								  l->goal_anim_state = QS_TURNL;
			else if (input & IN_RIGHT && !HandbrakeStarting)							  l->goal_anim_state = QS_TURNR;
			else if (input & IN_BRAKE)													  l->goal_anim_state = (bike->Velocity > ((MAX_SPEED / 3) * 2) ? QS_BRAKE : QS_SLOW);

			break;
		}
		case QS_BRAKE:
		case QS_SLOW:
		case QS_STOPSLOWLY:
		{
			if ((bike->Velocity >> 8) == 0) l->goal_anim_state = QS_STOP;
			else if (input & IN_LEFT)		l->goal_anim_state = QS_TURNL;
			else if (input & IN_RIGHT)		l->goal_anim_state = QS_TURNR;

			break;
		}
		case QS_TURNL:
		{
			if ((bike->Velocity >> 8) == 0)
				l->goal_anim_state = QS_STOP;
			else if (input & IN_RIGHT)
			{
				l->anim_number = objects[quadbike_anim_obj].anim_index + QUADBIKE_TURNR_A;
				l->frame_number = anims[l->anim_number].frame_base;
				l->current_anim_state = l->goal_anim_state = QS_TURNR;
			}
			else if (!(input & IN_LEFT))
				l->goal_anim_state = QS_DRIVE;

			break;
		}
		case QS_TURNR:
		{
			if ((bike->Velocity >> 8) == 0)
				l->goal_anim_state = QS_STOP;
			else if (input & IN_LEFT)
			{
				l->anim_number = objects[quadbike_anim_obj].anim_index + QUADBIKE_TURNL_A;
				l->frame_number = anims[l->anim_number].frame_base;
				l->current_anim_state = l->goal_anim_state = QS_TURNL;
			}
			else if (!(input & IN_RIGHT))
				l->goal_anim_state = QS_DRIVE;

			break;
		}
		case QS_FALL:
		{
			if (v->pos.y_pos == v->floor)				l->goal_anim_state = QS_LAND;
			else if (v->fallspeed > TERMINAL_FALLSPEED) bike->Flags |= QF_FALLING;

			break;
		}
		case QS_FALLOFF:
			break;
		case QS_HITFRONT:
		case QS_HITBACK:
		case QS_HITLEFT:
		case QS_HITRIGHT:
		{
			if (input & (IN_ACCELERATE | IN_BRAKE))
				l->goal_anim_state = QS_DRIVE;
		}
		}
	}

	if (room[v->room_number].flags & (UNDERWATER | SWAMP))
	{
		l->goal_anim_state = QS_FALLOFF;
		l->hit_points = 0;

		QuadbikeExplode(v);
	}
}

int UserControl(ITEM_INFO* v, int32_t height, int* pitch)
{
	auto bike = (QUADINFO*)v->data;

	if (!bike->Velocity && !(input & IN_HANDBRAKE) && !CanHandbrakeStart) CanHandbrakeStart = 1;
	else if (bike->Velocity)											  CanHandbrakeStart = 0;

	if (!(input & IN_HANDBRAKE))
		HandbrakeStarting = 0;

	if (!HandbrakeStarting)
	{
		if (bike->Revs > 0x10)
		{
			bike->Velocity += bike->Revs >> 4;
			bike->Revs -= bike->Revs >> 3;
		}
		else bike->Revs = 0;
	}

	if (v->pos.y_pos >= (height - STEP_L))
	{
		if (!bike->Velocity && (input & IN_LOOK))
			LookUpDown();

		if (bike->Velocity > 0)
		{
			if ((input & IN_HANDBRAKE) && !HandbrakeStarting && bike->Velocity > MIN_HANDBRAKE_SPEED)
			{
				if (input & IN_LEFT)
				{
					bike->skidoo_turn -= SKIDOO_HTURN;

					if (bike->skidoo_turn < -SKIDOO_MAX_HTURN)
						bike->skidoo_turn = -SKIDOO_MAX_HTURN;
				}
				else if (input & IN_RIGHT)
				{
					bike->skidoo_turn += SKIDOO_HTURN;

					if (bike->skidoo_turn > SKIDOO_MAX_HTURN)
						bike->skidoo_turn = SKIDOO_MAX_HTURN;
				}
			}
			else
			{
				if (input & IN_LEFT)
				{
					bike->skidoo_turn -= SKIDOO_TURN;

					if (bike->skidoo_turn < -SKIDOO_MAX_TURN)
						bike->skidoo_turn = -SKIDOO_MAX_TURN;
				}
				else if (input & IN_RIGHT)
				{
					bike->skidoo_turn += SKIDOO_TURN;

					if (bike->skidoo_turn > SKIDOO_MAX_TURN)
						bike->skidoo_turn = SKIDOO_MAX_TURN;
				}
			}
		}
		else if (bike->Velocity < 0)
		{
			if ((input & IN_HANDBRAKE) && !HandbrakeStarting && bike->Velocity < -MIN_HANDBRAKE_SPEED + 0x800)
			{
				if (input & IN_RIGHT)
				{
					bike->skidoo_turn -= SKIDOO_HTURN;

					if (bike->skidoo_turn < -SKIDOO_MAX_HTURN)
						bike->skidoo_turn = -SKIDOO_MAX_HTURN;
				}
				else if (input & IN_LEFT)
				{
					bike->skidoo_turn += SKIDOO_HTURN;

					if (bike->skidoo_turn > SKIDOO_MAX_HTURN)
						bike->skidoo_turn = SKIDOO_MAX_HTURN;
				}
			}
			else
			{
				if (input & IN_RIGHT)
				{
					bike->skidoo_turn -= SKIDOO_TURN;

					if (bike->skidoo_turn < -SKIDOO_MAX_TURN)
						bike->skidoo_turn = -SKIDOO_MAX_TURN;
				}
				else if (input & IN_LEFT)
				{
					bike->skidoo_turn += SKIDOO_TURN;

					if (bike->skidoo_turn > SKIDOO_MAX_TURN)
						bike->skidoo_turn = SKIDOO_MAX_TURN;
				}
			}
		}

		if (input & IN_BRAKE)
		{
			if ((input & IN_HANDBRAKE) && (CanHandbrakeStart || HandbrakeStarting))
			{
				HandbrakeStarting = 1;

				bike->Revs -= 0x200;

				if (bike->Revs < MAX_BACK)
					bike->Revs = MAX_BACK;
			}
			else if (bike->Velocity > 0)
				bike->Velocity -= BRAKE;
			else if (bike->Velocity > MAX_BACK)
				bike->Velocity += REVERSE_ACC;
		}
		else if (input & IN_ACCELERATE)
		{
			if ((input & IN_HANDBRAKE) && (CanHandbrakeStart || HandbrakeStarting))
			{
				HandbrakeStarting = 1;

				bike->Revs += 0x200;

				if (bike->Revs >= MAX_SPEED)
					bike->Revs = MAX_SPEED;
			}
			else if (bike->Velocity < MAX_SPEED)
			{
				if (bike->Velocity < ACCELERATION_1)	  bike->Velocity += 8 + ((ACCELERATION_1 + 0x800 - bike->Velocity) >> 3);
				else if (bike->Velocity < ACCELERATION_2) bike->Velocity += 4 + ((ACCELERATION_2 + 0x800 - bike->Velocity) >> 4);
				else if (bike->Velocity < MAX_SPEED)	  bike->Velocity += 2 + ((MAX_SPEED - bike->Velocity) >> 3);
			}
			else bike->Velocity = MAX_SPEED;

			bike->Velocity -= (ABS(v->pos.y_rot - bike->momentum_angle)) >> 6;
		}
		else if (bike->Velocity > FRICTION)
			bike->Velocity -= FRICTION;
		else if (bike->Velocity < -FRICTION)
			bike->Velocity += FRICTION;
		else bike->Velocity = 0;

		if (HandbrakeStarting && bike->Revs && !(input & (IN_ACCELERATE | IN_BRAKE)))
			bike->Revs -= (bike->Revs > 0x8 ? bike->Revs >> 3 : 0);

		v->speed = bike->Velocity >> 8;

		if (bike->EngineRevs > REV_PITCH2E)
			bike->EngineRevs = REV_PITCH1S;

		int revs;

		if (bike->Velocity < 0)						revs = abs(bike->Velocity >> 1);
		else if (bike->Velocity < ACCELERATION_2)   revs = REV_PITCH1S + ((bike->Velocity * (REV_PITCH1E - REV_PITCH1S)) / ACCELERATION_2);
		else if (bike->Velocity <= MAX_SPEED)		revs = REV_PITCH2S + (((bike->Velocity - ACCELERATION_2) * (REV_PITCH2E - REV_PITCH2S)) / (MAX_SPEED - ACCELERATION_2));

		revs += abs(bike->Revs);

		bike->EngineRevs += (revs - bike->EngineRevs) >> 3;
	}
	else if (bike->EngineRevs < REV_INAIR)
		bike->EngineRevs += (REV_INAIR - bike->EngineRevs) >> 3;

	*pitch = bike->EngineRevs;

	return 0;
}

void InitialiseQuadBike(int16_t item_number)
{
	auto v = &items[item_number];

	v->data = game_malloc(sizeof(QUADINFO), 0);

	auto bike = (QUADINFO*)v->data;

	bike->Velocity = 0;
	bike->skidoo_turn = 0;
	bike->left_fallspeed = bike->right_fallspeed = 0;
	bike->momentum_angle = v->pos.y_rot;
	bike->extra_rotation = 0;
	bike->track_mesh = 0;
	bike->pitch = 0;
	bike->Flags = 0;
}

void QuadBikeCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
		return;

	auto v = &items[item_number];

	if (GetOnQuadBike(item_number, coll))
	{
		if (auto entity = g_level->get_entity_by_item(v))
			if (!g_level->is_entity_streamed(entity))
			{
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

		lara.gun_status = LG_HANDSBUSY;

		if (int ang = phd_atan(v->pos.z_pos - l->pos.z_pos, v->pos.x_pos - l->pos.x_pos) - v->pos.y_rot;
			(ang > -(ONE_DEGREE * 45)) && (ang < (ONE_DEGREE * 135)))
		{
			l->anim_number = objects[quadbike_anim_obj].anim_index + QUADBIKE_GETONL_A;
			l->current_anim_state = l->goal_anim_state = QS_GETONL;
		}
		else
		{
			l->anim_number = objects[quadbike_anim_obj].anim_index + QUADBIKE_GETONR_A;
			l->current_anim_state = l->goal_anim_state = QS_GETONR;
		}

		l->frame_number = anims[l->anim_number].frame_base;

		v->hit_points = 1;

		l->pos.x_pos = v->pos.x_pos;
		l->pos.y_pos = v->pos.y_pos;
		l->pos.z_pos = v->pos.z_pos;
		l->pos.y_rot = v->pos.y_rot;

		lara.head_x_rot = lara.head_y_rot = 0;
		lara.torso_x_rot = lara.torso_y_rot = 0;
		lara.hit_direction = -1;

		AnimateItem(l);

		auto bike = (QUADINFO*)v->data;

		bike->Revs = 0;
	}
	else ObjectCollision(item_number, l, coll);
}

void QuadBikeDraw(ITEM_INFO* item)
{
	int16_t* frmptr[2] = { nullptr, nullptr };

	int rate,
		frac = GetFrames(item, frmptr, &rate);

	if (!frmptr[0] || !frmptr[1])
		return;

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	if (int clip = S_GetObjectBounds(frmptr[0]))
	{
		CalculateObjectLighting(item, frmptr[0]);

		auto object = &objects[item->object_number];
		auto meshpp = &meshes[object->mesh_index];
		auto bone = bones + object->bone_index;
		auto bike = (QUADINFO*)item->data;

		phd_TranslateRel((int32_t) * (frmptr[0] + 6), (int32_t) * (frmptr[0] + 7), (int32_t) * (frmptr[0] + 8));
		
		auto rotation = frmptr[0] + 9;
		
		gar_RotYXZsuperpack(&rotation, 0);

		phd_PutPolygons(*meshpp++, clip);

		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
		gar_RotYXZsuperpack(&rotation, 0);
		phd_PutPolygons(*meshpp++, clip);
		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
		gar_RotYXZsuperpack(&rotation, 0);
		phd_PutPolygons(*meshpp++, clip);

		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
		gar_RotYXZsuperpack(&rotation, 0);
		phd_RotX(bike->RearRot);
		phd_PutPolygons(*meshpp++, clip);
		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
		gar_RotYXZsuperpack(&rotation, 0);
		phd_RotX(bike->RearRot);
		phd_PutPolygons(*meshpp++, clip);
		phd_PopMatrix();

		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
		gar_RotYXZsuperpack(&rotation, 0);
		phd_PutPolygons(*meshpp++, clip);

		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
		gar_RotYXZsuperpack(&rotation, 0);
		phd_RotX(bike->FrontRot);
		phd_PutPolygons(*meshpp++, clip);
		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));
		gar_RotYXZsuperpack(&rotation, 0);
		phd_RotX(bike->FrontRot);
		phd_PutPolygons(*meshpp++, clip);
		phd_PopMatrix();
		phd_PopMatrix();
	}

	phd_PopMatrix();
}

int QuadBikeControl()
{
	auto l = lara_item;
	auto v = &items[lara.skidoo];
	auto bike = (QUADINFO*)v->data;

	GAME_VECTOR	oldpos { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, v->room_number };

	int collide = SkidooDynamics(v),
		pitch,
		drive;

	auto room_number = v->room_number;
	auto floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	PHD_VECTOR fl, fr;

	int hfl = TestHeightQB(v, SKIDOO_FRONT, -SKIDOO_SIDE, &fl),
		hfr = TestHeightQB(v, SKIDOO_FRONT, SKIDOO_SIDE, &fr);

	room_number = v->room_number;
	floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	int height = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);

	TestTriggers(trigger_index, 0);

	bool dead = false;

	if (lara_item->hit_points <= 0)
	{
		input &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		dead = true;
	}

	if (bike->Flags)
		collide = 0;
	else
	{
		switch (lara_item->current_anim_state)
		{
		case QS_GETONL:
		case QS_GETONR:
		case QS_GETOFFL:
		case QS_GETOFFR:
			g_audio->stop_sound(155);
			g_audio->stop_sound(153);
			drive = -1;
			collide = 0;
			break;
		default:
			drive = UserControl(v, height, &pitch);
			break;
		}
	}

	if (bike->Velocity || bike->Revs)
	{
		bike->pitch = pitch;

		if (bike->pitch < -0x8000)	   bike->pitch = -0x8000;
		else if (bike->pitch > 0xa000) bike->pitch = 0xa000;

		g_audio->play_sound(155, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos }, 1.f + (float(bike->pitch) / 80960.f));
	}
	else
	{
		bike->pitch = 0;

		g_audio->stop_sound(155);
	}

	v->floor = height;

	int16_t rotadd = bike->Velocity >> 2;

	bike->RearRot -= rotadd;
	bike->RearRot -= (bike->Revs >> 3);
	bike->FrontRot -= rotadd;
	bike->left_fallspeed = DoDynamicsQB(hfl, bike->left_fallspeed, (int*)&fl.y);
	bike->right_fallspeed = DoDynamicsQB(hfr, bike->right_fallspeed, (int*)&fr.y);

	v->fallspeed = DoDynamicsQB(height, v->fallspeed, (int*)&v->pos.y_pos);

	height = (fl.y + fr.y) >> 1;

	int16_t x_rot = phd_atan(SKIDOO_FRONT, v->pos.y_pos - height),
		   z_rot = phd_atan(SKIDOO_SIDE, height - fl.y);

	v->pos.x_rot += (x_rot - v->pos.x_rot) >> 1;
	v->pos.z_rot += (z_rot - v->pos.z_rot) >> 1;

	if (!(bike->Flags & QF_DEAD))
	{
		if (room_number != v->room_number)
		{
			ItemNewRoom(lara.skidoo, room_number);
			ItemNewRoom(lara.item_number, room_number);
		}

		l->pos.x_pos = v->pos.x_pos;
		l->pos.y_pos = v->pos.y_pos;
		l->pos.z_pos = v->pos.z_pos;
		l->pos.y_rot = v->pos.y_rot;
		l->pos.x_rot = v->pos.x_rot;
		l->pos.z_rot = v->pos.z_rot;

		AnimateQuadBike(v, collide, dead);
		AnimateItem(l);

		v->anim_number = objects[QUADBIKE].anim_index + (l->anim_number - objects[quadbike_anim_obj].anim_index);
		v->frame_number = anims[v->anim_number].frame_base + (l->frame_number - anims[l->anim_number].frame_base);

		camera.target_elevation = -30 * ONE_DEGREE;

		if (bike->Flags & QF_FALLING)
		{
			if (v->pos.y_pos == v->floor)
			{
				ExplodingDeath(lara.item_number, 0xffffffff, 1);

				lara_item->hit_points = 0;
				lara_item->flags |= ONESHOT;

				QuadbikeExplode(v);

				return 0;
			}
		}
	}

	if (l->current_anim_state != QS_GETONR &&
		l->current_anim_state != QS_GETONL &&
		l->current_anim_state != QS_GETOFFR &&
		l->current_anim_state != QS_GETOFFL)
	{

		for (int i = 0; i < 2; ++i)
		{
			PHD_VECTOR pos { quad_bites[i].x, quad_bites[i].y, quad_bites[i].z };

			GetJointAbsPosition(v, &pos, quad_bites[i].mesh_num);

			if (int16_t angle = v->pos.y_rot + ((i == 0) ? 0x9000 : 0x7000);v->speed > 32)
			{
				if (v->speed < 64)
					TriggerExhaustSmoke(pos.x, pos.y, pos.z, angle, 64 - v->speed, 1);
			}
			else
			{
				int speed = 0;

				if (ExhaustStart < 16)					speed = ((ExhaustStart++ << 1) + (GetRandomControl() & 7) + (GetRandomControl() & 16)) << 7;
				else if (HandbrakeStarting)				speed = (abs(bike->Revs) >> 2) + ((GetRandomControl() & 7) << 7);
				else if ((GetRandomControl() & 3) == 0) speed = ((GetRandomControl() & 15) + (GetRandomControl() & 16)) << 7;

				TriggerExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 0);
			}
		}
	}
	else ExhaustStart = 0;

	return SkidooCheckGetOff();
}

void TriggerExhaustSmoke(long x, long y, long z, int16_t angle, long speed, long moving)
{
	auto sptr = &spark[GetFreeSpark()];

	int zv = (speed * phd_cos(angle)) >> (W2V_SHIFT + 2),
		xv = (speed * phd_sin(angle)) >> (W2V_SHIFT + 2),
		size = (GetRandomControl() & 7) + 32 + (speed >> 7);

	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = 96;
	sptr->dG = 96;
	sptr->dB = 128;

	if (moving)
	{
		sptr->dR = (sptr->dR * speed) >> 5;
		sptr->dG = (sptr->dG * speed) >> 5;
		sptr->dB = (sptr->dB * speed) >> 5;
	}

	sptr->ColFadeSpeed = 4;
	sptr->FadeToBlack = 4;
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20 - (speed >> 12);
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = xv + ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 7) - 8;
	sptr->Zvel = zv + ((GetRandomControl() & 255) - 128);
	sptr->Friction = 4;

	if (sptr->sLife < 9)
		sptr->sLife = sptr->Life = 9;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 7) - 24 : (GetRandomControl() & 7) + 24);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 2;
	sptr->Gravity = -(GetRandomControl() & 3) - 4;
	sptr->MaxYvel = -(GetRandomControl() & 7) - 8;
	sptr->Width = sptr->sWidth = size >> 1;
	sptr->dWidth = size;
	sptr->Height = sptr->sHeight = size >> 1;
	sptr->dHeight = size;
}