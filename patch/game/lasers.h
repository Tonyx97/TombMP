#pragma once

void LaserControl(int16_t item_number);
void S_DrawLaser(ITEM_INFO* item);
void S_DrawLaserBeam(GAME_VECTOR* src, GAME_VECTOR* dest, uint8_t r, uint8_t g, uint8_t b);
void UpdateLaserShades();

inline uint8_t LaserShades[32];