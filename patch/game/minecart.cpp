#include <specific/standard.h>
#include <specific/input.h>
#include <specific/fn_stubs.h>

#include "objects.h"
#include "laraanim.h"
#include "laraflar.h"
#include "sphere.h"
#include "control.h"
#include "camera.h"
#include "effect2.h"
#include "minecart.h"
#include "game.h"
#include "physics.h"

#include <specific/init.h>

#include <mp/game/level.h>

#define GF2(a)					(anims[objects[minecart_anim_obj].anim_index + a].frame_base)
#define CART_GETINL_A			0
#define CART_OUTL_A				1
#define CART_PICK_A				5
#define CART_PICK_F				GF2(CART_PICK_A)
#define CART_USE_A				6
#define CART_DROP_A				7
#define CART_DROP_F				GF2(CART_DROP_A)
#define CART_WALLDEATH_A		23
#define CART_TURNDEATH_B		30
#define CART_TURNDEATH_A		31
#define CART_TURNDEATH_F		GF2(30)
#define CART_HIT_A				34
#define CART_HIT_F 				GF2(CART_HIT_A)
#define CART_GETINR_A			46
#define CART_OUTR_A				47
#define GETOFF_DIST				330
#define CART_RADIUS				100
#define CART_HEIGHT				768
#define CART_NHITS				25
#define CART_TO_BADDIE_RADIUS	256
#define CART_DEC				-0x600
#define CART_MIN_SPEED			0xa00
#define CART_MIN_VEL			32
#define TURN_DEATH_VEL			128
#define CART_FWD_GRAD			-128
#define CART_BACK_GRAD			128
#define CART_JUMP_VEL			0xfc00
#define MAX_CART_YVEL			0x3f00
#define TERMINAL_ANGLE			4096
#define TARGET_DIST				(WALL_L * 2)

enum
{
	CART_GETIN,
	CART_GETOUT,
	CART_GETOUTL,
	CART_GETOUTR,
	CART_STILL,
	CART_DUCK,
	CART_MOVE,
	CART_RIGHT,
	CART_HARDLEFT,
	CART_LEFT,
	CART_HARDRIGHT,
	CART_BRAKE,
	CART_FWD,
	CART_BACK,
	CART_TURNDEATH,
	CART_FALLDEATH,
	CART_WALLDEATH,
	CART_HIT,
	CART_USE,
	CART_BRAKING
};

int32_t TestHeight(ITEM_INFO* v, int32_t x, int32_t z)
{
	int c = phd_cos(v->pos.y_rot),
		s = phd_sin(v->pos.y_rot);

	PHD_VECTOR pos
	{
		v->pos.x_pos + (((z * s) + (x * c)) >> W2V_SHIFT),
		v->pos.y_pos - (z * phd_sin(v->pos.x_rot) >> W2V_SHIFT) + (x * phd_sin(v->pos.z_rot) >> W2V_SHIFT),
		v->pos.z_pos + (((z * c) - (x * s)) >> W2V_SHIFT)
	};

	auto room_number = v->room_number;

	return GetHeight(GetFloor(pos.x, pos.y, pos.z, &room_number), pos.x, pos.y, pos.z);
}

int16_t GetCollision(ITEM_INFO* v, int16_t ang, int32_t dist, int16_t* ceiling)
{
	int x = v->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT),
		y = v->pos.y_pos - LARA_HITE,
		z = v->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT);

	auto r = v->room_number;
	auto floor = GetFloor(x, y, z, &r);

	int height = GetHeight(floor, x, y, z),
		cheight = GetCeiling(floor, x, y, z);

	*ceiling = ((int16_t)cheight);

	if (height != NO_HEIGHT)
		height -= v->pos.y_pos;

	return ((int16_t)height);
}

int GetInMineCart(ITEM_INFO* v, ITEM_INFO* l, COLL_INFO* coll)
{
	if (!(input & IN_ACTION) ||
		lara.gun_status != LG_ARMLESS ||
		l->gravity_status ||
		!TestBoundsCollide(v, l, coll->radius) ||
		!TestCollision(v, l))
		return 0;

	int x = l->pos.x_pos - v->pos.x_pos,
		z = l->pos.z_pos - v->pos.z_pos,
		dist = (x * x) + (z * z);

	if (dist > 200000)
		return 0;

	auto room_number = v->room_number;

	return (GetHeight(GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number), v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) >= -32000);
}

int CanGetOut(int direction)
{
	auto v = &items[lara.skidoo];

	int angle = (direction < 0 ? v->pos.y_rot + 0x4000 : v->pos.y_rot - 0x4000);

	int x = v->pos.x_pos - (GETOFF_DIST * phd_sin(angle) >> W2V_SHIFT),
		y = v->pos.y_pos,
		z = v->pos.z_pos - (GETOFF_DIST * phd_cos(angle) >> W2V_SHIFT);

	auto room_number = v->room_number;
	auto floor = GetFloor(x, y, z, &room_number);

	int height = GetHeight(floor, x, y, z);

	if (height_type == BIG_SLOPE ||
		height_type == DIAGONAL ||
		height == NO_HEIGHT ||
		ABS(height - v->pos.y_pos) > WALL_L / 2)
		return 0;

	int ceiling = GetCeiling(floor, x, y, z);

	return ((ceiling - v->pos.y_pos <= -LARA_HITE) && (height - ceiling >= LARA_HITE));
}

void CartToBaddieCollision(ITEM_INFO* v)
{
	int16_t roomies[20],
		   numroom = 1;

	roomies[0] = v->room_number;

	if (auto door = room[v->room_number].door)
	{
		for (int i = (int)*(door++); i > 0; --i)
		{
			roomies[numroom++] = *(door);
			door += 16;
		}
	}

	for (int i = 0; i < numroom; ++i)
	{
		auto item_num = room[roomies[i]].item_number;

		while (item_num != NO_ITEM)
		{
			auto item = &items[item_num];
			auto nex = item->next_item;

			if (item->collidable && item->status != INVISIBLE)
			{
				auto object = &objects[item->object_number];

				if (object->collision && (object->intelligent || item->object_number == ANIMATING2))
				{
					int x = v->pos.x_pos - item->pos.x_pos,
						y = v->pos.y_pos - item->pos.y_pos,
						z = v->pos.z_pos - item->pos.z_pos;

					if (x > -TARGET_DIST &&
						x < TARGET_DIST &&
						z > -TARGET_DIST &&
						z < TARGET_DIST &&
						y > -TARGET_DIST &&
						y < TARGET_DIST)
					{
						if (TestBoundsCollide(item, v, CART_TO_BADDIE_RADIUS))
						{
							if (item->object_number == ANIMATING2)
							{
								if (item->frame_number == anims[item->anim_number].frame_base &&
									lara_item->current_anim_state == CART_USE &&
									lara_item->anim_number == objects[minecart_anim_obj].anim_index + CART_USE_A)
								{
									auto frame = lara_item->frame_number - anims[lara_item->anim_number].frame_base;

									if (frame >= 12 && frame <= 22)
									{
										g_audio->play_sound(220, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

										auto room_number = item->room_number;
										auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

										GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
										TestTriggers(trigger_index, 1);

										++item->frame_number;
									}
								}
							}
							else
							{
								DoLotsOfBlood(item->pos.x_pos, v->pos.y_pos - STEP_L, item->pos.z_pos, v->speed, v->pos.y_rot, item->room_number, 3);

								item->hit_points = 0;
							}
						}
					}
				}
			}

			item_num = nex;
		}
	}
}

void MoveCart(ITEM_INFO* v, ITEM_INFO* l, CARTINFO* cart)
{
	if (cart->StopDelay)
		--cart->StopDelay;

	if ((lara.mine_l && lara.mine_r && !cart->StopDelay) && ((v->pos.x_pos & 0x380) == 512 || (v->pos.z_pos & 0x380) == 512))
	{
		if (cart->Speed < 0xf000)
		{
			cart->Flags |= CF_STOPPED | CF_CONTROL;
			cart->Speed = v->speed = 0;

			return;
		}
		else cart->StopDelay = 16;
	}

	if ((lara.mine_l || lara.mine_r) && (!(lara.mine_l && lara.mine_r)) && !cart->StopDelay && !(cart->Flags & (CF_TURNINGL | CF_TURNINGR)))
	{
		auto rot = (((uint16_t)v->pos.y_rot) >> 14) | (lara.mine_l << 2);
		auto turn_modified_angle = (4096 + (minecart_turn_extra_blocks * 1024));

		switch (rot)
		{
		case 0:
			cart->TurnX = (v->pos.x_pos + turn_modified_angle) & ~1023;
			cart->TurnZ = v->pos.z_pos & ~1023;
			break;
		case 1:
			cart->TurnX = v->pos.x_pos & ~1023;
			cart->TurnZ = (v->pos.z_pos - turn_modified_angle) | 1023;
			break;
		case 2:
			cart->TurnX = (v->pos.x_pos - turn_modified_angle) | 1023;
			cart->TurnZ = v->pos.z_pos | 1023;
			break;
		case 3:
			cart->TurnX = v->pos.x_pos | 1023;
			cart->TurnZ = (v->pos.z_pos + turn_modified_angle) & ~1023;
			break;
		case 4:
			cart->TurnX = (v->pos.x_pos - turn_modified_angle) | 1023;
			cart->TurnZ = v->pos.z_pos & ~1023;
			break;
		case 5:
			cart->TurnX = v->pos.x_pos & ~1023;
			cart->TurnZ = (v->pos.z_pos + turn_modified_angle) & ~1023;
			break;
		case 6:
			cart->TurnX = (v->pos.x_pos + turn_modified_angle) & ~1023;
			cart->TurnZ = v->pos.z_pos | 1023;
			break;
		case 7:
			cart->TurnX = v->pos.x_pos | 1023;
			cart->TurnZ = (v->pos.z_pos - turn_modified_angle) | 1023;
			break;
		}

		if (int ang = m_atan2(v->pos.x_pos, v->pos.z_pos, cart->TurnX, cart->TurnZ) & 0x3fff; rot < 4)
		{
			cart->TurnRot = v->pos.y_rot;
			cart->TurnLen = ang;
		}
		else
		{
			cart->TurnRot = v->pos.y_rot;

			if (ang)
				ang = 16384 - ang;

			cart->TurnLen = ang;
		}

		cart->Flags |= lara.mine_l ? CF_TURNINGL : CF_TURNINGR;
	}

	if (cart->Speed < CART_MIN_SPEED)
		cart->Speed = CART_MIN_SPEED;

	cart->Speed += (-cart->Gradient << 2);

	if ((v->speed = cart->Speed >> 8) < CART_MIN_VEL)
	{
		v->speed = CART_MIN_VEL;

		g_audio->stop_sound(209);

		if (cart->YVel)
			g_audio->stop_sound(210);
		else g_audio->play_sound(210, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });
	}
	else
	{
		g_audio->stop_sound(210);

		if (cart->YVel)
			g_audio->stop_sound(209);
		else g_audio->play_sound(209, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });
	}

	if (cart->Flags & (CF_TURNINGL | CF_TURNINGR))
	{
		auto turn_len_modifier = (3 + -minecart_turn_extra_blocks);

		turn_len_modifier = turn_len_modifier == 0 ? 2 : turn_len_modifier;

		cart->TurnLen += (v->speed * turn_len_modifier);

		if (cart->TurnLen > (ONE_DEGREE * 90))
		{
			if (cart->Flags & CF_TURNINGL)
				v->pos.y_rot = cart->TurnRot - 16384;
			else v->pos.y_rot = cart->TurnRot + 16384;

			cart->Flags &= ~(CF_TURNINGL | CF_TURNINGR);
		}
		else
		{
			if (cart->Flags & CF_TURNINGL)
				v->pos.y_rot = cart->TurnRot - cart->TurnLen;
			else v->pos.y_rot = cart->TurnRot + cart->TurnLen;
		}

		if (cart->Flags & (CF_TURNINGL | CF_TURNINGR))
		{
			auto quad = uint16_t(((uint16_t)v->pos.y_rot) >> 14),
				 deg = uint16_t(v->pos.y_rot & 16383);

			int32_t x, z;

			switch (quad)
			{
			case 0:
				x = -phd_cos(deg);
				z = phd_sin(deg);
				break;
			case 1:
				x = phd_sin(deg);
				z = phd_cos(deg);
				break;
			case 2:
				x = phd_cos(deg);
				z = -phd_sin(deg);
				break;
			default:
				x = -phd_sin(deg);
				z = -phd_cos(deg);
				break;
			}

			if (cart->Flags & CF_TURNINGL)
			{
				x = -x;
				z = -z;
			}

			auto rot_angle = (4096 + (minecart_turn_extra_blocks * 1024) - 512);

			v->pos.x_pos = cart->TurnX + ((x * rot_angle) >> W2V_SHIFT);
			v->pos.z_pos = cart->TurnZ + ((z * rot_angle) >> W2V_SHIFT);
		}
	}
	else
	{
		v->pos.x_pos += (v->speed * phd_sin(v->pos.y_rot)) >> W2V_SHIFT;
		v->pos.z_pos += (v->speed * phd_cos(v->pos.y_rot)) >> W2V_SHIFT;
	}

	cart->MidPos = TestHeight(v, 0, 0);

	if (!cart->YVel)
	{
		cart->FrontPos = TestHeight(v, 0, 256);
		cart->Gradient = cart->MidPos - cart->FrontPos;
		v->pos.y_pos = cart->MidPos;
	}
	else
	{
		if (v->pos.y_pos > cart->MidPos)
		{
			if (cart->YVel > 0)
				g_audio->play_sound(202, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });

			v->pos.y_pos = cart->MidPos;

			cart->YVel = 0;
		}
		else
		{
			cart->YVel += GRAVITY * 171;

			if (cart->YVel > MAX_CART_YVEL)
				cart->YVel = MAX_CART_YVEL;

			v->pos.y_pos += cart->YVel >> 8;
		}
	}

	v->pos.x_rot = cart->Gradient << 5;

	int16_t val = v->pos.y_rot & 16383;

	if (cart->Flags & (CF_TURNINGL | CF_TURNINGR))
		v->pos.z_rot = ((cart->Flags & CF_TURNINGR) ? -(val * v->speed) >> 9 : ((16384 - val) * v->speed) >> 9);
	else v->pos.z_rot -= v->pos.z_rot >> 3;
}

void DoUserInput(ITEM_INFO* v, ITEM_INFO* l, CARTINFO* cart)
{
	bool getting_off = false;

	switch (l->current_anim_state)
	{
	case CART_MOVE:
	{
		if (input & IN_ACTION)						l->goal_anim_state = CART_USE;
		else if (input & IN_DUCK)					l->goal_anim_state = CART_DUCK;
		else if (input & IN_JUMP)					l->goal_anim_state = CART_BRAKE;
		else if (cart->Speed == CART_MIN_VEL ||
				 (cart->Flags & CF_STOPPED))		l->goal_anim_state = CART_STILL;
		else if (cart->Gradient < CART_FWD_GRAD)	l->goal_anim_state = CART_FWD;
		else if (cart->Gradient > CART_BACK_GRAD)	l->goal_anim_state = CART_BACK;
		else if (input & IN_LEFT)					l->goal_anim_state = CART_LEFT;
		else if (input & IN_RIGHT)					l->goal_anim_state = CART_RIGHT;

		break;
	}
	case CART_FWD:
	{
		if (input & IN_ACTION)					 l->goal_anim_state = CART_USE;
		else if (input & IN_DUCK)				 l->goal_anim_state = CART_DUCK;
		else if (input & IN_JUMP)				 l->goal_anim_state = CART_BRAKE;
		else if (cart->Gradient > CART_FWD_GRAD) l->goal_anim_state = CART_MOVE;

		break;
	}
	case CART_BACK:
	{
		if (input & IN_ACTION)					  l->goal_anim_state = CART_USE;
		else if (input & IN_DUCK)				  l->goal_anim_state = CART_DUCK;
		else if (input & IN_JUMP)				  l->goal_anim_state = CART_BRAKE;
		else if (cart->Gradient < CART_BACK_GRAD) l->goal_anim_state = CART_MOVE;

		break;
	}
	case CART_LEFT:
	{
		if (input & IN_ACTION)	  l->goal_anim_state = CART_USE;
		else if (input & IN_DUCK) l->goal_anim_state = CART_DUCK;
		else if (input & IN_JUMP) l->goal_anim_state = CART_BRAKE;

		if (!(input & IN_LEFT))
			l->goal_anim_state = CART_MOVE;

		break;
	}
	case CART_RIGHT:
	{
		if (input & IN_ACTION)	  l->goal_anim_state = CART_USE;
		else if (input & IN_DUCK) l->goal_anim_state = CART_DUCK;
		else if (input & IN_JUMP) l->goal_anim_state = CART_BRAKE;

		if (!(input & IN_RIGHT))
			l->goal_anim_state = CART_MOVE;

		break;
	}
	case CART_STILL:
	{
		if (!(cart->Flags & CF_CONTROL))
		{
			g_audio->play_sound(211, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });

			cart->Flags |= CF_CONTROL;
			cart->StopDelay = 64;
		}

		if ((input & IN_ROLL) && (cart->Flags & CF_STOPPED))
		{
			if ((input & IN_LEFT) && (CanGetOut(-1)))
			{
				l->goal_anim_state = CART_GETOUT;
				cart->Flags &= ~CF_RDIR;
			}

			else if ((input & IN_RIGHT) && (CanGetOut(1)))
			{
				l->goal_anim_state = CART_GETOUT;
				cart->Flags |= CF_RDIR;
			}
		}

		if (input & IN_DUCK)				 l->goal_anim_state = CART_DUCK;
		else if (cart->Speed > CART_MIN_VEL) l->goal_anim_state = CART_MOVE;

		break;
	}
	case CART_DUCK:
	{
		if (input & IN_ACTION)		 l->goal_anim_state = CART_USE;
		else if (input & IN_JUMP)	 l->goal_anim_state = CART_BRAKE;
		else if (!(input & IN_DUCK)) l->goal_anim_state = CART_STILL;

		break;
	}
	case CART_USE:
		l->goal_anim_state = CART_MOVE;
		break;
	case CART_BRAKING:
	{
		if (input & IN_DUCK)
		{
			l->goal_anim_state = CART_DUCK;
			g_audio->stop_sound(219);
		}
		else if (!(input & IN_JUMP) || (cart->Flags & CF_STOPPED))
		{
			l->goal_anim_state = CART_MOVE;
			g_audio->stop_sound(219);
		}

		else
		{
			cart->Speed += CART_DEC;
			g_audio->play_sound(220, { l->pos.x_pos, l->pos.y_pos, l->pos.z_pos });
		}

		break;
	}
	case CART_BRAKE:
		l->goal_anim_state = CART_BRAKING;
		break;
	case CART_GETOUT:
	{
		if (l->anim_number == objects[minecart_anim_obj].anim_index + CART_DROP_A)
		{
			if (l->frame_number == CART_DROP_F + 20 && (cart->Flags & CF_MESH))
			{
				auto tmp = lara.mesh_ptrs[HAND_R];

				lara.mesh_ptrs[HAND_R] = meshes[objects[minecart_anim_obj].mesh_ptr + HAND_R];
				meshes[objects[minecart_anim_obj].mesh_ptr + HAND_R] = tmp;

				cart->Flags &= ~CF_MESH;
			}

			l->goal_anim_state = ((cart->Flags & CF_RDIR) ? CART_GETOUTR : CART_GETOUTL);
		}

		break;
	}
	case CART_GETOUTL:
	{
		if (l->anim_number == objects[minecart_anim_obj].anim_index + CART_OUTL_A &&
			l->frame_number == anims[l->anim_number].frame_end)
		{
			PHD_VECTOR vec { 0, 640, 0 };

			get_lara_bone_pos(lara_item, &vec, HIPS);

			l->pos.x_pos = vec.x;
			l->pos.y_pos = vec.y;
			l->pos.z_pos = vec.z;
			l->pos.x_rot = 0;
			l->pos.y_rot = v->pos.y_rot + 0x4000;
			l->pos.z_rot = 0;
			l->anim_number = STOP_A;
			l->frame_number = STOP_F;
			l->current_anim_state = l->goal_anim_state = AS_STOP;

			lara.skidoo = NO_ITEM;
			lara.gun_status = LG_ARMLESS;

			getting_off = true;
		}

		break;
	}
	case CART_GETOUTR:
	{
		if (l->anim_number == objects[minecart_anim_obj].anim_index + CART_OUTR_A &&
			l->frame_number == anims[l->anim_number].frame_end)
		{
			PHD_VECTOR vec { 0, 640, 0 };

			get_lara_bone_pos(lara_item, &vec, HIPS);

			l->pos.x_pos = vec.x;
			l->pos.y_pos = vec.y;
			l->pos.z_pos = vec.z;
			l->pos.x_rot = 0;
			l->pos.y_rot = v->pos.y_rot - 0x4000;
			l->pos.z_rot = 0;
			l->anim_number = STOP_A;
			l->frame_number = STOP_F;
			l->current_anim_state = l->goal_anim_state = AS_STOP;

			lara.skidoo = NO_ITEM;
			lara.gun_status = LG_ARMLESS;

			getting_off = true;
		}

		break;
	}
	case CART_GETIN:
	{
		if (l->anim_number == objects[minecart_anim_obj].anim_index + CART_PICK_A &&
			l->frame_number == CART_PICK_F + 20 &&
			(!cart->Flags & CF_MESH))
		{
			auto tmp = lara.mesh_ptrs[HAND_R];

			lara.mesh_ptrs[HAND_R] = meshes[objects[minecart_anim_obj].mesh_ptr + HAND_R];
			meshes[objects[minecart_anim_obj].mesh_ptr + HAND_R] = tmp;

			cart->Flags |= CF_MESH;
		}

		break;
	}
	case CART_WALLDEATH:
		camera.target_elevation = -25 * ONE_DEGREE;
		camera.target_distance = WALL_L * 4;
		break;
	case CART_TURNDEATH:
	{
		camera.target_elevation = -45 * ONE_DEGREE;
		camera.target_distance = WALL_L * 2;

		int16_t ch,
			    fh = GetCollision(v, v->pos.y_rot, 512, &ch);

		if (fh > -STEP_L && fh < STEP_L)
		{
			if ((wibble & 7) == 0)
				g_audio->play_sound(202, { v->pos.x_pos, v->pos.y_pos, v->pos.z_pos });

			v->pos.x_pos += (TURN_DEATH_VEL * phd_sin(v->pos.y_rot)) >> W2V_SHIFT;
			v->pos.z_pos += (TURN_DEATH_VEL * phd_cos(v->pos.y_rot)) >> W2V_SHIFT;
		}
		else
		{
			if (l->anim_number == objects[minecart_anim_obj].anim_index + CART_TURNDEATH_B)
			{
				cart->Flags |= CF_NOANIM;
				l->hit_points = -1;
			}
		}

		break;
	}
	case CART_HIT:
	{
		if (l->hit_points <= 0 && l->frame_number == CART_HIT_F + 28)
		{
			l->frame_number = CART_HIT_F + 28;
			cart->Flags = (cart->Flags & ~CF_CONTROL) | (CF_NOANIM);
			cart->Speed = v->speed = 0;
		}
	}
	}

	if (lara.skidoo != NO_ITEM && !(cart->Flags & CF_NOANIM))
	{
		AnimateItem(l);

		v->anim_number = objects[MINECART].anim_index + (l->anim_number - objects[minecart_anim_obj].anim_index);
		v->frame_number = anims[v->anim_number].frame_base + (l->frame_number - anims[l->anim_number].frame_base);
	}

	if (l->current_anim_state != CART_TURNDEATH && l->current_anim_state != CART_WALLDEATH && l->hit_points > 0)
	{
		if (v->pos.z_rot > TERMINAL_ANGLE || v->pos.z_rot < -TERMINAL_ANGLE)
		{
			l->anim_number = objects[minecart_anim_obj].anim_index + CART_TURNDEATH_A;
			l->frame_number = anims[l->anim_number].frame_base;
			l->current_anim_state = l->goal_anim_state = CART_TURNDEATH;

			cart->Flags = (cart->Flags & ~CF_CONTROL) | CF_STOPPED | CF_DEAD;
			cart->Speed = v->speed = 0;

			return;
		}

		int16_t ch,
			    fh = GetCollision(v, v->pos.y_rot, 512, &ch);

		if (fh < -(STEP_L * 2))
		{
			l->anim_number = objects[minecart_anim_obj].anim_index + CART_WALLDEATH_A;
			l->frame_number = anims[l->anim_number].frame_base;
			l->current_anim_state = l->goal_anim_state = CART_WALLDEATH;
			l->hit_points = -1;

			cart->Flags = (cart->Flags & ~CF_CONTROL) | (CF_STOPPED | CF_DEAD);
			cart->Speed = v->speed = 0;

			return;
		}

		if (l->current_anim_state != CART_DUCK && l->current_anim_state != CART_HIT)
		{
			COLL_INFO coll;

			coll.quadrant = (uint16_t)(v->pos.y_rot + 0x2000) / 0x4000;
			coll.radius = CART_RADIUS;

			if (CollideStaticObjects(&coll, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, v->room_number, CART_HEIGHT))
			{
				l->anim_number = objects[minecart_anim_obj].anim_index + CART_HIT_A;
				l->frame_number = anims[l->anim_number].frame_base;
				l->current_anim_state = l->goal_anim_state = CART_HIT;

				DoLotsOfBlood(l->pos.x_pos, l->pos.y_pos - 768, l->pos.z_pos, v->speed, v->pos.y_rot, l->room_number, 3);

				int hits = (CART_NHITS * ((((uint16_t)cart->Speed)) >> 11));

				if (hits < 20)
					hits = 20;

				l->hit_points -= hits;

				return;
			}
		}

		if (fh > 512 + 64 && !cart->YVel)
			cart->YVel = int16_t(CART_JUMP_VEL);

		CartToBaddieCollision(v);
	}

	if (getting_off)
		if (auto entity = g_level->get_entity_by_item(v))
			g_level->request_entity_ownership(entity, false);
}

void MineCartInitialise(int16_t item_number)
{
	items[item_number].data = (CARTINFO*)game_malloc(sizeof(CARTINFO), 0);
}

void MineCartCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
		return;

	auto v = &items[item_number];

	if (GetInMineCart(v, l, coll))
	{
		if (auto entity = g_level->get_entity_by_item(v))
			if (!g_level->is_entity_streamed(entity))
			{
				ObjectCollision(item_number, l, coll);

				return g_level->request_entity_ownership(entity, true);
			}

		lara.skidoo = item_number;

		if (lara.gun_type == LG_FLARE)
		{
			create_flare(0);
			undraw_flare_meshes();

			lara.flare_control_left = 0;
			lara.request_gun_type = lara.gun_type = LG_ARMLESS;
		}

		lara.gun_status = LG_HANDSBUSY;

		auto ang = ((int16_t)m_atan2(v->pos.x_pos, v->pos.z_pos, l->pos.x_pos, l->pos.z_pos)) - v->pos.y_rot;

		l->anim_number = ((ang > -(ONE_DEGREE * 45)) && (ang < (ONE_DEGREE * 135)) ? objects[minecart_anim_obj].anim_index + CART_GETINR_A
																				   : objects[minecart_anim_obj].anim_index + CART_GETINL_A);
		l->frame_number = anims[l->anim_number].frame_base;
		l->current_anim_state = l->goal_anim_state = CART_GETIN;
		l->pos.x_pos = v->pos.x_pos;
		l->pos.y_pos = v->pos.y_pos;
		l->pos.z_pos = v->pos.z_pos;
		l->pos.x_rot = v->pos.x_rot;
		l->pos.y_rot = v->pos.y_rot;
		l->pos.z_rot = v->pos.z_rot;

		auto cart = (CARTINFO*)v->data;

		cart->Flags = 0;
		cart->Speed = 0;
		cart->YVel = 0;
		cart->Gradient = 0;
	}
	else ObjectCollision(item_number, l, coll);
}

int MineCartControl()
{
	auto l = lara_item;
	auto v = &items[lara.skidoo];
	auto cart = (CARTINFO*)v->data;

	DoUserInput(v, l, cart);

	if (cart->Flags & CF_CONTROL)
		MoveCart(v, l, cart);

	if (lara.skidoo != NO_ITEM)
	{
		l->pos.x_pos = v->pos.x_pos;
		l->pos.y_pos = v->pos.y_pos;
		l->pos.z_pos = v->pos.z_pos;
		l->pos.x_rot = v->pos.x_rot;
		l->pos.y_rot = v->pos.y_rot;
		l->pos.z_rot = v->pos.z_rot;
	}

	auto room_number = v->room_number;

	GetHeight(GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number), v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);

	if (room_number != v->room_number)
	{
		ItemNewRoom(lara.skidoo, room_number);
		ItemNewRoom(lara.item_number, room_number);
	}

	TestTriggers(trigger_index, 0);

	if (!(cart->Flags & CF_DEAD))
	{
		camera.target_elevation = -45 * ONE_DEGREE;
		camera.target_distance = WALL_L * 2;
	}

	return (lara.skidoo == NO_ITEM) ? 0 : 1;
}