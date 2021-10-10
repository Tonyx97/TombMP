#pragma once

#include "items.h"
#include "anim.h"

#define DONT_TARGET	(-16384)	// set item->hit_points to disable targetting

#define NUM_SG_SHELLS		2	// 2 shells in a box
#define GUN_AMMO_CLIP		16	// How many bullets in Each Clip
#define MAGNUM_AMMO_CLIP	5	// Pickup is Two clips at a time
#define UZI_AMMO_CLIP		20
#define SHOTGUN_AMMO_CLIP	12	// all these blasted in one go
#define HARPOON_AMMO_CLIP	3
#define M16_AMMO_CLIP		30
#define ROCKET_AMMO_CLIP	1
#define GRENADE_AMMO_CLIP	1

#define FLARE_AMMO_BOX		8

#define GUN_AMMO_QTY		(GUN_AMMO_CLIP * 2)						// How many bullets are in each pickup!
#define MAGNUM_AMMO_QTY		(MAGNUM_AMMO_CLIP * 2)					// Pickup is Two clips at a time
#define UZI_AMMO_QTY		(UZI_AMMO_CLIP * 2)
#define SHOTGUN_AMMO_QTY	(SHOTGUN_AMMO_CLIP * NUM_SG_SHELLS)   // all these blasted in one go
#define FLARE_AMMO_QTY		FLARE_AMMO_BOX
#define HARPOON_AMMO_QTY	HARPOON_AMMO_CLIP
#define M16_AMMO_QTY		(M16_AMMO_CLIP * 2)
#define ROCKET_AMMO_QTY		ROCKET_AMMO_CLIP
#define GRENADE_AMMO_QTY	(GRENADE_AMMO_CLIP*2)

#define PISTOL_DAMAGE		1

#define PISTOL_LOCK_YMIN	-60*ONE_DEGREE
#define PISTOL_LOCK_YMAX	+60*ONE_DEGREE
#define PISTOL_LOCK_XMIN	-60*ONE_DEGREE
#define PISTOL_LOCK_XMAX	+60*ONE_DEGREE

#define PISTOL_LARM_YMIN	-170*ONE_DEGREE
#define PISTOL_LARM_YMAX	+60*ONE_DEGREE
#define PISTOL_LARM_XMIN	-80*ONE_DEGREE
#define PISTOL_LARM_XMAX	+80*ONE_DEGREE

#define PISTOL_RARM_YMIN	-60*ONE_DEGREE
#define PISTOL_RARM_YMAX	+170*ONE_DEGREE
#define PISTOL_RARM_XMIN	-80*ONE_DEGREE
#define PISTOL_RARM_XMAX	+80*ONE_DEGREE

#define SHOTGUN_LOCK_YMIN	-60*ONE_DEGREE
#define SHOTGUN_LOCK_YMAX	+60*ONE_DEGREE
#define SHOTGUN_LOCK_XMIN	-55*ONE_DEGREE
#define SHOTGUN_LOCK_XMAX	+55*ONE_DEGREE

#define SHOTGUN_LARM_YMIN	-80*ONE_DEGREE
#define SHOTGUN_LARM_YMAX	+80*ONE_DEGREE
#define SHOTGUN_LARM_XMIN	-65*ONE_DEGREE
#define SHOTGUN_LARM_XMAX	+65*ONE_DEGREE

#define SHOTGUN_RARM_YMIN	-80*ONE_DEGREE
#define SHOTGUN_RARM_YMAX	+80*ONE_DEGREE
#define SHOTGUN_RARM_XMIN	-65*ONE_DEGREE
#define SHOTGUN_RARM_XMAX	+65*ONE_DEGREE

#define M16_LOCK_YMIN -60*ONE_DEGREE
#define M16_LOCK_YMAX +60*ONE_DEGREE
#define M16_LOCK_XMIN -55*ONE_DEGREE
#define M16_LOCK_XMAX +55*ONE_DEGREE

#define M16_LARM_YMIN -80*ONE_DEGREE
#define M16_LARM_YMAX +80*ONE_DEGREE
#define M16_LARM_XMIN -65*ONE_DEGREE
#define M16_LARM_XMAX +65*ONE_DEGREE

#define M16_RARM_YMIN -80*ONE_DEGREE
#define M16_RARM_YMAX +80*ONE_DEGREE
#define M16_RARM_XMIN -65*ONE_DEGREE
#define M16_RARM_XMAX +65*ONE_DEGREE

#define ROCKET_LOCK_YMIN	-60*ONE_DEGREE
#define ROCKET_LOCK_YMAX	+60*ONE_DEGREE
#define ROCKET_LOCK_XMIN	-55*ONE_DEGREE
#define ROCKET_LOCK_XMAX	+55*ONE_DEGREE

#define ROCKET_LARM_YMIN	-80*ONE_DEGREE
#define ROCKET_LARM_YMAX	+80*ONE_DEGREE
#define ROCKET_LARM_XMIN	-65*ONE_DEGREE
#define ROCKET_LARM_XMAX	+65*ONE_DEGREE

#define ROCKET_RARM_YMIN	-80*ONE_DEGREE
#define ROCKET_RARM_YMAX	+80*ONE_DEGREE
#define ROCKET_RARM_XMIN	-65*ONE_DEGREE
#define ROCKET_RARM_XMAX	+65*ONE_DEGREE

#define GRENADE_LOCK_YMIN	-60*ONE_DEGREE
#define GRENADE_LOCK_YMAX	+60*ONE_DEGREE
#define GRENADE_LOCK_XMIN	-55*ONE_DEGREE
#define GRENADE_LOCK_XMAX	+55*ONE_DEGREE

#define GRENADE_LARM_YMIN	-80*ONE_DEGREE
#define GRENADE_LARM_YMAX	+80*ONE_DEGREE
#define GRENADE_LARM_XMIN	-65*ONE_DEGREE
#define GRENADE_LARM_XMAX	+65*ONE_DEGREE

#define GRENADE_RARM_YMIN	-80*ONE_DEGREE
#define GRENADE_RARM_YMAX	+80*ONE_DEGREE
#define GRENADE_RARM_XMIN	-65*ONE_DEGREE
#define GRENADE_RARM_XMAX	+65*ONE_DEGREE

#define DESERT_LOCK_YMIN	-60*ONE_DEGREE
#define DESERT_LOCK_YMAX	+60*ONE_DEGREE
#define DESERT_LOCK_XMIN	-60*ONE_DEGREE
#define DESERT_LOCK_XMAX	+60*ONE_DEGREE

#define DESERT_LARM_YMIN	-10*ONE_DEGREE
#define DESERT_LARM_YMAX	+10*ONE_DEGREE
#define DESERT_LARM_XMIN	-80*ONE_DEGREE
#define DESERT_LARM_XMAX	+80*ONE_DEGREE

#define DESERT_RARM_YMIN	-0*ONE_DEGREE
#define DESERT_RARM_YMAX	+0*ONE_DEGREE
#define DESERT_RARM_XMIN	-0*ONE_DEGREE
#define DESERT_RARM_XMAX	+0*ONE_DEGREE

#define HARPOON_LOCK_YMIN -60*ONE_DEGREE
#define HARPOON_LOCK_YMAX +60*ONE_DEGREE
#define HARPOON_LOCK_XMIN -65*ONE_DEGREE
#define HARPOON_LOCK_XMAX +65*ONE_DEGREE

#define HARPOON_LARM_YMIN -20*ONE_DEGREE
#define HARPOON_LARM_YMAX +20*ONE_DEGREE
#define HARPOON_LARM_XMIN -75*ONE_DEGREE
#define HARPOON_LARM_XMAX +75*ONE_DEGREE

#define HARPOON_RARM_YMIN -80*ONE_DEGREE
#define HARPOON_RARM_YMAX +80*ONE_DEGREE
#define HARPOON_RARM_XMIN -75*ONE_DEGREE
#define HARPOON_RARM_XMAX +75*ONE_DEGREE

#define SKID_LOCK_YMIN -30*ONE_DEGREE
#define SKID_LOCK_YMAX +30*ONE_DEGREE
#define SKID_LOCK_XMIN -55*ONE_DEGREE
#define SKID_LOCK_XMAX +55*ONE_DEGREE

#define SKID_LARM_YMIN -30*ONE_DEGREE
#define SKID_LARM_YMAX +30*ONE_DEGREE
#define SKID_LARM_XMIN -55*ONE_DEGREE
#define SKID_LARM_XMAX +55*ONE_DEGREE

#define SKID_RARM_YMIN -30*ONE_DEGREE
#define SKID_RARM_YMAX +30*ONE_DEGREE
#define SKID_RARM_XMIN -55*ONE_DEGREE
#define SKID_RARM_XMAX +55*ONE_DEGREE

enum lara_arm_states
{
	LA_LOCKOFF,
	LA_LOCKON,
	LA_RECOIL
};

enum lara_gun_states
{
	LG_ARMLESS,
	LG_HANDSBUSY,
	LG_DRAW,
	LG_UNDRAW,
	LG_READY,
	LG_SPECIAL
};

enum lara_gun_types
{
	LG_UNARMED = 0,
	LG_PISTOLS,
	LG_MAGNUMS,
	LG_UZIS,
	LG_SHOTGUN,
	LG_M16,
	LG_ROCKET,
	LG_GRENADE,
	LG_HARPOON,
	LG_FLARE,
	LG_SKIDOO,
	NUM_WEAPONS
};

struct LARA_ARM
{
	int16_t* frame_base;
	int16_t frame_number;
	int16_t anim_number;
	int16_t lock;
	PHD_ANGLE y_rot, x_rot, z_rot;
	int16_t flash_gun;
};

struct WEAPON_INFO
{
	PHD_ANGLE lock_angles[4];	// Min/Max for Y,X rots
	PHD_ANGLE left_angles[4];	// Min/Max for Left arm Aim
	PHD_ANGLE right_angles[4];	// Min/Max for Right arm Aim
	PHD_ANGLE aim_speed;		// Speed at which arms lockon
	PHD_ANGLE shot_accuracy;	// Random angle range for Shot
	int16_t gun_height;			// Height of Gun from Source Origin
	int16_t target_dist;			// Target distance
	char damage;				// Amount of Damage Done
	char recoil_frame;			// Recoil frame for pistol-type guns
	char flash_time;			// How long GunFlash lasts
	char draw_frame;			// Draw frame for mesh-swap
	int16_t sample_num;			// Sample number for Weapon
};

struct AMMO_INFO
{
	int ammo;
};

inline WEAPON_INFO weapons[NUM_WEAPONS] = {
	{	// NULL
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},{	// PISTOLS
		{PISTOL_LOCK_YMIN,PISTOL_LOCK_YMAX,PISTOL_LOCK_XMIN,PISTOL_LOCK_XMAX},
		{PISTOL_LARM_YMIN,PISTOL_LARM_YMAX,PISTOL_LARM_XMIN,PISTOL_LARM_XMAX},
		{PISTOL_RARM_YMIN,PISTOL_RARM_YMAX,PISTOL_RARM_XMIN,PISTOL_RARM_XMAX},
		10 * ONE_DEGREE,		// aiming speed
		8 * ONE_DEGREE,		// randomness shot_accuracy
		650,				// gun height
		8 * WALL_L,			// targettable distance
		PISTOL_DAMAGE,		// damage
		9,				// Recoil frame for pistol-type guns
		3,				// How long GunFlash lasts for
		0,				// draw frame
		8,				// Sample Number for firing
	},{	// DESERT EAGLE
		{DESERT_LOCK_YMIN,DESERT_LOCK_YMAX,DESERT_LOCK_XMIN,DESERT_LOCK_XMAX},							// DESERT
		{DESERT_LARM_YMIN,DESERT_LARM_YMAX,DESERT_LARM_XMIN,DESERT_LARM_XMAX},
		{DESERT_RARM_YMIN,DESERT_RARM_YMAX,DESERT_RARM_XMIN,DESERT_RARM_XMAX},
		10 * ONE_DEGREE,
		4 * ONE_DEGREE,  // randomness
		650,
		8 * WALL_L,
		17 * PISTOL_DAMAGE,
		16,
		3,
		0,				// draw frame
		121,
	},{	// UZIS
		{PISTOL_LOCK_YMIN,PISTOL_LOCK_YMAX,PISTOL_LOCK_XMIN,PISTOL_LOCK_XMAX},
		{PISTOL_LARM_YMIN,PISTOL_LARM_YMAX,PISTOL_LARM_XMIN,PISTOL_LARM_XMAX},
		{PISTOL_RARM_YMIN,PISTOL_RARM_YMAX,PISTOL_RARM_XMIN,PISTOL_RARM_XMAX},
		10 * ONE_DEGREE,                        // aiming speed
		8 * ONE_DEGREE,                         // randomness
		650,                                  // gun height
		8 * WALL_L,							    // targettable distance
		1 * PISTOL_DAMAGE,                      // damage
		3,									// Recoil frame for pistol-type guns
		3,									// How long GunFlash lasts for
		0,				// draw frame
		43,                                   // Sample Number for firing
	},{	// SHOTGUN
		{SHOTGUN_LOCK_YMIN,SHOTGUN_LOCK_YMAX,SHOTGUN_LOCK_XMIN,SHOTGUN_LOCK_XMAX},
		{SHOTGUN_LARM_YMIN,SHOTGUN_LARM_YMAX,SHOTGUN_LARM_XMIN,SHOTGUN_LARM_XMAX},
		{SHOTGUN_RARM_YMIN,SHOTGUN_RARM_YMAX,SHOTGUN_RARM_XMIN,SHOTGUN_RARM_XMAX},
		10 * ONE_DEGREE,                     	// aiming speed
		0,			                     	// randomness ( Zero for Shotgun As Pellets are Randomised )
		500,                              	// gun height
		8 * WALL_L,								// targettable distance
		3 * PISTOL_DAMAGE,                     	// damage; was 4x for Tomb1
		9,									// Recoil frame for pistol-type guns
		3,									// How long GunFlash lasts for
		10,				// draw frame
		45,                                   // Sample Number for firing
	},{	// H&K
		{M16_LOCK_YMIN,M16_LOCK_YMAX,M16_LOCK_XMIN,M16_LOCK_XMAX},							// M16
		{M16_LARM_YMIN,M16_LARM_YMAX,M16_LARM_XMIN,M16_LARM_XMAX},
		{M16_RARM_YMIN,M16_RARM_YMAX,M16_RARM_XMIN,M16_RARM_XMAX},
		10 * ONE_DEGREE, // aiming speed
		4 * ONE_DEGREE,  // randomness (not much for M16)
		500,
		12 * WALL_L,     // long range weapon
		4 * PISTOL_DAMAGE, // rapid fire though
		0,
		3,
		16,				// draw frame
		0,
	},{	// ROCKET LAUNCHER
		{ROCKET_LOCK_YMIN,ROCKET_LOCK_YMAX,ROCKET_LOCK_XMIN,ROCKET_LOCK_XMAX},							// ROCKET
		{ROCKET_LARM_YMIN,ROCKET_LARM_YMAX,ROCKET_LARM_XMIN,ROCKET_LARM_XMAX},
		{ROCKET_RARM_YMIN,ROCKET_RARM_YMAX,ROCKET_RARM_XMIN,ROCKET_RARM_XMAX},
		10 * ONE_DEGREE, // aiming speed
		8 * ONE_DEGREE,  // randomness
		500,
		8 * WALL_L,
		30 * PISTOL_DAMAGE, // big mother!
		0,
		2,
		12,				// draw frame
		77,
	},{	// GRENADE LAUNCHER
		{GRENADE_LOCK_YMIN,GRENADE_LOCK_YMAX,GRENADE_LOCK_XMIN,GRENADE_LOCK_XMAX},							// GRENADE
		{GRENADE_LARM_YMIN,GRENADE_LARM_YMAX,GRENADE_LARM_XMIN,GRENADE_LARM_XMAX},
		{GRENADE_RARM_YMIN,GRENADE_RARM_YMAX,GRENADE_RARM_XMIN,GRENADE_RARM_XMAX},
		10 * ONE_DEGREE, // aiming speed
		8 * ONE_DEGREE,  // randomness
		500,
		8 * WALL_L,
		20 * PISTOL_DAMAGE, // big mother!
		0,
		2,
		10,				// draw frame
		0,
	},{	// HARPOON
		{HARPOON_LOCK_YMIN,HARPOON_LOCK_YMAX,HARPOON_LOCK_XMIN,HARPOON_LOCK_XMAX},							// HARPOON
		{HARPOON_LARM_YMIN,HARPOON_LARM_YMAX,HARPOON_LARM_XMIN,HARPOON_LARM_XMAX},
		{HARPOON_RARM_YMIN,HARPOON_RARM_YMAX,HARPOON_RARM_XMIN,HARPOON_RARM_XMAX},
		10 * ONE_DEGREE, // aiming speed
		8 * ONE_DEGREE,  // randomness
		500,
		8 * WALL_L,
		6 * PISTOL_DAMAGE, // slow firing (damage halved out of water)
		0,
		2,
		10,				// draw frame
		0,
	},{	// FLARE
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},{	// SKIDOO GUN ?
		{SKID_LOCK_YMIN,SKID_LOCK_YMAX,SKID_LOCK_XMIN,SKID_LOCK_XMAX},							// SKIDOO GUNS
		{SKID_LARM_YMIN,SKID_LARM_YMAX,SKID_LARM_XMIN,SKID_LARM_XMAX},
		{SKID_RARM_YMIN,SKID_RARM_YMAX,SKID_RARM_XMIN,SKID_RARM_XMAX},
		10 * ONE_DEGREE, // aiming speed
		8 * ONE_DEGREE,  // randomness
		400,
		8 * WALL_L,
		3 * PISTOL_DAMAGE, // like uzis
		0,
		2,
		0,
		43,
	}
};

void LaraGun();
void InitialiseNewWeapon();
void find_target_point(ITEM_INFO* item, GAME_VECTOR* target);
void LaraTargetInfo(WEAPON_INFO* winfo);
void LaraGetNewTarget(WEAPON_INFO* winfo);
void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm);
void SmashItem(int16_t item_number, int weapon_type);
void DoProperDetection(short item_number, long x, long y, long z, long xv, long yv, long zv);

bool HitTarget(ITEM_INFO* item, GAME_VECTOR* hitpos, int damage, bool is_player = false);

int FireWeapon(int weapon_type, ITEM_INFO* target, ITEM_INFO* src, PHD_ANGLE* angles);
int WeaponObject(int weapon_type);

PHD_VECTOR get_gun_shell_pos(int bone, int weapon);
void draw_weapon_smoke(ITEM_INFO* item, int weapon, int cl, int cr);
bool is_2guns(short weapon);
bool is_1gun(short weapon);

inline bool g_silenced_hk = false;