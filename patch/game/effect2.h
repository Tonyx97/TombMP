#pragma once

#include "anim.h"

#define	MAX_DYNAMIC2	64

#define	MAX_SPARKS	1024
#define	MAX_SPLASHES	8
#define	MAX_RIPPLES	32
#define	MAX_BATS		32

#define	SP_FLAT		1
#define	SP_SCALE		2
#define	SP_BLOOD		4
#define	SP_DEF		8
#define	SP_ROTATE		16
#define	SP_EXPLOSION	32
#define	SP_FX		64
#define	SP_ITEM		128
#define	SP_WIND		256
#define	SP_EXPDEF		512
#define	SP_USEFXOBJPOS	1024
#define	SP_UNDERWEXP	2048
#define	SP_NODEATTATCH	4096
#define	SP_PLASMAEXP	8192

#define	SD_EXPLOSION	1
#define	SD_UWEXPLOSION	2

#define	SEMITRANS		1
#define	COLADD		2
#define	COLSUB		3
#define	WEIRD		4

#define	ROCKET_YOFF		180
#define	ROCKET_ZOFF		72	// 20
#define	GRENADE_YOFF	180
#define	GRENADE_ZOFF	80
#define	SHOTGUN_YOFF	228
#define	SHOTGUN_ZOFF	32
#define	M16_YOFF		228
#define	M16_ZOFF		96
#define	PISTOLS_YOFF	128
#define	PISTOLS_ZOFF	40
#define	MAGNUMS_YOFF	160
#define	MAGNUMS_ZOFF	56
#define	UZIS_YOFF		140
#define	UZIS_ZOFF		48

#define	SPL_ON			1
#define	SPL_TRANS			2
#define	SPL_RIPPLEINNER	4
#define	SPL_RIPPLEMIDDLE	8
#define	SPL_MORETRANS		16
#define	SPL_BLOOD			32
#define SPL_KAYAK_SPLASH	64

#define	B_ON				1

enum
{
	SPN_PILOTFLAME,
	SPN_WINGMUTEPARTICLES,
	SPN_PUNKFLAME,
	SPN_PENDULUMFLAME,
	SPN_TONYHANDLFLAME,
	SPN_TONYHANDRFLAME,
	SPN_CLAWMUTEPLASMA,
	SPN_WILLBOSSLPLASMA,
	SPN_WILLBOSSRPLASMA,
	SPN_CLEANER5,
	SPN_CLEANER9,
	SPN_CLEANER13
};

struct DYNAMIC
{
	long x;
	long y;
	long z;

	unsigned short falloff;
	unsigned char used;
	unsigned char pad1[1];

	unsigned char on;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct SPARKS
{
	long x, y, z;

	short Xvel;
	short Yvel;
	short Zvel;
	short Gravity;					// Y vel.

	short RotAng;
	short Flags;					// See SP_... defs above.

	unsigned char sWidth;
	unsigned char dWidth;
	unsigned char sHeight;
	unsigned char dHeight;

	unsigned char Friction; 		// X & Z vels - (vels -= vels>>Friction)	// Top nibble = Yvel friction.
	unsigned char Width;
	unsigned char Height;
	unsigned char Scalar;

	unsigned char Def;
	signed char	RotAdd;
	signed char	MaxYvel;
	unsigned char On;				// Used ?

	unsigned char sR;				//
	unsigned char sG;  				// Start RGB.
	unsigned char sB;  				//
	unsigned char dR;				//

	unsigned char dG;  				// End RGB.
	unsigned char dB;  				//
	unsigned char R;				//
	unsigned char G;  				// Current RGB.

	unsigned char B;  				//
	unsigned char ColFadeSpeed;		// Speed at which to fade to endcol in frames.
	unsigned char FadeToBlack;		// When to start fading to black (life left). (0 = never).
	unsigned char sLife;

	unsigned char Life;				// Start life, current life.
	unsigned char TransType;		// 0 = Semi, 1 = Col add, 2 = Colsub, 3 = weird!
	unsigned char extras;
	signed char	Dynamic;			// Which dynamic-light-controller to use. -1 = no dynamic.

	unsigned char	FxObj;			// Attached to Fx or Object number.
	unsigned char	RoomNumber;		// For those which have the stupid fucking waste of time shit piss fuck.
	unsigned char	NodeNumber;		// Index into a table of bite_infos for attatching particles to nodes. (Defined in DRAWSPKS.C).
	unsigned char	pad;

};

struct SP_DYNAMIC
{
	unsigned char On;
	unsigned char Falloff;
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char Flags;
	unsigned char Pad[2];
};

struct SPLASH_VERTS
{
	short wx;
	short wy;				// XYZ offsets from origin.
	short wz;

	short xv;
	long yv;				// XYZ velocities (8 bit fraction).
	short zv;

	short oxv;
	short ozv;

	unsigned char friction;	// XZ friction (bit shift).
	unsigned char gravity;	//  Y gravity.
};

struct SPLASH_STRUCT
{
	long x;
	long y;
	long z;
	char flags; 		// bit 0 = on/off;
	unsigned char life;	// dictates transparency level.
	char pad[2];
	SPLASH_VERTS sv[48];
};

struct RIPPLE_STRUCT
{
	long x;
	long y;
	long z;
	char flags; 		// bit 0 = on/off;
	unsigned char life;	// dictates transparency level.
	unsigned char size;	// << 2
	unsigned char init;
};

struct SPLASH_SETUP
{
	long x;
	long y;
	long z;
	short InnerXZoff;
	short InnerXZsize;
	short InnerYsize;
	short InnerXZvel;
	short InnerYvel;
	short InnerGravity;
	short InnerFriction;
	short MiddleXZoff;
	short MiddleXZsize;
	short MiddleYsize;
	short MiddleXZvel;
	short MiddleYvel;
	short MiddleGravity;
	short MiddleFriction;
	short OuterXZoff;
	short OuterXZsize;
	short OuterXZvel;
	short OuterFriction;
};

struct BAT_STRUCT
{
	long x, y, z;
	short angle;
	short speed;
	unsigned char WingYoff;	// Make the wings flap;
	unsigned char flags;  	// bit 0 = on/off.
	unsigned char life;
	unsigned char pad;
};

inline DYNAMIC dynamics[MAX_DYNAMIC2 << 1];
inline RIPPLE_STRUCT ripples[MAX_RIPPLES];
inline SPLASH_STRUCT splashes[MAX_SPLASHES];
inline SPLASH_SETUP	splash_setup;
inline BAT_STRUCT bats[MAX_BATS];

inline SP_DYNAMIC spark_dynamics[32];
inline SPARKS spark[MAX_SPARKS];

inline long wibble = 0;
inline long splash_count = 0;
inline long smoke_count_l;
inline long smoke_count_r;
inline long smoke_weapon;
inline int next_spark = 0,
		   g_wind_x = 0,
		   g_wind_z = 0,
		   g_effects_draw_distance = -1;

inline long KillEverythingFlag = 0;

void KillAllCurrentItems(short item_number);

void InitialiseSparks();
void TriggerSideFlame(long x, long y, long z, long angle, long speed, long pilot);
long GetFreeSpark();
void UpdateSparks();
void TriggerRicochetSpark(GAME_VECTOR* pos, long angle, long size);
void TriggerFlareSparks(long x, long y, long z, long xv, long yv, long zv, long smoke);
void TriggerExplosionSparks(long x, long y, long z, long extra_trigs, long dyn, long uw, short roomnum);
void TriggerExplosionSmokeEnd(long x, long y, long z, long uw);
void TriggerExplosionSmoke(long x, long y, long z, long uw);
void TriggerFireSmoke(long x, long y, long z, long body_part, long type);
void TriggerFireFlame(long x, long y, long z, long body_part, long type);
void TriggerFireFlame(long x, long y, long z, long body_part, int r, int g, int b);
void TriggerStaticFlame(long x, long y, long z, long size);
void TriggerAlertLight(long x, long y, long z, long r, long g, long b, long angle, short room_no);
void DetachSpark(long num, long type);
void ControlGunShell(short fx_number);
void TriggerGunShell(int x, int y, int z, int ry, long type, long weapon, bool left, short room, bool sync = true);
void TriggerGunSmoke(long x, long y, long z, long xv, long yv, long zv, long initial, long weapon, long count, short room_id = NO_ROOM);
void TriggerExplosionBubble(long x, long y, long z, short roomnum);
void TriggerWaterfallMist(long x, long y, long z, long angle);
void ControlSmokeEmitter(int16_t item_number);
void ControlColouredLights(int16_t item_number);

void UpdateBats();
void BatEmitterControl(short item_number);
void TriggerBats(long x, long y, long z, long angle);
void S_DrawBat();
void SetupSplash(SPLASH_SETUP* spl);
void UpdateSplashes();
void S_DrawSplashes();

RIPPLE_STRUCT* SetupRipple(long x, long y, long z, long size, long moretrans);

void TriggerShotgunSparks(long x, long y, long z, long xv, long yv, long zv);
void TriggerRocketFlame(long x, long y, long z, long xv, long yv, long zv, long item_no);
void TriggerRocketSmoke(long x, long y, long z, long body_part);
void TriggerDartSmoke(long x, long y, long z, long xv, long zv, long hit);
void TriggerBlood(long x, long y, long z, long angle, long num);
void TriggerBloodD(long x, long y, long z, long angle, long num);
void TriggerUnderwaterBlood(long x, long y, long z, long size);
void TriggerUnderwaterBloodD(long x, long y, long z, long size);
void TriggerBubble(long x, long y, long z, long size, long sizerange, short itemnum);
void TriggerDynamicLight(long x, long y, long z, long falloff, long r, long g, long b);
void TriggerBreath(long x, long y, long z, long xv, long yv, long zv);
void KillEverything();
void ClearDynamics();