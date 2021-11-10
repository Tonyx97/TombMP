#include "effect2.h"
#include "objects.h"
#include "lara.h"
#include "control.h"
#include "traps.h"

void TriggerFlamethrowerHitFlame(long x, long y, long z);
void TriggerFlamethrowerSmoke(long x, long y, long z, long uw);

void ControlFlameThrower(int16_t fx_number)
{
	auto fx = &effects[fx_number];

	--fx->counter;

	if (!fx->counter)
	{
		KillEffect(fx_number);
		return;
	}

	int speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;

	fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
	fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
	fx->pos.y_pos += -((fx->speed * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);

	auto room_number = fx->room_number;
	auto floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

	if ((room[room_number].flags & UNDERWATER) && fx->flag1 == 0)
	{
		if (GetRandomControl() & 1)
			TriggerFlamethrowerSmoke(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 1);

		KillEffect(fx_number);

		return;
	}

	if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) ||
		fx->pos.y_pos <= GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos))
	{
		if (fx->flag1 == 0)
		{
			TriggerFlamethrowerHitFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
			TriggerDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 24, 31, 24, GetRandomControl() & 7);
		}

		KillEffect(fx_number);

		return;
	}

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, room_number);

	if (ItemNearLara(lara_item, &fx->pos, 350))
	{
		if (fx->flag1 == 0)
		{
			lara_item->hit_points -= 3;
			lara_item->hit_status = 1;

			LaraBurn();
		}
		else lara.poisoned += 4;
	}
}

void TriggerFlamethrowerHitFlame(long x, long y, long z)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	int size = (GetRandomControl() & 31) + 128;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sR = 255;
	sptr->sG = 48 + (GetRandomControl() & 31);
	sptr->sB = 48;
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;
	sptr->ColFadeSpeed = 8 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 20;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 31) - 16);
	sptr->y = y;
	sptr->z = z + ((GetRandomControl() & 31) - 16);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 15) - 16;
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 5;

	if (GetRandomControl() & 1)
	{
		sptr->Gravity = -(GetRandomControl() & 31) - 16;
		sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;
		sptr->Gravity = -(GetRandomControl() & 31) - 16;
		sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
	}

	sptr->Def = uint8_t(objects[EXPLOSION1].mesh_ptr);
	sptr->Scalar = 2;
	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 4;
	sptr->dHeight = size >> 4;
}

void TriggerFlamethrowerSmoke(long x, long y, long z, long uw)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	int size = (GetRandomControl() & 31) + 128;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	
	if (uw)
	{
		sptr->sR = 0;
		sptr->sG = 0;
		sptr->sB = 0;
		sptr->dR = 192;
		sptr->dG = 192;
		sptr->dB = 208;
	}
	else
	{
		sptr->sR = 144;
		sptr->sG = 144;
		sptr->sB = 144;
		sptr->dR = 64;
		sptr->dG = 64;
		sptr->dB = 64;
	}

	sptr->ColFadeSpeed = 8;
	sptr->FadeToBlack = 23;
	sptr->sLife = sptr->Life = (GetRandomControl() & 15) + 32;
	sptr->TransType = (uw ? COLADD : COLSUB);
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + (GetRandomControl() & 31) - 16;
	sptr->y = y + (GetRandomControl() & 31) - 16;
	sptr->z = z + (GetRandomControl() & 31) - 16;
	sptr->Xvel = ((GetRandomControl() & 4095) - 2048) >> 2;
	sptr->Yvel = (GetRandomControl() & 255) - 128;
	sptr->Zvel = ((GetRandomControl() & 4095) - 2048) >> 2;

	if (uw)
	{
		sptr->Yvel >>= 4;
		sptr->y += 32;
		sptr->Friction = 4 | (1 << 4);
	}
	else sptr->Friction = 6;

	sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
	sptr->RotAng = GetRandomControl() & 4095;
	sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	sptr->Def = uint8_t(objects[EXPLOSION1].mesh_ptr);
	sptr->Scalar = 3;

	if (uw)
		sptr->Gravity = sptr->MaxYvel = 0;
	else
	{
		sptr->Gravity = -(GetRandomControl() & 3) - 3;
		sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
	}

	sptr->Width = sptr->sWidth = size >> 2;
	sptr->dWidth = size;

	size += (GetRandomControl() & 31) + 32;

	sptr->Height = sptr->sHeight = size >> 3;
	sptr->dHeight = size;
}