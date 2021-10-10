#include "standard.h"
#include "directx.h"
#include "global.h"
#include "hwrender.h"
#include "output.h"

#include <main.h>
#include <game/inventry.h>
#include <3dsystem/hwinsert.h>

#define HealthBarX 8
#define HealthBarY 6
#define HealthBarW 100

#define AirBarX (game_setup.dump_width-110)
#define AirBarY 6
#define AirBarW 100

#define DASHBAR_X (game_setup.dump_width-110)
#define DASHBAR_Y 6

void S_DrawDashBar(int percent)
{
	HWR_EnableZBuffer(true, true);

	g_blue_effect = false;

	int x = DASHBAR_X,
		y = DASHBAR_Y,
		w = (percent * HealthBarW) / 100,
		z = phd_znear + 50;

	auto colour = (char)inv_colours[C_BLACK];
	InsertLine(x - 2, y + 1, x + HealthBarW + 1, y + 1, z, colour);
	InsertLine(x - 2, y + 2, x + HealthBarW + 1, y + 2, z, colour);
	InsertLine(x - 2, y + 3, x + HealthBarW + 1, y + 3, z, colour);
	InsertLine(x - 2, y + 4, x + HealthBarW + 1, y + 4, z, colour);
	InsertLine(x - 2, y + 5, x + HealthBarW + 1, y + 5, z, colour);
	InsertLine(x - 2, y + 6, x + HealthBarW + 1, y + 6, z, colour);
	InsertLine(x - 2, y + 7, x + HealthBarW + 1, y + 7, z, colour);

	z = phd_znear + 40;
	colour = (char)inv_colours[C_GREY];
	InsertLine(x - 2, y + 8, x + HealthBarW + 2, y + 8, z, colour);
	InsertLine(x + HealthBarW + 2, y, x + HealthBarW + 2, y + 8, z, colour);

	z = phd_znear + 30;
	colour = (char)inv_colours[C_WHITE];
	InsertLine(x - 2, y, x + HealthBarW + 2, y, z, colour);
	InsertLine(x - 2, y + 8, x - 2, y, z, colour);

	if (w)
	{
		colour = (char)inv_colours[C_DARKGREEN];
		InsertLine(x, y + 2, x + w, y + 2, z, colour);
		InsertLine(x, y + 3, x + w, y + 3, z, colour);
		InsertLine(x, y + 4, x + w, y + 4, z, colour);
		InsertLine(x, y + 5, x + w, y + 5, z, colour);
		InsertLine(x, y + 6, x + w, y + 6, z, colour);
	}
}

void S_DrawHealthBar(int percent, bool poisoned)
{
	HWR_EnableZBuffer(true, true);

	g_blue_effect = false;

	int x = HealthBarX,
		y = HealthBarY,
		w = (percent * HealthBarW) / 100,
		z = phd_znear + 50;

	auto colour = (char)inv_colours[C_BLACK];
	InsertLine(x - 2, y + 1, x + HealthBarW + 1, y + 1, z, colour);
	InsertLine(x - 2, y + 2, x + HealthBarW + 1, y + 2, z, colour);
	InsertLine(x - 2, y + 3, x + HealthBarW + 1, y + 3, z, colour);
	InsertLine(x - 2, y + 4, x + HealthBarW + 1, y + 4, z, colour);
	InsertLine(x - 2, y + 5, x + HealthBarW + 1, y + 5, z, colour);
	InsertLine(x - 2, y + 6, x + HealthBarW + 1, y + 6, z, colour);
	InsertLine(x - 2, y + 7, x + HealthBarW + 1, y + 7, z, colour);

	z = phd_znear + 40;
	colour = (char)inv_colours[C_GREY];
	InsertLine(x - 2, y + 8, x + HealthBarW + 2, y + 8, z, colour);
	InsertLine(x + HealthBarW + 2, y, x + HealthBarW + 2, y + 8, z, colour);

	z = phd_znear + 30;
	colour = (char)inv_colours[C_WHITE];
	InsertLine(x - 2, y, x + HealthBarW + 2, y, z, colour);
	InsertLine(x - 2, y + 8, x - 2, y, z, colour);

	if (w)
	{
		z = phd_znear + 20;
		colour = (char)inv_colours[poisoned ? C_YELLOW : C_RED];

		InsertLine(x, y + 2, x + w, y + 2, z, colour);
		InsertLine(x, y + 3, x + w, y + 3, z, colour);
		InsertLine(x, y + 4, x + w, y + 4, z, colour);
		InsertLine(x, y + 5, x + w, y + 5, z, colour);
		InsertLine(x, y + 6, x + w, y + 6, z, colour);
	}
}

void S_DrawHealthBar3D(int32_t wx, int32_t wy, int32_t wz, int percent, bool poisoned)
{
	HWR_EnableZBuffer(true, true);

	g_blue_effect = false;

	long result[XYZ];
	long scr[XYZ];

	mCalcPoint(wx, wy, wz, &result[0]);
	ProjectPCoord(result[_X], result[_Y], result[_Z], scr, f_centerx, f_centery, phd_persp);

	int x = scr[_X] - HealthBarW / 2,
		y = scr[_Y],
		w = (percent * HealthBarW) / 100,
		z = phd_znear + 50;

	auto colour = (char)inv_colours[C_BLACK];

	InsertLine(x - 2, y + 1, x + HealthBarW + 1, y + 1, z, colour);
	InsertLine(x - 2, y + 2, x + HealthBarW + 1, y + 2, z, colour);
	InsertLine(x - 2, y + 3, x + HealthBarW + 1, y + 3, z, colour);
	InsertLine(x - 2, y + 4, x + HealthBarW + 1, y + 4, z, colour);
	InsertLine(x - 2, y + 5, x + HealthBarW + 1, y + 5, z, colour);
	InsertLine(x - 2, y + 6, x + HealthBarW + 1, y + 6, z, colour);
	InsertLine(x - 2, y + 7, x + HealthBarW + 1, y + 7, z, colour);

	if (w)
	{
		z = phd_znear + 40;

		colour = (char)inv_colours[poisoned ? C_YELLOW : C_RED];
		InsertLine(x, y + 2, x + w, y + 2, z, colour);
		InsertLine(x, y + 3, x + w, y + 3, z, colour);
		InsertLine(x, y + 4, x + w, y + 4, z, colour);
		InsertLine(x, y + 5, x + w, y + 5, z, colour);
		InsertLine(x, y + 6, x + w, y + 6, z, colour);
	}
}

void S_DrawAirBar(int percent)
{
	HWR_EnableZBuffer(true, true);

	g_blue_effect = false;

	int x = AirBarX,
		y = AirBarY,
		w = (percent * AirBarW) / 100,
		z = phd_znear + 50;

	auto colour = (char)inv_colours[C_BLACK];
	InsertLine(x - 2, y + 1, x + AirBarW + 1, y + 1, z, colour);
	InsertLine(x - 2, y + 2, x + AirBarW + 1, y + 2, z, colour);
	InsertLine(x - 2, y + 3, x + AirBarW + 1, y + 3, z, colour);
	InsertLine(x - 2, y + 4, x + AirBarW + 1, y + 4, z, colour);
	InsertLine(x - 2, y + 5, x + AirBarW + 1, y + 5, z, colour);
	InsertLine(x - 2, y + 6, x + AirBarW + 1, y + 6, z, colour);
	InsertLine(x - 2, y + 7, x + AirBarW + 1, y + 7, z, colour);

	z = phd_znear + 40;
	colour = (char)inv_colours[C_GREY];
	InsertLine(x - 2, y + 8, x + AirBarW + 2, y + 8, z, colour);
	InsertLine(x + AirBarW + 2, y, x + AirBarW + 2, y + 8, z, colour);

	z = phd_znear + 30;
	colour = (char)inv_colours[C_WHITE];
	InsertLine(x - 2, y, x + AirBarW + 2, y, z, colour);
	InsertLine(x - 2, y + 8, x - 2, y, z, colour);

	if (percent > 0)
	{
		z = phd_znear + 20;
		colour = (char)inv_colours[C_WHITE];
		InsertLine(x, y + 3, x + w, y + 3, z, colour);

		colour = (char)inv_colours[C_BLUE];
		InsertLine(x, y + 2, x + w, y + 2, z, colour);
		InsertLine(x, y + 4, x + w, y + 4, z, colour);
		InsertLine(x, y + 5, x + w, y + 5, z, colour);
		InsertLine(x, y + 6, x + w, y + 6, z, colour);
	}
}