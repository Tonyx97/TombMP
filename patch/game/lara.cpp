#include <specific/standard.h>
#include <specific/input.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "laraclimb.h"
#include "camera.h"
#include "control.h"
#include "invfunc.h"
#include "inventry.h"
#include "laramisc.h"
#include "larafunc.h"
#include "effect2.h"
#include "sphere.h"
#include "quadbike.h"
#include "minecart.h"
#include "kayak.h"
#include "sub.h"
#include "biggun.h"
#include "physics.h"
#include "game.h"
#include "lara2gun.h"

#define CAM_A_HANG 				0
#define CAM_E_HANG 				-60 * ONE_DEGREE
#define HANG_HEIGHT				600
#define HANG_ANGLE				(35 * ONE_DEGREE)
#define HANG_RAD				(LARA_RAD + 16)
#define DAMAGE_START			140
#define DAMAGE_LENGTH			14
#define MAX_HEAD_ROTATION		(44 * ONE_DEGREE)
#define MAX_HEAD_TILT			(30 * ONE_DEGREE)
#define MIN_HEAD_TILT			(-35 * ONE_DEGREE)
#define HEAD_TURN				(2 * ONE_DEGREE)
#define SLOPE_DIF				60
#define BAD_JUMP_CEILING		((STEP_L * 3) / 4)
#define LARA_DEF_ADD_EDGE		(5 * ONE_DEGREE)
#define SWAMP_GRAVITY			2
#define VAULT_ANGLE				(30 * ONE_DEGREE)
#define LARA_DUCK_HEIGHT		400
#define LARA_DUCK_DEFLECT		(2 * ONE_DEGREE)
#define LARA_CRAWLB_RAD			250
#define LARA_DASH_LEAN_RATE		((ONE_DEGREE / 2) + LARA_LEAN_UNDO)
#define LARA_DASH_LEAN_MAX		((ONE_DEGREE * 15) + LARA_LEAN_UNDO)
#define LARA_DASH_TURN_RATE		((ONE_DEGREE / 4) + LARA_TURN_UNDO)
#define LARA_DASH_TURN_MAX		((ONE_DEGREE * 2) + LARA_TURN_UNDO)
#define LARA_MONKEY_TURN_RATE	((ONE_DEGREE / 4) + LARA_TURN_UNDO)
#define LARA_MONKEY_TURN_MAX	((ONE_DEGREE * 2) + LARA_TURN_UNDO)
#define DEATH_SPEED				100
#define DEATH_ACC				5
#define FASTFALL_FRICTION		item->speed = (item->speed * 95) / 100
#define HANG_HEIGHT				600

// TR IV/V additions

#define LARA_POLE_ROT			256
#define LARA_POLE_ACC			0x100
#define LARA_POLE_VEL			0x4000
#define LARA_POLE_DEC			-0x400

enum death_anims
{
	DEATH_EMPTY,
	DEATH_GRAB,
	DEATH_HANG
};

int16_t LeftClimbTab[4] = { 0x200, 0x400, 0x800, 0x100 };
int16_t RightClimbTab[4] = { 0x800, 0x100, 0x200, 0x400 };
int16_t DeathSlideBounds[12] { -256, 256, -100, 100, WALL_L / 2 - 256, WALL_L / 2, 0, 0, -25 * ONE_DEGREE, 25 * ONE_DEGREE, 0, 0 };

PHD_VECTOR DeathSlidePosition{ 0, 0, WALL_L / 2 - 141 };

int LaraLandedBad(ITEM_INFO* item, COLL_INFO* coll);
int LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll);
int GetStaticObjects(ITEM_INFO* item, PHD_ANGLE ang, int32_t hite, int32_t radius, int32_t dist);
int16_t LaraFloorFront(ITEM_INFO* item, PHD_ANGLE ang, int32_t dist);
int16_t LaraCeilingFront(ITEM_INFO* item, PHD_ANGLE ang, int32_t dist, int h);
int LaraFallen(ITEM_INFO* item, COLL_INFO* coll);
int LaraTestEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, int32_t* edge);
int LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll);

void GetLaraCollisionInfo(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->facing = lara.move_angle;

	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, LARA_HITE);
}

void SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, int16_t rot, bool flip)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_FORWARDJUMP;
		item->current_anim_state = AS_FORWARDJUMP;
		item->anim_number = FALLDOWN_A;
		item->frame_number = FALLDOWN_F;
		item->pos.y_pos += 256;
		item->gravity_status = 1;
		item->speed = 2;
		item->fallspeed = 1;
		lara.gun_status = LG_ARMLESS;
		item->pos.y_rot += rot / 2;
	}
	else if (flip)
	{
		item->anim_number = HANG_A;
		item->frame_number = HANG_F;
		item->goal_anim_state = AS_HANG;
		item->current_anim_state = AS_HANG;

		item->pos.x_pos = coll->old.x = lara.corner_x;
		item->pos.z_pos = coll->old.z = lara.corner_z;
		item->pos.y_rot += rot;
	}
}

int IsValidHangPos(ITEM_INFO* item, COLL_INFO* coll)
{
	if (LaraFloorFront(item, lara.move_angle, LARA_RAD) >= 200)
	{
		switch (PHD_ANGLE angle = (uint16_t)(item->pos.y_rot + 0x2000) / 0x4000)
		{
		case NORTH: item->pos.z_pos += 4; break;
		case WEST:	item->pos.x_pos -= 4; break;
		case SOUTH: item->pos.z_pos -= 4; break;
		case EAST:	item->pos.x_pos += 4; break;
		}

		coll->bad_pos = NO_BAD_POS;
		coll->bad_neg = -STEP_L * 2;
		coll->bad_ceiling = 0;

		lara.move_angle = item->pos.y_rot;

		GetLaraCollisionInfo(item, coll);

		if (coll->mid_ceiling < 0 && coll->coll_type == COLL_FRONT && !coll->hit_static)
		{
			if (ABS(coll->front_floor - coll->right_floor2) < SLOPE_DIF)
				return 1;
		}
	}

	return 0;
}

int LaraHangLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->anim_number != HANG_A)
		return 0;

	if (coll->hit_static)
		return 0;

	int oldx = item->pos.x_pos,
		oldz = item->pos.z_pos,
		front = coll->front_floor;

	auto oldy = item->pos.y_rot;
	auto angle = (uint16_t)((uint16_t)(item->pos.y_rot + 0x2000) / 0x4000);

	int x, z;

	switch (angle)
	{
	case NORTH:
	case SOUTH:
		x = (oldx & ~1023) + (1024 - (oldz & 1023));
		z = (oldz & ~1023) + (1024 - (oldx & 1023));
		break;
	default:
		x = (oldx & ~1023) + (oldz & 1023);
		z = (oldz & ~1023) + (oldx & 1023);
		break;
	}

	lara.corner_x = item->pos.x_pos = x;
	lara.corner_z = item->pos.z_pos = z;

	item->pos.y_rot -= 0x4000;

	int flag;

	if ((flag = -IsValidHangPos(item, coll)))
	{
		if (lara.climb_status)
		{
			if (!(GetClimbTrigger(x, item->pos.y_pos, z, item->room_number) & RightClimbTab[angle]))
				flag = 0;
		}
		else if (ABS(front - coll->front_floor) > SLOPE_DIF)
			flag = 0;
	}

	if (!flag)
	{
		item->pos.x_pos = oldx;
		item->pos.y_rot = lara.move_angle = oldy;
		item->pos.z_pos = oldz;

		if (LaraFloorFront(item, item->pos.y_rot - 0x4000, HANG_RAD) < 0)
			return 0;

		switch (angle)
		{
		case NORTH:
			x = (item->pos.x_pos & ~1023) - (1024 - (item->pos.z_pos & 1023));
			z = ((item->pos.z_pos + 1024) & ~1023) + (item->pos.x_pos & 1023);
			break;
		case WEST:
			x = (item->pos.x_pos & ~1023) - (item->pos.z_pos & 1023);
			z = (item->pos.z_pos & ~1023) - (item->pos.x_pos & 1023);
			break;
		case SOUTH:
			x = ((item->pos.x_pos + 1024) & ~1023) + (item->pos.z_pos & 1023);
			z = ((item->pos.z_pos - 1024) & ~1023) + (item->pos.x_pos & 1023);
			break;
		default:
			x = ((item->pos.x_pos + 1024) & ~1023) + (1024 - (item->pos.z_pos & 1023));
			z = ((item->pos.z_pos + 1024) & ~1023) + (1024 - (item->pos.x_pos & 1023));
		}

		lara.corner_x = item->pos.x_pos = x;
		lara.corner_z = item->pos.z_pos = z;

		item->pos.y_rot += 0x4000;

		if ((flag = IsValidHangPos(item, coll)))
		{
			item->pos.x_pos = oldx;
			item->pos.y_rot = lara.move_angle = oldy;
			item->pos.z_pos = oldz;

			if (lara.climb_status)
			{
				if (!(GetClimbTrigger(x, item->pos.y_pos, z, item->room_number) & LeftClimbTab[angle]))
				{
					front = LaraFloorFront(item, item->pos.y_rot, HANG_RAD);

					if (ABS(coll->front_floor - front) > SLOPE_DIF || front < -768)
						flag = 0;
				}
			}
			else
			{
				if (ABS(front - coll->front_floor) > SLOPE_DIF)
					flag = 0;
				else
				{
					switch (angle)
					{
					case NORTH:
					{
						if ((oldx & 1023) > 512)
							flag = 0;

						break;
					}
					case WEST:
					{
						if ((oldz & 1023) > 512)
							flag = 0;

						break;
					}
					case SOUTH:
					{
						if ((oldx & 1023) < 512)
							flag = 0;

						break;
					}
					case EAST:
					{
						if ((oldz & 1023) < 512)
							flag = 0;
					}
					}
				}
			}

			return flag;
		}
	}

	item->pos.x_pos = oldx;
	item->pos.y_rot = lara.move_angle = oldy;
	item->pos.z_pos = oldz;

	return flag;
}

int LaraHangRightCornerTest(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->anim_number != HANG_A)
		return 0;

	if (coll->hit_static)
		return 0;

	int oldx = item->pos.x_pos,
		oldz = item->pos.z_pos,
		front = coll->front_floor;

	auto old_ry = item->pos.y_rot;
	auto angle = (uint16_t)((uint16_t)(item->pos.y_rot + 0x2000) / 0x4000);

	int x, z;

	switch (angle)
	{
	case NORTH:
	case SOUTH:
		x = (oldx & ~1023) + (oldz & 1023);
		z = (oldz & ~1023) + (oldx & 1023);
		break;
	default:
		x = (oldx & ~1023) + (1024 - (oldz & 1023));
		z = (oldz & ~1023) + (1024 - (oldx & 1023));
	}

	lara.corner_x = item->pos.x_pos = x;
	lara.corner_z = item->pos.z_pos = z;

	item->pos.y_rot += 0x4000;

	int flag = 0;

	if ((flag = -IsValidHangPos(item, coll)))
	{
		if (lara.climb_status)
		{
			if (!(GetClimbTrigger(x, item->pos.y_pos, z, item->room_number) & LeftClimbTab[angle]))
				flag = 0;
		}
		else if (ABS(front - coll->front_floor) > SLOPE_DIF)
			flag = 0;
	}

	if (!flag)
	{
		item->pos.x_pos = oldx;
		item->pos.y_rot = lara.move_angle = old_ry;
		item->pos.z_pos = oldz;

		if (LaraFloorFront(item, item->pos.y_rot + 0x4000, HANG_RAD) < 0)
			return 0;

		switch (angle)
		{
		case NORTH:
			x = ((item->pos.x_pos + 1024) & ~1023) + (1024 - (item->pos.z_pos & 1023));
			z = ((item->pos.z_pos + 1024) & ~1023) + (1024 - (item->pos.x_pos & 1023));
			break;
		case WEST:
			x = ((item->pos.x_pos) & ~1023) - (1024 - (item->pos.z_pos & 1023));
			z = ((item->pos.z_pos + 1024) & ~1023) + (item->pos.x_pos & 1023);
			break;
		case SOUTH:
			x = ((item->pos.x_pos - 1024) & ~1023) + (1024 - (item->pos.z_pos & 1023));
			z = ((item->pos.z_pos - 1024) & ~1023) + (1024 - (item->pos.x_pos & 1023));
			break;
		default:
			x = ((item->pos.x_pos + 1024) & ~1023) + (item->pos.z_pos & 1023);
			z = (item->pos.z_pos & ~1023) - (1024 - (item->pos.x_pos & 1023));
		}

		lara.corner_x = item->pos.x_pos = x;
		lara.corner_z = item->pos.z_pos = z;

		item->pos.y_rot -= 0x4000;

		if ((flag = IsValidHangPos(item, coll)))
		{
			item->pos.x_pos = oldx;
			item->pos.y_rot = lara.move_angle = old_ry;
			item->pos.z_pos = oldz;

			if (lara.climb_status)
			{
				if (!(GetClimbTrigger(x, item->pos.y_pos, z, item->room_number) & RightClimbTab[angle]))
				{
					front = LaraFloorFront(item, item->pos.y_rot, HANG_RAD);

					if (ABS(coll->front_floor - front) > SLOPE_DIF || front < -768)
						flag = 0;
				}
			}
			else
			{
				if (ABS(front - coll->front_floor) > SLOPE_DIF)
					flag = 0;
				else
				{
					switch (angle)
					{
					case NORTH:
					{
						if ((oldx & 1023) < 512)
							flag = 0;

						break;
					}
					case WEST:
					{
						if ((oldz & 1023) < 512)
							flag = 0;

						break;
					}
					case SOUTH:
					{
						if ((oldx & 1023) > 512)
							flag = 0;

						break;
					}
					case EAST:
					{
						if ((oldz & 1023) > 512)
							flag = 0;
					}
					}
				}
			}

			return flag;
		}
	}

	item->pos.x_pos = oldx;
	item->pos.y_rot = lara.move_angle = old_ry;
	item->pos.z_pos = oldz;

	return flag;
}

int LaraDeflectEdgeDuck(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->coll_type == COLL_FRONT || coll->coll_type == COLL_TOPFRONT)
	{
		ShiftItem(item, coll);

		item->speed = 0;
		item->gravity_status = 0;

		return 1;
	}
	else if (coll->coll_type == COLL_LEFT)
	{
		ShiftItem(item, coll);
		item->pos.y_rot += LARA_DUCK_DEFLECT;
	}
	else if (coll->coll_type == COLL_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.y_rot -= LARA_DUCK_DEFLECT;
	}

	return 0;
}

int TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll)
{
	static PHD_ANGLE old_ang = 1;

	if (ABS(coll->tilt_x) <= 2 && ABS(coll->tilt_z) <= 2)
		return 0;

	PHD_ANGLE ang = 0;

	if (coll->tilt_x > 2)		ang = -16384;
	else if (coll->tilt_x < -2) ang = 16384;

	if (coll->tilt_z > 2 && coll->tilt_z > (ABS(coll->tilt_x)))    ang = -32768;
	else if (coll->tilt_z<-2 && -coll->tilt_z>(ABS(coll->tilt_x))) ang = 0;

	PHD_ANGLE adif = ang - item->pos.y_rot;

	ShiftItem(item, coll);

	if (adif >= -16384 && adif <= 16384)
	{
		if (item->current_anim_state != AS_SLIDE || old_ang != ang)
		{
			item->anim_number = SLIDE_A;
			item->frame_number = SLIDE_F;
			item->goal_anim_state = AS_SLIDE;
			item->current_anim_state = AS_SLIDE;
			item->pos.y_rot = ang;

			lara.move_angle = old_ang = ang;
		}
	}
	else
	{
		if (item->current_anim_state != AS_SLIDEBACK || old_ang != ang)
		{
			item->anim_number = SLIDEBACK_A;
			item->frame_number = SLIDEBACK_F;
			item->goal_anim_state = AS_SLIDEBACK;
			item->current_anim_state = AS_SLIDEBACK;
			item->pos.y_rot = ang - 32768;

			lara.move_angle = old_ang = ang;
		}
	}

	return 1;
}

int LaraTestClimbStance(ITEM_INFO* item, COLL_INFO* coll)
{
	int shift_r,
		result_r = LaraTestClimbPos(item, coll->radius, coll->radius + CLIMB_WIDTHR, -700, (STEP_L * 2), &shift_r);

	if (result_r != 1)
		return 0;

	int shift_l,
		result_l = LaraTestClimbPos(item, coll->radius, -(coll->radius + CLIMB_WIDTHL), -700, (STEP_L * 2), &shift_l);

	if (result_l != 1)
		return 0;

	if (shift_r)
	{
		if (shift_l)
		{
			if ((shift_l < 0) ^ (shift_r < 0))
				return 0;

			if (shift_r < 0 && shift_l < shift_r)	   shift_r = shift_l;
			else if (shift_r > 0 && shift_l > shift_r) shift_r = shift_l;
		}

		item->pos.y_pos += shift_r;
	}
	else if (shift_l)
		item->pos.y_pos += shift_l;

	return 1;
}

int TestLaraVault(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS)
		return 0;

	if (coll->coll_type == COLL_FRONT)
	{
		auto angle = item->pos.y_rot;

		if (angle >= 0 - VAULT_ANGLE && angle <= 0 + VAULT_ANGLE)				 angle = 0;
		else if (angle >= 16384 - VAULT_ANGLE && angle <= 16384 + VAULT_ANGLE)	 angle = 16384;
		else if (angle >= 32767 - VAULT_ANGLE || angle <= -32767 + VAULT_ANGLE)  angle = -32768;
		else if (angle >= -16384 - VAULT_ANGLE && angle <= -16384 + VAULT_ANGLE) angle = -16384;

		if (angle & 0x3fff)
			return 0;

		int hdif = coll->front_floor,
			slope = (ABS(coll->left_floor2 - coll->right_floor2) >= SLOPE_DIF);

		if (hdif >= -STEP_L * 2 - STEP_L / 2 && hdif <= -STEP_L * 2 + STEP_L / 2)
		{
			if (slope ||
				coll->front_floor - coll->front_ceiling < 0 ||
				coll->left_floor2 - coll->left_ceiling2 < 0 ||
				coll->right_floor2 - coll->right_ceiling2 < 0)
				return 0;

			if ((room[item->room_number].flags & SWAMP) && lara.water_surface_dist < -768)
				return 0;

			item->anim_number = VAULT12_A;
			item->frame_number = VAULT12_F;
			item->current_anim_state = AS_NULL;
			item->goal_anim_state = AS_STOP;
			item->pos.y_pos += STEP_L * 2 + hdif;

			lara.gun_status = LG_HANDSBUSY;
		}
		else if ((hdif >= (-STEP_L * 3 - STEP_L / 2)) && (hdif <= (-STEP_L * 3 + STEP_L / 2)))
		{
			if (slope || coll->front_floor - coll->front_ceiling < 0 || coll->left_floor2 - coll->left_ceiling2 < 0 || coll->right_floor2 - coll->right_ceiling2 < 0)
				return 0;

			if ((room[item->room_number].flags & SWAMP) && lara.water_surface_dist < -768)
				return 0;

			item->anim_number = VAULT34_A;
			item->frame_number = VAULT34_F;
			item->current_anim_state = AS_NULL;
			item->goal_anim_state = AS_STOP;
			item->pos.y_pos += STEP_L * 3 + hdif;

			lara.gun_status = LG_HANDSBUSY;
		}
		else if (!slope && hdif >= -STEP_L * 7 - STEP_L / 2 && hdif <= -STEP_L * 4 + STEP_L / 2)
		{
			if (room[item->room_number].flags & SWAMP)
				return 0;

			item->anim_number = STOP_A;
			item->frame_number = STOP_F;
			item->goal_anim_state = AS_UPJUMP;
			item->current_anim_state = AS_STOP;

			lara.calc_fallspeed = -(int16_t)(phd_sqrt((int)(-2 * GRAVITY * (hdif + 800))) + 3);

			AnimateLara(item);
		}
		else if (lara.climb_status &&
				 hdif <= -STEP_L * 8 + STEP_L / 2 &&
				 lara.water_status != LARA_WADE &&
				 coll->left_floor2 <= -STEP_L * 8 + STEP_L / 2 &&
				 coll->right_floor2 <= -STEP_L * 8 &&
				 coll->mid_ceiling <= -STEP_L * 8 + STEP_L / 2 + LARA_HITE)
		{
			item->anim_number = STOP_A;
			item->frame_number = STOP_F;
			item->goal_anim_state = AS_UPJUMP;
			item->current_anim_state = AS_STOP;

			lara.calc_fallspeed = -116;

			AnimateLara(item);
		}
		else if (lara.climb_status && (hdif < -STEP_L * 4 || coll->front_ceiling >= LARA_HITE - STEP_L) && (coll->mid_ceiling <= -STEP_L * 5 + LARA_HITE))
		{
			ShiftItem(item, coll);

			if (LaraTestClimbStance(item, coll))
			{
				item->anim_number = STOP_A;
				item->frame_number = STOP_F;
				item->goal_anim_state = AS_CLIMBSTNC;
				item->current_anim_state = AS_STOP;

				AnimateLara(item);

				item->pos.y_rot = angle;

				lara.gun_status = LG_HANDSBUSY;

				return 1;
			}

			return 0;
		}
		else return 0;

		item->pos.y_rot = angle;

		ShiftItem(item, coll);

		return 1;
	}
	else return 0;
}

int TestWall(ITEM_INFO* item, int32_t front, int32_t right, int32_t down)
{
	int x = item->pos.x_pos,
		y = item->pos.y_pos + down,
		z = item->pos.z_pos;

	auto room_num = item->room_number;

	const auto angle = uint16_t((uint16_t)(item->pos.y_rot + 0x2000) / 0x4000);

	switch (angle)
	{
	case NORTH: x -= right; break;
	case WEST:  z += right; break;
	case SOUTH: x += right; break;
	case EAST:  z -= right; break;
	}

	auto floor = GetFloor(x, y, z, &room_num);

	switch (angle)
	{
	case NORTH: z += front; break;
	case WEST:  x -= front; break;
	case SOUTH: z -= front; break;
	case EAST:  x += front; break;
	}

	floor = GetFloor(x, y, z, &room_num);

	int h = GetHeight(floor, x, y, z),
		c = GetCeiling(floor, x, y, z);

	if (h != NO_HEIGHT)
	{
		if (h - y > 0 && c - y < 0)
			return 0;

		return 2;
	}

	return 1;
}

void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll)
{
	switch (coll->old_anim_state)
	{
	case AS_STOP:
	case AS_TURN_L:
	case AS_TURN_R:
	case AS_FASTTURN:
	{
		item->current_anim_state = coll->old_anim_state;
		item->anim_number = coll->old_anim_number;
		item->frame_number = coll->old_frame_number;

		if (input & IN_LEFT)	   item->goal_anim_state = AS_TURN_L;
		else if (input & IN_RIGHT) item->goal_anim_state = AS_TURN_R;
		else					   item->goal_anim_state = AS_STOP;

		return AnimateLara(item);
	}
	}

	item->anim_number = STOP_A;
	item->frame_number = STOP_F;
}

void lara_slide_slope(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -512;
	coll->bad_ceiling = 0;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll))
		return;

	LaraDeflectEdge(item, coll);

	if (coll->mid_floor > 200)
	{
		if (item->current_anim_state == AS_SLIDE)
		{
			item->anim_number = FALLDOWN_A;
			item->frame_number = FALLDOWN_F;
			item->current_anim_state = AS_FORWARDJUMP;
			item->goal_anim_state = AS_FORWARDJUMP;
		}
		else
		{
			item->anim_number = FALLBACK_A;
			item->frame_number = FALLBACK_F;
			item->current_anim_state = AS_FALLBACK;
			item->goal_anim_state = AS_FALLBACK;
		}

		g_audio->stop_sound(3);

		item->fallspeed = 0;
		item->gravity_status = 1;

		return;
	}

	TestLaraSlide(item, coll);

	item->pos.y_pos += coll->mid_floor;

	if (ABS(coll->tilt_x) <= 2 && ABS(coll->tilt_z) <= 2)
	{
		item->goal_anim_state = AS_STOP;

		g_audio->stop_sound(3);
	}
}

void LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll)
{
	ShiftItem(item, coll);

	switch (coll->coll_type)
	{
	case COLL_FRONT:
	case COLL_TOPFRONT:
	{
		if (lara.climb_status && item->speed == 2)
			return;

		if (coll->mid_floor > 512)
		{
			item->goal_anim_state = AS_FASTFALL;
			item->current_anim_state = AS_FASTFALL;
			item->frame_number = FASTSPLAT_F;
			item->anim_number = FASTSPLAT_A;
		}
		else if (coll->mid_floor <= 128)
		{
			item->goal_anim_state = AS_LAND;
			item->current_anim_state = AS_LAND;
			item->frame_number = LAND_F;
			item->anim_number = LAND_A;
		}

		item->speed /= 4;

		lara.move_angle += -0x8000;

		if (item->fallspeed <= 0)
			item->fallspeed = 1;

		break;
	}
	case COLL_LEFT:
		item->pos.y_rot += LARA_DEF_ADD_EDGE;
		break;
	case COLL_RIGHT:
		item->pos.y_rot -= LARA_DEF_ADD_EDGE;
		break;
	case COLL_TOP:
	{
		if (item->fallspeed <= 0)
			item->fallspeed = 1;

		break;
	}
	case COLL_CLAMP:
	{
		item->pos.z_pos -= (phd_cos(coll->facing) * 100) >> W2V_SHIFT;
		item->pos.x_pos -= (phd_sin(coll->facing) * 100) >> W2V_SHIFT;
		item->speed = 0;

		coll->mid_floor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;
	}
	}
}

void LaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll)
{
	ShiftItem(item, coll);

	switch (coll->coll_type)
	{
	case COLL_LEFT:
		item->pos.y_rot += LARA_DEF_ADD_EDGE;
		break;
	case COLL_RIGHT:
		item->pos.y_rot -= LARA_DEF_ADD_EDGE;
		break;
	case COLL_TOP:
	case COLL_TOPFRONT:
	{
		if (item->fallspeed <= 0)
			item->fallspeed = 1;

		break;
	}
	case COLL_CLAMP:
	{
		item->pos.z_pos -= (phd_cos(coll->facing) * 100) >> W2V_SHIFT;
		item->pos.x_pos -= (phd_sin(coll->facing) * 100) >> W2V_SHIFT;
		item->speed = 0;

		coll->mid_floor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;
	}
	}
}

int LaraTestHangOnClimbWall(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!lara.climb_status || item->fallspeed < 0)
		return 0;

	switch ((uint16_t)(item->pos.y_rot + 0x2000) / 0x4000)
	{
	case NORTH:
	case SOUTH: item->pos.z_pos += coll->shift.z; break;
	case EAST:
	case WEST:  item->pos.x_pos += coll->shift.x; break;
	}

	auto bounds = GetBoundsAccurate(item);

	if (lara.move_angle != item->pos.y_rot)
	{
		int16_t l = LaraCeilingFront(item, item->pos.y_rot, 0, 0),
				r = LaraCeilingFront(item, lara.move_angle, 128, 0);

		if (ABS(l - r) > SLOPE_DIF)
			return 0;
	}

	int shift;

	if (!LaraTestClimbPos(item, coll->radius, coll->radius, bounds[2], bounds[3] - bounds[2], &shift) ||
		!LaraTestClimbPos(item, coll->radius, -coll->radius, bounds[2], bounds[3] - bounds[2], &shift))
		return 0;

	int result = LaraTestClimbPos(item, coll->radius, 0, bounds[2], bounds[3] - bounds[2], &shift);

	if (!result)		  return 0;
	else if (result == 1) return 1;

	item->pos.y_pos += shift;

	return 1;
}

void SnapLaraToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, PHD_ANGLE angle)
{
	if (item->current_anim_state == AS_HANGRIGHT)
	{
		switch (angle)
		{
		case NORTH: item->pos.x_pos = (coll->old.x & ~1023) | 0x390; break;
		case SOUTH: item->pos.x_pos = (coll->old.x & ~1023) | 0x70;  break;
		case EAST:	item->pos.z_pos = (coll->old.z & ~1023) | 0x70;  break;
		default:	item->pos.z_pos = (coll->old.z & ~1023) | 0x390;
		}
	}
	else if (item->current_anim_state == AS_HANGLEFT)
	{
		switch (angle)
		{
		case NORTH: item->pos.x_pos = (coll->old.x & ~1023) | 0x70;  break;
		case SOUTH: item->pos.x_pos = (coll->old.x & ~1023) | 0x390; break;
		case EAST:	item->pos.z_pos = (coll->old.z & ~1023) | 0x390; break;
		default:	item->pos.z_pos = (coll->old.z & ~1023) | 0x70;
		}
	}
}

int LaraHangTest(ITEM_INFO* item, COLL_INFO* coll)
{
	int16_t move = 0,
		    dir = lara.move_angle,
		    wall;

	if (dir == ((int16_t)(item->pos.y_rot - 0x4000)))		move = -100;
	else if (dir == ((int16_t)(item->pos.y_rot + 0x4000)))	move = 100;

	int flag = 0;

	if ((wall = LaraFloorFront(item, lara.move_angle, LARA_RAD)) < 200)
		flag = 1;

	auto ceiling = LaraCeilingFront(item, lara.move_angle, LARA_RAD, 0);

	PHD_ANGLE angle = (uint16_t)(item->pos.y_rot + 0x2000) / 0x4000;

	switch (angle)
	{
	case NORTH: item->pos.z_pos += 4; break;
	case WEST:  item->pos.x_pos -= 4; break;
	case SOUTH: item->pos.z_pos -= 4; break;
	case EAST:  item->pos.x_pos += 4;
	}

	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;

	GetLaraCollisionInfo(item, coll);

	if (lara.climb_status)
	{
		if (!(input & IN_ACTION) || item->hit_points <= 0)
		{
			item->goal_anim_state = AS_FORWARDJUMP;
			item->current_anim_state = AS_FORWARDJUMP;
			item->anim_number = FALLDOWN_A;
			item->frame_number = FALLDOWN_F;
			item->pos.y_pos += 256;
			item->gravity_status = 1;
			item->speed = 2;
			item->fallspeed = 1;

			lara.gun_status = LG_ARMLESS;

			return 0;
		}

		lara.move_angle = dir;

		if (!LaraTestHangOnClimbWall(item, coll))
		{
			if (item->anim_number != GRABLEFT_A && item->anim_number != GRABRIGHT_A)
			{
				SnapLaraToEdgeOfBlock(item, coll, angle);

				item->pos.y_pos = coll->old.y;
				item->goal_anim_state = AS_HANG;
				item->current_anim_state = AS_HANG;
				item->frame_number = HANG_F;
				item->anim_number = HANG_A;
			}

			return 1;
		}

		if (item->anim_number == HANG_A && item->frame_number == HANG_F && LaraTestClimbStance(item, coll))
			item->goal_anim_state = AS_CLIMBSTNC;

		return 0;
	}

	if (!(input & IN_ACTION) || item->hit_points <= 0 || coll->front_floor > 0)
	{
		item->goal_anim_state = AS_UPJUMP;
		item->current_anim_state = AS_UPJUMP;
		item->anim_number = STOPHANG_A;
		item->frame_number = STOPHANG_F;

		auto bounds = GetBoundsAccurate(item);

		item->pos.x_pos += coll->shift.x;
		item->pos.y_pos += bounds[3];
		item->pos.z_pos += coll->shift.z;
		item->gravity_status = 1;
		item->speed = 2;
		item->fallspeed = 1;

		lara.gun_status = LG_ARMLESS;

		return 0;
	}

	if (flag && wall > 0)
		if ((move > 0 && coll->left_floor > coll->right_floor) || (move < 0 && coll->left_floor < coll->right_floor))
			flag = 0;

	wall = 0;

	auto bounds = GetBoundsAccurate(item);
	auto oldfloor = coll->front_floor;

	int hdif = oldfloor - bounds[2],
		x, z;

	switch (angle)
	{
	case NORTH:
		x = item->pos.x_pos + move;
		z = item->pos.z_pos;
		break;
	case SOUTH:
		x = item->pos.x_pos - move;
		z = item->pos.z_pos;
		break;
	case EAST:
		x = item->pos.x_pos;
		z = item->pos.z_pos - move;
		break;
	default:
		x = item->pos.x_pos;
		z = item->pos.z_pos + move;
	}

	lara.move_angle = dir;

	if ((GetClimbTrigger(x, item->pos.y_pos, z, item->room_number) & (0x100 << angle)) && !LaraTestHangOnClimbWall(item, coll))
		hdif = 0;

	else if ((ABS(coll->left_floor2 - coll->right_floor2)) >= SLOPE_DIF)
	{
		if (move < 0 && coll->left_floor2 != coll->front_floor)
			wall = 1;

		if (move > 0 && coll->right_floor2 != coll->front_floor)
			wall = 1;
	}

	coll->front_floor = oldfloor;

	if (wall
		|| coll->mid_ceiling >= 0
		|| coll->coll_type != COLL_FRONT
		|| flag
		|| coll->hit_static
		|| ceiling > -950
		|| hdif < -SLOPE_DIF
		|| hdif > SLOPE_DIF)
	{
		item->pos.x_pos = coll->old.x;
		item->pos.y_pos = coll->old.y;
		item->pos.z_pos = coll->old.z;

		if (item->current_anim_state == AS_HANGLEFT || item->current_anim_state == AS_HANGRIGHT)
		{
			item->goal_anim_state = AS_HANG;
			item->current_anim_state = AS_HANG;
			item->frame_number = HANG_F;
			item->anim_number = HANG_A;
		}

		return 1;
	}

	switch (angle)
	{
	case NORTH:
	case SOUTH: item->pos.z_pos += coll->shift.z; break;
	case WEST:
	case EAST:  item->pos.x_pos += coll->shift.x; break;
	}

	item->pos.y_pos += hdif;

	return 0;
}

int TestHangSwingIn(ITEM_INFO* item, PHD_ANGLE angle)
{
	int x = item->pos.x_pos,
		y = item->pos.y_pos,
		z = item->pos.z_pos;

	auto room_num = item->room_number;

	switch (angle)
	{
	case 0:		 z += 256; break;
	case 16384:  x += 256; break;
	case -16384: x -= 256; break;
	case -32768: z -= 256;
	}

	auto floor = GetFloor(x, y, z, &room_num);

	int h = GetHeight(floor, x, y, z),
		c = GetCeiling(floor, x, y, z);

	if (h != NO_HEIGHT && h - y > 0 && c - y < -400 && y - 819 - c > -72)
		return 1;

	return 0;
}

int LaraTestHangJump(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || coll->hit_static)
		return 0;

	if (lara.can_monkey_swing && coll->coll_type == COLL_TOP)
	{
		lara.head_x_rot = lara.head_y_rot = 0;
		lara.torso_x_rot = lara.torso_y_rot = 0;

		item->goal_anim_state = AS_HANG2;
		item->current_anim_state = AS_HANG2;
		item->anim_number = GRABLEDGEIN_A;
		item->frame_number = GRABLEDGEIN_F;
		item->gravity_status = 0;
		item->speed = 0;
		item->fallspeed = 0;

		lara.gun_status = LG_HANDSBUSY;

		return 1;
	}

	if (coll->mid_ceiling > -STEPUP_HEIGHT || coll->mid_floor < 200 || coll->coll_type != COLL_FRONT)
		return 0;

	int32_t edge,
		   edge_catch = LaraTestEdgeCatch(item, coll, &edge);

	if (!edge_catch || (edge_catch < 0 && !LaraTestHangOnClimbWall(item, coll)))
		return 0;

	auto angle = item->pos.y_rot;

	if (angle >= 0 - HANG_ANGLE && angle <= 0 + HANG_ANGLE)					angle = 0;
	else if (angle >= 16384 - HANG_ANGLE && angle <= 16384 + HANG_ANGLE)	angle = 16384;
	else if (angle >= 32767 - HANG_ANGLE || angle <= -32767 + HANG_ANGLE)	angle = -32768;
	else if (angle >= -16384 - HANG_ANGLE && angle <= -16384 + HANG_ANGLE)	angle = -16384;

	if (angle & 0x3fff)
		return 0;

	if (TestHangSwingIn(item, angle))
	{
		lara.head_x_rot = lara.head_y_rot = 0;
		lara.torso_x_rot = lara.torso_y_rot = 0;

		item->anim_number = GRABLEDGEIN_A;
		item->frame_number = GRABLEDGEIN_F;
		item->current_anim_state = AS_HANG2;
		item->goal_anim_state = AS_HANG2;
	}
	else
	{
		item->anim_number = GRABLEDGE_A;
		item->frame_number = GRABLEDGE_F;
		item->current_anim_state = AS_HANG;
		item->goal_anim_state = AS_HANG;
	}

	auto bounds = GetBoundsAccurate(item);

	if (edge_catch > 0)
	{
		item->pos.y_pos += coll->front_floor - bounds[2];
		item->pos.x_pos += coll->shift.x;
		item->pos.z_pos += coll->shift.z;
	}
	else item->pos.y_pos = edge - bounds[2];

	item->pos.y_rot = angle;
	item->gravity_status = 1;
	item->speed = 2;
	item->fallspeed = 1;

	lara.gun_status = LG_HANDSBUSY;

	return 1;
}

void ResetLaraState()
{
	lara_item->goal_anim_state = AS_STOP;
	lara_item->current_anim_state =
	lara_item->goal_anim_state =
	lara_item->required_anim_state = AS_STOP;
	lara_item->anim_number = BREATH_A;
	lara_item->frame_number = BREATH_F;
	lara_item->gravity_status = 0;
	lara_item->fallspeed = 0;
	lara_item->speed = 0;

	if (lara.gun_type != LG_UNARMED)
	{
		undraw_pistol_mesh_left(lara.gun_type);
		undraw_pistol_mesh_right(lara.gun_type);
	}

	lara.extra_anim = 0;
	lara.gun_status = LG_ARMLESS;
	lara.back_gun = LG_UNARMED;
	lara.gun_type = LG_UNARMED;
	lara.request_gun_type = LG_UNARMED;
	lara.last_gun_type = LG_UNARMED;
	lara.hit_direction = -1;
}

void LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->old.x = item->pos.x_pos;
	coll->old.y = item->pos.y_pos;
	coll->old.z = item->pos.z_pos;
	coll->old_anim_state = item->current_anim_state;
	coll->old_anim_number = item->anim_number;
	coll->old_frame_number = item->frame_number;
	coll->radius = LARA_RAD;
	coll->trigger = nullptr;
	coll->lava_is_pit = 0;
	coll->slopes_are_walls = 0;
	coll->slopes_are_pits = 0;
	coll->enable_spaz = 1;
	coll->enable_baddie_push = 1;

	if ((input & IN_LOOK) && !lara.extra_anim && lara.look)
		LookLeftRight();
	else ResetLook();

	lara.look = 1;

	if (lara.skidoo != NO_ITEM)
	{
		if (items[lara.skidoo].object_number == KAYAK)
		{
			if (KayakControl())
				return;
		}
		else if (items[lara.skidoo].object_number == QUADBIKE)
		{
			if (QuadBikeControl())
				return;
		}
		else if (items[lara.skidoo].object_number == UPV)
		{
			if (SubControl())
				return;
		}
		else if (items[lara.skidoo].object_number == BIGGUN)
		{
			if (BigGunControl(coll))
				return;
		}
		else if (items[lara.skidoo].object_number == MINECART)
		{
			if (MineCartControl())
				return;
		}
		else return LaraGun();
	}

	reinterpret_cast<void(__cdecl*)(ITEM_INFO*, COLL_INFO*)>(*(lara.extra_anim ? extra_control_routines : lara_control_routines)[item->current_anim_state])(item, coll);

	if (item->pos.z_rot >= -LARA_LEAN_UNDO && item->pos.z_rot <= LARA_LEAN_UNDO) item->pos.z_rot = 0;
	else if (item->pos.z_rot < -LARA_LEAN_UNDO)									 item->pos.z_rot += LARA_LEAN_UNDO;
	else																		 item->pos.z_rot -= LARA_LEAN_UNDO;

	if (lara.turn_rate >= -LARA_TURN_UNDO && lara.turn_rate <= LARA_TURN_UNDO) lara.turn_rate = 0;
	else if (lara.turn_rate < -LARA_TURN_UNDO)								   lara.turn_rate += LARA_TURN_UNDO;
	else																	   lara.turn_rate -= LARA_TURN_UNDO;

	item->pos.y_rot += lara.turn_rate;

	AnimateLara(item);

	if (!lara.extra_anim)
	{
		LaraBaddieCollision(item, coll);

		if (lara.skidoo == NO_ITEM)
			reinterpret_cast<void(__cdecl*)(ITEM_INFO*, COLL_INFO*)>(*lara_collision_routines[item->current_anim_state])(item, coll);
	}

	UpdateLaraRoom(item, -LARA_HITE / 2);
	LaraGun();
	TestTriggers(coll->trigger, 0);
}

void lara_as_duck(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 1;
	coll->enable_baddie_push = 1;

	lara.is_ducked = 1;

	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_ALL4S;
		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (input & IN_LOOK)
		LookUpDown();

	if ((input & (IN_FORWARD | IN_BACK)) && lara.gun_status == LG_ARMLESS && item->frame_number > DUCKBREATHE_F + 10)
	{
		lara.torso_x_rot = lara.torso_y_rot = 0;

		item->goal_anim_state = AS_ALL4S;

		return;
	}
}

void lara_col_duck(ITEM_INFO* item, COLL_INFO* coll)
{
	item->gravity_status = item->fallspeed = 0;
	coll->facing = lara.move_angle = item->pos.y_rot;
	coll->radius = LARA_RAD * 2;
	coll->bad_pos = STEPUP_HEIGHT;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_walls = 1;

	GetCollisionInfo(
		coll,
		item->pos.x_pos,
		item->pos.y_pos,
		item->pos.z_pos,
		item->room_number,
		LARA_DUCK_HEIGHT);

	if (LaraFallen(item, coll))
	{
		lara.gun_status = LG_ARMLESS;
		return;
	}

	lara.keep_ducked = (coll->mid_ceiling >= -LARA_HITE + LARA_DUCK_HEIGHT);

	ShiftItem(item, coll);
	item->pos.y_pos += coll->mid_floor;

	if ((!(input & IN_DUCK) || lara.water_status == LARA_WADE) && !lara.keep_ducked && item->anim_number == DUCKBREATHE_A)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}
}

void lara_as_all4s(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_DEATH;
		return;
	}

	auto crawl_jump_down_info = g_extended_anim_info[CRAWL_JUMP_DOWN];

	if (crawl_jump_down_info && enable_engine_extended_features && (input & IN_JUMP))
	{
		 int16_t height = LaraFloorFront(item, item->pos.y_rot, 768),
				 ceiling = LaraCeilingFront(item, item->pos.y_rot, 768, 512);

		if (height >= 512 && ceiling != NO_HEIGHT && ceiling <= 0)
		{
			GAME_VECTOR s
			{
				lara_item->pos.x_pos,
				lara_item->pos.y_pos - 96,
				lara_item->pos.z_pos,
				lara_item->room_number
			};
			
			GAME_VECTOR d
			{
				s.x + ((phd_sin(lara_item->pos.y_rot) * 768) >> W2V_SHIFT),
				s.y + 160,
				s.z + ((phd_cos(lara_item->pos.y_rot) * 768) >> W2V_SHIFT)
			};

			if (LOS(&s, &d))
			{
				item->anim_number = crawl_jump_down_info.id;
				item->frame_number = crawl_jump_down_info.frame;
				item->goal_anim_state = AS_CONTROLLED;
				item->current_anim_state = AS_CONTROLLED;
			}
		}
	}

	if (input & IN_LOOK)
		LookUpDown();

	lara.torso_x_rot = lara.torso_y_rot = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 1;

	if (item->anim_number == 258)
		lara.gun_status = LG_HANDSBUSY;

	if (TestLaraSlide(item, coll))
		return;

	camera.target_elevation = -23 * ONE_DEGREE;
}

void lara_col_all4s(ITEM_INFO* item, COLL_INFO* coll)
{
	item->gravity_status = item->fallspeed = 0;

	if (item->goal_anim_state == AS_CRAWL2HANG)
		return;

	coll->facing = lara.move_angle = item->pos.y_rot;
	coll->radius = LARA_RAD * 2;
	coll->bad_pos = STEP_L - 1;
	coll->bad_neg = -(STEP_L - 1);
	coll->bad_ceiling = LARA_DUCK_HEIGHT;
	coll->slopes_are_walls = 1;
	coll->slopes_are_pits = 1;

	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, LARA_DUCK_HEIGHT);

	if (LaraFallen(item, coll))
	{
		lara.gun_status = LG_ARMLESS;
		return;
	}

	lara.keep_ducked = (coll->mid_ceiling >= -LARA_HITE + LARA_DUCK_HEIGHT);

	ShiftItem(item, coll);

	item->pos.y_pos += coll->mid_floor;

	if ((!(input & IN_DUCK) && lara.keep_ducked == 0) || (input & IN_DRAW))
		item->goal_anim_state = AS_DUCK;
	else if (item->anim_number == ALL4S_A || item->anim_number == ALL4S2_A)
	{
		if (input & IN_FORWARD)
		{
			auto height = LaraFloorFront(item, item->pos.y_rot, 256);

			if (height < STEP_L - 1 && height > -STEP_L + 1 && height_type != BIG_SLOPE)
				item->goal_anim_state = AS_CRAWL;
		}
		else if (input & IN_BACK)
		{
			auto height = LaraCeilingFront(item, item->pos.y_rot, -300, 128);

			if (height == NO_HEIGHT || height > 256)
				return;

			height = LaraFloorFront(item, item->pos.y_rot, -300);

			if (height < STEP_L - 1 && height > -STEP_L + 1 && height_type != BIG_SLOPE)
				item->goal_anim_state = AS_CRAWLBACK;

			else if ((input & IN_ACTION) && (height > 768))
			{
				if (GetStaticObjects(item, item->pos.y_rot + 0x8000, 512, 50, 300) == 0)
				{
					switch ((uint16_t)(item->pos.y_rot + 0x2000) / 0x4000)
					{
					case NORTH:
						item->pos.z_pos = (item->pos.z_pos & ~1023) + 225;
						item->pos.y_rot = 0;
						break;
					case SOUTH:
						item->pos.z_pos = (item->pos.z_pos | 1023) - 225;
						item->pos.y_rot = PHD_ANGLE(0x8000);
						break;
					case EAST:
						item->pos.x_pos = (item->pos.x_pos & ~1023) + 225;
						item->pos.y_rot = PHD_ANGLE(0x4000);
						break;
					case WEST:
						item->pos.x_pos = (item->pos.x_pos | 1023) - 225;
						item->pos.y_rot = PHD_ANGLE(0xC000);
						break;
					}

					item->goal_anim_state = AS_CRAWL2HANG;
				}
			}
		}
		else if (input & IN_LEFT)
		{
			item->current_anim_state = AS_ALL4TURNL;
			item->goal_anim_state = AS_ALL4TURNL;
			item->anim_number = ALL4TURNL_A;
			item->frame_number = ALL4TURNL_F;
		}
		else if (input & IN_RIGHT)
		{
			item->current_anim_state = AS_ALL4TURNR;
			item->goal_anim_state = AS_ALL4TURNR;
			item->anim_number = ALL4TURNR_A;
			item->frame_number = ALL4TURNR_F;
		}
	}
}

void lara_as_crawl(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_ALL4S;
		return;
	}

	if (input & IN_LOOK)
		LookUpDown();

	lara.torso_x_rot = lara.torso_y_rot = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 1;

	camera.target_elevation = -23 * ONE_DEGREE;

	if (TestLaraSlide(item, coll))
		return;

	if (!(input & IN_FORWARD) || (!(input & IN_DUCK) && !lara.keep_ducked))
		item->goal_anim_state = AS_ALL4S;
	else if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_JUMP_TURN)
			lara.turn_rate = -LARA_JUMP_TURN;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_JUMP_TURN)
			lara.turn_rate = LARA_JUMP_TURN;
	}
}

void lara_col_crawl(ITEM_INFO* item, COLL_INFO* coll)
{
	item->gravity_status = item->fallspeed = 0;

	coll->radius = LARA_RAD * 2;
	coll->bad_pos = STEP_L - 1;
	coll->bad_neg = -(STEP_L - 1);
	coll->bad_ceiling = LARA_DUCK_HEIGHT;
	coll->slopes_are_walls = 1;
	coll->slopes_are_pits = 1;
	coll->facing = lara.move_angle = item->pos.y_rot;

	GetCollisionInfo(
		coll,
		item->pos.x_pos,
		item->pos.y_pos,
		item->pos.z_pos,
		item->room_number,
		LARA_DUCK_HEIGHT);

	if (LaraDeflectEdgeDuck(item, coll))
	{
		item->current_anim_state = AS_ALL4S;
		item->goal_anim_state = AS_ALL4S;

		if (item->anim_number != ALL4S_A)
		{
			item->anim_number = ALL4S_A;
			item->frame_number = ALL4S_F;
		}

		return;
	}

	if (LaraFallen(item, coll))
	{
		lara.gun_status = LG_ARMLESS;
		return;
	}

	ShiftItem(item, coll);

	item->pos.y_pos += coll->mid_floor;
}

void lara_as_all4turnl(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_ALL4S;
		return;
	}

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 1;

	lara.torso_x_rot = lara.torso_y_rot = 0;

	camera.target_elevation = -23 * ONE_DEGREE;

	if (TestLaraSlide(item, coll))
		return;

	item->pos.y_rot -= (ONE_DEGREE + (ONE_DEGREE / 2));

	if (!(input & IN_LEFT))
		item->goal_anim_state = AS_ALL4S;
}

void lara_col_all4turnl(ITEM_INFO* item, COLL_INFO* coll)
{
	GetCollisionInfo(
		coll,
		item->pos.x_pos,
		item->pos.y_pos,
		item->pos.z_pos,
		item->room_number,
		LARA_DUCK_HEIGHT);
}

void lara_as_all4turnr(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_ALL4S;
		return;
	}

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 1;

	lara.torso_x_rot = lara.torso_y_rot = 0;

	camera.target_elevation = -23 * ONE_DEGREE;

	if (TestLaraSlide(item, coll))
		return;

	item->pos.y_rot += (ONE_DEGREE + (ONE_DEGREE / 2));

	if (!(input & IN_RIGHT))
		item->goal_anim_state = AS_ALL4S;
}

void lara_col_all4turnr(ITEM_INFO* item, COLL_INFO* coll)
{
	GetCollisionInfo(
		coll,
		item->pos.x_pos,
		item->pos.y_pos,
		item->pos.z_pos,
		item->room_number,
		LARA_DUCK_HEIGHT);
}

void lara_as_crawlb(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_ALL4S;
		return;
	}

	if (input & IN_LOOK)
		LookUpDown();

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 1;

	lara.torso_x_rot = lara.torso_y_rot = 0;

	camera.target_elevation = -23 * ONE_DEGREE;

	if (TestLaraSlide(item, coll))
		return;

	if (!(input & IN_BACK))
		item->goal_anim_state = AS_ALL4S;
	else if (input & IN_RIGHT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_JUMP_TURN)
			lara.turn_rate = -LARA_JUMP_TURN;
	}
	else if (input & IN_LEFT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_JUMP_TURN)
			lara.turn_rate = LARA_JUMP_TURN;
	}
}

void lara_col_crawlb(ITEM_INFO* item, COLL_INFO* coll)
{
	item->gravity_status = item->fallspeed = 0;

	coll->radius = LARA_CRAWLB_RAD;
	coll->bad_pos = STEP_L - 1;
	coll->bad_neg = -(STEP_L - 1);
	coll->bad_ceiling = LARA_DUCK_HEIGHT;
	coll->slopes_are_walls = 1;
	coll->slopes_are_pits = 1;
	coll->facing = lara.move_angle = item->pos.y_rot - 32768;

	GetCollisionInfo(
		coll,
		item->pos.x_pos,
		item->pos.y_pos,
		item->pos.z_pos,
		item->room_number,
		LARA_DUCK_HEIGHT);

	if (LaraDeflectEdgeDuck(item, coll))
	{
		item->current_anim_state = AS_ALL4S;
		item->goal_anim_state = AS_ALL4S;

		if (item->anim_number != ALL4S_A)
		{
			item->anim_number = ALL4S_A;
			item->frame_number = ALL4S_F;
		}
		return;
	}

	if (LaraFallen(item, coll))
	{
		lara.gun_status = LG_ARMLESS;
		return;
	}

	ShiftItem(item, coll);

	item->pos.y_pos += coll->mid_floor;

	lara.move_angle = item->pos.y_rot;
}

void lara_col_crawl2hang(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->anim_number == 302)
	{
		item->fallspeed = 512;
		item->pos.y_pos |= (STEP_L - 1);

		coll->bad_pos = NO_BAD_POS;
		coll->bad_neg = -STEPUP_HEIGHT;
		coll->bad_ceiling = BAD_JUMP_CEILING;
		coll->facing = lara.move_angle = item->pos.y_rot;

		GetCollisionInfo(
			coll,
			item->pos.x_pos,
			item->pos.y_pos,
			item->pos.z_pos,
			item->room_number,
			870);

		int32_t edge,
			   edge_catch = LaraTestEdgeCatch(item, coll, &edge);

		if (!edge_catch || (edge_catch < 0 && !LaraTestHangOnClimbWall(item, coll)))
			return;

		auto angle = item->pos.y_rot;

		if (angle >= 0 - HANG_ANGLE && angle <= 0 + HANG_ANGLE)					angle = 0;
		else if (angle >= 16384 - HANG_ANGLE && angle <= 16384 + HANG_ANGLE)	angle = 16384;
		else if (angle >= 32767 - HANG_ANGLE || angle <= -32767 + HANG_ANGLE)	angle = -32768;
		else if (angle >= -16384 - HANG_ANGLE && angle <= -16384 + HANG_ANGLE)	angle = -16384;

		if (angle & 0x3fff)
			return;

		if (TestHangSwingIn(item, angle))
		{
			lara.head_x_rot = lara.head_y_rot = 0;
			lara.torso_x_rot = lara.torso_y_rot = 0;
			item->anim_number = GRABLEDGEIN_A;
			item->frame_number = GRABLEDGEIN_F;
			item->current_anim_state = AS_HANG2;
			item->goal_anim_state = AS_HANG2;
		}
		else
		{
			item->anim_number = GRABLEDGE_A;
			item->frame_number = GRABLEDGE_F;
			item->current_anim_state = AS_HANG;
			item->goal_anim_state = AS_HANG;
		}

		auto bounds = GetBoundsAccurate(item);

		if (edge_catch > 0)
		{
			item->pos.y_pos += coll->front_floor - bounds[2];
			item->pos.x_pos += coll->shift.x;
			item->pos.z_pos += coll->shift.z;
		}
		else item->pos.y_pos = edge - bounds[2];

		item->pos.y_rot = angle;
		item->gravity_status = 1;
		item->speed = 2;
		item->fallspeed = 1;
		lara.gun_status = LG_HANDSBUSY;
	}
}

void lara_as_dash(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_RUN;
		return;
	}

	if (!lara.dash || !(input & IN_DASH) || lara.water_status == LARA_WADE)
	{
		item->goal_anim_state = AS_RUN;
		return;
	}

	--lara.dash;

	if ((input & IN_DUCK) && (lara.water_status != LARA_WADE) && ((lara.gun_status == LG_ARMLESS)
			|| (lara.gun_type == LG_UNARMED)
			|| (lara.gun_type == LG_PISTOLS)
			|| (lara.gun_type == LG_MAGNUMS)
			|| (lara.gun_type == LG_UZIS)
			|| (lara.gun_type == LG_FLARE)))
	{
		item->goal_anim_state = AS_DUCK;
		return;
	}

	if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_DASH_TURN_RATE;

		if (lara.turn_rate < -LARA_DASH_TURN_MAX)
			lara.turn_rate = -LARA_DASH_TURN_MAX;

		item->pos.z_rot -= LARA_DASH_LEAN_RATE;

		if (item->pos.z_rot < -LARA_DASH_LEAN_MAX)
			item->pos.z_rot = -LARA_DASH_LEAN_MAX;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_DASH_TURN_RATE;

		if (lara.turn_rate > LARA_DASH_TURN_MAX)
			lara.turn_rate = LARA_DASH_TURN_MAX;

		item->pos.z_rot += LARA_DASH_LEAN_RATE;

		if (item->pos.z_rot > LARA_DASH_LEAN_MAX)
			item->pos.z_rot = LARA_DASH_LEAN_MAX;
	}

	if ((input & IN_JUMP) && !item->gravity_status) item->goal_anim_state = AS_DASHDIVE;
	else if (input & IN_FORWARD)					item->goal_anim_state = ((input & IN_SLOW) ? AS_WALK : AS_DASH);
	else if (!(input & (IN_LEFT | IN_RIGHT)))		item->goal_anim_state = AS_STOP;
}

void lara_col_dash(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll) || TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.z_rot = 0;

		if (TestWall(item, 256, 0, -640))
		{
			item->current_anim_state = AS_SPLAT;
			item->anim_number = HITWALLLEFT_A;
			item->frame_number = HITWALLLEFT_F;

			return;
		}

		LaraCollideStop(item, coll);
	}

	if (LaraFallen(item, coll))
		return;

	if (coll->mid_floor >= -STEPUP_HEIGHT && coll->mid_floor < -STEP_L / 2)
	{
		if (item->frame_number >= 3 && item->frame_number <= 14)
		{
			item->anim_number = RUNSTEPUP_LEFT_A;
			item->frame_number = RUNSTEPUP_LEFT_F;
		}
		else
		{
			item->anim_number = RUNSTEPUP_RIGHT_A;
			item->frame_number = RUNSTEPUP_RIGHT_F;
		}
	}

	if (TestLaraSlide(item, coll))
		return;

	if (coll->mid_floor >= 50)
	{
		item->pos.y_pos += 50;
		return;
	}

	item->pos.y_pos += coll->mid_floor;
}

void lara_as_dashdive(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->goal_anim_state != AS_DEATH &&
		item->goal_anim_state != AS_STOP &&
		item->goal_anim_state != AS_RUN &&
		item->fallspeed > LARA_FASTFALL_SPEED)
	{
		item->goal_anim_state = AS_FASTFALL;
	}
}

void lara_col_dashdive(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = (item->speed >= 0 ? item->pos.y_rot : item->pos.y_rot - 0x8000);

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEP_L;
	coll->slopes_are_walls = 1;
	coll->bad_ceiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if (LaraFallen(item, coll))
		return;

	if (item->speed < 0)
		lara.move_angle = item->pos.y_rot;

	if (coll->mid_floor <= 0 && item->fallspeed > 0)
	{
		if (LaraLandedBad(item, coll))
			item->goal_anim_state = AS_DEATH;
		else
		{
			if (lara.water_status == LARA_WADE)
				item->goal_anim_state = AS_STOP;
			else item->goal_anim_state = ((input & IN_FORWARD && !(input & IN_SLOW)) ? AS_RUN : AS_STOP);
		}

		item->fallspeed = 0;
		item->pos.y_pos += coll->mid_floor;
		item->gravity_status = 0;
		item->speed = 0;

		AnimateLara(item);
	}

	ShiftItem(item, coll);

	item->pos.y_pos += coll->mid_floor;
}

void MonkeySwingFall(ITEM_INFO* item)
{
	item->goal_anim_state = AS_UPJUMP;
	item->current_anim_state = AS_UPJUMP;
	item->anim_number = STOPHANG_A;
	item->frame_number = STOPHANG_F;
	item->gravity_status = 1;
	item->speed = 2;
	item->fallspeed = 1;
	item->pos.y_pos += 256;

	lara.gun_status = LG_ARMLESS;
}

void MonkeySwingSnap(ITEM_INFO* item, COLL_INFO* coll)
{
	auto room_num = item->room_number;

	item->pos.y_pos = GetCeiling(
		GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_num),
		item->pos.x_pos,
		item->pos.y_pos,
		item->pos.z_pos) + (768 - 64);
}

int16_t GetDirOctant(int32_t rot)
{
	return (abs(rot) >= 8192 && abs(rot) <= 24576);
}

int16_t TestMonkeyLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = NO_BAD_NEG;
	coll->bad_ceiling = 0;
	coll->facing = lara.move_angle = item->pos.y_rot - 16384;
	coll->radius = LARA_RAD;
	coll->slopes_are_walls = 0;

	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, HANG_HEIGHT);

	if (abs(coll->mid_ceiling - coll->front_ceiling) > 50)
		return 0;

	if (coll->coll_type)
	{
		auto oct = GetDirOctant(item->pos.y_rot);

		if ((oct == 0 && coll->coll_type == COLL_FRONT) ||
			(oct == 0 && coll->coll_type == COLL_LEFT) ||
			(oct == 1 && coll->coll_type == COLL_RIGHT) ||
			(oct == 1 && coll->coll_type == COLL_LEFT))
			return 0;
	}

	return 1;
}

int16_t TestMonkeyRight(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->facing = lara.move_angle = item->pos.y_rot + 16384;
	coll->radius = LARA_RAD;
	coll->slopes_are_walls = 0;

	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, HANG_HEIGHT);

	if (abs(coll->mid_ceiling - coll->front_ceiling) > 50)
		return 0;

	if (coll->coll_type)
	{
		auto oct = GetDirOctant(item->pos.y_rot);

		if ((oct == 0 && coll->coll_type == COLL_FRONT) ||
			(oct == 1 && coll->coll_type == COLL_FRONT) ||
			(oct == 1 && coll->coll_type == COLL_RIGHT) ||
			(oct == 1 && coll->coll_type == COLL_LEFT))
			return 0;
	}

	return 1;
}

void lara_as_hang2(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	lara.torso_x_rot = lara.torso_y_rot = 0;

	if (lara.can_monkey_swing)
	{
		if (!(input & IN_ACTION) || item->hit_points <= 0)
			return MonkeySwingFall(item);

		camera.target_angle = CAM_A_HANG;
		camera.target_elevation = CAM_E_HANG;
	}
	else if (input & IN_LOOK)
		LookUpDown();
}

void lara_col_hang2(ITEM_INFO* item, COLL_INFO* coll)
{
	if (lara.can_monkey_swing)
	{
		coll->bad_pos = NO_BAD_POS;
		coll->bad_neg = NO_BAD_NEG;
		coll->bad_ceiling = 0;
		coll->facing = lara.move_angle = item->pos.y_rot;
		coll->radius = LARA_RAD;
		coll->slopes_are_walls = 0;

		GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, HANG_HEIGHT);

		if ((input & IN_FORWARD) && coll->coll_type != COLL_FRONT && abs(coll->mid_ceiling - coll->front_ceiling) < 50) item->goal_anim_state = AS_MONKEYSWING;
		else if ((input & IN_STEPL) && TestMonkeyLeft(item, coll))														item->goal_anim_state = AS_MONKEYL;
		else if ((input & IN_STEPR) && TestMonkeyRight(item, coll))														item->goal_anim_state = AS_MONKEYR;
		else if (input & IN_LEFT)																						item->goal_anim_state = AS_HANGTURNL;
		else if (input & IN_RIGHT)																						item->goal_anim_state = AS_HANGTURNR;

		MonkeySwingSnap(item, coll);
	}
	else
	{
		LaraHangTest(item, coll);

		if (item->goal_anim_state == AS_HANG2)
		{
			if (input & IN_FORWARD)
			{
				if (coll->front_floor > -850 &&
					coll->front_floor < -650 &&
					coll->front_floor - coll->front_ceiling >= 0 &&
					coll->left_floor2 - coll->left_ceiling2 >= 0 &&
					coll->right_floor2 - coll->right_ceiling2 >= 0 &&
					!coll->hit_static)
				{
					item->goal_anim_state = ((input & IN_SLOW) ? AS_GYMNAST : AS_NULL);
					return;
				}
			}

			if ((input & (IN_DUCK | IN_FORWARD)) &&
				coll->front_floor > -850 &&
				coll->front_floor < -650 &&
				coll->front_floor - coll->front_ceiling >= -STEP_L &&
				coll->left_floor2 - coll->left_ceiling2 >= -STEP_L &&
				coll->right_floor2 - coll->right_ceiling2 >= -STEP_L &&
				!coll->hit_static)
			{
				item->goal_anim_state = AS_HANG2DUCK;
				item->required_anim_state = AS_DUCK;
			}
			else if (input & IN_LEFT || input & IN_STEPL)  item->goal_anim_state = AS_HANGLEFT;
			else if (input & IN_RIGHT || input & IN_STEPR) item->goal_anim_state = AS_HANGRIGHT;
		}
	}
}

void lara_as_hang2duck(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_monkeyswing(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_HANG2;
		return;
	}

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	lara.torso_x_rot = lara.torso_y_rot = 0;

	item->goal_anim_state = ((input & IN_FORWARD) ? AS_MONKEYSWING : AS_HANG2);

	if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_JUMP_TURN)
			lara.turn_rate = -LARA_JUMP_TURN;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_JUMP_TURN)
			lara.turn_rate = LARA_JUMP_TURN;
	}
}

void lara_col_monkeyswing(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(input & IN_ACTION) || !(lara.can_monkey_swing))
		return MonkeySwingFall(item);

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = NO_BAD_NEG;
	coll->bad_ceiling = 0;
	coll->facing = lara.move_angle = item->pos.y_rot;
	coll->radius = LARA_RAD;
	coll->slopes_are_walls = 0;

	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, HANG_HEIGHT);

	if (coll->coll_type == COLL_FRONT || abs(coll->mid_ceiling - coll->front_ceiling) > 50)
	{
		item->current_anim_state = AS_HANG2;
		item->goal_anim_state = AS_HANG2;
		item->anim_number = MONKEYHANG_A;
		item->frame_number = MONKEYHANG_F;

		return;
	}

	camera.target_elevation = 10 * ONE_DEGREE;

	MonkeySwingSnap(item, coll);
}

void lara_as_monkeyl(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_HANG2;
		return;
	}

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	lara.torso_x_rot = lara.torso_y_rot = 0;

	item->goal_anim_state = ((input & IN_STEPL) ? AS_MONKEYL : AS_HANG2);

	camera.target_elevation = 10 * ONE_DEGREE;
}

void lara_col_monkeyl(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(input & IN_ACTION) || !(lara.can_monkey_swing))
		return MonkeySwingFall(item);

	if (!TestMonkeyLeft(item, coll))
	{
		item->current_anim_state = AS_HANG2;
		item->goal_anim_state = AS_HANG2;
		item->anim_number = MONKEYHANG_A;
		item->frame_number = MONKEYHANG_F;
	}
	else MonkeySwingSnap(item, coll);
}

void lara_as_monkeyr(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_HANG2;
		return;
	}

	lara.torso_x_rot = lara.torso_y_rot = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	item->goal_anim_state = ((input & IN_STEPR) ? AS_MONKEYR : AS_HANG2);

	camera.target_elevation = 10 * ONE_DEGREE;
}

void lara_col_monkeyr(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(input & IN_ACTION) || !(lara.can_monkey_swing))
		return MonkeySwingFall(item);

	if (!TestMonkeyRight(item, coll))
	{
		item->current_anim_state = AS_HANG2;
		item->goal_anim_state = AS_HANG2;
		item->anim_number = MONKEYHANG_A;
		item->frame_number = MONKEYHANG_F;
	}
	else MonkeySwingSnap(item, coll);
}

void lara_as_hangturnl(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_HANG2;
		return;
	}

	camera.target_elevation = 10 * ONE_DEGREE;
	lara.torso_x_rot = lara.torso_y_rot = 0;

	item->pos.y_rot -= (ONE_DEGREE + (ONE_DEGREE / 2));

	if (!(input & IN_LEFT))
		item->goal_anim_state = AS_HANG2;
}

void lara_col_hangturnlr(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(input & IN_ACTION) || !lara.can_monkey_swing)
		return MonkeySwingFall(item);

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->facing = lara.move_angle = item->pos.y_rot;
	coll->radius = LARA_RAD;
	coll->slopes_are_walls = 1;

	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, HANG_HEIGHT);
	MonkeySwingSnap(item, coll);
}

void lara_as_hangturnr(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_HANG2;
		return;
	}

	lara.torso_x_rot = lara.torso_y_rot = 0;

	camera.target_elevation = 10 * ONE_DEGREE;

	item->pos.y_rot += (ONE_DEGREE + (ONE_DEGREE / 2));

	if (!(input & IN_RIGHT))
		item->goal_anim_state = AS_HANG2;
}

void lara_as_monkey180(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;
	item->goal_anim_state = AS_HANG2;
}

void lara_col_monkey180(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_monkeyswing(item, coll);
}

void lara_as_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	int16_t fheight = NO_HEIGHT,
		   rheight = NO_HEIGHT;

	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_DEATH;
		return;
	}

	if (item->anim_number != 226 && item->anim_number != 228)
		g_audio->stop_sound(3);

	if ((input & IN_ROLL) && lara.water_status != LARA_WADE)
	{
		item->anim_number = ROLL_A;
		item->frame_number = ROLL_F;
		item->current_anim_state = AS_ROLL;
		item->goal_anim_state = AS_STOP;

		return;
	}

	if ((input & IN_DUCK) &&
		lara.water_status != LARA_WADE &&
		item->current_anim_state == AS_STOP &&
		(lara.gun_status == LG_ARMLESS ||
		lara.gun_type == LG_UNARMED ||
		lara.gun_type == LG_PISTOLS ||
		lara.gun_type == LG_MAGNUMS ||
		lara.gun_type == LG_UZIS ||
		lara.gun_type == LG_FLARE))
	{
		item->goal_anim_state = AS_DUCK;
		return;
	}

	item->goal_anim_state = AS_STOP;

	if (input & IN_LOOK)
		LookUpDown();

	if (input & IN_FORWARD)   fheight = LaraFloorFront(item, item->pos.y_rot, LARA_RAD + 4);
	else if (input & IN_BACK) rheight = LaraFloorFront(item, item->pos.y_rot + 32768, LARA_RAD + 4);

	if (room[item->room_number].flags & SWAMP)
	{
		if (input & IN_LEFT)	   item->goal_anim_state = AS_TURN_L;
		else if (input & IN_RIGHT) item->goal_anim_state = AS_TURN_R;
	}
	else
	{
		if (input & IN_STEPL)
		{
			auto height = LaraFloorFront(item, item->pos.y_rot - 16384, LARA_RAD + 48),
				 ceiling = LaraCeilingFront(item, item->pos.y_rot - 16384, LARA_RAD + 48, LARA_HITE);

			if (height < 128 && height > -128 && height_type != BIG_SLOPE && ceiling <= 0)
				item->goal_anim_state = AS_STEPLEFT;
		}
		else if (input & IN_STEPR)
		{
			auto height = LaraFloorFront(item, item->pos.y_rot + 16384, LARA_RAD + 48),
				ceiling = LaraCeilingFront(item, item->pos.y_rot + 16384, LARA_RAD + 48, LARA_HITE);

			if (height < 128 && height > -128 && height_type != BIG_SLOPE && ceiling <= 0)
				item->goal_anim_state = AS_STEPRIGHT;
		}
		else if (input & IN_LEFT)  item->goal_anim_state = AS_TURN_L;
		else if (input & IN_RIGHT) item->goal_anim_state = AS_TURN_R;
	}

	if (lara.water_status == LARA_WADE)
	{
		if ((input & IN_JUMP) && !(room[item->room_number].flags & SWAMP))
			item->goal_anim_state = AS_COMPRESS;

		if (input & IN_FORWARD)
		{
			int wade = 0;

			if (room[item->room_number].flags & SWAMP)
			{
				if (fheight > -(STEPUP_HEIGHT - 1))
				{
					lara_as_wade(item, coll);

					wade = 1;
				}
			}

			else
			{
				if (fheight < STEPUP_HEIGHT - 1 && fheight > -STEPUP_HEIGHT + 1)
				{
					lara_as_wade(item, coll);

					wade = 1;
				}
			}

			if (!wade)
			{
				lara.move_angle = item->pos.y_rot;

				coll->bad_pos = NO_BAD_POS;
				coll->bad_neg = -STEPUP_HEIGHT;
				coll->bad_ceiling = 0;
				coll->slopes_are_walls = 1;
				coll->radius = LARA_RAD + 2;

				GetLaraCollisionInfo(item, coll);

				if (TestLaraVault(item, coll))
					return;

				coll->radius = LARA_RAD;
			}
		}
		else if (input & IN_BACK)
		{
			if (rheight < STEPUP_HEIGHT - 1 && rheight > -STEPUP_HEIGHT + 1)
				lara_as_back(item, coll);
		}
	}
	else
	{
		if (input & IN_JUMP)
			item->goal_anim_state = AS_COMPRESS;
		else if (input & IN_FORWARD)
		{
			auto height = LaraFloorFront(item, item->pos.y_rot, LARA_RAD + 4),
				 ceiling = LaraCeilingFront(item, item->pos.y_rot, LARA_RAD + 4, LARA_HITE);

			if ((height_type == BIG_SLOPE) && height < 0)
			{
				item->goal_anim_state = AS_STOP;
				return;
			}

			if (ceiling > 0)
			{
				item->goal_anim_state = AS_STOP;
				return;
			}

			if (input & IN_SLOW)
				lara_as_walk(item, coll);
			else lara_as_run(item, coll);
		}

		else if (input & IN_BACK)
		{
			if (input & IN_SLOW)
			{
				if (rheight < STEPUP_HEIGHT - 1 && rheight > -STEPUP_HEIGHT + 1 && height_type != BIG_SLOPE)
					lara_as_back(item, coll);
			}
			else if (rheight > -(STEPUP_HEIGHT - 1))
				item->goal_anim_state = AS_FASTBACK;
		}
	}
}

void lara_col_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	item->gravity_status = item->fallspeed = 0;

	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = STEPUP_HEIGHT;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_pits = 1;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll) || LaraFallen(item, coll) || TestLaraSlide(item, coll))
		return;

	ShiftItem(item, coll);

	if (!(room[item->room_number].flags & SWAMP) || coll->mid_floor < 0) item->pos.y_pos += coll->mid_floor;
	else if ((room[item->room_number].flags & SWAMP) && coll->mid_floor) item->pos.y_pos += SWAMP_GRAVITY;
}

int LaraTestHangJumpUp(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || coll->hit_static)
		return 0;

	if (lara.can_monkey_swing && coll->coll_type == COLL_TOP)
	{
		item->goal_anim_state = AS_HANG2;
		item->current_anim_state = AS_HANG2;
		item->anim_number = UPJUMPGRAB_A;
		item->frame_number = UPJUMPGRAB_F;
		item->gravity_status = 0;
		item->speed = 0;
		item->fallspeed = 0;
		lara.gun_status = LG_HANDSBUSY;

		MonkeySwingSnap(item, coll);

		return 1;
	}

	int32_t edge,
		   edge_catch = LaraTestEdgeCatch(item, coll, &edge);

	if (coll->coll_type != COLL_FRONT || coll->mid_ceiling > -STEPUP_HEIGHT || !edge_catch)
		return 0;

	if (edge_catch < 0 && !LaraTestHangOnClimbWall(item, coll))
		return 0;

	auto angle = item->pos.y_rot;

	if (angle >= 0 - HANG_ANGLE && angle <= 0 + HANG_ANGLE)					angle = 0;
	else if (angle >= 16384 - HANG_ANGLE && angle <= 16384 + HANG_ANGLE)	angle = 16384;
	else if (angle >= 32767 - HANG_ANGLE || angle <= -32767 + HANG_ANGLE)	angle = -32768;
	else if (angle >= -16384 - HANG_ANGLE && angle <= -16384 + HANG_ANGLE)	angle = -16384;

	if (angle & 0x3fff)
		return 0;

	if (TestHangSwingIn(item, angle))
	{
		item->anim_number = UPJUMPGRAB_A;
		item->frame_number = UPJUMPGRAB_F;
		item->goal_anim_state = AS_HANG2;
		item->current_anim_state = AS_HANG2;
	}
	else
	{
		item->goal_anim_state = AS_HANG;
		item->current_anim_state = AS_HANG;
		item->anim_number = STARTHANG_A;
		item->frame_number = STARTHANG_F;
	}

	auto bounds = GetBoundsAccurate(item);

	if (edge_catch > 0)
		item->pos.y_pos += coll->front_floor - bounds[2];
	else item->pos.y_pos = edge - bounds[2];

	item->pos.x_pos += coll->shift.x;
	item->pos.z_pos += coll->shift.z;
	item->pos.y_rot = angle;
	item->gravity_status = 0;
	item->speed = 0;
	item->fallspeed = 0;

	lara.gun_status = LG_HANDSBUSY;
	lara.torso_x_rot = lara.torso_y_rot = 0;

	return 1;
}

void lara_as_upjump(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->fallspeed > LARA_FASTFALL_SPEED)
		item->goal_anim_state = AS_FASTFALL;
}

void lara_col_upjump(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = BAD_JUMP_CEILING;
	coll->facing = (item->speed < 0) ? lara.move_angle - 32768 : lara.move_angle;
	coll->hit_ceiling = 0;

	GetCollisionInfo(
		coll,
		item->pos.x_pos,
		item->pos.y_pos,
		item->pos.z_pos,
		item->room_number,
		870);

	if (LaraTestHangJumpUp(item, coll))
		return;

	ShiftItem(item, coll);

	if (coll->coll_type == COLL_CLAMP || coll->coll_type == COLL_TOP || coll->coll_type == COLL_TOPFRONT || coll->hit_ceiling)
		item->fallspeed = 1;

	if (coll->coll_type)
		item->speed = (item->speed > 0) ? 2 : -2;
	else if (item->fallspeed < -70)
	{
		if ((input & IN_FORWARD) && item->speed < 5)	++item->speed;
		else if ((input & IN_BACK) && item->speed > -5) item->speed -= 2;
	}

	if (item->fallspeed > 0)
	{
		if (coll->mid_floor <= 0)
		{
			item->goal_anim_state = (LaraLandedBad(item, coll) ? AS_DEATH : AS_STOP);
			item->fallspeed = 0;
			item->pos.y_pos += coll->mid_floor;
			item->gravity_status = 0;
		}
	}
}

void lara_as_forwardjump(ITEM_INFO* item, COLL_INFO* coll)
{
	if ((item->goal_anim_state == AS_SWANDIVE) || (item->goal_anim_state == AS_REACH))
		item->goal_anim_state = AS_FORWARDJUMP;

	if ((item->goal_anim_state != AS_DEATH) && (item->goal_anim_state != AS_STOP) && (item->goal_anim_state != AS_RUN))
	{
		if ((input & IN_ACTION) && lara.gun_status == LG_ARMLESS) item->goal_anim_state = AS_REACH;
		if ((input & IN_ROLL) || (input & IN_BACK))				  item->goal_anim_state = AS_TWIST;
		if ((input & IN_SLOW) && lara.gun_status == LG_ARMLESS)   item->goal_anim_state = AS_SWANDIVE;
		if (item->fallspeed > LARA_FASTFALL_SPEED)				  item->goal_anim_state = AS_FASTFALL;
	}

	if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_JUMP_TURN)
			lara.turn_rate = -LARA_JUMP_TURN;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_JUMP_TURN)
			lara.turn_rate = LARA_JUMP_TURN;
	}
}

void lara_as_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_SLOW_TURN)
			lara.turn_rate = -LARA_SLOW_TURN;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_SLOW_TURN)
			lara.turn_rate = LARA_SLOW_TURN;
	}
	if (input & IN_FORWARD)
	{
		if (lara.water_status == LARA_WADE)
			item->goal_anim_state = AS_WADE;
		else item->goal_anim_state = (!(input & IN_SLOW) ? AS_RUN : AS_WALK);
	}
	else item->goal_anim_state = AS_STOP;
}

void lara_as_run(ITEM_INFO* item, COLL_INFO* coll)
{
	static bool jump_ok = true;

	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_DEATH;
		return;
	}

	if (input & IN_ROLL)
	{
		item->anim_number = ROLL_A;
		item->frame_number = ROLL_F;
		item->current_anim_state = AS_ROLL;
		item->goal_anim_state = AS_STOP;

		return;
	}

	if (input & IN_DASH)
	{
		item->goal_anim_state = AS_DASH;
		return;
	}

	if ((input & IN_DUCK) && lara.water_status != LARA_WADE)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_FAST_TURN)
			lara.turn_rate = -LARA_FAST_TURN;

		item->pos.z_rot -= LARA_LEAN_RATE;

		if (item->pos.z_rot < -LARA_LEAN_MAX)
			item->pos.z_rot = -LARA_LEAN_MAX;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_FAST_TURN)
			lara.turn_rate = LARA_FAST_TURN;

		item->pos.z_rot += LARA_LEAN_RATE;

		if (item->pos.z_rot > LARA_LEAN_MAX)
			item->pos.z_rot = LARA_LEAN_MAX;
	}

	if (item->anim_number == 6)
		jump_ok = false;
	else if (item->anim_number == 0)
	{
		if (item->frame_number == 4)
			jump_ok = true;
	}
	else jump_ok = true;

	if (input & IN_JUMP && jump_ok && !item->gravity_status)
		item->goal_anim_state = AS_FORWARDJUMP;
	else if (input & IN_FORWARD)
	{
		if (lara.water_status == LARA_WADE)
			item->goal_anim_state = AS_WADE;
		else item->goal_anim_state = ((input & IN_SLOW) ? AS_WALK : AS_RUN);
	}
	else item->goal_anim_state = AS_STOP;
}

void lara_as_pose(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_fastback(ITEM_INFO* item, COLL_INFO* coll)
{
	item->goal_anim_state = AS_STOP;

	if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_MED_TURN)
			lara.turn_rate = -LARA_MED_TURN;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_MED_TURN)
			lara.turn_rate = LARA_MED_TURN;
	}
}

void lara_as_turn_r(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	lara.turn_rate += LARA_TURN_RATE;

	if (lara.gun_status == LG_READY && lara.water_status != LARA_WADE)
		item->goal_anim_state = AS_FASTTURN;
	else if (lara.turn_rate > LARA_SLOW_TURN)
	{
		if (!(input & IN_SLOW) && lara.water_status != LARA_WADE)
			item->goal_anim_state = AS_FASTTURN;
		else lara.turn_rate = LARA_SLOW_TURN;
	}

	if (input & IN_FORWARD)
	{
		if (lara.water_status == LARA_WADE)
			item->goal_anim_state = AS_WADE;
		else item->goal_anim_state = ((input & IN_SLOW) ? AS_WALK : AS_RUN);
	}
	else if (!(input & IN_RIGHT))
		item->goal_anim_state = AS_STOP;
}

void lara_as_turn_l(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	lara.turn_rate -= LARA_TURN_RATE;

	if (lara.gun_status == LG_READY && lara.water_status != LARA_WADE)
		item->goal_anim_state = AS_FASTTURN;

	else if (lara.turn_rate < -LARA_SLOW_TURN)
	{
		if (!(input & IN_SLOW) && lara.water_status != LARA_WADE)
			item->goal_anim_state = AS_FASTTURN;
		else lara.turn_rate = -LARA_SLOW_TURN;
	}

	if (input & IN_FORWARD)
	{
		if (lara.water_status == LARA_WADE)
			item->goal_anim_state = AS_WADE;
		else item->goal_anim_state = ((input & IN_SLOW) ? AS_WALK : AS_RUN);
	}
	else if (!(input & IN_LEFT))
		item->goal_anim_state = AS_STOP;
}

void lara_as_death(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;
}

void lara_as_fastfall(ITEM_INFO* item, COLL_INFO* coll)
{
	FASTFALL_FRICTION;

	if (item->fallspeed == DAMAGE_START + DAMAGE_LENGTH)
		g_audio->play_sound(30, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
}

void lara_void_func(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	if (input & IN_LOOK)
		LookUpDown();

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = CAM_A_HANG;
	camera.target_elevation = CAM_E_HANG;

	if (input & (IN_LEFT | IN_STEPL))		item->goal_anim_state = AS_HANGLEFT;
	else if (input & (IN_RIGHT | IN_STEPR)) item->goal_anim_state = AS_HANGRIGHT;
}

void lara_as_reach(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_angle = 85 * ONE_DEGREE;

	if (item->fallspeed > LARA_FASTFALL_SPEED)
		item->goal_anim_state = AS_FASTFALL;
}

void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;
}

void lara_as_land(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_compress(ITEM_INFO* item, COLL_INFO* coll)
{
	if (lara.water_status != LARA_WADE)
	{
		if ((input & IN_FORWARD) && LaraFloorFront(item, item->pos.y_rot, 256) >= -STEPUP_HEIGHT)
		{
			item->goal_anim_state = AS_FORWARDJUMP;
			lara.move_angle = item->pos.y_rot;
		}
		else if ((input & IN_LEFT) && LaraFloorFront(item, (int16_t)(item->pos.y_rot - 16384), 256) >= -STEPUP_HEIGHT)
		{
			item->goal_anim_state = AS_LEFTJUMP;
			lara.move_angle = item->pos.y_rot - 16384;
		}
		else if ((input & IN_RIGHT) && LaraFloorFront(item, (int16_t)(item->pos.y_rot + 16384), 256) >= -STEPUP_HEIGHT)
		{
			item->goal_anim_state = AS_RIGHTJUMP;
			lara.move_angle = item->pos.y_rot + 16384;
		}
		else if ((input & IN_BACK) && LaraFloorFront(item, (int16_t)(item->pos.y_rot - 32768), 256) >= -STEPUP_HEIGHT)
		{
			item->goal_anim_state = AS_BACKJUMP;
			lara.move_angle = item->pos.y_rot - 32768;
		}
	}

	if (item->fallspeed > LARA_FASTFALL_SPEED)
		item->goal_anim_state = AS_FASTFALL;
}

void lara_as_back(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	item->goal_anim_state = (!(input & IN_BACK) || (!(input & IN_SLOW) && lara.water_status != LARA_WADE) ? AS_STOP : AS_BACK);

	if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_SLOW_TURN)
			lara.turn_rate = -LARA_SLOW_TURN;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_SLOW_TURN)
			lara.turn_rate = LARA_SLOW_TURN;
	}
}

void lara_as_null(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;
}

void lara_as_fastturn(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	if (lara.turn_rate < 0)
	{
		lara.turn_rate = -LARA_FAST_TURN;

		if (!(input & IN_LEFT))
			item->goal_anim_state = AS_STOP;
	}
	else
	{
		lara.turn_rate = LARA_FAST_TURN;

		if (!(input & IN_RIGHT))
			item->goal_anim_state = AS_STOP;
	}
}

void lara_as_stepright(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	if (!(input & IN_STEPR))
		item->goal_anim_state = AS_STOP;

	if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_SLOW_TURN)
			lara.turn_rate = -LARA_SLOW_TURN;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_SLOW_TURN)
			lara.turn_rate = LARA_SLOW_TURN;
	}
}

void lara_as_stepleft(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	if (!(input & IN_STEPL))
		item->goal_anim_state = AS_STOP;

	if (input & IN_LEFT)
	{
		lara.turn_rate -= LARA_TURN_RATE;

		if (lara.turn_rate < -LARA_SLOW_TURN)
			lara.turn_rate = -LARA_SLOW_TURN;
	}
	else if (input & IN_RIGHT)
	{
		lara.turn_rate += LARA_TURN_RATE;

		if (lara.turn_rate > LARA_SLOW_TURN)
			lara.turn_rate = LARA_SLOW_TURN;
	}
}

void lara_as_slide(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.flags = NO_CHUNKY;
	camera.target_elevation = -45 * ONE_DEGREE;

	if ((input & IN_JUMP) && !(input & IN_BACK))
		item->goal_anim_state = AS_FORWARDJUMP;
}

void lara_as_backjump(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_angle = ONE_DEGREE * 135;

	if (item->fallspeed > LARA_FASTFALL_SPEED)											  item->goal_anim_state = AS_FASTFALL;
	else if (item->goal_anim_state == AS_RUN)											  item->goal_anim_state = AS_STOP;
	else if ((input & IN_FORWARD || input & IN_ROLL) && item->goal_anim_state != AS_STOP) item->goal_anim_state = AS_TWIST;
}

void lara_as_rightjump(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	if (item->fallspeed > LARA_FASTFALL_SPEED)					  item->goal_anim_state = AS_FASTFALL;
	else if (input & IN_LEFT && item->goal_anim_state != AS_STOP) item->goal_anim_state = AS_TWIST;
}

void lara_as_leftjump(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	if (item->fallspeed > LARA_FASTFALL_SPEED)					   item->goal_anim_state = AS_FASTFALL;
	else if (input & IN_RIGHT && item->goal_anim_state != AS_STOP) item->goal_anim_state = AS_TWIST;
}

void lara_as_fallback(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->fallspeed > LARA_FASTFALL_SPEED)
		item->goal_anim_state = AS_FASTFALL;

	if ((input & IN_ACTION) && lara.gun_status == LG_ARMLESS)
		item->goal_anim_state = AS_REACH;
}

void lara_as_hangleft(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = CAM_A_HANG;
	camera.target_elevation = CAM_E_HANG;

	if (!(input & IN_LEFT) && !(input & IN_STEPL))
		item->goal_anim_state = AS_HANG;
}

void lara_as_hangright(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = CAM_A_HANG;
	camera.target_elevation = CAM_E_HANG;

	if (!(input & IN_RIGHT) && !(input & IN_STEPR))
		item->goal_anim_state = AS_HANG;
}

void lara_as_slideback(ITEM_INFO* item, COLL_INFO* coll)
{
	if ((input & IN_JUMP) && !(input & IN_FORWARD))
		item->goal_anim_state = AS_BACKJUMP;
}

void lara_as_pushblock(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.flags = FOLLOW_CENTRE;
	camera.target_angle = 35 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
}

void lara_as_pullblock(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.flags = FOLLOW_CENTRE;
	camera.target_angle = 35 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
}

void lara_as_ppready(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = 75 * ONE_DEGREE;

	if (!(input & IN_ACTION))
		item->goal_anim_state = AS_STOP;
}

void lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = -130 * ONE_DEGREE;
	camera.target_elevation = -15 * ONE_DEGREE;
	camera.target_distance = WALL_L;
}

void lara_as_pickupflare(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = +130 * ONE_DEGREE;
	camera.target_elevation = -15 * ONE_DEGREE;
	camera.target_distance = WALL_L;

	if (item->frame_number == anims[item->anim_number].frame_end - 1)
		lara.gun_status = LG_ARMLESS;
}

void lara_as_switchon(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = 80 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
	camera.target_distance = WALL_L;
	camera.speed = 6;
}

void lara_as_switchoff(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = 80 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
	camera.target_distance = WALL_L;
	camera.speed = 6;
}

void lara_as_usekey(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = -80 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
	camera.target_distance = WALL_L;
}

void lara_as_usepuzzle(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.target_angle = -80 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
	camera.target_distance = WALL_L;
}

void lara_as_roll(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_roll2(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_special(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.flags = FOLLOW_CENTRE;
	camera.target_angle = 170 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
}

void lara_as_usemidas(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_diemidas(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_swandive(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 1;

	if (item->fallspeed > LARA_FASTFALL_SPEED && item->goal_anim_state != AS_DIVE)
		item->goal_anim_state = AS_FASTDIVE;
}

void lara_as_fastdive(ITEM_INFO* item, COLL_INFO* coll)
{
	if ((input & IN_ROLL) && item->goal_anim_state == AS_FASTDIVE)
		item->goal_anim_state = AS_TWIST;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 1;

	FASTFALL_FRICTION;
}

void lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;
}

void lara_as_waterout(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	camera.flags = FOLLOW_CENTRE;
}

void lara_as_laratest1(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_laratest2(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_laratest3(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_STOP;
		return;
	}

	camera.target_elevation = -22 * ONE_DEGREE;

	if (room[item->room_number].flags & SWAMP)
	{
		if (input & IN_LEFT)
		{
			lara.turn_rate -= LARA_TURN_RATE;

			if (lara.turn_rate < -LARA_FAST_TURN >> 1)
				lara.turn_rate = -LARA_FAST_TURN >> 1;

			item->pos.z_rot -= LARA_LEAN_RATE;

			if (item->pos.z_rot < -LARA_LEAN_MAX >> 1)
				item->pos.z_rot = -LARA_LEAN_MAX >> 1;
		}
		else if (input & IN_RIGHT)
		{
			lara.turn_rate += LARA_TURN_RATE;

			if (lara.turn_rate > LARA_FAST_TURN >> 1)
				lara.turn_rate = LARA_FAST_TURN >> 1;

			item->pos.z_rot += LARA_LEAN_RATE;

			if (item->pos.z_rot > LARA_LEAN_MAX >> 1)
				item->pos.z_rot = LARA_LEAN_MAX >> 1;
		}

		item->goal_anim_state = ((input & IN_FORWARD) ? AS_WADE : AS_STOP);
	}
	else
	{
		if (input & IN_LEFT)
		{
			lara.turn_rate -= LARA_TURN_RATE;

			if (lara.turn_rate < -LARA_FAST_TURN)
				lara.turn_rate = -LARA_FAST_TURN;

			item->pos.z_rot -= LARA_LEAN_RATE;

			if (item->pos.z_rot < -LARA_LEAN_MAX)
				item->pos.z_rot = -LARA_LEAN_MAX;
		}
		else if (input & IN_RIGHT)
		{
			lara.turn_rate += LARA_TURN_RATE;

			if (lara.turn_rate > LARA_FAST_TURN)
				lara.turn_rate = LARA_FAST_TURN;

			item->pos.z_rot += LARA_LEAN_RATE;

			if (item->pos.z_rot > LARA_LEAN_MAX)
				item->pos.z_rot = LARA_LEAN_MAX;
		}

		if (input & IN_FORWARD)
			item->goal_anim_state = (lara.water_status == LARA_ABOVEWATER ? AS_RUN : AS_WADE);
		else item->goal_anim_state = AS_STOP;
	}
}

void lara_as_twist(ITEM_INFO* item, COLL_INFO* coll) {}
void lara_as_kick(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_deathslide(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_angle = ONE_DEGREE * 70;

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	coll->trigger = trigger_index;

	if (!(input & IN_ACTION))
	{
		item->goal_anim_state = AS_FORWARDJUMP;

		AnimateLara(item);

		lara_item->gravity_status = 1;
		lara_item->speed = 100;
		lara_item->fallspeed = 40;

		lara.move_angle = item->pos.y_rot;
	}
}

void extra_as_breath(ITEM_INFO* item, COLL_INFO* coll)
{
	item->anim_number = BREATH_A;
	item->frame_number = BREATH_F;
	item->current_anim_state = item->goal_anim_state = AS_STOP;

	lara.gun_status = LG_ARMLESS;

	if (camera.type != TARGET_CAMERA)
		camera.type = CHASE_CAMERA;

	AlterFOV(g_window->get_fov());

	lara.extra_anim = 0;
}

void extra_as_plunger(ITEM_INFO* item, COLL_INFO* coll) {}
void extra_as_yetikill(ITEM_INFO* item, COLL_INFO* coll) {}

void extra_as_sharkkill(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_angle = ONE_DEGREE * 160;
	camera.target_distance = 3 * WALL_L;

	lara.hit_direction = -1;

	if (item->frame_number == anims[item->anim_number].frame_end)
	{
		int wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
		if (wh != NO_HEIGHT && wh < item->pos.y_pos - 100)
			item->pos.y_pos -= 5;
	}
}

void extra_as_airlock(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_angle = 80 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
}

void extra_as_gongbong(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_angle = -25 * ONE_DEGREE;
	camera.target_elevation = -20 * ONE_DEGREE;
	camera.target_distance = WALL_L * 3;
}

void extra_as_dinokill(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.flags = FOLLOW_CENTRE;
	camera.target_angle = 170 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;

	lara.hit_direction = -1;
}

void extra_as_pulldagger(ITEM_INFO* item, COLL_INFO* coll) {}

void extra_as_startanim(ITEM_INFO* item, COLL_INFO* coll)
{
	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	TestTriggers(trigger_index, 0);
}

void extra_as_starthouse(ITEM_INFO* item, COLL_INFO* coll) {}
void extra_as_finalanim(ITEM_INFO* item, COLL_INFO* coll) {}

void extra_as_trainkill(ITEM_INFO* item, COLL_INFO* col)
{
	lara.hit_direction = -1;

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	item->gravity_status = 0;
	item->hit_points = -1;
}

void extra_as_rapidsdrown(ITEM_INFO* item, COLL_INFO* coll)
{
	GetLaraCollisionInfo(item, coll);

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	int h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	item->pos.y_pos = h + 384;
	item->pos.y_rot += 1024;

	if ((wibble & 3) == 0)
		TriggerWaterfallMist(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, GetRandomControl() & 4095);
}

void lara_col_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	item->gravity_status = item->fallspeed = 0;
	coll->bad_pos = STEPUP_HEIGHT;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_walls = 1;
	coll->slopes_are_pits = 1;
	coll->lava_is_pit = 1;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll) || TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		if (item->frame_number >= 22 + 7 && item->frame_number <= 22 + 25)
		{
			item->anim_number = STOP_RIGHT_A;
			item->frame_number = STOP_RIGHT_F;
		}
		else if ((item->frame_number >= 22 && item->frame_number <= 22 + 6) ||
				(item->frame_number >= 22 + 26 && item->frame_number <= 22 + 35))
		{
			item->anim_number = STOP_LEFT_A;
			item->frame_number = STOP_LEFT_F;
		}
		else LaraCollideStop(item, coll);
	}

	if (LaraFallen(item, coll))
		return;

	if (coll->mid_floor > STEP_L / 2)
	{
		if (item->frame_number >= 28 && item->frame_number <= 45)
		{
			item->anim_number = WALKSTEPD_RIGHT_A;
			item->frame_number = WALKSTEPD_RIGHT_F;
		}
		else
		{
			item->anim_number = WALKSTEPD_LEFT_A;
			item->frame_number = WALKSTEPD_LEFT_F;
		}
	}
	if (coll->mid_floor >= -STEPUP_HEIGHT && coll->mid_floor < -STEP_L / 2)
	{
		if (item->frame_number >= 27 && item->frame_number <= 44)
		{
			item->anim_number = WALKSTEPUP_RIGHT_A;
			item->frame_number = WALKSTEPUP_RIGHT_F;
		}
		else
		{
			item->anim_number = WALKSTEPUP_LEFT_A;
			item->frame_number = WALKSTEPUP_LEFT_F;
		}
	}

	if (TestLaraSlide(item, coll))
		return;

	item->pos.y_pos += coll->mid_floor;
}

void lara_col_run(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll) || TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.z_rot = 0;

		if (item->anim_number != STARTRUN_A && TestWall(item, 256, 0, -640))
		{
			item->current_anim_state = AS_SPLAT;

			if (item->frame_number >= 0 && item->frame_number <= 9)
			{
				item->anim_number = HITWALLLEFT_A;
				item->frame_number = HITWALLLEFT_F;
				return;
			}

			if (item->frame_number >= 10 && item->frame_number <= 21)
			{
				item->anim_number = HITWALLRIGHT_A;
				item->frame_number = HITWALLRIGHT_F;
				return;
			}
		}

		LaraCollideStop(item, coll);
	}

	if (LaraFallen(item, coll))
		return;

	if (coll->mid_floor >= -STEPUP_HEIGHT && coll->mid_floor < -STEP_L / 2)
	{
		if (item->frame_number >= 3 && item->frame_number <= 14)
		{
			item->anim_number = RUNSTEPUP_LEFT_A;
			item->frame_number = RUNSTEPUP_LEFT_F;
		}
		else
		{
			item->anim_number = RUNSTEPUP_RIGHT_A;
			item->frame_number = RUNSTEPUP_RIGHT_F;
		}
	}

	if (TestLaraSlide(item, coll))
		return;

	if (coll->mid_floor >= 50)
	{
		item->pos.y_pos += 50;
		return;
	}

	item->pos.y_pos += coll->mid_floor;
}

void lara_col_forwardjump(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = (item->speed >= 0 ? item->pos.y_rot : item->pos.y_rot - 0x8000);

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if (item->speed < 0)
		lara.move_angle = item->pos.y_rot;

	if (coll->mid_floor <= 0 && item->fallspeed > 0)
	{
		if (LaraLandedBad(item, coll))
			item->goal_anim_state = AS_DEATH;
		else
		{
			if (lara.water_status == LARA_WADE)
				item->goal_anim_state = AS_STOP;
			else item->goal_anim_state = (((input & IN_FORWARD) && !(input & IN_SLOW)) ? AS_RUN : AS_STOP);
		}

		item->fallspeed = 0;
		item->pos.y_pos += coll->mid_floor;
		item->gravity_status = 0;
		item->speed = 0;

		AnimateLara(item);
	}
}

void lara_col_pose(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stop(item, coll);
}

void lara_col_fastback(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot - 32768;

	item->gravity_status = item->fallspeed = 0;
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_pits = 1;
	coll->slopes_are_walls = 0;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll))
		return;

	if (coll->mid_floor > 200)
	{
		item->anim_number = FALLBACK_A;
		item->frame_number = FALLBACK_F;
		item->current_anim_state = AS_FALLBACK;
		item->goal_anim_state = AS_FALLBACK;
		item->fallspeed = 0;
		item->gravity_status = 1;

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	item->pos.y_pos += coll->mid_floor;
}

void lara_col_turn_r(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	item->gravity_status = item->fallspeed = 0;
	coll->bad_pos = STEPUP_HEIGHT;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_pits = 1;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);

	if (coll->mid_floor > 100 && !(room[item->room_number].flags & SWAMP))
	{
		item->anim_number = FALLDOWN_A;
		item->frame_number = FALLDOWN_F;
		item->current_anim_state = AS_FORWARDJUMP;
		item->goal_anim_state = AS_FORWARDJUMP;
		item->fallspeed = 0;
		item->gravity_status = 1;

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (!(room[item->room_number].flags & SWAMP) || coll->mid_floor < 0) item->pos.y_pos += coll->mid_floor;
	else if ((room[item->room_number].flags & SWAMP) && coll->mid_floor) item->pos.y_pos += SWAMP_GRAVITY;
}

void lara_col_turn_l(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_turn_r(item, coll);
}

void lara_col_death(ITEM_INFO* item, COLL_INFO* coll)
{
	g_audio->stop_sound(30);

	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = STEPUP_HEIGHT;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->radius = LARA_RAD * 4;

	GetLaraCollisionInfo(item, coll);
	ShiftItem(item, coll);

	item->pos.y_pos += coll->mid_floor;
	item->hit_points = -1;

	lara.air = -1;
}

void lara_col_fastfall(ITEM_INFO* item, COLL_INFO* coll)
{
	item->gravity_status = 1;
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
	LaraSlideEdgeJump(item, coll);

	if (coll->mid_floor <= 0)
	{
		if (LaraLandedBad(item, coll))
			item->goal_anim_state = AS_DEATH;
		else
		{
			item->goal_anim_state = AS_STOP;
			item->current_anim_state = AS_STOP;
			item->frame_number = LANDFAR_F;
			item->anim_number = LANDFAR_A;
		}

		g_audio->stop_sound(30);

		item->pos.y_pos += coll->mid_floor;
		item->gravity_status = 0;
		item->fallspeed = 0;
	}
}

void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll)
{
	item->gravity_status = item->fallspeed = 0;

	lara.move_angle = item->pos.y_rot;

	LaraHangTest(item, coll);

	if (item->goal_anim_state == AS_HANG)
	{
		if (input & IN_FORWARD)
		{
			if (coll->front_floor > -850 &&
				coll->front_floor < -650 &&
				coll->front_floor - coll->front_ceiling >= 0 &&
				coll->left_floor2 - coll->left_ceiling2 >= 0 &&
				coll->right_floor2 - coll->right_ceiling2 >= 0 &&
				!coll->hit_static)
			{
				item->goal_anim_state = ((input & IN_SLOW) ? AS_GYMNAST : AS_NULL);
				return;
			}
			else if (lara.climb_status &&
					 item->anim_number == HANG_A &&
					 item->frame_number == HANG_F &&
					 coll->mid_ceiling <= -256)
			{
				item->goal_anim_state = AS_HANG;
				item->current_anim_state = AS_HANG;
				item->anim_number = HANGUP_A;
				item->frame_number = HANGUP_F;
				return;
			}
		}

		if ((input & (IN_DUCK | IN_FORWARD)) &&
			coll->front_floor > -850 &&
			coll->front_floor < -650 &&
			coll->front_floor - coll->front_ceiling >= -STEP_L &&
			coll->left_floor2 - coll->left_ceiling2 >= -STEP_L &&
			coll->right_floor2 - coll->right_ceiling2 >= -STEP_L &&
			!coll->hit_static)
		{
			item->goal_anim_state = AS_HANG2DUCK;
			item->required_anim_state = AS_DUCK;
		}
		else if ((input & IN_BACK) && lara.climb_status && item->anim_number == HANG_A && item->frame_number == HANG_F)
		{
			item->goal_anim_state = AS_HANG;
			item->current_anim_state = AS_HANG;
			item->anim_number = HANGDOWN_A;
			item->frame_number = HANGDOWN_F;
		}
	}
}

void lara_col_reach(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	item->gravity_status = 1;
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = 0;
	coll->bad_ceiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);

	if (LaraTestHangJump(item, coll))
		return;

	LaraSlideEdgeJump(item, coll);
	GetLaraCollisionInfo(item, coll);
	ShiftItem(item, coll);

	if (item->fallspeed > 0)
	{
		if (coll->mid_floor <= 0)
		{
			item->goal_anim_state = (LaraLandedBad(item, coll) ? AS_DEATH : AS_STOP);
			item->fallspeed = 0;
			item->pos.y_pos += coll->mid_floor;
			item->gravity_status = 0;
		}
	}
}

void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = STEPUP_HEIGHT;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_pits = 1;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);
	ShiftItem(item, coll);

	if (coll->mid_floor > -STEP_L && coll->mid_floor < STEP_L)
		item->pos.y_pos += coll->mid_floor;
}

void lara_col_land(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stop(item, coll);
}

void lara_col_compress(ITEM_INFO* item, COLL_INFO* coll)
{
	item->gravity_status = item->fallspeed = 0;
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = NO_BAD_NEG;
	coll->bad_ceiling = 0;

	GetLaraCollisionInfo(item, coll);

	if (coll->mid_ceiling > -100)
	{
		item->goal_anim_state = AS_STOP;
		item->current_anim_state = AS_STOP;
		item->anim_number = STOP_A;
		item->frame_number = STOP_F;
		item->speed = 0;
		item->fallspeed = 0;
		item->gravity_status = 0;
		item->pos.x_pos = coll->old.x;
		item->pos.y_pos = coll->old.y;
		item->pos.z_pos = coll->old.z;
	}

	if (coll->mid_floor > -STEP_L && coll->mid_floor < STEP_L)
		item->pos.y_pos += coll->mid_floor;
}

void lara_col_back(ITEM_INFO* item, COLL_INFO* coll)
{
	item->gravity_status = item->fallspeed = 0;

	coll->bad_pos = (lara.water_status == LARA_WADE ? NO_BAD_POS : STEPUP_HEIGHT);
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_pits = 1;
	coll->slopes_are_walls = 1;

	lara.move_angle = item->pos.y_rot - 32768;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (LaraFallen(item, coll))
		return;

	if (coll->mid_floor > STEP_L / 2 && coll->mid_floor < (STEP_L * 3) / 2)
	{
		if (item->frame_number >= 964 && item->frame_number <= 993)
		{
			item->anim_number = BACKSTEPD_RIGHT_A;
			item->frame_number = BACKSTEPD_RIGHT_F;
		}
		else
		{
			item->anim_number = BACKSTEPD_LEFT_A;
			item->frame_number = BACKSTEPD_LEFT_F;
		}
	}
	if (TestLaraSlide(item, coll))
		return;

	if (!(room[item->room_number].flags & SWAMP) || coll->mid_floor < 0) item->pos.y_pos += coll->mid_floor;
	else if ((room[item->room_number].flags & SWAMP) && coll->mid_floor) item->pos.y_pos += SWAMP_GRAVITY;
}

void lara_col_null(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_fastturn(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stop(item, coll);
}

void lara_col_stepright(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = (item->current_anim_state == AS_STEPRIGHT ? item->pos.y_rot + 16384 : item->pos.y_rot - 16384);

	item->gravity_status = item->fallspeed = 0;

	coll->bad_pos = (lara.water_status == LARA_WADE ? NO_BAD_POS : STEP_L / 2);
	coll->bad_neg = -STEP_L / 2;
	coll->bad_ceiling = 0;
	coll->slopes_are_pits = 1;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (LaraFallen(item, coll) || TestLaraSlide(item, coll))
		return;

	item->pos.y_pos += coll->mid_floor;
}

void lara_col_stepleft(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stepright(item, coll);
}

void lara_col_slide(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;
	lara_slide_slope(item, coll);
}

void lara_col_backjump(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot - 32768;
	lara_col_jumper(item, coll);
}

void lara_col_rightjump(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot + 16384;
	lara_col_jumper(item, coll);
}

void lara_col_leftjump(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot - 16384;
	lara_col_jumper(item, coll);
}

void lara_col_fallback(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot - 32768;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if (coll->mid_floor <= 0 && item->fallspeed > 0)
	{
		item->goal_anim_state = (LaraLandedBad(item, coll) ? AS_DEATH : AS_STOP);
		item->fallspeed = 0;
		item->pos.y_pos += coll->mid_floor;
		item->gravity_status = 0;
	}
}

void lara_col_hangleft(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot - 16384;

	LaraHangTest(item, coll);

	lara.move_angle = item->pos.y_rot - 16384;

	if (!enable_engine_extended_features)
		return;

	if ((item->anim_number == HANG_A) && (item->frame_number == HANG_F))
	{
		if (input & (IN_LEFT | IN_STEPL))
		{
			if (int flag = LaraHangLeftCornerTest(item, coll))
			{
				if (flag > 0)
				{
					item->anim_number = EXTCORNERL_A;
					item->frame_number = EXTCORNERL_F;
					item->current_anim_state = item->goal_anim_state = AS_CORNEREXTL;
				}
				else
				{
					item->anim_number = INTCORNERL_A;
					item->frame_number = INTCORNERL_F;
					item->current_anim_state = item->goal_anim_state = AS_CORNERINTL;
				}
			}
		}
	}
}

void lara_col_hangright(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot + 16384;

	LaraHangTest(item, coll);

	lara.move_angle = item->pos.y_rot + 16384;

	if (!enable_engine_extended_features)
		return;

	if ((item->anim_number == HANG_A) && (item->frame_number == HANG_F))
	{
		if (input & (IN_RIGHT | IN_STEPR))
		{
			if (int flag = LaraHangRightCornerTest(item, coll))
			{
				if (flag > 0)
				{
					item->anim_number = EXTCORNERR_A;
					item->frame_number = GF(EXTCORNERR_A, 0);
					item->current_anim_state = item->goal_anim_state = AS_CORNEREXTR;
				}
				else
				{
					item->anim_number = INTCORNERR_A;
					item->frame_number = GF(INTCORNERR_A, 0);
					item->current_anim_state = item->goal_anim_state = AS_CORNERINTR;
				}
			}
		}
	}
}

void lara_col_slideback(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot - 32768;
	lara_slide_slope(item, coll);
}

void lara_col_pushblock(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_pullblock(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_ppready(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_pickup(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_switchon(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_switchoff(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_usekey(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_usepuzzle(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	item->gravity_status = item->fallspeed = 0;
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll) || TestLaraSlide(item, coll) || LaraFallen(item, coll))
		return;

	ShiftItem(item, coll);

	item->pos.y_pos += coll->mid_floor;
}

void lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot - 32768;

	item->gravity_status = item->fallspeed = 0;
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll) || TestLaraSlide(item, coll))
		return;

	if (coll->mid_floor > 200)
	{
		item->anim_number = FALLBACK_A;
		item->frame_number = FALLBACK_F;
		item->current_anim_state = AS_FALLBACK;
		item->goal_anim_state = AS_FALLBACK;
		item->fallspeed = 0;
		item->gravity_status = 1;

		return;
	}

	ShiftItem(item, coll);

	item->pos.y_pos += coll->mid_floor;
}

void lara_col_special(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_col_usemidas(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_diemidas(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_swandive(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if (coll->mid_floor <= 0 && item->fallspeed > 0)
	{
		item->goal_anim_state = AS_STOP;
		item->fallspeed = 0;
		item->pos.y_pos += coll->mid_floor;
		item->gravity_status = 0;
	}
}

void lara_col_fastdive(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if (coll->mid_floor <= 0 && item->fallspeed > 0)
	{
		item->goal_anim_state = (item->fallspeed > 133 ? AS_DEATH : AS_STOP);
		item->fallspeed = 0;
		item->pos.y_pos += coll->mid_floor;
		item->gravity_status = 0;
	}
}

void lara_col_gymnast(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_waterout(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_default_col(item, coll);
}

void lara_col_laratest1(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_col_laratest2(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_col_laratest3(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_col_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);

	if (LaraHitCeiling(item, coll) || TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.z_rot = 0;

		if ((coll->front_type == WALL || coll->front_type == SPLIT_TRI) && coll->front_floor < -(STEP_L * 5) / 2 && !(room[item->room_number].flags & SWAMP))
		{
			item->current_anim_state = AS_SPLAT;

			if (item->frame_number >= 0 && item->frame_number <= 9)
			{
				item->anim_number = HITWALLLEFT_A;
				item->frame_number = HITWALLLEFT_F;
				return;
			}

			if (item->frame_number >= 10 && item->frame_number <= 21)
			{
				item->anim_number = HITWALLRIGHT_A;
				item->frame_number = HITWALLRIGHT_F;
				return;
			}
		}

		LaraCollideStop(item, coll);
	}

	if (coll->mid_floor >= -STEPUP_HEIGHT && coll->mid_floor < -STEP_L / 2 && !(room[item->room_number].flags & SWAMP))
	{
		if (item->frame_number >= 3 && item->frame_number <= 14)
		{
			item->anim_number = RUNSTEPUP_LEFT_A;
			item->frame_number = RUNSTEPUP_LEFT_F;
		}
		else
		{
			item->anim_number = RUNSTEPUP_RIGHT_A;
			item->frame_number = RUNSTEPUP_RIGHT_F;
		}
	}

	if (coll->mid_floor >= 50 && !(room[item->room_number].flags & SWAMP))
	{
		item->pos.y_pos += 50;
		return;
	}

	if (!(room[item->room_number].flags & SWAMP) || coll->mid_floor < 0) item->pos.y_pos += coll->mid_floor;
	else if ((room[item->room_number].flags & SWAMP) && coll->mid_floor) item->pos.y_pos += SWAMP_GRAVITY;
}

void lara_col_twist(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_default_col(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.move_angle = item->pos.y_rot;

	coll->bad_pos = STEPUP_HEIGHT;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->slopes_are_pits = 1;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);
}

void lara_col_jumper(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0)
	{
		if (coll->mid_floor <= 0)
		{
			item->goal_anim_state = (LaraLandedBad(item, coll) ? AS_DEATH : AS_STOP);
			item->fallspeed = 0;
			item->pos.y_pos += coll->mid_floor;
			item->gravity_status = 0;
		}
	}
}

void lara_col_kick(ITEM_INFO* item, COLL_INFO* coll) {}
void lara_col_deathslide(ITEM_INFO* item, COLL_INFO* coll) {}

void lara_as_extcornerl(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_elevation = -(6 * WALL_L);

	SetCornerAnim(item, coll, 0x4000, item->anim_number == EXTCORNERL_A + 1);
}

void lara_as_intcornerl(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_elevation = -(6 * WALL_L);

	SetCornerAnim(item, coll, -0x4000, item->anim_number == INTCORNERL_A + 1);
}

void lara_as_extcornerr(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_elevation = -(6 * WALL_L);

	SetCornerAnim(item, coll, -0x4000, item->anim_number == EXTCORNERR_A + 1);
}

void lara_as_intcornerr(ITEM_INFO* item, COLL_INFO* coll)
{
	camera.target_elevation = -(6 * WALL_L);

	SetCornerAnim(item, coll, 0x4000, item->anim_number == INTCORNERR_A + 1);
}

void lara_as_controlled(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.look = 0;

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	if (item->frame_number == anims[item->anim_number].frame_end - 1)
	{
		lara.gun_status = LG_ARMLESS;

		if (UseForcedFixedCamera)
			UseForcedFixedCamera = 0;
	}
}

void lara_as_poleleft(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	if (((input & (IN_ACTION | IN_LEFT)) != (IN_ACTION | IN_LEFT)) || (input & (IN_FORWARD | IN_BACK)) || item->hit_points <= 0)
		item->goal_anim_state = AS_POLESTAT;
	else item->pos.y_rot += LARA_POLE_ROT;
}

void lara_as_poleright(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	if (((input & (IN_ACTION | IN_RIGHT)) != (IN_ACTION | IN_RIGHT)) || (input & (IN_FORWARD | IN_BACK)) || item->hit_points <= 0)
		item->goal_anim_state = AS_POLESTAT;
	else item->pos.y_rot -= LARA_POLE_ROT;
}

void lara_col_polestat(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_FASTFALL;
		return;
	}

	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	if (item->anim_number == POLESTAT_A)
	{
		coll->bad_pos = NO_BAD_POS;
		coll->bad_neg = -STEPUP_HEIGHT;
		coll->bad_ceiling = BAD_JUMP_CEILING;
		coll->facing = lara.move_angle = item->pos.y_rot;
		coll->radius = LARA_RAD;
		coll->slopes_are_walls = 1;

		GetLaraCollisionInfo(item, coll);

		if (!(input & IN_ACTION))
		{
			if (coll->mid_floor > 0)
			{
				item->goal_anim_state = AS_FASTFALL;
				item->pos.x_pos -= (phd_sin(item->pos.y_rot) * 64) >> W2V_SHIFT;
				item->pos.z_pos -= (phd_cos(item->pos.y_rot) * 64) >> W2V_SHIFT;
			}
			else item->goal_anim_state = AS_STOP;
		}
		else
		{
			item->goal_anim_state = AS_POLESTAT;

			if (input & IN_LEFT)
				item->goal_anim_state = AS_POLELEFT;
			else if (input & IN_RIGHT)
				item->goal_anim_state = AS_POLERIGHT;

			if (input & IN_LOOK)
				LookUpDown();

			if (input & IN_FORWARD)
			{
				auto room_num = item->room_number;
				auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_num);
				auto ceiling = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

				if (item->pos.y_pos - ceiling > 768 + 256)
					item->goal_anim_state = AS_POLEUP;
			}
			else if (input & IN_BACK)
			{
				if (coll->mid_floor > 0)
				{
					item->goal_anim_state = AS_POLEDOWN;
					item->item_flags[2] = 0;
				}
			}

			if (input & IN_JUMP)
				item->goal_anim_state = AS_BACKJUMP;
		}
	}
}

void lara_col_poleup(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	if (input & IN_LOOK)
		LookUpDown();

	if (((input & (IN_ACTION | IN_FORWARD)) != (IN_ACTION | IN_FORWARD)) || item->hit_points <= 0)
		item->goal_anim_state = AS_POLESTAT;

	auto room_num = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_num);
	auto ceiling = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if ((item->pos.y_pos - ceiling) < (768 + 256))
		item->goal_anim_state = AS_POLESTAT;
}

void lara_col_poledown(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;

	if (input & IN_LOOK)
		LookUpDown();

	if (((input & (IN_ACTION | IN_BACK)) != (IN_ACTION | IN_BACK)) || item->hit_points <= 0)
		item->goal_anim_state = AS_POLESTAT;

	coll->bad_pos = NO_BAD_POS;
	coll->bad_neg = -STEPUP_HEIGHT;
	coll->bad_ceiling = 0;
	coll->facing = lara.move_angle = item->pos.y_rot;
	coll->radius = LARA_RAD;
	coll->slopes_are_walls = 1;

	GetLaraCollisionInfo(item, coll);

	if (coll->mid_floor < 0)
	{
		FLOOR_INFO* floor;
		int16_t room_number;

		room_number = item->room_number;

		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
		item->floor = item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos - LARA_HITE, item->pos.z_pos);

		item->goal_anim_state = AS_POLESTAT;
		item->item_flags[2] = 0;
	}

	if (input & IN_LEFT)
		item->pos.y_rot += LARA_POLE_ROT;
	else if (input & IN_RIGHT)
		item->pos.y_rot -= LARA_POLE_ROT;

	item->item_flags[2] += (item->anim_number == POLESLIDEE_A ? LARA_POLE_DEC : LARA_POLE_ACC);
	
	//g_audio->play_sound(374, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

	if (item->item_flags[2] > LARA_POLE_VEL)
		item->item_flags[2] = LARA_POLE_VEL;
	else if (item->item_flags[2] < 0)
		item->item_flags[2] = 0;

	item->pos.y_pos += item->item_flags[2] >> 8;
}

void AddJointPos(ITEM_INFO* item, int mesh)
{
	auto room_num = item->room_number;

	GetFloor(
		(phd_mxptr[M03] >> W2V_SHIFT) + item->pos.x_pos,
		(phd_mxptr[M13] >> W2V_SHIFT) + item->pos.y_pos,
		(phd_mxptr[M23] >> W2V_SHIFT) + item->pos.z_pos,
		&room_num);

	GotJointPos[mesh] = 1;
}

void get_lara_bone_pos(ITEM_INFO* item, PHD_VECTOR* vec, int bone_id)
{
	int16_t* frmptr[2];

	int rate,
		frac = GetFrames(item, frmptr, &rate);

	if (lara.hit_direction < 0 && frac)
		return get_lara_bone_pos_int(item, vec, frmptr[0], frmptr[1], frac, rate, bone_id);

	auto object = &objects[item->object_number];

	int16_t* frame;

	if (lara.hit_direction >= 0)
	{
		int size;

		switch (lara.hit_direction)
		{
		default:
		case NORTH:
			frame = anims[SPAZ_FORWARD_A].frame_ptr;
			size = anims[SPAZ_FORWARD_A].interpolation >> 8;
			break;
		case EAST:
			frame = anims[SPAZ_RIGHT_A].frame_ptr;
			size = anims[SPAZ_RIGHT_A].interpolation >> 8;
			break;
		case SOUTH:
			frame = anims[SPAZ_BACK_A].frame_ptr;
			size = anims[SPAZ_BACK_A].interpolation >> 8;
		case WEST:
			frame = anims[SPAZ_LEFT_A].frame_ptr;
			size = anims[SPAZ_LEFT_A].interpolation >> 8;
			break;
		}

		frame += (int)(lara.hit_frame * size);
	}
	else frame = frmptr[0];

	phd_PushUnitMatrix();

	*(phd_mxptr + M03) = 0;
	*(phd_mxptr + M13) = 0;
	*(phd_mxptr + M23) = 0;

	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
	phd_PushMatrix();

	auto bone = bones + object->bone_index;
	auto rotation = frame + 9;

	phd_TranslateRel((int32_t)*(frame + 6), (int32_t)*(frame + 7), (int32_t)*(frame + 8));
	gar_RotYXZsuperpack(&rotation, 0);

	if (!GotJointPos[HIPS])
		AddJointPos(item, HIPS);

	if (bone_id == THIGH_L || bone_id == CALF_L || bone_id == FOOT_L)
	{
		phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
		gar_RotYXZsuperpack(&rotation, 0);

		if (!GotJointPos[THIGH_L])
			AddJointPos(item, THIGH_L);

		if (bone_id != THIGH_L)
		{
			phd_TranslateRel(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
			gar_RotYXZsuperpack(&rotation, 0);

			if (!GotJointPos[CALF_L])
				AddJointPos(item, CALF_L);

			if (bone_id != CALF_L)
			{
				phd_TranslateRel(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
				gar_RotYXZsuperpack(&rotation, 0);

				if (!GotJointPos[FOOT_L])
					AddJointPos(item, FOOT_L);
			}
		}
	}
	else if (bone_id == THIGH_R || bone_id == CALF_R || bone_id == FOOT_R)
	{
		phd_TranslateRel(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
		gar_RotYXZsuperpack(&rotation, 3);

		if (!GotJointPos[THIGH_R])
			AddJointPos(item, THIGH_R);

		if (bone_id != THIGH_R)
		{
			phd_TranslateRel(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
			gar_RotYXZsuperpack(&rotation, 0);

			if (!GotJointPos[CALF_R])
				AddJointPos(item, CALF_R);

			if (bone_id != CALF_R)
			{
				phd_TranslateRel(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
				gar_RotYXZsuperpack(&rotation, 0);

				if (!GotJointPos[FOOT_R])
					AddJointPos(item, FOOT_R);
			}
		}
	}
	else if (bone_id != HIPS)
	{
		phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));

		if (lara.weapon_item != NO_ITEM &&
			lara.gun_type == LG_M16 &&
			(items[lara.weapon_item].current_anim_state == 0 ||
			items[lara.weapon_item].current_anim_state == 2 ||
			items[lara.weapon_item].current_anim_state == 4))
		{
			rotation = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
			gar_RotYXZsuperpack(&rotation, 7);
		}
		else gar_RotYXZsuperpack(&rotation, 6);

		phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);

		if (!GotJointPos[TORSO])
			AddJointPos(item, TORSO);

		if (bone_id == HEAD)
		{
			phd_TranslateRel(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
			gar_RotYXZsuperpack(&rotation, 6);
			phd_RotYXZ(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);

			if (!GotJointPos[HEAD])
				AddJointPos(item, HEAD);
		}
		else if (bone_id != TORSO)
		{
			int fire_arms = LG_UNARMED;

			if (lara.gun_status == LG_READY || lara.gun_status == LG_SPECIAL || lara.gun_status == LG_DRAW || lara.gun_status == LG_UNDRAW)
				fire_arms = lara.gun_type;

			if (bone_id == HAND_L || bone_id == UARM_L || bone_id == LARM_L)
			{
				phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));

				switch (fire_arms)
				{
				case LG_UNARMED:
				case LG_FLARE:
				{
					if (lara.flare_control_left)
					{
						rotation = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;
						gar_RotYXZsuperpack(&rotation, 11);
					}
					else gar_RotYXZsuperpack(&rotation, 3);

					break;
				}
				case LG_PISTOLS:
				case LG_MAGNUMS:
				case LG_UZIS:
				{
					*(phd_mxptr + M00) = *(phd_mxptr + M00 - 12);
					*(phd_mxptr + M01) = *(phd_mxptr + M01 - 12);
					*(phd_mxptr + M02) = *(phd_mxptr + M02 - 12);
					*(phd_mxptr + M10) = *(phd_mxptr + M10 - 12);
					*(phd_mxptr + M11) = *(phd_mxptr + M11 - 12);
					*(phd_mxptr + M12) = *(phd_mxptr + M12 - 12);
					*(phd_mxptr + M20) = *(phd_mxptr + M20 - 12);
					*(phd_mxptr + M21) = *(phd_mxptr + M21 - 12);
					*(phd_mxptr + M22) = *(phd_mxptr + M22 - 12);

					phd_RotYXZ(lara.left_arm.y_rot, lara.left_arm.x_rot, lara.left_arm.z_rot);

					rotation = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;

					gar_RotYXZsuperpack(&rotation, 11);

					break;
				}
				case LG_SHOTGUN:
				case LG_HARPOON:
				case LG_ROCKET:
				case LG_GRENADE:
				case LG_M16:
				default:
					rotation = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
					gar_RotYXZsuperpack(&rotation, 11);
					break;
				}

				if (!GotJointPos[UARM_L])
					AddJointPos(item, UARM_L);

				if (bone_id != UARM_L)
				{
					phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
					gar_RotYXZsuperpack(&rotation, 0);

					if (!GotJointPos[LARM_L])
						AddJointPos(item, LARM_L);

					if (bone_id != LARM_L)
					{
						phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
						gar_RotYXZsuperpack(&rotation, 0);

						if (!GotJointPos[HAND_L])
							AddJointPos(item, HAND_L);
					}
				}
			}
			else if (bone_id == HAND_R || bone_id == UARM_R || bone_id == LARM_R)
			{
				phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));

				switch (fire_arms)
				{
				case LG_UNARMED:
				case LG_FLARE:
					gar_RotYXZsuperpack(&rotation, 0);
					break;
				case LG_PISTOLS:
				case LG_MAGNUMS:
				case LG_UZIS:
				{
					*(phd_mxptr + M00) = *(phd_mxptr + M00 - 12);
					*(phd_mxptr + M01) = *(phd_mxptr + M01 - 12);
					*(phd_mxptr + M02) = *(phd_mxptr + M02 - 12);
					*(phd_mxptr + M10) = *(phd_mxptr + M10 - 12);
					*(phd_mxptr + M11) = *(phd_mxptr + M11 - 12);
					*(phd_mxptr + M12) = *(phd_mxptr + M12 - 12);
					*(phd_mxptr + M20) = *(phd_mxptr + M20 - 12);
					*(phd_mxptr + M21) = *(phd_mxptr + M21 - 12);
					*(phd_mxptr + M22) = *(phd_mxptr + M22 - 12);

					phd_RotYXZ(lara.right_arm.y_rot, lara.right_arm.x_rot, lara.right_arm.z_rot);

					rotation = lara.right_arm.frame_base + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;

					gar_RotYXZsuperpack(&rotation, 8);

					break;
				}
				case LG_SHOTGUN:
				case LG_HARPOON:
				case LG_ROCKET:
				case LG_GRENADE:
				case LG_M16:
					rotation = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
					gar_RotYXZsuperpack(&rotation, 8);
					break;
				}

				if (!GotJointPos[UARM_R])
					AddJointPos(item, UARM_R);

				if (bone_id != UARM_R)
				{
					phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
					gar_RotYXZsuperpack(&rotation, 0);

					if (!GotJointPos[LARM_R])
						AddJointPos(item, LARM_R);

					if (bone_id != LARM_R)
					{
						phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
						gar_RotYXZsuperpack(&rotation, 0);

						if (!GotJointPos[HAND_R])
							AddJointPos(item, HAND_R);
					}
				}
			}
		}
	}

	phd_TranslateRel(vec->x, vec->y, vec->z);

	vec->x = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
	vec->y = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
	vec->z = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;

	phd_PopMatrix();
	phd_PopMatrix();
}

void get_lara_bone_pos_int(ITEM_INFO* item, PHD_VECTOR* vec, int16_t* frame1, int16_t* frame2, int frac, int rate, int bone_id)
{
	auto object = &objects[item->object_number];

	phd_PushUnitMatrix();

	*(phd_mxptr + M03) = 0;
	*(phd_mxptr + M13) = 0;
	*(phd_mxptr + M23) = 0;

	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	phd_PushMatrix();
	phd_PushMatrix();

	auto bone = bones + object->bone_index;

	auto rotation1 = frame1 + 9,
		 rotation2 = frame2 + 9;

	InitInterpolate(frac, rate);

	phd_TranslateRel_ID(
		(int32_t) * (frame1 + 6),
		(int32_t) * (frame1 + 7),
		(int32_t) * (frame1 + 8),
		(int32_t) * (frame2 + 6),
		(int32_t) * (frame2 + 7),
		(int32_t) * (frame2 + 8));

	gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

	if (!GotJointPos[HIPS])
		AddJointPos(item, HIPS);

	if (bone_id == THIGH_L || bone_id == CALF_L || bone_id == FOOT_L)
	{
		phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

		if (!GotJointPos[THIGH_L])
			AddJointPos(item, THIGH_L);

		if (bone_id != THIGH_L)
		{
			phd_TranslateRel_I(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
			gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

			if (!GotJointPos[CALF_L])
				AddJointPos(item, CALF_L);

			if (bone_id != CALF_L)
			{
				phd_TranslateRel_I(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
				gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

				if (!GotJointPos[FOOT_L])
					AddJointPos(item, FOOT_L);
			}
		}
	}
	else if (bone_id == THIGH_R || bone_id == CALF_R || bone_id == FOOT_R)
	{
		phd_TranslateRel_I(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 3);

		if (!GotJointPos[THIGH_R])
			AddJointPos(item, THIGH_R);

		if (bone_id != THIGH_R)
		{
			phd_TranslateRel_I(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
			gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

			if (!GotJointPos[CALF_R])
				AddJointPos(item, CALF_R);
			
			if (bone_id != CALF_R)
			{
				phd_TranslateRel_I(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
				gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

				if (!GotJointPos[FOOT_R])
					AddJointPos(item, FOOT_R);
			}
		}
	}
	else if (bone_id != HIPS)
	{
		phd_TranslateRel_I(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));

		if (lara.weapon_item != NO_ITEM &&
			lara.gun_type == LG_M16 &&
			(items[lara.weapon_item].current_anim_state == 0 ||
			items[lara.weapon_item].current_anim_state == 2 ||
			items[lara.weapon_item].current_anim_state == 4))
		{
			rotation1 = rotation2 = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
			gar_RotYXZsuperpack_I(&rotation1, &rotation2, 7);
		}
		else gar_RotYXZsuperpack_I(&rotation1, &rotation2, 6);

		phd_RotYXZ_I(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);

		if (!GotJointPos[TORSO])
			AddJointPos(item, TORSO);

		if (bone_id == HEAD)
		{
			phd_TranslateRel_I(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
			gar_RotYXZsuperpack_I(&rotation1, &rotation2, 6);
			phd_RotYXZ_I(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);

			if (!GotJointPos[HEAD])
				AddJointPos(item, HEAD);
		}
		else if (bone_id != TORSO)
		{
			int fire_arms = LG_UNARMED;

			if (lara.gun_status == LG_READY || lara.gun_status == LG_SPECIAL || lara.gun_status == LG_DRAW || lara.gun_status == LG_UNDRAW)
				fire_arms = lara.gun_type;

			if (bone_id == HAND_L || bone_id == UARM_L || bone_id == LARM_L)
			{
				phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));

				switch (fire_arms)
				{
				case LG_UNARMED:
				case LG_FLARE:
				{
					if (lara.flare_control_left)
					{
						rotation1 = rotation2 = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;
						gar_RotYXZsuperpack_I(&rotation1, &rotation2, 11);
					}
					else gar_RotYXZsuperpack_I(&rotation1, &rotation2, 3);

					if (!GotJointPos[UARM_L])
						AddJointPos(item, UARM_L);

					if (bone_id != UARM_L)
					{
						phd_TranslateRel_I(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
						gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

						if (!GotJointPos[LARM_L])
							AddJointPos(item, LARM_L);

						if (bone_id != LARM_L)
						{
							phd_TranslateRel_I(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
							gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

							if (!GotJointPos[HAND_L])
								AddJointPos(item, HAND_L);
						}
					}

					break;
				}
				case LG_PISTOLS:
				case LG_MAGNUMS:
				case LG_UZIS:
				{
					InterpolateArmMatrix();

					phd_RotYXZ(lara.left_arm.y_rot, lara.left_arm.x_rot, lara.left_arm.z_rot);

					rotation1 = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;

					gar_RotYXZsuperpack(&rotation1, 11);

					if (bone_id != UARM_L)
					{
						phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
						gar_RotYXZsuperpack(&rotation1, 0);

						if (bone_id != LARM_L)
						{
							phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
							gar_RotYXZsuperpack(&rotation1, 0);
						}
					}

					phd_TranslateRel(vec->x, vec->y, vec->z);

					vec->x = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
					vec->y = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
					vec->z = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;

					phd_PopMatrix();
					phd_PopMatrix();
					phd_PopMatrix();

					return;
				}
				case LG_SHOTGUN:
				case LG_HARPOON:
				case LG_ROCKET:
				case LG_GRENADE:
				case LG_M16:
				{
					rotation1 = rotation2 = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;

					gar_RotYXZsuperpack_I(&rotation1, &rotation2, 11);

					if (bone_id != UARM_L)
					{
						phd_TranslateRel_I(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
						gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

						if (bone_id != LARM_L)
						{
							phd_TranslateRel_I(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
							gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
						}
					}
				}
				}
			}
			else if (bone_id == HAND_R || bone_id == UARM_R || bone_id == LARM_R)
			{
				switch (fire_arms)
				{
				case LG_UNARMED:
				case LG_FLARE:
				{
					phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
					gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

					if (!GotJointPos[UARM_R])
						AddJointPos(item, UARM_R);

					if (bone_id != UARM_R)
					{
						phd_TranslateRel_I(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
						gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

						if (!GotJointPos[LARM_R])
							AddJointPos(item, LARM_R);

						if (bone_id != LARM_R)
						{
							phd_TranslateRel_I(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
							gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

							if (!GotJointPos[HAND_R])
								AddJointPos(item, HAND_R);
						}
					}

					break;
				}
				case LG_PISTOLS:
				case LG_MAGNUMS:
				case LG_UZIS:
				{
					phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));

					InterpolateArmMatrix();

					phd_RotYXZ(lara.right_arm.y_rot, lara.right_arm.x_rot, lara.right_arm.z_rot);

					rotation1 = lara.right_arm.frame_base + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
					gar_RotYXZsuperpack(&rotation1, 8);

					if (bone_id != UARM_R)
					{
						phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
						gar_RotYXZsuperpack(&rotation1, 0);

						if (bone_id != LARM_R)
						{
							phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
							gar_RotYXZsuperpack(&rotation1, 0);
						}
					}

					phd_TranslateRel(vec->x, vec->y, vec->z);

					vec->x = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
					vec->y = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
					vec->z = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;

					phd_PopMatrix();
					phd_PopMatrix();
					phd_PopMatrix();

					return;
				}
				case LG_SHOTGUN:
				case LG_HARPOON:
				case LG_ROCKET:
				case LG_GRENADE:
				case LG_M16:
				{
					phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));

					rotation1 = rotation2 = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;

					gar_RotYXZsuperpack_I(&rotation1, &rotation2, 8);

					if (bone_id != UARM_R)
					{
						phd_TranslateRel_I(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
						gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

						if (bone_id != LARM_R)
						{
							phd_TranslateRel_I(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
							gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
						}
					}
				}
				}
			}
		}
	}

	phd_TranslateRel_I(vec->x, vec->y, vec->z);
	InterpolateMatrix();

	vec->x = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
	vec->y = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
	vec->z = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;

	phd_PopMatrix();
	phd_PopMatrix();
	phd_PopMatrix();
}

void LookUpDown()
{
	camera.type = LOOK_CAMERA;

	if (input & IN_FORWARD)
	{
		input -= IN_FORWARD;

		if (lara.head_x_rot > MIN_HEAD_TILT)
			lara.head_x_rot -= HEAD_TURN;
	}
	else if (input & IN_BACK)
	{
		input -= IN_BACK;

		if (lara.head_x_rot < MAX_HEAD_TILT)
			lara.head_x_rot += HEAD_TURN;
	}

	if (lara.gun_status != LG_HANDSBUSY && lara.skidoo == NO_ITEM && lara.left_arm.lock == 0 && lara.right_arm.lock == 0)
		lara.torso_x_rot = lara.head_x_rot;
}

void LookLeftRight()
{
	camera.type = LOOK_CAMERA;

	if (input & IN_LEFT)
	{
		input -= IN_LEFT;

		if (lara.head_y_rot > -MAX_HEAD_ROTATION)
			lara.head_y_rot -= HEAD_TURN;
	}
	else if (input & IN_RIGHT)
	{
		input -= IN_RIGHT;

		if (lara.head_y_rot < MAX_HEAD_ROTATION)
			lara.head_y_rot += HEAD_TURN;
	}

	if (lara.gun_status != LG_HANDSBUSY && lara.skidoo == NO_ITEM && lara.left_arm.lock == 0 && lara.right_arm.lock == 0)
		lara.torso_y_rot = lara.head_y_rot;
}

void ResetLook()
{
	if (camera.type == LOOK_CAMERA)
		return;

	if ((lara.head_x_rot > -HEAD_TURN) && (lara.head_x_rot < HEAD_TURN))
		lara.head_x_rot = 0;
	else lara.head_x_rot -= lara.head_x_rot / 8;

	if ((lara.head_y_rot > -HEAD_TURN) && (lara.head_y_rot < HEAD_TURN))
		lara.head_y_rot = 0;
	else lara.head_y_rot -= lara.head_y_rot / 8;

	if (lara.gun_status != LG_HANDSBUSY && lara.skidoo == NO_ITEM)
	{
		lara.torso_x_rot = lara.head_x_rot;
		lara.torso_y_rot = lara.head_y_rot;
	}
}

void DeathSlideCollision(int16_t item_number, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	if (!(input & IN_ACTION) ||
		laraitem->gravity_status ||
		lara.gun_status != LG_ARMLESS ||
		laraitem->current_anim_state != AS_STOP)
		return;

	auto item = &items[item_number];

	if (item->status != NOT_ACTIVE)
		return;

	if (TestLaraPosition(DeathSlideBounds, item, laraitem))
	{
		AlignLaraPosition(&DeathSlidePosition, item, laraitem);

		lara.gun_status = LG_HANDSBUSY;

		laraitem->goal_anim_state = AS_DEATHSLIDE;

		do AnimateItem(laraitem);
		while (laraitem->current_anim_state != AS_NULL);

		if (!item->active)
			AddActiveItem(item_number);

		item->status = ACTIVE;
		item->flags |= ONESHOT;
	}
}

void ControlDeathSlide(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->status == ACTIVE)
	{
		if (!(item->flags & ONESHOT))
		{
			auto old = (GAME_VECTOR*)item->data;

			item->pos.x_pos = old->x;
			item->pos.y_pos = old->y;
			item->pos.z_pos = old->z;

			if (old->room_number != item->room_number)
				ItemNewRoom(item_number, old->room_number);

			item->status = NOT_ACTIVE;
			item->current_anim_state = item->goal_anim_state = DEATH_GRAB;
			item->anim_number = objects[item->object_number].anim_index;
			item->frame_number = anims[item->anim_number].frame_base;

			RemoveActiveItem(item_number);

			return;
		}

		if (item->current_anim_state == DEATH_GRAB)
			return AnimateItem(item);

		AnimateItem(item);

		if (item->fallspeed < DEATH_SPEED)
			item->fallspeed += DEATH_ACC;

		int c = phd_cos(item->pos.y_rot),
			s = phd_sin(item->pos.y_rot);

		item->pos.z_pos += item->fallspeed * c >> W2V_SHIFT;
		item->pos.x_pos += item->fallspeed * s >> W2V_SHIFT;
		item->pos.y_pos += item->fallspeed >> 2;

		auto room_number = item->room_number;

		GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (room_number != item->room_number)
			ItemNewRoom(item_number, room_number);

		bool lara_on_deathslide = (lara_item->current_anim_state == AS_DEATHSLIDE);

		if (lara_on_deathslide)
		{
			lara_item->pos.x_pos = item->pos.x_pos;
			lara_item->pos.y_pos = item->pos.y_pos;
			lara_item->pos.z_pos = item->pos.z_pos;
		}

		int z = item->pos.z_pos + (WALL_L * c >> W2V_SHIFT),
			x = item->pos.x_pos + (WALL_L * s >> W2V_SHIFT),
			y = item->pos.y_pos + (STEP_L >> 2);

		auto floor = GetFloor(x, y, z, &room_number);

		if (GetHeight(floor, x, y, z) <= y + STEP_L || GetCeiling(floor, x, y, z) >= y - STEP_L)
		{
			if (lara_on_deathslide)
			{
				lara_item->goal_anim_state = AS_FORWARDJUMP;

				AnimateLara(lara_item);

				lara_item->gravity_status = 1;
				lara_item->speed = item->fallspeed;
				lara_item->fallspeed = item->fallspeed >> 2;
			}

			g_audio->stop_sound(280);
			g_audio->play_sound(281, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			RemoveActiveItem(item_number);

			item->status = NOT_ACTIVE;
			item->flags -= ONESHOT;
		}
		else g_audio->play_sound(280, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}
}

int LaraLandedBad(ITEM_INFO* item, COLL_INFO* coll)
{
	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	int oy = item->pos.y_pos;

	item->floor = item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos - LARA_HITE, item->pos.z_pos);

	if (abs(oy - item->pos.y_pos) > 256)
		item->pos.y_pos = oy;

	TestTriggers(trigger_index, 0);

	item->pos.y_pos = oy;

	int landspeed = item->fallspeed - DAMAGE_START;

	if (landspeed <= 0)					return 0;
	else if (landspeed > DAMAGE_LENGTH) item->hit_points = -1;
	else								item->hit_points -= (landspeed * landspeed * LARA_HITPOINTS) / (DAMAGE_LENGTH * DAMAGE_LENGTH);

	return (item->hit_points <= 0);
}

int LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->coll_type == COLL_TOP || coll->coll_type == COLL_CLAMP)
	{
		item->pos.x_pos = coll->old.x;
		item->pos.y_pos = coll->old.y;
		item->pos.z_pos = coll->old.z;
		item->goal_anim_state = AS_STOP;
		item->current_anim_state = AS_STOP;
		item->frame_number = STOP_F;
		item->anim_number = STOP_A;
		item->speed = 0;
		item->fallspeed = 0;
		item->gravity_status = 0;

		return 1;
	}

	return 0;
}

int16_t LaraFloorFront(ITEM_INFO* item, PHD_ANGLE ang, int32_t dist)
{
	int x = item->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT),
		y = item->pos.y_pos - LARA_HITE,
		z = item->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT);

	auto room = item->room_number;
	auto floor = GetFloor(x, y, z, &room);

	int height = GetHeight(floor, x, y, z);
	if (height != NO_HEIGHT)
		height -= item->pos.y_pos;

	return (int16_t)height;
}

int16_t LaraCeilingFront(ITEM_INFO* item, PHD_ANGLE ang, int32_t dist, int h)
{
	int x = item->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT),
		y = item->pos.y_pos - h,
		z = item->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT);

	auto room = item->room_number;
	auto floor = GetFloor(x, y, z, &room);

	int height = GetCeiling(floor, x, y, z);
	if (height != NO_HEIGHT)
		height -= item->pos.y_pos - h;

	return (int16_t)height;
}

int LaraFallen(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->mid_floor > STEPUP_HEIGHT && lara.water_status != LARA_WADE)
	{
		item->anim_number = FALLDOWN_A;
		item->frame_number = FALLDOWN_F;
		item->current_anim_state = AS_FORWARDJUMP;
		item->goal_anim_state = AS_FORWARDJUMP;
		item->fallspeed = 0;
		item->gravity_status = 1;

		return 1;
	}

	return 0;
}

int LaraTestEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, int32_t* edge)
{
	auto bounds = GetBoundsAccurate(item);

	int16_t hdif = coll->front_floor - bounds[2];

	if ((hdif < 0 && hdif + item->fallspeed < 0) || (hdif > 0 && hdif + item->fallspeed > 0))
	{
		hdif = item->pos.y_pos + bounds[2];

		if ((hdif >> (WALL_SHIFT - 2)) != ((hdif + item->fallspeed) >> (WALL_SHIFT - 2)))
		{
			*edge = (item->fallspeed > 0 ? (hdif + item->fallspeed) & ~(STEP_L - 1) : hdif & ~(STEP_L - 1));

			return -1;
		}

		return 0;
	}

	return (ABS(coll->left_floor2 - coll->right_floor2) < SLOPE_DIF);
}

int LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll)
{
	if ((coll->coll_type == COLL_FRONT) || (coll->coll_type == COLL_TOPFRONT))
	{
		ShiftItem(item, coll);

		item->goal_anim_state = AS_STOP;
		item->current_anim_state = AS_STOP;
		item->speed = 0;
		item->gravity_status = 0;

		return 1;
	}
	else if (coll->coll_type == COLL_LEFT)
	{
		ShiftItem(item, coll);
		item->pos.y_rot += LARA_DEF_ADD_EDGE;
	}

	else if (coll->coll_type == COLL_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.y_rot -= LARA_DEF_ADD_EDGE;
	}

	return 0;
}

int GetStaticObjects(ITEM_INFO* item, PHD_ANGLE ang, int32_t hite, int32_t radius, int32_t dist)
{
	auto room_number = item->room_number;

	int x = item->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT),
		y = item->pos.y_pos,
		z = item->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT),
		inxmin = x - radius,
		inxmax = x + radius,
		inymin = y - hite,
		inymax = y,
		inzmin = z - radius,
		inzmax = z + radius;

	GetNearByRooms(x, y, z, radius + 50, hite + 50, room_number);

	for (int i = 0; i < number_draw_rooms; ++i)
	{
		auto r = &room[rooms_to_draw[i]];
		auto mesh = r->mesh;

		for (int j = r->num_meshes; j > 0; --j, ++mesh)
		{
			auto sinfo = &static_objects[mesh->static_number];

			if (sinfo->flags & 1)
				continue;

			int ymin = mesh->y + sinfo->y_minc,
				ymax = mesh->y + sinfo->y_maxc,
				xmin, xmax, zmin, zmax;

			switch (mesh->y_rot)
			{
			case 16384:
				xmin = mesh->x + sinfo->z_minc;
				xmax = mesh->x + sinfo->z_maxc;
				zmin = mesh->z - sinfo->x_maxc;
				zmax = mesh->z - sinfo->x_minc;
				break;
			case -32768:
				xmin = mesh->x - sinfo->x_maxc;
				xmax = mesh->x - sinfo->x_minc;
				zmin = mesh->z - sinfo->z_maxc;
				zmax = mesh->z - sinfo->z_minc;
				break;
			case -16384:
				xmin = mesh->x - sinfo->z_maxc;
				xmax = mesh->x - sinfo->z_minc;
				zmin = mesh->z + sinfo->x_minc;
				zmax = mesh->z + sinfo->x_maxc;
				break;
			default:
				xmin = mesh->x + sinfo->x_minc;
				xmax = mesh->x + sinfo->x_maxc;
				zmin = mesh->z + sinfo->z_minc;
				zmax = mesh->z + sinfo->z_maxc;
				break;
			}

			if (inxmax <= xmin || inxmin >= xmax ||
				inymax <= ymin || inymin >= ymax ||
				inzmax <= zmin || inzmin >= zmax)
				continue;

			return 1;
		}
	}

	return 0;
}