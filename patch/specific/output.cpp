#include "standard.h"
#include "input.h"
#include "hwrender.h"
#include "global.h"
#include "init.h"
#include "output.h"
#include "drawprimitive.h"
#include "process.h"
#include "picture.h"
#include "texture.h"
#include "time.h"

#include <3dsystem/hwinsert.h>
#include <3dsystem/3d_gen.h>
#include <3dsystem/3dglodef.h>

#include <game/types.h>
#include <game/effect2.h>
#include <game/items.h>

#include <specific/litesrc.h>

#include <main.h>

#include <keycode/keycode.h>

// in frames (60 per second)
#define SUNSET_TIME			(20 * 60 * 60)
#define SUNSET_BLUETIME		(SUNSET_TIME / 2)
#define SUNSET_ORANGETIME	(SUNSET_TIME * 3 / 4)
#define DEPTHQ_END			(20 * 1024)
#define DEPTHQ_START		(DEPTHQ_END - (8 * 1024))
#define ANIMATED_TEXTURE	8
#define CLEAN_SCREEN

enum lighting_type
{
	LIGHT_NONE,
	LIGHT_FLICKER,
	LIGHT_PULSE,
	LIGHT_SUNSET,
	NUMBER_LIGHT_TYPES
};

int16_t shadowman[] =
{
	0, 0, 0,
	32767,
	1,
	8,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0
};

extern float ZTable[];

int objbcnt;

int light_level[NUMBER_LIGHT_TYPES];

int GnR = 0xc0,
	GnG = 0xc0,
	GnB = 0xc0;

// initialise anything that needs initialising before each frame can be drawn

void S_InitialisePolyList(int tClearScreen)
{
	if (fullscreen_clear_needed)
	{
		DX_CheckForLostSurfaces();		// ensure surfaces are around before clearing them...
		DX_DoFlipWait();
		DX_ClearBuffers(DXCB_FRONT | DXCB_BACK);

		tClearScreen = fullscreen_clear_needed = false;
	}

	DWORD dwFlags = DXCB_CLRWINDOW | DXCB_ZBUFFER;

	if (tClearScreen || g_window->get_fill_mode() < 3)
		dwFlags |= DXCB_BACK;

	DX_ClearBuffers(dwFlags);

	HWR_BeginScene();

	phd_InitPolyList();

	objbcnt = 0;
}

int32_t dump_screen()
{
	// dump the screen to video RAM and return the frame compensating count

	auto nframes = Sync();

	ScreenPartialDump();

	while (nframes < 2)
	{
		while (!Sync())
			SwitchToThread();
		
		++nframes;
	}

	return nframes;
}

void S_ClearScreen()
{
	ScreenClear();
}

void S_InitialiseScreen()
{
	hwr_init_state();
}

void output_polylist()
{
#ifdef GAMEDEBUG
	if (g_keycode->is_key_down(KEY_Y))
	{
		PHD_VBUF v1, v2, v3, v4;
		PHDTEXTURESTRUCT tex;
		static int nTPage = 39;
		static bool tU = false, tI = false;

		float fOOZ = one / f_znear;
		v1.xs = 64.f + 256.f;		v1.ys = 64.f;	v1.zv = f_znear;	v1.g = (int16_t)0xffff;	v1.ooz = fOOZ;	v1.clip = 0;
		v2.xs = 320.f + 256.f;		v2.ys = 64.f;	v2.zv = f_znear;	v2.g = (int16_t)0xffff;	v2.ooz = fOOZ;	v2.clip = 0;
		v3.xs = 320.f + 256.f;		v3.ys = 320.f;	v3.zv = f_znear;	v3.g = (int16_t)0xffff;	v3.ooz = fOOZ;	v3.clip = 0;
		v4.xs = 64.f + 256.f;		v4.ys = 320.f;	v4.zv = f_znear;	v4.g = (int16_t)0xfffff;	v4.ooz = fOOZ;	v4.clip = 0;
		tex.drawtype = 1;
		tex.tpage = nTPage;
		tex.u1 = 0;tex.v1 = 0;
		tex.u2 = 0xffff;tex.v2 = 0;
		tex.u3 = 0xffff;tex.v3 = 0xffff;
		tex.u4 = 0;tex.v4 = 0xffff;

		InsertGT4(&v1, &v2, &v3, &v4, &tex, MID_SORT, 0);

		if (g_keycode->is_key_down(KEY_U))
		{
			if (!tU)
			{
				if (nTPage)
					--nTPage;
				tU = true;
			}
		}
		else tU = false;

		if (g_keycode->is_key_down(KEY_I))
		{
			if (!tI)
			{
				if (nTPage < 255)
					++nTPage;
				tI = true;
			}
		}
		else tI = false;
	}
#endif

	if (App.lpZBuffer)
	{
		HWR_EnableColorKey(false);
		HWR_EnableZBuffer(true, true);

		DrawBuckets();

		phd_SortPolyList(surfacenumbf, sort3d_bufferbf);
		HWR_EnableZBuffer(false, true);
		HWR_DrawPolyListBF(surfacenumbf, (int32_t*)sort3d_bufferbf);
	}
	else
	{
		phd_SortPolyList(surfacenumbf, sort3d_bufferbf);
		HWR_DrawPolyList(surfacenumbf, (int32_t*)sort3d_bufferbf);
	}

	HWR_EndScene();
}

/**
* Calculate on screen coordinates of bounding box for object
* @return 0 if object box is totally off the screen, 1 if the object box is totally on screen
* and -1 if object is z-clipped or x/y clipped
*/
int S_GetObjectBounds(int16_t* bptr)
{
	if (*(phd_mxptr + M23) >= phd_zfar && outside == 0)
		return 0;

	++objbcnt;

	int32_t minx = (int32_t)*(bptr + 0),
		   maxx = (int32_t)*(bptr + 1),
		   miny = (int32_t)*(bptr + 2),
		   maxy = (int32_t)*(bptr + 3),
		   minz = (int32_t)*(bptr + 4),
		   maxz = (int32_t)*(bptr + 5);

	int32_t vertex[8][3];

	vertex[0][0] = minx;		// V.0
	vertex[0][1] = miny;
	vertex[0][2] = minz;

	vertex[1][0] = maxx;		// V.1
	vertex[1][1] = miny;
	vertex[1][2] = minz;

	vertex[2][0] = maxx;		// V.2
	vertex[2][1] = maxy;
	vertex[2][2] = minz;

	vertex[3][0] = minx;		// V.3
	vertex[3][1] = maxy;
	vertex[3][2] = minz;

	vertex[4][0] = minx;		// V.4
	vertex[4][1] = miny;
	vertex[4][2] = maxz;

	vertex[5][0] = maxx;		// V.5
	vertex[5][1] = miny;
	vertex[5][2] = maxz;

	vertex[6][0] = maxx;		// V.6
	vertex[6][1] = maxy;
	vertex[6][2] = maxz;

	vertex[7][0] = minx;		// V.7
	vertex[7][1] = maxy;
	vertex[7][2] = maxz;

	auto mptr = phd_mxptr;
	auto vertptr = &vertex[0][0];

	int32_t numz = 0;
	
	minx = miny = 0x3fffffff;
	maxx = maxy = -0x3fffffff;

	for (int i = 0; i < 8; ++i, vertptr += 3)
	{
		int32_t zv = *(mptr + M20) * *(vertptr + 0) + *(mptr + M21) * *(vertptr + 1) + *(mptr + M22) * *(vertptr + 2) + *(mptr + M23);

		if (zv > phd_znear && zv < phd_zfar)
		{
			++numz;

			zv /= phd_persp;

			int32_t coord = (*(mptr + M00) * *(vertptr + 0) + 	// calculate X coordinate
							*(mptr + M01) * *(vertptr + 1) +
							*(mptr + M02) * *(vertptr + 2) +
							*(mptr + M03)) / zv;

			if (coord < minx) minx = coord;
			if (coord > maxx) maxx = coord;

			coord = (*(mptr + M10) * *(vertptr + 0) + 	// cCalculate Y coordinate
					 *(mptr + M11) * *(vertptr + 1) +
					 *(mptr + M12) * *(vertptr + 2) +
					 *(mptr + M13)) / zv;

			if (coord < miny)
				miny = coord;

			if (coord > maxy)
				maxy = coord;
		}
	}

	minx += phd_centerx;	// add on Screen Offsets
	maxx += phd_centerx;	// to MinMax
	miny += phd_centery;
	maxy += phd_centery;

	if (numz < 8 && outside > 0)
		return -1;

	if (numz < 8 ||
		minx < 0 ||
		miny < 0 ||				// bounding Box is partially
		maxx > phd_winxmax ||	// on Whole Window
		maxy > phd_winymax)		// or/and Z-Clipped...
		return -1;

	if (!numz ||				// if No Coords are Visible
		minx > phd_right ||
		miny > phd_bottom ||	// thro current Doorway then
		maxx < phd_left ||
		maxy < phd_top)			// exit with ( 0 )
		return 0;

	return 1;	// return 1 If Box Wholly On Screen...
}

void S_InsertBackPolygon(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y, int colour)
{
	// rather than clear the whole screen, back polygons are calculated to cover
	// the parts of rooms not drawn because they are so far in the distance

	min_x += phd_winxmin;
	max_x += phd_winxmin;
	min_y += phd_winymin;
	max_y += phd_winymin;

	InsertFlatRect(min_x, min_y, max_x, max_y, phd_zfar + 1, inv_colours[C_BLACK]);
}

void S_PrintShadow(int16_t size, int16_t* bptr, ITEM_INFO* iptr, int unknown)
{
	int midx = (*(bptr + 0) + *(bptr + 1)) / 2,
		midz = (*(bptr + 4) + *(bptr + 5)) / 2,
		xadd = ((*(bptr + 1) - *(bptr + 0)) * size) / (UNIT_SHADOW * 4),
		zadd = ((*(bptr + 5) - *(bptr + 4)) * size) / (UNIT_SHADOW * 4),
		xadd2 = xadd * 2,
		zadd2 = zadd * 2;

	shadowman[2 + 4] = midx - xadd;	// V.0
	shadowman[4 + 4] = midz + zadd2;

	shadowman[5 + 4] = midx + xadd;   	// V.1
	shadowman[7 + 4] = midz + zadd2;

	shadowman[8 + 4] = midx + xadd2;  	// V.2
	shadowman[10 + 4] = midz + zadd;

	shadowman[11 + 4] = midx + xadd2;  	// V.3
	shadowman[13 + 4] = midz - zadd;

	shadowman[14 + 4] = midx + xadd;    // V.4
	shadowman[16 + 4] = midz - zadd * 2;

	shadowman[17 + 4] = midx - xadd;    // V.5
	shadowman[19 + 4] = midz - zadd * 2;

	shadowman[20 + 4] = midx - xadd2;  	// V.6
	shadowman[22 + 4] = midz - zadd;

	shadowman[23 + 4] = midx - xadd2;  	// V.7
	shadowman[25 + 4] = midz + zadd;

	phd_leftfloat = (float)(phd_winxmin + phd_left);
	phd_topfloat = (float)(phd_winymin + phd_top);
	phd_rightfloat = (float)(phd_winxmin + phd_right + 1);
	phd_bottomfloat = (float)(phd_winymin + phd_bottom + 1);
	f_centerx = (float)(phd_winxmin + phd_centerx);
	f_centery = (float)(phd_winymin + phd_centery);

	phd_PushMatrix();
	phd_TranslateAbs(iptr->pos.x_pos, iptr->floor, iptr->pos.z_pos);
	phd_RotY(iptr->pos.y_rot);

	if (calc_object_vertices(&shadowman[4]))
		InsertTrans8(vbuf, 32);	// 32 is special transparent depthq table

	phd_PopMatrix();
}

void S_LightRoom(ROOM_INFO* r)
{
	// check on room lighting

	if (r->lighting != LIGHT_NONE)
	{
		int nLightLevel = light_level[r->lighting];

		auto objptr = r->data;

		for (int i = (int)*(objptr++); i > 0; --i, objptr += 6)
			((uint8_t*)objptr)[2] += ((nLightLevel * (((uint8_t*)objptr)[3] & 0x1f)) >> 6);
	}
}

void AnimateTextures(int nframes)
{
	static int frame_comp = 0;

	frame_comp += nframes;

	while (frame_comp > ANIMATED_TEXTURE)
	{
		auto ptr = anim_tex_ranges;

		for (int i = (int)*(ptr++); i > 0; --i, ptr++)
		{
			int j = (int)*(ptr++);
			const auto& temp = phdtextinfo[*(ptr)];

			for (; j > 0; --j, ++ptr)
				phdtextinfo[*(ptr)] = phdtextinfo[*(ptr + 1)];

			phdtextinfo[*(ptr)] = temp;
		}

		frame_comp -= ANIMATED_TEXTURE;
	}
}

void S_SetupBelowWater(int underwater)
{
	// if camera underwater then blue palette
	// wibble underwater if camera NOT underwater

	water_effect = !underwater;
	g_blue_effect = true;
}

void S_SetupAboveWater(int underwater)
{
	// Wibble above water if camera underwater

	water_effect = underwater;
	g_blue_effect = bool(underwater);
}

void animate_textures(int nframes)
{
	// underwater frame compensated waves

	wibble_offset = (wibble_offset + (nframes)) & (WIBBLE_SIZE - 1);

	// room lighting effects

	light_level[LIGHT_FLICKER] = GetRandomDraw() & (WIBBLE_SIZE - 1);
	light_level[LIGHT_PULSE] = (phd_sin(wibble_offset * 65536 / WIBBLE_SIZE) + (1 << W2V_SHIFT)) * (WIBBLE_SIZE - 1) >> (W2V_SHIFT + 1);

	if (GF_SunsetEnabled)
	{
		sunset += nframes;

		light_level[LIGHT_SUNSET] = (sunset < SUNSET_TIME ? sunset * (WIBBLE_SIZE - 1) / SUNSET_TIME : WIBBLE_SIZE - 1);
	}

	// animate Room Textures

	AnimateTextures(nframes);
}

void ScreenDump()
{
	DX_UpdateFrame(0);
}

void ScreenPartialDump()
{
	DX_UpdateFrame(&grc_dump_window);
}

void ScreenClear(bool tWindow)
{
	DX_ClearBuffers(DXCB_BACK);
}

void mCalcPoint(long dx, long dy, long dz, long* result)
{
	auto mptr = w2v_matrix;

	dx -= *(mptr + M03);
	dy -= *(mptr + M13);
	dz -= *(mptr + M23);

	*(result + _X) = ((*(mptr + M00) * dx + *(mptr + M01) * dy + *(mptr + M02) * dz) >> W2V_SHIFT);
	*(result + _Y) = ((*(mptr + M10) * dx + *(mptr + M11) * dy + *(mptr + M12) * dz) >> W2V_SHIFT);
	*(result + _Z) = ((*(mptr + M20) * dx + *(mptr + M21) * dy + *(mptr + M22) * dz) >> W2V_SHIFT);
}

void ProjectPCoord(long x, long y, long z, long* result, long cx, long cy, long fov)
{
	if (z > 0)
	{
		*(result + _X) = ((x * fov) / z) + cx;
		*(result + _Y) = ((y * fov) / z) + cy;
		*(result + _Z) = z;
	}
	else if (z < 0)
	{
		*(result + _X) = ((x * fov) / -z) + cx;
		*(result + _Y) = ((y * fov) / -z) + cy;
		*(result + _Z) = z;
	}
	else
	{
		*(result + _X) = ((x * fov)) + cx;
		*(result + _Y) = ((y * fov)) + cy;
		*(result + _Z) = z;
	}
}

bool world_to_screen(int x, int y, int z, int& ox, int& oy)
{
	long res[XYZ];

	auto [sw, sh] = g_window->get_resolution();

	mCalcPoint(x, y, z, res);

	if (res[_Z] <= 0)
		return false;

	ProjectPCoord(res[_X], res[_Y], res[_Z], res, sw >> 1, sh >> 1, phd_persp);

	ox = res[_X];
	oy = res[_Y];

	return true;
}