#pragma once

#include "items.h"

inline uint8_t lara_meshes[14 * 2] =
{
	0, 1, 1, 2, 2, 3,
	0, 4, 4, 5, 5, 6,
	0, 7,
	7, 8, 8, 9, 9, 10,
	7, 11, 11, 12, 12, 13,
	7, 14
};

inline uint8_t lara_last_points[14] =
{
	0, 0, 1,
	0, 0, 1,
	1,
	0, 0, 1,
	0, 0, 1,
	1
};

inline uint8_t lara_line_counts[6] = { 12, 12, 4, 12, 12, 4 };

inline short electricity_points[32][6];

void LaraElectricDeath(long copy, ITEM_INFO* item);
void UpdateElectricityPoints();