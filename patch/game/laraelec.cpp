#include "laraelec.h"
#include "control.h"

#define	MAX_ELEC_POS	256
#define	MAX_ELEC_POS2	MAX_ELEC_POS + (MAX_ELEC_POS >> 1)

void UpdateElectricityPoints()
{
	long tx, ty, tz, xv, yv, zv;
	short* leccy;

	leccy = &electricity_points[0][0];

	for (int i = 0; i < 32; ++i)
	{
		int random = GetRandomDraw();

		tx = *leccy;
		ty = *(leccy + 1);
		tz = *(leccy + 2);
		xv = *(leccy + 3);
		yv = *(leccy + 4);
		zv = *(leccy + 5);

		if (((tx > MAX_ELEC_POS || tx < -MAX_ELEC_POS) &&
			(ty > MAX_ELEC_POS || ty < -MAX_ELEC_POS) &&
			(tz > MAX_ELEC_POS || tz < -MAX_ELEC_POS)) ||
			((tx > MAX_ELEC_POS2 || tx < -MAX_ELEC_POS2) ||
			(ty > MAX_ELEC_POS2 || ty < -MAX_ELEC_POS2) ||
			(tz > MAX_ELEC_POS2 || tz < -MAX_ELEC_POS2)))
			xv = tx = yv = ty = zv = tz = 0;

		if (!xv)
			xv = ((random & 1) ? -(GetRandomDraw() & 3) - 1 : (GetRandomDraw() & 3) + 1);
		else if (xv < 0)
			xv -= 2;
		else xv += 2;

		if (!yv)
			yv = ((random & 2) ? -(GetRandomDraw() & 3) - 1 : (GetRandomDraw() & 3) + 1);
		else if (yv < 0)
			yv -= 2;
		else yv += 2;

		if (!zv)
			zv = ((random & 4) ? -(GetRandomDraw() & 3) - 1 : (GetRandomDraw() & 3) + 1);
		else if (zv < 0)
			--zv;
		else ++zv;

		tx += xv;
		ty += yv;
		tz += zv;

		*leccy++ = tx;
		*leccy++ = ty;
		*leccy++ = tz;
		*leccy++ = xv;
		*leccy++ = yv;
		*leccy++ = zv;
	}
}