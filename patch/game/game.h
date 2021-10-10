#pragma once

#include "types.h"

enum lava_death_type
{
	LAVA_DEATH_FIRE,
	LAVA_DEATH_RAPIDS,
	LAVA_DEATH_ELECTRIC,
};

int32_t StartGame(int level_number);
int32_t GameLoop();
int32_t GetRandomDraw(void);
void SeedRandomDraw(int32_t seed);
int32_t GetRandomControl(void);
void SeedRandomControl(int32_t seed);

int32_t ControlPhase(int32_t nframes);

int ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target);
void AnimateItem(ITEM_INFO* item);
int GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
void TranslateItem(ITEM_INFO* item, int32_t x, int32_t y, int32_t z);

FLOOR_INFO* GetFloor(int32_t x, int32_t y, int32_t z, int16_t* room_number);

int32_t GetHeight(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z);
int32_t GetWaterHeight(int32_t x, int32_t y, int32_t z, int16_t room_number);
int32_t GetCeiling(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z);
int16_t GetDoor(FLOOR_INFO* floor);

void TestTriggers(int16_t* data, int heavy);
bool TriggerActive(ITEM_INFO* item);

void RemoveRoomFlipItems(ROOM_INFO* r);
void AddRoomFlipItems(ROOM_INFO* r);
void SoundEffects();
void DisplayModeInfo(char* szString);
void FinishLevel();

inline int lava_type = LAVA_DEATH_FIRE,
		   fish_shoal_type = 0,
		   biggun_anim_obj = VEHICLE_ANIM,
		   sub_anim_obj = VEHICLE_ANIM,
		   quadbike_anim_obj = VEHICLE_ANIM,
		   boat_anim_obj = VEHICLE_ANIM,
		   minecart_anim_obj = VEHICLE_ANIM,
		   kayak_anim_obj = VEHICLE_ANIM;

inline bool level_complete = 0,
			enable_lara_breath = false,
			enable_cold_exposure = false,
			artifact_pickup_finish = true,
			enable_propeller_insta_death = false,
			enable_island_spikes_sound = false,
			enable_deadly_swamp = false,
			enable_rapids_fire_type = false,
			enable_flamer_friendly = false,
			enable_punks_friendly = false,
			enable_fusebox = false,
			enable_smash1_destruction = true,
			enable_killable_ai_patrol = false,
			enable_footprints = false,
			enable_engine_extended_features = false;