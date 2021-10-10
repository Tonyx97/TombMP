#include "standard.h"
#include "global.h"
#include "output.h"

#include <3dsystem/hwinsert.h>

#include <game/effect2.h>
#include <game/objects.h>

#include <main.h>

// Table of which points make up which faces.
unsigned char SplashLinks[8 * 4] =
{
	 8 << 2,  9 << 2, 0 << 2, 1 << 2,
	 9 << 2, 10 << 2, 1 << 2, 2 << 2,
	10 << 2, 11 << 2, 2 << 2, 3 << 2,
	11 << 2, 12 << 2, 3 << 2, 4 << 2,
	12 << 2, 13 << 2, 4 << 2, 5 << 2,
	13 << 2, 14 << 2, 5 << 2, 6 << 2,
	14 << 2, 15 << 2, 6 << 2, 7 << 2,
	15 << 2,  8 << 2, 7 << 2, 0 << 2
};

void S_DrawSplashes()
{
	long mscrpoints[48 * 4],
		 mresult[3];

	long* result,
		* scrpoints;

	auto [sw, sh] = g_window->get_resolution();

	auto sptr = &splashes[0];

	for (int i = 0; i < MAX_SPLASHES; ++sptr, ++i)
	{
		if (!(sptr->flags & SPL_ON))
			continue;

		auto svptr = &sptr->sv[0];

		scrpoints = (long*)&mscrpoints;
		result = (long*)&mresult;

		for (int j = 0; j < 48; ++svptr, ++j)
		{
			mCalcPoint((svptr->wx >> 4) + sptr->x, svptr->wy + sptr->y, (svptr->wz >> 4) + sptr->z, result);
			ProjectPCoord(result[_X], result[_Y], result[_Z], scrpoints, sw >> 1, sh >> 1, phd_persp);

			scrpoints += 4;
		}

		scrpoints = (long*)&mscrpoints;

		for (int j = 0; j < 3; ++j)
		{
			int nSprite = (j == 2 || (j == 0 && (sptr->flags & SPL_RIPPLEINNER)) || (j == 1 && (sptr->flags & SPL_RIPPLEMIDDLE)))
							? objects[EXPLOSION1].mesh_index + 4 + ((wibble >> 4) & 3)
							: objects[EXPLOSION1].mesh_index + 8;

			auto pptr = &SplashLinks[0];

			auto linkadd = j << 6;

			for (int k = 0; k < 8; ++k)
			{
				int x1 = scrpoints[(*pptr) + linkadd],
					y1 = scrpoints[(*pptr) + 1 + linkadd],
					z1 = scrpoints[(*pptr++) + 2 + linkadd],
					x2 = scrpoints[(*pptr) + linkadd],
					y2 = scrpoints[(*pptr) + 1 + linkadd],
					z2 = scrpoints[(*pptr++) + 2 + linkadd],
					x3 = scrpoints[(*pptr) + linkadd],
					y3 = scrpoints[(*pptr) + 1 + linkadd],
					z3 = scrpoints[(*pptr++) + 2 + linkadd],
					x4 = scrpoints[(*pptr) + linkadd],
					y4 = scrpoints[(*pptr) + 1 + linkadd],
					z4 = scrpoints[(*pptr++) + 2 + linkadd];

				if ((x1 < 0 && x2 < 0 && x3 < 0 && x4 < 0) ||
					(x1 >= sw && x2 >= sw && x3 >= sw && x4 >= sw) ||
					(y1 < 0 && y2 < 0 && y3 < 0 && y4 < 0) ||
					(y1 >= sh && y2 >= sh && y3 >= sh && y4 >= sh))
				{
					continue;
				}

				z1 <<= W2V_SHIFT;
				if (z1 < phd_znear || z1 > phd_zfar) continue;

				z2 <<= W2V_SHIFT;
				if (z2 < phd_znear || z2 > phd_zfar) continue;

				z3 <<= W2V_SHIFT;
				if (z3 < phd_znear || z3 > phd_zfar) continue;

				z4 <<= W2V_SHIFT;
				if (z4 < phd_znear || z4 > phd_zfar) continue;

				int a = (sptr->life >> 2),
					b = (sptr->life >> 2) - (sptr->life >> 4);

				int nShade1 = a << 10 | a << 5 | a,
					nShade2 = b << 10 | b << 5 | b;

				if ((((x3 - x2) * (y1 - y2)) - ((x1 - x2) * (y3 - y2))) < 0)
					HWI_InsertAlphaSprite_Sorted(x1, y1, z1, nShade1,
												 x3, y3, z3, nShade2,
												 x4, y4, z4, nShade2,
												 x2, y2, z2, nShade1,
												 nSprite, DRAW_TLV_GTA, 1);
				else HWI_InsertAlphaSprite_Sorted(x1, y1, z1, nShade1,
												  x2, y2, z2, nShade1,
												  x4, y4, z4, nShade2,
												  x3, y3, z3, nShade2,
												  nSprite, DRAW_TLV_GTA, 0);
			}
		}
	}

	result = (long*)&mresult;

	auto rptr = &ripples[0];

	for (int i = 0; i < MAX_RIPPLES; ++rptr, ++i)
	{
		if (!(rptr->flags & SPL_ON))
			continue;

		auto size = rptr->size << 2,
			 nSprite = objects[EXPLOSION1].mesh_index + 9;

		scrpoints = (long*)&mscrpoints;
		mCalcPoint(rptr->x - size, rptr->y, rptr->z - size, result);
		ProjectPCoord(result[_X], result[_Y], result[_Z], scrpoints, sw >> 1, sh >> 1, phd_persp);

		scrpoints += 3;
		mCalcPoint(rptr->x + size, rptr->y, rptr->z - size, result);
		ProjectPCoord(result[_X], result[_Y], result[_Z], scrpoints, sw >> 1, sh >> 1, phd_persp);

		scrpoints += 3;
		mCalcPoint(rptr->x - size, rptr->y, rptr->z + size, result);
		ProjectPCoord(result[_X], result[_Y], result[_Z], scrpoints, sw >> 1, sh >> 1, phd_persp);

		scrpoints += 3;
		mCalcPoint(rptr->x + size, rptr->y, rptr->z + size, result);
		ProjectPCoord(result[_X], result[_Y], result[_Z], scrpoints, sw >> 1, sh >> 1, phd_persp);

		scrpoints -= 9;

		int x1 = *scrpoints++,
			y1 = *scrpoints++,
			z1 = *scrpoints++,
			x2 = *scrpoints++,
			y2 = *scrpoints++,
			z2 = *scrpoints++,
			x3 = *scrpoints++,
			y3 = *scrpoints++,
			z3 = *scrpoints++,
			x4 = *scrpoints++,
			y4 = *scrpoints++,
			z4 = *scrpoints;

		if ((x1 < 0 && x2 < 0 && x3 < 0 && x4 < 0) ||
			(x1 >= sw && x2 >= sw && x3 >= sw && x4 >= sw) ||
			(y1 < 0 && y2 < 0 && y3 < 0 && y4 < 0) ||
			(y1 >= sh && y2 >= sh && y3 >= sh && y4 >= sh))
		{
			continue;
		}

		z1 <<= W2V_SHIFT;
		if (z1 < phd_znear || z1 > phd_zfar) continue;

		z2 <<= W2V_SHIFT;
		if (z2 < phd_znear || z2 > phd_zfar) continue;

		z3 <<= W2V_SHIFT;
		if (z3 < phd_znear || z3 > phd_zfar) continue;

		z4 <<= W2V_SHIFT;
		if (z4 < phd_znear || z4 > phd_zfar) continue;

		int nShade = 0;

		if (rptr->flags & SPL_MORETRANS)
		{
			if (rptr->flags & SPL_BLOOD)
			{
				nSprite = objects[EXPLOSION1].mesh_index;
				nShade = (rptr->init ? (rptr->init >> 1) << 10 : (rptr->life >> 1) << 10);
			}
			else nShade = (rptr->init ? (rptr->init >> 2) << 10 | (rptr->init >> 2) << 5 | rptr->init >> 2
									  : (rptr->life >> 2) << 10 | (rptr->life >> 2) << 5 | rptr->life >> 2);
		}
		else nShade = (rptr->init ? (rptr->init >> 1) << 10 | (rptr->init >> 1) << 5 | rptr->init >> 1
								  : (rptr->life >> 1) << 10 | (rptr->life >> 1) << 5 | rptr->life >> 1);

		if ((((x3 - x2) * (y1 - y2)) - ((x1 - x2) * (y3 - y2))) < 0)
			HWI_InsertAlphaSprite_Sorted(x1, y1, z1, nShade,
										 x3, y3, z3, nShade,
										 x4, y4, z4, nShade,
										 x2, y2, z2, nShade,
										 nSprite, DRAW_TLV_GTA, 1);
		else HWI_InsertAlphaSprite_Sorted(x1, y1, z1, nShade,
										  x2, y2, z2, nShade,
										  x4, y4, z4, nShade,
										  x3, y3, z3, nShade,
										  nSprite, DRAW_TLV_GTA, 0);
	}
}