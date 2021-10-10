#include "standard.h"
#include "global.h"
#include "hwrender.h"
#include "drawprimitive.h"
#include "output.h"

#include <main.h>

void S_DrawDarts(ITEM_INFO* item)
{
	long screencoords[XYZ],
		 screencoords2[XYZ];

	auto [sw, sh] = g_window->get_resolution();

	int32_t x = item->pos.x_pos,
		   y = item->pos.y_pos,
		   z = item->pos.z_pos;

	long result[XYZ];

	mCalcPoint(x, y, z, &result[0]);
	ProjectPCoord(result[_X], result[_Y], result[_Z], screencoords, sw >> 1, sh >> 1, phd_persp);

	result[_Z] >>= 3;

	switch (item->pos.y_rot)
	{
	case 0:		 z -= 64; break;
	case 16384:  x -= 64; break;
	case -32768: z += 64; break;
	case -16384: x += 64; break;
	}

	long result2[XYZ];

	mCalcPoint(x, y, z, &result2[0]);
	ProjectPCoord(result2[_X], result2[_Y], result2[_Z], screencoords2, sw >> 1, sh >> 1, phd_persp);

	if (result[_Z] > 2048 || result[_Z] < 32 || (result2[_Z] >> 3) < 32 ||
		screencoords[_X] < 0 || screencoords2[_X] < 0 ||
		screencoords[_X] > sw || screencoords2[_X] > sw ||
		screencoords[_Y] < 0 || screencoords2[_Y] < 0 ||
		screencoords[_Y] > sh || screencoords2[_Y] > sh)
	{
		return;
	}

	D3DTLVERTEX v[2];

	v[0].sx = (float)screencoords[_X];
	v[0].sy = (float)screencoords[_Y];
	v[0].sz = f_a - f_boo * (one / (float)(result2[_Z] << W2V_SHIFT));

	v[1].sx = (float)screencoords2[_X];
	v[1].sy = (float)screencoords2[_Y];
	v[1].sz = f_a - f_boo * (one / (float)(result2[_Z] << W2V_SHIFT));

	v[0].color = RGB_MAKE(0, 0, 0);
	v[1].color = RGB_MAKE(255, 255, 255);

	v[0].specular = 0;
	v[1].specular = 0;

	v[0].rhw = v[0].sz;
	v[1].rhw = v[1].sz;

	HWR_SetCurrentTexture(0);
	HWR_EnableZBuffer(true, true);
	DrawPrimitive(D3DPT_LINESTRIP, D3DVT_TLVERTEX, &v, 2, D3DDP_DONOTCLIP);
	HWR_EnableZBuffer(false, false);
}
