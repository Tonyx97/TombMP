#pragma once

#include "items.h"
#include "collide.h"
#include "objects.h"

enum command_types
{
	COMMAND_NULL,
	COMMAND_MOVE_ORIGIN,
	COMMAND_JUMP_VELOCITY,
	COMMAND_ATTACK_READY,
	COMMAND_DEACTIVATE,
	COMMAND_SOUND_FX,
	COMMAND_EFFECT
};

struct GAME_VECTOR
{
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t room_number;
	int16_t box_number;
};

struct OBJECT_VECTOR
{
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t data;
	int16_t flags;
};

struct ANIM_STRUCT
{
	int16_t* frame_ptr;
	int16_t interpolation;
	int16_t current_anim_state;
	int32_t velocity;
	int32_t acceleration;
	int16_t frame_base;
	int16_t frame_end;
	int16_t jump_anim_num;
	int16_t jump_frame_num;
	int16_t number_changes;
	int16_t* change_ptr;
	int16_t number_commands;
	int16_t* command_ptr;
};

struct CHANGE_STRUCT
{
	int16_t goal_anim_state;
	int16_t number_ranges;
	int16_t* range_ptr;
};

struct RANGE_STRUCT
{
	int16_t start_frame;
	int16_t end_frame;
	int16_t link_anim_num;
	int16_t link_frame_num;
};

struct BOUNDING_BOX
{
	int16_t MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
};

struct ANIM_FRAME
{
	BOUNDING_BOX box;
	int16_t off_x, off_y, off_z;
	//uint16_t* angle_sets;
};

struct OBJECT_INFO
{
	int16_t nmeshes;
	int16_t mesh_index;
	int32_t bone_index;
	int16_t* frame_base;
	void (*initialise)(int16_t item_number);
	void (*control)(int16_t item_number);
	void (*floor)(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
	void (*ceiling)(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
	void (*draw_routine)(ITEM_INFO* item);
	void (*collision)(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
	int16_t anim_index;
	int16_t hit_points;
	int16_t pivot_length;		// replacement for 'head_size': distance from centre to neck rotation
	int16_t radius;
	int16_t shadow_size;			// size of shadow ( -1 if none )
	uint16_t bite_offset;			// offset into table of BITE_INFO structures for enemies that fire weapons. (Table in DRAW.C, set up bite_offset in SETUP.C).
	uint16_t loaded;			// is this object loaded on this level
	uint16_t intelligent;		// does this object need AI info??
	uint16_t non_lot;			// does this creature not use LOT system (e.g. Compys)
	uint16_t semi_transparent;	// is sprite object semi transparent
	uint16_t water_creature;	// is this is water based baddie? needed for SFX in shallow water
};

struct STATIC_INFO
{
	int16_t mesh_number;
	int16_t flags;											// flags for collision and stuff.
	int16_t x_minp, x_maxp, y_minp, y_maxp, z_minp, z_maxp;	// draw bounds
	int16_t x_minc, x_maxc, y_minc, y_maxc, z_minc, z_maxc;  // collision Bounds
};

inline OBJECT_INFO* objects = nullptr;
inline STATIC_INFO* static_objects = nullptr;

inline int32_t* bones;
inline ANIM_STRUCT* anims;
inline RANGE_STRUCT* ranges;
inline CHANGE_STRUCT* changes;
inline int16_t** meshes,
			  * meshes_base,
			  * commands,
			  * frames;

inline int32_t number_static_objects = 0;

inline int32_t number_meshes = 0,
			   number_bones = 0;

inline int32_t number_anims = 0,
			   number_custom_anims = 0,
			   max_number_custom_anims = 0,
			   number_anim_changes = 0,
			   number_anim_ranges = 0,
			   number_anim_commands = 0,
			   number_anim_frames = 0;

void init_custom_animations_pools(int normal_anims_count);

bool is_valid_anim(int16_t id);
bool unload_animation(int16_t id);
bool is_custom_anim_loaded(int16_t id);

int16_t load_animation(const std::string& filename);