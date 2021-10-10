#pragma once

inline int slots_used = 0;
inline CREATURE_INFO* baddie_slots = nullptr;

void InitialiseLOTarray(void);
void DisableBaddieAI(int16_t item_num);
int EnableBaddieAI(int16_t item_number, int Always);
void InitialiseSlot(int16_t item_number, int slot);
int InitialiseLOT(LOT_INFO* LOT);
void ClearLOT(LOT_INFO* LOT);
int	EnableNonLotAI(int16_t item_number, int Always);
void InitialiseNonLotAI(int16_t item_number, int slot);