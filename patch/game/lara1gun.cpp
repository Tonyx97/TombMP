#include <specific/standard.h>
#include <specific/input.h>
#include <specific/fn_stubs.h>

#include <mp/client.h>
#include <mp/game/level.h>
#include <mp/game/player.h>

#include "objects.h"
#include "laraanim.h"
#include "control.h"
#include "effect2.h"
#include "lara1gun.h"
#include "triboss.h"
#include "camera.h"
#include "lara2gun.h"
#include "lara1gun.h"
#include "physics.h"
#include "game.h"

#define HARPOON_DRAW_ANIM		1
#define ROCKET_DRAW_ANIM		0
#define PELLET_SCATTER  		(20 * ONE_DEGREE)
#define HARPOON_SPEED			256
#define HARPOON_TIME			256
#define ROCKET_SPEED 			512
#define GRENADE_SPEED 			128
#define	MAX_GRENADE_FALLSPEED	128
#define PISTOL_DAMAGE			1
#define ROCKET_BLAST_RADIUS		(WALL_L * 1)
#define GRENADE_BLAST_RADIUS	(WALL_L * 1)

enum weapon_anim
{
	W_AIM,
	W_DRAW,
	W_RECOIL,
	W_UNDRAW,
	W_UNAIM,
	W_RELOAD,
	W_UAIM,
	W_UUNAIM,
	W_URECOIL,
	W_SURF_UNDRAW
};

void draw_shotgun_meshes(int weapon_type)
{
	lara.mesh_ptrs[HAND_R] = meshes[objects[WeaponObject(weapon_type)].mesh_index + HAND_R];
	lara.back_gun = 0;
}

void undraw_shotgun_meshes(int weapon_type)
{
	lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
	lara.back_gun = WeaponObject(weapon_type);
}

void ready_shotgun(int weapon_type)
{
	lara.gun_status = LG_READY;
	lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
	lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
	lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
	lara.left_arm.lock = lara.right_arm.lock = 0;
	lara.target = nullptr;
	lara.left_arm.frame_base = lara.right_arm.frame_base = objects[WeaponObject(weapon_type)].frame_base;
}

void RifleHandler(int weapon_type)
{
	auto winfo = &weapons[weapon_type];

	if (input & IN_ACTION)
		LaraTargetInfo(winfo);
	else lara.target = nullptr;

	LaraGetNewTarget(winfo);

	AimWeapon(winfo, &lara.left_arm);

	if (lara.left_arm.lock)
	{
		lara.torso_y_rot = lara.left_arm.y_rot;
		lara.torso_x_rot = lara.left_arm.x_rot;

		if (camera.old_type != LOOK_CAMERA)
			lara.head_x_rot = lara.head_y_rot = 0;
	}

	if (weapon_type == LG_MAGNUMS)
		AnimatePistols(weapon_type);
	else animate_shotgun(weapon_type);

	if (lara.right_arm.flash_gun)
	{
		if (weapon_type == LG_SHOTGUN || weapon_type == LG_M16)
		{
			TriggerDynamicLight(
				lara_item->pos.x_pos + (phd_sin(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)) + ((GetRandomControl() & 255) - 128),
				lara_item->pos.y_pos - WALL_L / 2 + ((GetRandomControl() & 127) - 63),
				lara_item->pos.z_pos + (phd_cos(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)) + ((GetRandomControl() & 255) - 128),
				(weapon_type == LG_SHOTGUN) ? 12 : 11,
				(GetRandomControl() & 7) + 24,
				(GetRandomControl() & 3) + 16,
				GetRandomControl() & 7
			);
		}
		else if (weapon_type == LG_MAGNUMS)
		{
			PHD_VECTOR pos { (GetRandomControl() & 255) - 128, (GetRandomControl() & 127) - 63, (GetRandomControl() & 255) - 128 };

			get_lara_bone_pos(lara_item, &pos, HAND_R);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 12, (GetRandomControl() & 7) + 24, (GetRandomControl() & 3) + 16, GetRandomControl() & 7);
		}
	}
}

void FireShotgun()
{
	PHD_ANGLE angles[]
	{
		lara.left_arm.y_rot + lara_item->pos.y_rot,
		lara.left_arm.x_rot
	};

	if (!lara.left_arm.lock)
	{
		angles[0] += lara.torso_y_rot;
		angles[1] += lara.torso_x_rot;
	}

	bool fired = false;

	for (int i = 0; i < SHOTGUN_AMMO_CLIP; ++i)
	{
		PHD_ANGLE dangles[]
		{
			PHD_ANGLE(angles[0] + ((int)((GetRandomControl() - 16384) * PELLET_SCATTER) / 65536)),
			PHD_ANGLE(angles[1] + ((int)((GetRandomControl() - 16384) * PELLET_SCATTER) / 65536))
		};

		if (FireWeapon(LG_SHOTGUN, lara.target, lara_item, dangles))
			fired = true;
	}

	if (fired)
	{
		PHD_VECTOR pos { 0, SHOTGUN_YOFF, SHOTGUN_ZOFF };

		get_lara_bone_pos(lara_item, &pos, HAND_R);

		int x = pos.x,
			y = pos.y,
			z = pos.z;

		pos = { 0, SHOTGUN_YOFF + 1280, SHOTGUN_ZOFF };

		get_lara_bone_pos(lara_item, &pos, HAND_R);

		smoke_count_l = 32;
		smoke_weapon = LG_SHOTGUN;

		gns::fx::gun_smoke info
		{
			.x = x,
			.y = y,
			.z = z,
			.vx = pos.x - x,
			.vy = pos.y - y,
			.vz = pos.z - z,
			.initial = 1,
			.weapon = LG_SHOTGUN,
			.count = 32,
			.room = lara_item->room_number
		};

		g_client->send_packet(ID_FX_GUN_SMOKE, info);

		for (int i = 0; i < 7; ++i)  TriggerGunSmoke(info.x, info.y, info.z, info.vx, info.vy, info.vz, info.initial, info.weapon, info.count, info.room);
		for (int i = 0; i < 12; ++i) TriggerShotgunSparks(info.x, info.y, info.z, info.vx << 1, info.vy << 1, info.vz << 1);

		lara.right_arm.flash_gun = weapons[LG_SHOTGUN].flash_time;

		g_audio->play_sound((int)weapons[LG_SHOTGUN].sample_num, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
	}
}

void FireM16(int running)
{
	PHD_ANGLE angles[]
	{
		lara.left_arm.y_rot + lara_item->pos.y_rot,
		lara.left_arm.x_rot
	};

	if (!lara.left_arm.lock)
	{
		angles[0] += lara.torso_y_rot;
		angles[1] += lara.torso_x_rot;
	}

	if (running)
	{
		weapons[M16].shot_accuracy = ONE_DEGREE * 12;
		weapons[M16].damage = 1;
	}
	else
	{
		weapons[M16].shot_accuracy = ONE_DEGREE * 4;
		weapons[M16].damage = 3;
	}

	if (FireWeapon(LG_M16, lara.target, lara_item, angles))
	{
		smoke_count_l = 24;
		smoke_weapon = LG_M16;

		if (g_silenced_hk)
		{
			PHD_VECTOR pos { 0, M16_YOFF, M16_ZOFF };

			get_lara_bone_pos(lara_item, &pos, HAND_R);

			int x = pos.x,
				y = pos.y,
				z = pos.z;

			pos = { 0, M16_YOFF + 1280, M16_ZOFF };

			get_lara_bone_pos(lara_item, &pos, HAND_R);

			gns::fx::gun_smoke info
			{
				.x = x,
				.y = y,
				.z = z,
				.vx = pos.x - x,
				.vy = pos.y - y,
				.vz = pos.z - z,
				.initial = 1,
				.weapon = LG_M16,
				.count = 16,
				.room = lara_item->room_number
			};

			g_client->send_packet(ID_FX_GUN_SMOKE, info);

			TriggerGunSmoke(info.x, info.y, info.z, info.vx, info.vy, info.vz, info.initial, info.weapon, info.count, info.room);
		}

		auto shell_pos = get_gun_shell_pos(HAND_R, LG_M16);

		TriggerGunShell(shell_pos.x, shell_pos.y, shell_pos.z, lara_item->pos.y_rot, GUNSHELL, LG_M16, false, lara_item->room_number, true);

		lara.right_arm.flash_gun = weapons[LG_M16].flash_time;
	}
}

void FireHarpoon()
{
	if (lara.harpoon.ammo <= 0)
		return;

	lara.harpoon.ammo--;

	if (auto item_number = CreateItem(); item_number != NO_ITEM)
	{
		auto item = &items[item_number];

		GAME_VECTOR pos { -2, 373, 77 };

		get_lara_bone_pos(lara_item, (PHD_VECTOR*)&pos, HAND_R);

		item->shade = int16_t(0x4210 | 0x8000);
		item->object_number = HARPOON_BOLT;
		item->room_number = lara_item->room_number;
		item->pos.x_pos = pos.x;
		item->pos.y_pos = pos.y;
		item->pos.z_pos = pos.z;

		InitialiseItem(item_number);

		if (lara.target)
		{
			find_target_point(lara.target, &pos);

			int distance = phd_sqrt(SQUARE(pos.z - item->pos.z_pos) + SQUARE(pos.x - item->pos.x_pos));

			item->pos.y_rot = phd_atan(pos.z - item->pos.z_pos, pos.x - item->pos.x_pos);
			item->pos.x_rot = -phd_atan(distance, pos.y - item->pos.y_pos);
		}
		else
		{
			item->pos.x_rot = lara_item->pos.x_rot + lara.torso_x_rot;
			item->pos.y_rot = lara_item->pos.y_rot + lara.torso_y_rot;
		}

		item->pos.z_rot = 0;
		item->fallspeed = (int16_t)(-HARPOON_SPEED * phd_sin(item->pos.x_rot) >> W2V_SHIFT);
		item->speed = (int16_t)(HARPOON_SPEED * phd_cos(item->pos.x_rot) >> W2V_SHIFT);
		item->hit_points = HARPOON_TIME;
		item->item_flags[1] = 0;

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
			.health = item->hit_points
		};

		g_client->send_packet(ID_PROJECTILE_CREATE, info);

		AddActiveItem(item_number);
	}
}

void ControlHarpoonBolt(int16_t item_number)
{
	auto item = &items[item_number];

	int old_x = item->pos.x_pos,
		old_y = item->pos.y_pos,
		old_z = item->pos.z_pos,
		oldroom = item->room_number;

	item->pos.z_pos += item->speed * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
	item->pos.x_pos += item->speed * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
	item->pos.y_pos += item->fallspeed;

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	ITEM_INFO* target;

	for (auto target_number = room[item->room_number].item_number; target_number != NO_ITEM; target_number = target->next_item)
	{
		target = &items[target_number];

		if (target != lara_item && (!target->collidable || target->object_number == UPV))
			continue;

		if (target->object_number == SMASH_WINDOW ||
			target->object_number == SMASH_OBJECT1 ||
			target->object_number == SMASH_OBJECT2 ||
			target->object_number == SMASH_OBJECT3 ||
			target->object_number == CARCASS ||
			target->object_number == EXTRAFX6 ||
			target == lara_item ||
			(target->status != INVISIBLE && objects[target->object_number].collision))
		{
			auto bounds = GetBestFrame(target);

			if (item->pos.y_pos < target->pos.y_pos + bounds[2] || item->pos.y_pos > target->pos.y_pos + bounds[3])
				continue;

			int c = phd_cos(target->pos.y_rot),
				s = phd_sin(target->pos.y_rot),
				x = item->pos.x_pos - target->pos.x_pos,
				z = item->pos.z_pos - target->pos.z_pos,
				rx = (c * x - s * z) >> W2V_SHIFT,
				ox = old_x - target->pos.x_pos,
				oz = old_z - target->pos.z_pos,
				sx = (c * ox - s * oz) >> W2V_SHIFT;

			if ((rx < bounds[0] && sx < bounds[0]) || (rx > bounds[1] && sx > bounds[1]))
				continue;

			int rz = (c * z + s * x) >> W2V_SHIFT,
				sz = (c * oz + s * ox) >> W2V_SHIFT;

			if ((rz < bounds[4] && sz < bounds[4]) || (rz > bounds[5] && sz > bounds[5]))
				continue;

			if (target->object_number == SMASH_OBJECT1 && enable_smash1_destruction)
				SmashWindow(target_number);
			else if (target->object_number == SMASH_WINDOW ||
					 target->object_number == SMASH_OBJECT2 ||
					 target->object_number == SMASH_OBJECT3)
			{
				SmashWindow(target_number);
			}
			else if (target->object_number == CARCASS || target->object_number == EXTRAFX6)
			{
				if (item->status != ACTIVE)
				{
					item->status = ACTIVE;

					AddActiveItem(target_number);
				}
			}
			else if (target->object_number != SMASH_OBJECT1)
			{
				if (!item->item_flags[1] && objects[target->object_number].intelligent)
				{
					bool is_player = (target == lara_item || g_level->has_player(target));

					if (target->object_number != TRIBEBOSS || !TribeBossShieldOn)
					{
						GAME_VECTOR hitpos { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos };

						HitTarget(target, &hitpos, weapons[LG_HARPOON].damage << item->item_flags[0], is_player);
					}
					else FindClosestShieldPoint(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, target);
				}

				return KillItem(item_number);
			}
		}
	}

	if (item->pos.y_pos >= item->floor || item->pos.y_pos <= GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos))
	{
		if (item->hit_points == HARPOON_TIME)
			item->current_anim_state = item->pos.x_rot;

		if (item->hit_points >= 192)
		{
			item->pos.x_rot = item->current_anim_state + (((m_sin(((item->hit_points - 192) << 9) & 4095, 1) - 1024) * (item->hit_points - 192)) >> 6);
			--item->hit_points;
		}

		if (--item->hit_points == 0)
			return KillItem(item_number);

		item->speed = item->fallspeed = 0;
	}
	else
	{
		item->pos.z_rot += (35 * ONE_DEGREE);

		if (!(room[item->room_number].flags & UNDERWATER))
		{
			item->pos.x_rot -= (1 * ONE_DEGREE);

			if (item->pos.x_rot < -16384)
				item->pos.x_rot = -16384;

			item->fallspeed = (int16_t)(-HARPOON_SPEED * phd_sin(item->pos.x_rot) >> W2V_SHIFT);
			item->speed = (int16_t)(HARPOON_SPEED * phd_cos(item->pos.x_rot) >> W2V_SHIFT);
		}
		else
		{
			if ((wibble & 15) == 0)
				CreateBubble(&item->pos, item->room_number, 2, 8);

			TriggerRocketSmoke(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 64);

			item->fallspeed = (int16_t)(-(HARPOON_SPEED >> 1) * phd_sin(item->pos.x_rot) >> W2V_SHIFT);
			item->speed = (int16_t)((HARPOON_SPEED >> 1) * phd_cos(item->pos.x_rot) >> W2V_SHIFT);
		}
	}

	room_number = item->room_number;

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);
}

void FireRocket()
{
	if (lara.rocket.ammo <= 0)
		return;

	lara.rocket.ammo--;
	lara.has_fired = 1;

	if (auto item_number = CreateItem(); item_number != NO_ITEM)
	{
		auto item = &items[item_number];

		PHD_VECTOR pos { 0, ROCKET_YOFF, ROCKET_ZOFF };

		get_lara_bone_pos(lara_item, &pos, HAND_R);

		int x = pos.x,
			y = pos.y,
			z = pos.z;

		item->object_number = ROCKET;
		item->room_number = lara_item->room_number;
		item->pos.x_pos = pos.x;
		item->pos.y_pos = pos.y;
		item->pos.z_pos = pos.z;

		pos = { 0, ROCKET_YOFF + 1024, ROCKET_ZOFF };

		get_lara_bone_pos(lara_item, &pos, HAND_R);

		smoke_count_l = 32;
		smoke_weapon = LG_ROCKET;

		for (int i = 0; i < 5; ++i)
			TriggerGunSmoke(x, y, z, pos.x - x, pos.y - y, pos.z - z, 1, LG_ROCKET, 32);

		InitialiseItem(item_number);

		item->pos.x_rot = lara_item->pos.x_rot + lara.left_arm.x_rot;
		item->pos.y_rot = lara_item->pos.y_rot + lara.left_arm.y_rot;
		item->pos.z_rot = 0;

		if (!lara.left_arm.lock)
		{
			item->pos.x_rot += lara.torso_x_rot;
			item->pos.y_rot += lara.torso_y_rot;
		}

		item->speed = ROCKET_SPEED >> 5;
		item->item_flags[0] = 0;
		item->item_flags[1] = 0;

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
			.flags0 = 0,
		};

		g_client->send_packet(ID_PROJECTILE_CREATE, info);

		AddActiveItem(item_number);

		phd_PushUnitMatrix();

		*(phd_mxptr + M03) = 0;
		*(phd_mxptr + M13) = 0;
		*(phd_mxptr + M23) = 0;

		phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
		phd_PushMatrix();
		phd_TranslateRel(0, 0, -128);

		int wx = (*(phd_mxptr + M03) >> W2V_SHIFT),
			wy = (*(phd_mxptr + M13) >> W2V_SHIFT),
			wz = (*(phd_mxptr + M23) >> W2V_SHIFT),
			xv, yv, zv;

		phd_PopMatrix();

		for (int i = 0; i < 8; ++i)
		{
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

void ControlRocket(int16_t item_number)
{
	auto item = &items[item_number];

	auto oldroom = item->room_number;

	int old_x = item->pos.x_pos,
		old_y = item->pos.y_pos,
		old_z = item->pos.z_pos;

	bool abovewater = false,
		 explode = false;

	if (room[item->room_number].flags & UNDERWATER)
	{
		if (item->speed > ROCKET_SPEED >> 2)
			item->speed -= (item->speed >> 2);
		else
		{
			item->speed += (item->speed >> 2) + 4;

			if (item->speed > ROCKET_SPEED >> 2)
				item->speed = ROCKET_SPEED >> 2;
		}

		item->pos.z_rot += (((item->speed >> 3) + 3) * ONE_DEGREE);

		abovewater = false;
	}
	else
	{
		if (item->speed < ROCKET_SPEED)
			item->speed += (item->speed >> 2) + 4;

		item->pos.z_rot += (((item->speed >> 2) + 7) * ONE_DEGREE);

		abovewater = true;
	}

	item->shade = int16_t(0x4210 | 0x8000);

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

	phd_TranslateRel(0, 0, -1536 - (GetRandomControl() & 511));

	int xv = (*(phd_mxptr + M03) >> W2V_SHIFT),
		yv = (*(phd_mxptr + M13) >> W2V_SHIFT),
		zv = (*(phd_mxptr + M23) >> W2V_SHIFT);

	phd_PopMatrix();

	if (wibble & 4)
		TriggerRocketFlame(wx, wy, wz, xv - wx, yv - wy, zv - wz, item_number);

	TriggerRocketSmoke(wx + item->pos.x_pos, wy + item->pos.y_pos, wz + item->pos.z_pos, -1);

	if (room[item->room_number].flags & UNDERWATER)
	{
		PHD_3DPOS pos { wx + item->pos.x_pos, wy + item->pos.y_pos, wz + item->pos.z_pos };

		CreateBubble(&pos, item->room_number, 4, 8);
	}

	TriggerDynamicLight(wx + item->pos.x_pos + (GetRandomControl() & 15) - 8, wy + item->pos.y_pos + (GetRandomControl() & 15) - 8, wz + item->pos.z_pos + (GetRandomControl() & 15) - 8, 14, 28 + (GetRandomControl() & 3), 16 + (GetRandomControl() & 7), (GetRandomControl() & 7));

	int speed = (item->speed * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;

	item->pos.z_pos += (speed * phd_cos(item->pos.y_rot)) >> W2V_SHIFT;
	item->pos.x_pos += (speed * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;
	item->pos.y_pos += -((item->speed * phd_sin(item->pos.x_rot)) >> W2V_SHIFT);

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	int radius = 0;

	if (item->pos.y_pos >= item->floor || item->pos.y_pos <= GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos))
	{
		radius = ROCKET_BLAST_RADIUS << item->item_flags[0];
		explode = true;
	}

	if (room[item->room_number].flags & UNDERWATER && abovewater)
	{
		splash_setup.x = item->pos.x_pos;
		splash_setup.y = room[item->room_number].maxceiling;
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerXZoff = 16;
		splash_setup.InnerXZsize = 12;
		splash_setup.InnerYsize = -96;
		splash_setup.InnerXZvel = 0xa0;
		splash_setup.InnerYvel = -(128 << 7);
		splash_setup.InnerGravity = 0x80;
		splash_setup.InnerFriction = 7;
		splash_setup.MiddleXZoff = 24;
		splash_setup.MiddleXZsize = 24;
		splash_setup.MiddleYsize = -64;
		splash_setup.MiddleXZvel = 0xe0;
		splash_setup.MiddleYvel = -(128 << 6);
		splash_setup.MiddleGravity = 0x48;
		splash_setup.MiddleFriction = 8;
		splash_setup.OuterXZoff = 32;
		splash_setup.OuterXZsize = 32;
		splash_setup.OuterXZvel = 0x110;
		splash_setup.OuterFriction = 9;

		SetupSplash(&splash_setup);
	}

	GetNearByRooms(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, radius << 2, radius << 2, item->room_number);

	for (int i = 0; i < number_draw_rooms; ++i)
	{
		ITEM_INFO* target;

		for (auto target_number = room[rooms_to_draw[i]].item_number; target_number != NO_ITEM; target_number = target->next_item)
		{
			target = &items[target_number];

			if (target != lara_item && !target->collidable)
				continue;

			if (target->object_number == SMASH_WINDOW ||
				target->object_number == SMASH_OBJECT1 ||
				target->object_number == SMASH_OBJECT2 ||
				target->object_number == SMASH_OBJECT3 ||
				target->object_number == CARCASS ||
				target->object_number == EXTRAFX6 ||
				target->object_number == FLYING_MUTANT_EMITTER ||
				target == lara_item ||
				(objects[target->object_number].intelligent && target->status != INVISIBLE && objects[target->object_number].collision))
			{
				auto bounds = GetBestFrame(target);

				if (item->pos.y_pos + radius < target->pos.y_pos + bounds[2] || item->pos.y_pos - radius > target->pos.y_pos + bounds[3])
					continue;

				int c = phd_cos(target->pos.y_rot),
					s = phd_sin(target->pos.y_rot),
					x = item->pos.x_pos - target->pos.x_pos,
					z = item->pos.z_pos - target->pos.z_pos,
					rx = (c * x - s * z) >> W2V_SHIFT,
					ox = old_x - target->pos.x_pos,
					oz = old_z - target->pos.z_pos,
					sx = (c * ox - s * oz) >> W2V_SHIFT;

				if ((rx + radius < bounds[0] && sx + radius < bounds[0]) ||
					(rx - radius > bounds[1] && sx - radius > bounds[1]))
					continue;

				int rz = (c * z + s * x) >> W2V_SHIFT,
					sz = (c * oz + s * ox) >> W2V_SHIFT;

				if ((rz + radius < bounds[4] && sz + radius < bounds[4]) ||
					(rz - radius > bounds[5] && sz - radius > bounds[5]))
					continue;

				if (target->object_number == SMASH_OBJECT1 && !enable_smash1_destruction)
				{
					if (item->item_flags[0] == 1)
						SmashWindow(target_number);

					if (!explode)
					{
						explode = true;
						radius = ROCKET_BLAST_RADIUS << item->item_flags[0];
						i = -1;
						break;
					}
				}
				else if (target->object_number == SMASH_OBJECT1 && enable_smash1_destruction)
					SmashWindow(target_number);
				else if (target->object_number == SMASH_WINDOW ||
						 target->object_number == SMASH_OBJECT2 ||
						 target->object_number == SMASH_OBJECT3)
				{
					SmashWindow(target_number);
				}
				else if (target->object_number == CARCASS || target->object_number == EXTRAFX6)
				{
					if (item->status != ACTIVE)
					{
						item->status = ACTIVE;

						AddActiveItem(target_number);
					}
				}
				else if (target->object_number != SMASH_OBJECT1)
				{
					if (!item->item_flags[1])
					{
						bool died = false,
							 is_player = (target == lara_item || g_level->has_player(target));

						if (target->object_number != TRIBEBOSS || !TribeBossShieldOn)
							died = HitTarget(target, nullptr, (30 * PISTOL_DAMAGE) << item->item_flags[0], is_player);
						else FindClosestShieldPoint(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, target);

						if (died)
						{
							if (target->object_number != TRIBEBOSS &&
								target->object_number != WILLARD_BOSS &&
								target->object_number != TONY &&
								target->object_number != LON_BOSS &&
								target->object_number != ELECTRIC_CLEANER &&
								target->object_number != WHALE &&
								target->object_number != FLYING_MUTANT_EMITTER)
							{
								if (target->object_number == LIZARD_MAN && lizard_man_active)
									lizard_man_active = 0;

								CreatureDie(target_number, true, is_player);
							}
						}
					}

					if (!explode)
					{
						explode = true;
						radius = ROCKET_BLAST_RADIUS << item->item_flags[0];
						i = -1;
						break;
					}
				}
			}
		}
	}

	if (explode)
	{
		if (room[oldroom].flags & UNDERWATER)
		{
			item->pos.x_pos = old_x;
			item->pos.y_pos = old_y;
			item->pos.z_pos = old_z;

			ItemNewRoom(item_number, oldroom);
			TriggerUnderwaterExplosion(item);
		}
		else
		{
			TriggerExplosionSparks(old_x, old_y, old_z, 3, -2, 0, item->room_number);

			for (int i = 0; i < 2; ++i)
				TriggerExplosionSparks(old_x, old_y, old_z, 3, -1, 0, item->room_number);
		}

		AlertNearbyGuards(item);

		g_audio->play_sound(105, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		g_audio->play_sound(106, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

		KillItem(item_number);
	}
}

void FireGrenade()
{
	if (lara.grenade.ammo <= 0)
		return;

	lara.grenade.ammo--;
	lara.has_fired = 1;

	if (auto item_number = CreateItem(); item_number != NO_ITEM)
	{
		auto item = &items[item_number];

		PHD_VECTOR pos { 0, GRENADE_YOFF + 96, GRENADE_ZOFF };

		get_lara_bone_pos(lara_item, &pos, HAND_R);

		int x = pos.x,
			y = pos.y,
			z = pos.z;

		item->shade = int16_t(0x4210 | 0x8000);
		item->object_number = GRENADE;
		item->room_number = lara_item->room_number;
		item->pos.x_pos = pos.x;
		item->pos.y_pos = pos.y;
		item->pos.z_pos = pos.z;

		auto floor = GetFloor(pos.x, pos.y, pos.z, &item->room_number);

		if (GetHeight(floor, pos.x, pos.y, pos.z) < pos.y)
		{
			item->pos.x_pos = lara_item->pos.x_pos;
			item->pos.y_pos = pos.y;
			item->pos.z_pos = lara_item->pos.z_pos;
			item->room_number = lara_item->room_number;
		}

		pos = { 0, GRENADE_YOFF + 1024, GRENADE_ZOFF };

		get_lara_bone_pos(lara_item, &pos, HAND_R);

		smoke_count_l = 32;
		smoke_weapon = LG_GRENADE;

		for (int i = 0; i < 5; ++i)
			TriggerGunSmoke(x, y, z, pos.x - x, pos.y - y, pos.z - z, 1, LG_GRENADE, 32);

		InitialiseItem(item_number);

		item->pos.x_rot = lara_item->pos.x_rot + lara.left_arm.x_rot;
		item->pos.y_rot = lara_item->pos.y_rot + lara.left_arm.y_rot;
		item->pos.z_rot = 0;

		if (!lara.left_arm.lock)
		{
			item->pos.x_rot += lara.torso_x_rot;
			item->pos.y_rot += lara.torso_y_rot;
		}

		item->speed = GRENADE_SPEED;
		item->fallspeed = (-item->speed * phd_sin(item->pos.x_rot)) >> W2V_SHIFT;
		item->current_anim_state = item->pos.x_rot;
		item->goal_anim_state = item->pos.y_rot;
		item->required_anim_state = 0;
		item->hit_points = 4 * 30;
		item->item_flags[1] = 0;

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
			.current_anim_state = item->current_anim_state,
			.goal_anim_state = item->goal_anim_state,
			.required_anim_state = item->required_anim_state,
		};

		g_client->send_packet(ID_PROJECTILE_CREATE, info);

		AddActiveItem(item_number);
	}
}

void ControlGrenade(int16_t item_number)
{
	auto item = &items[item_number];

	bool abovewater = false;

	int old_x = item->pos.x_pos,
		old_y = item->pos.y_pos,
		old_z = item->pos.z_pos;

	item->shade = int16_t(0x4210 | 0x8000);

	if (room[item->room_number].flags & UNDERWATER)
	{
		abovewater = false;

		item->fallspeed += (5 - item->fallspeed) >> 1;
		item->speed -= item->speed >> 2;

		if (item->speed)
		{
			item->pos.z_rot += (((item->speed >> 4) + 3) * ONE_DEGREE);

			if (item->required_anim_state) item->pos.y_rot += (((item->speed >> 2) + 3) * ONE_DEGREE);
			else						   item->pos.x_rot += (((item->speed >> 2) + 3) * ONE_DEGREE);
		}
	}
	else
	{
		abovewater = true;

		item->fallspeed += GRAVITY >> 1;

		if (item->speed)
		{
			item->pos.z_rot += (((item->speed >> 2) + 7) * ONE_DEGREE);

			if (item->required_anim_state) item->pos.y_rot += (((item->speed >> 1) + 7) * ONE_DEGREE);
			else						   item->pos.x_rot += (((item->speed >> 1) + 7) * ONE_DEGREE);
		}
	}

	phd_PushUnitMatrix();

	*(phd_mxptr + M03) = 0;
	*(phd_mxptr + M13) = 0;
	*(phd_mxptr + M23) = 0;

	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
	phd_TranslateRel(0, 0, -64);

	int wx = (*(phd_mxptr + M03) >> W2V_SHIFT),
		wy = (*(phd_mxptr + M13) >> W2V_SHIFT),
		wz = (*(phd_mxptr + M23) >> W2V_SHIFT),
		xv, yv, zv;

	phd_PopMatrix();

	if (item->speed && abovewater)
		TriggerRocketSmoke(wx + item->pos.x_pos, wy + item->pos.y_pos, wz + item->pos.z_pos, -1);

	int syrot = item->pos.y_rot;

	item->pos.z_pos += (zv = ((item->speed * phd_cos(item->goal_anim_state)) >> W2V_SHIFT));
	item->pos.x_pos += (xv = ((item->speed * phd_sin(item->goal_anim_state)) >> W2V_SHIFT));
	item->pos.y_pos += (yv = item->fallspeed);
	item->pos.y_rot = item->goal_anim_state;

	DoProperDetection(item_number, old_x, old_y, old_z, xv, yv, zv);

	item->goal_anim_state = item->pos.y_rot;
	item->pos.y_rot = syrot;

	if ((room[item->room_number].flags & UNDERWATER) && abovewater)
	{
		splash_setup.x = item->pos.x_pos;
		splash_setup.y = room[item->room_number].maxceiling;
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerXZoff = 16;
		splash_setup.InnerXZsize = 12;
		splash_setup.InnerYsize = -96;
		splash_setup.InnerXZvel = 0xa0;
		splash_setup.InnerYvel = -(item->fallspeed << 5) - (64 << 5);
		splash_setup.InnerGravity = 0x80;
		splash_setup.InnerFriction = 7;
		splash_setup.MiddleXZoff = 24;
		splash_setup.MiddleXZsize = 24;
		splash_setup.MiddleYsize = -64;
		splash_setup.MiddleXZvel = 0xe0;
		splash_setup.MiddleYvel = -(item->fallspeed << 4) - (64 << 4);
		splash_setup.MiddleGravity = 0x48;
		splash_setup.MiddleFriction = 8;
		splash_setup.OuterXZoff = 32;
		splash_setup.OuterXZsize = 32;
		splash_setup.OuterXZvel = 0x110;
		splash_setup.OuterFriction = 9;

		SetupSplash(&splash_setup);
	}

	int radius = 0;

	bool explode = false;

	if (item->hit_points)
	{
		if (--item->hit_points == 0)
		{
			radius = GRENADE_BLAST_RADIUS;
			explode = true;
		}
	}

	GetNearByRooms(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, radius << 2, radius << 2, item->room_number);

	for (int i = 0; i < number_draw_rooms; ++i)
	{
		ITEM_INFO* target;

		for (auto target_number = room[rooms_to_draw[i]].item_number; target_number != NO_ITEM; target_number = target->next_item)
		{
			target = &items[target_number];

			if (target != lara_item && !target->collidable)
				continue;

			if (target->object_number == SMASH_WINDOW ||
				target->object_number == SMASH_OBJECT1 ||
				target->object_number == SMASH_OBJECT2 ||
				target->object_number == SMASH_OBJECT3 ||
				target->object_number == CARCASS ||
				target->object_number == EXTRAFX6 ||
				target->object_number == FLYING_MUTANT_EMITTER ||
				target == lara_item ||
				(objects[target->object_number].intelligent && target->status != INVISIBLE && objects[target->object_number].collision))
			{
				auto bounds = GetBestFrame(target);

				if (item->pos.y_pos + radius < target->pos.y_pos + bounds[2] || item->pos.y_pos - radius > target->pos.y_pos + bounds[3])
					continue;

				int c = phd_cos(target->pos.y_rot),
					s = phd_sin(target->pos.y_rot),
					x = item->pos.x_pos - target->pos.x_pos,
					z = item->pos.z_pos - target->pos.z_pos,
					rx = (c * x - s * z) >> W2V_SHIFT,
					ox = old_x - target->pos.x_pos,
					oz = old_z - target->pos.z_pos,
					sx = (c * ox - s * oz) >> W2V_SHIFT;

				if ((rx + radius < bounds[0] && sx + radius < bounds[0]) ||
					(rx - radius > bounds[1] && sx - radius > bounds[1]))
					continue;

				int rz = (c * z + s * x) >> W2V_SHIFT,
					sz = (c * oz + s * ox) >> W2V_SHIFT;

				if ((rz + radius < bounds[4] && sz + radius < bounds[4]) ||
					(rz - radius > bounds[5] && sz - radius > bounds[5]))
					continue;

				if (target->object_number == SMASH_OBJECT1 && enable_smash1_destruction)
					SmashWindow(target_number);
				else if (target->object_number == SMASH_WINDOW ||
						 target->object_number == SMASH_OBJECT2 ||
						 target->object_number == SMASH_OBJECT3)
				{
					SmashWindow(target_number);
				}
				else if (target->object_number == CARCASS || target->object_number == EXTRAFX6)
				{
					if (item->status != ACTIVE)
					{
						item->status = ACTIVE;

						AddActiveItem(target_number);
					}
				}
				else if (target->object_number != SMASH_OBJECT1)
				{
					if (!item->item_flags[1])
					{
						bool died = false,
							 is_player = (target == lara_item || g_level->has_player(target));

						if (target->object_number != TRIBEBOSS || !TribeBossShieldOn)
							died = HitTarget(target, nullptr, 20 * PISTOL_DAMAGE, is_player);
						else FindClosestShieldPoint(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, target);

						if (died)
						{
							if (target->object_number != TRIBEBOSS && target->object_number != WHALE && target->object_number != WILLARD_BOSS && target->object_number != TONY && target->object_number != LON_BOSS && target->object_number != ELECTRIC_CLEANER && target->object_number != FLYING_MUTANT_EMITTER)
							{
								if (target->object_number == LIZARD_MAN && lizard_man_active)
									lizard_man_active = 0;

								CreatureDie(target_number, true, is_player);
							}
						}
					}

					if (!explode)
					{
						explode = true;
						radius = GRENADE_BLAST_RADIUS;
						i = -1;
						break;
					}
				}
			}
		}
	}

	if (explode)
	{
		if (room[item->room_number].flags & UNDERWATER)
			TriggerUnderwaterExplosion(item);
		else
		{
			TriggerExplosionSparks(old_x, old_y, old_z, 3, -2, 0, item->room_number);

			for (int i = 0; i < 2; ++i)
				TriggerExplosionSparks(old_x, old_y, old_z, 3, -1, 0, item->room_number);
		}

		AlertNearbyGuards(item);

		g_audio->play_sound(105, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		g_audio->play_sound(106, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

		KillItem(item_number);
	}
}

void draw_shotgun(int weapon_type)
{
	ITEM_INFO* item;

	if (lara.weapon_item == NO_ITEM)
	{
		lara.weapon_item = CreateItem();

		item = &items[lara.weapon_item];
		item->object_number = WeaponObject(weapon_type);

		if (weapon_type == LG_ROCKET)		item->anim_number = objects[ROCKET_GUN].anim_index + 1;
		else if (weapon_type == LG_GRENADE) item->anim_number = objects[GRENADE_GUN].anim_index + ROCKET_DRAW_ANIM;
		else								item->anim_number = objects[item->object_number].anim_index + HARPOON_DRAW_ANIM;

		item->frame_number = anims[item->anim_number].frame_base;
		item->current_anim_state = item->goal_anim_state = W_DRAW;
		item->status = ACTIVE;
		item->room_number = NO_ROOM;

		lara.left_arm.frame_base = lara.right_arm.frame_base = objects[item->object_number].frame_base;
	}
	else item = &items[lara.weapon_item];

	AnimateItem(item);

	if (item->current_anim_state == W_AIM || item->current_anim_state == W_UAIM)							ready_shotgun(weapon_type);
	else if ((item->frame_number - anims[item->anim_number].frame_base) == weapons[weapon_type].draw_frame) draw_shotgun_meshes(weapon_type);
	else if (lara.water_status == LARA_UNDERWATER)															item->goal_anim_state = W_UAIM;

	lara.left_arm.frame_base = lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
	lara.left_arm.frame_number = lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	lara.left_arm.anim_number = lara.right_arm.anim_number = item->anim_number;
}

void undraw_shotgun(int weapon_type)
{
	auto item = &items[lara.weapon_item];

	item->goal_anim_state = (lara.water_status == LARA_SURFACE ? W_SURF_UNDRAW : W_UNDRAW);

	AnimateItem(item);

	if (item->status == DEACTIVATED)
	{
		lara.gun_status = LG_ARMLESS;
		lara.target = nullptr;
		lara.left_arm.lock = lara.right_arm.lock = 0;

		KillItem(lara.weapon_item);

		lara.weapon_item = NO_ITEM;
		lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
	}
	else if (item->current_anim_state == W_UNDRAW && item->frame_number - anims[item->anim_number].frame_base == 21)
		undraw_shotgun_meshes(weapon_type);

	lara.left_arm.frame_base = lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
	lara.left_arm.frame_number = lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	lara.left_arm.anim_number = lara.right_arm.anim_number = item->anim_number;
}

void animate_shotgun(int weapon_type)
{
	static bool m16_firing = false;

	bool running = (weapon_type == LG_M16 && lara_item->speed != 0);

	draw_weapon_smoke(lara_item, smoke_weapon, smoke_count_l, smoke_count_r);

	auto item = &items[lara.weapon_item];

	switch (item->current_anim_state)
	{
	case W_AIM:
	{
		g_audio->stop_sound(g_silenced_hk ? 370 : 78);

		m16_firing = false;

		if (lara.water_status == LARA_UNDERWATER || running)
			item->goal_anim_state = W_UAIM;
		else if (((input & IN_ACTION) && !lara.target) || lara.left_arm.lock)
			item->goal_anim_state = W_RECOIL;
		else item->goal_anim_state = W_UNAIM;

		break;
	}
	case W_UAIM:
	{
		g_audio->stop_sound(g_silenced_hk ? 370 : 78);

		m16_firing = false;

		if (lara.water_status != LARA_UNDERWATER && !running)
			item->goal_anim_state = W_AIM;
		else if (((input & IN_ACTION) && !lara.target) || lara.left_arm.lock)
			item->goal_anim_state = W_URECOIL;
		else item->goal_anim_state = W_UUNAIM;

		break;
	}
	case W_RECOIL:
	{
		if (item->frame_number == anims[item->anim_number].frame_base)
		{
			item->goal_anim_state = W_UNAIM;

			if (lara.water_status != LARA_UNDERWATER && !running)
			{
				if ((input & IN_ACTION) && (!lara.target || lara.left_arm.lock))
				{
					if (weapon_type == LG_HARPOON)
						FireHarpoon();
					else if (weapon_type == LG_ROCKET)
						FireRocket();
					else if (weapon_type == LG_GRENADE)
						FireGrenade();
					else if (weapon_type == LG_M16)
					{
						FireM16(0);

						g_audio->play_sound(g_silenced_hk ? 370 : 78, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos }, 2.f);

						m16_firing = true;
					}
					else FireShotgun();

					item->goal_anim_state = W_RECOIL;
				}
				else if (lara.left_arm.lock)
					item->goal_anim_state = W_AIM;
			}

			if (item->goal_anim_state != W_RECOIL && m16_firing)
			{
				g_audio->stop_sound(g_silenced_hk ? 370 : 78);
				m16_firing = false;
			}
		}
		else if (weapon_type == LG_SHOTGUN && !(input & IN_ACTION) && !lara.left_arm.lock)
			item->goal_anim_state = W_UNAIM;

		if (item->frame_number - anims[item->anim_number].frame_base == 12 && weapon_type == LG_SHOTGUN)
		{
			auto pos = get_gun_shell_pos(HAND_R, LG_M16);

			TriggerGunShell(pos.x, pos.y, pos.z, lara_item->pos.y_rot, SHOTGUNSHELL, LG_SHOTGUN, false, lara_item->room_number, true);
		}

		break;
	}
	case W_URECOIL:
	{
		if (item->frame_number - anims[item->anim_number].frame_base == 0)
		{
			item->goal_anim_state = W_UUNAIM;

			if (lara.water_status == LARA_UNDERWATER || running)
			{
				if ((input & IN_ACTION) && (!lara.target || lara.left_arm.lock))
				{
					if (weapon_type == LG_HARPOON)
						FireHarpoon();
					else
					{
						FireM16(1);

						g_audio->play_sound(g_silenced_hk ? 370 : 78, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos }, 2.f);
					}

					item->goal_anim_state = W_URECOIL;
				}
				else if (lara.left_arm.lock)
					item->goal_anim_state = W_UAIM;
				else
					g_audio->stop_sound(g_silenced_hk ? 370 : 78);
			}
		}
	}
	}

	AnimateItem(item);

	lara.left_arm.frame_base = lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
	lara.left_arm.frame_number = lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	lara.left_arm.anim_number = lara.right_arm.anim_number = item->anim_number;
}

void TriggerUnderwaterExplosion(ITEM_INFO* item)
{
	TriggerExplosionBubble(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
	TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, -2, 1, item->room_number);

	for (int i = 0; i < 3; ++i)
		TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, -1, 1, item->room_number);

	if (int wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number); wh != NO_HEIGHT)
	{
		if (int hfw = item->pos.y_pos - wh; hfw < 2048)
		{
			hfw = 2048 - hfw;

			splash_setup.x = item->pos.x_pos;
			splash_setup.y = room[item->room_number].maxceiling;
			splash_setup.z = item->pos.z_pos;
			splash_setup.InnerXZoff = 16 + (hfw >> 6);
			splash_setup.InnerXZsize = 12 + (hfw >> 6);
			splash_setup.InnerYsize = -96;
			splash_setup.InnerXZvel = 0xa0;
			splash_setup.InnerYvel = (-hfw << 3) - 4096;
			splash_setup.InnerGravity = 0x60;
			splash_setup.InnerFriction = 7;
			splash_setup.MiddleXZoff = 24 + (hfw >> 6);
			splash_setup.MiddleXZsize = 24 + (hfw >> 6);
			splash_setup.MiddleYsize = -64;
			splash_setup.MiddleXZvel = 0xe0;
			splash_setup.MiddleYvel = (-hfw << 2) - 3072;
			splash_setup.MiddleGravity = 0x38;
			splash_setup.MiddleFriction = 8;
			splash_setup.OuterXZoff = 32 + (hfw >> 6);
			splash_setup.OuterXZsize = 32 + (hfw >> 6);
			splash_setup.OuterXZvel = 0x110;
			splash_setup.OuterFriction = 9;

			SetupSplash(&splash_setup);
		}
	}
}