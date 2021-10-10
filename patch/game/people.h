#pragma once

bool Targetable(ITEM_INFO* item, AI_INFO* info);
int16_t GunShot(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number);
int16_t GunHit(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number);
int16_t GunMiss(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number);
bool ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, int16_t extra_rotation, int damage);
void WinstonControl(int16_t item_number);
bool TargetVisible(ITEM_INFO* item, AI_INFO* info);