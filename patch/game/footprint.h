#pragma once

#include "items.h"

#define MAX_FOOTPRINTS	32

struct FOOTPRINT
{
	int32_t x, y, z;
	int16_t YRot, Active;
};

void InitFootPrints();
void AddFootprint(ITEM_INFO *item);
void S_DrawFootPrints();

inline int FootPrintNum;
inline FOOTPRINT FootPrint[MAX_FOOTPRINTS];