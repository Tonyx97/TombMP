#include <specific/standard.h>
#include <specific/global.h>
#include <specific/litesrc.h>

#include "hwinsert.h"

uint32_t g_text_light[] =
{
	0xffffff,		// normal
	0x00b0b0,		// gold
	0xa0a0a0,		// grey
	0x6060ff,		// red
	0xff8080,		// blue
	0x4080c0,		// bronze
	0x64d1b6,
	0xc0ffc0,		// bronze
};

uint32_t g_text_dark[] =
{
	0x808080,		// normal
	0x005050,		// gold
	0x181818,		// grey
	0x000018,		// red
	0x180000,		// blue
	0x001040,		// bronze
	0x1320b6,
	0xc0ffc0,		// bronze
};

void S_DrawSprite(uint32_t dwFlags, int32_t nX, int32_t nY, int32_t nZ, int16_t* nSprite, int16_t nShade, int16_t nScale)
{
	int32_t xv, yv, zv;
	
	if (dwFlags & SPRITE_ABS)
	{
		nX -= w2v_matrix[M03];
		nY -= w2v_matrix[M13];
		nZ -= w2v_matrix[M23];

		if (nX<-phd_viewdist || nX>phd_viewdist ||
			nY<-phd_viewdist || nY>phd_viewdist ||
			nZ<-phd_viewdist || nZ>phd_viewdist)
			return;

		zv = w2v_matrix[M20] * nX + w2v_matrix[M21] * nY + w2v_matrix[M22] * nZ;
		if (zv < phd_znear || zv >= phd_zfar)
			return;

		xv = w2v_matrix[M00] * nX + w2v_matrix[M01] * nY + w2v_matrix[M02] * nZ;
		yv = w2v_matrix[M10] * nX + w2v_matrix[M11] * nY + w2v_matrix[M12] * nZ;
	}
	else
	{
		if ((nX | nY | nZ) == 0)
		{
			zv = phd_mxptr[M23];
			if (zv < phd_znear || zv > phd_zfar)
				return;

			xv = phd_mxptr[M03];
			yv = phd_mxptr[M13];
		}
		else
		{
			zv = phd_mxptr[M20] * nX + phd_mxptr[M21] * nY + phd_mxptr[M22] * nZ + phd_mxptr[M23];
			if (zv < phd_znear || zv > phd_zfar)
				return;

			xv = phd_mxptr[M00] * nX + phd_mxptr[M01] * nY + phd_mxptr[M02] * nZ + phd_mxptr[M03];
			yv = phd_mxptr[M10] * nX + phd_mxptr[M11] * nY + phd_mxptr[M12] * nZ + phd_mxptr[M13];
		}
	}

	auto zp = zv / phd_persp;
	auto sprite_info = (PHDSPRITESTRUCT*)nSprite;
	auto x1 = sprite_info->x1;
	auto y1 = sprite_info->y1;
	auto x2 = sprite_info->x2;
	auto y2 = sprite_info->y2;

	if (dwFlags & SPRITE_SCALE)
	{
		x1 = (x1 * nScale) << (W2V_SHIFT - 8);
		y1 = (y1 * nScale) << (W2V_SHIFT - 8);
		x2 = (x2 * nScale) << (W2V_SHIFT - 8);
		y2 = (y2 * nScale) << (W2V_SHIFT - 8);
	}
	else
	{
		x1 <<= W2V_SHIFT;
		y1 <<= W2V_SHIFT;
		x2 <<= W2V_SHIFT;
		y2 <<= W2V_SHIFT;
	}

	x1 = (int32_t)((xv + x1) / zp + phd_centerx); if (x1 >= phd_winwidth) return;
	y1 = (int32_t)((yv + y1) / zp + phd_centery); if (y1 >= phd_winheight) return;
	x2 = (int32_t)((xv + x2) / zp + phd_centerx); if (x2 < 0) return;
	y2 = (int32_t)((yv + y2) / zp + phd_centery); if (y2 < 0) return;

	if (dwFlags & SPRITE_SHADE)
	{
		if (zv > DPQ_S)	// Allow for depth-cueing of Sprite
		{
			nShade += (int16_t)((zv >> W2V_SHIFT) - DPQ_START);
			if (nShade > 0x1fff)
				return;
		}
	}
	else nShade = 0x1000;

	if (dwFlags & 0xffffff)
	{
		nShade = (int16_t)(dwFlags & 0xffffff);

		int r = (nShade & 0xff) >> 3,
			g = ((nShade >> 8) & 0xff) >> 3,
			b = ((nShade >> 16) & 0xff) >> 3;

		nShade = r << 10 | g << 5 | b;
	}

	InsertSprite(zv, x1, y1, x2, y2, nSprite, nShade, -1, (dwFlags & 0xffffff ? DRAW_TLV_GTA : DRAW_TLV_WGT), 0);
}

/**
* Insert scaled sprite off room data
*/
int16_t* ins_room_sprite(int16_t* objptr, int num)
{
	for (; num > 0; --num, objptr += 2)
	{
		auto vn = &vbuf[*(objptr)];
		if (vn->clip < 0)	// if z is behind us then ignore
			continue;

		auto sptr = &phdsprinfo[*(objptr + 1)];
		auto zv = (int32_t)vn->zv / phd_persp;
		auto x1 = (int32_t)((vn->xv + (sptr->x1 << W2V_SHIFT)) / zv + phd_centerx);
		auto y1 = (int32_t)((vn->yv + (sptr->y1 << W2V_SHIFT)) / zv + phd_centery);
		auto x2 = (int32_t)((vn->xv + (sptr->x2 << W2V_SHIFT)) / zv + phd_centerx);
		auto y2 = (int32_t)((vn->yv + (sptr->y2 << W2V_SHIFT)) / zv + phd_centery);

		if (x2 < phd_left || y2 < phd_top || x1 >= phd_right || y1 >= phd_bottom)
			continue;

		InsertSprite((int)vn->zv, x1, y1, x2, y2, (int16_t*)(phdsprinfo + objptr[1]), (int)vn->g, -1, DRAW_TLV_WGT, 0);
	}

	return objptr;
}

/**
* Draw sprite at specific screen location with scalable width and height
*/
void S_DrawScreenSprite2d(int32_t sx, int32_t sy, int32_t z, int32_t scaleH, int32_t scaleV, int16_t* sprnum, int16_t shade, uint16_t flags)
{
	auto sptr = (PHDSPRITESTRUCT*)sprnum;
	auto x1 = ((sptr->x1 * scaleH) >> 16) + sx;
	auto y1 = ((sptr->y1 * scaleV) >> 16) + sy;
	auto x2 = ((sptr->x2 * scaleH) >> 16) + sx;
	auto y2 = ((sptr->y2 * scaleV) >> 16) + sy;

	if (x2 < 0 || y2 < 0 || x1 >= phd_winwidth || y1 >= phd_winheight)
		return;

	int b = (g_text_light[shade] >> (16 + 3)) & 0x1f,
		g = (g_text_light[shade] >> (8 + 3)) & 0x1f,
		r = (g_text_light[shade] >> (3)) & 0x1f;

	int nShade1 = r << 10 | g << 5 | b;

	b = (g_text_dark[shade] >> (16 + 3)) & 0x1f;
	g = (g_text_dark[shade] >> (8 + 3)) & 0x1f;
	r = (g_text_dark[shade] >> (3)) & 0x1f;

	int nShade2 = r << 10 | g << 5 | b;

	auto sprite = (int16_t*)sptr;

	if (flags != 65535)
	{
		InsertSprite(phd_znear + (6 << 3), x1, y1, x2, y2, sprite, nShade1, nShade2, DRAW_TLV_WGT, 4);
		InsertSprite(phd_znear + (6 << 3), x1 + 2, y1 + 2, x2 + 2, y2 + 2, sprite, 0, 0, DRAW_TLV_WGT, 4);
	}
	else InsertSprite(phd_znear, x1, y1, x2, y2, sprite, nShade1, nShade2, DRAW_TLV_WGT, 0);
}

/**
* Draw sprite at specific screen location with scalable width and height
*/
void S_DrawScreenSprite(int32_t sx, int32_t sy, int32_t z, int32_t scaleH, int32_t scaleV, int16_t sprnum, int16_t shade, uint16_t flags)
{
	auto sptr = &phdsprinfo[sprnum];
	auto x1 = (((sptr->x1 >> 3) * scaleH) >> 16) + sx;
	auto x2 = (((sptr->x2 >> 3) * scaleH) >> 16) + sx;
	auto y1 = (((sptr->y1 >> 3) * scaleV) >> 16) + sy;
	auto y2 = (((sptr->y2 >> 3) * scaleV) >> 16) + sy;

	if (x2 < 0 || y2 < 0 || x1 >= phd_winwidth || y1 >= phd_winheight)
		return;

	InsertSprite(z << 3, x1, y1, x2, y2, (int16_t*)(phdsprinfo + sprnum), shade, -1, DRAW_TLV_WGT, 0);
}