import prof;

#include <specific/standard.h>
#include <specific/litesrc.h>

#include "objects.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "hair.h"
#include "sphere.h"
#include "footprint.h"
#include "effect2.h"
#include "game.h"

#include <specific/fn_stubs.h>

#define MAX_TIME_ALLOWED	((59 * 60 * 30) + (59 * 30) + 27)
#define BLOOD_RADIUS		256
#define SPHERE_RADIUS		(STEP_L * 5 / 2)
#define WF_RANGE			(WALL_L * 16)
#define MAX_BOUNCE			100
#define FLIPFLAG			64
#define UNFLIPFLAG			128
#define SWIM_DEPTH			730
#define DRAGON_FLAME_SPEED	200

int ItemNearLara(ITEM_INFO* item, PHD_3DPOS* pos, int32_t distance)
{
	int x = pos->x_pos - item->pos.x_pos,
		y = pos->y_pos - item->pos.y_pos,
		z = pos->z_pos - item->pos.z_pos;

	if (x < -distance || x > distance || z < -distance || z > distance || y < -WALL_L * 3 || y > WALL_L * 3)
		return 0;

	if (x * x + z * z > SQUARE(distance))
		return 0;

	auto bounds = GetBoundsAccurate(item);

	return (y >= bounds[2] && y <= bounds[3] + 100);
}

void SoundEffects()
{
	auto sound = sound_effects;

	for (int i = number_sound_effects; i > 0; --i, ++sound)
	{
		if (!flip_status && (sound->flags & UNFLIPFLAG) == 0)
			continue;
		else if (flip_status && (sound->flags & FLIPFLAG) == 0)
			continue;

		int sound_id = int(sound->data);
		if (sound_id == 333)
			continue;

		g_audio->play_sound(sound_id, { sound->x, sound->y, sound->z });
	}

	if (flipeffect != -1)
		(*effect_routines[flipeffect])(nullptr);

	//if (camera.mike_at_lara)
		//g_audio->update({ lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos }, (float(camera.actual_angle) / 32768.f) * 3.14159f);
	//else
		g_audio->update({ camera.mike_pos.x, camera.mike_pos.y, camera.mike_pos.z }, (float(camera.actual_angle) / 32768.f) * 3.14159f);
}

int16_t DragonFire(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number)
{
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
		fx->speed = DRAGON_FLAME_SPEED;
		fx->frame_number = GetRandomControl() * (objects[DRAGON_FIRE].nmeshes + 1) >> 15;
		fx->object_number = DRAGON_FIRE;
		fx->shade = 14 * 256;

		ShootAtLara(fx);

		fx->counter = 20;
	}

	return fx_number;
}

void Richochet(GAME_VECTOR* pos)
{
	auto angle = (m_atan2(pos->z, pos->x, lara_item->pos.z_pos, lara_item->pos.x_pos) >> 4) & 4095;

	TriggerRicochetSpark(pos, angle, 8);

	g_audio->play_sound(10, { pos->x, pos->y, pos->z });
}

int16_t DoBloodSplatEx(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE direction, int16_t room_num, int16_t num)
{
	if (room[room_num].flags & UNDERWATER)
		TriggerUnderwaterBlood(x, y, z, num == -1 ? (GetRandomDraw() & 7) : num);
	else TriggerBlood(x, y, z, direction >> 4, num == -1 ? (GetRandomDraw() & 7) + 6 : num);

	return NO_ITEM;
}

int16_t DoBloodSplat(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE direction, int16_t room_num)
{
	if (room[room_num].flags & UNDERWATER)
		TriggerUnderwaterBlood(x, y, z, (GetRandomDraw() & 7));
	else TriggerBlood(x, y, z, direction >> 4, (GetRandomDraw() & 7) + 6);

	return NO_ITEM;
}

int16_t DoBloodSplatD(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE direction, int16_t room_num)
{
	if (room[room_num].flags & UNDERWATER)
		TriggerUnderwaterBloodD(x, y + 64, z, (GetRandomDraw() & 7));
	else TriggerBloodD(x, y, z, direction >> 4, (GetRandomDraw() & 7) + 6);

	return NO_ITEM;
}

void DoLotsOfBlood(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE direction, int16_t room_num, int number)
{
	for (; number > 0; --number)
	{
		DoBloodSplat(
			x + BLOOD_RADIUS - (GetRandomControl() * BLOOD_RADIUS * 2 / 0x8000),
			y + BLOOD_RADIUS - (GetRandomControl() * BLOOD_RADIUS * 2 / 0x8000),
			z + BLOOD_RADIUS - (GetRandomControl() * BLOOD_RADIUS * 2 / 0x8000),
			speed, direction, room_num);
	}
}

void DoLotsOfBloodD(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE direction, int16_t room_num, int number)
{
	for (; number > 0; --number)
	{
		DoBloodSplatD(
			x + BLOOD_RADIUS - (GetRandomDraw() * BLOOD_RADIUS * 2 / 0x8000),
			y,
			z + BLOOD_RADIUS - (GetRandomDraw() * BLOOD_RADIUS * 2 / 0x8000),
			speed, direction, room_num);
	}
}

void CreateBubble(PHD_3DPOS* pos, int16_t room_number, long size, long sizerange)
{
	auto roomnum = room_number;
	auto floor = GetFloor(pos->x_pos, pos->y_pos, pos->z_pos, &roomnum);

	if (!(room[roomnum].flags & UNDERWATER))
		return;

	if (auto fx_number = CreateEffect(room_number); fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		fx->pos.x_pos = pos->x_pos;
		fx->pos.y_pos = pos->y_pos;
		fx->pos.z_pos = pos->z_pos;
		fx->speed = 64 + (GetRandomControl() & 255);
		fx->flag1 = (GetRandomControl() & 31) + 32;
		fx->flag2 = 0;
		fx->frame_number = 0;
		fx->object_number = BUBBLES1;

		TriggerBubble(pos->x_pos, pos->y_pos, pos->z_pos, size, sizerange, fx_number);
	}
}

void LaraBubbles(ITEM_INFO* item)
{
	PHD_VECTOR offset { 0, -4, 64 };

	g_audio->play_sound(37, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

	get_lara_bone_pos(lara_item, &offset, HEAD);

	for (int i = (GetRandomControl() & 3) + 2; i > 0; --i)
		CreateBubble((PHD_3DPOS*)&offset, item->room_number, 8, 8);
}

void LaraBreath(ITEM_INFO* item)
{
	if (lara.water_status == LARA_UNDERWATER || lara_item->hit_points <= 0)
		return;

	if (lara_item->current_anim_state == AS_STOP)
	{
		if (lara_item->frame_number < BREATH_F + 30)
			return;
	}
	else if (lara_item->current_anim_state == AS_DUCK)
	{
		if (lara_item->frame_number < DUCKBREATHE_F + 30)
			return;
	}
	else if (wibble < 0x80 || wibble > 0xc0)
		return;

	PHD_VECTOR offset { 0, -4, 64 },
			   offsetv { (GetRandomControl() & 7) - 4, -8 + (GetRandomControl() & 7), 64 + (GetRandomControl() & 127) };

	get_lara_bone_pos(lara_item, &offset, HEAD);
	get_lara_bone_pos(lara_item, &offsetv, HEAD);

	TriggerBreath(offset.x, offset.y, offset.z, offsetv.x - offset.x, offsetv.y - offset.y, offsetv.z - offset.z);
}

void ControlBubble1(int16_t fx_num)
{
	auto fx = &effects[fx_num];
	auto object = &objects[fx->object_number];

	fx->pos.y_rot += 9 * ONE_DEGREE;
	fx->pos.x_rot += 13 * ONE_DEGREE;
	fx->speed += fx->flag1;

	int x = fx->pos.x_pos + ((phd_sin(fx->pos.y_rot) * 3) >> W2V_SHIFT),
		y = fx->pos.y_pos - (fx->speed >> 8),
		z = fx->pos.z_pos + ((phd_cos(fx->pos.x_rot) * 1) >> W2V_SHIFT);

	auto roomy = fx->room_number;
	auto floor = GetFloor(x, y, z, &roomy);

	int h = GetHeight(floor, x, y, z);

	if (y > h)
	{
		KillEffect(fx_num);
		return;
	}

	if (!floor || !(room[roomy].flags & UNDERWATER))
	{
		if (floor)
			SetupRipple(fx->pos.x_pos, room[fx->room_number].maxceiling, fx->pos.z_pos, -2 - (GetRandomControl() & 1), 1);
		
		return KillEffect(fx_num);
	}

	h = GetCeiling(floor, x, y, z);

	if (h == NO_HEIGHT || y <= h)
	{
		KillEffect(fx_num);
		return;
	}

	if (fx->room_number != roomy)
		EffectNewRoom(fx_num, roomy);

	fx->pos.x_pos = x;
	fx->pos.y_pos = y;
	fx->pos.z_pos = z;
}

void Splash(ITEM_INFO* item)
{
	auto room_number = item->room_number;

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (!(room[room_number].flags & UNDERWATER))
		return;

	int height = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, room_number);

	splash_setup.x = item->pos.x_pos;
	splash_setup.y = height;
	splash_setup.z = item->pos.z_pos;
	splash_setup.InnerXZoff = 32;
	splash_setup.InnerXZsize = 8;
	splash_setup.InnerYsize = -128;
	splash_setup.InnerXZvel = 0x140;
	splash_setup.InnerYvel = -item->fallspeed * 40;
	splash_setup.InnerGravity = 0xa0;
	splash_setup.InnerFriction = 7;
	splash_setup.MiddleXZoff = 48;
	splash_setup.MiddleXZsize = 32;
	splash_setup.MiddleYsize = -64;
	splash_setup.MiddleXZvel = 0x1e0;
	splash_setup.MiddleYvel = -item->fallspeed * 20;
	splash_setup.MiddleGravity = 0x60;
	splash_setup.MiddleFriction = 8;
	splash_setup.OuterXZoff = 32;
	splash_setup.OuterXZsize = 128;
	splash_setup.OuterXZvel = 0x220;
	splash_setup.OuterFriction = 9;

	SetupSplash(&splash_setup);
}

void WadeSplash(ITEM_INFO* item, int water, int waterdepth)
{
	auto room_number = item->room_number;

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (!(room[room_number].flags & UNDERWATER))
		return;

	auto bounds = GetBestFrame(item);

	if (item->pos.y_pos + bounds[2] > water || item->pos.y_pos + bounds[3] < water)
		return;

	if (item->fallspeed > 0 && waterdepth < SWIM_DEPTH - STEP_L && splash_count == 0)
	{
		splash_setup.x = item->pos.x_pos;
		splash_setup.y = water;
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerXZoff = 16;
		splash_setup.InnerXZsize = 12;
		splash_setup.InnerYsize = -96;
		splash_setup.InnerXZvel = 0xa0;
		splash_setup.InnerYvel = -item->fallspeed * 72;
		splash_setup.InnerGravity = 0x80;
		splash_setup.InnerFriction = 7;
		splash_setup.MiddleXZoff = 24;
		splash_setup.MiddleXZsize = 24;
		splash_setup.MiddleYsize = -64;
		splash_setup.MiddleXZvel = 0xe0;
		splash_setup.MiddleYvel = -item->fallspeed * 36;
		splash_setup.MiddleGravity = 0x48;
		splash_setup.MiddleFriction = 8;
		splash_setup.OuterXZoff = 32;
		splash_setup.OuterXZsize = 32;
		splash_setup.OuterXZvel = 0x110;
		splash_setup.OuterFriction = 9;
		SetupSplash(&splash_setup);

		splash_count = 16;
	}
	else if ((wibble & 15) == 0 && ((GetRandomControl() & 15) == 0 || item->current_anim_state != AS_STOP))
		SetupRipple(item->pos.x_pos, water, item->pos.z_pos, -16 - (GetRandomControl() & 15), item->current_anim_state != AS_STOP ? 0 : 1);
}

void WaterFall(int16_t item_number)
{
	auto item = &items[item_number];

	int x = item->pos.x_pos - lara_item->pos.x_pos,
		y = item->pos.y_pos - lara_item->pos.y_pos,
		z = item->pos.z_pos - lara_item->pos.z_pos;

	if (x < -WF_RANGE || x > WF_RANGE || z < -WF_RANGE || z > WF_RANGE || y < -WF_RANGE || y > WF_RANGE)
		return;

	S_CalculateLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, &item->il);

	int ang = ((item->pos.y_rot >> 4) & 4095) << 1;

	z = (m_cos(ang) * 544) >> 12;
	x = (m_sin(ang) * 544) >> 12;

	if ((wibble & 12) == 0)
		TriggerWaterfallMist(item->pos.x_pos + x, item->pos.y_pos, item->pos.z_pos + z, item->pos.y_rot >> 4);

	g_audio->play_sound(79, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
}

void void_effect(ITEM_INFO* item) {}

void finish_level_effect(ITEM_INFO* item)
{
	FinishLevel();
}

void turn180_effect(ITEM_INFO* item)
{
	item->pos.y_rot += -0x8000;
	item->pos.x_rot = -item->pos.x_rot;
}

void floor_shake_effect(ITEM_INFO* item)
{
	int x = item->pos.x_pos - camera.pos.x,
		y = item->pos.y_pos - camera.pos.y,
		z = item->pos.z_pos - camera.pos.z;

	if (ABS(x) < 16 * WALL_L && ABS(y) < 16 * WALL_L && ABS(z) < 16 * WALL_L)
	{
		int dist = (x * x + y * y + z * z) / 256;

		camera.bounce = ((WALL_L * WALL_L - dist) * MAX_BOUNCE) / (WALL_L * WALL_L);
	}
}

void lara_normal_effect(ITEM_INFO* item)
{
	item->current_anim_state = AS_STOP;
	item->goal_anim_state = AS_STOP;
	item->frame_number = STOP_F;
	item->anim_number = STOP_A;

	if (camera.type != TARGET_CAMERA)
		camera.type = CHASE_CAMERA;

	AlterFOV(g_window->get_fov());
}

void BoilerFX(ITEM_INFO* item)
{
	g_audio->play_sound(338);

	flipeffect = -1;
}

void FloodFX(ITEM_INFO* item)
{
	if (fliptimer > 30 * 4)
		flipeffect = -1;
	else
	{
		PHD_3DPOS pos
		{
			lara_item->pos.x_pos,
			(fliptimer < 30 ? camera.target.y + (30 - fliptimer) * 100 : camera.target.y + (fliptimer - 30) * 100),
			lara_item->pos.z_pos
		};

		g_audio->play_sound(163, { pos.x_pos, pos.y_pos, pos.z_pos });
	}

	++fliptimer;
}

void RubbleFX(ITEM_INFO* item)
{
	g_audio->play_sound(24);

	camera.bounce = -350;
	flipeffect = -1;
}

void ChandelierFX(ITEM_INFO* item)
{
	g_audio->play_sound(278);

	if (++fliptimer > 30)
		flipeffect = -1;
}

void ExplosionFX(ITEM_INFO* item)
{
	g_audio->play_sound(105);

	camera.bounce = -75;
	flipeffect = -1;
}

void PistonFX(ITEM_INFO* item)
{
	g_audio->play_sound(190);

	flipeffect = -1;
}

void CurtainFX(ITEM_INFO* item)
{
	g_audio->play_sound(191);

	flipeffect = -1;
}

void StatueFX(ITEM_INFO* item)
{
	g_audio->play_sound(331);

	flipeffect = -1;
}

void SetChangeFX(ITEM_INFO* item)
{
	g_audio->play_sound(330);

	flipeffect = -1;
}

void ControlDingDong(int16_t item_number)
{
	auto item = &items[item_number];

	if ((item->flags & CODE_BITS) == CODE_BITS)
	{
		g_audio->play_sound(334, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

		item->flags -= CODE_BITS;
	}
}

void ControlLaraAlarm(int16_t item_number)
{
	if (auto item = &items[item_number]; (item->flags & CODE_BITS) == CODE_BITS)
		g_audio->play_sound(335, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
}

void ControlBirdTweeter(int16_t item_number)
{
	if (auto item = &items[item_number]; item->object_number == BIRD_TWEETER)
	{
		if (GetRandomControl() < 0x400)
			g_audio->play_sound(316, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}
	else if (GetRandomControl() < 0x100)
		g_audio->play_sound(329, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
}

void DoChimeSound(ITEM_INFO* item)
{
	PHD_3DPOS pos
	{
		lara_item->pos.x_pos + ((item->pos.x_pos - lara_item->pos.x_pos) >> 6),
		lara_item->pos.y_pos + ((item->pos.y_pos - lara_item->pos.y_pos) >> 6),
		lara_item->pos.z_pos + ((item->pos.z_pos - lara_item->pos.z_pos) >> 6)
	};

	g_audio->play_sound(208, { pos.x_pos, pos.y_pos, pos.z_pos });
}

void ControlClockChimes(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->timer == 0)
		return;

	if ((item->timer % 60) == 59)
		DoChimeSound(item);

	if (--item->timer == 0)
	{
		DoChimeSound(item);

		item->timer = -1;

		RemoveActiveItem(item_number);

		item->status = NOT_ACTIVE;
		item->flags &= ~(CODE_BITS);
	}
}

void lara_hands_free(ITEM_INFO* item)
{
	lara.gun_status = LG_ARMLESS;
}

void flip_map_effect(ITEM_INFO* item)
{
	FlipMap();
}

void draw_right_gun(ITEM_INFO* item)
{
	auto temp = lara.mesh_ptrs[THIGH_R];

	lara.mesh_ptrs[THIGH_R] = *(objects[PISTOLS].mesh_ptr + THIGH_R);
	*(objects[PISTOLS].mesh_ptr + THIGH_R) = temp;

	temp = lara.mesh_ptrs[HAND_R];

	lara.mesh_ptrs[HAND_R] = *(objects[PISTOLS].mesh_ptr + HAND_R);
	*(objects[PISTOLS].mesh_ptr + HAND_R) = temp;
}

void draw_left_gun(ITEM_INFO* item)
{
	auto temp = lara.mesh_ptrs[THIGH_L];

	lara.mesh_ptrs[THIGH_L] = *(objects[PISTOLS].mesh_ptr + THIGH_L);
	*(objects[PISTOLS].mesh_ptr + THIGH_L) = temp;

	temp = lara.mesh_ptrs[HAND_L];

	lara.mesh_ptrs[HAND_L] = *(objects[PISTOLS].mesh_ptr + HAND_L);
	*(objects[PISTOLS].mesh_ptr + HAND_L) = temp;
}

void shoot_right_gun(ITEM_INFO* item)
{
	lara.right_arm.flash_gun = 3;
}

void shoot_left_gun(ITEM_INFO* item)
{
	lara.left_arm.flash_gun = 3;
}

void nofunc(ITEM_INFO* item) {}

void swap_meshes_with_meshswap1(ITEM_INFO* item)
{
	auto obj = &objects[item->object_number];
	auto nmeshes = obj->nmeshes;

	for (int i = 0; i < obj->nmeshes; ++i)
	{
		auto temp = *(obj->mesh_ptr + i);

		*(obj->mesh_ptr + i) = *(objects[MESHSWAP1].mesh_ptr + i);
		*(objects[MESHSWAP1].mesh_ptr + i) = temp;
	}
}

void swap_meshes_with_meshswap2(ITEM_INFO* item)
{
	auto obj = &objects[item->object_number];
	auto nmeshes = obj->nmeshes;

	for (int i = 0; i < obj->nmeshes; ++i)
	{
		auto temp = *(obj->mesh_ptr + i);

		*(obj->mesh_ptr + i) = *(objects[MESHSWAP2].mesh_ptr + i);
		*(objects[MESHSWAP2].mesh_ptr + i) = temp;
	}
}

void swap_meshes_with_meshswap3(ITEM_INFO* item)
{
	auto obj = &objects[item->object_number];
	auto nmeshes = obj->nmeshes;

	for (int i = 0; i < obj->nmeshes; ++i)
	{
		auto temp = *(obj->mesh_ptr + i);

		*(obj->mesh_ptr + i) = *(objects[MESHSWAP3].mesh_ptr + i);

		if (item == lara_item)
			lara.mesh_ptrs[i] = *(objects[MESHSWAP3].mesh_ptr + i);

		*(objects[MESHSWAP3].mesh_ptr + i) = temp;
	}
}

void invisibility_on(ITEM_INFO* item)
{
	item->status = INVISIBLE;
}

void invisibility_off(ITEM_INFO* item)
{
	item->status = ACTIVE;
}

void dynamic_light_on(ITEM_INFO* item)
{
	item->dynamic_light = 1;
}

void dynamic_light_off(ITEM_INFO* item)
{
	item->dynamic_light = 0;
}

void reset_hair(ITEM_INFO* item)
{
	InitialiseHair((vec3d*)g_hair, (int_vec3*)g_hair_vel);
}

void TubeTrain(ITEM_INFO* item)
{
	g_audio->play_sound(30);

	camera.bounce = -350;
	flipeffect = -1;
}

void RumbleNoShake(ITEM_INFO* item)
{
	g_audio->play_sound(149);

	flipeffect = -1;
}

void BaddieBiteEffect(ITEM_INFO* item, BITE_INFO* bite)
{
	PHD_VECTOR pos { bite->x, bite->y, bite->z };

	GetJointAbsPosition(item, &pos, bite->mesh_num);

	DoBloodSplat(pos.x, pos.y, pos.z, item->speed, item->pos.y_rot, item->room_number);
}

void LaraWakeUpEffect(ITEM_INFO* item)
{
	static bool test = true;

	if (test)
	{
		lara_item->current_anim_state = AS_STOP;
		lara_item->goal_anim_state = AS_STOP;
		lara_item->anim_number = 1714;
		lara_item->frame_number = GF(1714, 0);
		test = false;
	}

	flipeffect = -1;
}

void (*effect_routines[])(ITEM_INFO*) =
{
	turn180_effect,				// 0
	floor_shake_effect,			// 1
	lara_normal_effect,			// 2
	LaraBubbles,				// 3
	finish_level_effect,		// 4
	FloodFX,					// 5
	ChandelierFX,				// 6
	RubbleFX,					// 7
	PistonFX,					// 8
	CurtainFX,					// 9
	SetChangeFX,				// 10
	ExplosionFX,				// 11
	lara_hands_free,			// 12
	flip_map_effect,			// 13
	draw_right_gun,				// 14
	draw_left_gun,				// 15 (ChainBlockFX)
	shoot_right_gun,			// 16 (FlickerFX)
	shoot_left_gun,				// 17
	swap_meshes_with_meshswap1,	// 18
	swap_meshes_with_meshswap2,	// 19
	swap_meshes_with_meshswap3,	// 20
	invisibility_on,			// 21
	invisibility_off,			// 22
	dynamic_light_on,			// 23
	dynamic_light_off,			// 24
	StatueFX,					// 25
	reset_hair,					// 26
	BoilerFX,					// 27
	LaraWakeUpEffect,			// 28 (new)
	nofunc,						// 29
	nofunc,						// 30
	nofunc,						// 31
	AddFootprint,				// 32
	nofunc,						// 33
	nofunc,						// 34
	nofunc,						// 35
	nofunc,						// 36
	nofunc,						// 37
	nofunc,						// 38
	nofunc,						// 39
	nofunc,						// 40
	nofunc,						// 41
	nofunc,						// 42
	nofunc,						// 43
	nofunc,						// 44
	nofunc,						// 45
	nofunc,						// 46
	nofunc,						// 47
	nofunc,						// 48
	nofunc,						// 49
	nofunc,						// 50
	nofunc,						// 51
	nofunc,						// 52
	nofunc,						// 53
	nofunc,						// 54
	nofunc,						// 55
	nofunc,						// 56
	nofunc,						// 57
	TubeTrain,					// 58
	RumbleNoShake,				// 59
};