#include <specific/standard.h>
#include <specific/global.h>
#include <specific/input.h>
#include <specific/fn_stubs.h>

#include "objects.h"
#include "laraanim.h"
#include "laraswim.h"
#include "control.h"
#include "camera.h"
#include "sphere.h"
#include "laraflar.h"
#include "effect2.h"
#include "sub.h"
#include "game.h"

#include <specific/init.h>
#include <specific/output.h>

#include <mp/game/level.h>

#define GF2(a)				(anims[objects[UPV].anim_index + a].frame_base)
#define SUB_DEATH1_A		0
#define SUB_DEATH2_A		0
#define SUB_POSE_A			5
#define SUB_POSE_F			GF2(SUB_POSE_A)
#define SUB_GETOFFSURF_A	8
#define SUB_GETOFFSURF_F	GF2(SUB_GETOFFSURF_A)
#define SUB_GETOFFSURF1_A	9
#define SUB_GETOFFSURF1_F	GF2(SUB_GETOFFSURF1_A)
#define SUB_GETONSURF_A		10
#define SUB_GETONSURF_F		GF2(SUB_GETONSURF_A)
#define SUB_GETONSURF1_A	11
#define SUB_GETOFF_A		12
#define SUB_GETOFF_F  		GF2(SUB_GETOFF_A)
#define SUB_GETON_A			13
#define SUB_GETON_F			GF2(SUB_GETON_A)
#define UWDEATH_A			124
#define UWDEATH_F			GF(UWDEATH_A, 17)
#define NUM_WAKE_SPRITES	32
#define WAKE_SIZE 			32
#define WAKE_SPEED 			4
#define SUB_DRAW_SHIFT		128
#define SF_CONTROL			1
#define SF_SURFACE			2
#define SF_DIVE				4
#define SF_DEAD				8
#define ACCELERATION		0x50000
#define FRICTION			0x18000
#define MAX_SPEED			0x600000
#define ROT_ACCELERATION	0x400000
#define ROT_SLOWACCEL		0x200000
#define ROT_FRICTION 		0x100000
#define MAX_ROTATION		0x1c00000
#define UPDOWN_ACCEL		((ONE_DEGREE * 2) << 16)
#define UPDOWN_SLOWACCEL	(((ONE_DEGREE * 3) / 2) << 16)
#define UPDOWN_FRICTION		((ONE_DEGREE / 1) << 16)
#define MAX_UPDOWN			((ONE_DEGREE * 2) << 16)
#define UPDOWN_LIMIT		(ONE_DEGREE * 80)
#define UPDOWN_SPEED		10
#define SURFACE_DIST		210
#define SURFACE_ANGLE		(ONE_DEGREE * 50)
#define DIVE_ANGLE			(ONE_DEGREE * 15)
#define DIVE_SPEED			(ONE_DEGREE * 5)
#define SUB_RADIUS			300
#define SUB_HEIGHT			400
#define SUB_LENGTH			1024
#define FRONT_TOLERANCE		((45 * ONE_DEGREE) << 16)
#define TOP_TOLERANCE		((45 * ONE_DEGREE) << 16)
#define WALLDEFLECT			((2 * ONE_DEGREE) << 16)
#define GETOFF_DIST 		1024
#define HARPOON_SPEED		256
#define HARPOON_TIME		256
#define HARPOON_RELOAD		15

enum
{
	SUBS_DIE,
	SUBS_HIT,
	SUBS_GETOFFS,
	SUBS_1,
	SUBS_MOVE,
	SUBS_POSE,
	SUBS_2,
	SUBS_3,
	SUBS_GETON,
	SUBS_GETOFF
};

enum
{
	SUB_FAN,
	SUB_FRONT_LIGHT,
	SUB_LEFT_FIN_LEFT,
	SUB_LEFT_FIN_RIGHT,
	SUB_RIGHT_FIN_RIGHT,
	SUB_RIGHT_FIN_LEFT
};

struct WAKE_PTS
{
	int32_t 	x[2];
	int32_t 	y[2];
	int32_t	z[2];
	uint8_t 	life;
	uint8_t	pad[3];
};

WAKE_PTS SubWakePts[NUM_WAKE_SPRITES][2];
uint8_t SubCurrentStartWake = 0;
uint8_t SubWakeShade = 0;

BITE_INFO sub_bites[6] =
{
	{ 0,	0,	0,	 3 },	// Fan.
	{ 0,	96, 256, 0 },	// Front light.
	{ -128, 0, -64,	 1 },	// Left Fin Left.
	{ 0,	0, -64,	 1 },	// Left Fin right.
	{ 128,	0, -64,  2 },	// Right Fin Right.
	{ 0,	0, -64,  2 }	// Right fin left.
};

void TriggerSubMist(long x, long y, long z, long speed, int16_t angle);

void DoWake(ITEM_INFO* v, int16_t leftright)
{
	if (!TriggerActive(v) || SubWakePts[SubCurrentStartWake][leftright].life)
		return;

	SubWakePts[SubCurrentStartWake][leftright].life = 0x20;

	for (int i = 0; i < 2; ++i)
	{
		PHD_VECTOR pos
		{
			sub_bites[2 + (leftright << 1) + i].x,
			sub_bites[2 + (leftright << 1) + i].y,
			sub_bites[2 + (leftright << 1) + i].z
		};

		GetJointAbsPosition(v, &pos, sub_bites[2 + (leftright << 1) + i].mesh_num);

		SubWakePts[SubCurrentStartWake][leftright].x[i] = pos.x;
		SubWakePts[SubCurrentStartWake][leftright].y[i] = pos.y + SUB_DRAW_SHIFT;
		SubWakePts[SubCurrentStartWake][leftright].z[i] = pos.z;
	}

	if (leftright == 1)
	{
		++SubCurrentStartWake;
		SubCurrentStartWake &= (NUM_WAKE_SPRITES - 1);
	}
}

void UpdateWakeFX()
{
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < NUM_WAKE_SPRITES; ++j)
			if (SubWakePts[j][i].life)
				--SubWakePts[j][i].life;
}

void FireSubHarpoon(ITEM_INFO* v)
{
	if (auto item_number = CreateItem(); item_number != NO_ITEM)
	{
		static char lr = 0;

		auto item = &items[item_number];

		item->object_number = HARPOON_BOLT;
		item->shade = int16_t(0x4210 | 0x8000);
		item->room_number = v->room_number;

		PHD_VECTOR pos { (lr) ? 22 : -22, 24, 230 };

		GetJointAbsPosition(v, &pos, 3);

		item->pos.x_pos = pos.x;
		item->pos.y_pos = pos.y;
		item->pos.z_pos = pos.z;

		InitialiseItem(item_number);

		item->pos.x_rot = v->pos.x_rot;
		item->pos.y_rot = v->pos.y_rot;
		item->pos.z_rot = 0;
		item->fallspeed = (int16_t)(-HARPOON_SPEED * phd_sin(item->pos.x_rot) >> W2V_SHIFT);
		item->speed = (int16_t)(HARPOON_SPEED * phd_cos(item->pos.x_rot) >> W2V_SHIFT);
		item->hit_points = HARPOON_TIME;
		item->item_flags[0] = 1;

		AddActiveItem(item_number);

		g_audio->play_sound(23, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

		--lara.harpoon.ammo;

		lr ^= 1;
	}
}

int CanGetOff(ITEM_INFO* v)
{
	if (lara.current_xvel || lara.current_zvel)
		return 0;

	auto yangle = v->pos.y_rot + 0x8000;

	int speed = (GETOFF_DIST * phd_cos(v->pos.x_rot)) >> W2V_SHIFT,
		x = v->pos.x_pos + (speed * phd_sin(yangle) >> W2V_SHIFT),
		z = v->pos.z_pos + (speed * phd_cos(yangle) >> W2V_SHIFT),
		y = v->pos.y_pos - ((GETOFF_DIST * phd_sin(-v->pos.x_rot)) >> W2V_SHIFT);

	auto room_number = v->room_number;
	auto floor = GetFloor(x, y, z, &room_number);

	int height = GetHeight(floor, x, y, z);
	if (height == NO_HEIGHT || y > height)
		return 0;

	int ceiling = GetCeiling(floor, x, y, z);

	return (height - ceiling >= 256 && y >= ceiling && ceiling != NO_HEIGHT);
}

int GetOnSub(int16_t item_number, COLL_INFO* coll)
{
	auto l = lara_item;
	auto v = &items[item_number];

	if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
		return 0;

	int y = abs(l->pos.y_pos - (v->pos.y_pos - 128));
	if (y > 256)
		return 0;

	int x = l->pos.x_pos - v->pos.x_pos,
		z = l->pos.z_pos - v->pos.z_pos,
		dist = (x * x) + (z * z);

	if (dist > SQUARE(512))
		return 0;

	auto room_number = v->room_number;
	auto floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	return (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) >= -32000);
}

void DoCurrent(ITEM_INFO* item)
{
	if (!lara.current_active)
	{
		int absvel = abs(lara.current_xvel),
			shifter = 2;

		if (absvel > 16)	 shifter = 4;
		else if (absvel > 8) shifter = 3;

		lara.current_xvel -= lara.current_xvel >> shifter;

		if (abs(lara.current_xvel) < 4)
			lara.current_xvel = 0;

		absvel = abs(lara.current_zvel);

		if (absvel > 16)	 shifter = 4;
		else if (absvel > 8) shifter = 3;
		else				 shifter = 2;

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
			speed = camera.fixed[sinkval].data,
			dx = (m_sin(angle << 1) * speed) >> 2,
			dz = (m_cos(angle << 1) * speed) >> 2;

		lara.current_xvel += (dx - lara.current_xvel) >> 4;
		lara.current_zvel += (dz - lara.current_zvel) >> 4;
	}

	item->pos.x_pos += lara.current_xvel >> 8;
	item->pos.z_pos += lara.current_zvel >> 8;

	lara.current_active = 0;
}

void BackgroundCollision(ITEM_INFO* v, ITEM_INFO* l, SUBINFO* sub)
{
	COLL_INFO cinfo,
			* coll = &cinfo;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -SUB_HEIGHT;
	coll->bad_ceiling = SUB_HEIGHT;
	coll->old.x = v->pos.x_pos;
	coll->old.y = v->pos.y_pos;
	coll->old.z = v->pos.z_pos;
	coll->radius = SUB_RADIUS;
	coll->trigger = NULL;
	coll->slopes_are_walls = 0;
	coll->slopes_are_pits = 0;
	coll->lava_is_pit = 0;
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 1;
	coll->facing = (v->pos.x_rot >= -16384 && v->pos.x_rot <= 16384 ? lara.move_angle = v->pos.y_rot
																	: lara.move_angle = v->pos.y_rot - 32768);

	int height = phd_sin(v->pos.x_rot) * SUB_LENGTH >> W2V_SHIFT;

	if (height < 0)   height = -height;
	if (height < 200) height = 200;

	coll->bad_neg = -height;

	GetCollisionInfo(coll, v->pos.x_pos, v->pos.y_pos + height / 2, v->pos.z_pos, v->room_number, height);
	ShiftItem(v, coll);

	if (coll->coll_type == COLL_FRONT)
	{
		if (sub->RotX > FRONT_TOLERANCE)
			sub->RotX += WALLDEFLECT;
		else if (sub->RotX < -FRONT_TOLERANCE)
			sub->RotX -= WALLDEFLECT;
		else sub->Vel = 0;
	}
	else if (coll->coll_type == COLL_TOP)
	{
		if (sub->RotX >= -TOP_TOLERANCE)
			sub->RotX -= WALLDEFLECT;
	}
	else if (coll->coll_type == COLL_TOPFRONT)
		sub->Vel = 0;
	else if (coll->coll_type == COLL_LEFT)
		v->pos.y_rot += 5 * ONE_DEGREE;
	else if (coll->coll_type == COLL_RIGHT)
		v->pos.y_rot -= 5 * ONE_DEGREE;
	else if (coll->coll_type == COLL_CLAMP)
	{
		v->pos.x_pos = coll->old.x;
		v->pos.y_pos = coll->old.y;
		v->pos.z_pos = coll->old.z;
		sub->Vel = 0;

		return;
	}

	if (coll->mid_floor < 0)
	{
		v->pos.y_pos += coll->mid_floor;
		sub->RotX += WALLDEFLECT;
	}
}

void UserInput(ITEM_INFO* v, ITEM_INFO* l, SUBINFO* sub)
{
	CanGetOff(v);

	int16_t anim = l->anim_number - objects[sub_anim_obj].anim_index,
		   frame = l->frame_number - anims[l->anim_number].frame_base;

	bool getting_off = false;

	switch (l->current_anim_state)
	{
	case SUBS_MOVE:
	{
		if (l->hit_points > 0)
		{
			if (input & IN_LEFT)	   sub->Rot -= ROT_ACCELERATION;
			else if (input & IN_RIGHT) sub->Rot += ROT_ACCELERATION;

			if (sub->Flags & SF_SURFACE)
			{
				if (v->pos.x_rot > SURFACE_ANGLE)	   v->pos.x_rot -= ONE_DEGREE;
				else if (v->pos.x_rot < SURFACE_ANGLE) v->pos.x_rot += ONE_DEGREE;
			}
			else
			{
				if (input & IN_FORWARD)   sub->RotX -= UPDOWN_ACCEL;
				else if (input & IN_BACK) sub->RotX += UPDOWN_ACCEL;
			}

			if (input & IN_JUMP)
			{
				if ((sub->Flags & SF_SURFACE) && (input & IN_FORWARD) && v->pos.x_rot > -DIVE_ANGLE)
					sub->Flags |= SF_DIVE;

				sub->Vel += ACCELERATION;
			}
			else l->goal_anim_state = SUBS_POSE;
		}

		break;
	}
	case SUBS_POSE:
	{
		if (l->hit_points > 0)
		{
			if (input & IN_LEFT)	   sub->Rot -= ROT_SLOWACCEL;
			else if (input & IN_RIGHT) sub->Rot += ROT_SLOWACCEL;

			if (sub->Flags & SF_SURFACE)
			{
				if (v->pos.x_rot > SURFACE_ANGLE)	   v->pos.x_rot -= ONE_DEGREE;
				else if (v->pos.x_rot < SURFACE_ANGLE) v->pos.x_rot += ONE_DEGREE;
			}
			else
			{
				if (input & IN_FORWARD)   sub->RotX -= UPDOWN_ACCEL;
				else if (input & IN_BACK) sub->RotX += UPDOWN_ACCEL;
			}

			if ((input & IN_ROLL) && CanGetOff(v))
			{
				l->goal_anim_state = ((sub->Flags & SF_SURFACE) ? SUBS_GETOFFS : SUBS_GETOFF);

				sub->Flags &= ~SF_CONTROL;

				g_audio->stop_sound(346);
				g_audio->play_sound(348, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });
			}
			else if (input & IN_JUMP)
			{
				if ((sub->Flags & SF_SURFACE) && (input & IN_FORWARD) && v->pos.x_rot > -DIVE_ANGLE)
					sub->Flags |= SF_DIVE;

				l->goal_anim_state = SUBS_MOVE;
			}
		}
		else l->goal_anim_state = SUBS_DIE;

		break;
	}
	case SUBS_GETON:
	{
		if (anim == SUB_GETONSURF1_A)
		{
			v->pos.y_pos += 4;
			v->pos.x_rot += (ONE_DEGREE);

			if (frame == 30) g_audio->play_sound(347, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });
			if (frame == 50) sub->Flags |= SF_CONTROL;
		}

		else if (anim == SUB_GETON_A)
		{
			if (frame == 30) g_audio->play_sound(347, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });
			if (frame == 42) sub->Flags |= SF_CONTROL;
		}

		break;
	}
	case SUBS_GETOFF:
	{
		if (anim == SUB_GETOFF_A && frame == 42)
		{
			PHD_VECTOR vec {};

			get_lara_bone_pos(lara_item, &vec, HIPS);

			GAME_VECTOR VPos { vec.x, vec.y, vec.z, v->room_number },
						LPos { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, v->room_number };

			mgLOS(&VPos, &LPos, 0);

			l->pos.x_pos = LPos.x;
			l->pos.y_pos = LPos.y;
			l->pos.z_pos = LPos.z;
			l->anim_number = TREAD_A;
			l->frame_number = TREAD_F;
			l->current_anim_state = AS_TREAD;
			l->fallspeed = 0;
			l->gravity_status = 0;
			l->pos.x_rot = l->pos.z_rot = 0;

			UpdateLaraRoom(l, 0);

			lara.water_status = LARA_UNDERWATER;
			lara.gun_status = LG_ARMLESS;
			lara.skidoo = NO_ITEM;

			v->hit_points = 0;

			getting_off = true;
		}

		break;
	}
	case SUBS_GETOFFS:
	{
		if (anim == SUB_GETOFFSURF1_A && frame == 51)
		{
			PHD_VECTOR vec{};

			int wd = GetWaterDepth(l->pos.x_pos, l->pos.y_pos, l->pos.z_pos, l->room_number),
				wh = GetWaterHeight(l->pos.x_pos, l->pos.y_pos, l->pos.z_pos, l->room_number),
				hfw = (wh != NO_HEIGHT ? l->pos.y_pos - wh : NO_HEIGHT);

			get_lara_bone_pos(lara_item, &vec, HIPS);

			l->pos.x_pos = vec.x;
			l->pos.y_pos = l->pos.y_pos + 1 - hfw;
			l->pos.y_pos = vec.y;
			l->pos.z_pos = vec.z;
			l->anim_number = SURFTREAD_A;
			l->frame_number = SURFTREAD_F;
			l->current_anim_state = l->goal_anim_state = AS_SURFTREAD;
			l->fallspeed = 0;
			l->gravity_status = 0;
			l->pos.x_rot = l->pos.z_rot = 0;

			UpdateLaraRoom(l, -LARA_HITE / 2);

			lara.water_status = LARA_SURFACE;
			lara.water_surface_dist = -hfw;
			lara.dive_count = DIVE_COUNT + 1;
			lara.torso_x_rot = lara.torso_y_rot = 0;
			lara.head_x_rot = lara.head_y_rot = 0;
			lara.gun_status = LG_ARMLESS;
			lara.skidoo = NO_ITEM;

			v->hit_points = 0;

			getting_off = true;
		}

		break;
	}
	case SUBS_DIE:
	{
		if ((anim == SUB_DEATH1_A && frame == 16) || (anim == SUB_DEATH2_A && frame == 17))
		{
			PHD_VECTOR vec {};

			get_lara_bone_pos(lara_item, &vec, HIPS);

			l->pos.x_pos = vec.x;
			l->pos.y_pos = vec.y;
			l->pos.z_pos = vec.z;
			l->anim_number = UWDEATH_A;
			l->frame_number = UWDEATH_F;
			l->current_anim_state = l->goal_anim_state = AS_UWDEATH;
			l->fallspeed = 0;
			l->gravity_status = 0;
			l->pos.x_rot = l->pos.z_rot = 0;

			sub->Flags |= SF_DEAD;
		}

		v->speed = 0;
	}
	}

	if (sub->Flags & SF_DIVE)
	{
		if (v->pos.x_rot > -DIVE_ANGLE)
			v->pos.x_rot -= DIVE_SPEED;
		else sub->Flags &= ~SF_DIVE;
	}

	if (sub->Vel > 0)
	{
		if ((sub->Vel -= FRICTION) < 0)
			sub->Vel = 0;
	}
	else if (sub->Vel < 0)
	{
		if ((sub->Vel += FRICTION) > 0)
			sub->Vel = 0;
	}

	if (sub->Vel > MAX_SPEED)		sub->Vel = MAX_SPEED;
	else if (sub->Vel < -MAX_SPEED) sub->Vel = -MAX_SPEED;

	if (sub->Rot > 0)
	{
		if ((sub->Rot -= ROT_FRICTION) < 0)
			sub->Rot = 0;
	}
	else if (sub->Rot < 0)
	{
		if ((sub->Rot += ROT_FRICTION) > 0)
			sub->Rot = 0;
	}

	if (sub->RotX > 0)
	{
		if ((sub->RotX -= UPDOWN_FRICTION) < 0)
			sub->RotX = 0;
	}
	else if (sub->RotX < 0)
	{
		if ((sub->RotX += UPDOWN_FRICTION) > 0)
			sub->RotX = 0;
	}

	if (sub->Rot > MAX_ROTATION)		sub->Rot = MAX_ROTATION;
	else if (sub->Rot < -MAX_ROTATION)	sub->Rot = -MAX_ROTATION;
	if (sub->RotX > MAX_UPDOWN)			sub->RotX = MAX_UPDOWN;
	else if (sub->RotX < -MAX_UPDOWN)	sub->RotX = -MAX_UPDOWN;

	if (getting_off)
		if (auto entity = g_level->get_entity_by_item(v))
			g_level->request_entity_ownership(entity, false);
}

void SubInitialise(int16_t item_number)
{
	auto v = &items[item_number];

	v->data = game_malloc(sizeof(SUBINFO), 0);

	auto sub = (SUBINFO*)v->data;

	sub->Vel = sub->Rot = 0;
	sub->Flags = SF_SURFACE;
	sub->WeaponTimer = 0;

	for (int i = 0; i < NUM_WAKE_SPRITES; ++i)
	{
		SubWakePts[i][0].life = 0;
		SubWakePts[i][1].life = 0;
	}
}

void SubCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
		return;

	auto v = &items[item_number];

	if (int geton = GetOnSub(item_number, coll))
	{
		if (auto entity = g_level->get_entity_by_item(v))
			if (!g_level->is_entity_streamed(entity))
			{
				v->pos.y_pos += SUB_DRAW_SHIFT;

				ObjectCollision(item_number, l, coll);

				v->pos.y_pos -= SUB_DRAW_SHIFT;

				return g_level->request_entity_ownership(entity, true);
			}

		lara.skidoo = item_number;
		lara.water_status = LARA_ABOVEWATER;

		if (lara.gun_type == LG_FLARE)
		{
			create_flare(0);
			undraw_flare_meshes();

			lara.flare_control_left = 0;
			lara.request_gun_type = lara.gun_type = LG_ARMLESS;
		}

		lara.gun_status = LG_HANDSBUSY;

		v->hit_points = 1;
		l->pos.x_pos = v->pos.x_pos;
		l->pos.y_pos = v->pos.y_pos;
		l->pos.z_pos = v->pos.z_pos;
		l->pos.y_rot = v->pos.y_rot;

		if (l->current_anim_state == AS_SURFTREAD || l->current_anim_state == AS_SURFSWIM)
		{
			l->anim_number = objects[sub_anim_obj].anim_index + SUB_GETONSURF_A;
			l->frame_number = SUB_GETONSURF_F;
			l->current_anim_state = l->goal_anim_state = SUBS_GETON;
		}
		else
		{
			l->anim_number = objects[sub_anim_obj].anim_index + SUB_GETON_A;
			l->frame_number = SUB_GETON_F;
			l->current_anim_state = l->goal_anim_state = SUBS_GETON;
		}

		AnimateItem(l);
	}
	else
	{
		v->pos.y_pos += SUB_DRAW_SHIFT;

		ObjectCollisionSub(item_number, l, coll);

		v->pos.y_pos -= SUB_DRAW_SHIFT;
	}
}

void SubDraw(ITEM_INFO* v)
{
	int16_t* frmptr[2] = { nullptr, nullptr };

	int rate,
		frac = GetFrames(v, frmptr, &rate);

	if (!frmptr[0] || !frmptr[1])
		return;

	auto object = &objects[v->object_number];

	S_PrintShadow(256, frmptr[0], v, 0);

	phd_PushMatrix();
	phd_TranslateAbs(v->pos.x_pos, v->pos.y_pos + SUB_DRAW_SHIFT, v->pos.z_pos);
	phd_RotYXZ(v->pos.y_rot, v->pos.x_rot, v->pos.z_rot);

	if (int clip = S_GetObjectBounds(frmptr[0]))
	{
		SUBINFO* sub = (SUBINFO*)v->data;

		CalculateObjectLighting(v, frmptr[0]);

		auto meshpp = object->mesh_ptr;
		auto bone = object->bone_ptr;

		if (!frac)
		{
			phd_TranslateRel((int32_t)*(frmptr[0] + 6), (int32_t)*(frmptr[0] + 7), (int32_t)*(frmptr[0] + 8));

			auto rotation = frmptr[0] + 9;

			gar_RotYXZsuperpack(&rotation, 0);
			phd_PutPolygons(*meshpp++, clip);

			phd_PushMatrix();
			phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
			gar_RotYXZsuperpack(&rotation, 0);
			phd_RotX(v->pos.z_rot + (sub->RotX >> 13));
			phd_PutPolygons(*meshpp++, clip);
			phd_PopMatrix();

			phd_PushMatrix();
			phd_TranslateRel(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
			gar_RotYXZsuperpack(&rotation, 0);
			phd_RotX(-v->pos.z_rot + (sub->RotX >> 13));
			phd_PutPolygons(*meshpp++, clip);
			phd_PopMatrix();

			phd_PushMatrix();
			phd_TranslateRel(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
			gar_RotYXZsuperpack(&rotation, 0);
			phd_RotZ(sub->FanRot);
			phd_PutPolygons(*meshpp++, clip);
			phd_PopMatrix();
		}
		else
		{
			InitInterpolate(frac, rate);

			auto rotation = frmptr[0] + 9,
				 rotation2 = frmptr[1] + 9;

			phd_TranslateRel_ID(
				(int32_t)*(frmptr[0] + 6),
				(int32_t)*(frmptr[0] + 7),
				(int32_t)*(frmptr[0] + 8),
				(int32_t)*(frmptr[1] + 6),
				(int32_t)*(frmptr[1] + 7),
				(int32_t)*(frmptr[1] + 8));

			gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
			phd_PutPolygons_I(*meshpp++, clip);

			phd_PushMatrix_I();
			phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
			gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
			phd_RotX_I(v->pos.z_rot + (sub->RotX >> 13));
			InterpolateMatrix();
			phd_PutPolygons_I(*meshpp++, clip);
			phd_PopMatrix_I();

			phd_PushMatrix_I();
			phd_TranslateRel_I(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
			gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
			phd_RotX_I(-v->pos.z_rot + (sub->RotX >> 13));
			phd_PutPolygons_I(*meshpp++, clip);
			phd_PopMatrix_I();

			phd_PushMatrix_I();
			phd_TranslateRel_I(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
			gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
			phd_RotZ_I(sub->FanRot);
			phd_PutPolygons_I(*meshpp++, clip);
			phd_PopMatrix_I();
		}
	}

	phd_PopMatrix();
}

int SubControl()
{
	auto l = lara_item;
	auto v = &items[lara.skidoo];
	auto sub = (SUBINFO*)v->data;

	if (!(sub->Flags & SF_DEAD))
	{
		UserInput(v, l, sub);

		v->speed = sub->Vel >> 16;
		v->pos.x_rot += sub->RotX >> 16;
		v->pos.y_rot += (sub->Rot >> 16);
		v->pos.z_rot = (sub->Rot >> 12);

		if (v->pos.x_rot > UPDOWN_LIMIT)	   v->pos.x_rot = UPDOWN_LIMIT;
		else if (v->pos.x_rot < -UPDOWN_LIMIT) v->pos.x_rot = -UPDOWN_LIMIT;

		v->pos.x_pos += (((phd_sin(v->pos.y_rot) * v->speed) >> W2V_SHIFT) * phd_cos(v->pos.x_rot)) >> W2V_SHIFT;
		v->pos.y_pos -= (phd_sin(v->pos.x_rot) * v->speed) >> W2V_SHIFT;
		v->pos.z_pos += (((phd_cos(v->pos.y_rot) * v->speed) >> W2V_SHIFT) * phd_cos(v->pos.x_rot)) >> W2V_SHIFT;
	}

	auto room_number = v->room_number;
	auto floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);

	v->floor = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);

	if ((sub->Flags & SF_CONTROL) && !(sub->Flags & SF_DEAD))
	{
		int h = GetWaterHeight(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, room_number);

		if ((h != NO_HEIGHT) && (!(room[v->room_number].flags & UNDERWATER)))
		{
			if ((h - v->pos.y_pos) >= -SURFACE_DIST)
				v->pos.y_pos = h + SURFACE_DIST;

			if (!(sub->Flags & SF_SURFACE))
			{
				g_audio->play_sound(36, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				sub->Flags &= ~SF_DIVE;
			}

			sub->Flags |= SF_SURFACE;
		}
		else if ((h != NO_HEIGHT) && ((h - v->pos.y_pos) >= -SURFACE_DIST))
		{
			v->pos.y_pos = h + SURFACE_DIST;

			if (!(sub->Flags & SF_SURFACE))
			{
				g_audio->play_sound(36, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				sub->Flags &= ~SF_DIVE;
			}

			sub->Flags |= SF_SURFACE;
		}
		else sub->Flags &= ~SF_SURFACE;

		if (!(sub->Flags & SF_SURFACE))
		{
			if (l->hit_points >= 0)
			{
				if (--lara.air < 0)
				{
					lara.air = -1;

					l->hit_points -= 5;
				}
			}
		}
		else
		{
			if (l->hit_points >= 0)
			{
				lara.air += 10;

				if (lara.air > LARA_AIR)
					lara.air = LARA_AIR;
			}
		}
	}

	TestTriggers(trigger_index, 0);

	if (lara.skidoo != NO_ITEM)
		SubEffects(lara.skidoo);

	if (lara.skidoo != NO_ITEM && !(sub->Flags & SF_DEAD))
	{
		DoCurrent(v);

		if ((input & IN_ACTION) && (sub->Flags & SF_CONTROL) && !sub->WeaponTimer)
		{
			if (lara.harpoon.ammo > 0)
			{
				FireSubHarpoon(v);

				sub->WeaponTimer = HARPOON_RELOAD;
			}
		}

		if (room_number != v->room_number)
		{
			ItemNewRoom(lara.skidoo, room_number);
			ItemNewRoom(lara.item_number, room_number);
		}

		l->pos.x_pos = v->pos.x_pos;
		l->pos.y_pos = v->pos.y_pos + SUB_DRAW_SHIFT;
		l->pos.z_pos = v->pos.z_pos;
		l->pos.x_rot = v->pos.x_rot;
		l->pos.y_rot = v->pos.y_rot;
		l->pos.z_rot = v->pos.z_rot;

		AnimateItem(l);
		BackgroundCollision(v, l, sub);

		if (sub->Flags & SF_CONTROL)
			g_audio->play_sound(346, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });

		v->anim_number = objects[UPV].anim_index + (l->anim_number - objects[sub_anim_obj].anim_index);
		v->frame_number = anims[v->anim_number].frame_base + (l->frame_number - anims[l->anim_number].frame_base);

		camera.target_elevation = ((sub->Flags & SF_SURFACE) ? -60 * ONE_DEGREE : 0);

		return 1;
	}
	else if (sub->Flags & SF_DEAD)
	{
		AnimateItem(l);

		if (room_number != v->room_number)
			ItemNewRoom(lara.skidoo, room_number);

		BackgroundCollision(v, l, sub);

		sub->RotX = 0;

		v->anim_number = SUB_POSE_A;
		v->frame_number = SUB_POSE_F;
		v->goal_anim_state = v->current_anim_state = SUBS_POSE;
		v->fallspeed = 0;
		v->speed = 0;
		v->gravity_status = 1;

		AnimateItem(v);

		return 1;
	}

	return 0;
}

void SubEffects(int16_t item_number)
{
	auto v = &items[item_number];
	auto sub = (SUBINFO*)v->data;

	if (lara.skidoo == item_number)
	{
		if (!sub->Vel)
			sub->FanRot += (ONE_DEGREE * 2);
		else sub->FanRot += sub->Vel >> 12;

		if (sub->Vel)
		{
			PHD_VECTOR pos { sub_bites[SUB_FAN].x, sub_bites[SUB_FAN].y, sub_bites[SUB_FAN].z };

			GetJointAbsPosition(v, &pos, sub_bites[SUB_FAN].mesh_num);

			TriggerSubMist(pos.x, pos.y + SUB_DRAW_SHIFT, pos.z, abs(sub->Vel) >> 16, v->pos.y_rot + 0x8000);

			if ((GetRandomControl() & 1) == 0)
			{
				PHD_3DPOS pos3d { pos.x + (GetRandomControl() & 63) - 32, pos.y + SUB_DRAW_SHIFT, pos.z + (GetRandomControl() & 63) - 32 };

				auto room_number = v->room_number;

				GetFloor(pos3d.x_pos, pos3d.y_pos, pos3d.z_pos, &room_number);
				CreateBubble(&pos3d, room_number, 4, 8);
			}
		}
	}

	GAME_VECTOR	source;

	for (int i = 0; i < 2; ++i)
	{
		PHD_VECTOR pos { sub_bites[SUB_FRONT_LIGHT].x,sub_bites[SUB_FRONT_LIGHT].y, sub_bites[SUB_FRONT_LIGHT].z << (i * 6) };

		GetJointAbsPosition(v, &pos, sub_bites[SUB_FRONT_LIGHT].mesh_num);

		if (i == 1)
		{
			GAME_VECTOR target { pos.x, pos.y, pos.z, v->room_number };

			LOS(&source, &target);

			pos.x = target.x;
			pos.y = target.y;
			pos.z = target.z;
		}
		else source = { pos.x, pos.y, pos.z, v->room_number };

		int r = 31 - (GetRandomControl() & 3);

		TriggerDynamicLight(pos.x, pos.y, pos.z, 16 + (i << 3), r, r, r);
	}

	if (!(wibble & 15) && sub->Vel)
	{
		DoWake(v, 0);
		DoWake(v, 1);
	}

	if (sub->Vel == 0)
	{
		if (SubWakeShade)
			--SubWakeShade;
	}
	else if (SubWakeShade < 16)
		++SubWakeShade;

	if (sub->WeaponTimer)
		--sub->WeaponTimer;

	UpdateWakeFX();
}

void TriggerSubMist(long x, long y, long z, long speed, int16_t angle)
{
	auto sptr = &spark[GetFreeSpark()];

	int zv = (speed * phd_cos(angle)) >> (W2V_SHIFT + 2),
		xv = (speed * phd_sin(angle)) >> (W2V_SHIFT + 2),
		size = (GetRandomControl() & 7) + (speed >> 1) + 16;;

	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = 64;
	sptr->dG = 64;
	sptr->dB = 64;
	sptr->ColFadeSpeed = 4 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 12;
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = xv + ((GetRandomControl() & 127) - 64);
	sptr->Yvel = 0;
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
	sptr->Scalar = 3;
	sptr->Gravity = sptr->MaxYvel = 0;
	sptr->Width = sptr->sWidth = size >> 2;
	sptr->Height = sptr->sHeight = size >> 2;
	sptr->dWidth = size;
	sptr->dHeight = size;
}