#include "standard.h"
#include "global.h"
#include "output.h"

#include <game/fish.h>
#include <game/objects.h>

#include <3dsystem/hwinsert.h>

#include <specific/fn_stubs.h>

#include <main.h>

void S_DrawFish(ITEM_INFO *item)
{
	auto [sw, sh] = g_window->get_resolution();

	int clipx = phd_winxmin + phd_winxmax,
		clipy = phd_winymin + phd_winymax;

	if (item->active == 0)
		return;

	auto leader = item->hit_points;
	if (leader == -1)
		return;

	if (!lead_info[leader].on)
		return;

	int32_t xoff = item->pos.x_pos,
		   yoff = item->pos.y_pos,
		   zoff = item->pos.z_pos;

	auto pSpriteInfo = (item->object_number == PIRAHNAS ? phdsprinfo + objects[EXPLOSION1].mesh_index + 10
														: phdsprinfo + objects[EXPLOSION1].mesh_index + 11);

	auto fptr = &fish[MAX_FISH + (leader * 24)];

	for (int i = 0; i < 24; ++i)
	{
		int angle = (fptr->angle + 2048 + m_sin(fptr->swim << 7, 5)) & 4095;

		int x = fptr->x + xoff,
			y = fptr->y + yoff,
			z = fptr->z + zoff;

		long result[XYZ],
			 scr[3][XYZ];

		mCalcPoint(x, y, z, &result[0]);
		ProjectPCoord(result[_X], result[_Y], result[_Z], &scr[0][0], sw >> 1, sh >> 1, phd_persp);

		int size = 192;

		z += (m_cos(angle << 1) * size) >> 12;
		x += -((m_sin(angle << 1) * size) >> 12);
		y -= size;

		mCalcPoint(x, y, z, &result[0]);
		ProjectPCoord(result[_X], result[_Y], result[_Z], &scr[1][0], sw >> 1, sh >> 1, phd_persp);

		y += size << 1;
		mCalcPoint(x, y, z, &result[0]);
		ProjectPCoord(result[_X], result[_Y], result[_Z], &scr[2][0], sw >> 1, sh >> 1, phd_persp);

		int x1 = scr[0][_X],
			x2 = scr[1][_X],
			x3 = scr[2][_X],
			y1 = scr[0][_Y],
			y2 = scr[1][_Y],
			y3 = scr[2][_Y],
			z1 = scr[0][_Z],
			z2 = scr[1][_Z],
			z3 = scr[2][_Z];

		if ((z1 > 0x5000) ||
			(z1 < 32 || z2 < 32 || z3 < 32) ||
			(x1 < phd_winxmin && x2 < phd_winxmin && x3 < phd_winxmin) ||
			(x1 >= clipx && x2 >= clipx && x3 >= clipx) ||
			(y1 < phd_winymin && y2 < phd_winymin && y3 < phd_winymin) ||
			(y1 >= clipy && y2 >= clipy && y3 >= clipy))
		{
			++fptr;
			continue;
		}

		if (angle < 1024)	   angle -= 512;
		else if (angle < 2048) angle -= 1536;
		else if (angle < 3072) angle -= 2560;
		else				   angle -= 3584;

		if (angle > 512 || angle < 0) angle = 0;
		else if (angle < 256)		  angle >>= 2;
		else						  angle = (512 - angle) >> 2;

		angle += i;

		if (angle > 128)
			angle = 128;

		z1 <<= W2V_SHIFT;
		z2 <<= W2V_SHIFT;
		z3 <<= W2V_SHIFT;

		auto rgb = 80 + angle;
		rgb = (rgb >> 3) << 10 | (rgb >> 3) << 5 | (rgb >> 3);

		int clipflag = 0;

		if (x1 < phd_winxmin) ++clipflag;
		else if (x1 > clipx)  clipflag += 2;

		if (y1 < phd_winymin) clipflag += 4;
		else if (y1 > clipy)  clipflag += 8;

		PHD_VBUF v[3];

		v[0].xs = (float)x1;
		v[0].ys = (float)y1;
		v[0].ooz = (f_persp / (float)z1) * f_oneopersp;
		v[0].clip = clipflag;
		v[0].g = rgb;

		clipflag = 0;

		if (x2 < phd_winxmin)
			clipflag++;
		else if (x2 > clipx)
			clipflag += 2;

		if (y2 < phd_winymin)
			clipflag += 4;
		else if (y2 > clipy)
			clipflag += 8;

		v[1].xs = (float)x2;
		v[1].ys = (float)y2;
		v[1].ooz = (f_persp / (float)z2) * f_oneopersp;
		v[1].clip = clipflag;
		v[1].g = rgb;

		clipflag = 0;

		if (x3 < phd_winxmin)
			clipflag++;
		else if (x3 > clipx)
			clipflag += 2;

		if (y3 < phd_winymin)
			clipflag += 4;
		else if (y3 > clipy)
			clipflag += 8;

		v[2].xs = (float)x3;
		v[2].ys = (float)y3;
		v[2].ooz = (f_persp / (float)z2) * f_oneopersp;
		v[2].clip = clipflag;
		v[2].g = rgb;

		v[0].zv = (float)z1;
		v[1].zv = (float)z2;
		v[2].zv = (float)z3;

		uint16_t u1 = ((pSpriteInfo->offset << 8) & 0xff00),
			   v1 = (pSpriteInfo->offset & 0xff00),
			   u2 = u1 + pSpriteInfo->width,
			   v2 = v1 + pSpriteInfo->height;

		u1 += App.nUVAdd;
		u2 -= App.nUVAdd;
		v1 += App.nUVAdd;
		v2 -= App.nUVAdd;

		PHDTEXTURESTRUCT Tex;

		Tex.drawtype = 1;
		Tex.tpage = pSpriteInfo->tpage;

		if (item->object_number == PIRAHNAS)
		{
			if (i & 1)
			{
				v[0].u = u1;
				v[0].v = v1;
				v[1].u = u2;
				v[1].v = v1;
				v[2].u = u1;
				v[2].v = v2;
			}
			else
			{
				v[0].u = u2;
				v[0].v = v2;
				v[1].u = u1;
				v[1].v = v2;
				v[2].u = u2;
				v[2].v = v1;
			}
		}
		else
		{
			if (leader & 1)
			{
				v[0].u = u1;
				v[0].v = v1;
				v[1].u = u2;
				v[1].v = v1;
				v[2].u = u1;
				v[2].v = v2;
			}
			else
			{
				v[0].u = u2;
				v[0].v = v2;
				v[1].u = u1;
				v[1].v = v2;
				v[2].u = u2;
				v[2].v = v1;
			}
		}

		InsertGT3(&v[0], &v[1], &v[2], &Tex, &v[0].u, &v[1].u, &v[2].u, MID_SORT, 1);

		++fptr;
	}
}