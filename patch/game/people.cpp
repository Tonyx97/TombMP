#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "control.h"
#include "sphere.h"
#include "effect2.h"
#include "game.h"

#define PEOPLE_SHOOT_RANGE SQUARE(WALL_L*8)
#define PEOPLE_HIT_CHANCE 0x2000
#define PEOPLE_TARGETING_SPEED 300
#define TARGET_HEIGHT_OFFSET (STEP_L*2)
#define LOS_START_HEIGHT_OFFSET (STEP_L*3)
#define LOS_TARGET_HEIGHT_OFFSET (STEP_L*2)

bool TargetVisible(ITEM_INFO* item, AI_INFO* info)
{
	auto creature = (CREATURE_INFO*)item->data;
	auto enemy = creature->enemy;

	if (!enemy || enemy->hit_points <= 0 || !enemy->data)
		return false;

	if (int true_angle = info->angle - creature->joint_rotation[2];
		true_angle > -FRONT_ARC && true_angle < FRONT_ARC && info->distance < PEOPLE_SHOOT_RANGE)
	{
		auto bounds = GetBestFrame(enemy);

		GAME_VECTOR start  { item->pos.x_pos, item->pos.y_pos - LOS_START_HEIGHT_OFFSET, item->pos.z_pos, item->room_number },
					target { enemy->pos.x_pos, enemy->pos.y_pos + (int32_t)(((*(bounds + 2) << 1) + *(bounds + 2) + *(bounds + 3)) >> 2), enemy->pos.z_pos };

		return !!LOS(&start, &target);
	}

	return false;
}

bool Targetable(ITEM_INFO* item, AI_INFO* info)
{
	auto creature = (CREATURE_INFO*)item->data;
	auto enemy = creature->enemy;

	if (!enemy || enemy->hit_points <= 0 || !enemy->data)
		return false;

	if (info->ahead && info->distance < PEOPLE_SHOOT_RANGE)
	{
		auto bounds = GetBestFrame(item);

		GAME_VECTOR start { item->pos.x_pos, item->pos.y_pos + (int32_t)(((*(bounds + 2) << 1) + *(bounds + 2) + *(bounds + 3)) >> 2), item->pos.z_pos, item->room_number };

		bounds = GetBestFrame(enemy);

		GAME_VECTOR target { enemy->pos.x_pos, enemy->pos.y_pos + (int32_t)(((*(bounds + 2) << 1) + *(bounds + 2) + *(bounds + 3)) >> 2), enemy->pos.z_pos };

		return !!LOS(&start, &target);
	}

	return false;
}

int16_t GunShot(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number)
{
	return NO_ITEM;
	auto fx_number = CreateEffect(room_number);

	if (fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		fx->pos.x_pos = x;
		fx->pos.y_pos = y;
		fx->pos.z_pos = z;
		fx->room_number = room_number;
		fx->pos.x_rot = fx->pos.z_rot = 0;
		fx->pos.y_rot = yrot;
		fx->counter = 3;
		fx->frame_number = 0;
		fx->object_number = GUN_FLASH;
		fx->shade = 16 * 256;
	}

	return fx_number;
}

int16_t GunHit(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number)
{
	PHD_VECTOR pos {};

	GetJointAbsPosition(lara_item, &pos, GetRandomControl() * 25 / 0x7fff);
	DoBloodSplat(pos.x, pos.y, pos.z, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number);

	g_audio->play_sound(50, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

	return GunShot(x, y, z, speed, yrot, room_number);
}

int16_t GunMiss(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number)
{
	GAME_VECTOR pos
	{
		lara_item->pos.x_pos + (GetRandomControl() - 0x4000) * (WALL_L / 2) / 0x7fff,
		lara_item->floor,
		lara_item->pos.z_pos + (GetRandomControl() - 0x4000) * (WALL_L / 2) / 0x7fff,
		lara_item->room_number
	};

	Richochet(&pos);

	return GunShot(x, y, z, speed, yrot, room_number);
}

bool ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, int16_t extra_rotation, int damage)
{
	auto creature = (CREATURE_INFO*)item->data;
	auto enemy = creature->enemy;

	bool targetable = false,
		 hit = false;

	if (info->distance > PEOPLE_SHOOT_RANGE || !Targetable(item, info))
		hit = targetable = false;
	else
	{
		int distance = info->distance + SQUARE((phd_sin(info->enemy_facing) * enemy->speed >> W2V_SHIFT) * PEOPLE_SHOOT_RANGE / PEOPLE_TARGETING_SPEED);

		hit = (distance > PEOPLE_SHOOT_RANGE ? 0
											 : (GetRandomControl() < ((PEOPLE_SHOOT_RANGE - info->distance) / (PEOPLE_SHOOT_RANGE / 0x5000) + PEOPLE_HIT_CHANCE)));
		targetable = true;
	}

	item->fired_weapon = 3;

	auto fx_number = CreateEffect(item->room_number);

	if (fx_number != NO_ITEM)
	{
		PHD_VECTOR pos { gun->x >> 2, gun->y >> 2, gun->z >> 2 };

		GetJointAbsPosition(item, &pos, gun->mesh_num);

		auto fx = &effects[fx_number];

		fx->pos.x_pos = pos.x;
		fx->pos.y_pos = pos.y;
		fx->pos.z_pos = pos.z;
		fx->room_number = item->room_number;
		fx->pos.y_rot = 0;
		fx->pos.x_rot = 0;
		fx->pos.z_rot = GetRandomControl();
		fx->speed = 16 + (GetRandomControl() & 31);
		fx->fallspeed = -48 - (GetRandomControl() & 7);
		fx->object_number = GUNSHELL;
		fx->frame_number = objects[GUNSHELL].mesh_index;
		fx->shade = 0x4210;
		fx->counter = 1;	// Number of bounces.
		fx->flag1 = item->pos.y_rot - 0x4800 + (GetRandomControl() & 0xfff);

		pos = { gun->x - (gun->x >> 2), gun->y - (gun->y >> 2), gun->z - (gun->z >> 2) };

		GetJointAbsPosition(item, &pos, gun->mesh_num);
		TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, LG_PISTOLS, 32);
	}

	if (damage)
	{
		if (enemy == lara_item)
		{
			if (hit)
			{
				CreatureEffect(item, gun, GunHit);

				lara_item->hit_points -= damage;
				lara_item->hit_status = 1;
			}
			else if (targetable)
				CreatureEffect(item, gun, GunMiss);
		}
		else
		{
			CreatureEffect(item, gun, GunShot);

			if (hit)
			{
				int random = GetRandomControl() & 0xF;
				if (random > 14)
					random = 0;

				enemy->hit_points -= damage / 10;
				enemy->hit_status = 1;

				PHD_VECTOR pos {};

				GetJointAbsPosition(enemy, &pos, random);
				DoBloodSplat(pos.x, pos.y, pos.z, enemy->speed, enemy->pos.y_rot, enemy->room_number);
			}
		}
	}

	GAME_VECTOR start { item->pos.x_pos, item->pos.y_pos - STEP_L * 3, item->pos.z_pos, item->room_number },
				target { enemy->pos.x_pos, enemy->pos.y_pos - STEP_L * 3, enemy->pos.z_pos };

	if (auto smash_item = ObjectOnLOS(&start, &target); smash_item != NO_ITEM)
		if (!(items[smash_item].object_number == SMASH_OBJECT1 && !enable_smash1_destruction))
			SmashItem(smash_item, 0);

	return targetable;
}