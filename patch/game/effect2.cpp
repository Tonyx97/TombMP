#include "objects.h"
#include "lara.h"
#include "control.h"
#include "effect2.h"
#include "fish.h"
#include "draw.h"
#include "gameflow.h"
#include "physics.h"
#include "laraanim.h"

#include <specific/standard.h>
#include <specific/litesrc.h>
#include <specific/fn_stubs.h>

#include <mp/client.h>

short SplashRings[8][2] =
{
	{ 0, -24 },
	{ 17, -17 },
	{ 24, 0 },
	{ 17, 17 },
	{ 0, 24 },
	{ -17, 17 },
	{ -24, 0 },
	{ -17, -17 }
};

void DetachSpark(long num, long type)
{
	auto sptr = &spark[0];

	for (int i = 0; i < MAX_SPARKS; ++i, ++sptr)
	{
		if (sptr->On && (sptr->Flags & type) && sptr->FxObj == num)
		{
			if (type == SP_FX)
			{
				if (sptr->Flags & SP_USEFXOBJPOS)
					sptr->On = 0;
				else
				{
					auto fx = &effects[num];

					sptr->x += fx->pos.x_pos;
					sptr->y += fx->pos.y_pos;
					sptr->z += fx->pos.z_pos;
					sptr->Flags &= (~SP_FX);
				}
			}
			else if (type == SP_ITEM)
			{
				if (sptr->Flags & SP_USEFXOBJPOS)
					sptr->On = 0;
				else
				{
					auto item = &items[num];

					sptr->x += item->pos.x_pos;
					sptr->y += item->pos.y_pos;
					sptr->z += item->pos.z_pos;
					sptr->Flags &= (~SP_ITEM);
				}
			}
		}
	}
}

long GetFreeSpark()
{
	auto sptr = &spark[next_spark];

	for (int free = next_spark, i = 0; i < MAX_SPARKS; ++i)
	{
		if (sptr->On == 0)
		{
			next_spark = (free + 1) & (MAX_SPARKS - 1);

			return free;
		}

		if (free == MAX_SPARKS - 1)
		{
			sptr = &spark[0];
			free = 0;
		}
		else
		{
			++free;
			++sptr;
		}
	}

	int eldest = 0xfff,
		free = 0;

	sptr = &spark[0];

	for (int i = 0; i < MAX_SPARKS; ++i, ++sptr)
	{
		if (sptr->Life < eldest && sptr->Dynamic == -1)
		{
			if (!(sptr->Flags & SP_BLOOD) || (i & 1))
			{
				free = i;
				eldest = sptr->Life;
			}
		}
	}

	next_spark = (free + 1) & (MAX_SPARKS - 1);

	return free;
}

void InitialiseSparks()
{
	auto sptr = &spark[0];

	for (int i = 0; i < MAX_SPARKS; ++i, ++sptr)
	{
		sptr->On = 0;
		sptr->Dynamic = -1;
	}

	for (int i = 0; i < MAX_SPLASHES; ++i) splashes[i].flags = 0;
	for (int i = 0; i < MAX_RIPPLES; ++i)  ripples[i].flags = 0;
	for (int i = 0; i < MAX_BATS; ++i) 	   bats[i].flags = 0;
}

void UpdateSparks()
{
	auto sptr = &spark[0];

	for (int i = 0; i < MAX_SPARKS; ++i, ++sptr)
	{
		if (sptr->On == 0)
			continue;

		if (!(sptr->Flags & SP_USEFXOBJPOS) || ((sptr->Flags & SP_USEFXOBJPOS) && sptr->Life > 16))
			--sptr->Life;

		if (sptr->Life == 0)
		{
			if (sptr->Dynamic != -1)
			{
				spark_dynamics[sptr->Dynamic].On = 0;
				sptr->Dynamic = -1;
			}

			sptr->On = 0;

			continue;
		}

		if (sptr->sLife - sptr->Life < sptr->ColFadeSpeed)
		{
			int divmult = (65536 * (sptr->sLife - sptr->Life)) / sptr->ColFadeSpeed;

			sptr->R = sptr->sR + (((sptr->dR - sptr->sR) * divmult) >> 16);
			sptr->G = sptr->sG + (((sptr->dG - sptr->sG) * divmult) >> 16);
			sptr->B = sptr->sB + (((sptr->dB - sptr->sB) * divmult) >> 16);
		}
		else if (sptr->Life < sptr->FadeToBlack)
		{
			int divmult = 65536 - ((65536 * (sptr->FadeToBlack - sptr->Life)) / sptr->FadeToBlack);

			sptr->R = (sptr->dR * divmult) >> 16;
			sptr->G = (sptr->dG * divmult) >> 16;
			sptr->B = (sptr->dB * divmult) >> 16;
		}
		else
		{
			sptr->R = sptr->dR;
			sptr->G = sptr->dG;
			sptr->B = sptr->dB;
		}

		if (sptr->Life == sptr->FadeToBlack && (sptr->Flags & SP_UNDERWEXP))
		{
			sptr->dWidth >>= 2;
			sptr->dHeight >>= 2;
		}

		if (sptr->Flags & SP_ROTATE)
		{
			sptr->RotAng += sptr->RotAdd;
			sptr->RotAng &= 4095;
		}

		if (sptr->Flags & SP_EXPDEF)
		{
			if (sptr->R < 16 && sptr->G < 16 && sptr->B < 16)	   sptr->Def = objects[EXPLOSION1].mesh_index + 3;
			else if (sptr->R < 64 && sptr->G < 64 && sptr->B < 64) sptr->Def = objects[EXPLOSION1].mesh_index + 2;
			else if (sptr->R < 96 && sptr->G < 96 && sptr->B < 96) sptr->Def = objects[EXPLOSION1].mesh_index + 1;
			else												   sptr->Def = objects[EXPLOSION1].mesh_index;
		}

		if ((sptr->sLife - sptr->Life == (sptr->extras >> 3)) && (sptr->extras & 7))
		{
			int expflag = 0;

			if (sptr->Flags & SP_UNDERWEXP)		 expflag = 1;
			else if (sptr->Flags & SP_PLASMAEXP) expflag = 2;

			for (int j = 0; j < (sptr->extras & 7); ++j)
			{
				TriggerExplosionSparks(sptr->x, sptr->y, sptr->z, (sptr->extras & 7) - 1, sptr->Dynamic, expflag, sptr->RoomNumber);

				sptr->Dynamic = -1;
			}

			if (sptr->Flags & SP_UNDERWEXP)
				TriggerExplosionBubble(sptr->x, sptr->y, sptr->z, sptr->RoomNumber);

			sptr->extras = 0;
		}

		int divmult = sptr->sLife ? (65536 * (sptr->sLife - sptr->Life)) / sptr->sLife : 0;

		sptr->Yvel += sptr->Gravity;

		if (sptr->MaxYvel)
		{
			if ((sptr->Yvel < 0 && sptr->Yvel < (sptr->MaxYvel << 5)) ||
				(sptr->Yvel > 0 && sptr->Yvel > (sptr->MaxYvel << 5)))
				sptr->Yvel = sptr->MaxYvel << 5;
		}

		if (sptr->Friction & 15)
		{
			sptr->Xvel -= (sptr->Xvel >> (sptr->Friction & 15));
			sptr->Zvel -= (sptr->Zvel >> (sptr->Friction & 15));
		}

		if ((sptr->Friction >> 4) & 15)
			sptr->Yvel -= (sptr->Yvel >> ((sptr->Friction >> 4) & 15));

		sptr->x += sptr->Xvel >> 5;
		sptr->y += sptr->Yvel >> 5;
		sptr->z += sptr->Zvel >> 5;

		if (sptr->Flags & SP_WIND)
		{
			sptr->x += g_wind_x >> 1;
			sptr->z += g_wind_z >> 1;
		}

		sptr->Width = sptr->sWidth + (((sptr->dWidth - sptr->sWidth) * divmult) >> 16);
		sptr->Height = sptr->sHeight + (((sptr->dHeight - sptr->sHeight) * divmult) >> 16);
	}

	sptr = &spark[0];

	for (int i = 0; i < MAX_SPARKS; ++i, ++sptr)
	{
		if (sptr->On && sptr->Dynamic != -1)
		{
			auto sdptr = &spark_dynamics[sptr->Dynamic];

			if (sdptr->Flags & (SD_EXPLOSION | SD_UWEXPLOSION))
			{
				int random = GetRandomControl();

				int x = sptr->x + ((random & 0xf) << 4),
					y = sptr->y + ((random & 0xf0)),
					z = sptr->z + ((random & 0xf00) >> 4),
					falloff = sptr->sLife - sptr->Life - 1,
					r, g, b;

				if (falloff < 2)
				{
					if (sdptr->Falloff < 28)
						sdptr->Falloff += 6;

					r = 31 - (random & 3) - (falloff);
					g = 31 - (random & 3) - (falloff << 1);
					b = 31 - (random & 3) - (falloff << 3);
				}
				else if (falloff < 4)
				{
					if (sdptr->Falloff < 28)
						sdptr->Falloff += 6;

					r = 31 - (random & 3) - (falloff);
					g = 16 - (falloff);
					b = 16 - (falloff << 2);

					if (b < 0)
						b = 0;
				}
				else
				{
					if (sdptr->Falloff > 0)
						--sdptr->Falloff;

					r = 28 + (random & 3);
					g = 16 + ((random >> 4) & 3);
					b = ((random >> 8) & 7);
				}

				if (sptr->Flags & SP_PLASMAEXP)
					TriggerDynamicLight(x, y, z, ((sdptr->Falloff > 31) ? 31 : sdptr->Falloff), b, r, g);
				else TriggerDynamicLight(x, y, z, ((sdptr->Falloff > 31) ? 31 : sdptr->Falloff), r, g, b);
			}
		}
	}
}

void TriggerRicochetSpark(GAME_VECTOR* pos, long angle, long size)
{
	auto sptr = &spark[GetFreeSpark()];

	int tangle = (angle + ((GetRandomControl() & 2047) - 1024)) & 4095,
		lp1 = 128 + (GetRandomControl() & 63);

	sptr->On = 1;
	sptr->sR = 255;
	sptr->sG = 32 + (GetRandomControl() & 31);
	sptr->sB = 0;
	sptr->dR = 192;
	sptr->dG = 96 + (GetRandomControl() & 63);
	sptr->dB = 0;
	sptr->ColFadeSpeed = 8;
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = 24;
	sptr->TransType = COLADD;
	sptr->Dynamic = -1;
	sptr->x = pos->x;
	sptr->y = pos->y;
	sptr->z = pos->z;
	sptr->Zvel = m_cos(tangle << 1, 1);
	sptr->Xvel = -m_sin(tangle << 1, 1);
	sptr->Yvel = ((GetRandomControl() & 511) - 384) << 1;
	sptr->Friction = 1;
	sptr->Flags = SP_SCALE;
	sptr->Scalar = 3;
	sptr->Gravity = ((GetRandomControl() & 31)) + (abs(sptr->Yvel >> 6));
	sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
	sptr->dWidth = (GetRandomControl() & 1) + 1;
	sptr->Height = sptr->sHeight = (GetRandomControl() & 3) + 4;
	sptr->dHeight = (GetRandomControl() & 1) + 1;
	sptr->MaxYvel = 0;
	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = lp1;
	sptr->sG = lp1;
	sptr->sB = lp1;
	sptr->dR = lp1 >> 1;
	sptr->dG = lp1 >> 1;
	sptr->dB = lp1 >> 1;
	sptr->ColFadeSpeed = 8;
	sptr->FadeToBlack = 16;
	sptr->sLife = sptr->Life = 28;
	sptr->TransType = COLSUB;
	sptr->Dynamic = -1;
	sptr->x = pos->x;
	sptr->y = pos->y;
	sptr->z = pos->z;
	tangle = (angle + ((GetRandomControl() & 2047) - 1023)) & 4095;
	sptr->Zvel = m_cos(tangle << 1, 1);
	sptr->Xvel = -m_sin(tangle << 1, 1);
	sptr->Yvel = ((GetRandomControl() & 511) - 384);
	sptr->Friction = 1 | 32;
	sptr->Flags = SP_SCALE;
	sptr->Scalar = 3;
	sptr->Gravity = ((GetRandomControl() & 7)) + 4;
	sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
	sptr->dWidth = (GetRandomControl() & 1) + 1;
	sptr->Height = sptr->sHeight = (GetRandomControl() & 3) + 4;
	sptr->dHeight = (GetRandomControl() & 1) + 1;
	sptr->MaxYvel = 0;
}

void TriggerFlareSparks(long x, long y, long z, long xv, long yv, long zv, long smoke)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = 24 + (GetRandomDraw() & 15);

	sptr->On = 1;
	sptr->sR = 255;
	sptr->sG = 255;
	sptr->sB = 255;
	sptr->dR = 255;
	sptr->dG = (GetRandomDraw() & 127) + 64;
	sptr->dB = 192 - sptr->dG;
	sptr->ColFadeSpeed = 3;
	sptr->FadeToBlack = 5;
	sptr->sLife = sptr->Life = 10;
	sptr->TransType = COLADD;
	sptr->Dynamic = -1;
	sptr->x = x + (GetRandomDraw() & 7) - 3;
	sptr->y = y + (GetRandomDraw() & 7) - 3;
	sptr->z = z + (GetRandomDraw() & 7) - 3;
	sptr->Xvel = xv + ((GetRandomDraw() & 255) - 128);
	sptr->Yvel = yv + ((GetRandomDraw() & 255) - 128);
	sptr->Zvel = zv + ((GetRandomDraw() & 255) - 128);
	sptr->Friction = 2 | (2 << 4);
	sptr->Scalar = 1;
	sptr->Width = sptr->sWidth = (GetRandomDraw() & 3) + 4;
	sptr->dWidth = (GetRandomDraw() & 1) + 1;
	sptr->Height = sptr->sHeight = (GetRandomDraw() & 3) + 4;
	sptr->dHeight = (GetRandomDraw() & 1) + 1;
	sptr->Gravity = sptr->MaxYvel = 0;
	sptr->Flags = SP_SCALE;

	if (smoke)
	{
		auto sptr1 = &spark[GetFreeSpark()];

		sptr1->On = 1;
		sptr1->sR = sptr->dR >> 1;
		sptr1->sG = sptr->dG >> 1;
		sptr1->sB = sptr->dB >> 1;
		sptr1->dR = 32;
		sptr1->dG = 32;
		sptr1->dB = 32;
		sptr1->ColFadeSpeed = 8 + (GetRandomDraw() & 3);
		sptr1->FadeToBlack = 4;
		sptr1->sLife = sptr1->Life = (GetRandomDraw() & 7) + 13;
		sptr1->TransType = COLADD;
		sptr1->extras = 0;
		sptr1->Dynamic = -1;
		sptr1->x = x + (xv >> 5);
		sptr1->y = y + (yv >> 5);
		sptr1->z = z + (zv >> 5);
		sptr1->Xvel = xv + ((GetRandomDraw() & 63) - 32);
		sptr1->Yvel = yv;
		sptr1->Zvel = zv + ((GetRandomDraw() & 63) - 32);
		sptr1->Friction = 4;

		if (GetRandomDraw() & 1)
		{
			sptr1->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
			sptr1->RotAng = GetRandomDraw() & 4095;
			sptr1->RotAdd = ((GetRandomDraw() & 1) ? -(GetRandomDraw() & 15) - 16 : (GetRandomDraw() & 15) + 16);
		}
		else sptr1->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

		sptr1->Def = objects[EXPLOSION1].mesh_index;
		sptr1->Scalar = 2;
		sptr1->Gravity = -(GetRandomDraw() & 3) - 8;
		sptr1->MaxYvel = -(GetRandomDraw() & 3) - 4;
		sptr1->Width = sptr1->sWidth = size >> 3;
		sptr1->dWidth = size;
		sptr1->Height = sptr1->sHeight = size >> 3;
		sptr1->dHeight = size;
	}
}

void TriggerExplosionSparks(long x, long y, long z, long extra_trigs, long dyn, long uw, short roomnum)
{
	static constexpr unsigned char extra_tab[6] = { 0, 4, 7, 10 };

	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;

	if (uw == 1)
	{
		sptr->sR = 255;
		sptr->sG = 128 + (GetRandomControl() & 63);
		sptr->sB = 32;
		sptr->dR = 192;
		sptr->dG = 64 + (GetRandomControl() & 31);
		sptr->dB = 0;
	}
	else
	{
		sptr->sR = 255;
		sptr->sG = 32 + (GetRandomControl() & 15);
		sptr->sB = 0;
		sptr->dR = 192 + (GetRandomControl() & 63);
		sptr->dG = 128 + (GetRandomControl() & 63);
		sptr->dB = 32;
	}

	if (uw == 1)
	{
		sptr->ColFadeSpeed = 7;
		sptr->FadeToBlack = 8;
		sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 16;
		sptr->TransType = COLADD;
		sptr->RoomNumber = roomnum;
	}
	else
	{
		sptr->ColFadeSpeed = 8;
		sptr->FadeToBlack = 16;
		sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
		sptr->TransType = COLADD;
	}

	int fric = extra_tab[extra_trigs] + (GetRandomControl() & 7) - 4;

	sptr->extras = extra_trigs | (fric << 3);
	sptr->Dynamic = dyn;

	if (dyn == -2)
	{
		auto sdptr = &spark_dynamics[0];
		
		int i = 0;

		for (; i < 32; ++i, ++sdptr)
		{
			if (!sdptr->On)
			{
				sdptr->On = 1;
				sdptr->Falloff = 4;
				sdptr->Flags = (uw == 1 ? SD_UWEXPLOSION : SD_EXPLOSION);

				sptr->Dynamic = i;

				break;
			}
		}

		if (i == 32)
			sptr->Dynamic = -1;
	}

	sptr->x = x + (GetRandomControl() & 31) - 16;
	sptr->y = y + (GetRandomControl() & 31) - 16;
	sptr->z = z + (GetRandomControl() & 31) - 16;
	sptr->Xvel = ((GetRandomControl() & 4095) - 2048);
	sptr->Yvel = ((GetRandomControl() & 4095) - 2048);
	sptr->Zvel = ((GetRandomControl() & 4095) - 2048);

	if (dyn == -2 && uw != 1)
	{
		sptr->x = x + (GetRandomControl() & 511) - 256;
		sptr->y = y + (GetRandomControl() & 511) - 256;
		sptr->z = z + (GetRandomControl() & 511) - 256;
	}
	else
	{
		sptr->x = x + (GetRandomControl() & 31) - 16;
		sptr->y = y + (GetRandomControl() & 31) - 16;
		sptr->z = z + (GetRandomControl() & 31) - 16;
	}

	sptr->Friction = (uw == 1 ? 1 | (1 << 4) : 3 | (3 << 4));

	if (GetRandomControl() & 1)
	{
		sptr->Flags = (uw == 1 ? (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_UNDERWEXP) : (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF));
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = (GetRandomControl() & 255) - 128;
	}
	else sptr->Flags = (uw == 1 ? (SP_SCALE | SP_DEF | SP_EXPDEF | SP_UNDERWEXP) : (SP_SCALE | SP_DEF | SP_EXPDEF));

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;
	sptr->Gravity = 0;

	fric = (GetRandomControl() & 15) + 40;

	sptr->Width = sptr->sWidth = fric;
	sptr->dWidth = fric << 1;

	fric += (GetRandomControl() & 7) + 8;

	sptr->Height = sptr->sHeight = fric;
	sptr->dHeight = fric << 1;
	sptr->dHeight = fric << 1;
	sptr->MaxYvel = 0;

	if (uw != 2)
	{
		if (extra_trigs == 0)
			TriggerExplosionSmokeEnd(x, y, z, uw);
		else TriggerExplosionSmoke(x, y, z, uw);
	}
	else
	{
		int tr = sptr->sR,
			tg = sptr->sG,
			tb = sptr->sB;

		sptr->sR = tb;
		sptr->sG = tr;
		sptr->sB = tg;

		tr = sptr->dR;
		tg = sptr->dG;
		tb = sptr->dB;

		sptr->dR = tb;
		sptr->dG = tr;
		sptr->dB = tg;

		sptr->Flags |= SP_PLASMAEXP;
	}
}

void TriggerExplosionSmokeEnd(long x, long y, long z, long uw)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 128;

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
	sptr->FadeToBlack = 64;
	sptr->sLife = sptr->Life = (GetRandomControl() & 31) + 96;
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
	sptr->Def = objects[EXPLOSION1].mesh_index;
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

void TriggerExplosionSmoke(long x, long y, long z, long uw)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 128;
	
	sptr->On = 1;
	sptr->sR = 144;
	sptr->sG = 144;
	sptr->sB = 144;
	sptr->dR = 64;
	sptr->dG = 64;
	sptr->dB = 64;
	sptr->ColFadeSpeed = 2;
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 10;
	sptr->TransType = COLSUB;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + (GetRandomControl() & 511) - 256;
	sptr->y = y + (GetRandomControl() & 511) - 256;
	sptr->z = z + (GetRandomControl() & 511) - 256;
	sptr->Xvel = ((GetRandomControl() & 4095) - 2048) >> 2;
	sptr->Yvel = (GetRandomControl() & 255) - 128;
	sptr->Zvel = ((GetRandomControl() & 4095) - 2048) >> 2;
	sptr->Friction = (uw ? 2 : 6);
	sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
	sptr->RotAng = GetRandomControl() & 4095;
	sptr->RotAdd = (GetRandomControl() & 15) + 16;
	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 1;
	sptr->Gravity = -(GetRandomControl() & 3) - 3;
	sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
	sptr->Width = sptr->sWidth = size >> 2;
	sptr->dWidth = size;

	size += (GetRandomControl() & 31) + 32;

	sptr->Height = sptr->sHeight = size >> 3;
	sptr->dHeight = size;
}

void TriggerFireSmoke(long x, long y, long z, long body_part, long type)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 63) + 64;
	
	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = 32;
	sptr->dG = 32;
	sptr->dB = 32;

	if (body_part != -1)
	{
		sptr->ColFadeSpeed = 4 + (GetRandomControl() & 3);
		sptr->FadeToBlack = 12;
		sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
	}
	else
	{
		if (type == 255)
		{
			sptr->ColFadeSpeed = 16 + (GetRandomControl() & 3);
			sptr->FadeToBlack = 8;
			sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 28;
		}
		else
		{
			sptr->ColFadeSpeed = 32 + (GetRandomControl() & 7);
			sptr->FadeToBlack = 16;
			sptr->sLife = sptr->Life = (GetRandomControl() & 15) + 57;
		}
	}

	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y - (GetRandomControl() & 127) - 256;
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 15) - 16;
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 4;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;
	sptr->Gravity = -(GetRandomControl() & 15) - 16;
	sptr->MaxYvel = -(GetRandomControl() & 7) - 8;
	sptr->Width = sptr->sWidth = size >> 2;
	sptr->dWidth = size;
	sptr->Height = sptr->sHeight = size >> 2;
	sptr->dHeight = size;
}

void TriggerGunSmoke(long x, long y, long z, long xv, long yv, long zv, long initial, long weapon, long count, short room_id)
{
	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 7) + 24 - ((weapon != LG_ROCKET && weapon != LG_GRENADE) ? 12 : 0);
	
	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = count << 2;
	sptr->dG = count << 2;
	sptr->dB = count << 2;
	sptr->ColFadeSpeed = 4;
	sptr->FadeToBlack = 32 - (initial << 4);
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 40;

	if (weapon == LG_PISTOLS || weapon == LG_MAGNUMS || weapon == LG_UZIS)
	{
		if (sptr->dR > 64)
		{
			sptr->dR = 64;
			sptr->dG = 64;
			sptr->dB = 64;
		}
	}

	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + (GetRandomControl() & 31) - 16;
	sptr->y = y + (GetRandomControl() & 31) - 16;
	sptr->z = z + (GetRandomControl() & 31) - 16;

	if (initial)
	{
		sptr->Xvel = ((GetRandomControl() & 1023) - 512) + xv;
		sptr->Yvel = ((GetRandomControl() & 1023) - 512) + yv;
		sptr->Zvel = ((GetRandomControl() & 1023) - 512) + zv;
	}
	else
	{
		sptr->Xvel = ((GetRandomControl() & 511) - 256) >> 1;
		sptr->Yvel = ((GetRandomControl() & 511) - 256) >> 1;
		sptr->Zvel = ((GetRandomControl() & 511) - 256) >> 1;
	}

	sptr->Friction = 4;

	room_id = room_id == NO_ROOM ? lara_item->room_number : room_id;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = ((room[room_id].flags & NOT_INSIDE) ? (SP_SCALE | SP_DEF | SP_ROTATE | SP_WIND | SP_EXPDEF) : (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF));
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = ((room[room_id].flags & NOT_INSIDE) ? (SP_SCALE | SP_DEF | SP_WIND | SP_EXPDEF) : (SP_SCALE | SP_DEF | SP_EXPDEF));

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;
	sptr->Gravity = -(GetRandomControl() & 1) - 2;
	sptr->MaxYvel = -(GetRandomControl() & 1) - 2;

	if (initial)
	{
		sptr->Width = sptr->sWidth = size >> 1;
		sptr->dWidth = (size << 1) + 8;
	}
	else
	{
		sptr->Width = sptr->sWidth = size >> 2;
		sptr->dWidth = size;
	}

	if (initial)
	{
		sptr->Height = sptr->sHeight = size >> 1;
		sptr->dHeight = (size << 1) + 8;
	}
	else
	{
		sptr->Height = sptr->sHeight = size >> 2;
		sptr->dHeight = size;
	}
}

void TriggerFireFlame(long x, long y, long z, long body_part, long type)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;

	if (type != 2)
	{
		if (type == 254)
		{
			sptr->sG = 255;
			sptr->sB = 48 + (GetRandomControl() & 31);
			sptr->sR = 48;
			sptr->dG = 192 + (GetRandomControl() & 63);
			sptr->dB = 128 + (GetRandomControl() & 63);
			sptr->dR = 32;
		}
		else
		{
			sptr->sR = 255;
			sptr->sG = 48 + (GetRandomControl() & 31);
			sptr->sB = 48;
		}
	}
	else
	{
		sptr->sR = 48 + (GetRandomControl() & 31);
		sptr->sG = sptr->sR;
		sptr->sB = 192 + (GetRandomControl() & 63);
	}

	if (type != 254)
	{
		sptr->dR = 192 + (GetRandomControl() & 63);
		sptr->dG = 128 + (GetRandomControl() & 63);
		sptr->dB = 32;
	}

	if (body_part != -1)
	{
		sptr->ColFadeSpeed = 8 + (GetRandomControl() & 3);
		sptr->FadeToBlack = 16;
		sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 28;
	}
	else
	{
		if (type != 2 && type != 255 && type != 254)
		{
			sptr->ColFadeSpeed = 20 + (GetRandomControl() & 3);
			sptr->FadeToBlack = 8;
			sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 40;
		}
		else
		{
			sptr->ColFadeSpeed = 5 + (GetRandomControl() & 3);
			sptr->FadeToBlack = 6;
			sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 16 + ((type >= 254) ? 8 : 0);
		}
	}

	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;

	if (body_part != -1)
	{
		sptr->x = ((GetRandomControl() & 31) - 16);
		sptr->y = 0;
		sptr->z = ((GetRandomControl() & 31) - 16);
	}
	else
	{
		if (type == 0 || type == 1)
		{
			sptr->x = x + ((GetRandomControl() & 31) - 16);
			sptr->y = y;
			sptr->z = z + ((GetRandomControl() & 31) - 16);
		}
		else if (type >= 254)
		{
			sptr->x = x + ((GetRandomControl() & 63) - 32);
			sptr->y = y;
			sptr->z = z + ((GetRandomControl() & 63) - 32);
		}
		else
		{
			sptr->x = x + ((GetRandomControl() & 15) - 8);
			sptr->y = y;
			sptr->z = z + ((GetRandomControl() & 15) - 8);
		}
	}

	if (type != 2)
	{
		sptr->Xvel = ((GetRandomControl() & 255) - 128);
		sptr->Yvel = -(GetRandomControl() & 15) - 16;
		sptr->Zvel = ((GetRandomControl() & 255) - 128);
		sptr->Friction = (type == 1 ? 3 | (3 << 4) : 5);
	}
	else
	{
		sptr->Xvel = ((GetRandomControl() & 31) - 16);
		sptr->Yvel = -(GetRandomControl() & 511) - 1024;
		sptr->Zvel = ((GetRandomControl() & 31) - 16);
		sptr->Friction = 4 | (4 << 4);
	}

	if (GetRandomControl() & 1)
	{
		if (body_part != -1)
		{
			sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_FX | SP_EXPDEF;
			sptr->FxObj = body_part;
			sptr->Gravity = -(GetRandomControl() & 63) - 32;
			sptr->MaxYvel = -(GetRandomControl() & 7) - 24;
		}
		else
		{
			sptr->Gravity = -(GetRandomControl() & 31) - 16;
			sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
			sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		}

		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else
	{
		if (body_part != -1)
		{
			sptr->Flags = SP_SCALE | SP_DEF | SP_FX | SP_EXPDEF;
			sptr->FxObj = body_part;
			sptr->Gravity = -(GetRandomControl() & 63) - 32;
			sptr->MaxYvel = -(GetRandomControl() & 7) - 24;
		}
		else
		{
			sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;
			sptr->Gravity = -(GetRandomControl() & 31) - 16;
			sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
		}
	}

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 2;

	int size = 0;

	if (type == 0)		  size = (GetRandomControl() & 31) + 128;
	else if (type == 1)   size = (GetRandomControl() & 31) + 64;
	else if (type >= 254) size = (GetRandomControl() & 15) + 48;
	else
	{
		sptr->Gravity = sptr->MaxYvel = 0;
		size = (GetRandomControl() & 31) + 32;
	}

	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;

	if (type != 2)
	{
		sptr->dWidth = size >> 4;
		sptr->dHeight = size >> 4;
	}
	else
	{
		sptr->dWidth = size >> 2;
		sptr->dHeight = size >> 2;
	}
}

void TriggerFireFlame(long x, long y, long z, long body_part, int r, int g, int b)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;

	sptr->sR = r;
	sptr->sG = g;
	sptr->sB = b;
	sptr->dR = r;
	sptr->dG = g;
	sptr->dB = b;

	if (body_part != -1)
	{
		sptr->ColFadeSpeed = 8 + (GetRandomControl() & 3);
		sptr->FadeToBlack = 16;
		sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 28;
	}
	else
	{
		sptr->ColFadeSpeed = 5 + (GetRandomControl() & 3);
		sptr->FadeToBlack = 6;
		sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 16 + 8;
	}

	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;

	if (body_part != -1)
	{
		sptr->x = ((GetRandomControl() & 31) - 16);
		sptr->y = 0;
		sptr->z = ((GetRandomControl() & 31) - 16);
	}
	else
	{
		sptr->x = x + ((GetRandomControl() & 63) - 32);
		sptr->y = y;
		sptr->z = z + ((GetRandomControl() & 63) - 32);
	}

	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 15) - 16;
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 5;

	if (GetRandomControl() & 1)
	{
		if (body_part != -1)
		{
			sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_FX | SP_EXPDEF;
			sptr->FxObj = body_part;
			sptr->Gravity = -(GetRandomControl() & 63) - 32;
			sptr->MaxYvel = -(GetRandomControl() & 7) - 24;
		}
		else
		{
			sptr->Gravity = -(GetRandomControl() & 31) - 16;
			sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
			sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		}

		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else
	{
		if (body_part != -1)
		{
			sptr->Flags = SP_SCALE | SP_DEF | SP_FX | SP_EXPDEF;
			sptr->FxObj = body_part;
			sptr->Gravity = -(GetRandomControl() & 63) - 32;
			sptr->MaxYvel = -(GetRandomControl() & 7) - 24;
		}
		else
		{
			sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;
			sptr->Gravity = -(GetRandomControl() & 31) - 16;
			sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
		}
	}

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 2;

	int size = (GetRandomControl() & 15) + 48;

	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 4;
	sptr->dHeight = size >> 4;
}

void TriggerSideFlame(long x, long y, long z, long angle, long speed, long pilot)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int zv = (m_cos(angle) * speed) >> 11,
		xv = (m_sin(angle) * speed) >> 11,
		size = (GetRandomControl() & 31) + 128;

	sptr->On = 1;
	sptr->sR = 48 + (GetRandomControl() & 31);
	sptr->sG = sptr->sR;
	sptr->sB = 192 + (GetRandomControl() & 63);
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 28;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 31) - 16);
	sptr->y = y + ((GetRandomControl() & 31) - 16);
	sptr->z = z + ((GetRandomControl() & 31) - 16);

	if (!pilot)
	{
		speed <<= 8;
		speed += (GetRandomControl() & 511);
	}
	else
	{
		speed <<= 7;
		speed += (GetRandomControl() & 31);
	}

	sptr->Xvel = xv + ((GetRandomControl() & 127) - 64);
	sptr->Yvel = -(GetRandomControl() & 7) - 6;
	sptr->Zvel = zv + ((GetRandomControl() & 127) - 64);
	sptr->Friction = 4;
	sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;
	sptr->Gravity = -(GetRandomControl() & 15) - 8;
	sptr->MaxYvel = -(GetRandomControl() & 7) - 8;
	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;

	if (pilot)
		size >>= 2;

	sptr->Width = sptr->sWidth = size >> 1;
	sptr->Height = sptr->sHeight = size >> 1;
	sptr->dWidth = size;
	sptr->dHeight = size;
}

void TriggerAlertLight(long x, long y, long z, long r, long g, long b, long angle, short room_no)
{
	GetFloor(x, y, z, (short*)&room_no);

	int sin = m_sin(angle << 1) << 1,
		cos = m_cos(angle << 1) << 1;

	GAME_VECTOR	src { x, y, z, room_no },
				target { x + sin, y, z + cos };

	if (!LOS(&src, &target))
		TriggerDynamicLight(target.x, target.y, target.z, 8, r, g, b);
}

void TriggerGunShell(int x, int y, int z, int ry, long shelltype, long weapon, bool left, short room, bool sync)
{
	if (weapon != LG_M16)
		TriggerGunSmoke(x, y, z, 0, 0, 0, 0, weapon, weapon == LG_SHOTGUN ? 24 : 16);

	if (auto fx_number = CreateEffect(room); fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];
		
		fx->pos.x_pos = x;
		fx->pos.y_pos = y;
		fx->pos.z_pos = z;
		fx->room_number = room;
		fx->pos.y_rot = 0;
		fx->pos.x_rot = 0;
		fx->pos.z_rot = GetRandomControl();
		fx->speed = 16 + (GetRandomControl() & 31);
		fx->fallspeed = -48 - (GetRandomControl() & 7);
		fx->object_number = shelltype;
		fx->frame_number = objects[fx->object_number].mesh_index;
		fx->shade = 0x4210;
		fx->counter = (GetRandomControl() & 1) + 1;

		if (left)
			fx->flag1 = lara.left_arm.y_rot + ry - 0x4800 + (GetRandomControl() & 0xfff);
		else
		{
			if (weapon == LG_SHOTGUN)
			{
				fx->flag1 = lara.left_arm.y_rot + ry + lara.torso_y_rot + 0x2800 - (GetRandomControl() & 0xfff);
				fx->pos.y_rot += lara.left_arm.y_rot + ry + lara.torso_y_rot;

				if (fx->speed < 24)
					fx->speed += 24;
			}
			else if (weapon == LG_M16)
			{
				fx->flag1 = lara.left_arm.y_rot + ry + lara.torso_y_rot + 0x4800 - (GetRandomControl() & 0xfff);
				fx->pos.y_rot += lara.left_arm.y_rot + ry + lara.torso_y_rot;
			}
			else fx->flag1 = lara.left_arm.y_rot + ry + 0x4800 - (GetRandomControl() & 0xfff);
		}
	}

	if (sync)
		g_client->send_packet(ID_FX_GUNSHELL, gns::fx::gunshell
		{
			.x = x,
			.y = y,
			.z = z,
			.ry = ry,
			.shelltype = shelltype,
			.weapon = weapon,
			.room = room,
			.left = left
		});
}

void ControlGunShell(short fx_number)
{
	auto fx = &effects[fx_number];

	int ox = fx->pos.x_pos,
		oy = fx->pos.y_pos,
		oz = fx->pos.z_pos;

	fx->pos.y_rot += (ONE_DEGREE * fx->speed);
	fx->pos.x_rot += (ONE_DEGREE * ((fx->speed >> 1) + 7));
	fx->pos.z_rot += (ONE_DEGREE * 23);
	fx->pos.z_pos += (fx->speed * phd_cos(fx->flag1)) >> (W2V_SHIFT + 1);
	fx->pos.x_pos += (fx->speed * phd_sin(fx->flag1)) >> (W2V_SHIFT + 1);
	fx->fallspeed += GRAVITY;
	fx->pos.y_pos += fx->fallspeed;

	auto room_number = fx->room_number;
	auto floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, room_number);

	if (room[room_number].flags & UNDERWATER)
	{
		SetupRipple(fx->pos.x_pos, room[room_number].maxceiling, fx->pos.z_pos, -8 - (GetRandomControl() & 3), 1);
		return KillEffect(fx_number);
	}

	int ceiling = GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);

	if (fx->pos.y_pos < ceiling)
	{
		g_audio->play_sound(49, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });

		--fx->counter;
		fx->speed -= 4;

		if (fx->counter < 0 || fx->speed < 8)
			return KillEffect(fx_number);

		fx->fallspeed = -fx->fallspeed;
		fx->pos.y_pos = ceiling;
	}

	int height = GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
	if (fx->pos.y_pos >= height)
	{
		g_audio->play_sound(49, { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos });

		fx->speed -= 8;
		--fx->counter;

		if (fx->counter < 0 || fx->speed < 8)
			return KillEffect(fx_number);

		if (oy > height)
		{
			fx->flag1 += -0x8000;
			fx->pos.x_pos = ox;
			fx->pos.z_pos = oz;
		}
		else fx->fallspeed = -fx->fallspeed >> 1;

		fx->pos.y_pos = oy;
	}

	room_number = fx->room_number;
	floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, room_number);
}

void TriggerShotgunSparks(long x, long y, long z, long xv, long yv, long zv)
{
	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sR = 255;
	sptr->sG = 255;
	sptr->sB = 0;
	sptr->dR = 255;
	sptr->dG = (GetRandomControl() & 127) + 64;
	sptr->dB = 0;
	sptr->ColFadeSpeed = 3;
	sptr->FadeToBlack = 5;
	sptr->sLife = sptr->Life = 10;
	sptr->TransType = COLADD;
	sptr->Dynamic = -1;
	sptr->x = x + (GetRandomControl() & 7) - 3;
	sptr->y = y + (GetRandomControl() & 7) - 3;
	sptr->z = z + (GetRandomControl() & 7) - 3;
	sptr->Xvel = xv + ((GetRandomControl() & 511) - 256);
	sptr->Yvel = yv + ((GetRandomControl() & 511) - 256);
	sptr->Zvel = zv + ((GetRandomControl() & 511) - 256);
	sptr->Friction = 0;
	sptr->Flags = SP_SCALE;
	sptr->Scalar = 2;
	sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
	sptr->dWidth = 1;
	sptr->Height = sptr->sHeight = (GetRandomControl() & 3) + 4;
	sptr->dHeight = 1;
	sptr->Gravity = sptr->MaxYvel = 0;
}

void TriggerRocketFlame(long x, long y, long z, long xv, long yv, long zv, long item_no)
{
	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 7) + 32;
	
	sptr->On = 1;
	sptr->sR = 48 + (GetRandomControl() & 31);
	sptr->sG = sptr->sR;
	sptr->sB = 192 + (GetRandomControl() & 63);
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 12;
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 28;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 31) - 16);
	sptr->y = y;
	sptr->z = z + ((GetRandomControl() & 31) - 16);
	sptr->Xvel = xv;
	sptr->Yvel = yv;
	sptr->Zvel = zv;
	sptr->Friction = 3 | (3 << 4);

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_ITEM | SP_EXPDEF;
		sptr->FxObj = item_no;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ITEM | SP_EXPDEF;
		sptr->FxObj = item_no;
	}

	sptr->Gravity = 0;
	sptr->MaxYvel = 0;
	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 2;
	sptr->Width = sptr->sWidth = size;
	sptr->dWidth = 2;
	sptr->Height = sptr->sHeight = size;
	sptr->dHeight = 2;
}

void TriggerRocketSmoke(long x, long y, long z, long body_part)
{
	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 7) + 32;
	
	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = 64 + body_part;
	sptr->dG = 64 + body_part;
	sptr->dB = 64 + body_part;
	sptr->ColFadeSpeed = 4 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 12;
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 3) - 4;
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 4;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;
	sptr->Gravity = -(GetRandomControl() & 3) - 4;
	sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
	sptr->Width = sptr->sWidth = size >> 2;
	sptr->dWidth = size;
	sptr->Height = sptr->sHeight = size >> 2;
	sptr->dHeight = size;
}

void TriggerBlood(long x, long y, long z, long angle, long num)
{
	for (int i = 0; i < num; ++i)
	{
		auto sptr = &spark[GetFreeSpark()];

		int tangle = (angle + ((GetRandomControl() & 31) - 16)) & 4095,
			tspeed = GetRandomControl() & 15;

		sptr->On = 1;
		sptr->sR = 225;
		sptr->sG = 0;
		sptr->sB = 32;
		sptr->dR = 255;
		sptr->dG = 0;
		sptr->dB = 0;
		sptr->ColFadeSpeed = 8;
		sptr->FadeToBlack = 8;
		sptr->sLife = sptr->Life = 24;
		sptr->TransType = SEMITRANS;
		sptr->Dynamic = -1;
		sptr->x = x + ((GetRandomControl() & 31) - 16);
		sptr->y = y + ((GetRandomControl() & 31) - 16);
		sptr->z = z + ((GetRandomControl() & 31) - 16);
		sptr->Zvel = (m_cos(tangle << 1) * 10) >> 6;
		sptr->Xvel = (-m_sin(tangle << 1) * 10) >> 6;
		sptr->Yvel = -((GetRandomControl() & 255) + 128);
		sptr->Friction = 4;
		sptr->Flags = SP_BLOOD;
		sptr->Scalar = 3;
		sptr->Gravity = ((GetRandomControl() & 31) + 31);
		sptr->Width = sptr->sWidth = 4;
		sptr->dWidth = 2 - (GetRandomControl() & 1);
		sptr->Height = sptr->sHeight = 4;
		sptr->dHeight = 2 - (GetRandomControl() & 1);
		sptr->MaxYvel = 0;
	}
}

void TriggerBloodD(long x, long y, long z, long angle, long num)
{
	for (int i = 0; i < num; ++i)
	{
		auto sptr = &spark[GetFreeSpark()];

		int tangle = (angle + ((GetRandomDraw() & 31) - 16)) & 4095,
			tspeed = GetRandomDraw() & 15;

		sptr->On = 1;
		sptr->sR = 224;
		sptr->sG = 0;
		sptr->sB = 32;
		sptr->dR = 192;
		sptr->dG = 0;
		sptr->dB = 24;
		sptr->ColFadeSpeed = 8;
		sptr->FadeToBlack = 8;
		sptr->sLife = sptr->Life = 24;
		sptr->TransType = SEMITRANS;
		sptr->Dynamic = -1;
		sptr->x = x + ((GetRandomDraw() & 31) - 16);
		sptr->y = y + ((GetRandomDraw() & 31) - 16);
		sptr->z = z + ((GetRandomDraw() & 31) - 16);
		sptr->Zvel = (m_cos(tangle << 1) * tspeed) >> 5;
		sptr->Xvel = (-m_sin(tangle << 1) * tspeed) >> 5;
		sptr->Yvel = -((GetRandomDraw() & 255) + 128);
		sptr->Friction = 4;
		sptr->Flags = 0;
		sptr->Scalar = 3;
		sptr->Gravity = ((GetRandomDraw() & 31) + 31);
		sptr->Width = sptr->sWidth = 2;
		sptr->dWidth = 2 - (GetRandomDraw() & 1);
		sptr->Height = sptr->sHeight = 2;
		sptr->dHeight = 2 - (GetRandomDraw() & 1);
		sptr->MaxYvel = 0;
	}
}

void SetupSplash(SPLASH_SETUP* spl)
{
	auto sptr = &splashes[0];

	for (int i = 0; i < MAX_SPLASHES; ++i, ++sptr)
	{
		if (!(sptr->flags & SPL_ON))
		{
			sptr->flags = SPL_ON | SPL_TRANS;

			if (spl->OuterFriction == -9)
			{
				sptr->flags |= SPL_KAYAK_SPLASH;
				spl->OuterFriction = 9;
			}

			sptr->x = spl->x;
			sptr->y = spl->y;
			sptr->z = spl->z;
			sptr->life = 63;

			auto svptr = &sptr->sv[0];

			for (int j = 0; j < 8; ++j)
			{
				svptr->wx = (spl->InnerXZoff * SplashRings[j][0]) << 1;
				svptr->wz = (spl->InnerXZoff * SplashRings[j][1]) << 1;
				svptr->wy = 0;
				svptr->xv = (spl->InnerXZvel * SplashRings[j][0]) / 12;
				svptr->zv = (spl->InnerXZvel * SplashRings[j][1]) / 12;
				svptr->yv = 0;
				svptr->oxv = svptr->xv >> 3;
				svptr->ozv = svptr->zv >> 3;
				svptr->friction = spl->InnerFriction - 2;
				svptr->gravity = 0;

				svptr++;
			}

			for (int j = 0; j < 8; ++j)
			{
				svptr->wx = ((spl->InnerXZoff + spl->InnerXZsize) * SplashRings[j][0]) << 1;
				svptr->wz = ((spl->InnerXZoff + spl->InnerXZsize) * SplashRings[j][1]) << 1;
				svptr->wy = spl->InnerYsize;
				svptr->xv = (spl->InnerXZvel * SplashRings[j][0]) >> 3;
				svptr->zv = (spl->InnerXZvel * SplashRings[j][1]) >> 3;
				svptr->yv = spl->InnerYvel;
				svptr->oxv = svptr->xv >> 3;
				svptr->ozv = svptr->zv >> 3;
				svptr->friction = spl->InnerFriction;
				svptr->gravity = spl->InnerGravity;

				++svptr;
			}

			for (int j = 0; j < 8; ++j)
			{
				svptr->wx = (spl->MiddleXZoff * SplashRings[j][0]) << 1;
				svptr->wz = (spl->MiddleXZoff * SplashRings[j][1]) << 1;
				svptr->wy = 0;
				svptr->xv = (spl->MiddleXZvel * SplashRings[j][0]) / 12;
				svptr->zv = (spl->MiddleXZvel * SplashRings[j][1]) / 12;
				svptr->yv = 0;
				svptr->oxv = svptr->xv >> 3;
				svptr->ozv = svptr->zv >> 3;
				svptr->friction = spl->MiddleFriction - 2;
				svptr->gravity = 0;

				++svptr;
			}

			for (int j = 0; j < 8; ++j)
			{
				svptr->wx = ((spl->MiddleXZoff + spl->MiddleXZsize) * SplashRings[j][0]) << 1;
				svptr->wz = ((spl->MiddleXZoff + spl->MiddleXZsize) * SplashRings[j][1]) << 1;
				svptr->wy = spl->MiddleYsize;
				svptr->xv = (spl->MiddleXZvel * SplashRings[j][0]) >> 3;
				svptr->zv = (spl->MiddleXZvel * SplashRings[j][1]) >> 3;
				svptr->yv = spl->MiddleYvel;
				svptr->oxv = svptr->xv >> 3;
				svptr->ozv = svptr->zv >> 3;
				svptr->friction = spl->MiddleFriction;
				svptr->gravity = spl->MiddleGravity;

				++svptr;
			}

			for (int j = 0; j < 8; ++j)
			{
				svptr->wx = (spl->OuterXZoff * SplashRings[j][0]) << 1;
				svptr->wz = (spl->OuterXZoff * SplashRings[j][1]) << 1;
				svptr->wy = 0;
				svptr->xv = (spl->OuterXZvel * SplashRings[j][0]) / 12;
				svptr->zv = (spl->OuterXZvel * SplashRings[j][1]) / 12;
				svptr->yv = 0;
				svptr->oxv = svptr->xv >> 3;
				svptr->ozv = svptr->zv >> 3;
				svptr->friction = spl->OuterFriction - 2;
				svptr->gravity = 0;

				++svptr;
			}

			for (int j = 0; j < 8; ++j, ++svptr)
			{
				svptr->wx = ((spl->OuterXZoff + spl->OuterXZsize) * SplashRings[j][0]) << 1;
				svptr->wz = ((spl->OuterXZoff + spl->OuterXZsize) * SplashRings[j][1]) << 1;
				svptr->wy = 0;
				svptr->xv = (spl->OuterXZvel * SplashRings[j][0]) >> 3;
				svptr->zv = (spl->OuterXZvel * SplashRings[j][1]) >> 3;
				svptr->yv = 0;
				svptr->oxv = svptr->xv >> 3;
				svptr->ozv = svptr->zv >> 3;
				svptr->friction = spl->OuterFriction;
				svptr->gravity = 0;
			}

			break;
		}
	}

	g_audio->play_sound(33, { spl->x, spl->y, spl->z });
}

void UpdateSplashes()
{
	auto sptr = &splashes[0];

	for (int i = 0; i < MAX_SPLASHES; ++i, ++sptr)
	{
		if (sptr->flags & SPL_ON)
		{
			auto svptr = &sptr->sv[0];

			int declife = 0;

			for (int j = 0; j < 48; ++j, ++svptr)
			{
				svptr->wx += svptr->xv >> 2;
				svptr->wy += svptr->yv >> 6;
				svptr->wz += svptr->zv >> 2;
				svptr->xv -= (svptr->xv >> svptr->friction);
				svptr->zv -= (svptr->zv >> svptr->friction);

				if (svptr->oxv < 0 && svptr->xv > svptr->oxv)	   svptr->xv = svptr->oxv;
				else if (svptr->oxv > 0 && svptr->xv < svptr->oxv) svptr->xv = svptr->oxv;
				else if (svptr->ozv < 0 && svptr->zv > svptr->ozv) svptr->zv = svptr->ozv;
				else if (svptr->ozv > 0 && svptr->zv < svptr->ozv) svptr->zv = svptr->ozv;

				int oyv = svptr->yv;

				svptr->yv += svptr->gravity << 3;

				if (svptr->yv > 0x10000)
					svptr->yv = 0x10000;

				if (svptr->wy > 0)
				{
					if (j < 16)	     sptr->flags |= SPL_RIPPLEINNER;
					else if (j < 32) sptr->flags |= SPL_RIPPLEMIDDLE;

					svptr->wy = 0;

					declife = 1;
				}
			}

			if (declife && --sptr->life == 0)
				sptr->flags = 0;
		}
	}

	auto rptr = &ripples[0];

	for (int i = 0; i < MAX_RIPPLES; ++i, ++rptr)
	{
		if (rptr->flags & SPL_ON)
		{
			if (rptr->size < 254)
				rptr->size += 2;

			if (!rptr->init)
			{
				rptr->life -= 2;

				if (rptr->life > 250)
					rptr->flags = 0;
			}
			else if (rptr->init < rptr->life)
			{
				rptr->init += 4;

				if (rptr->init >= rptr->life)
					rptr->init = 0;
			}
		}
	}
}

RIPPLE_STRUCT* SetupRipple(long x, long y, long z, long size, long moretrans)
{
	auto rptr = &ripples[0];

	for (int i = 0; i < MAX_RIPPLES; ++i)
	{
		if (!(rptr->flags & SPL_ON))
		{
			if (size < 0)
			{
				rptr->flags = (moretrans ? (SPL_ON | SPL_TRANS | SPL_MORETRANS) : (SPL_ON | SPL_TRANS));

				size = -size;
			}
			else rptr->flags = SPL_ON;

			rptr->life = 48 + (GetRandomControl() & 15);
			rptr->init = 1;
			rptr->size = size;
			rptr->x = x + (GetRandomControl() & 127) - 64;
			rptr->y = y;
			rptr->z = z + (GetRandomControl() & 127) - 64;

			break;
		}

		++rptr;
	}

	return rptr;
}

void TriggerUnderwaterBlood(long x, long y, long z, long size)
{
	auto rptr = &ripples[0];

	for (int i = 0; i < MAX_RIPPLES; ++i, ++rptr)
	{
		if (!(rptr->flags & SPL_ON))
		{
			rptr->flags = SPL_ON | SPL_TRANS | SPL_MORETRANS | SPL_BLOOD;
			rptr->life = 240 + (GetRandomControl() & 7);
			rptr->init = 1;
			rptr->size = size;
			rptr->x = x + (GetRandomControl() & 63) - 32;
			rptr->y = y;
			rptr->z = z + (GetRandomControl() & 63) - 32;

			break;
		}
	}
}

void TriggerUnderwaterBloodD(long x, long y, long z, long size)
{
	auto rptr = &ripples[0];

	for (int i = 0; i < MAX_RIPPLES; ++i, ++rptr)
	{
		if (!(rptr->flags & SPL_ON))
		{
			rptr->flags = SPL_ON | SPL_TRANS | SPL_MORETRANS | SPL_BLOOD;
			rptr->life = 240 + (GetRandomDraw() & 7);
			rptr->init = 1;
			rptr->size = size;
			rptr->x = x + (GetRandomDraw() & 63) - 32;
			rptr->y = y;
			rptr->z = z + (GetRandomDraw() & 63) - 32;

			break;
		}
	}
}

void TriggerWaterfallMist(long x, long y, long z, long angle)
{
	static constexpr int dist_tab[4] = { 512 + 64, 171 + 32, -171 - 32, -512 - 64 };

	for (int i = 0; i < 4; ++i)
	{
		auto sptr = &spark[GetFreeSpark()];

		int dist = dist_tab[i] + (GetRandomControl() & 31) - 16,
			tangle = (angle + 1024) & 4095,
			xsize = (m_sin(tangle << 1) * dist) >> 12,
			zsize = (m_cos(tangle << 1) * dist) >> 12;

		sptr->On = 1;
		sptr->sR = 128;
		sptr->sG = 128;
		sptr->sB = 128;
		sptr->dR = 192;
		sptr->dG = 192;
		sptr->dB = 192;
		sptr->ColFadeSpeed = 2;
		sptr->FadeToBlack = 4;
		sptr->sLife = sptr->Life = 6 + (GetRandomControl() & 3);
		sptr->TransType = COLADD;
		sptr->extras = 0;
		sptr->Dynamic = -1;
		sptr->x = x + xsize + ((GetRandomControl() & 15) - 8);
		sptr->y = y + ((GetRandomControl() & 15) - 8);
		sptr->z = z + zsize + ((GetRandomControl() & 15) - 8);
		sptr->Xvel = m_sin(angle << 1, 12);
		sptr->Zvel = m_cos(angle << 1, 12);
		sptr->Yvel = 0;
		sptr->Friction = 3;

		if (GetRandomControl() & 1)
		{
			sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
			sptr->RotAng = GetRandomControl() & 4095;
			sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
		}
		else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

		sptr->Def = objects[EXPLOSION1].mesh_index;
		sptr->Scalar = 6;
		sptr->Gravity = 0;
		sptr->MaxYvel = 0;
		xsize = (GetRandomControl() & 7) + 12;
		sptr->Width = sptr->sWidth = xsize >> 1;
		sptr->dWidth = xsize;
		sptr->Height = sptr->sHeight = xsize >> 1;
		sptr->dHeight = xsize;
	}
}

void UpdateBats()
{
	auto bptr = &bats[0];

	for (int i = 0; i < MAX_BATS; ++i, ++bptr)
	{
		if (bptr->flags & B_ON)
		{
			if ((i & 3) == 0 && (GetRandomControl() & 7) == 0)
				g_audio->play_sound(157, { bptr->x, bptr->y, bptr->z });

			bptr->x -= (m_cos(bptr->angle << 1) * bptr->speed) >> 14;
			bptr->y -= GetRandomControl() & 3;
			bptr->z += (m_sin(bptr->angle << 1) * bptr->speed) >> 14;
			bptr->WingYoff += 11;
			bptr->WingYoff &= 63;

			if (bptr->life < 128)
			{
				bptr->y -= (i >> 1) + 4;

				if ((GetRandomControl() & 3) == 0)
				{
					bptr->angle += (GetRandomControl() & 255) - 128;
					bptr->angle &= 4095;
					bptr->speed += (GetRandomControl() & 3);
				}
			}

			bptr->speed += 12;

			if (bptr->speed > 300)
				bptr->speed = 300;

			if (bptr->life && (wibble & 4) && --bptr->life == 0)
				bptr->flags = 0;
		}
	}
}

void BatEmitterControl(short item_number)
{
	if (auto item = &items[item_number]; item->active)
	{
		TriggerBats(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->pos.y_rot >> 4);
		KillItem(item_number);
	}
}

void TriggerBats(long x, long y, long z, long angle)
{
	angle -= 0x400;
	angle &= 4095;

	auto bptr = &bats[0];

	for (int i = 0; i < MAX_BATS; ++i, ++bptr)
	{
		bptr->x = x + (GetRandomControl() & 511) - 256;
		bptr->y = y + 256 - (GetRandomControl() & 255);
		bptr->z = z + (GetRandomControl() & 511) - 256;
		bptr->angle = (angle + (GetRandomControl() & 127) - 64) & 4095;
		bptr->speed = (GetRandomControl() & 31) + 64;
		bptr->WingYoff = GetRandomControl() & 63;
		bptr->flags |= B_ON;
		bptr->life = 144 + (GetRandomControl() & 7);
	}
}

void TriggerStaticFlame(long x, long y, long z, long size)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sR = sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->sG = sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->sB = sptr->dB = 64;
	sptr->ColFadeSpeed = 1;
	sptr->FadeToBlack = 0;
	sptr->sLife = sptr->Life = 2;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 7) - 4);
	sptr->y = y;
	sptr->z = z + ((GetRandomControl() & 7) - 4);
	sptr->Xvel = sptr->Yvel = sptr->Zvel = sptr->Friction = sptr->Gravity = sptr->MaxYvel = 0;
	sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;
	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 2;
	sptr->Width = sptr->sWidth = size;
	sptr->dWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dHeight = size;
}

void TriggerDartSmoke(long x, long y, long z, long xv, long zv, long hit)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 63) + 72;
	
	sptr->On = 1;
	sptr->sR = 16;
	sptr->sG = 8;
	sptr->sB = 4;
	sptr->dR = 64;
	sptr->dG = 48;
	sptr->dB = 32;
	sptr->ColFadeSpeed = 8;
	sptr->FadeToBlack = 4;
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 32;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 31) - 16);
	sptr->y = y + ((GetRandomControl() & 31) - 16);
	sptr->z = z + ((GetRandomControl() & 31) - 16);

	if (hit)
	{
		sptr->Xvel = -xv + ((GetRandomControl() & 255) - 128);
		sptr->Yvel = -(GetRandomControl() & 3) - 4;
		sptr->Zvel = -zv + ((GetRandomControl() & 255) - 128);
		sptr->Friction = 3;
	}
	else
	{
		sptr->Xvel = (xv ? -xv : ((GetRandomControl() & 255) - 128));
		sptr->Yvel = -(GetRandomControl() & 3) - 4;
		sptr->Zvel = (xv ? -zv : ((GetRandomControl() & 255) - 128));
		sptr->Friction = 3;
	}

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 1;

	if (hit)
	{
		size >>= 1;
		sptr->Width = sptr->sWidth = size >> 2;
		sptr->Height = sptr->sHeight = size >> 2;
		sptr->Gravity = sptr->MaxYvel = 0;
	}
	else
	{
		sptr->Width = sptr->sWidth = size >> 4;
		sptr->Height = sptr->sHeight = size >> 4;
		sptr->Gravity = -(GetRandomControl() & 3) - 4;
		sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
	}

	sptr->dWidth = size;
	sptr->dHeight = size;
}

void KillAllCurrentItems(short item_number)
{
	KillEverythingFlag = 1;
}

void KillEverything()
{
	auto item_num = next_item_active;

	while (item_num != NO_ITEM)
	{
		auto next = items[item_num].next_active;

		if (items[item_num].object_number != LARA && items[item_num].object_number != FLARE && items[item_num].active && !(items[item_num].flags & REVERSE))
			KillItem(item_num);

		item_num = next;
	}

	item_num = next_fx_active;

	while (item_num != NO_ITEM)
	{
		auto next = effects[item_num].next_active;

		if (objects[effects[item_num].object_number].control && !(effects[item_num].object_number == FLAME && effects[item_num].counter < 0))
			KillEffect(item_num);

		item_num = next;
	}

	for (int i = 0; i < MAX_FISH; ++i)
		lead_info[i].on = 0;

	KillEverythingFlag = 0;
}

void TriggerBubble(long x, long y, long z, long size, long sizerange, short itemnum)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = 144;
	sptr->dG = 144;
	sptr->dB = 144;
	sptr->ColFadeSpeed = 4;
	sptr->FadeToBlack = 2;
	sptr->sLife = sptr->Life = 128;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = 0;
	sptr->y = 0;
	sptr->z = 0;
	sptr->Xvel = 0;
	sptr->Yvel = 0;
	sptr->Zvel = 0;
	sptr->Friction = 0;
	sptr->Flags = SP_SCALE | SP_DEF | SP_USEFXOBJPOS | SP_FX;
	sptr->FxObj = itemnum;
	sptr->Def = objects[BUBBLES1].mesh_index;
	sptr->Scalar = 0;
	sptr->Gravity = 0;
	sptr->MaxYvel = 0;

	size += (GetRandomControl() % sizerange);

	sptr->Width = sptr->sWidth = size;
	sptr->dWidth = size << 3;
	sptr->Height = sptr->sHeight = size;
	sptr->dHeight = size << 3;
}

void TriggerExplosionBubble(long x, long y, long z, short roomnum)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 7) + 63;

	sptr->On = 1;
	sptr->sR = 128;
	sptr->sG = 64;
	sptr->sB = 0;
	sptr->dR = 128;
	sptr->dG = 128;
	sptr->dB = 128;
	sptr->ColFadeSpeed = 8;
	sptr->FadeToBlack = 12;
	sptr->sLife = sptr->Life = 24;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x;
	sptr->y = y;
	sptr->z = z;
	sptr->Xvel = 0;
	sptr->Yvel = 0;
	sptr->Zvel = 0;
	sptr->Friction = 0;
	sptr->Flags = SP_SCALE | SP_DEF | SP_UNDERWEXP;
	sptr->Def = objects[BUBBLES1].mesh_index;
	sptr->Scalar = 3;
	sptr->Gravity = 0;
	sptr->MaxYvel = 0;
	sptr->Width = sptr->sWidth = size >> 1;
	sptr->dWidth = size << 1;
	sptr->Height = sptr->sHeight = size >> 1;
	sptr->dHeight = size << 1;

	for (int i = 0; i < 7; ++i)
	{
		PHD_3DPOS pos
		{
			x + (GetRandomControl() & 511) - 256,
			y + (GetRandomControl() & 127) - 64,
			z + (GetRandomControl() & 511) - 256
		};
		
		CreateBubble(&pos, roomnum, 6, 16);
	}
}

void TriggerBreath(long x, long y, long z, long xv, long yv, long zv)
{
	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 7) + 32;
	
	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = 32;
	sptr->dG = 32;
	sptr->dB = 32;
	sptr->ColFadeSpeed = 4;
	sptr->FadeToBlack = 32;
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 37;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = xv;
	sptr->Yvel = yv;
	sptr->Zvel = zv;
	sptr->Friction = 0;
	sptr->Flags = ((room[lara_item->room_number].flags & NOT_INSIDE) ? (SP_SCALE | SP_DEF | SP_EXPDEF | SP_WIND) : (SP_SCALE | SP_DEF | SP_EXPDEF));
	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;
	sptr->Gravity = 0;
	sptr->MaxYvel = 0;
	sptr->Width = sptr->sWidth = size >> 3;
	sptr->dWidth = size;
	sptr->Height = sptr->sHeight = size >> 3;
	sptr->dHeight = size;
}

void TriggerDynamicLight(long x, long y, long z, long falloff, long r, long g, long b)
{
	if (number_dynamics == MAX_DYNAMIC2)
		return;

	auto dlptr = &dynamics[number_dynamics];

	dlptr->on = 1;
	dlptr->x = x;
	dlptr->y = y;
	dlptr->z = z;
	dlptr->falloff = falloff << 8;
	dlptr->r = r;
	dlptr->g = g;
	dlptr->b = b;

	++number_dynamics;
}

void ClearDynamics()
{
	number_dynamics = 0;

	for (int i = 0; i < 32; ++i)
		dynamics[i].on = 0;
}

void ControlSmokeEmitter(int16_t item_number)
{
	auto item = &items[item_number];

	if (!TriggerActive(item) || (wibble & 12))
		return;

	if (item->object_number == STEAM_EMITTER && (wibble & 31))
		return;

	int dx = lara_item->pos.x_pos - item->pos.x_pos,
		dz = lara_item->pos.z_pos - item->pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 128;
	
	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = 32;
	sptr->dG = 32;
	sptr->dB = 32;
	sptr->ColFadeSpeed = 16 + (GetRandomControl() & 7);
	sptr->FadeToBlack = 64;
	sptr->sLife = sptr->Life = (GetRandomControl() & 15) + 96;
	sptr->TransType = (item->object_number == SMOKE_EMITTER_BLACK ? COLSUB : COLADD);
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = item->pos.x_pos + ((GetRandomControl() & 15) - 8);
	sptr->y = item->pos.y_pos + ((GetRandomControl() & 15) - 8);
	sptr->z = item->pos.z_pos + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 15) - 16;
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 4;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_WIND;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 7) - 4 : (GetRandomControl() & 7) + 4);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_WIND;

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;
	sptr->Gravity = -(GetRandomControl() & 7) - 8;
	sptr->MaxYvel = -(GetRandomControl() & 7) - 4;
	sptr->Width = sptr->sWidth = size >> 2;
	sptr->dWidth = size;

	size += (GetRandomControl() & 31) + 32;

	sptr->Height = sptr->sHeight = size >> 2;
	sptr->dHeight = size;

	if (item->object_number == STEAM_EMITTER)
	{
		sptr->Gravity >>= 1;
		sptr->MaxYvel >>= 1;
		sptr->Yvel >>= 1;
		sptr->dR = 24;
		sptr->dG = 24;
		sptr->dB = 24;
	}
}

void ControlColouredLights(int16_t item_number)
{
	static constexpr unsigned char cols[5][3] =
	{
		{ 31, 0, 0 },
		{ 0, 31, 0 },
		{ 0, 0, 31 },
		{ 31, 24, 0 },
		{ 28, 28, 31 }
	};

	auto item = &items[item_number];

	if (TriggerActive(item))
	{
		int itemnum = item->object_number - RED_LIGHT;

		TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 24, cols[itemnum][0], cols[itemnum][1], cols[itemnum][2]);
	}
}