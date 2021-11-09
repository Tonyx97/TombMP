#pragma once

#include "collide.h"
#include "effects.h"

#define MAX_EXPANSION 5

#define NUM_SLOTS			256
#define NUM_NONLOT_SLOTS	256
#define NO_BOX 				0x7ff
#define BOX_NUMBER			0x7ff
#define BOX_END_BIT			0x8000

#define EXPAND_LEFT   0x1
#define EXPAND_RIGHT  0x2
#define EXPAND_TOP    0x4
#define EXPAND_BOTTOM 0x8

#define BLOCKABLE     0x8000
#define BLOCKED       0x4000
#define OVERLAP_INDEX 0x3fff

#define SEARCH_NUMBER  0x7fff
#define BLOCKED_SEARCH 0x8000

#define TIMID   0
#define VIOLENT 1

#define NO_FLYING	0
#define FLY_ZONE	0x2000

#define FRONT_ARC			0x4000
#define MAX_HEAD_CHANGE		(ONE_DEGREE*5)
#define MAX_JOINT_CHANGE	(ONE_DEGREE*5)
#define MAX_TILT			(ONE_DEGREE*3)
#define MAX_JOINTS			4

#define TURN_TO_FACE -1

// get the correct zone from the allowed step height for the creature
#define ZONE(A) (((A) >> (WALL_SHIFT-2)) - 1)

struct BOX_INFO
{
	uint8_t left, right, top, bottom;
	int16_t height;
	int16_t overlap_index;
};

struct BOX_NODE
{
	int16_t exit_box;
	uint16_t search_number;
	int16_t next_expansion;
	int16_t box_number;     // for storing zone box lists
};

enum TARGET_TYPE
{
	NO_TARGET,
	PRIME_TARGET,
	SECONDARY_TARGET
};

enum MOOD_TYPE
{
	BORED_MOOD,
	ATTACK_MOOD,
	ESCAPE_MOOD,
	STALK_MOOD
};

struct AI_INFO
{
	int16_t zone_number, enemy_zone;
	int32_t distance, ahead, bite;
	int16_t angle, x_angle, enemy_facing;
};

struct LOT_INFO
{
	BOX_NODE* node;
	int16_t head, tail;
	uint16_t search_number, block_mask;
	int16_t step, drop, fly, zone_count;
	int16_t target_box, required_box;
	PHD_VECTOR target;
};

struct CREATURE_INFO
{
	int16_t joint_rotation[MAX_JOINTS];	// up to 4 extra rotations in addition to normal animation
	int16_t maximum_turn;
	int16_t flags;						// spare bits that can be used for any purpose by different types of creature
	uint16_t alerted : 1;					// time to bit pack - Lots of miscellaneous AI & behaviour stuff;
	uint16_t head_left : 1;				// eventually these might replace 'flags'
	uint16_t head_right : 1;
	uint16_t reached_goal : 1;
	uint16_t hurt_by_lara : 1;
	uint16_t patrol2 : 1;
	MOOD_TYPE mood;
	int16_t item_num;
	PHD_VECTOR target;
	ITEM_INFO* enemy;
	LOT_INFO LOT;
};

inline int number_boxes;

inline BOX_INFO* boxes = nullptr;

inline uint16_t* overlap = nullptr;
inline int16_t* ground_zone[4][2],
			 * fly_zone[2];

void InitialiseCreature(int16_t item_number);
int	CreatureActive(int16_t item_number);
void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info);

void CreateZone(ITEM_INFO* item);
int UpdateLOT(LOT_INFO* LOT, int expansion);

void TargetBox(LOT_INFO* LOT, int16_t box_number);
void GetCreatureMood(ITEM_INFO* item, AI_INFO* info, int violent);
void CreatureMood(ITEM_INFO* item, AI_INFO* info, int violent);
TARGET_TYPE CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT);

int CreatureAnimation(int16_t item_number, int16_t angle, int16_t tilt);

int16_t CreatureTurn(ITEM_INFO* item, int16_t maximum_turn);
void CreatureTilt(ITEM_INFO* item, int16_t angle);
void CreatureJoint(ITEM_INFO* item, int16_t joint, int16_t required);
void CreatureFloat(int16_t item_number);
void CreatureUnderwater(ITEM_INFO* item, int32_t depth);
int16_t CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, int16_t(*generate)(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number));
int CreatureVault(int16_t item_number, int16_t angle, int vault, int shift);
void CreatureKill(ITEM_INFO* item, int kill_anim, int kill_state, int lara_kill_state);
void CreatureDie(int16_t item_number, bool explode, bool is_player = false, bool sync_explode = true);

void AlertAllGuards(int16_t item_number);
void AlertNearbyGuards(ITEM_INFO* info);
void GetAITarget(CREATURE_INFO* creature);
int16_t AIGuard(CREATURE_INFO* creature);
int16_t SameZone(CREATURE_INFO* creature, ITEM_INFO* target_item);

void EarthQuake(int16_t item_number);

void ControlBodyPart(int16_t fx_number);
void ControlMissile(int16_t fx_number);
void ShootAtLara(FX_INFO* fx);

int16_t GunShot(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number);
int16_t GunHit(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number);
int16_t GunMiss(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number);
void WinstonControl(int16_t item_number);

void AdjustStopperFlag(ITEM_INFO* item, long dir, long set);
void AdjustStopperFlag(int x, int y, int z, int16_t room_id, long dir, long set);