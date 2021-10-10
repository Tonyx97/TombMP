#include "standard.h"
#include "global.h"

#include <main.h>

#include <3dsystem/hwinsert.h>

#include <game/footprint.h>
#include <game/objects.h>

PHD_VECTOR FV1 {    0, 0, -64 };
PHD_VECTOR FV2 { -128, 0,  64 };
PHD_VECTOR FV3 {  128, 0,  64 };

void S_DrawFootPrints()
{
	auto [sw, sh] = g_window->get_resolution();

	auto f = &FootPrint[0];

	auto pSpriteInfo = phdsprinfo + objects[EXPLOSION1].mesh_index + 17;

	uint16_t u1 = (pSpriteInfo->offset << 8) & 0xff00,
		     v1 = pSpriteInfo->offset & 0xff00,
		     u2 = u1 + pSpriteInfo->width,
		     v2 = v1 + pSpriteInfo->height;

	u1 += App.nUVAdd;
	u2 -= App.nUVAdd;
	v1 += App.nUVAdd;
	v2 -= App.nUVAdd;

	for (int i = 0; i < MAX_FOOTPRINTS; ++i, ++f)
	{
		if (f->Active)
		{
			--f->Active;

			int16_t rgb = (f->Active > 28 ? 56 << 1 : f->Active << 2);

			phd_PushMatrix();
			phd_TranslateAbs(f->x, f->y, f->z);
			phd_RotY(f->YRot);

			auto mptr = phd_mxptr;

			int x1 = (mptr[M00] * FV1.x + mptr[M01] * FV1.y + mptr[M02] * FV1.z + mptr[M03]),
				y1 = (mptr[M10] * FV1.x + mptr[M11] * FV1.y + mptr[M12] * FV1.z + mptr[M13]),
				z1 = (mptr[M20] * FV1.x + mptr[M21] * FV1.y + mptr[M22] * FV1.z + mptr[M23]);

			float zv = f_persp / (float)z1;

			x1 = (short)((float)(x1 * zv + f_centerx));
			y1 = (short)((float)(y1 * zv + f_centery));

			int x2 = (mptr[M00] * FV2.x + mptr[M01] * FV2.y + mptr[M02] * FV2.z + mptr[M03]),
				y2 = (mptr[M10] * FV2.x + mptr[M11] * FV2.y + mptr[M12] * FV2.z + mptr[M13]),
				z2 = (mptr[M20] * FV2.x + mptr[M21] * FV2.y + mptr[M22] * FV2.z + mptr[M23]);

			zv = f_persp / (float)z2;

			x2 = (short)((float)(x2 * zv + f_centerx));
			y2 = (short)((float)(y2 * zv + f_centery));

			int x3 = (mptr[M00] * FV3.x + mptr[M01] * FV3.y + mptr[M02] * FV3.z + mptr[M03]),
				y3 = (mptr[M10] * FV3.x + mptr[M11] * FV3.y + mptr[M12] * FV3.z + mptr[M13]),
				z3 = (mptr[M20] * FV3.x + mptr[M21] * FV3.y + mptr[M22] * FV3.z + mptr[M23]);

			zv = f_persp / (float)z3;

			x3 = (short)((float)(x3 * zv + f_centerx));
			y3 = (short)((float)(y3 * zv + f_centery));

			phd_PopMatrix();

			if (z1 >> W2V_SHIFT < 32 || z2 >> W2V_SHIFT < 32 || z3 >> W2V_SHIFT < 32)
				continue;

			int clipflag = 0;

			if (x1 < 0)		  ++clipflag;
			else if (x1 > sw) clipflag += 2;

			if (y1 < 0)		  clipflag += 4;
			else if (y1 > sh) clipflag += 8;

			PHD_VBUF v[3];

			v[0].xs = float(x1);
			v[0].ys = float(y1);
			v[0].ooz = (f_persp / (float)z1) * f_oneopersp;
			v[0].clip = clipflag;
			v[0].g = (rgb >> 3) << 10 | (rgb >> 3) << 5 | (rgb >> 3);

			clipflag = 0;

			if (x2 < 0)		  ++clipflag;
			else if (x2 > sw) clipflag += 2;

			if (y2 < 0)		  clipflag += 4;
			else if (y2 > sh) clipflag += 8;

			v[1].xs = (float)x2;
			v[1].ys = (float)y2;
			v[1].ooz = (f_persp / (float)z2) * f_oneopersp;
			v[1].clip = clipflag;
			v[1].g = (rgb >> 3) << 10 | (rgb >> 3) << 5 | (rgb >> 3);

			clipflag = 0;

			if (x3 < 0)		  ++clipflag;
			else if (x3 > sw) clipflag += 2;

			if (y3 < 0)		  clipflag += 4;
			else if (y3 > sh) clipflag += 8;

			v[2].xs = (float)x3;
			v[2].ys = (float)y3;
			v[2].ooz = (f_persp / (float)z3) * f_oneopersp;
			v[2].clip = clipflag;
			v[2].g = (rgb >> 3) << 10 | (rgb >> 3) << 5 | (rgb >> 3);

			v[0].zv = (float)z1;
			v[1].zv = (float)z2;
			v[2].zv = (float)z3;

			v[0].u = u1;
			v[0].v = v1;
			v[1].u = u2;
			v[1].v = v1;
			v[2].u = u1;
			v[2].v = v2;

			PHDTEXTURESTRUCT Tex;

			Tex.drawtype = 2;
			Tex.tpage = pSpriteInfo->tpage;

			InsertGT3(&v[0], &v[1], &v[2], &Tex, &v[0].u, &v[1].u, &v[2].u, MID_SORT, 1);
		}
	}
}