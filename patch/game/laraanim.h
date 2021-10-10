#pragma once

#include "items.h"
#include "larafire.h"
#include "box.h"
#include "effects.h"

#define LARA_AIR		(2 * 30 * 30)
#define LARA_HITPOINTS	1000
#define LARA_DASH_TIME	(30 * 4)

#define LARA_EXPOSURE_TIME 	((30 * 10) * 2)	// max exposure time (* precision)
#define LARA_WADE_EXPOSURE	1				// wade exposure rate
#define LARA_SWIM_EXPOSURE	2				// swim exposure rate
#define LARA_HEAT_EXPOSURE	1				// replenish exposure rate

#define LARA_RAD		100	// Global Radius Of Lara for Collisions
#define LARA_HITE		762	// Global Height of Lara Less than 3/4 BLOCK!!!
#define UW_RADIUS		300	// DITTO UnderWater
#define UW_HITE			400
#define CLIMB_WIDTHR	120	// extra width for Lara in climb stance
#define CLIMB_WIDTHL	120	// ## 80

#define DIVE_COUNT		10

#define LARA_LEAN_UNDO  	(ONE_DEGREE)        				// Leaning around Corners ..
#define LARA_LEAN_RATE		((ONE_DEGREE/2) + LARA_LEAN_UNDO)
#define LARA_LEAN_MAX		((10*ONE_DEGREE) + LARA_LEAN_UNDO)

#define LARA_TURN_UNDO  	(2*ONE_DEGREE) 						// Large Turn UNDO limits LAG
#define LARA_TURN_RATE		((ONE_DEGREE/4) + LARA_TURN_UNDO)
#define LARA_JUMP_TURN		((ONE_DEGREE*1) + LARA_TURN_UNDO)
#define LARA_SLOW_TURN		((ONE_DEGREE*2) + LARA_TURN_UNDO)
#define LARA_MED_TURN		((ONE_DEGREE*4) + LARA_TURN_UNDO)
#define LARA_FAST_TURN		((ONE_DEGREE*6) + LARA_TURN_UNDO)

#define LARA_FASTFALL_SPEED	(FASTFALL_SPEED+3)
#define STEPUP_HEIGHT		((STEP_L*3)/2)

#define GF(a, b)	(anims[a].frame_base + b)

#define STARTRUN_A			6

#define VAULT12_A			50										// Vault Anim and Frame Numbers
#define VAULT12_F			GF(VAULT12_A,0)		//759            	// to jump straight to..
#define VAULT34_A			42										// NOTE These can change if Tuby
#define VAULT34_F			GF(VAULT34_A,0)		//614            	// goes and deletes anims from Lara project.
#define LAND_A				31
#define LAND_F				GF(LAND_A,0)		//460
#define FASTSPLAT_A			32
#define FASTSPLAT_F			GF(FASTSPLAT_A,1)	//481
#define FASTFALL_A			23
#define FASTFALL_F			GF(FASTFALL_A,0)
#define STOP_A				11
#define STOP_F				GF(STOP_A,0)		//185
#define BREATH_A			103
#define BREATH_F			GF(BREATH_A,0)
#define FALLDOWN_A			34
#define FALLDOWN_F			GF(FALLDOWN_A,0)	//492
#define STOP_LEFT_A			2
#define STOP_LEFT_F			GF(STOP_LEFT_A,0)	//58
#define STOP_RIGHT_A		3
#define STOP_RIGHT_F		GF(STOP_RIGHT_A,0)	//74
#define HITWALLLEFT_A		53
#define HITWALLLEFT_F		GF(HITWALLLEFT_A,0)	//800
#define HITWALLRIGHT_A		54
#define HITWALLRIGHT_F		GF(HITWALLRIGHT_A,0)	//815
#define RUNSTEPUP_LEFT_A    56
#define RUNSTEPUP_LEFT_F    GF(RUNSTEPUP_LEFT_A,0)	//837
#define RUNSTEPUP_RIGHT_A	55
#define RUNSTEPUP_RIGHT_F   GF(RUNSTEPUP_RIGHT_A,0)	//830
#define WALKSTEPUP_LEFT_A   57
#define WALKSTEPUP_LEFT_F   GF(WALKSTEPUP_LEFT_A,0)	//844
#define WALKSTEPUP_RIGHT_A	58
#define WALKSTEPUP_RIGHT_F  GF(WALKSTEPUP_RIGHT_A,0)	//858
#define WALKSTEPD_LEFT_A    60
#define WALKSTEPD_LEFT_F    GF(WALKSTEPD_LEFT_A,0)	//887
#define WALKSTEPD_RIGHT_A	59
#define WALKSTEPD_RIGHT_F   GF(WALKSTEPD_RIGHT_A,0)	//874
#define BACKSTEPD_LEFT_A	61
#define BACKSTEPD_LEFT_F    GF(BACKSTEPD_LEFT_A,0)	//899
#define BACKSTEPD_RIGHT_A   62
#define BACKSTEPD_RIGHT_F   GF(BACKSTEPD_RIGHT_A,0)	//930
#define LANDFAR_A 			24
#define LANDFAR_F			GF(LANDFAR_A,0)			//358
#define GRABLEDGE_A			96        							// 32
#define GRABLEDGE_F			GF(GRABLEDGE_A,0)		//1493      // 621
#define SWIMGLIDE_A			87
#define SWIMGLIDE_F			GF(SWIMGLIDE_A,0)	//1431
#define FALLBACK_A			93
#define FALLBACK_F			GF(FALLBACK_A,0)	//1473
#define HANG_A				96        						 // 33
#define HANG_F				GF(HANG_A,21)		//1514       // 642
#define STARTHANG_A			96		   						 // 33		// where to start Hang from JumpUp
#define STARTHANG_F			GF(STARTHANG_A,12)	//1505       // 634
#define STOPHANG_A			28 								 // where to jump into Upjump from Hang
#define STOPHANG_F			GF(STOPHANG_A,9)	//448
#define SLIDE_A				70              				 // Slope Slide Forward Animation
#define SLIDE_F				GF(SLIDE_A,0)		//1133
#define SLIDEBACK_A			104     						 // Slope Slide Backward Animation
#define SLIDEBACK_F			GF(SLIDEBACK_A,0)	//1677
#define	COMPRESS_A			73
#define	COMPRESS_F			GF(COMPRESS_A,0)

#define TREAD_A				108
#define TREAD_F				GF(TREAD_A,0)		//1736
#define SURFTREAD_A			114             				// Treading Water Animation
#define SURFTREAD_F			GF(SURFTREAD_A,0)	//1937
#define	SURFSWIM_A			116								// Swimming forward on water surface
#define	SURFSWIM_F			GF(SURFSWIM_A,0)
#define	SURFBACK_A			141								// Swimming backward on water surface
#define	SURFBACK_F			GF(SURFBACK_A,0)
#define SURFTRD2BK_A		140								// Surf Tread to swim back
#define	SURFTRD2BK_F		GF(SURFTRD2BK_A,0)
#define SURFLEFT_A			143								// Swim left
#define SURFLEFT_F			GF(SURFLEFT_A,0)
#define	SURFRIGHT_A			144								// Swim Right
#define	SURFRIGHT_F			GF(SURFRIGHT_A,0)
#define SURFDIVE_A			119             				// Diving Animation..
#define SURFDIVE_F			GF(SURFDIVE_A,0)	//2041
#define SURFCLIMB_A			111								// Climb Out Of Water Animation
#define SURFCLIMB_F         GF(SURFCLIMB_A,0)	//1849
#define	SURF2WADE1_A		190
#define SURF2WADE1_F         GF(SURF2WADE1_A,0)	//1849
#define	SURF2WADE2_A		176
#define SURF2WADE2_F         GF(SURF2WADE2_A,0)	//1849
#define SWIM2QSTND_A		192
#define	SWIM2QSTND_F		GF(SWIM2QSTND_A,0)
#define SURF2QSTND_A		193
#define	SURF2QSTND_F		GF(SURF2QSTND_A,0)
#define	SURF2STND_A			191
#define SURF2STND_F			GF(SURF2STND_A,0)
#define UW2WADE_A			176
#define	UW2WADE_F			GF(UW2WADE_A,0)
#define JUMPIN_A			112
#define JUMPIN_F			GF(JUMPIN_A,0)		//1895
#define WADE_A				177
#define	WADE_F				GF(WADE_A,0)
#define ROLL_A				146
#define ROLL_F			    GF(ROLL_A,2)		//3857
#define WATERROLL_A			203
#define WATERROLL_F			GF(WATERROLL_A,0)
#define RBALL_DEATH_A		139
#define RBALL_DEATH_F       GF(RBALL_DEATH_A,0)	//3561
#define SPIKE_DEATH_A		149
#define SPIKE_DEATH_F		GF(SPIKE_DEATH_A,0)	//3887
#define GRABLEDGEIN_A		150
#define GRABLEDGEIN_F		GF(GRABLEDGEIN_A,0)	//3974
#define THROWFLARE_A		189
#define THROWFLARE_F		GF(THROWFLARE_A,0)

#define	CLIMBSTNC_A			164
#define	CLIMBSTNC_F			GF(CLIMBSTNC_A,0)		//4556
#define CLIMBRIGHT_A		170
#define CLIMBRIGHT_F		GF(CLIMBRIGHT_A,0)
#define	CLIMBLEFT_A			171
#define	CLIMBLEFT_F			GF(CLIMBLEFT_A,0)
#define HANGUP_A			187
#define HANGUP_F			GF(HANGUP_A,0)
#define HANGDOWN_A			188
#define	HANGDOWN_F			GF(HANGDOWN_A,0)
#define GRABRIGHT_A			201
#define	GRABRIGHT_F			GF(GRABRIGHT_A,0)
#define GRABLEFT_A			202
#define	GRABLEFT_F			GF(GRABLEFT_A,0)
#define	CLIMBING_A			161
#define	CLIMBUPLEND_A		162
#define	CLIMBUPREND_A		163
#define	CLIMBDOWN_A			168
#define	CLIMBDOWNLEND_A		166
#define	CLIMBDOWNREND_A		167
#define	CLIMB2HANG_A		194

#define CLIMBUP_LNK1		29
#define CLIMBUP_LNK2		58

#define	CLIMBDN_LNK1		29
#define	CLIMBDN_LNK2		58

#define PICKUPF_UW_A		 206
#define PICKUPF_UW_F		GF(PICKUPF_UW_A,20) // underwater flare pickup

#define PPREADY_F			GF(120,19)			// 2091			// Are we ready to Push or Pull
#define PICKUP_F			GF(135,42)			// 3443            // Are we ready to actually pickup the object??
#define PICKUPF_F			GF(204,58)			// Flare pickup frame
#define PICKUP_UW_F			GF(130,18)			// 2970            // (base+18) Are we ready to actually pickup object UnderWater
#define PICKUPSCION_F		44					//  44 frames  from base
#define USEPUZZLE_F			GF(134,80)			// 3372			// (base+80) Are we ready to swap PuzzleHole Meshes

#define SPAZ_FORWARD_A		125					// Anims for Spastication
#define SPAZ_BACK_A         126
#define SPAZ_RIGHT_A        127
#define SPAZ_LEFT_A         128

#define UWDEATH_A			124
#define UWDEATH_F			GF(UWDEATH_A, 17)

// Tomb Raider III additions

#define STAND2DUCK_A	217	// 218
#define STAND2DUCK_F	GF(STAND2DUCK_A,0)
#define DUCKBREATHE_A	222	// 223
#define DUCKBREATHE_F	GF(DUCKBREATHE_A,0)
#define DASH_A			223	// 224
#define DASH_F			GF(DASH_A,0)
#define DASHDIVE_A		230	// 231
#define DASHDIVE_F		GF(DASHDIVE_A,0)
#define UPJUMPGRAB_A	233
#define UPJUMPGRAB_F	GF(UPJUMPGRAB_A, 0)
#define MONKEYHANG_A	234	// 235
#define MONKEYHANG_F	GF(MONKEYHANG_A, 0)
#define MONKEYSWING_A	239	// 237
#define MONKEYSWING_F	GF(MONKEYSWING_A, 0)

#define MONKEYL_A		253
#define MONKEYL_F		GF(MONKEYL_A, 0)
#define MONKEYR_A		255
#define MONKEYR_F		GF(MONKEYL_A, 0)
#define MONKEY180_A		257
#define MONKEY180_F		GF(MONKEYL_A, 0)
#define ALL4S_A			263
#define ALL4S_F			GF(ALL4S_A, 0)
#define ALL4S2_A		264
#define ALL4S2_F		GF(ALL4S2_A, 0)
#define CRAWL_A			260
#define CRAWL_F			GF(CRAWL_A, 0)

#define ALL4TURNL_A		269
#define ALL4TURNL_F		GF(ALL4TURNL_A, 0)
#define ALL4TURNR_A		270
#define ALL4TURNR_F		GF(ALL4TURNR_A, 0)
#define ALL4BACK_A		276
#define ALL4BACK_F		GF(ALL4BACK_A, 0)
#define HANGTURNL_A		271
#define HANGTURNL_F		GF(HANGTURNL_A, 0)
#define HANGTURNR_A		272
#define HANGTURNR_F		GF(HANGTURNR_A, 0)

#define HANG2STOP_A		150
#define HANG2STOP_F		GF(HANG2STOP_A, 122)

#define HANG2DUCK_A		287
#define HANG2DUCK_F		GF(HANG2DUCK_A, 0)
#define CRAWL2HANG_A	289
#define CRAWL2HANG_F	GF(CRAWL2HANG_A, 0)
#define DUCKPICKUP_A	291
#define DUCKPICKUP_F	GF(DUCKPICKUP_A, 22)
#define DUCKPICKUPF_A	312
#define DUCKPICKUPF_F	GF(DUCKPICKUPF_A, 22)
#define ALL4SPICKUP_A	291
#define ALL4SPICKUP_F	GF(ALL4SPICKUP_A, 20)

#define SPAZ_DUCKF_A	294
#define SPAZ_DUCKB_A	293
#define SPAZ_DUCKL_A	296
#define SPAZ_DUCKR_A	295

// TR IV/V additions

// corner grabbing

#define EXTCORNERL_A	312
#define EXTCORNERL_F	GF(EXTCORNERL_A, 0)
#define INTCORNERL_A	314
#define INTCORNERL_F	GF(INTCORNERL_A, 0)
#define EXTCORNERR_A	316
#define EXTCORNERR_F	GF(EXTCORNERR_A, 0)
#define INTCORNERR_A	318
#define INTCORNERR_F	GF(INTCORNERR_A, 0)

// crawl jump down

#define CRAWLJUMP_A		320
#define CRAWLJUMP_F		GF(CRAWLJUMP_A, 0)

// polerope

#define STAT2POLE_A		321
#define STAT2POLE_F		GF(STAT2POLE_A, 0)

#define POLESTAT_A		322
#define POLESLIDEE_A	330

#define REACH2POLE_A 	334
#define REACH2POLE_F 	GF(REACH2POLE_A, 0)

#define JUMP2POLE_A 	335
#define JUMP2POLE_F 	GF(JUMP2POLE_A, 0)

enum lara_water_states
{
	LARA_ABOVEWATER,
	LARA_UNDERWATER,
	LARA_SURFACE,
	LARA_CHEAT,
	LARA_WADE
};

enum lara_meshes
{
	HIPS,
	THIGH_L, CALF_L, FOOT_L,
	THIGH_R, CALF_R, FOOT_R,
	TORSO,
	UARM_R, LARM_R, HAND_R,
	UARM_L, LARM_L, HAND_L,
	HEAD,
	MAX_LARA_MESHES,
};

enum lara_states
{
	AS_WALK = 0,
	AS_RUN,
	AS_STOP,
	AS_FORWARDJUMP,
	AS_POSE,
	AS_FASTBACK,
	AS_TURN_R,
	AS_TURN_L,
	AS_DEATH,
	AS_FASTFALL,
	AS_HANG,
	AS_REACH,
	AS_SPLAT,
	AS_TREAD,
	AS_LAND,
	AS_COMPRESS,
	AS_BACK,
	AS_SWIM,
	AS_GLIDE,
	AS_NULL,
	AS_FASTTURN,
	AS_STEPRIGHT,
	AS_STEPLEFT,
	AS_HIT,
	AS_SLIDE,
	AS_BACKJUMP,
	AS_RIGHTJUMP,
	AS_LEFTJUMP,
	AS_UPJUMP,
	AS_FALLBACK,
	AS_HANGLEFT,
	AS_HANGRIGHT,
	AS_SLIDEBACK,
	AS_SURFTREAD,
	AS_SURFSWIM,
	AS_DIVE,
	AS_PUSHBLOCK,
	AS_PULLBLOCK,
	AS_PPREADY,
	AS_PICKUP,
	AS_SWITCHON,
	AS_SWITCHOFF,
	AS_USEKEY,
	AS_USEPUZZLE,
	AS_UWDEATH,
	AS_ROLL,
	AS_SPECIAL,
	AS_SURFBACK,
	AS_SURFLEFT,
	AS_SURFRIGHT,
	AS_USEMIDAS,
	AS_DIEMIDAS,
	AS_SWANDIVE,
	AS_FASTDIVE,
	AS_GYMNAST,
	AS_WATEROUT,
	AS_CLIMBSTNC,
	AS_CLIMBING,
	AS_CLIMBLEFT,
	AS_CLIMBEND,
	AS_CLIMBRIGHT,
	AS_CLIMBDOWN,
	AS_LARATEST1,
	AS_LARATEST2,
	AS_LARATEST3,
	AS_WADE,
	AS_WATERROLL,
	AS_FLAREPICKUP,
	AS_TWIST,
	AS_KICK,
	AS_DEATHSLIDE,
	AS_DUCK,
	AS_DUCKROLL,
	AS_DASH,
	AS_DASHDIVE,
	AS_HANG2,
	AS_MONKEYSWING,
	AS_MONKEYL,
	AS_MONKEYR,
	AS_MONKEY180,
	AS_ALL4S,
	AS_CRAWL,
	AS_HANGTURNL,
	AS_HANGTURNR,
	AS_ALL4TURNL,
	AS_ALL4TURNR,
	AS_CRAWLBACK,
	AS_HANG2DUCK,
	AS_CRAWL2HANG,
	AS_CORNEREXTL,
	AS_CORNERINTL,
	AS_CORNEREXTR,
	AS_CORNERINTR,
	AS_CONTROLLED,
	AS_POLESTAT,
	AS_POLELEFT,
	AS_POLERIGHT,
	AS_POLEUP,
	AS_POLEDOWN,
	AS_LAST
};

enum lara_extras
{
	EXTRA_BREATH,
	EXTRA_PLUNGER,
	EXTRA_YETIKILL,
	EXTRA_SHARKKILL,
	EXTRA_AIRLOCK,
	EXTRA_GONGBONG,
	EXTRA_DINOKILL,
	EXTRA_PULLDAGGER,
	EXTRA_STARTANIM,
	EXTRA_STARTHOUSE,
	EXTRA_FINALANIM,
	EXTRA_TRAINKILL,
	EXTRA_RAPIDSDROWN
};