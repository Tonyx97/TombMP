#pragma once

struct BOSS_STRUCT
{
	int16_t attack_count;
	int16_t death_count;
	uint8_t attack_flag;
	uint8_t attack_type;
	uint8_t attack_head_count;
	uint8_t ring_count;
	int16_t explode_count;
	int16_t LizmanItem, LizmanRoom;
	int16_t hp_counter;
	int16_t dropped_icon;
	uint8_t charged;
	uint8_t dead;
	PHD_VECTOR BeamTarget;
};

inline BOSS_STRUCT bossdata {};

bool InitialiseLevel(int level_number);
void InitialiseLevelFlags();
void InitialiseObjects();
void GetCarriedItems();
void GetAIPickups();

void BuildOutsideTable();