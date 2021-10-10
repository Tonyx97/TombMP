#pragma once

#include "text.h"

inline int overlay_flag = 1;
inline TEXTSTRING* ammo_text = nullptr;

int FlashIt();
void draw_game_info(int timed);
void DrawHealthBar(int flash_state);
void DrawAirBar(int flash_state);
void MakeAmmoString(char* string);
void DrawAmmoInfo();
void InitialisePickUpDisplay();
void DrawPickups(int timed);
void AddDisplayPickup(int16_t objnum);
void DisplayModeInfo(char* szString);
void DrawModeInfo();