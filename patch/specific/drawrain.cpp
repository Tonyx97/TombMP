#include "standard.h"
#include "global.h"
#include "hwrender.h"
#include "drawprimitive.h"
#include "output.h"

#include <game/control.h>

#include <specific/fn_stubs.h>

#include <3dsystem/hwinsert.h>

#include <main.h>

int RainYAdd = 1024,
	RainAngDiv = 1023,
	RainAngAdd = 512,
	scale;

void DoRain()
{
	auto [sw, sh] = g_window->get_resolution();

	const auto dir = CamRot.y;

	long distance = 3072;

	if (CamRot.x < 2048)
	{
		scale = CamRot.x;

		RainYAdd = ((2048 * scale) >> 10) + 1024;
		RainAngDiv = ((3072 * scale) >> 10) + 1024;
		RainAngAdd = ((1536 * scale) >> 10) + 512;

		distance -= CamRot.x << 2;
	}
	else
	{
		scale = -(3072 - CamRot.x);

		RainYAdd = ((2048 * scale) >> 10) - 1024;
		RainAngDiv = 2048 - ((1024 * scale) >> 10);
		RainAngAdd = 1024 - ((512 * scale) >> 10);

		distance -= (4096 - CamRot.x) << 2;
	}

	D3DTLVERTEX	Lines[128 * 2];

	int LineCnt = 0;

	for (int i = 0; i < 128; ++i)
	{
		PHD_ANGLE radius = (rand() & 4095) + distance - 2048,
				  angle = (dir + (rand() % RainAngDiv) - RainAngAdd) & 4095;

		long x = -((m_sin(angle << 1) * radius) >> 12),
			 y = -(rand() & 2047) + RainYAdd,
			 z = ((m_cos(angle << 1) * radius) >> 12);

		if (IsRoomOutside(CamPos.x+x, CamPos.y+y, CamPos.z+z))
		{
			long result[XYZ],
				 screencoords[XYZ];

			mCalcPoint(x + CamPos.x, y + CamPos.y, z + CamPos.z, &result[0]);
			ProjectPCoord(result[_X], result[_Y], result[_Z], screencoords, sw >> 1, sh >> 1, 320);

			result[_Z] >>= 3;

			y += -((rand() & 31) + 32);

			long result2[XYZ],
				 screencoords2[XYZ];

			mCalcPoint(x + CamPos.x, y + CamPos.y, z + CamPos.z, &result2[0]);
			ProjectPCoord(result2[_X], result2[_Y], result2[_Z], screencoords2, sw >> 1, sh >> 1, 320);

			if (result[_Z] > 0x5000 || result[_Z] < 32 || (result2[_Z] >> 3) < 32 ||
				screencoords[_X] < 0 || screencoords2[_X] < 0 ||
				screencoords[_X] > sw || screencoords2[_X] > sw ||
				screencoords[_Y] < 0 || screencoords2[_Y] < 0 ||
				screencoords[_Y] > sh || screencoords2[_Y] > sh)
			{
				continue;
			}

			int c = (rand() & 24);

			Lines[LineCnt].sx = (float)screencoords[_X];
			Lines[LineCnt].sy = (float)screencoords[_Y];
			Lines[LineCnt].sz = f_a - f_boo * (one / (float)(result2[_Z] << W2V_SHIFT));
			Lines[LineCnt].color = RGBA_MAKE(0, 0, 0, 0);
			Lines[LineCnt].specular = 0;
			Lines[LineCnt].rhw = 0;

			Lines[LineCnt + 1].sx = (float)screencoords2[_X];
			Lines[LineCnt + 1].sy = (float)screencoords2[_Y];
			Lines[LineCnt + 1].sz = f_a - f_boo * (one / (float)(result2[_Z] << W2V_SHIFT));
			Lines[LineCnt + 1].color = RGBA_MAKE(32 + c, 48 + c, 48 + c, 0xff);
			Lines[LineCnt + 1].specular = 0;
			Lines[LineCnt + 1].rhw = 0;

			LineCnt += 2;
		}
	}

	if (LineCnt > 0)
	{
		HWR_SetCurrentTexture(0);

		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		DrawPrimitive(D3DPT_LINELIST, D3DVT_TLVERTEX, &Lines, LineCnt, D3DDP_DONOTCLIP);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
	}
}