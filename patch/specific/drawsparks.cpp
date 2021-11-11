#include "standard.h"
#include "global.h"
#include "output.h"

#include <3dsystem/hwinsert.h>

#include <game/effect2.h>
#include <game/effects.h>

#include <specific/fn_stubs.h>

#include <main.h>

void S_DrawSparks()
{
	auto [sw, sh] = g_window->get_resolution();

	auto sptr = &spark[0];

	for (int i = 0; i < MAX_SPARKS; ++sptr, ++i)
	{
		if (sptr->On == 0)
			continue;

		long wx, wy, wz;

		if (sptr->Flags & SP_FX)
		{
			auto fx = &effects[sptr->FxObj];
			wx = fx->pos.x_pos + sptr->x;
			wy = fx->pos.y_pos + sptr->y;
			wz = fx->pos.z_pos + sptr->z;
		}
		else if (sptr->Flags & SP_ITEM)
		{
			auto item = &items[sptr->FxObj];
			wx = item->pos.x_pos + sptr->x;
			wy = item->pos.y_pos + sptr->y;
			wz = item->pos.z_pos + sptr->z;
		}
		else
		{
			wx = sptr->x;
			wy = sptr->y;
			wz = sptr->z;
		}

		long result[XYZ],
			 scr[3][XYZ];

		mCalcPoint(wx, wy, wz, &result[0]);
		ProjectPCoord(result[_X], result[_Y], result[_Z], &scr[0][0], sw >> 1, sh >> 1, phd_persp);

		if (sptr->Flags & SP_DEF)
		{
			long w, h;

			if (sptr->Flags & SP_SCALE)
			{
				if (scr[0][_Z] == 0)
					++scr[0][_Z];

				w = ((sptr->Width * phd_persp) << sptr->Scalar) / (scr[0][_Z]);
				h = ((sptr->Height * phd_persp) << sptr->Scalar) / (scr[0][_Z]);

				if (w > sptr->Width << sptr->Scalar)
					w = sptr->Width << sptr->Scalar;
				else if (w < 4)
					w = 4;
				if (h > sptr->Height << sptr->Scalar)
					h = sptr->Height << sptr->Scalar;
				else if (h < 4)
					h = 4;

			}
			else
			{
				w = sptr->Width;
				h = sptr->Height;
			}

			auto z = scr[0][_Z] << W2V_SHIFT;

			if ((z < phd_znear || z > phd_zfar) ||
				(scr[0][_X] + (w >> 1) < 0) ||
				(scr[0][_X] - (w >> 1) > sw) ||
				(scr[0][_Y] + (h >> 1) < 0) ||
				(scr[0][_Y] - (h >> 1) > sh))
			{
				continue;
			}

			if (sptr->Flags & SP_ROTATE)
			{
				long sin = m_sin(sptr->RotAng << 1),
					 cos = m_cos(sptr->RotAng << 1),
					 sinx1 = ((-w >> 1) * sin) >> 12,
					 sinx2 = ((w >> 1) * sin) >> 12,
					 siny1 = ((-h >> 1) * sin) >> 12,
					 siny2 = ((h >> 1) * sin) >> 12,
					 cosx1 = ((-w >> 1) * cos) >> 12,
					 cosx2 = ((w >> 1) * cos) >> 12,
					 cosy1 = ((-h >> 1) * cos) >> 12,
					 cosy2 = ((h >> 1) * cos) >> 12;

				int x1 = sinx1 - cosy1 + scr[0][_X],
					x2 = sinx2 - cosy1 + scr[0][_X],
					x3 = sinx2 - cosy2 + scr[0][_X],
					x4 = sinx1 - cosy2 + scr[0][_X],
					y1 = cosx1 + siny1 + scr[0][_Y],
					y2 = cosx2 + siny1 + scr[0][_Y],
					y3 = cosx2 + siny2 + scr[0][_Y],
					y4 = cosx1 + siny2 + scr[0][_Y];

				int nShade = ((sptr->R >> 3) << 10) | ((sptr->G >> 3) << 5) | ((sptr->B >> 3));

				if (z > DPQ_S)
				{
					int r = sptr->R,
						g = sptr->G,
						b = sptr->B,
						v = 2048 - ((z - DPQ_S) >> 16);

					r *= v;
					g *= v;
					b *= v;

					r >>= 14;
					g >>= 14;
					b >>= 14;

					if (r < 0) r = 0;
					if (g < 0) g = 0;
					if (b < 0) b = 0;

					nShade = r << 10 | g << 5 | b;
				}

				const int type = (sptr->TransType == COLADD || sptr->TransType == COLSUB ? DRAW_TLV_GTA : DRAW_TLV_WGT);

				HWI_InsertAlphaSprite_Sorted(x1, y1, z, nShade,
											 x2, y2, z, nShade,
											 x3, y3, z, nShade,
											 x4, y4, z, nShade,
											 sptr->Def, type, 0);

				sptr->RotAng += sptr->RotAdd;
				sptr->RotAng &= 4095;

			}
			else
			{
				int x1 = scr[0][_X] - (w >> 1),
					x2 = scr[0][_X] + (w >> 1),
					y1 = scr[0][_Y] - (h >> 1),
					y2 = scr[0][_Y] + (h >> 1);

				int nShade = ((sptr->R >> 3) << 10) | ((sptr->G >> 3) << 5) | ((sptr->B >> 3));

				if (z > DPQ_S)
				{
					int r = sptr->R,
						g = sptr->G,
						b = sptr->B,
						v = 2048 - ((z - DPQ_S) >> 16);

					r *= v;
					g *= v;
					b *= v;

					r >>= 14;
					g >>= 14;
					b >>= 14;

					if (r < 0) r = 0;
					if (g < 0) g = 0;
					if (b < 0) b = 0;

					nShade = r << 10 | g << 5 | b;
				}

				const int type = (sptr->TransType == COLADD || sptr->TransType == COLSUB ? DRAW_TLV_GTA : DRAW_TLV_WGT);

				HWI_InsertAlphaSprite_Sorted(x1, y1, z, nShade,
											 x2, y1, z, nShade,
											 x2, y2, z, nShade,
											 x1, y2, z, nShade,
											 sptr->Def, type, 0);
			}
		}
		else
		{
			int w, h;

			if (sptr->Flags & SP_SCALE)
			{
				if (scr[0][_Z] == 0) scr[0][_Z] = 1;

				w = ((sptr->Width * phd_persp) << sptr->Scalar) / (scr[0][_Z]);
				h = ((sptr->Height * phd_persp) << sptr->Scalar) / (scr[0][_Z]);

				if (w > sptr->Width << 2) w = sptr->Width << 2;
				else if (w < 1)			  w = 1;

				if (h > sptr->Height << 2) h = sptr->Height << 2;
				else if (h < 1)			   h = 1;
			}
			else
			{
				w = sptr->Width;
				h = sptr->Height;
			}

			auto z = scr[0][_Z] << W2V_SHIFT;

			if ((z < phd_znear || z > phd_zfar) ||
				(scr[0][_X] + (w >> 1) < 0) ||
				(scr[0][_X] - (w >> 1) > sw) ||
				(scr[0][_Y] + (h >> 1) < 0) ||
				(scr[0][_Y] - (h >> 1) > sh))
			{
				continue;
			}

			int x1 = scr[0][_X] - (w >> 1),
				y1 = scr[0][_Y] - (h >> 1),
				x2 = x1 + w,
				y2 = y1 + h;

			int nShade = ((sptr->R >> 3) << 10) | ((sptr->G >> 3) << 5) | ((sptr->B >> 3));

			if (z > DPQ_S)
			{
				int r = sptr->R,
					g = sptr->G,
					b = sptr->B,
					v = 2048 - ((z - DPQ_S) >> 16);
				
				r *= v;
				g *= v;
				b *= v;

				r >>= 14;
				g >>= 14;
				b >>= 14;

				if (r < 0) r = 0;
				if (g < 0) g = 0;
				if (b < 0) b = 0;

				nShade = r << 10 | g << 5 | b;
			}

			const int type = (sptr->TransType == COLADD || sptr->TransType == COLSUB ? DRAW_TLV_GA : DRAW_TLV_G);

			HWI_InsertAlphaSprite_Sorted(x1, y1, z, nShade,
										 x2, y1, z, nShade,
										 x2, y2, z, nShade,
										 x1, y2, z, nShade,
										 nullptr, type, 0);
		}
	}
}