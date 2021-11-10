import prof;

#include <specific/standard.h>
#include <specific/global.h>

#include <3dsystem/3d_gen.h>

#include <shared/scripting/resource_system.h>
#include <shared/game_net/entity_type.h>

#include <scripting/events.h>

#include <mp/client.h>
#include <mp/game/level.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "invfunc.h"
#include "laramisc.h"
#include "lara1gun.h"
#include "laraflar.h"
#include "hair.h"
#include "setup.h"
#include "lot.h"
#include "health.h"
#include "game.h"
#include "bird.h"
#include "boat.h"
#include "dog.h"
#include "effects.h"
#include "effect2.h"
#include "moveblok.h"
#include "people.h"
#include "pickup.h"
#include "rat.h"
#include "orca.h"
#include "fish.h"
#include "traps.h"
#include "dino.h"
#include "objlight.h"
#include "quadbike.h"
#include "minecart.h"
#include "tiger.h"
#include "diver.h"
#include "orca.h"
#include "kayak.h"
#include "sub.h"
#include "triboss.h"
#include "tonyboss.h"
#include "londonboss.h"
#include "willboss.h"
#include "flamer.h"
#include "lizman.h"
#include "lasers.h"
#include "tribeaxe.h"
#include "blowpipe.h"
#include "raptor.h"
#include "compy.h"
#include "51rocket.h"
#include "shiva.h"
#include "51laser.h"
#include "firehead.h"
#include "wingmute.h"
#include "monkey.h"
#include "cobra.h"
#include "cleaner.h"
#include "oilred.h"
#include "oilsmg.h"
#include "punk.h"
#include "londonsec.h"
#include "clawmute.h"
#include "hybrid.h"
#include "sealmute.h"
#include "armysmg.h"
#include "51baton.h"
#include "51civvy.h"
#include "mpgun.h"
#include "prisoner.h"
#include "autogun.h"
#include "swat.h"
#include "dragfire.h"
#include "biggun.h"
#include "rapmaker.h"
#include "flymaker.h"
#include "train.h"
#include "winston.h"
#include "boomute.h"
#include "croc.h"
#include "target.h"
#include "fusebox.h"

#include <specific/file.h>
#include <specific/init.h>
#include <specific/output.h>
#include <specific/litesrc.h>

#define OUTSIDE_Z				64
#define UNIT_SHADOW				256
#define DEFAULT_RADIUS			10
#define DOG_HIT_POINTS			16
#define DOG_RADIUS				(WALL_L / 3)
#define MOUSE_HIT_POINTS		4
#define MOUSE_RADIUS			(WALL_L / 10)
#define COMPY_HIT_POINTS		10
#define COMPY_RADIUS			(WALL_L / 10)
#define COBRA_HIT_POINTS		8
#define COBRA_RADIUS			(WALL_L / 10)
#define MONKEY_HIT_POINTS		8
#define MONKEY_RADIUS			(WALL_L / 10)
#define CULT1_HIT_POINTS		25
#define CULT1_RADIUS			(WALL_L / 10)
#define TONYBOSS_HIT_POINTS		200
#define TONYBOSS_RADIUS			(WALL_L / 10)
#define TRIBEBOSS_HIT_POINTS	200
#define TRIBEBOSS_RADIUS		(WALL_L / 10)
#define LONDONBOSS_HIT_POINTS	300
#define LONDONBOSS_RADIUS		(WALL_L / 10)
#define WILLARDBOSS_HIT_POINTS	100
#define WILLARDBOSS_RADIUS		(WALL_L / 10)
#define SHARK_HIT_POINTS		30
#define SHARK_RADIUS			(WALL_L / 3)
#define TIGER_HIT_POINTS		24
#define TIGER_RADIUS			(WALL_L / 3)
#define DIVER_HIT_POINTS		20
#define DIVER_RADIUS			(WALL_L / 3)
#define ORCA_HIT_POINTS			5000
#define ORCA_RADIUS				(WALL_L / 3)
#define WORK1_HIT_POINTS		25
#define WORK1_RADIUS			(WALL_L / 10)
#define WORK2_HIT_POINTS		20
#define WORK2_RADIUS			(WALL_L / 10)
#define WORK3_HIT_POINTS		27
#define WORK3_RADIUS			(WALL_L / 10)
#define LIZMAN_HIT_POINTS		36
#define LIZMAN_RADIUS			(WALL_L / 5)
#define BATON_HIT_POINTS		25
#define BATON_RADIUS			(WALL_L / 10)
#define CIVVY_HIT_POINTS		15
#define CIVVY_RADIUS			(WALL_L / 10)
#define PUNK_HIT_POINTS			20
#define PUNK_RADIUS				(WALL_L / 10)
#define CROC_HIT_POINTS			42
#define CROC_RADIUS				(WALL_L / 3)
#define OILSMG_HIT_POINTS		30
#define OILSMG_RADIUS			(WALL_L / 10)
#define SWAT_HIT_POINTS			45
#define SWAT_RADIUS				(WALL_L / 10)
#define ARMYSMG_HIT_POINTS		30
#define ARMYSMG_RADIUS			(WALL_L / 10)
#define PRISONER_HIT_POINTS		20
#define PRISONER_RADIUS			(WALL_L / 10)
#define AUTOGUN_HIT_POINTS		100
#define AUTOGUN_RADIUS			(WALL_L / 10)
#define FLAMER_HIT_POINTS		36
#define FLAMER_RADIUS			(WALL_L / 10)
#define CULT3_HIT_POINTS		150
#define CULT3_RADIUS			(WALL_L / 10)
#define TRIBEAXE_HIT_POINTS  	28
#define TRIBEAXE_RADIUS	    	(WALL_L / 10)
#define BLOWPIPE_HIT_POINTS  	28
#define BLOWPIPE_RADIUS	    	(WALL_L / 10)
#define SHIVA_HIT_POINTS  		100
#define SHIVA_RADIUS	    	(WALL_L / 3)
#define MONK_HIT_POINTS			30
#define MONK_RADIUS				(WALL_L / 10)
#define WINGMUTE_HIT_POINTS		24
#define WINGMUTE_RADIUS			(WALL_L / 5)
#define VULTURE_HIT_POINTS		18
#define VULTURE_RADIUS			(WALL_L / 5)
#define CROW_HIT_POINTS			8
#define CROW_RADIUS				(WALL_L / 5)
#define BANDIT1_HIT_POINTS		34
#define BANDIT1_RADIUS			(WALL_L / 10)
#define BANDIT2_HIT_POINTS		28
#define BANDIT2_RADIUS			(WALL_L / 10)
#define CULT2_HIT_POINTS		60
#define CULT2_RADIUS			(WALL_L / 10)
#define RAPTOR_HIT_POINTS		90
#define RAPTOR_RADIUS			(WALL_L / 3)
#define DINO_HIT_POINTS			800
#define DINO_RADIUS				(WALL_L / 2)
#define CLAWMUTE_HIT_POINTS		130
#define CLAWMUTE_RADIUS			(WALL_L / 4)
#define HYBRID_HIT_POINTS		90
#define HYBRID_RADIUS			(WALL_L / 8)
#define SEALMUTE_HIT_POINTS		50
#define SEALMUTE_RADIUS			(WALL_L / 5)
#define AI_PICKUP_HIT_POINTS	0
#define WASP_FACTORY_HIT_POINTS 150

#define IMP_HIT_POINTS			12
#define IMP_RADIUS				(WALL_L / 10)

#define INIT_PICKUP(a)	obj = &objects[a];	\
						obj->collision = PickUpCollision;

#define INIT_PICKUP_ANIM(a)	obj = &objects[a];	\
							obj->control = AnimatingPickUp;	\
							obj->collision = PickUpCollision;	\
							obj->intelligent = 0;

#define INIT_KEYHOLE(a)	obj = &objects[a];	\
						obj->collision = KeyHoleCollision;

#define INIT_PUZZLEHOLE(a)	obj = &objects[a];	\
							obj->control = ControlAnimating_1_4;	\
							obj->collision = PuzzleHoleCollision;

#define INIT_DOOR(a)	obj = &objects[a]; \
						obj->initialise = InitialiseDoor; \
						obj->draw_routine = DrawUnclippedItem; \
						obj->control = DoorControl; \
						obj->collision = DoorCollision;

#define INIT_MOVBLOCK(a)	obj = &objects[a]; \
							obj->initialise = InitialiseMovingBlock; \
							obj->control = MovableBlock; \
							obj->collision = MovableBlockCollision; \
   							obj->draw_routine = DrawMovableBlock;

enum
{
	OILRED_BITE,
	WHITE_SOLDIER_BITE,
	SWAT_GUN_BITE,
	SWAT_GUN_LASER_BITE,
	AUTOGUN_LEFT_BITE,
	AUTOGUN_RIGHT_BITE,
	ARMYSMG_GUN_BITE,
};

void InitialiseGameFlags()
{
	for (int i = 0; i < MAX_FLIPMAPS; ++i) flipmap[i] = 0;

	flip_status = 0;
	sunset = 0;
	ammo_text = nullptr;
	level_complete = false;
	flipeffect = -1;
	carcass_item = -1;
	compys_attack_lara = 0;

	lara.item_number = NO_ITEM;
}

void BaddyObjects()
{
	OBJECT_INFO* obj;

	obj = &objects[LARA];
	obj->initialise = InitialiseLaraLoad;
	obj->shadow_size = (UNIT_SHADOW * 10) / 16;
	obj->hit_points = LARA_HITPOINTS;
	obj->draw_routine = DrawDummyItem;
	obj->intelligent = 1;
	obj->collision = CreatureCollision;

	obj = &objects[LARA_EXTRA];
	obj->control = ControlLaraExtra;

	if ((obj = &objects[COBRA])->loaded)
	{
		obj->initialise = InitialiseCobra;
		obj->control = CobraControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = COBRA_HIT_POINTS;
		obj->radius = COBRA_RADIUS;
		obj->intelligent = 1;
		obj->non_lot = 1;

		bones[obj->bone_index + 0 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_Y;
	}

	if ((obj = &objects[MONKEY])->loaded)
	{
		if (!objects[MESHSWAP2].loaded)
			S_ExitSystem();

		obj->draw_routine = DrawMonkey;
		obj->initialise = InitialiseMonkey;
		obj->control = MonkeyControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = MONKEY_HIT_POINTS;
		obj->radius = MONKEY_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 0 * 4] |= ROT_Z;
		bones[obj->bone_index + 7 * 4] |= ROT_X;
		bones[obj->bone_index + 7 * 4] |= ROT_Y;
	}

	if ((obj = &objects[ROBOT_SENTRY_GUN])->loaded)
	{
		obj->initialise = InitialiseAutogun;
		obj->control = AutogunControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = 0;
		obj->hit_points = AUTOGUN_HIT_POINTS;
		obj->radius = AUTOGUN_RADIUS;
		obj->intelligent = 1;
		obj->non_lot = 1;
		obj->bite_offset = AUTOGUN_LEFT_BITE;

		bones[obj->bone_index + 0 * 4] |= ROT_Y;
		bones[obj->bone_index + 1 * 4] |= ROT_X;
	}

	if ((obj = &objects[DIVER])->loaded)
	{
		obj->control = DiverControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = DIVER_HIT_POINTS;
		obj->radius = DIVER_RADIUS;
		obj->intelligent = 1;
		obj->water_creature = 1;
		obj->pivot_length = 50;
	}

	if ((obj = &objects[WHALE])->loaded)
	{
		obj->control = OrcaControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = DONT_TARGET;
		obj->pivot_length = 200;
		obj->radius = ORCA_RADIUS;
		obj->intelligent = 1;
		obj->water_creature = 1;
	}

	if ((obj = &objects[TREX])->loaded)
	{
		obj->control = DinoControl;
		obj->collision = CreatureCollision;
		obj->hit_points = DINO_HIT_POINTS;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->pivot_length = 1800;
		obj->radius = DINO_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 9 * 4] |= ROT_Y;
		bones[obj->bone_index + 11 * 4] |= ROT_Y;
		bones[obj->bone_index + 20 * 4] |= ROT_Y;
		bones[obj->bone_index + 22 * 4] |= ROT_Y;
	}

	if ((obj = &objects[RAPTOR])->loaded)
	{
		obj->control = RaptorControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = RAPTOR_HIT_POINTS;
		obj->pivot_length = 600;
		obj->radius = RAPTOR_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 20 * 4] |= ROT_Y;
		bones[obj->bone_index + 21 * 4] |= ROT_Y;
		bones[obj->bone_index + 23 * 4] |= ROT_Y;
		bones[obj->bone_index + 25 * 4] |= ROT_Y;
	}

	if ((obj = &objects[DOG])->loaded)
	{
		obj->initialise = InitialiseDog;
		obj->control = DogControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = DOG_HIT_POINTS;
		obj->pivot_length = 300;
		obj->radius = DOG_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 2 * 4] |= ROT_Y;
		bones[obj->bone_index + 2 * 4] |= ROT_X;
	}

	if ((obj = &objects[HUSKIE])->loaded)
	{
		obj->initialise = InitialiseDog;
		obj->control = DogControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = DOG_HIT_POINTS;
		obj->pivot_length = 300;
		obj->radius = DOG_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 2 * 4] |= ROT_Y;
		bones[obj->bone_index + 2 * 4] |= ROT_X;
	}

	if ((obj = &objects[SMALL_RAT])->loaded)
	{
		obj->control = MouseControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = MOUSE_HIT_POINTS;
		obj->pivot_length = 50;
		obj->radius = MOUSE_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 3 * 4] |= ROT_Y;
	}

	if ((obj = &objects[COMPY])->loaded)
	{
		obj->initialise = InitialiseCompy;
		obj->control = CompyControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 3;
		obj->hit_points = COMPY_HIT_POINTS;
		obj->pivot_length = 50;
		obj->radius = COMPY_RADIUS;
		obj->intelligent = 1;
		obj->non_lot = 1;

		bones[obj->bone_index + 1 * 4] |= ROT_Y;
		bones[obj->bone_index + 2 * 4] |= ROT_Y;
	}

	if ((obj = &objects[TRIBEBOSS])->loaded)
	{
		obj->initialise = InitialiseTribeBoss;
		obj->control = TribeBossControl;
		obj->draw_routine = S_DrawTribeBoss;
		obj->collision = CreatureCollision;
		obj->shadow_size = 0;
		obj->hit_points = TRIBEBOSS_HIT_POINTS;
		obj->pivot_length = 50;
		obj->radius = TRIBEBOSS_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 4 * 4] |= ROT_Y;
		bones[obj->bone_index + 7 * 4] |= ROT_Y | ROT_X;
	}

	if ((obj = &objects[TONY])->loaded)
	{
		obj->initialise = InitialiseTonyBoss;
		obj->control = TonyBossControl;
		obj->draw_routine = S_DrawTonyBoss;
		obj->collision = CreatureCollision;
		obj->shadow_size = 0;
		obj->hit_points = TONYBOSS_HIT_POINTS;
		obj->pivot_length = 50;
		obj->radius = TONYBOSS_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
	}

	if ((obj = &objects[LON_BOSS])->loaded)
	{
		obj->initialise = InitialiseLondonBoss;
		obj->control = LondonBossControl;
		obj->draw_routine = S_DrawLondonBoss;
		obj->collision = CreatureCollision;
		obj->shadow_size = 0;
		obj->hit_points = LONDONBOSS_HIT_POINTS;
		obj->pivot_length = 50;
		obj->radius = LONDONBOSS_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
	}

	if ((obj = &objects[WILLARD_BOSS])->loaded)
	{
		obj->initialise = InitialiseWillBoss;
		obj->control = WillBossControl;
		obj->draw_routine = S_DrawWillBoss;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = WILLARDBOSS_HIT_POINTS;
		obj->pivot_length = 50;
		obj->radius = WILLARDBOSS_RADIUS;
		obj->intelligent = 1;
	}

	if ((obj = &objects[TIGER])->loaded)
	{
		obj->control = TigerControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = TIGER_HIT_POINTS;
		obj->pivot_length = 200;
		obj->radius = TIGER_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 21 * 4] |= ROT_Y;
	}

	if ((obj = &objects[SHIVA])->loaded)
	{
		if (!objects[MESHSWAP1].loaded)
			S_ExitSystem();

		obj->initialise = InitialiseShiva;
		obj->draw_routine = DrawShiva;
		obj->control = ShivaControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = SHIVA_HIT_POINTS;
		obj->radius = SHIVA_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 25 * 4] |= ROT_Y;
		bones[obj->bone_index + 25 * 4] |= ROT_X;
	}

	if ((obj = &objects[WHITE_SOLDIER])->loaded)
	{
		obj->initialise = InitialiseOilSMG;
		obj->control = OilSMGControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = OILSMG_HIT_POINTS;
		obj->radius = OILSMG_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
		obj->bite_offset = WHITE_SOLDIER_BITE;

		bones[obj->bone_index + 0 * 4] |= ROT_Y;
		bones[obj->bone_index + 0 * 4] |= ROT_X;
		bones[obj->bone_index + 7 * 4] |= ROT_Y;
	}

	if ((obj = &objects[MUTANT2])->loaded)
	{
		obj->control = ClawmuteControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = CLAWMUTE_HIT_POINTS;
		obj->radius = CLAWMUTE_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 0 * 4] |= ROT_Z;
		bones[obj->bone_index + 0 * 4] |= ROT_X;
		bones[obj->bone_index + 7 * 4] |= ROT_Y;
	}

	if ((obj = &objects[MUTANT3])->loaded)
	{
		obj->control = HybridControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = HYBRID_HIT_POINTS;
		obj->radius = HYBRID_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 0 * 4] |= ROT_Z;
		bones[obj->bone_index + 0 * 4] |= ROT_X;
		bones[obj->bone_index + 7 * 4] |= ROT_Y;
	}

	if ((obj = &objects[BURNT_MUTANT])->loaded)
	{
		obj->control = SealmuteControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = SEALMUTE_HIT_POINTS;
		obj->radius = SEALMUTE_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 8 * 4] |= ROT_Z;
		bones[obj->bone_index + 8 * 4] |= ROT_X;
		bones[obj->bone_index + 9 * 4] |= ROT_Y;
	}

	if ((obj = &objects[SWAT_GUN])->loaded)
	{
		obj->initialise = InitialiseSwat;
		obj->control = SwatControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = SWAT_HIT_POINTS;
		obj->radius = SWAT_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
		obj->bite_offset = SWAT_GUN_BITE;

		bones[obj->bone_index + 0 * 4] |= ROT_Y;
		bones[obj->bone_index + 0 * 4] |= ROT_X;
		bones[obj->bone_index + 7 * 4] |= ROT_Y;
	}

	if ((obj = &objects[STHPAC_MERCENARY])->loaded)
	{
		obj->initialise = InitialiseArmySMG;
		obj->control = ArmySMGControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = ARMYSMG_HIT_POINTS;
		obj->radius = ARMYSMG_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
		obj->bite_offset = ARMYSMG_GUN_BITE;

		bones[obj->bone_index + 0 * 4] |= ROT_Y;
		bones[obj->bone_index + 0 * 4] |= ROT_X;
		bones[obj->bone_index + 7 * 4] |= ROT_Y;
	}

	if ((obj = &objects[LIZARD_MAN])->loaded)
	{
		obj->control = LizManControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = LIZMAN_HIT_POINTS;
		obj->radius = LIZMAN_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 1 * 4] |= ROT_Z;
		bones[obj->bone_index + 9 * 4] |= ROT_Z;
	}

	if ((obj = &objects[MP1])->loaded)
	{
		obj->initialise = InitialiseBaton;
		obj->control = BatonControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = BATON_HIT_POINTS;
		obj->radius = BATON_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
	}

	if ((obj = &objects[CIVVIE])->loaded)
	{
		obj->initialise = InitialiseCivvy;
		obj->control = CivvyControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = CIVVY_HIT_POINTS;
		obj->radius = CIVVY_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
	}

	if ((obj = &objects[BOB])->loaded)
	{
		obj->initialise = InitialisePrisoner;
		obj->control = PrisonerControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = PRISONER_HIT_POINTS;
		obj->radius = PRISONER_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
	}

	if ((obj = &objects[FLAMETHROWER_BLOKE])->loaded)
	{
		obj->control = FlamerControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = FLAMER_HIT_POINTS;
		obj->radius = FLAMER_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 0 * 4] |= ROT_Y;
		bones[obj->bone_index + 0 * 4] |= ROT_X;
		bones[obj->bone_index + 7 * 4] |= ROT_Y;
	}

	if ((obj = &objects[TRIBEAXE])->loaded)
	{
		obj->control = TribeAxeControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = TRIBEAXE_HIT_POINTS;
		obj->radius = TRIBEAXE_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 13 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_Y;
	}

	if ((obj = &objects[BLOWPIPE])->loaded)
	{
		obj->control = BlowpipeControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = BLOWPIPE_HIT_POINTS;
		obj->radius = BLOWPIPE_RADIUS;
		obj->intelligent = 1;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
		bones[obj->bone_index + 13 * 4] |= ROT_X;
	}

	if ((obj = &objects[MUTANT1])->loaded)
	{
		obj->initialise = InitialiseWingmute;
		obj->control = WingmuteControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = WINGMUTE_HIT_POINTS;
		obj->radius = WINGMUTE_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
	}

	if ((obj = &objects[VULTURE])->loaded)
	{
		obj->initialise = InitialiseVulture;
		obj->control = VultureControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = VULTURE_HIT_POINTS;
		obj->radius = VULTURE_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
	}

	if ((obj = &objects[CROW])->loaded)
	{
		obj->initialise = InitialiseVulture;
		obj->control = VultureControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = CROW_HIT_POINTS;
		obj->radius = CROW_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
	}

	if ((obj = &objects[OILRED])->loaded)
	{
		obj->control = OilRedControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = BANDIT1_HIT_POINTS;
		obj->radius = BANDIT1_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
		obj->bite_offset = OILRED_BITE;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
	}

	if ((obj = &objects[SECURITY_GUARD])->loaded)
	{
		obj->control = LondSecControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = BANDIT2_HIT_POINTS;
		obj->radius = BANDIT2_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
		obj->bite_offset = OILRED_BITE;	

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
	}

	if ((obj = &objects[ELECTRIC_CLEANER])->loaded)
	{
		obj->initialise = InitialiseCleaner;
		obj->control = CleanerControl;
		obj->collision = ObjectCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = DONT_TARGET;
		obj->radius = STEP_L * 2;
		obj->intelligent = 1;
		obj->non_lot = 1;
	}

	if ((obj = &objects[MP2])->loaded)
	{
		obj->control = MPGunControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = BANDIT2_HIT_POINTS;
		obj->radius = BANDIT2_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
		obj->bite_offset = OILRED_BITE;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
	}

	if ((obj = &objects[LON_MERCENARY1])->loaded)
	{
		obj->initialise = InitialiseSwat;
		obj->control = SwatControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = SWAT_HIT_POINTS;
		obj->radius = SWAT_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;
		obj->bite_offset = SWAT_GUN_BITE;

		bones[obj->bone_index + 0 * 4] |= ROT_Y;
		bones[obj->bone_index + 0 * 4] |= ROT_X;
		bones[obj->bone_index + 7 * 4] |= ROT_Y;
	}

	if ((obj = &objects[PUNK1])->loaded)
	{
		obj->initialise = InitialisePunk;
		obj->control = PunkControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = PUNK_HIT_POINTS;
		obj->radius = PUNK_RADIUS;
		obj->intelligent = 1;
		obj->pivot_length = 0;

		bones[obj->bone_index + 6 * 4] |= ROT_Y;
		bones[obj->bone_index + 6 * 4] |= ROT_X;
		bones[obj->bone_index + 13 * 4] |= ROT_Y;
	}

	if ((obj = &objects[CROCODILE])->loaded)
	{
		obj->control = CrocControl;
		obj->collision = CreatureCollision;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->hit_points = CROC_HIT_POINTS;
		obj->radius = CROC_RADIUS;
		obj->intelligent = 1;
		obj->water_creature = 1;
		obj->pivot_length = 200;

		bones[obj->bone_index + 7 * 4] |= ROT_Y;
	}

	if ((obj = &objects[WINSTON])->loaded)
	{
		obj->control = nullptr;
		obj->collision = nullptr;
		obj->hit_points = 0;
		obj->shadow_size = UNIT_SHADOW / 4;
		obj->radius = 0;
		obj->intelligent = 0;
	}

	if ((obj = &objects[ARMY_WINSTON])->loaded)
	{
		obj->control = nullptr;
		obj->collision = nullptr;
		obj->hit_points = 0;
		obj->shadow_size = UNIT_SHADOW / 4;
		obj->radius = 0;
		obj->intelligent = 0;
	}

	if ((obj = &objects[TARGETS])->loaded)
	{
		obj->control = TargetControl;
		obj->collision = ObjectCollision;
		obj->hit_points = 8;
		obj->shadow_size = UNIT_SHADOW / 2;
		obj->radius = WALL_L / 10;
		obj->intelligent = 1;
	}

	if ((obj = &objects[AI_GUARD])->loaded)
	{
		obj->draw_routine = DrawDummyItem;
		obj->collision = AIPickupCollision;
		obj->hit_points = AI_PICKUP_HIT_POINTS;
	}

	if ((obj = &objects[AI_AMBUSH])->loaded)
	{
		obj->draw_routine = DrawDummyItem;
		obj->collision = AIPickupCollision;
		obj->hit_points = AI_PICKUP_HIT_POINTS;
	}

	if ((obj = &objects[AI_PATROL1])->loaded)
	{
		obj->draw_routine = DrawDummyItem;
		obj->collision = AIPickupCollision;
		obj->hit_points = AI_PICKUP_HIT_POINTS;
	}

	if ((obj = &objects[AI_PATROL2])->loaded)
	{
		obj->draw_routine = DrawDummyItem;
		obj->collision = AIPickupCollision;
		obj->hit_points = AI_PICKUP_HIT_POINTS;
	}

	if ((obj = &objects[AI_FOLLOW])->loaded)
	{
		obj->draw_routine = DrawDummyItem;
		obj->collision = AIPickupCollision;
		obj->hit_points = AI_PICKUP_HIT_POINTS;
	}

	if ((obj = &objects[AI_X1])->loaded)
	{
		obj->draw_routine = DrawDummyItem;
		obj->collision = AIPickupCollision;
		obj->hit_points = AI_PICKUP_HIT_POINTS;
	}

	if ((obj = &objects[AI_X2])->loaded)
	{
		obj->draw_routine = DrawDummyItem;
		obj->collision = AIPickupCollision;
		obj->hit_points = AI_PICKUP_HIT_POINTS;
	}

	if ((obj = &objects[AI_X3])->loaded)
	{
		obj->draw_routine = DrawDummyItem;
		obj->collision = AIPickupCollision;
		obj->hit_points = AI_PICKUP_HIT_POINTS;
	}

	if ((obj = &objects[AI_MODIFY])->loaded)
	{
		obj->draw_routine = DrawDummyItem;
		obj->collision = AIPickupCollision;
		obj->hit_points = AI_PICKUP_HIT_POINTS;
	}
}

void TrapObjects()
{
	OBJECT_INFO* obj;

	obj = &objects[KILL_ALL_TRIGGERS];
	obj->hit_points = 0;
	obj->draw_routine = DrawDummyItem;
	obj->control = KillAllCurrentItems;

	obj = &objects[MINI_COPTER];
	obj->control = MiniCopterControl;

	obj = &objects[MOVING_BAR];
	obj->collision = ObjectCollision;
	obj->control = GeneralControl;
	obj->water_creature = 1;

	obj = &objects[DEATH_SLIDE];
	obj->initialise = InitialiseRollingBall;
	obj->collision = DeathSlideCollision;
	obj->control = ControlDeathSlide;

	obj = &objects[SPIKE_WALL];
	obj->control = ControlSpikeWall;
	obj->collision = ObjectCollision;

	obj = &objects[CEILING_SPIKES];
	obj->control = ControlCeilingSpikes;
	obj->collision = TrapCollision;

	obj = &objects[HOOK];
	obj->control = HookControl;
	obj->collision = CreatureCollision;

	obj = &objects[SAW];
	obj->control = PropellerControl;
	obj->collision = ObjectCollision;

	obj = &objects[FAN];
	obj->water_creature = 1;
	obj->control = PropellerControl;
	obj->collision = TrapCollision;

	obj = &objects[SMALL_FAN];
	obj->control = PropellerControl;
	obj->collision = TrapCollision;

	obj = &objects[SPINNING_BLADE];
	obj->initialise = InitialiseKillerStatue;
	obj->control = SpinningBlade;
	obj->collision = ObjectCollision;

	obj = &objects[ICICLES];
	obj->control = IcicleControl;
	obj->collision = TrapCollision;

	obj = &objects[BLADE];
	obj->initialise = InitialiseBlade;
	obj->control = BladeControl;
	obj->collision = TrapCollision;

	obj = &objects[SPRING_BOARD];
	obj->control = SpringBoardControl;

	obj = &objects[FALLING_BLOCK];
	obj->control = FallingBlock;
	obj->floor = FallingBlockFloor;
	obj->ceiling = FallingBlockCeiling;

	obj = &objects[FALLING_BLOCK2];
	obj->control = FallingBlock;
	obj->floor = FallingBlockFloor;
	obj->ceiling = FallingBlockCeiling;

	obj = &objects[FALLING_PLANK];
	obj->control = FallingBlock;
	obj->floor = FallingBlockFloor;
	obj->ceiling = FallingBlockCeiling;

	obj = &objects[PENDULUM];
	obj->shadow_size = UNIT_SHADOW / 2;
	obj->control = Pendulum;
	obj->collision = ObjectCollision;

	obj = &objects[SWING_BOX];
	obj->shadow_size = UNIT_SHADOW / 2;
	obj->control = Pendulum;
	obj->collision = ObjectCollision;

	obj = &objects[TEETH_TRAP];
	obj->collision = TrapCollision;
	obj->control = TeethTrap;

	obj = &objects[AVALANCHE];
	obj->collision = RollingBallCollision;
	obj->control = RollingBallControl;
	obj->initialise = InitialiseRollingBall;

	obj = &objects[OILDRUMS];
	obj->collision = RollingBallCollision;
	obj->control = RollingBallControl;
	obj->initialise = InitialiseRollingBall;

	obj = &objects[ROLLING_BALL];
	obj->collision = RollingBallCollision;
	obj->control = RollingBallControl;
	obj->initialise = InitialiseRollingBall;

	obj = &objects[BIG_ROLLING_BALL];
	obj->collision = RollingBallCollision;
	obj->control = RollingBallControl;
	obj->initialise = InitialiseRollingBall;

	obj = &objects[SPIKES];
	obj->control = SpikeControl;
	obj->collision = SpikeCollision;

	obj = &objects[FALLING_CEILING1];
	obj->control = FallingCeiling;
	obj->collision = TrapCollision;

	for (int i = MOVABLE_BLOCK; i <= MOVABLE_BLOCK4; ++i)
	{
		INIT_MOVBLOCK(i);
	}

	objects[DART_EMITTER].control = DartEmitterControl;
	objects[DART_EMITTER].draw_routine = DrawDummyItem;

	objects[HOMING_DART_EMITTER].control = DartEmitterControl;
	objects[HOMING_DART_EMITTER].draw_routine = DrawDummyItem;

	objects[RAPTOR_EMITTER].initialise = InitialiseRaptorEmitter;
	objects[RAPTOR_EMITTER].control = RaptorEmitterControl;
	objects[RAPTOR_EMITTER].draw_routine = DrawDummyItem;

	objects[FLYING_MUTANT_EMITTER].control = FlyEmitterControl;
	objects[FLYING_MUTANT_EMITTER].draw_routine = DrawDummyItem;
	objects[FLYING_MUTANT_EMITTER].hit_points = WASP_FACTORY_HIT_POINTS;

	objects[FLAME].control = FlameControl;
	objects[FLAME].draw_routine = DrawDummyItem;

	obj = &objects[BOO_MUTANT];
	obj->control = BoomuteControl;

	obj = &objects[SPECIAL_FX1];
	obj->control = ControlArea51Rocket;

	obj = &objects[SPECIAL_FX2];
	obj->control = ControlArea51Rocket;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[SPECIAL_FX3];
	obj->control = ControlArea51Struts;
	obj->initialise = InitialiseArea51Struts;

	obj = &objects[AREA51_LASER];
	obj->control = ControlArea51Laser;
	obj->initialise = InitialiseArea51Laser;

	obj = &objects[DARTS];
	obj->control = DartsControl;
	obj->draw_routine = S_DrawDarts;
	obj->collision = ObjectCollision;
	obj->shadow_size = UNIT_SHADOW / 2;

	obj = &objects[FLAME_EMITTER];
	obj->control = FlameEmitterControl;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[FLAME_EMITTER2];
	obj->control = FlameEmitter2Control;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[FLAME_EMITTER3];
	obj->control = FlameEmitter3Control;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[SIDE_FLAME_EMITTER];
	obj->control = SideFlameEmitterControl;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[POLEROPE];
	obj->collision = PoleCollision;
}

void ObjectObjects()
{
	OBJECT_INFO* obj;

	objects[CAMERA_TARGET].draw_routine = DrawDummyItem;

	obj = &objects[FIREHEAD];
	obj->control = ControlFireHead;
	obj->collision = ObjectCollision;
	obj->initialise = InitialiseFireHead;

	obj = &objects[TONYFIREBALL];
	obj->control = ControlTonyFireBall;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[EXTRAFX1];
	obj->control = ControlClawmutePlasmaBall;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[EXTRAFX2];
	obj->control = ControlWillbossPlasmaBall;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[EXTRAFX3];
	obj->control = ControlRotateyThing;

	obj = &objects[EXTRAFX4];
	obj->control = ControlLaserBolts;
	//obj->draw_routine = S_DrawLaserBolts;

	obj = &objects[EXTRAFX5];
	obj->control = ControlLondBossPlasmaBall;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[EXTRAFX6];
	obj->control = ControlFusebox;
	obj->collision = ObjectCollision;

	obj = &objects[ROCKET];
	obj->control = ControlRocket;

	obj = &objects[GRENADE];
	obj->control = ControlGrenade;

	obj = &objects[HARPOON_BOLT];
	obj->control = ControlHarpoonBolt;

	obj = &objects[KNIFE];
	obj->control = ControlMissile;

	obj = &objects[DIVER_HARPOON];
	obj->control = ControlMissile;

	obj = &objects[KAYAK];
	obj->initialise = KayakInitialise;
	obj->draw_routine = KayakDraw;
	obj->collision = KayakCollision;

	obj = &objects[BOAT];
	obj->initialise = InitialiseBoat;
	obj->control = BoatControl;
	obj->collision = BoatCollision;
	obj->draw_routine = DrawBoat;

	bones[obj->bone_index + (1 * 4)] |= ROT_Z;

	obj = &objects[QUADBIKE];
	obj->initialise = InitialiseQuadBike;
	obj->collision = QuadBikeCollision;
	obj->draw_routine = QuadBikeDraw;

	obj = &objects[MINECART];
	obj->initialise = MineCartInitialise;
	obj->collision = MineCartCollision;

	obj = &objects[BIGGUN];
	obj->initialise = BigGunInitialise;
	obj->collision = BigGunCollision;
	obj->draw_routine = BigGunDraw;

	obj = &objects[UPV];
	obj->initialise = SubInitialise;
	obj->control = SubEffects;
	obj->collision = SubCollision;
	obj->draw_routine = SubDraw;

	obj = &objects[TRAIN];
	obj->control = TrainControl;
	obj->collision = TrainCollision;

	obj = &objects[FLARE_ITEM];
	obj->control = FlareControl;
	obj->collision = PickUpCollision;
	obj->draw_routine = DrawFlareInAir;
	obj->pivot_length = 256;
	obj->hit_points = 256;

	obj = &objects[SMASH_WINDOW];
	obj->initialise = InitialiseWindow;
	obj->control = WindowControl;
	obj->collision = ObjectCollision;

	obj = &objects[SMASH_OBJECT1];
	obj->initialise = InitialiseWindow;
	obj->control = WindowControl;
	obj->collision = ObjectCollision;

	obj = &objects[SMASH_OBJECT2];
	obj->initialise = InitialiseWindow;
	obj->control = WindowControl;
	obj->collision = ObjectCollision;

	obj = &objects[SMASH_OBJECT3];
	obj->initialise = InitialiseWindow;
	obj->control = WindowControl;
	obj->collision = ObjectCollision;

	obj = &objects[CARCASS];
	obj->control = CarcassControl;
	obj->collision = ObjectCollision;

	obj = &objects[LIFT];
	obj->initialise = InitialiseLift;
	obj->control = LiftControl;
	obj->floor = LiftFloor;
	obj->ceiling = LiftCeiling;

	obj = &objects[TROPICAL_FISH];
	obj->draw_routine = S_DrawFish;
	obj->control = ControlFish;
	obj->hit_points = -1;

	obj = &objects[PIRAHNAS];
	obj->draw_routine = S_DrawFish;
	obj->control = ControlFish;
	obj->hit_points = -1;

	obj = &objects[BAT_EMITTER];
	obj->control = BatEmitterControl;
	obj->draw_routine = DrawDummyItem;

	obj = &objects[LIGHTNING_EMITTER2];
	obj->control = ControlElectricFence;
	obj->draw_routine = DrawDummyItem;

	objects[STROBE_LIGHT].control = ControlStrobeLight;

	objects[PULSE_LIGHT].control = ControlPulseLight;
	objects[PULSE_LIGHT].draw_routine = DrawDummyItem;

	objects[ON_OFF_LIGHT].control = ControlOnOffLight;
	objects[ON_OFF_LIGHT].draw_routine = DrawDummyItem;

	objects[ELECTRICAL_LIGHT].control = ControlElectricalLight;
	objects[ELECTRICAL_LIGHT].draw_routine = DrawDummyItem;

	objects[EXTRA_LIGHT1].control = ControlBeaconLight;
	objects[EXTRA_LIGHT1].draw_routine = DrawDummyItem;

	obj = &objects[BRIDGE_FLAT];
	obj->floor = BridgeFlatFloor;
	obj->ceiling = BridgeFlatCeiling;

	obj = &objects[BRIDGE_TILT1];
	obj->floor = BridgeTilt1Floor;
	obj->ceiling = BridgeTilt1Ceiling;

	obj = &objects[BRIDGE_TILT2];
	obj->floor = BridgeTilt2Floor;
	obj->ceiling = BridgeTilt2Ceiling;

	if ((obj = &objects[DRAW_BRIDGE])->loaded)
	{
		obj->control = GeneralControl;
		obj->collision = DrawBridgeCollision;
		obj->floor = DrawBridgeFloor;
		obj->ceiling = DrawBridgeCeiling;
	}

	obj = &objects[SMALL_SWITCH];
	obj->control = SwitchControl;
	obj->collision = SwitchCollision;

	obj = &objects[PUSH_SWITCH];
	obj->control = SwitchControl;
	obj->collision = SwitchCollision;

	obj = &objects[AIRLOCK_SWITCH];
	obj->control = SwitchControl;
	obj->collision = SwitchCollision;

	obj = &objects[SWITCH_TYPE1];
	obj->control = SwitchControl;
	obj->collision = SwitchCollision;

	obj = &objects[SWITCH_TYPE2];
	obj->control = SwitchControl;
	obj->collision = SwitchCollision2;

	// normal doors

	for (int i = DOOR_TYPE1; i <= DOOR_TYPE8; ++i)
	{
		INIT_DOOR(i);
	}

	// dors with no box block

	for (int i = DOOR_TYPE9; i <= DOOR_TYPE9; ++i)
	{
		INIT_DOOR(i);
	}

	obj = &objects[TRAPDOOR];
	obj->control = TrapDoorControl;
	obj->floor = TrapDoorFloor;
	obj->ceiling = TrapDoorCeiling;

	obj = &objects[TRAPDOOR2];
	obj->control = TrapDoorControl;
	obj->floor = TrapDoorFloor;
	obj->ceiling = TrapDoorCeiling;

	INIT_PICKUP(PICKUP_ITEM1);
	INIT_PICKUP(PICKUP_ITEM2);
	INIT_PICKUP(KEY_ITEM1);
	INIT_PICKUP(KEY_ITEM2);
	INIT_PICKUP(KEY_ITEM3);
	INIT_PICKUP(KEY_ITEM4);
	INIT_PICKUP(PUZZLE_ITEM1);
	INIT_PICKUP(PUZZLE_ITEM2);
	INIT_PICKUP(PUZZLE_ITEM3);
	INIT_PICKUP(PUZZLE_ITEM4);
	INIT_PICKUP(GUN_ITEM);
	INIT_PICKUP(SHOTGUN_ITEM);
	INIT_PICKUP(HARPOON_ITEM);
	INIT_PICKUP(ROCKET_GUN_ITEM);
	INIT_PICKUP(GRENADE_GUN_ITEM);
	INIT_PICKUP(M16_ITEM);
	INIT_PICKUP(MAGNUM_ITEM);
	INIT_PICKUP(UZI_ITEM);
	INIT_PICKUP(FLAREBOX_ITEM);
	INIT_PICKUP(GUN_AMMO_ITEM);
	INIT_PICKUP(SG_AMMO_ITEM);
	INIT_PICKUP(MAG_AMMO_ITEM);
	INIT_PICKUP(UZI_AMMO_ITEM);
	INIT_PICKUP(HARPOON_AMMO_ITEM);
	INIT_PICKUP(M16_AMMO_ITEM);
	INIT_PICKUP(ROCKET_AMMO_ITEM);
	INIT_PICKUP(GRENADE_AMMO_ITEM);
	INIT_PICKUP(MEDI_ITEM);
	INIT_PICKUP(BIGMEDI_ITEM);
	INIT_PICKUP_ANIM(ICON_PICKUP1_ITEM);
	INIT_PICKUP_ANIM(ICON_PICKUP2_ITEM);
	INIT_PICKUP_ANIM(ICON_PICKUP3_ITEM);
	INIT_PICKUP_ANIM(ICON_PICKUP4_ITEM);
	INIT_PICKUP_ANIM(SAVEGAME_CRYSTAL_ITEM);
	INIT_KEYHOLE(KEY_HOLE1);
	INIT_KEYHOLE(KEY_HOLE2);
	INIT_KEYHOLE(KEY_HOLE3);
	INIT_KEYHOLE(KEY_HOLE4);

	obj = &objects[DETONATOR];
	obj->collision = DetonatorCollision;
	obj->control = DetonatorControl;

	INIT_PUZZLEHOLE(PUZZLE_HOLE1);
	INIT_PUZZLEHOLE(PUZZLE_HOLE2);
	INIT_PUZZLEHOLE(PUZZLE_HOLE3);
	INIT_PUZZLEHOLE(PUZZLE_HOLE4);

	for (int i = PUZZLE_DONE1; i <= PUZZLE_DONE4; ++i)
	{
		obj = &objects[i];
		obj->control = ControlAnimating_1_4;
	}

	/*-------------------Effects------------------------- */

	for (int i = ANIMATING1; i <= ANIMATING6; ++i)
	{
		obj = &objects[i];
		obj->control = ControlAnimating_1_4;
		obj->collision = ObjectCollision;
	}

	for (int i = SMOKE_EMITTER_WHITE; i <= STEAM_EMITTER; ++i)
	{
		obj = &objects[i];
		obj->control = ControlSmokeEmitter;
		obj->draw_routine = DrawDummyItem;
	}

	objects[GHOST_GAS_EMITTER].control = ControlGhostGasEmitter;
	objects[GHOST_GAS_EMITTER].draw_routine = DrawDummyItem;

	for (int i = RED_LIGHT; i <= WHITE_LIGHT; ++i)
	{
		obj = &objects[i];
		obj->control = ControlColouredLights;
		obj->draw_routine = DrawDummyItem;
	}

	objects[BUBBLES1].control = ControlBubble1;
	objects[BUBBLES1].draw_routine = DrawDummyItem;
	objects[DRAGON_FIRE].control = ControlFlameThrower;
	objects[DRAGON_FIRE].draw_routine = DrawDummyItem;

	objects[WATERFALL].control = WaterFall;
	objects[WATERFALL].draw_routine = DrawDummyItem;

	for (int i = SECURITY_LASER_ALARM; i <= SECURITY_LASER_KILLER; ++i)
	{
		obj = &objects[i];
		obj->initialise = LaserControl;
		obj->control = LaserControl;
		obj->draw_routine = S_DrawLaser;
	}

	objects[BODY_PART].control = ControlBodyPart;
	objects[BODY_PART].nmeshes = 0;
	objects[BODY_PART].loaded = 1;

	objects[BIRD_TWEETER].control = ControlBirdTweeter;
	objects[BIRD_TWEETER].draw_routine = DrawDummyItem;

	objects[WATER_DRIP].control = ControlBirdTweeter;
	objects[WATER_DRIP].draw_routine = DrawDummyItem;

	objects[DING_DONG].control = ControlDingDong;
	objects[DING_DONG].draw_routine = DrawDummyItem;

	objects[LARA_ALARM].control = ControlLaraAlarm;
	objects[LARA_ALARM].draw_routine = DrawDummyItem;

	objects[CLOCK_CHIMES].control = ControlClockChimes;
	objects[CLOCK_CHIMES].draw_routine = DrawDummyItem;

	objects[FINAL_LEVEL].draw_routine = DrawDummyItem;
	objects[FINAL_LEVEL].control = nullptr;	// ?

	objects[CUT_SHOTGUN].control = ControlCutShotgun;

	objects[EARTHQUAKE].draw_routine = DrawDummyItem;
	objects[EARTHQUAKE].control = EarthQuake;

	objects[GUNSHELL].control = ControlGunShell;
	objects[SHOTGUNSHELL].control = ControlGunShell;
}

bool InitialiseLevel(int level_number)
{
#ifdef GAMEDEBUG
	prof::print(YELLOW, "Initialising Level: {} '{}'", level_number, GF_Level_Names[level_number]);
#endif

	InitialiseGameFlags();

	if (!load_level())
		return 0;

	if (lara.item_number != NO_ITEM)
		InitialiseLara();

	GetCarriedItems();
	GetAIPickups();

	effects = (FX_INFO*)game_malloc(NUM_EFFECTS * sizeof(FX_INFO), EFFECTS_ARRAY);

	InitialiseFXArray();
	InitialiseLOTarray();
	InitColours();
	T_InitPrint();
	InitialisePickUpDisplay();
	S_InitialiseScreen();

	camera.underwater = 0;

	health_bar_timer = 100;

	g_audio->stop_all();

	std::vector<std::pair<int, int16_t>> synced_level_entities;

	for (int i = 0; i < level_items; ++i)
		if (auto subtype = g_level->get_level_entity_subtype_from_obj_id(items[i].object_number); subtype != ENTITY_LEVEL_TYPE_NONE)
			synced_level_entities.emplace_back(subtype, int16_t(i));

	// maybe sync with players should be the first so we can send players
	// to the client from the server?

	g_client->join(true);
	g_client->sync_with_players();
	g_client->sync_level_entities(synced_level_entities);
	g_client->sync_spawned_entities();

	fix_skybox = true;

	g_client->set_level_loaded(true);

	return 1;
}

void InitialiseLevelFlags()
{
	compy_scared_timer = compys_attack_lara = 0;
	carcass_item = -1;
}

void InitialiseObjects()
{
	for (int i = 0; i < NUMBER_OBJECTS; ++i)
	{
		auto obj = &objects[i];

		obj->initialise = nullptr;
		obj->collision = nullptr;
		obj->control = nullptr;
		obj->draw_routine = (i == WINSTON || i == ARMY_WINSTON ? DrawDummyItem : DrawAnimatingItem);
		obj->floor = obj->ceiling = nullptr;
		obj->pivot_length = 0;
		obj->radius = DEFAULT_RADIUS;
		obj->shadow_size = 0;
		obj->hit_points = DONT_TARGET;
		obj->intelligent = obj->water_creature = 0;
	}

	BaddyObjects();
	TrapObjects();
	ObjectObjects();

	InitialiseHair((vec3d*)g_hair, (int_vec3*)g_hair_vel);
	InitialiseSparks();
}

void GetCarriedItems()
{
	ITEM_INFO* item = items;

	for (int i = 0; i < level_items; ++i, ++item)
	{
		if (objects[item->object_number].intelligent)
		{
			item->carried_item = NO_ITEM;

			auto pickup_number = room[item->room_number].item_number;

			do {
				auto pickup = &items[pickup_number];

				if (pickup->pos.x_pos == item->pos.x_pos &&
					pickup->pos.y_pos == item->pos.y_pos &&
					pickup->pos.z_pos == item->pos.z_pos &&
					objects[pickup->object_number].collision == PickUpCollision)
				{
					pickup->carried_item = item->carried_item;
					item->carried_item = pickup_number;

					RemoveDrawnItem(pickup_number);

					pickup->room_number = NO_ROOM;
				}

				pickup_number = pickup->next_item;
			} while (pickup_number != NO_ITEM);
		}
	}
}

void GetAIPickups()
{
	ITEM_INFO* item = items;

	for (int i = 0; i < level_items; ++i, ++item)
	{
		if (objects[item->object_number].intelligent)
		{
			item->ai_bits = 0;

			auto pickup_number = room[item->room_number].item_number;

			do {
				auto pickup = &items[pickup_number];

				if (pickup->pos.x_pos == item->pos.x_pos &&
					pickup->pos.z_pos == item->pos.z_pos &&
					objects[pickup->object_number].collision == AIPickupCollision &&
					pickup->object_number < AI_PATROL2)
				{
					item->ai_bits |= 1 << (pickup->object_number - AI_GUARD);
					item->item_flags[3] = pickup->pos.y_rot;

					if (!(pickup->object_number == AI_PATROL1 && enable_killable_ai_patrol))
					{
						KillItem(pickup_number);

						pickup->room_number = NO_ROOM;
					}
				}
				else if (pickup->object_number == FLAME_EMITTER && item->object_number == PUNK1)
				{
					item->item_flags[2] = 3;

					KillItem(pickup_number);

					pickup->room_number = NO_ROOM;
				}

				pickup_number = pickup->next_item;
			} while (pickup_number != NO_ITEM);
		}
	}
}

void BuildOutsideTable()
{
	memset((OutsideRoomTable = (uint8_t*)game_malloc(27 * 27 * OUTSIDE_Z, 0)), 0xff, 27 * 27 * OUTSIDE_Z);

	prof::print(YELLOW, "X %d, Y %d, Z %d, Xs %d, Ys %d\n", room[0].x, room[0].y, room[0].z, room[0].x_size, room[0].y_size);

	int max_slots = 0;

	for (int y = 0; y < 108; y += 4)
	{
		for (int x = 0; x < 108; x += 4)
		{
			for (int i = 0; i < number_rooms; ++i)
			{
				uint8_t* d;
				int rx, ry, j = 0, xl, yl;
				ROOM_INFO* r = &room[i];

				rx = (r->z >> 10) + 1;
				ry = (r->x >> 10) + 1;

				for (yl = 0; yl < 4; ++yl)
				{
					for (xl = 0; xl < 4; ++xl)
					{
						if ((x + xl) >= rx && (x + xl) < (rx + r->x_size - 2) &&
							(y + yl) >= ry && (y + yl) < (ry + r->y_size - 2))
						{
							j = 1;
							break;
						}
					}
				}

				if (!j)
					continue;

#if defined(GAMEDEBUG)
				if (i == 255)
				{
					prof::print(RED, "ERROR : Room 255 fuckeroony - go tell Chris\n");
					exit(0);
				}
#endif
				d = &OutsideRoomTable[((x >> 2) * OUTSIDE_Z) + ((y >> 2) * (27 * OUTSIDE_Z))];

				j = 0;

				for (; j < OUTSIDE_Z; ++j)
				{
					if (d[j] == 0xff)
					{
						d[j] = i;

						if (j > max_slots)
							max_slots = j;

						break;
					}
				}

#if defined(GAMEDEBUG)
				if (j == OUTSIDE_Z)
				{
					prof::print(RED, "ERROR : Buffer shittage - go tell Chris\n");
					exit(0);
					break;
				}
#endif
			}
		}
	}

	auto s = OutsideRoomTable;

	for (int y = 0; y < 27; ++y)
	{
		for (int x = 0; x < 27; ++x)
		{
			int z = 0,
				i = (y * 27) + x;

			auto d = &OutsideRoomTable[(x * OUTSIDE_Z) + (y * (27 * OUTSIDE_Z))];

			while (d[z] != 0xff)
				++z;

			if (z == 0)		 OutsideRoomOffsets[i] = (short)0xffff;
			else if (z == 1) OutsideRoomOffsets[i] = *d | 0x8000;
			else
			{
				auto p = OutsideRoomTable;

				while (p < s)
				{
					if (memcmp(p, d, z) == 0)
					{
						OutsideRoomOffsets[i] = (long)p - (long)OutsideRoomTable;
						break;
					}
					else
					{
						int z2 = 0;

						while (p[z2] != 0xff)
							++z2;

						p += z2 + 1;
					}
				}

				if (p >= s)
				{
					OutsideRoomOffsets[i] = (long)s - (long)OutsideRoomTable;

					for (; z; --z)
						*s++ = *d++;

					*s++ = 0xff;
				}
			}
		}
	}

#if defined(GAMEDEBUG)
	prof::print(GREEN, "Ouside room table = {} bytes, max_slots = {}\n", (long)s - (long)OutsideRoomTable, max_slots);
#endif
}