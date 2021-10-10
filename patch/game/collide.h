#pragma once

#include "items.h"
#include "room.h"

#define DRAWN			0x20		// For clearing bodies.
#define SWONESHOT		0x40		// For switches.
#define ATONESHOT		0x80		// For anti-triggers.
#define KILLED_ITEM		(-0x8000)
#define NO_BAD_POS		(-NO_HEIGHT)
#define NO_BAD_NEG 		NO_HEIGHT
#define COLL_NONE		0
#define COLL_FRONT		1	// Solid Wall in Front
#define COLL_LEFT		2	// Solid Wall to LHS
#define COLL_RIGHT		4	// Solid Wall to RHS
#define COLL_TOP		8	// MidPoint Ceiling
#define COLL_TOPFRONT	16	// MidPoint at Front
#define COLL_CLAMP		32	// MidPoint Ceiling and Floor BAD!!!

enum headings
{
	NORTH,
	EAST,
	SOUTH,
	WEST
};

enum height_types
{
	WALL,
	SMALL_SLOPE,
	BIG_SLOPE,
	DIAGONAL,
	SPLIT_TRI
};

struct COLL_INFO
{
	int32_t mid_floor;				// relative floor and ceiling heights at midpoint
	int32_t mid_ceiling;
	int32_t mid_type;
	int32_t front_floor;				// relative floor and ceiling heights at front
	int32_t front_ceiling;
	int32_t front_type;
	int32_t left_floor;				// relative floor and ceiling heights at front/left side
	int32_t left_ceiling;
	int32_t left_type;
	int32_t right_floor;				// relative floor and ceiling heights at front/right side
	int32_t right_ceiling;
	int32_t right_type;
	int32_t left_floor2;				// relative floor and ceiling heights at front/left side
	int32_t left_ceiling2;
	int32_t left_type2;
	int32_t right_floor2;			// relative floor and ceiling heights at front/right side
	int32_t right_ceiling2;
	int32_t right_type2;
	int32_t radius;					// INPUT Collision Radius...
	int32_t bad_pos, bad_neg;			// INPUT Relative Heights We want to Collide against...
	int32_t bad_ceiling;				// INPUT Relative Bad Ceiling Height...
	PHD_VECTOR shift;				// shift/push values..
	PHD_VECTOR old;					// INPUT Old Positions
	int16_t old_anim_state;			// INPUT old animation
	int16_t old_anim_number;
	int16_t old_frame_number;
	int16_t facing;					// INPUT Angle we are Moving...
	int16_t quadrant;				// Quadrant we are moving ( Not necessarily facing! )
	int16_t coll_type;				// Type of Collision
	int16_t* trigger;				// Trigger info index
	signed char tilt_x, tilt_z;		// Type of Tilt of Floor
	char hit_by_baddie;				// Flag to indicate Lara has been hit by a Baddie
	char hit_static;				// Flag to indicate Lara has collided with a Static object
	uint16_t slopes_are_walls : 2;		// Treat big slopesUp as walls
	uint16_t slopes_are_pits : 1;		// Treat big slopesDown as pits
	uint16_t lava_is_pit : 1;			// Treat Lava as Bad place to go onto
	uint16_t enable_baddie_push : 1;	// Baddies Can push Lara
	uint16_t enable_spaz : 1;			// Spaz animations are enabled
	uint16_t hit_ceiling : 1;			// Has Lara hit ceiling? (For up jump only).
};

inline int height_type,
		   tiltxoff,
		   tiltyoff;

void GetCollisionInfo(COLL_INFO* info, int32_t xpos, int32_t ypos, int32_t zpos, int16_t room_number, int32_t objheight);
void GetNearByRooms(int32_t x, int32_t y, int32_t z, int32_t r, int32_t h, int16_t room_number);
void GetNewRoom(int32_t x, int32_t y, int32_t z, int16_t room_number);
int16_t GetTiltType(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z);

int32_t FindGridShift(int32_t src, int32_t dst);
int32_t FindGridShift2(int32_t src, int32_t dst);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
int Move3DPosTo3DPos(PHD_3DPOS* srcpos, PHD_3DPOS* destpos, int velocity, PHD_ANGLE angadd);

int CollideStaticObjects(COLL_INFO* coll, int32_t x, int32_t y, int32_t z, int16_t room_number, int32_t hite);
void CreatureCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void ObjectCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void TrapCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void DoorCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void AIPickupCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);

void UpdateLaraRoomTeleport(ITEM_INFO* item, int height);
void UpdateLaraRoom(ITEM_INFO* item, int height);
void LaraBaddieCollision(ITEM_INFO* litem, COLL_INFO* coll);
void ItemPushLara(ITEM_INFO* item, ITEM_INFO* laraitem, COLL_INFO* coll, int spazon, int bigpush);
int TestLaraPosition(int16_t* bounds, ITEM_INFO* item, ITEM_INFO* laraitem, PHD_ANGLE override_y_rot = -1);
void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* laraitem);
int MoveLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* laraitem);

int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* laraitem, int32_t radius);

void ObjectCollisionSub(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);