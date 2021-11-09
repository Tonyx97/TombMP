#pragma once

#include <unordered_map>

#include <specific/stypes.h>

#include <shared/game/math.h>

#define NUMBER_ITEMS	1024
#define NO_ITEM			-1
#define NOT_ACTIVE		0
#define ACTIVE			1
#define DEACTIVATED		2
#define INVISIBLE		3

#define ROT_X 4
#define ROT_Y 8
#define ROT_Z 16

#define GUARD		1			// AI bits
#define AMBUSH		2
#define PATROL1		4
#define MODIFY		8
#define FOLLOW		16
#define EXPLODED	32

struct ITEM_LIGHT
{
	PHD_VECTOR sun;
	PHD_VECTOR bulb;
	PHD_VECTOR dynamic;
	unsigned char sunr, sung, sunb;
	char init;
	unsigned char bulbr, bulbg, bulbb;
	unsigned char ambient;
	unsigned char dynamicr, dynamicg, dynamicb, pad2;
};

class ITEM_INFO
{
public:

	PHD_3DPOS pos,
			  local_pos;

	ITEM_INFO* parent;

	void* data;

	int32_t floor;
	uint32_t touch_bits;
	uint32_t mesh_bits;
	int16_t object_number;
	int16_t current_anim_state;
	int16_t goal_anim_state;
	int16_t required_anim_state;	// May need to pass through several goals
	int16_t anim_number;
	int16_t frame_number;
	int16_t room_number;
	int16_t next_item;       		// For linked list within rooms
	int16_t next_active;
	int16_t speed;
	int16_t fallspeed;
	int16_t hit_points;
	uint16_t box_number;
	int16_t timer;					// For timer switches
	int16_t flags;					// For oneshot and code switches (i.e. NOT flags)
	int16_t shade, shadeB;			// -1 for dynamic lighting
	int16_t carried_item;			// 6/8/97: bad guys hold whatever pickup
	int16_t after_death;
	uint16_t fired_weapon;			// Has item fired a weapon (counter) ?
	int16_t item_flags[4];			// Had to be added 'cos it was getting fucking stupid.
	ITEM_LIGHT il;
	uint16_t active;				// Is Item Active or Not??
	uint16_t status;				// Bit packing fun
	uint16_t gravity_status;
	uint16_t hit_status;			// Has item been hit by a bullet
	uint16_t collidable;			// Can Lara Collide with It??
	uint16_t looked_at;				// has item ever been a camera target
	uint16_t dynamic_light;			// Used in cutscenes to attach light to first pivot point
	uint16_t clear_body;			// do we want corpse to be cleared away?
	uint16_t ai_bits;				// store AI instruction pickups
	uint16_t really_active;			// Special for Claxons and Sentry guns to say whether or not to do their stuff.
	uint16_t id;					// Special for Claxons and Sentry guns to say whether or not to do their stuff.

	void set_rotation_x(PHD_ANGLE v);
	void set_rotation_y(PHD_ANGLE v);
	void set_rotation_z(PHD_ANGLE v);

	PHD_ANGLE get_rotation_x(bool skip_local = false);
	PHD_ANGLE get_rotation_y(bool skip_local = false);
	PHD_ANGLE get_rotation_z(bool skip_local = false);
};

inline std::unordered_map<ITEM_INFO*, std::vector<ITEM_INFO*>> g_attachments;

inline ITEM_INFO* items = nullptr;
inline int16_t body_bag;
inline int16_t next_item_free;
inline int16_t next_item_active;
inline int32_t level_items = 0;

void InitialiseItemArray(int numitems);
void KillItem(int16_t item_number, bool sync = true);
int16_t CreateItem();
void InitialiseItem(int16_t item_num);
void BasicSetupItem(int16_t item_num);
void RemoveActiveItem(int16_t item_num);
void RemoveDrawnItem(int16_t item_num);
void AddActiveItem(int16_t item_num);
void ItemNewRoom(int16_t item_number, int16_t room_number);
int GlobalItemReplace(int in_objnum, int out_objnum);

bool IsValidItem(ITEM_INFO* item);

void InitialiseFXArray();
void KillEffect(int16_t fx_num);
int16_t CreateEffect(int16_t room_num);
void EffectNewRoom(int16_t fx_num, int16_t room_number);

void ClearBodyBag();

void attach_entities(ITEM_INFO* attached, ITEM_INFO* target, const int_vec3& offset = {}, const short_vec3& rotation = {});
void dispatch_entities_attachments();