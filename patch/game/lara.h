#pragma once

#include "items.h"
#include "collide.h"
#include "effects.h"
#include "larafire.h"
#include "box.h"
#include "draw.h"

#define HAIR_SEGMENTS	6
#define SQUARE(A)		((A) * (A))

struct LARA_INFO
{
	int16_t item_number;
	int16_t gun_status;
	int16_t gun_type;
	int16_t request_gun_type;
	int16_t last_gun_type;
	int16_t calc_fallspeed;
	int16_t water_status;
	int16_t climb_status;
	int16_t pose_count;
	int16_t hit_frame;
	int16_t hit_direction;
	int16_t air;
	int16_t exposure;
	int16_t dash;
	int16_t dive_count;
	int16_t current_active;			// Underwater current
	int16_t current_xvel;
	int16_t current_yvel;			// Underwater current velocities.
	int16_t current_zvel;
	int16_t spaz_effect_count;		// Effects can make lara spaz
	int16_t flare_age;				// How old is the flare in Lara's hand?
	int16_t skidoo;					// flag whether Lara is skidooing
	int16_t weapon_item;				// dummy item used for new weapon control
	int16_t back_gun;				// object number of gun that Lara has on her back currently
	int16_t flare_frame;
	int16_t poisoned;				// Number of hits to take off Laras energy when poisoned (max val is ?).
	int16_t electric;				// Electrocuted ?
	uint16_t flare_control_left;
	uint16_t flare_control_right;
	uint16_t extra_anim : 1;			// 1 if Lara is running an anim from another project (LARA_EXTRA usually)
	uint16_t look : 1;				// 1 if OK for Lara to LookLeftRight
	uint16_t burn : 1;				// lara's botty is hot
	uint16_t keep_ducked : 1;			// can't get out of duck
	uint16_t can_monkey_swing : 1;
	uint16_t mine_l : 1;
	uint16_t mine_r : 1;
	uint16_t burn_red;			// Fire red
	uint16_t burn_green;			// Fire green
	uint16_t burn_blue;			// Fire blue
	uint16_t is_ducked : 1;			// In a duck animation
	uint16_t has_fired : 1;			// Has she fired a gun recently?
	int32_t water_surface_dist;		// <0 if Lara's origin is below water)
	PHD_VECTOR last_pos;
	FX_INFO* spaz_effect;
	int mesh_effects;				// Used for Special Effects
	int16_t* mesh_ptrs[15];
	ITEM_INFO* target;
	int target_mesh = -1;
	PHD_ANGLE target_angles[2];
	PHD_ANGLE turn_rate;
	PHD_ANGLE move_angle;
	PHD_ANGLE head_y_rot, head_x_rot, head_z_rot;
	PHD_ANGLE torso_y_rot, torso_x_rot, torso_z_rot;
	LARA_ARM left_arm, right_arm;
	AMMO_INFO pistols;
	AMMO_INFO magnums;
	AMMO_INFO uzis;
	AMMO_INFO shotgun;
	AMMO_INFO harpoon;
	AMMO_INFO rocket;
	AMMO_INFO grenade;
	AMMO_INFO m16;
	CREATURE_INFO* creature;		// lara uses baddie AI for water current
	int32_t corner_x;
	int32_t corner_z;

	int16_t skin = -1;

	bool spawned = false,
		 respawned = false,
		 underwater = false,
		 dead = false,
		 frozen = false,
		 hair_enabled = true,
		 angry_face = true,
		 holster_enabled = true;
};

inline LARA_INFO lara {};

inline ITEM_INFO* lara_item = nullptr;

inline int health_bar_timer = 0;

inline PHD_3DPOS g_hair[HAIR_SEGMENTS + 1];
inline PHD_VECTOR g_hair_vel[HAIR_SEGMENTS + 1];

inline char GotJointPos[15];

void ResetLaraState();
void LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll);

void LookUpDown();
void LookLeftRight();
void ResetLook();

void get_lara_bone_pos(ITEM_INFO* item, PHD_VECTOR* vec, int bone_id);
void get_lara_bone_pos_int(ITEM_INFO* item, PHD_VECTOR* vec, int16_t* frame1, int16_t* frame2, int frac, int rate, int bone_id);

void ControlDeathSlide(int16_t item_number);
void DeathSlideCollision(int16_t item_number, ITEM_INFO* laraitem, COLL_INFO* coll);