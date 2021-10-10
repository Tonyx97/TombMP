#include "standard.h"
#include "global.h"
#include "hwrender.h"
#include "drawprimitive.h"

#include <game/lasers.h>

#include <main.h>

bool ClipLine(long& x1, long& y1, long& x2, long& y2, int sw, int sh)
{
	if (x1 < 0 && x2 < 0)	return false;
	if (y1 < 0 && y2 < 0)	return false;
	if (x1 > sw && x2 > sw) return false;
	if (y1 > sh && y2 > sh) return false;

	float clipper;

	if (x1 > sw)
	{
		clipper = ((float)sw - x2) / (float)(x1 - x2);
		x1 = sw;
		y1 = (long)((float)(y2 + (y1 - y2)) * clipper);
	}

	if (x2 > sw)
	{
		clipper = ((float)sw - x1) / (float)(x2 - x1);
		x2 = sw;
		y2 = (long)((float)(y1 + (y2 - y1)) * clipper);
	}

	if (x1 < 0)
	{
		clipper = (0.f - x1) / (float)(x2 - x1);
		x1 = 0;
		y1 = (long)((float)(y1 + (y2 - y1)) * clipper);
	}

	if (x2 < 0)
	{
		clipper = (0.f - x2) / (float)(x1 - x2);
		x2 = 0;
		y2 = (long)((float)(y2 + (y1 - y2)) * clipper);
	}

	if (y1 > sh)
	{
		clipper = ((float)sh - y2) / (float)(y1 - y2);
		y1 = sh;
		x1 = (long)((float)(x2 + (x1 - x2)) * clipper);
	}

	if (y2 > sh)
	{
		clipper = ((float)sh - y1) / (float)(y2 - y1);
		y2 = sh;
		x2 = (long)((float)(x1 + (x2 - x1)) * clipper);
	}

	if (y1 < 0)
	{
		clipper = (0.f - y1) / (float)(y2 - y1);
		y1 = 0;
		x1 = (long)((float)(x1 + (x2 - x1)) * clipper);
	}

	if (y2 < 0)
	{
		clipper = (0.f - y2) / (float)(y1 - y2);
		y2 = 0;
		x2 = (long)((float)(x2 + (x1 - x2)) * clipper);
	}

	return true;
}

// fix this function, doesn't work, all lasers are random and come from everywhere
void S_DrawLaserBeam(GAME_VECTOR* src, GAME_VECTOR* dest, uint8_t r, uint8_t g, uint8_t b)
{
	long screencoords[3 * 200];
	long rgbs[3 * 200];

	auto [sw, sh] = g_window->get_resolution();

	--sw; --sh;

	UpdateLaserShades();

	phd_PushMatrix();
	phd_TranslateAbs(src->x, src->y, src->z);

	int dx = (src->x - dest->x),
		dz = (src->z - dest->z);

	dx = phd_sqrt((dx * dx) + (dz * dz));

	auto segments = std::clamp(dx >> 9, 8, 32),
		 dy = (dest->y - src->y) / segments;

	dx = (dest->x - src->x) / segments;
	dz = (dest->z - src->z) / segments;

	long wx = 0,
		 wy = 0,
		 wz = 0;

	for (int i = 0; i < segments + 1; ++i)
	{
		auto mptr = phd_mxptr;

		auto x1 = (mptr[M00] * wx + mptr[M01] * wy + mptr[M02] * wz + mptr[M03]),
			 y1 = (mptr[M10] * wx + mptr[M11] * wy + mptr[M12] * wz + mptr[M13]),
			 z1 = (mptr[M20] * wx + mptr[M21] * wy + mptr[M22] * wz + mptr[M23]);

		float zv = f_persp / (float)z1;
		screencoords[(i * 3) + 0] = (short)((float)(x1 * zv + f_centerx));
		screencoords[(i * 3) + 1] = (short)((float)(y1 * zv + f_centery));
		screencoords[(i * 3) + 2] = z1 >> W2V_SHIFT;

		wx += dx;
		wy += dy;
		wz += dz;

		if (i == 0 || i == segments)
		{
			rgbs[(i * 3) + _X] = 0;
			rgbs[(i * 3) + _Y] = 0;
			rgbs[(i * 3) + _Z] = 0;
		}
		else
		{
			const auto rnd = LaserShades[i];

			rgbs[(i * 3) + _X] = (r == 255 ? rnd + 32 : rnd >> r);
			rgbs[(i * 3) + _Y] = (g == 255 ? rnd + 32 : rnd >> g);
			rgbs[(i * 3) + _Z] = (b == 255 ? rnd + 32 : rnd >> b);
		}
	}

	auto x1 = screencoords[0 + _X],
		 y1 = screencoords[0 + _Y],
		 z1 = screencoords[0 + _Z],
		 r1 = rgbs[0],
		 g1 = rgbs[1],
		 b1 = rgbs[2];

	for (int i = 0; i < segments; ++i)
	{
		auto x2 = screencoords[((i + 1) * 3) + _X],
			 y2 = screencoords[((i + 1) * 3) + _Y],
			 z2 = screencoords[((i + 1) * 3) + _Z],
			 r2 = rgbs[((i + 1) * 3) + 0],
			 g2 = rgbs[((i + 1) * 3) + 1],
			 b2 = rgbs[((i + 1) * 3) + 2];

		if (z1 > 32 && z2 > 32)
		{
			if (ClipLine(x1, y1, x2, y2, sw, sh))
			{
				if (x1 >= 0 && x1 <= sw && y1 >= 0 && y1 <= sh && x2 >= 0 && x2 <= sw && y2 >= 0 && y2 <= sh)
				{
					D3DTLVERTEX v[2];

					v[0].sx = (float)x1;
					v[0].sy = (float)y1;
					v[0].sz = f_a - f_boo * (one / (float)(z1 << W2V_SHIFT));

					v[1].sx = (float)x2;
					v[1].sy = (float)y2;
					v[1].sz = f_a - f_boo * (one / (float)(z2 << W2V_SHIFT));

					v[0].color = RGB_MAKE(r1, g1, b1);
					v[1].color = RGB_MAKE(r2, g2, b2);

					v[0].specular = 0;
					v[1].specular = 0;

					v[0].rhw = v[0].sz;
					v[1].rhw = v[1].sz;

					HWR_SetCurrentTexture(0);
					HWR_EnableZBuffer(true, true);

					DrawPrimitive(D3DPT_LINESTRIP, D3DVT_TLVERTEX, &v, 2, D3DDP_DONOTCLIP);

					HWR_EnableZBuffer(false, false);
				}
			}
		}

		x1 = x2;
		y1 = y2;
		z1 = z2;
		r1 = r2;
		g1 = g2;
		b1 = b2;
	}

	phd_PopMatrix();
}