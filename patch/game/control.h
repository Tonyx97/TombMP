#pragma once

#include "items.h"
#include "anim.h"
#include "room.h"
#include "collide.h"

#define TRIG_BITS(T)	((T & 0x3fff) >> 10)

inline int compy_scared_timer;
inline int compys_attack_lara;
inline int16_t carcass_item;

inline uint8_t* OutsideRoomTable;
inline short OutsideRoomOffsets[27 * 27];
inline short IsRoomOutsideNo;

inline int16_t FXType;

inline int OnObject;
inline ITEM_INFO* CeilingObject;

inline int16_t* trigger_index;
inline int chunky_flag;

inline int flip_status;
inline int flipmap[MAX_FLIPMAPS];
inline int flipeffect = -1;
inline int fliptimer = 0;

inline uint8_t HeavyTriggered;

int32_t ControlPhase(int32_t nframes);

int32_t IsRoomOutside(int32_t x, int32_t y, int32_t z);

int ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target);
void AnimateItem(ITEM_INFO* item);
int GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
void TranslateItem(ITEM_INFO* item, int32_t x, int32_t y, int32_t z);

FLOOR_INFO* FindFloor(int32_t x, int32_t y, int32_t z, int16_t* room_number);
FLOOR_INFO* GetFloor(int32_t x, int32_t y, int32_t z, int16_t* room_number);
int16_t GetDoor(FLOOR_INFO* floor);
long CheckNoColFloorTriangle(FLOOR_INFO* floor, long x, long z);
long CheckNoColCeilingTriangle(FLOOR_INFO* floor, long x, long z);
int32_t GetHeight(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z);
int32_t GetWaterHeight(int32_t x, int32_t y, int32_t z, int16_t room_number);
int32_t GetCeiling(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z);

void TestTriggers(int16_t* data, int heavy);
bool TriggerActive(ITEM_INFO* item);

int LOS(GAME_VECTOR* start, GAME_VECTOR* target);
int xLOS(GAME_VECTOR* start, GAME_VECTOR* target);
int zLOS(GAME_VECTOR* start, GAME_VECTOR* target);
int ObjectOnLOS(GAME_VECTOR* start, GAME_VECTOR* target);

void FlipMap(bool sync = true);
void RemoveRoomFlipItems(ROOM_INFO* r);
void AddRoomFlipItems(ROOM_INFO* r);

int32_t GetRandomControl();
int32_t GetRandomDraw();
void SeedRandomControl(int32_t seed);
void SeedRandomDraw(int32_t seed);