#pragma once

#include "invdata.h"

inline int16_t LevelSecrets[] = { 0, 6, 4, 5, 0, 3, 3, 3, 1, 5, 5, 6, 1, 3, 2, 3, 3, 3, 3, 0, 0 };

void draw_game_info();
void InitColours();
int Inv_AddItem(int itemNum);
int Inv_RequestItem(int itemNum);
void Inv_RemoveAllItems();
int Inv_RemoveItem(int itemNum);
int Inv_GetCount();
int Inv_GetItemOption(int itemNum);

void RingIsOpen(RING_INFO *ring);
void RingIsNotOpen(RING_INFO *ring);
void RingNotActive(INVENTORY_ITEM *inv_item);
void RingActive();