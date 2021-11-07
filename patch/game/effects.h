#pragma once

#include "items.h"
#include "anim.h"

#define NUM_EFFECTS 64

struct FX_INFO
{
	PHD_3DPOS pos;
	int16_t room_number;
	int16_t object_number;
	int16_t next_fx;
	int16_t next_active;
	int16_t speed;
	int16_t fallspeed;
	int16_t frame_number;
	int16_t counter;
	int16_t shade;
	int16_t flag1;
	int16_t flag2;
	int16_t flag3;
	int16_t item;
};

struct BITE_INFO
{
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t mesh_num;
};

inline int number_sound_effects;
inline OBJECT_VECTOR* sound_effects;

inline FX_INFO* effects = nullptr;
inline int16_t next_fx_free;
inline int16_t next_fx_active;

void SoundEffects();

int16_t DoBloodSplat(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE direction, int16_t room_num);
int16_t DoBloodSplatEx(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE direction, int16_t room_num, int16_t num = -1);
void DoLotsOfBlood(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE direction, int16_t room_num, int number);
void DoLotsOfBloodD(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE direction, int16_t room_num, int number);
void CreateBubble(PHD_3DPOS* pos, int16_t room_number, long size, long sizerange);
void ControlBubble1(int16_t item_num);
void Richochet(GAME_VECTOR* pos);
void LaraBubbles(ITEM_INFO* item);
void Splash(ITEM_INFO* item);
void WadeSplash(ITEM_INFO* item, int waterheight, int waterdepth);
void floor_shake_effect(ITEM_INFO* item);
void BaddieBiteEffect(ITEM_INFO* item, BITE_INFO* bite);
void LaraBreath(ITEM_INFO* item);

void (*effect_routines[])(ITEM_INFO* item);