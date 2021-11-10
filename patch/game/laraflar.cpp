#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "control.h"
#include "invfunc.h"
#include "effect2.h"
#include "laraflar.h"
#include "physics.h"

#include <specific/standard.h>
#include <specific/output.h>
#include <specific/litesrc.h>

int do_flare_light(PHD_VECTOR* pos, int flare_age)
{
	if (flare_age >= MAX_FLARE_AGE)
		return 0;

	int random = GetRandomControl(),
		x = pos->x + ((random & 0xf) << 3),
		y = pos->y + ((random & 0xf0) >> 1),
		z = pos->z + ((random & 0xf00) >> 5);

	if (flare_age < (FLARE_YOUNG >> 2))
	{
		int falloff = (flare_age << 2) + 4 + (random & 3);
		if (falloff > 16)
			falloff -= ((random >> 12) & 3);

		TriggerDynamicLight(x, y, z, falloff, (flare_age << 1) + 20 + (random & 3), flare_age + 4 + ((random >> 4) & 3), (flare_age << 1) + ((random >> 8) & 3));
	}
	else if (flare_age < FLARE_YOUNG)
		TriggerDynamicLight(x, y, z, flare_age + 2 + (random & 1), (flare_age >> 1) + 16 + (random & 7), (flare_age >> 1) + 8 + ((random >> 4) & 3), (flare_age >> 1) + 2 + ((random >> 8) & 3));
	else if (flare_age < FLARE_OLD)
		TriggerDynamicLight(x, y, z, 16, 24 + (random & 7), 16 + ((random >> 4) & 3), ((random >> 8) & 7) + (((random >> 6) & 16) >> 2));
	else if (flare_age < FLARE_DEAD)
	{
		if (random > 0x2000)
			TriggerDynamicLight(x, y, z, 16, 24 + (random & 7), 16 + ((random >> 4) & 3), ((random >> 8) & 7) + (((random >> 6) & 16) >> 2));
		else
		{
			TriggerDynamicLight(x, y, z, (GetRandomControl() & 6) + 8, (GetRandomControl() & 7) + 24, (GetRandomControl() & 7) + 8, GetRandomControl() & 15);

			return 0;
		}
	}
	else
	{
		TriggerDynamicLight(x, y, z, 16 - ((flare_age - FLARE_DEAD) >> 1), (GetRandomControl() & 7) + 24, (GetRandomControl() & 7) + 8, GetRandomControl() & 3);

		return (random & 1);
	}

	return 1;
}

void do_flare_in_hand(int flare_age)
{
	PHD_VECTOR pos { 11, 32, 41 };

	get_lara_bone_pos(lara_item, &pos, HAND_L);

	lara.left_arm.flash_gun = do_flare_light(&pos, flare_age);

	if (lara.flare_age < MAX_FLARE_AGE)
	{
		++lara.flare_age;

		if (room[lara_item->room_number].flags & UNDERWATER)
		{
			g_audio->play_sound(12, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

			if (GetRandomControl() < 0x4000)
				CreateBubble((PHD_3DPOS*)&pos, lara_item->room_number, FLARE_BUBBLESIZE, FLARE_BUBBLERANGE);
		}
		else g_audio->play_sound(12, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
	}
	else if (lara.gun_status == LG_ARMLESS)
		lara.gun_status = LG_UNDRAW;
}

void DrawFlareInAir(ITEM_INFO* item)
{
	int16_t* frmptr[2];
	int rate;

	GetFrames(item, frmptr, &rate);

	auto object = &objects[item->object_number];

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	if (int clip = S_GetObjectBounds(frmptr[0]))
	{
		CalculateObjectLighting(item, frmptr[0]);
		phd_PutPolygons(meshes[objects[FLARE_ITEM].mesh_index], clip);

		if ((int32_t)item->data & 0x8000)
		{
			phd_TranslateRel(-6, 6, 48);
			phd_RotX(-90 * ONE_DEGREE);
			S_CalculateStaticLight(8 * 256);
			phd_PushUnitMatrix();
			phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
			phd_PushMatrix();
			phd_TranslateRel(-6, 6, 32);
			
			int xv = item->pos.x_pos + (phd_mxptr[M03] >> W2V_SHIFT),
				yv = item->pos.y_pos + (phd_mxptr[M13] >> W2V_SHIFT),
				zv = item->pos.z_pos + (phd_mxptr[M23] >> W2V_SHIFT);

			phd_PopMatrix();
			phd_TranslateRel((GetRandomDraw() & 127) - 64, (GetRandomDraw() & 127) - 64, (GetRandomDraw() & 511) + 512);

			for (int i = 0; i < (GetRandomDraw() & 3) + 4; ++i)
				TriggerFlareSparks(xv, yv, zv, (phd_mxptr[M03] >> W2V_SHIFT), (phd_mxptr[M13] >> W2V_SHIFT), (phd_mxptr[M23] >> W2V_SHIFT), i >> 2);

			phd_PopMatrix();
		}
	}

	phd_PopMatrix();
}

void create_flare(int thrown)
{
	auto flare_item = CreateItem();

	if (flare_item != NO_ITEM)
	{
		auto flare = &items[flare_item];

		flare->object_number = FLARE_ITEM;
		flare->room_number = lara_item->room_number;

		PHD_VECTOR pos { -16, 32, 42 };

		get_lara_bone_pos(lara_item, &pos, HAND_L);

		auto floor = GetFloor(pos.x, pos.y, pos.z, &flare->room_number);

		if (GetHeight(floor, pos.x, pos.y, pos.z) < pos.y)
		{
			flare->pos.x_pos = lara_item->pos.x_pos;
			flare->pos.y_pos = pos.y;
			flare->pos.z_pos = lara_item->pos.z_pos;
			flare->pos.y_rot = lara_item->pos.y_rot;
			flare->room_number = lara_item->room_number;
		}
		else
		{
			flare->pos.x_pos = pos.x;
			flare->pos.y_pos = pos.y;
			flare->pos.z_pos = pos.z;
			flare->pos.y_rot = (thrown ? lara_item->pos.y_rot : lara_item->pos.y_rot - 0x2000);
		}

		InitialiseItem(flare_item);

		flare->pos.z_rot = 0;
		flare->pos.x_rot = 0;
		flare->shade = -1;

		if (thrown)
		{
			flare->speed = lara_item->speed + 50;
			flare->fallspeed = lara_item->fallspeed - 50;
		}
		else
		{
			flare->speed = lara_item->speed + 10;
			flare->fallspeed = lara_item->fallspeed + 50;
		}

		flare->data = (do_flare_light((PHD_VECTOR*)&flare->pos, lara.flare_age) ? (void*)(lara.flare_age | 0x8000)
																			    : (void*)(lara.flare_age & 0x7fff));

		AddActiveItem(flare_item);

		flare->status = ACTIVE;
	}
}

void set_flare_arm(int frame)
{
	int anim_base = objects[FLARE].anim_index;

	if (frame < FL_THROW_F)			lara.left_arm.anim_number = anim_base;
	else if (frame < FL_DRAW_F)		lara.left_arm.anim_number = anim_base + 1;
	else if (frame < FL_IGNITE_F)	lara.left_arm.anim_number = anim_base + 2;
	else if (frame < FL_2HOLD_F)	lara.left_arm.anim_number = anim_base + 3;
	else							lara.left_arm.anim_number = anim_base + 4;

	lara.left_arm.frame_base = anims[lara.left_arm.anim_number].frame_ptr;
}

void draw_flare()
{
	if (lara_item->current_anim_state == AS_FLAREPICKUP || lara_item->current_anim_state == AS_PICKUP)
	{
		do_flare_in_hand(lara.flare_age);

		lara.flare_control_left = 0;
		lara.left_arm.frame_number = FL_2HOLD_F - 2;
		set_flare_arm(lara.left_arm.frame_number);

		return;
	}

	auto ani = lara.left_arm.frame_number + 1;

	lara.flare_control_left = 1;

	if (ani < FL_DRAW_F || ani > FL_2HOLD_F - 1)
		ani = FL_DRAW_F;
	else if (ani == FL_DRAWGOTIT_F)
		draw_flare_meshes();
	else if (ani >= FL_IGNITE_F && ani <= (FL_2HOLD_F - 2))
	{
		if (ani == FL_IGNITE_F)
		{
			g_audio->play_sound(115);
			g_audio->play_sound(11, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

			lara.flare_age = 0;
		}

		do_flare_in_hand(lara.flare_age);
	}
	else if (ani == (FL_2HOLD_F - 1))
	{
		ready_flare();

		ani = FL_HOLD_F;

		do_flare_in_hand(lara.flare_age);
	}

	lara.left_arm.frame_number = ani;
	set_flare_arm(lara.left_arm.frame_number);
}

void undraw_flare()
{
	auto ani = lara.left_arm.frame_number,
		 ani2 = lara.flare_frame;

	lara.flare_control_left = 1;

	if (lara_item->goal_anim_state == AS_STOP && lara.skidoo == NO_ITEM)
	{
		if (lara_item->anim_number == BREATH_A)
		{
			lara_item->anim_number = THROWFLARE_A;
			lara_item->frame_number = lara.flare_frame = ani2 = THROWFLARE_F + ani;
		}

		if (lara_item->anim_number == THROWFLARE_A)
		{
			lara.flare_control_left = 0;

			if (ani2 >= THROWFLARE_F + FL_THROW_FRAMES - 1)
			{
				ani2 = 0;

				lara.gun_type = lara.request_gun_type = lara.last_gun_type;
				lara.gun_status = LG_ARMLESS;

				InitialiseNewWeapon();

				lara.target = nullptr;
				lara.left_arm.lock = lara.right_arm.lock = 0;
				lara_item->anim_number = STOP_A;
				lara_item->frame_number = lara.flare_frame = STOP_F;
				lara_item->current_anim_state = AS_STOP;
				lara_item->goal_anim_state = AS_STOP;

				return;
			}

			lara.flare_frame = ++ani2;
		}
	}
	else
	{
		if (lara_item->current_anim_state == AS_STOP && lara.skidoo == NO_ITEM)
		{
			lara_item->anim_number = STOP_A;
			lara_item->frame_number = STOP_F;
		}
	}

	if (ani == FL_HOLD_F)
		ani = FL_THROW_F;
	else if (ani >= FL_IGNITE_F && ani < FL_IGNITE_F + 23)
	{
		if (++ani == FL_IGNITE_F + FL_IGNITE_FT - 1)
			ani = FL_THROW_F;
	}
	else if (ani >= FL_THROW_F && ani < FL_DRAW_F)
	{
		if (++ani == FL_THROWRELEASE_F)
		{
			create_flare(1);
			undraw_flare_meshes();
		}
		else if (ani == FL_DRAW_F)
		{
			ani = 0;

			lara.gun_type = lara.request_gun_type = lara.last_gun_type;
			lara.gun_status = LG_ARMLESS;

			InitialiseNewWeapon();

			lara.target = nullptr;
			lara.left_arm.lock = lara.right_arm.lock = 0;
			lara.flare_control_left = 0;
			lara.flare_frame = 0;
		}
	}
	else if (ani >= FL_2HOLD_F && ani < FL_END_F)
	{
		if (++ani == FL_END_F)
			ani = FL_THROW_F;
	}

	if (ani >= FL_THROW_F && ani < FL_THROWRELEASE_F)
		do_flare_in_hand(lara.flare_age);

	lara.left_arm.frame_number = ani;
	set_flare_arm(lara.left_arm.frame_number);
}

void draw_flare_meshes()
{
	lara.mesh_ptrs[HAND_L] = meshes[objects[FLARE].mesh_index + HAND_L];
}

void undraw_flare_meshes()
{
	lara.mesh_ptrs[HAND_L] = meshes[objects[LARA].mesh_index + HAND_L];
}

void ready_flare()
{
	lara.gun_status = LG_ARMLESS;
	lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
	lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
	lara.left_arm.lock = lara.right_arm.lock = 0;
	lara.target = nullptr;
}

void FlareControl(int16_t item_number)
{
	auto flare = &items[item_number];

	if (room[flare->room_number].flags & SWAMP)
	{
		KillItem(item_number);
		return;
	}

	if (flare->fallspeed)
	{
		flare->pos.x_rot += (ONE_DEGREE * 3);
		flare->pos.z_rot += (ONE_DEGREE * 5);
	}
	else
	{
		flare->pos.x_rot = 0;
		flare->pos.z_rot = 0;
	}

	int x = flare->pos.x_pos,
		y = flare->pos.y_pos,
		z = flare->pos.z_pos,
		xv = (flare->speed * phd_sin(flare->pos.y_rot) >> W2V_SHIFT),
		yv = 0,
		zv = (flare->speed * phd_cos(flare->pos.y_rot) >> W2V_SHIFT);

	flare->pos.x_pos += xv;
	flare->pos.z_pos += zv;

	if (room[flare->room_number].flags & UNDERWATER)
	{
		flare->fallspeed += (5 - flare->fallspeed) / 2;
		flare->speed += (5 - flare->speed) / 2;
	}
	else flare->fallspeed += GRAVITY;

	flare->pos.y_pos += (yv = flare->fallspeed);

	DoProperDetection(item_number, x, y, z, xv, yv, zv);

	int flare_age = (int32_t)flare->data & 0x7fff;

	if (flare_age < MAX_FLARE_AGE)
		++flare_age;
	else if (flare->fallspeed == 0 && flare->speed == 0)
	{
		KillItem(item_number);
		return;
	}

	if (do_flare_light((PHD_VECTOR*)&flare->pos, flare_age))
	{
		flare_age |= 0x8000;

		if (room[flare->room_number].flags & UNDERWATER)
		{
			g_audio->play_sound(12, { flare->pos.x_pos, flare->pos.y_pos, flare->pos.z_pos });

			if (GetRandomControl() < 0x4000)
				CreateBubble(&flare->pos, flare->room_number, FLARE_BUBBLESIZE, FLARE_BUBBLERANGE);
		}
		else g_audio->play_sound(12, { flare->pos.x_pos, flare->pos.y_pos, flare->pos.z_pos });
	}

	flare->data = (void*)flare_age;
}