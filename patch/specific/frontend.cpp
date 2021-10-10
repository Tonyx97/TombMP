#include "standard.h"
#include "global.h"
#include "frontend.h"
#include "input.h"
#include "output.h"

#include <3dsystem/hwinsert.h>

#include <game/text.h>
#include <game/inventry.h>

#define B_YELLOW 15
#define B_ORANGE 31

#define RED_WEIGHT 1.0
#define GREEN_WEIGHT 1.0
#define BLUE_WEIGHT 1.0

uint8_t SWR_FindNearestPaletteEntry(uint8_t* pPal, int nR, int nG, int nB, bool tDontUseWindowsColours)
{
	int i, nEnd, nColour;

	if (tDontUseWindowsColours)
	{
		i = 10;
		nEnd = 246;
		pPal += 10 * 3;
	}
	else
	{
		i = 0;
		nEnd = 256;
	}

	auto dClosest = 10000000000.0;

	for (; i < nEnd; ++i)
	{
		auto dR = double(nR - *(pPal++)),
			 dG = double(nG - *(pPal++)),
			 dB = double(nB - *(pPal++));

		if (auto dDist = dR * dR * RED_WEIGHT + dG * dG * GREEN_WEIGHT + dB * dB * BLUE_WEIGHT; dDist < dClosest)
		{
			nColour = i;
			dClosest = dDist;
		}
	}

	return (uint8_t)nColour;
}

SG_COL S_COLOUR(int R, int G, int B)
{
	// find closest colour in palette
	return (SG_COL)SWR_FindNearestPaletteEntry(game_palette, R, G, B, false);	// windows colours are OK.
}

void S_DrawScreenLine(int32_t sx, int32_t sy, int32_t z, int32_t w, int32_t h, int32_t col, uint16_t* grdptr, uint16_t flags)
{
	InsertLine(sx, sy, sx + w, sy + h, phd_znear + (z << 3), (char)col);
}

void S_DrawLine(int32_t x1, int32_t y1, int32_t z, int32_t x2, int32_t y2, int32_t col, uint16_t* grdptr, uint16_t flags)
{
	InsertLine(x1, y1, x2, y2, phd_znear + (z << 3), (char)col);
}

void S_DrawScreenBox(int32_t sx, int32_t sy, int32_t z, int32_t	w, int32_t h, int32_t col, uint16_t* grdptr, uint16_t flags)
{
	S_DrawScreenLine(sx, sy - 1, z, w + 1, 0, B_YELLOW, NULL, flags);
	S_DrawScreenLine(sx + 1, sy, z, w - 1, 0, B_ORANGE, NULL, flags);

	S_DrawScreenLine(sx + w, sy + 1, z, 0, h - 1, B_YELLOW, NULL, flags);
	S_DrawScreenLine(sx + w + 1, sy, z, 0, h + 1, B_ORANGE, NULL, flags);

	S_DrawScreenLine(sx - 1, sy - 1, z, 0, h + 1, B_YELLOW, NULL, flags);
	S_DrawScreenLine(sx, sy, z, 0, h - 1, B_ORANGE, NULL, flags);

	S_DrawScreenLine(sx, sy + h, z, w - 1, 0, B_YELLOW, NULL, flags);
	S_DrawScreenLine(sx - 1, sy + h + 1, z, w + 1, 0, B_ORANGE, NULL, flags);
}

void S_DrawScreenFBox(int32_t sx, int32_t sy, int32_t z, int32_t	w, int32_t h, int32_t col, uint16_t* grdptr, uint16_t flags)
{
	// draw flat shaded box (should be gouraud but will look crap)
	InsertTransQuad(phd_winxmin + sx, phd_winymin + sy, w + 1, h + 1, phd_znear + (z << 3));
}

void S_FadeToBlack()
{
	uint8_t* p = game_palette;

	for (int i = 0; i < 256 * 3; ++i, ++p)
		*p = 0;

	ScreenClear();
	ScreenDump();
	ScreenClear();
	ScreenDump();
}