#include "standard.h"
#include "global.h"
#include <main.h>
#include "hwrender.h"
#include "drawprimitive.h"

#include <game/laraelec.h>
#include <game/effects.h>
#include <game/lara.h>
#include <game/sphere.h>

bool ClipLine(long& x1, long& y1, long& x2, long& y2, int sw, int sh);

void LaraElectricDeath(long copy, ITEM_INFO* item)
{
	long screencoords[3 * 200];

	short distances[200];

	PHD_VECTOR pos1 { 0, 0, 0 },
			   pos2 { 0, 0, 0 };

	auto [sw, sh] = g_window->get_resolution();

	--sw; --sh;

	int dcnt = 0;

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	for (int i = 0; i < 14; ++i)
	{
		auto leccy = &electricity_points[(i * 5) & 15][0];

		int mesh_num_b1 = lara_meshes[(i << 1)],
			mesh_num_b2 = lara_meshes[(i << 1) + 1];

		pos1.y = 0;

		if (copy)
		{
			pos1.z = -48;
			pos1.x = -48;
		}
		else
		{
			pos1.x = 48;
			pos1.z = 48;
		}

		GetJointAbsPosition(item, &pos1, mesh_num_b1);

		pos2.y = 0;

		if (!lara_last_points[i] || i == 13)
		{
			if (copy)
			{
				pos2.z = -48;
				pos2.x = -48;
			}
			else
			{
				pos2.z = 48;
				pos2.x = 48;
			}

			if (i == 13)
				pos2.y = -64;
		}
		else pos2.x = pos2.z = 0;

		GetJointAbsPosition(item, &pos2, mesh_num_b2);

		pos1.x -= item->pos.x_pos;
		pos1.y -= item->pos.y_pos;
		pos1.z -= item->pos.z_pos;
		pos2.x -= item->pos.x_pos;
		pos2.y -= item->pos.y_pos;
		pos2.z -= item->pos.z_pos;

		int wx = pos1.x,
			wy = pos1.y,
			wz = pos1.z,
			dx = (pos2.x - pos1.x) >> 2,
			dy = (pos2.y - pos1.y) >> 2,
			dz = (pos2.z - pos1.z) >> 2;

		for (int j = 0; j < 5; ++j)
		{
			short TempMesh[3];
			
			if (j == 4 && !lara_last_points[i])
				break;
			else if (j == 4 && lara_last_points[i])
			{
				TempMesh[0] = (short)pos2.x;
				TempMesh[1] = (short)pos2.y;
				TempMesh[2] = (short)pos2.z;
			}
			else
			{
				TempMesh[0] = (short)wx;
				TempMesh[1] = (short)wy;
				TempMesh[2] = (short)wz;
			}

			if (j != 0 && j != 4)
			{
				int tx = *leccy++,
					ty = *leccy++,
					tz = *leccy++;

				if (copy)
				{
					TempMesh[0] -= (short)(tx >> 3);
					TempMesh[1] -= (short)(ty >> 3);
					TempMesh[2] -= (short)(tz >> 3);
				}
				else
				{
					TempMesh[0] += (short)(tx >> 3);
					TempMesh[1] += (short)(ty >> 3);
					TempMesh[2] += (short)(tz >> 3);
				}

				leccy += 3;

				int xv = abs(tx),
					yv = abs(ty),
					zv = abs(tz);

				if (yv > xv) xv = yv;
				if (zv > xv) xv = zv;

				distances[dcnt] = (short)xv;
			}
			else distances[dcnt] = 0;

			auto mptr = phd_mxptr;
			long x1, y1, z1;

			x1 = (mptr[M00] * TempMesh[0] + mptr[M01] * TempMesh[1] + mptr[M02] * TempMesh[2] + mptr[M03]);
			y1 = (mptr[M10] * TempMesh[0] + mptr[M11] * TempMesh[1] + mptr[M12] * TempMesh[2] + mptr[M13]);
			z1 = (mptr[M20] * TempMesh[0] + mptr[M21] * TempMesh[1] + mptr[M22] * TempMesh[2] + mptr[M23]);

			float zv = f_persp / (float)z1;

			screencoords[(dcnt * 3) + 0] = (short)((float)(x1 * zv + f_centerx));
			screencoords[(dcnt * 3) + 1] = (short)((float)(y1 * zv + f_centery));
			screencoords[(dcnt * 3) + 2] = z1 >> W2V_SHIFT;

			++dcnt;

			wx += dx;
			wy += dy;
			wz += dz;
		}
	}

	dcnt = 0;

	for (int i = 0; i < 6; ++i)
	{
		for (int j = 0; j < lara_line_counts[i]; ++j)
		{
			long x1 = screencoords[dcnt * 3],
				 y1 = screencoords[(dcnt * 3) + 1],
				 x2 = screencoords[(dcnt * 3) + 3],
				 y2 = screencoords[(dcnt * 3) + 4],
				 z1 = screencoords[(dcnt * 3) + 2],
				 rgb = distances[dcnt],
				 rgb1 = distances[dcnt + 1];

			++dcnt;

			if (z1 < 32)
				continue;

			if (rgb > 255)
			{
				rgb = 511 - rgb;
				if (rgb < 0)
					rgb = 0;
			}

			if (rgb1 > 255)
			{
				rgb1 = 511 - rgb1;
				if (rgb1 < 0)
					rgb1 = 0;
			}

			if (copy)
			{
				rgb >>= 1;
				rgb1 >>= 1;
			}

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
					v[1].sz = f_a - f_boo * (one / (float)(z1 << W2V_SHIFT));

					v[0].color = RGB_MAKE(0, rgb, rgb);
					v[1].color = RGB_MAKE(0, rgb1, rgb1);

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

		++dcnt;
	}

	phd_PopMatrix();

	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
}