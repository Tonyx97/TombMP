#include <specific/standard.h>
#include <specific/input.h>

#include "lara.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "laramisc.h"
#include "laraswim.h"

#define SURF_MAXSPEED			(15 * 4)
#define SURF_RADIUS				100
#define SURF_HITE				700
#define MAX_HEAD_ROTATION_SURF 	(50 * ONE_DEGREE)
#define MAX_HEAD_TILT_SURF	  	(40 * ONE_DEGREE)
#define MIN_HEAD_TILT_SURF 	  	(-40 * ONE_DEGREE)
#define HEAD_TURN_SURF 		  	(3 * ONE_DEGREE)
#define UW_WALLDEFLECT			(2 * ONE_DEGREE)
#define SLOPE_DIF				60
#define HANG_ANGLE				(35 * ONE_DEGREE)

int LaraTestWaterStepOut(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->coll_type == COLL_FRONT || coll->mid_type == BIG_SLOPE || coll->mid_type == DIAGONAL || coll->mid_floor >= 0)
		return 0;

	if (coll->mid_floor < -(STEP_L / 2))
	{
		item->anim_number = SURF2WADE1_A;
		item->frame_number = SURF2WADE1_F;
		item->current_anim_state = AS_WATEROUT;
		item->goal_anim_state = AS_STOP;
	}
	else
	{
		if (item->goal_anim_state == AS_SURFLEFT)
			item->goal_anim_state = AS_STEPLEFT;
		else if (item->goal_anim_state == AS_SURFRIGHT)
			item->goal_anim_state = AS_STEPRIGHT;
		else
		{
			item->goal_anim_state = AS_WADE;
			item->current_anim_state = AS_WADE;
			item->anim_number = WADE_A;
			item->frame_number = WADE_F;
		}
	}

	item->pos.y_pos += (coll->front_floor + SURF_HITE - 5);

	UpdateLaraRoom(item, -LARA_HITE / 2);

	item->pos.x_rot = item->pos.z_rot = 0;
	item->gravity_status = 0;
	item->speed = 0;
	item->fallspeed = 0;

	lara.water_status = LARA_WADE;

	return 1;
}

int LaraTestWaterClimbOut(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->coll_type != COLL_FRONT || !(input & IN_ACTION) || ABS(coll->left_floor2 - coll->right_floor2) >= SLOPE_DIF)
		return 0;

	if (lara.gun_status != LG_ARMLESS && !(lara.gun_status == LG_READY && lara.gun_type == LG_FLARE))
		return 0;

	if (coll->front_ceiling > 0 || coll->mid_ceiling > -STEPUP_HEIGHT)
		return 0;

	int hdif = coll->front_floor + SURF_HITE;
	if (hdif <= -(STEP_L * 2) || hdif > SURF_HITE - STEPUP_HEIGHT)
		return 0;

	PHD_ANGLE angle = item->pos.y_rot;

	if (angle >= 0 - HANG_ANGLE && angle <= 0 + HANG_ANGLE)				   angle = 0;
	else if (angle >= 16384 - HANG_ANGLE && angle <= 16384 + HANG_ANGLE)   angle = 16384;
	else if (angle >= 32767 - HANG_ANGLE || angle <= -32767 + HANG_ANGLE)  angle = -32768;
	else if (angle >= -16384 - HANG_ANGLE && angle <= -16384 + HANG_ANGLE) angle = -16384;

	if (angle & 0x3fff)
		return 0;

	item->pos.y_pos += (hdif - 5);

	UpdateLaraRoom(item, -LARA_HITE / 2);

	switch (angle)
	{
	case 0:		  item->pos.z_pos = (item->pos.z_pos & -WALL_L) + WALL_L + LARA_RAD; break;
	case 0x4000:  item->pos.x_pos = (item->pos.x_pos & -WALL_L) + WALL_L + LARA_RAD; break;
	case -0x8000: item->pos.z_pos = (item->pos.z_pos & -WALL_L) - LARA_RAD;			 break;
	case -0x4000: item->pos.x_pos = (item->pos.x_pos & -WALL_L) - LARA_RAD;			 break;
	}

	if (hdif < -128)
	{
		item->anim_number = SURFCLIMB_A;
		item->frame_number = SURFCLIMB_F;
	}
	else if (hdif < 128)
	{
		item->anim_number = SURF2STND_A;
		item->frame_number = SURF2STND_F;
	}
	else
	{
		item->anim_number = SURF2QSTND_A;
		item->frame_number = SURF2QSTND_F;
	}

	item->current_anim_state = AS_WATEROUT;
	item->goal_anim_state = AS_STOP;
	item->pos.y_rot = angle;
	item->pos.x_rot = item->pos.z_rot = 0;
	item->gravity_status = 0;
	item->speed = 0;
	item->fallspeed = 0;

	lara.gun_status = LG_HANDSBUSY;
	lara.water_status = LARA_ABOVEWATER;

	return 1;
}

void LaraSurfaceCollision(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->facing = lara.move_angle;

	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos + SURF_HITE, item->pos.z_pos, item->room_number, SURF_HITE + 100);
	ShiftItem(item, coll);

	if (coll->coll_type & (COLL_FRONT | COLL_TOP | COLL_CLAMP | COLL_TOPFRONT) || (coll->mid_floor < 0 && (coll->mid_type == BIG_SLOPE || coll->mid_type == DIAGONAL)))
	{
		item->fallspeed = 0;
		item->pos.x_pos = coll->old.x;
		item->pos.y_pos = coll->old.y;
		item->pos.z_pos = coll->old.z;
	}
	else if (coll->coll_type == COLL_LEFT)
		item->pos.y_rot += 5 * ONE_DEGREE;
	else if (coll->coll_type == COLL_RIGHT)
		item->pos.y_rot -= 5 * ONE_DEGREE;

	if (int wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
		wh - item->pos.y_pos <= -100)
	{
		item->goal_anim_state = AS_SWIM;
		item->current_anim_state = AS_DIVE;
		item->anim_number = SURFDIVE_A;
		item->frame_number = SURFDIVE_F;
		item->pos.x_rot = -45 * ONE_DEGREE;
		item->fallspeed = 20 * 4;

		lara.water_status = LARA_UNDERWATER;
	}
	else LaraTestWaterStepOut(item, coll);
}

void LaraSurface(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_elevation = -22 * ONE_DEGREE;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEP_L / 2;
	coll->bad_ceiling = 100;
	coll->old.x = item->pos.x_pos;
	coll->old.y = item->pos.y_pos;
	coll->old.z = item->pos.z_pos;
	coll->radius = SURF_RADIUS;
	coll->trigger = NULL;
	coll->slopes_are_walls = 0;
	coll->slopes_are_pits = 0;
	coll->lava_is_pit = 0;
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	if ((input & IN_LOOK) && lara.look)
		LookLeftRight();
	else ResetLook();

	lara.look = 1;

	reinterpret_cast<void(__cdecl*)(ITEM_INFO*, COLL_INFO*)>(*lara_control_routines[item->current_anim_state])(item, coll);

	if (item->pos.z_rot >= -(2 * LARA_LEAN_UNDO) && item->pos.z_rot <= 2 * LARA_LEAN_UNDO)
		item->pos.z_rot = 0;
	else if (item->pos.z_rot < 0)
		item->pos.z_rot += 2 * LARA_LEAN_UNDO;
	else item->pos.z_rot -= 2 * LARA_LEAN_UNDO;

	if (lara.current_active && lara.water_status != LARA_CHEAT)
		LaraWaterCurrent(coll);

	AnimateLara(item);

	item->pos.x_pos += ((phd_sin(lara.move_angle) * item->fallspeed) >> (W2V_SHIFT + 2));
	item->pos.z_pos += ((phd_cos(lara.move_angle) * item->fallspeed) >> (W2V_SHIFT + 2));

	LaraBaddieCollision(item, coll);

	if (lara.skidoo == NO_ITEM)
		reinterpret_cast<void(__cdecl*)(ITEM_INFO*, COLL_INFO*)>(*lara_collision_routines[item->current_anim_state])(item, coll);

	UpdateLaraRoom(item, 100);
	LaraGun();
	TestTriggers(coll->trigger, 0);
}

void lara_as_surfswim(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	lara.dive_count = 0;

	if (input & IN_LEFT)
		item->pos.y_rot -= LARA_SLOW_TURN;
	else if (input & IN_RIGHT)
		item->pos.y_rot += LARA_SLOW_TURN;
	if (!(input & IN_FORWARD))
		item->goal_anim_state = AS_SURFTREAD;
	if (input & IN_JUMP)
		item->goal_anim_state = AS_SURFTREAD;

	item->fallspeed += 2 * 4;

	if (item->fallspeed > SURF_MAXSPEED)
		item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surfback(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	lara.dive_count = 0;

	if (input & IN_LEFT)
		item->pos.y_rot -= LARA_SLOW_TURN / 2;
	else if (input & IN_RIGHT)
		item->pos.y_rot += LARA_SLOW_TURN / 2;
	if (!(input & IN_BACK))
		item->goal_anim_state = AS_SURFTREAD;

	item->fallspeed += 2 * 4;

	if (item->fallspeed > SURF_MAXSPEED)
		item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surfleft(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	lara.dive_count = 0;

	if (input & IN_LEFT)
		item->pos.y_rot -= LARA_SLOW_TURN / 2;
	else if (input & IN_RIGHT)
		item->pos.y_rot += LARA_SLOW_TURN / 2;
	if (!(input & IN_STEPL))
		item->goal_anim_state = AS_SURFTREAD;

	item->fallspeed += 2 * 4;

	if (item->fallspeed > SURF_MAXSPEED)
		item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surfright(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	lara.dive_count = 0;

	if (input & IN_LEFT)
		item->pos.y_rot -= LARA_SLOW_TURN / 2;
	else if (input & IN_RIGHT)
		item->pos.y_rot += LARA_SLOW_TURN / 2;
	if (!(input & IN_STEPR))
		item->goal_anim_state = AS_SURFTREAD;

	item->fallspeed += 2 * 4;

	if (item->fallspeed > SURF_MAXSPEED)
		item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surftread(ITEM_INFO* item, COLL_INFO* coll)
{
	item->fallspeed -= 4;

	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	if (input & IN_LOOK)
	{
		LookUpDown();
		return;
	}

	if (input & IN_LEFT)
		item->pos.y_rot -= LARA_SLOW_TURN;
	else if (input & IN_RIGHT)
		item->pos.y_rot += LARA_SLOW_TURN;
	if (input & IN_FORWARD)
		item->goal_anim_state = AS_SURFSWIM;
	else if (input & IN_BACK)
		item->goal_anim_state = AS_SURFBACK;
	if (input & IN_STEPL)
		item->goal_anim_state = AS_SURFLEFT;
	else if (input & IN_STEPR)
		item->goal_anim_state = AS_SURFRIGHT;

	if (input & IN_JUMP)
	{
		if (++lara.dive_count == DIVE_COUNT)
			item->goal_anim_state = AS_SWIM;
	}
	else lara.dive_count = 0;
}

void lara_col_surfswim(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->bad_neg = -STEPUP_HEIGHT;
	lara.move_angle = item->pos.y_rot;
	LaraSurfaceCollision(item, coll);
	LaraTestWaterClimbOut(item, coll);
}

void lara_col_surfback(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot - 32768;
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfleft(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot - 16384;
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfright(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot + 16384;
	LaraSurfaceCollision(item, coll);
}

void lara_col_surftread(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->goal_anim_state == AS_SWIM)
	{
		item->current_anim_state = AS_DIVE;
		item->anim_number = SURFDIVE_A;
		item->frame_number = SURFDIVE_F;
		item->pos.x_rot = -45 * ONE_DEGREE;
		item->fallspeed = 20 * 4;
		lara.water_status = LARA_UNDERWATER;
	}

	lara.move_angle = item->pos.y_rot;

	LaraSurfaceCollision(item, coll);
}