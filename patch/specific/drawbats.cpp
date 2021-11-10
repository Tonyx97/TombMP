#include "standard.h"
#include "global.h"

#include <3dsystem/hwinsert.h>

#include <game/effect2.h>
#include <game/objects.h>

#include <specific/fn_stubs.h>

#include <main.h>

#define BATCOLOR (250 >> 3) << 10 | (160 >> 3) << 5 | (100 >> 3)

short BatMesh[5][3] = { { -48 << 2, 0, -12 << 2	},		//
						{ -48 << 2, 0,  12 << 2	},		// Body.
						{  24 << 2, 0, 0 },				//
						{ -36 << 2, 0, -48 << 2	},		// Left wing.
						{ -36 << 2, 0,  48 << 2	} };	// Right wing.

unsigned char BatLinks[9] = { 0 << 1, 1 << 1, 2 << 1,	//
							  3 << 1, 0 << 1, 2 << 1,	// Face links.
							  1 << 1, 4 << 1, 2 << 1 };	//


void S_DrawBat()
{
	short myscrxy[10],
		  myTempMesh[3];

	long myscrz[10],
		 result[3];

	auto [sw, sh] = g_window->get_resolution();

	auto scrxy = (short*)&myscrxy,
		 TempMesh = (short*)&myTempMesh;

	auto scrz = (long*)&myscrz;

	auto bptr = &bats[0];

	for (int i = 0; i < MAX_BATS; ++i)
	{
		if (bptr->flags & B_ON)
		{
			phd_PushMatrix();
			phd_TranslateAbs(bptr->x, bptr->y, bptr->z);
			phd_RotY(bptr->angle << 4);

			auto mptr = phd_mxptr;

			for (int j = 0; j < 5; ++j)
			{
				TempMesh[_X] = BatMesh[j][_X];

				if (j >= 3)
					TempMesh[_Y] = BatMesh[j][_Y] - 512 + m_sin(bptr->WingYoff << 7, 4);
				else TempMesh[_Y] = BatMesh[j][_Y] - 512 + m_sin(((bptr->WingYoff + 32) & 63) << 7, 8);

				TempMesh[_Z] = BatMesh[j][_Z];

				result[_X] = (mptr[M00] * TempMesh[_X] + mptr[M01] * TempMesh[_Y] + mptr[M02] * TempMesh[_Z] + mptr[M03]);
				result[_Y] = (mptr[M10] * TempMesh[_X] + mptr[M11] * TempMesh[_Y] + mptr[M12] * TempMesh[_Z] + mptr[M13]);
				result[_Z] = (mptr[M20] * TempMesh[_X] + mptr[M21] * TempMesh[_Y] + mptr[M22] * TempMesh[_Z] + mptr[M23]);

				float zv = f_persp / (float)result[_Z];

				scrxy[0] = short(float(result[_X] * zv + f_centerx));
				scrxy[1] = short(float(result[_Y] * zv + f_centery));
				scrz[0] = result[_Z];

				scrxy += 2;
				scrz += 2;
			}

			phd_PopMatrix();

			scrxy = (short*)&myscrxy;
			scrz = (long*)&myscrz;

			auto pptr = &BatLinks[0];

			for (int j = 0; j < 3; ++j)
			{
				long x1 = scrxy[(*pptr)],
					 y1 = scrxy[(*pptr) + 1],
					 z1 = scrz[(*pptr++)],
					 x2 = scrxy[(*pptr)],
					 y2 = scrxy[(*pptr) + 1],
					 z2 = scrz[(*pptr++)],
					 x3 = scrxy[(*pptr)],
					 y3 = scrxy[(*pptr) + 1],
					 z3 = scrz[(*pptr++)];

				if ((z1 >> W2V_SHIFT < 32 || z2 >> W2V_SHIFT < 32 || z3 >> W2V_SHIFT < 32) ||
					(x1 < 0 && x2 < 0 && x3 < 0) ||
					(x1 >= sw && x2 >= sw && x3 >= sw) ||
					(y1 < 0 && y2 < 0 && y3 < 0) ||
					(y1 >= sh && y2 >= sh && y3 >= sh))
				{
					continue;
				}

				PHD_VBUF v[3];
				PHDTEXTURESTRUCT Tex;

				int clipflag = 0;

				if (x1 < 0)		  ++clipflag;
				else if (x1 > sw) clipflag += 2;

				if (y1 < 0)		  clipflag += 4;
				else if (y1 > sh) clipflag += 8;

				v[0].xs = (float)x1;
				v[0].ys = (float)y1;
				v[0].ooz = (f_persp / (float)z1) * f_oneopersp;
				v[0].clip = clipflag;
				v[0].g = BATCOLOR;

				clipflag = 0;

				if (x2 < 0)		  ++clipflag;
				else if (x2 > sw) clipflag += 2;

				if (y2 < 0)		  clipflag += 4;
				else if (y2 > sh) clipflag += 8;

				v[1].xs = (float)x2;
				v[1].ys = (float)y2;
				v[1].ooz = (f_persp / (float)z2) * f_oneopersp;
				v[1].clip = clipflag;
				v[1].g = BATCOLOR;

				clipflag = 0;

				if (x3 < 0)		  ++clipflag;
				else if (x3 > sw) clipflag += 2;

				if (y3 < 0)		  clipflag += 4;
				else if (y3 > sh) clipflag += 8;

				v[2].xs = (float)x3;
				v[2].ys = (float)y3;
				v[2].ooz = (f_persp / (float)z3) * f_oneopersp;
				v[2].clip = clipflag;
				v[2].g = BATCOLOR;

				auto pSpriteInfo = phdsprinfo + objects[EXPLOSION1].mesh_ptr + 12;

				uint16_t u1 = (pSpriteInfo->offset << 8) & 0xff00,
					   v1 = pSpriteInfo->offset & 0xff00,
					   u2 = u1 + pSpriteInfo->width,
					   v2 = v1 + pSpriteInfo->height;

				u1 += App.nUVAdd;
				u2 -= App.nUVAdd;
				v1 += App.nUVAdd;
				v2 -= App.nUVAdd;

				Tex.drawtype = 1;
				Tex.tpage = pSpriteInfo->tpage;

				v[0].zv = (float)z1;
				v[1].zv = (float)z2;
				v[2].zv = (float)z3;

				if (!j)
				{
					v[0].u = u1;
					v[0].v = v1 + App.nUVAdd;
					v[1].u = u2 - App.nUVAdd;
					v[1].v = v2;
					v[2].u = u1;
					v[2].v = v2;
				}
				else if (j == 1)
				{
					v[0].u = u2;
					v[0].v = v1;
					v[1].u = u1;
					v[1].v = v1;
					v[2].u = u2;
					v[2].v = v2;
				}
				else
				{
					v[0].u = u1;
					v[0].v = v1;
					v[1].u = u2;
					v[1].v = v1;
					v[2].u = u2;
					v[2].v = v2;
				}

				InsertGT3(&v[0], &v[1], &v[2], &Tex, &v[0].u, &v[1].u, &v[2].u, MID_SORT, 1);
			}
		}

		++bptr;
	}
}