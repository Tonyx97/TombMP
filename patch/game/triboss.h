#pragma once

void FindClosestShieldPoint(long x, long y, long z, ITEM_INFO* item);
void TribeBossControl(int16_t item_number);
void S_DrawTribeBoss(ITEM_INFO* item);
void InitialiseTribeBoss(int16_t);

inline char lizard_man_active = 0;
inline char TribeBossShieldOn;