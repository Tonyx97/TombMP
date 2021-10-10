#include "standard.h"
#include "global.h"
#include "drawprimitive.h"
#include "init.h"

#include <3dsystem/3d_gen.h>

#include <game/room.h>
#include <game/draw.h>
#include <game/effect2.h>
#include <game/lara.h>

#include <specific/litesrc.h>
#include <specific/output.h>

#include <main.h>

float ZTable[DPQ_END * 2];

float m00, m01, m02, m03;
float m10, m11, m12, m13;
float m20, m21, m22, m23;

void InitZTable()
{
	for (int n = 0, z = 0; n < DPQ_END; ++z, ++n)
	{
		ZTable[(n * 2) + 0] = float(f_persp / float(z << 14));
		ZTable[(n * 2) + 1] = float(f_persp / float(z << 14)) * f_oneopersp;
	}
}

void MatrixToFloat(int32_t* mptr)
{
	m00 = float(mptr[M00]);
	m01 = float(mptr[M01]);
	m02 = float(mptr[M02]);
	m03 = float(mptr[M03]);

	m10 = float(mptr[M10]);
	m11 = float(mptr[M11]);
	m12 = float(mptr[M12]);
	m13 = float(mptr[M13]);

	m20 = float(mptr[M20]);
	m21 = float(mptr[M21]);
	m22 = float(mptr[M22]);
	m23 = float(mptr[M23]);
}

int16_t* calc_object_vertices(int16_t* objptr)
{
	double fconv;

	float f_mid_sort = App.lpZBuffer ? 0.f : (float)(mid_sort << W2V_SHIFT + 8);

	auto mptr = phd_mxptr;
	auto dest = vbuf;

	int8_t totclip = -1;

	auto temp_objptr = objptr + 2;

	MatrixToFloat(mptr);

	for (int16_t numvert = (int16_t)objptr[1]; numvert > 0; --numvert, ++dest, temp_objptr += 3)
	{
		float zv = m20 * float(temp_objptr[0]) + m21 * float(temp_objptr[1]) + m22 * float(temp_objptr[2]) + m23;

		int32_t z = dest->z = FTOL(zv);

		int8_t clipflag;

		dest->xv = m00 * float(temp_objptr[0]) + m01 * float(temp_objptr[1]) + m02 * float(temp_objptr[2]) + m03;
		dest->yv = m10 * float(temp_objptr[0]) + m11 * float(temp_objptr[1]) + m12 * float(temp_objptr[2]) + m13;

		if (z >= phd_znear)
		{
			if (z >= phd_zfar)
			{
				dest->zv = f_zfar;

				zv = f_persp / zv;
				dest->xs = dest->xv * zv + f_centerx;
				dest->ys = dest->yv * zv + f_centery;

				if (g_no_depth > 0.f)
					dest->ooz = g_no_depth + zv * f_oneopersp;
				else dest->ooz = zv * f_oneopersp;
			}
			else
			{
				float lz = ZTable[((z >> 14) << 1) + 0];

				dest->zv = zv + f_mid_sort;
				dest->xs = dest->xv * lz + f_centerx;
				dest->ys = dest->yv * lz + f_centery;

				if (g_no_depth > 0.f)
					dest->ooz = g_no_depth + ZTable[((z >> 14) << 1) + 1];
				else dest->ooz = ZTable[((z >> 14) << 1) + 1];
			}

			clipflag = 0;

			if (dest->xs < phd_leftfloat)		++clipflag;
			else if (dest->xs > phd_rightfloat) clipflag += 2;

			if (dest->ys < phd_topfloat)		 clipflag += 4;
			else if (dest->ys > phd_bottomfloat) clipflag += 8;
		}
		else
		{
			dest->zv = zv;
			clipflag = -128;
		}

		dest->clip = clipflag;
		totclip &= clipflag;
	}

	return (totclip ? nullptr : temp_objptr);
}

int16_t* calc_roomvert(int16_t* sobjptr, int far_clip)
{
	double fconv;

	auto this_room = &room[CurrentRoom];
	auto mptr = phd_mxptr;
	auto dest = vbuf;

	int nRoomVerts = (int)*(sobjptr++);

	auto objptr = sobjptr;

	float f_mid_sort = App.lpZBuffer ? 0.f : (float)(mid_sort << W2V_SHIFT + 8);

	int8_t clipflag;

	MatrixToFloat(mptr);

	for (int numvert = nRoomVerts; numvert > 0; --numvert, ++dest, objptr += 6)
	{
		float zv;

		dest->xv = m00 * float(objptr[0]) + m01 * float(objptr[1]) + m02 * float(objptr[2]) + m03;
		dest->yv = m10 * float(objptr[0]) + m11 * float(objptr[1]) + m12 * float(objptr[2]) + m13;
		dest->zv = zv = m20 * float(objptr[0]) + m21 * float(objptr[1]) + m22 * float(objptr[2]) + m23;
		dest->g = objptr[5];

		int32_t z = FTOL(zv);

		if (zv >= f_znear)
		{
			dest->zv += f_mid_sort;
			zv = f_persp / zv;
			clipflag = 0;

			if (zv <= DPQ_E)
				dest->ooz = zv * f_oneopersp;
			else
			{
				clipflag = far_clip;
				dest->zv = f_zfar;
				dest->ooz = f_oneozfar;
			}

			dest->xs = dest->xv * zv + f_centerx;
			dest->ys = dest->yv * zv + f_centery;

			uint32_t r = dest->g & 0x7c00,
				   g = dest->g & 0x03e0,
				   b = dest->g & 0x001f;

			for (int i = 0; i < number_dynamics; ++i)
			{
				if (dynamics[i].on)
				{
					int x1 = (dynamics[i].x - (objptr[0] + this_room->x)) >> 7,
						y1 = (dynamics[i].y - (objptr[1] + this_room->y)) >> 7,
						z1 = (dynamics[i].z - (objptr[2] + this_room->z)) >> 7;

					if (int mod = (x1 * x1) + (y1 * y1) + (z1 * z1); mod >= 0 && mod < 1024)
					{
						if (int dist = SqrTable[mod], falloff = dynamics[i].falloff >> 8; dist < falloff)
						{
							r += RColorTable[falloff][dist][dynamics[i].r];
							g += GColorTable[falloff][dist][dynamics[i].g];
							b += BColorTable[falloff][dist][dynamics[i].b];
						}
					}
				}
			}

			if (r > 0x7c00) r = 0x7c00;
			if (g > 0x03e0) g = 0x03e0;
			if (b > 0x001f) b = 0x001f;

			dest->g = r | g | b;

			int off = (((objptr[0] + this_room->x) >> 6) +
					   ((objptr[1] + this_room->y) >> 6) +
					   ((objptr[2] + this_room->z) >> 7)) & 0xfc;

			if (objptr[4] & (1 << 13))
			{
				dest->yv += float(WaterTable[this_room->MeshEffect][((WaterTable[this_room->MeshEffect][off & 63].random + (wibble >> 2)) & 63)].choppy << W2V_SHIFT);

				int r = ((dest->g >> 10) & 0x1f),
					g = ((dest->g >> 5) & 0x1f),
					b = ((dest->g) & 0x1f),
					shade = -WaterTable[this_room->MeshEffect][((WaterTable[this_room->MeshEffect][off & 63].random + (wibble >> 2)) & 63)].choppy >> 4;

				r += shade;
				g += shade;
				b += shade;

				if (r > 0x1f) r = 0x1f;
				if (g > 0x1f) g = 0x1f;
				if (b > 0x1f) b = 0x1f;

				if (r < 0) r = 0;
				if (g < 0) g = 0;
				if (b < 0) b = 0;

				dest->g = r << 10 | g << 5 | b;
			}

			if (objptr[4] & (1 << 14))
			{
				int r = ((dest->g >> 10) & 0x1f),
					g = ((dest->g >> 5) & 0x1f),
					b = ((dest->g) & 0x1f),
					shade = WaterTable[this_room->MeshEffect][((WaterTable[this_room->MeshEffect][off & 63].random + (wibble >> 2)) & 63)].shimmer;

				shade += WaterTable[this_room->MeshEffect][((WaterTable[this_room->MeshEffect][off & 63].random + (wibble >> 2)) & 63)].abs;

				r += shade;
				g += shade;
				b += shade;

				if (r > 0x1f) r = 0x1f;
				if (g > 0x1f) g = 0x1f;
				if (b > 0x1f) b = 0x1f;

				if (r < 0) r = 0;
				if (g < 0) g = 0;
				if (b < 0) b = 0;

				dest->g = r << 10 | g << 5 | b;
			}

			if (water_effect && objptr[4] >= 0)
				dest->ys += wibble_table[(FTOL(dest->xs) + wibble >> 3) & (WIBBLE_SIZE - 1)];

			if (dest->xs < phd_leftfloat)		++clipflag;
			else if (dest->xs > phd_rightfloat) clipflag += 2;

			if (dest->ys < phd_topfloat)		 clipflag += 4;
			else if (dest->ys > phd_bottomfloat) clipflag += 8;

			dest->clip = clipflag;

		}
		else dest->clip = -128;

		// apply depth Q

		if (z > DPQ_S)
		{
			int r = ((dest->g >> 10) & 0x1f) << 3,
				g = ((dest->g >> 5) & 0x1f) << 3,
				b = (dest->g & 0x1f) << 3,
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

			dest->g = r << 10 | g << 5 | b;
		}
	}

	return objptr;
}