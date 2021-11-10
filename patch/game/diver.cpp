#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "control.h"
#include "effect2.h"

#define DIVER_HARPOON_SPEED 150
#define DIVER_SWIM1_TURN	(ONE_DEGREE * 3)
#define DIVER_SWIM2_TURN	(ONE_DEGREE * 3)
#define DIVER_DIE_ANIM		16

int16_t Harpoon(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number);
int32_t GetWaterSurface(int32_t x, int32_t y, int32_t z, int16_t room_number);

enum diver_anim
{
	DIVER_EMPTY,
	DIVER_SWIM1,
	DIVER_SWIM2,
	DIVER_SHOOT1,
	DIVER_AIM1,
	DIVER_NULL1,
	DIVER_AIM2,
	DIVER_SHOOT2,
	DIVER_NULL2,
	DIVER_DEATH
};

BITE_INFO diver_poon = { 17, 164, 44, 18 };

void DiverControl(int16_t item_number)
{
	int32_t water_level;
	GAME_VECTOR start, target;

	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto diver = (CREATURE_INFO*)item->data;

	int16_t angle = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != DIVER_DEATH)
		{
			item->anim_number = objects[DIVER].anim_index + DIVER_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = DIVER_DEATH;
		}

		CreatureFloat(item_number);

		return;
	}

	AI_INFO info;

	CreatureAIInfo(item, &info);
	GetCreatureMood(item, &info, TIMID);
	CreatureMood(item, &info, TIMID);

	bool shoot = false;

	if (lara.water_status == LARA_ABOVEWATER)
	{
		start.x = item->pos.x_pos;
		start.y = item->pos.y_pos - STEP_L;
		start.z = item->pos.z_pos;
		start.room_number = item->room_number;

		target.x = lara_item->pos.x_pos;
		target.y = lara_item->pos.y_pos - (LARA_HITE - 150);
		target.z = lara_item->pos.z_pos;

		if (shoot = LOS(&start, &target))
		{
			diver->target.x = lara_item->pos.x_pos;
			diver->target.y = lara_item->pos.y_pos;
			diver->target.z = lara_item->pos.z_pos;
		}

		if (info.angle < -0x2000 || info.angle > 0x2000)
			shoot = false;
	}
	else if (info.angle > -0x2000 && info.angle < 0x2000)
	{
		start.x = item->pos.x_pos;
		start.y = item->pos.y_pos;
		start.z = item->pos.z_pos;
		start.room_number = item->room_number;

		target.x = lara_item->pos.x_pos;
		target.y = lara_item->pos.y_pos;
		target.z = lara_item->pos.z_pos;

		shoot = LOS(&start, &target);
	}

	angle = CreatureTurn(item, diver->maximum_turn);

	water_level = GetWaterSurface(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number) + WALL_L / 2;

	switch (item->current_anim_state)
	{
	case DIVER_SWIM1:
	{
		diver->maximum_turn = DIVER_SWIM1_TURN;

		if (diver->target.y < water_level && item->pos.y_pos < water_level + diver->LOT.fly)
			item->goal_anim_state = DIVER_SWIM2;
		else if (diver->mood == ESCAPE_MOOD)
			break;
		else if (shoot)
			item->goal_anim_state = DIVER_AIM1;

		break;
	}
	case DIVER_AIM1:
	{
		diver->flags = 0;
		item->goal_anim_state = (!shoot || diver->mood == ESCAPE_MOOD || (diver->target.y < water_level&& item->pos.y_pos < water_level + diver->LOT.fly) ? DIVER_SWIM1
																																						  : DIVER_SHOOT1);
		break;
	}
	case DIVER_SHOOT1:
	{
		if (!diver->flags)
		{
			CreatureEffect(item, &diver_poon, Harpoon);
			diver->flags = 1;
		}

		break;
	}
	case DIVER_SWIM2:
	{
		diver->maximum_turn = DIVER_SWIM2_TURN;

		if (diver->target.y > water_level)
			item->goal_anim_state = DIVER_SWIM1;
		else if (diver->mood == ESCAPE_MOOD)
			break;
		else if (shoot)
			item->goal_anim_state = DIVER_AIM2;

		break;
	}
	case DIVER_AIM2:
	{
		diver->flags = 0;
		item->goal_anim_state = (!shoot || diver->mood == ESCAPE_MOOD || diver->target.y > water_level ? DIVER_SWIM2 : DIVER_SHOOT2);
		break;
	}
	case DIVER_SHOOT2:
	{
		if (!diver->flags)
		{
			CreatureEffect(item, &diver_poon, Harpoon);
			diver->flags = 1;
		}
	}
	}

	CreatureAnimation(item_number, angle, 0);

	switch (item->current_anim_state)
	{
	case DIVER_SWIM1:
	case DIVER_AIM1:
	case DIVER_SHOOT1:
		CreatureUnderwater(item, WALL_L / 2);
		break;
	default:
		item->pos.y_pos = water_level - WALL_L / 2;
	}
}

int16_t Harpoon(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number)
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
		fx->speed = DIVER_HARPOON_SPEED;
		fx->fallspeed = 0;
		fx->frame_number = 0;
		fx->object_number = DIVER_HARPOON;
		fx->shade = 14 * 256;

		ShootAtLara(fx);
	}

	return fx_number;
}

int32_t GetWaterSurface(int32_t x, int32_t y, int32_t z, int16_t room_number)
{
	auto r = &room[room_number];
	auto floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];

	if (r->flags & UNDERWATER)
	{
		while (floor->sky_room != NO_ROOM)
		{
			r = &room[floor->sky_room];

			if (!(r->flags & UNDERWATER))
				return (floor->ceiling << 8);

			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
		}

		return NO_HEIGHT;
	}
	else
	{
		while (floor->pit_room != NO_ROOM)
		{
			r = &room[floor->pit_room];

			if (r->flags & UNDERWATER)
				return (floor->floor << 8);

			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
		}
	}

	return NO_HEIGHT;
}

void ControlGhostGasEmitter(int16_t item_number)
{
	auto item = &items[item_number];

	if (!TriggerActive(item) || (wibble & 15))
		return;

	int dx = lara_item->pos.x_pos - item->pos.x_pos,
		dz = lara_item->pos.z_pos - item->pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	int size = (GetRandomControl() & 31) + 96;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = 12;
	sptr->dG = 32;
	sptr->dB = 0;
	sptr->ColFadeSpeed = 24 + (GetRandomControl() & 7);
	sptr->FadeToBlack = 32;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 64;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = item->pos.x_pos + ((GetRandomControl() & 511) - 256);
	sptr->y = item->pos.y_pos - 256 - ((GetRandomControl() & 15) + 8);
	sptr->z = item->pos.z_pos + ((GetRandomControl() & 1023) - 512);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 1) - 1;
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 4;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 7) - 4 : (GetRandomControl() & 7) + 4);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	sptr->Def = uint8_t(objects[EXPLOSION1].mesh_ptr);
	sptr->Scalar = 3;
	sptr->Gravity = sptr->MaxYvel = 0;
	sptr->Width = sptr->sWidth = size >> 1;
	sptr->dWidth = size >> 1;

	size += (GetRandomControl() & 31) + 32;

	sptr->Height = sptr->sHeight = size >> 1;
	sptr->dHeight = size >> 1;
}