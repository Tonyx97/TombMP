import prof;

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "laraswim.h"
#include "control.h"
#include "camera.h"
#include "sphere.h"
#include "traps.h"
#include "laraflar.h"
#include "lot.h"
#include "effect2.h"
#include "text.h"
#include "biggun.h"
#include "game.h"

#include <specific/standard.h>
#include <specific/input.h>
#include <specific/output.h>
#include <specific/init.h>

#include <mp/client.h>

#include <mp/game/level.h>

#define GF2(a)				(anims[objects[BIGGUN].anim_index + a].frame_base)
#define BGUN_GETON_A		0
#define BGUN_GETOFF_A		1
#define BGUN_UPDOWN_A		2
#define BGUN_RECOIL_A		3
#define BGUN_GETON_F		GF2(BGUN_GETON_A)
#define BGUN_GETOFF_F		GF2(BGUN_GETOFF_A)
#define BGUN_UPDOWN_F		GF2(BGUN_UPDOWN_A)
#define BGUN_RECOIL_F		GF2(BGUN_RECOIL_A)
#define RECOIL_TIME			26
#define RECOIL_START		20
#define RECOIL_Z			96
#define ROCKET_SPEED		512
#define BGF_UPDOWN			1
#define BGF_AUTOROT			2
#define BGF_GETOFF			4
#define BGF_FIRE			8
#define BGUN_GETON_NFRAMES	48
#define BGUN_GETOFF_NFRAMES	12
#define BGUN_UPDOWN_NFRAMES	59
#define BGUN_RECOIL_NFRAMES	20

enum
{
	BGUNS_GETON,
	BGUNS_GETOFF,
	BGUNS_UPDOWN,
	BGUNS_RECOIL
};

PHD_VECTOR PooPos = { 0, 0, 0 };

void FireBigGun(ITEM_INFO* v)
{
	if (auto item_number = CreateItem(); item_number != NO_ITEM)
	{
		auto item = &items[item_number];
		auto gun = (BIGGUNINFO*)v->data;

		item->object_number = ROCKET;
		item->room_number = lara_item->room_number;

		PHD_VECTOR pos { 0, 0, 256 };

		GetJointAbsPosition(v, &pos, 2);

		item->pos.x_pos = pos.x;
		item->pos.y_pos = pos.y;
		item->pos.z_pos = pos.z;

		InitialiseItem(item_number);

		item->pos.x_rot = -((gun->RotX - 32) * (ONE_DEGREE));
		item->pos.y_rot = v->pos.y_rot;
		item->pos.z_rot = 0;
		item->speed = ROCKET_SPEED >> 5;
		item->item_flags[0] = 1;

		gns::projectile::create info
		{
			.vec =
			{
				.pos = { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos },
				.rot = { item->pos.x_rot, item->pos.y_rot, item->pos.z_rot },
				.room = item->room_number
			},

			.obj = item->object_number,
			.speed = item->speed,
			.fallspeed = item->fallspeed,
			.health = item->hit_points,
			.flags0 = 1,
		};

		g_client->send_packet(ID_PROJECTILE_CREATE, info);

		AddActiveItem(item_number);

		g_audio->play_sound(77, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

		smoke_count_l = 32;
		smoke_weapon = LG_ROCKET;

		for (int i = 0; i < 5; ++i)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, LG_ROCKET, 32);

		phd_PushUnitMatrix();

		*(phd_mxptr + M03) = 0;
		*(phd_mxptr + M13) = 0;
		*(phd_mxptr + M23) = 0;

		phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
		phd_PushMatrix();
		phd_TranslateRel(0, 0, -128);

		int wx = (*(phd_mxptr + M03) >> W2V_SHIFT),
			wy = (*(phd_mxptr + M13) >> W2V_SHIFT),
			wz = (*(phd_mxptr + M23) >> W2V_SHIFT);

		phd_PopMatrix();

		for (int i = 0; i < 8; ++i)
		{
			int xv, yv, zv;

			phd_PushMatrix();
			{
				phd_TranslateRel(0, 0, -(GetRandomControl() & 2047));

				xv = (*(phd_mxptr + M03) >> W2V_SHIFT);
				yv = (*(phd_mxptr + M13) >> W2V_SHIFT);
				zv = (*(phd_mxptr + M23) >> W2V_SHIFT);
			}
			phd_PopMatrix();

			TriggerRocketFlame(wx, wy, wz, xv - wx, yv - wy, zv - wz, item_number);
		}

		phd_PopMatrix();
	}
}

bool CanUseGun(ITEM_INFO* v, ITEM_INFO* l)
{
	if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
		return 0;

	int x = l->pos.x_pos - v->pos.x_pos,
		z = l->pos.z_pos - v->pos.z_pos,
		dist = (x * x) + (z * z);

	return (dist > 0 && dist <= 30000);
}

void BigGunInitialise(int16_t item_number)
{
	auto v = &items[item_number];
	auto gun = (BIGGUNINFO*)game_malloc(sizeof(BIGGUNINFO), 0);

	v->data = gun;

	gun->Flags = gun->FireCount = 0;
	gun->RotX = 30;
	gun->StartRotY = v->pos.y_rot;
}

void BigGunCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
		return;

	auto v = &items[item_number];

	if (CanUseGun(v, l))
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

		l->pos.x_pos = v->pos.x_pos;
		l->pos.y_pos = v->pos.y_pos;
		l->pos.z_pos = v->pos.z_pos;
		l->pos.x_rot = v->pos.x_rot;
		l->pos.y_rot = v->pos.y_rot;
		l->pos.z_rot = v->pos.z_rot;
		l->anim_number = objects[biggun_anim_obj].anim_index + BGUN_GETON_A;
		l->frame_number = BGUN_GETON_F;
		l->current_anim_state = l->goal_anim_state = BGUNS_GETON;

		auto gun = (BIGGUNINFO*)v->data;

		gun->Flags = 0;
		gun->RotX = 30;
	}
	else ObjectCollision(item_number, l, coll);
}

void BigGunDraw(ITEM_INFO* v)
{
	int16_t* frmptr[2];
	int rate;

	auto frac = GetFrames(v, frmptr, &rate);
	auto object = &objects[v->object_number];

	phd_PushMatrix();
	phd_TranslateAbs(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
	phd_RotYXZ(v->pos.y_rot, v->pos.x_rot, v->pos.z_rot);

	if (auto clip = S_GetObjectBounds(frmptr[0]))
	{
		auto gun = (BIGGUNINFO*)v->data;
		auto meshpp = object->mesh_ptr;
		auto bone = object->bone_ptr; 

		CalculateObjectLighting(v, frmptr[0]);

		if (!frac)
		{
			phd_TranslateRel((int32_t)*(frmptr[0] + 6), (int32_t)*(frmptr[0] + 7), (int32_t)*(frmptr[0] + 8));

			auto rotation = frmptr[0] + 9;

			gar_RotYXZsuperpack(&rotation, 0);
			phd_PutPolygons(*meshpp++, clip);

			phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
			gar_RotYXZsuperpack(&rotation, 0);
			phd_PutPolygons(*meshpp++, clip);

			phd_TranslateRel(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
			gar_RotYXZsuperpack(&rotation, 0);
			phd_PutPolygons(*meshpp++, clip);
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

			phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
			gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
			phd_PutPolygons_I(*meshpp++, clip);

			phd_TranslateRel_I(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
			gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
			phd_PutPolygons_I(*meshpp++, clip);
		}
	}

	phd_PopMatrix();
}

int BigGunControl(COLL_INFO* coll)
{
	auto l = lara_item;
	auto v = &items[lara.skidoo];
	auto gun = (BIGGUNINFO*)v->data;

	if (gun->Flags & BGF_UPDOWN)
	{
		if ((input & IN_ACTION) && gun->FireCount == 0)
		{
			FireBigGun(v);

			gun->Flags = BGF_FIRE;
			gun->FireCount = RECOIL_TIME;
		}

		if ((l->hit_points <= 0) || (input & IN_ROLL))
			gun->Flags = BGF_AUTOROT;
		else
		{
			PHD_ANGLE add = 0;

			if (input & IN_LEFT)
			{
				add -= 3;

				if ((wibble & 7) == 0)
					g_audio->play_sound(44, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });
			}
			else if (input & IN_RIGHT)
			{
				add += 3;

				if ((wibble & 7) == 0)
					g_audio->play_sound(44, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });
			}

			auto new_rot_y = PHD_ANGLE(v->get_rotation_y() / ONE_DEGREE + add);

			if (new_rot_y < -180)		new_rot_y = 180;
			else if (new_rot_y > 180)	new_rot_y = -180;

			new_rot_y *= ONE_DEGREE;

			v->set_rotation_y(new_rot_y);

			if ((input & IN_FORWARD) && gun->RotX < BGUN_UPDOWN_NFRAMES)
				++gun->RotX;
			else if ((input & IN_BACK) && gun->RotX)
				--gun->RotX;
		}
	}

	if (gun->Flags & BGF_AUTOROT)
	{
		if (gun->RotX < 30)
			++gun->RotX;
		else if (gun->RotX > 30)
			--gun->RotX;
		else
		{
			l->anim_number = objects[biggun_anim_obj].anim_index + BGUN_GETOFF_A;
			l->frame_number = BGUN_GETOFF_F;
			l->current_anim_state = l->goal_anim_state = BGUNS_GETOFF;

			gun->Flags = BGF_GETOFF;
		}
	}

	switch (l->current_anim_state)
	{
	case BGUNS_GETON:
	case BGUNS_GETOFF:
	{
		AnimateItem(l);

		v->anim_number = objects[BIGGUN].anim_index + (l->anim_number - objects[biggun_anim_obj].anim_index);
		v->frame_number = anims[v->anim_number].frame_base + (l->frame_number - anims[l->anim_number].frame_base);

		if ((gun->Flags & BGF_GETOFF) && l->frame_number == anims[l->anim_number].frame_end)
		{
			l->anim_number = STOP_A;
			l->frame_number = STOP_F;
			l->current_anim_state = l->goal_anim_state = AS_STOP;

			lara.skidoo = NO_ITEM;
			lara.gun_status = LG_ARMLESS;

			if (auto entity = g_level->get_entity_by_item(v))
				g_level->request_entity_ownership(entity, false);
		}

		break;
	}
	case BGUNS_UPDOWN:
	{
		l->anim_number = objects[biggun_anim_obj].anim_index + BGUN_UPDOWN_A;
		l->frame_number = BGUN_UPDOWN_F + gun->RotX;

		v->anim_number = objects[BIGGUN].anim_index + (l->anim_number - objects[biggun_anim_obj].anim_index);
		v->frame_number = anims[v->anim_number].frame_base + (l->frame_number - anims[l->anim_number].frame_base);

		if (gun->FireCount)
			--gun->FireCount;

		gun->Flags = BGF_UPDOWN;
	}
	}

	l->pos.x_pos = v->pos.x_pos;
	l->pos.y_pos = v->pos.y_pos;
	l->pos.z_pos = v->pos.z_pos;
	l->pos.x_rot = v->pos.x_rot;
	l->pos.y_rot = v->pos.y_rot;
	l->pos.z_rot = v->pos.z_rot;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	LaraBaddieCollision(l, coll);

	camera.target_elevation = -15 * ONE_DEGREE;

	return 1;
}