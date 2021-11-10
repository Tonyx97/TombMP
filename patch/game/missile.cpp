import utils;

#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "control.h"
#include "effect2.h"
#include "traps.h"
#include "physics.h"

#define KNIFE_DAMAGE			50
#define DIVER_HARPOON_DAMAGE	50
#define MISSILE_RANGE			SQUARE(WALL_L/6)
#define ROCKET_RANGE			SQUARE(WALL_L)

void ControlMissile(int16_t fx_number)
{
	auto fx = &effects[fx_number];

	if (fx->object_number == DIVER_HARPOON && !(room[fx->room_number].flags & UNDERWATER) && fx->pos.x_rot > -0x3000)
		fx->pos.x_rot -= ONE_DEGREE;

	int speed = fx->speed * phd_cos(fx->pos.x_rot) >> W2V_SHIFT;

	fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot) >> W2V_SHIFT);
	fx->pos.y_pos += (fx->speed * phd_sin(-fx->pos.x_rot) >> W2V_SHIFT);
	fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot) >> W2V_SHIFT);

	auto room_number = fx->room_number;
	auto floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

	if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) ||
		fx->pos.y_pos <= GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos))
	{
		if (fx->object_number == KNIFE || fx->object_number == DIVER_HARPOON)
			g_audio->play_sound((fx->object_number == DIVER_HARPOON) ? 10 : 258, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });
		else if (fx->object_number == DRAGON_FIRE)
		{
			TriggerFireFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1, 0);
			TriggerDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 24, 31, 24, GetRandomControl() & 7);
			KillEffect(fx_number);
		}

		return;
	}

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, room_number);

	if (fx->object_number == DRAGON_FIRE)
	{
		if (ItemNearLara(lara_item, &fx->pos, 350))
		{
			lara_item->hit_points -= 3;
			lara_item->hit_status = 1;

			LaraBurn();

			return;
		}
	}
	else if (ItemNearLara(lara_item, &fx->pos, 200))
	{
		if (fx->object_number == KNIFE)
		{
			lara_item->hit_points -= KNIFE_DAMAGE;

			g_audio->play_sound(317, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });

			KillEffect(fx_number);
		}
		else if (fx->object_number == DIVER_HARPOON)
		{
			lara_item->hit_points -= DIVER_HARPOON_DAMAGE;

			g_audio->play_sound(317, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });

			KillEffect(fx_number);
		}
		lara_item->hit_status = 1;

		fx->pos.y_rot = lara_item->pos.y_rot;
		fx->speed = lara_item->speed;
		fx->frame_number = fx->counter = 0;
	}

	if (fx->object_number == DIVER_HARPOON && room[fx->room_number].flags & UNDERWATER)
	{
		if ((wibble & 15) == 0)
			CreateBubble(&fx->pos, fx->room_number, 8, 8);

		TriggerRocketSmoke(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 64);
	}
	else if (fx->object_number == DRAGON_FIRE && !fx->counter--)
	{
		TriggerDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 24, 31, 24, GetRandomControl() & 7);
		g_audio->play_sound(305, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });
		KillEffect(fx_number);
	}
	else if (fx->object_number == KNIFE)
		fx->pos.z_rot += 30 * ONE_DEGREE;
}

void ShootAtLara(FX_INFO* fx)
{
	int x = lara_item->pos.x_pos - fx->pos.x_pos,
		y = lara_item->pos.y_pos - fx->pos.y_pos,
		z = lara_item->pos.z_pos - fx->pos.z_pos,
		distance = phd_sqrt(SQUARE(x) + SQUARE(z));

	auto bounds = GetBoundsAccurate(lara_item);

	y += bounds[3] + (bounds[2] - bounds[3]) * 3 / 4;

	fx->pos.x_rot = -phd_atan(distance, y);
	fx->pos.y_rot = phd_atan(z, x);

	fx->pos.x_rot += (GetRandomControl() - 0x4000) / 0x40;
	fx->pos.y_rot += (GetRandomControl() - 0x4000) / 0x40;
}

int ExplodingDeath(int16_t item_number, int32_t mesh_bits, int16_t damage)
{
	auto item = &items[item_number];
	auto object = &objects[item->object_number];
	auto frame = GetBestFrame(item);

	phd_PushUnitMatrix();

	*(phd_mxptr + M03) = 0;
	*(phd_mxptr + M13) = 0;
	*(phd_mxptr + M23) = 0;

	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	phd_TranslateRel((int32_t)*(frame + 6), (int32_t)*(frame + 7), (int32_t)*(frame + 8));

	auto rotation = frame + 9;

	gar_RotYXZsuperpack(&rotation, 0);

	auto bone = object->bone_ptr;
	auto extra_rotation = (int16_t*)item->data;

	int bit = 1;

	if ((bit & mesh_bits) && (bit & item->mesh_bits))
	{
		if (item->object_number == SMASH_WINDOW ||
			item->object_number == SMASH_OBJECT1 ||
			item->object_number == SMASH_OBJECT2 ||
			item->object_number == SMASH_OBJECT3)
		{
			if (auto fx_number = CreateEffect(item->room_number); fx_number != NO_ITEM)
			{
				auto fx = &effects[fx_number];

				fx->pos.x_pos = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
				fx->pos.y_pos = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
				fx->pos.z_pos = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;
				fx->room_number = item->room_number;
				fx->pos.y_rot = (GetRandomControl() - 0x4000) << 1;
				fx->speed = (int16_t)GetRandomControl() >> 8;
				fx->fallspeed = (int16_t)-GetRandomControl() >> 8;
				fx->flag3 = 0;

				if (item->object_number == SMASH_WINDOW ||
					item->object_number == SMASH_OBJECT1 ||
					item->object_number == SMASH_OBJECT2 ||
					item->object_number == SMASH_OBJECT3 ||
					item->object_number == MUTANT2 ||
					item->object_number == QUADBIKE)
					fx->counter = 0;
				else fx->counter = (damage << 2) | (GetRandomControl() & 3);

				fx->frame_number = object->mesh_ptr;
				fx->object_number = BODY_PART;
			}

			item->mesh_bits -= bit;
		}
	}

	for (int i = 1; i < object->nmeshes; ++i, bone += 3)
	{
		int poppush = *(bone++);

		if (poppush & 1) phd_PopMatrix();
		if (poppush & 2) phd_PushMatrix();

		phd_TranslateRel(*(bone), *(bone + 1), *(bone + 2));
		gar_RotYXZsuperpack(&rotation, 0);

		if (extra_rotation && (poppush & (ROT_X | ROT_Y | ROT_Z)))
		{
			if (poppush & ROT_Y) phd_RotY(*(extra_rotation++));
			if (poppush & ROT_X) phd_RotX(*(extra_rotation++));
			if (poppush & ROT_Z) phd_RotZ(*(extra_rotation++));
		}

		bit <<= 1;

		if ((bit & mesh_bits) && (bit & item->mesh_bits))
		{
			if (auto fx_number = CreateEffect(item->room_number); fx_number != NO_ITEM)
			{
				auto fx = &effects[fx_number];

				fx->pos.x_pos = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
				fx->pos.y_pos = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
				fx->pos.z_pos = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;
				fx->room_number = item->room_number;
				fx->pos.y_rot = (GetRandomControl() - 0x4000) << 1;
				fx->speed = -(50 + int16_t(utils::mt() % 100));
				fx->fallspeed = -(50 + int16_t(utils::mt() % 100));

				if (item->object_number == SMASH_WINDOW ||
					item->object_number == SMASH_OBJECT1 ||
					item->object_number == SMASH_OBJECT2 ||
					item->object_number == SMASH_OBJECT3 ||
					item->object_number == MUTANT2 ||
					item->object_number == QUADBIKE)
					fx->counter = 0;
				else
				{
					fx->flag3 = 1;
					fx->counter = (damage << 2) | (GetRandomControl() & 3);
				}

				fx->frame_number = object->mesh_ptr + i;
				fx->object_number = BODY_PART;
				fx->shade = 0x4210;
			}

			item->mesh_bits -= bit;
		}
	}

	phd_PopMatrix();

	return !((0x7fffffff >> (31 - object->nmeshes)) & item->mesh_bits);
}

void ControlBodyPart(int16_t fx_number)
{
	auto fx = &effects[fx_number];

	fx->pos.x_rot += (ONE_DEGREE * 5);
	fx->pos.z_rot += (ONE_DEGREE * 10);
	fx->pos.z_pos += (fx->speed * phd_cos(fx->pos.y_rot) >> W2V_SHIFT) >> 2;
	fx->pos.x_pos += (fx->speed * phd_sin(fx->pos.y_rot) >> W2V_SHIFT) >> 2;
	fx->fallspeed += GRAVITY >> 1;
	fx->pos.y_pos += fx->fallspeed;

	/*if ((wibble & 12) == 0)
	{
		if (fx->counter & 1) TriggerFireFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx_number, 0);
		if (fx->counter & 2) TriggerFireSmoke(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1, 0);
	}*/

	auto room_number = fx->room_number;
	auto floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
	auto ceiling = GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);

	const bool is_alive = fx->flag3;

	if (is_alive)
		DoBloodSplatEx(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->speed, fx->pos.y_rot, room_number, 1);

	if (fx->pos.y_pos < ceiling)
	{
		fx->fallspeed = -fx->fallspeed;
		fx->pos.y_pos = ceiling;
	}

	int height = GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);

	if (fx->pos.y_pos >= height)
	{
		if (is_alive && (fx->counter & 3))
			g_audio->play_sound(AUDIO_FOOTSTEPS_MUD, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });

		KillEffect(fx_number);

		return;
	}

	if (ItemNearLara(lara_item, &fx->pos, (fx->counter & (~3))))
	{
		lara_item->hit_points -= fx->counter >> 2;
		lara_item->hit_status = 1;

		if (is_alive && (fx->counter & 3))
		{
			g_audio->play_sound(AUDIO_FOOTSTEPS_MUD, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });

			KillEffect(fx_number);
		}
		else KillEffect(fx_number);
	}

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, room_number);
}