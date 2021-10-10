#include "objects.h"
#include "box.h"
#include "control.h"
#include "sphere.h"
#include "effect2.h"

#define BOOMUTE_TURN (1 * ONE_DEGREE)

void TriggerSealmuteGas(long x, long y, long z, long xv, long yv, long zv, long fxnum);
int16_t TriggerSealmuteGasThrower(ITEM_INFO* item, BITE_INFO* bite, int16_t speed);

BITE_INFO seal_gas = { 0, 48, 140, 10 };

void BoomuteControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->status == DEACTIVATED)
		return;
	else if (item->status == ACTIVE)
	{
		if (item->frame_number >= anims[item->anim_number].frame_base + 1 &&
			item->frame_number <= anims[item->anim_number].frame_end - 8)
		{
			uint32_t length = item->frame_number - anims[item->anim_number].frame_base + 1;

			if (length > 24)
			{
				length = anims[item->anim_number].frame_end - item->frame_number - 8;

				if (length <= 0)
					length = 1;
				else if (length > 24)
					length = (GetRandomControl() & 15) + 8;
			}

			TriggerSealmuteGasThrower(item, &seal_gas, length);
		}

		AnimateItem(item);
	}
}

void TriggerSealmuteGas(long x, long y, long z, long xv, long yv, long zv, long fxnum)
{
	int size = (GetRandomControl() & 31) + 48;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sR = 128 + (GetRandomControl() & 63);
	sptr->sG = 128 + (GetRandomControl() & 63);
	sptr->sB = 32;
	sptr->dR = 32 + (GetRandomControl() & 15);
	sptr->dG = 32 + (GetRandomControl() & 15);
	sptr->dB = 0;

	if (xv || yv || zv)
	{
		sptr->ColFadeSpeed = 6;
		sptr->FadeToBlack = 2;
		sptr->sLife = sptr->Life = (GetRandomControl() & 1) + 16;
	}
	else
	{
		sptr->ColFadeSpeed = 8;
		sptr->FadeToBlack = 16;
		sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 28;
	}

	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 31) - 16);
	sptr->y = y;
	sptr->z = z + ((GetRandomControl() & 31) - 16);
	sptr->Xvel = ((GetRandomControl() & 15) - 16) + xv;
	sptr->Yvel = yv;
	sptr->Zvel = ((GetRandomControl() & 15) - 16) + zv;
	sptr->Friction = 0;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = (fxnum >= 0 ? (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX) : (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF));
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = (fxnum >= 0 ? (SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX) : (SP_SCALE | SP_DEF | SP_EXPDEF));

	sptr->FxObj = fxnum;
	sptr->Gravity = sptr->MaxYvel = 0;
	sptr->Def = objects[EXPLOSION1].mesh_index;

	if (xv || yv || zv)
	{
		sptr->sWidth = sptr->Width = size >> 5;
		sptr->sHeight = sptr->Height = size >> 5;
		sptr->Scalar = (fxnum == -2 ? 2 : 3);
	}
	else
	{
		sptr->sWidth = sptr->Width = size >> 4;
		sptr->sHeight = sptr->Height = size >> 4;
		sptr->Scalar = 4;
	}

	sptr->dWidth = size >> 1;
	sptr->dHeight = size >> 1;
}

int16_t TriggerSealmuteGasThrower(ITEM_INFO* item, BITE_INFO* bite, int16_t speed)
{
	auto fx_number = CreateEffect(item->room_number);

	if (fx_number != NO_ITEM)
	{
		PHD_VECTOR pos1 { bite->x, bite->y, bite->z },
				   pos2 { bite->x, bite->y << 1, bite->z << 3 };

		GetJointAbsPosition(item, &pos1, bite->mesh_num);
		GetJointAbsPosition(item, &pos2, bite->mesh_num);

		auto angles = phd_GetVectorAngles({ pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z });

		auto fx = &effects[fx_number];

		fx->pos.x_pos = pos1.x;
		fx->pos.y_pos = pos1.y;
		fx->pos.z_pos = pos1.z;
		fx->room_number = item->room_number;
		fx->pos.x_rot = angles.y;
		fx->pos.z_rot = 0;
		fx->pos.y_rot = angles.x;
		fx->speed = speed << 2;
		fx->object_number = DRAGON_FIRE;
		fx->counter = 20;
		fx->flag1 = 1;

		TriggerSealmuteGas(0, 0, 0, 0, 0, 0, fx_number);

		for (int i = 0; i < 2; ++i)
		{
			int spd = (GetRandomControl() % (speed << 2)) + 32,
				vel = (spd * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT,
				zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT,
				xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT,
				yv = -((spd * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);

			TriggerSealmuteGas(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -1);
		}

		int vel = ((speed << 1) * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT,
			zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT,
			xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT,
			yv = -(((speed << 1) * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);

		TriggerSealmuteGas(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -2);
	}
	
	return fx_number;
}