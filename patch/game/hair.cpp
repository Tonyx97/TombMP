#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "control.h"
#include "sphere.h"
#include "effect2.h"

#include <shared/game/math.h>

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#include <3dsystem/hwinsert.h>

int first_hair;

extern int g_wind_x, g_wind_z;

void InitialiseHair(vec3d* data, int_vec3* hair_vel)
{
	first_hair = 1;

	auto bone = objects[HAIR].bone_ptr;

	data[0].rot.y = 0;
	data[0].rot.x = -0x4000;

	for (int i = 1; i < HAIR_SEGMENTS + 1; ++i, bone += 4)
	{
		data[i].pos.x = *(bone + 1);
		data[i].pos.y = *(bone + 2);
		data[i].pos.z = *(bone + 3);
		data[i].rot.x = -0x4000;
		data[i].rot.y = data[i].rot.z = 0;
		hair_vel[i].x = hair_vel[i].y = hair_vel[i].z = 0;
	}
}

void HairControl(ITEM_INFO* item, vec3d* data, int_vec3* hair_vel)
{
	static int wind = 0;
	static int wind_angle = 2048, dwind_angle = 2048;

	auto object = &objects[LARA];
	auto objptr = lara.mesh_ptrs[HIPS];

	int16_t* frame;
	int32_t size;

	if (lara.hit_direction >= 0)
	{
		int16_t spaz;

		switch (lara.hit_direction)
		{
		case NORTH: spaz = (lara.is_ducked ? SPAZ_DUCKF_A : SPAZ_FORWARD_A); break;
		case SOUTH: spaz = (lara.is_ducked ? SPAZ_DUCKB_A : SPAZ_BACK_A);	 break;
		case EAST:	spaz = (lara.is_ducked ? SPAZ_DUCKR_A : SPAZ_RIGHT_A);	 break;
		default:	spaz = (lara.is_ducked ? SPAZ_DUCKL_A : SPAZ_LEFT_A);	 break;
		}

		size = anims[spaz].interpolation >> 8;
		frame = anims[spaz].frame_ptr + (int)(lara.hit_frame * size);
	}
	else frame = GetBestFrame(item);

	phd_PushUnitMatrix();

	*(phd_mxptr + M03) = item->pos.x_pos << W2V_SHIFT;
	*(phd_mxptr + M13) = item->pos.y_pos << W2V_SHIFT;
	*(phd_mxptr + M23) = item->pos.z_pos << W2V_SHIFT;

	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	auto rotation = frame + 9;

	auto bone = object->bone_ptr;

	phd_TranslateRel((int32_t)*(frame + 6), (int32_t)*(frame + 7), (int32_t)*(frame + 8));
	gar_RotYXZsuperpack(&rotation, 0);

	phd_PushMatrix();

	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));

	SPHERE sphere[5];

	sphere[0].x = *(phd_mxptr + M03) >> W2V_SHIFT;
	sphere[0].y = *(phd_mxptr + M13) >> W2V_SHIFT;
	sphere[0].z = *(phd_mxptr + M23) >> W2V_SHIFT;
	sphere[0].r = (int32_t)*(objptr + 3);

	phd_PopMatrix();

	phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));

	if (lara.weapon_item != NO_ITEM && lara.gun_type == LG_M16 &&
		 (items[lara.weapon_item].current_anim_state == 0 || items[lara.weapon_item].current_anim_state == 2 || items[lara.weapon_item].current_anim_state == 4))
	{
		rotation = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
		gar_RotYXZsuperpack(&rotation, 7);
	}
	else gar_RotYXZsuperpack(&rotation, 6);

	phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
	phd_PushMatrix();

	objptr = lara.mesh_ptrs[TORSO];

	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));

	sphere[1].x = *(phd_mxptr + M03) >> W2V_SHIFT;
	sphere[1].y = *(phd_mxptr + M13) >> W2V_SHIFT;
	sphere[1].z = *(phd_mxptr + M23) >> W2V_SHIFT;
	sphere[1].r = (int32_t)*(objptr + 3);

	phd_PopMatrix();

	phd_PushMatrix();
	phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
	gar_RotYXZsuperpack(&rotation, 0);

	objptr = lara.mesh_ptrs[UARM_R];

	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));

	sphere[3].x = *(phd_mxptr + M03) >> W2V_SHIFT;
	sphere[3].y = *(phd_mxptr + M13) >> W2V_SHIFT;
	sphere[3].z = *(phd_mxptr + M23) >> W2V_SHIFT;
	sphere[3].r = (int32_t)*(objptr + 3) * 3 / 2;

	phd_PopMatrix();

	phd_PushMatrix();
	phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
	gar_RotYXZsuperpack(&rotation, 2);

	objptr = lara.mesh_ptrs[UARM_R];

	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));

	sphere[4].x = *(phd_mxptr + M03) >> W2V_SHIFT;
	sphere[4].y = *(phd_mxptr + M13) >> W2V_SHIFT;
	sphere[4].z = *(phd_mxptr + M23) >> W2V_SHIFT;
	sphere[4].r = (int32_t)*(objptr + 3) * 3 / 2;

	phd_PopMatrix();

	phd_TranslateRel(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
	gar_RotYXZsuperpack(&rotation, 2);
	phd_RotYXZ(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);

	phd_PushMatrix();
	objptr = lara.mesh_ptrs[HEAD];
	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));

	sphere[2].x = *(phd_mxptr + M03) >> W2V_SHIFT;
	sphere[2].y = *(phd_mxptr + M13) >> W2V_SHIFT;
	sphere[2].z = *(phd_mxptr + M23) >> W2V_SHIFT;
	sphere[2].r = (int32_t)*(objptr + 3);

	phd_PopMatrix();

	phd_TranslateRel(0, -23, -55);

	PHD_VECTOR pos { *(phd_mxptr + M03) >> W2V_SHIFT, *(phd_mxptr + M13) >> W2V_SHIFT, *(phd_mxptr + M23) >> W2V_SHIFT };

	phd_PopMatrix();

	bone = objects[HAIR].bone_ptr;

	if (first_hair)
	{
		first_hair = 0;

		data[0].pos.x = pos.x;
		data[0].pos.y = pos.y;
		data[0].pos.z = pos.z;

		for (int i = 0; i < HAIR_SEGMENTS; ++i, bone += 4)
		{
			phd_PushUnitMatrix();
			{
				*(phd_mxptr + M03) = data[i].pos.x << W2V_SHIFT;
				*(phd_mxptr + M13) = data[i].pos.y << W2V_SHIFT;
				*(phd_mxptr + M23) = data[i].pos.z << W2V_SHIFT;

				phd_RotYXZ(data[i].rot.y, data[i].rot.x, 0);
				phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));

				data[i + 1].pos.x = *(phd_mxptr + M03) >> W2V_SHIFT;
				data[i + 1].pos.y = *(phd_mxptr + M13) >> W2V_SHIFT;
				data[i + 1].pos.z = *(phd_mxptr + M23) >> W2V_SHIFT;
			}
			phd_PopMatrix();
		}

		wind = g_wind_x = g_wind_z = 0;
		wind_angle = dwind_angle = 2048;
	}
	else
	{
		data[0].pos.x = pos.x;
		data[0].pos.y = pos.y;
		data[0].pos.z = pos.z;

		auto room_number = item->room_number;

		int x = item->pos.x_pos + (frame[0] + frame[1]) / 2,
			y = item->pos.y_pos + (frame[2] + frame[3]) / 2,
			z = item->pos.z_pos + (frame[4] + frame[5]) / 2,
			water_level = GetWaterHeight(x, y, z, room_number);

		int i = (GetRandomControl() & 7);

		wind += i - 3;

		if (wind <= -2)		++wind;
		else if (wind >= 9) --wind;

		dwind_angle += ((GetRandomControl() & 63) - 32) << 1;
		dwind_angle &= 8190;

		if (dwind_angle < 1024)
			dwind_angle += (1024 - dwind_angle) << 1;
		else if (dwind_angle > 3072)
			dwind_angle -= (dwind_angle - 3072) << 1;

		int diff = dwind_angle - wind_angle;

		wind_angle += diff >> 3;
		wind_angle &= 8190;

		g_wind_x = ((m_sin(wind_angle) * wind) >> 12);
		g_wind_z = ((m_cos(wind_angle) * wind) >> 12);

		for (int i = 1; i < HAIR_SEGMENTS + 1; ++i, bone += 4)
		{
			int height = GetHeight(GetFloor(data[i].pos.x, data[i].pos.y, data[i].pos.z, &room_number), data[i].pos.x, data[i].pos.y, data[i].pos.z);

			hair_vel[0].x = data[i].pos.x;
			hair_vel[0].y = data[i].pos.y;
			hair_vel[0].z = data[i].pos.z;

			data[i].pos.x += hair_vel[i].x * 3 / 4;
			data[i].pos.y += hair_vel[i].y * 3 / 4;
			data[i].pos.z += hair_vel[i].z * 3 / 4;

			if (lara.water_status == LARA_ABOVEWATER && room[room_number].flags & NOT_INSIDE)
			{
				data[i].pos.x += g_wind_x;
				data[i].pos.z += g_wind_z;
			}

			switch (lara.water_status)
			{
			case LARA_ABOVEWATER:
			{
				data[i].pos.y += 10;

				if (water_level != NO_HEIGHT && data[i].pos.y > water_level)
					data[i].pos.y = water_level;
				else if (data[i].pos.y > height)
				{
					data[i].pos.x = hair_vel[0].x;
					data[i].pos.z = hair_vel[0].z;
				}

				break;
			}
			case LARA_UNDERWATER:
			case LARA_SURFACE:
			case LARA_WADE:
			{
				if (data[i].pos.y < water_level) data[i].pos.y = water_level;
				else if (data[i].pos.y > height) data[i].pos.y = height;
			}
			}

			int distance = 0;

			for (int j = 0; j < 5; ++j)
			{
				int x = data[i].pos.x - sphere[j].x,
					y = data[i].pos.y - sphere[j].y,
					z = data[i].pos.z - sphere[j].z;

				distance = x * x + y * y + z * z;

				if (distance < SQUARE(sphere[j].r))
				{
					distance = phd_sqrt(distance);

					if (distance == 0)
						distance = 1;

					data[i].pos.x = sphere[j].x + x * sphere[j].r / distance;
					data[i].pos.y = sphere[j].y + y * sphere[j].r / distance;
					data[i].pos.z = sphere[j].z + z * sphere[j].r / distance;
				}
			}

			distance = phd_sqrt(SQUARE(data[i].pos.z - data[i - 1].pos.z) + SQUARE(data[i].pos.x - data[i - 1].pos.x));
			data[i - 1].rot.y = phd_atan((data[i].pos.z - data[i - 1].pos.z), (data[i].pos.x - data[i - 1].pos.x));
			data[i - 1].rot.x = -phd_atan(distance, data[i].pos.y - data[i - 1].pos.y);

			phd_PushUnitMatrix();
			{
				*(phd_mxptr + M03) = data[i - 1].pos.x << W2V_SHIFT;
				*(phd_mxptr + M13) = data[i - 1].pos.y << W2V_SHIFT;
				*(phd_mxptr + M23) = data[i - 1].pos.z << W2V_SHIFT;

				phd_RotYXZ(data[i - 1].rot.y, data[i - 1].rot.x, 0);

				if (i == HAIR_SEGMENTS)
					phd_TranslateRel(*(bone - 3), *(bone - 2), *(bone - 1));
				else phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));

				data[i].pos.x = *(phd_mxptr + M03) >> W2V_SHIFT;
				data[i].pos.y = *(phd_mxptr + M13) >> W2V_SHIFT;
				data[i].pos.z = *(phd_mxptr + M23) >> W2V_SHIFT;

				hair_vel[i].x = data[i].pos.x - hair_vel[0].x;
				hair_vel[i].y = data[i].pos.y - hair_vel[0].y;
				hair_vel[i].z = data[i].pos.z - hair_vel[0].z;
			}
			phd_PopMatrix();
		}
	}
}

void DrawHair(vec3d* data)
{
	auto object = &objects[HAIR];
	auto mesh = object->mesh_ptr;

	for (int i = 0; i < HAIR_SEGMENTS; ++i)
	{
		phd_PushMatrix();
		{
			phd_TranslateAbs(data[i].pos.x, data[i].pos.y, data[i].pos.z);
			phd_RotY(data[i].rot.y);
			phd_RotX(data[i].rot.x);
			phd_PutPolygons(*mesh++, 1);
		}
		phd_PopMatrix();
	}
}