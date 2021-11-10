#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "effect2.h"
#include "lot.h"
#include "sphere.h"
#include "control.h"
#include "traps.h"
#include "invfunc.h"
#include "setup.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define WILLBOSS_TURN 			(ONE_DEGREE * 5)
#define WILLBOSS_ATTACK_TURN	(ONE_DEGREE * 2)
#define WILLBOSS_TOUCH			(0x900000)
#define WILLBOSS_BITE_DAMAGE	220
#define WILLBOSS_TOUCH_DAMAGE	10
#define WILLBOSS_ATTACK_RANGE	SQUARE(WALL_L * 3 / 2)
#define WILLBOSS_LUNGE_RANGE	SQUARE(WALL_L * 2)
#define WILLBOSS_FIRE_RANGE		SQUARE(WALL_L * 4)
#define WILLBOSS_STOP_ANIM 		0
#define WILLBOSS_KILL_ANIM 		6
#define WILLBOSS_HP_AFTER_KO	200
#define WILLBOSS_KO_TIME		280
#define WILLBOSS_PATH_DISTANCE	1024
#define NO_AI_PATH				-1
#define WILLBOSS_STUN_ANIM		7

void WillBossDie(int16_t item_number);
void ExplodeWillBoss(ITEM_INFO* item);

void TriggerWillbossPlasma(int16_t item_number, long node, long size);
void TriggerWillbossPlasmaBallFlame(int16_t fx_number, long type, long xv, long yv, long zv);
void TriggerWillbossPlasmaBall(PHD_VECTOR* pos, int16_t room_number, int16_t angle, int16_t type);

enum
{
	ATTACK_HEAD,
	ATTACK_HAND1,
	ATTACK_HAND2
};

enum tribeboss_anims
{
	WILLBOSS_STOP,
	WILLBOSS_WALK,
	WILLBOSS_LUNGE,
	WILLBOSS_BIGKILL,
	WILLBOSS_STUNNED,
	WILLBOSS_KNOCKOUT,
	WILLBOSS_GETUP,
	WILLBOSS_WALKATAK1,
	WILLBOSS_WALKATAK2,
	WILLBOSS_180,
	WILLBOSS_SHOOT
};

long death_radii[5];
long death_heights[5];
long dradii[5]		= { 100 << 4,350 << 4,400 << 4,350 << 4,100 << 4 };
long dheights1[5]	= { -1536 - (768 << 3),-1152 - (384 << 3),-768,-384 + (384 << 3),0 + (768 << 3) };
long dheights2[5]	= { -1536,-1152,-768,-384,0 };

uint8_t puzzle_complete;

/*
0 - Electric from head
1 - Electric beam hitting wall.
2 - Boss firing electric beam.
3 - Lara being electricuted.
4 - Lara hit by electric beam.
*/
inline PHD_VECTOR TrigDynamics[5];

struct SHIELD_POINTS
{
	short x;
	short y;
	short z;
	uint8_t rsub;
	uint8_t gsub;
	uint8_t bsub;
	uint8_t pad[3];
	long rgb;
};

struct EXPLOSION_VERTS
{
	short x;
	short z;
	long rgb;
};

struct EXPLOSION_RING
{
	short on;
	short life;		// 0 - 32.
	short speed;
	short radius;	// Width is 1/4 of radius.
	short xrot;
	short zrot;
	long x;
	long y;
	long z;
	EXPLOSION_VERTS	verts[16];
};

BITE_INFO willboss_bite_left  = { 19, -13, 3, 20 };
BITE_INFO willboss_bite_right = { 19, -13, 3, 23 };

SHIELD_POINTS WillBossShield[40];
EXPLOSION_RING ExpRings[7];

PHD_3DPOS ai_path[16];
PHD_3DPOS ai_junction[4];

int junction_index[4];
int closest_ai_path = NO_AI_PATH;
int lara_ai_path = NO_AI_PATH;
int lara_junction = NO_AI_PATH;
int direction = 1;
int desired_direction = 1;

import prof;

void WillBossControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto willboss = (CREATURE_INFO*)item->data;
	
	bool lara_alive = (lara_item->hit_points > 0);

	if (closest_ai_path == NO_AI_PATH)
	{
		for (int linknum = room[item->room_number].item_number, pathnum = 0, juncnum = 0; linknum != NO_ITEM; linknum = items[linknum].next_item)
			if (items[linknum].object_number == AI_X1 && pathnum < 16)
				ai_path[pathnum++] = items[linknum].pos;
			else if (items[linknum].object_number == AI_X2 && juncnum < 4)
				ai_junction[juncnum++] = items[linknum].pos;

		int best_distance = 0x7fffffff;

		closest_ai_path = -1;

		for (int i = 0; i < 16; ++i)
		{
			int x = (ai_path[i].x_pos - item->pos.x_pos) >> 6,
				z = (ai_path[i].z_pos - item->pos.z_pos) >> 6,
				distance = x * x + z * z;

			if (distance < best_distance)
			{
				closest_ai_path = i;
				best_distance = distance;
			}
		}

		lara_ai_path = -1;
		best_distance = 0x7fffffff;

		for (int i = 0; i < 16; ++i)
		{
			int x = (ai_path[i].x_pos - lara_item->pos.x_pos) >> 6,
				z = (ai_path[i].z_pos - lara_item->pos.z_pos) >> 6,
				distance = x * x + z * z;

			if (distance < best_distance)
			{
				lara_ai_path = i;
				best_distance = distance;
			}
		}

		for (int juncnum = 0; juncnum < 4; ++juncnum)
		{
			int pathnum = -1;

			best_distance = 0x7fffffff;

			for (int i = 0; i < 16; ++i)
			{
				int x = abs((ai_path[i].x_pos - ai_junction[juncnum].x_pos) >> 6),
					z = abs((ai_path[i].z_pos - ai_junction[juncnum].z_pos) >> 6),
					distance = (x > z ? x + (z >> 1) : x + (z >> 1));

				if (distance < best_distance)
				{
					pathnum = i;
					best_distance = distance;
				}
			}

			junction_index[juncnum] = pathnum;
		}
	}

	int pathnum = -1,
		best_distance = 0x7fffffff,
		old_closest = closest_ai_path;

	for (int i = old_closest - 1; i < old_closest + 2; ++i)
	{
		if (i < 0)			pathnum = i + 16;
		else if (i > 15)	pathnum = i - 16;
		else				pathnum = i;

		int x = (ai_path[pathnum].x_pos - item->pos.x_pos) >> 6,
			z = (ai_path[pathnum].z_pos - item->pos.z_pos) >> 6,
			distance = x * x + z * z;

		if (distance < best_distance)
		{
			closest_ai_path = pathnum;
			best_distance = distance;
		}
	}

	pathnum = -1;
	best_distance = 0x7fffffff;
	old_closest = lara_ai_path;

	for (int i = old_closest - 1; i < old_closest + 2; ++i)
	{
		if (i < 0)		 pathnum = i + 16;
		else if (i > 15) pathnum = i - 16;
		else			 pathnum = i;

		int x = (ai_path[pathnum].x_pos - lara_item->pos.x_pos) >> 6,
			z = (ai_path[pathnum].z_pos - lara_item->pos.z_pos) >> 6,
			distance = x * x + z * z;

		if (distance < best_distance)
		{
			lara_ai_path = pathnum;
			best_distance = distance;
		}
	}

	int best_junc_distance = 0x7fffffff;

	for (int i = 0; i < 4; ++i)
	{
		int x = (ai_junction[i].x_pos - lara_item->pos.x_pos) >> 6,
			z = (ai_junction[i].z_pos - lara_item->pos.z_pos) >> 6,
			distance = (x * x) + (z * z);

		if (distance < best_junc_distance)
		{
			lara_junction = i;
			best_junc_distance = distance;
		}
	}

	int x = ai_junction[lara_junction].x_pos - item->pos.x_pos,
		z = ai_junction[lara_junction].z_pos - item->pos.z_pos,
		will2junc_distance = (x * x) + (z * z);

	bool in_fire_zone = ((best_junc_distance < best_distance) || (item->pos.y_pos > lara_item->pos.y_pos + WALL_L * 2));
	
	if (item->hit_points <= 0)
	{
		puzzle_complete = Inv_RequestItem(ICON_PICKUP1_ITEM);
		puzzle_complete += Inv_RequestItem(ICON_PICKUP2_ITEM);
		puzzle_complete += Inv_RequestItem(ICON_PICKUP3_ITEM);
		puzzle_complete += Inv_RequestItem(ICON_PICKUP4_ITEM);

		if (puzzle_complete != 4 || item->item_flags[1] == 0)
		{
			willboss->maximum_turn = 0;

			switch (item->current_anim_state)
			{
			case WILLBOSS_STOP:
				item->goal_anim_state = WILLBOSS_STUNNED;
				break;
			case WILLBOSS_STUNNED:
				bossdata.death_count = WILLBOSS_KO_TIME;
				break;
			case WILLBOSS_KNOCKOUT:
				if (bossdata.death_count-- < 0)
					item->goal_anim_state = WILLBOSS_GETUP;
				break;
			case WILLBOSS_GETUP:
			{
				item->hit_points = WILLBOSS_HP_AFTER_KO;

				if (puzzle_complete == 4)
					item->item_flags[1] = 1;

				willboss->maximum_turn = WILLBOSS_ATTACK_TURN;

				break;
			}
			default: item->goal_anim_state = WILLBOSS_STOP;
			}
		}
		else
		{
			if (item->current_anim_state != WILLBOSS_STUNNED)
			{
				item->anim_number = objects[item->object_number].anim_index + WILLBOSS_STUN_ANIM;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = WILLBOSS_STUNNED;
			}
			else if (item->frame_number >= anims[item->anim_number].frame_end - 2)
			{
				item->frame_number = anims[item->anim_number].frame_end - 2;
				item->mesh_bits = 0;

				if (bossdata.explode_count == 0)
				{
					bossdata.ring_count = 0;

					for (int i = 0; i < 6; ++i)
					{
						ExpRings[i].on = 0;
						ExpRings[i].life = 32;
						ExpRings[i].radius = 512;
						ExpRings[i].speed = 128 + (i << 5);
						ExpRings[i].xrot = ((GetRandomControl() & 511) - 256) & 4095;
						ExpRings[i].zrot = ((GetRandomControl() & 511) - 256) & 4095;
					}
				}

				if (bossdata.explode_count < 256)
					++bossdata.explode_count;

				if (bossdata.explode_count > 128 && bossdata.ring_count == 6/* && ExpRings[5].life == 0*/)
				{
					WillBossDie(item_number);
					bossdata.dead = 1;
				}
				else ExplodeWillBoss(item);

				return;
			}
		}
	}
	else
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);

		if (item->touch_bits)
			lara_item->hit_points -= WILLBOSS_TOUCH_DAMAGE;

		int pathdiff = lara_ai_path - closest_ai_path;

		if (direction == -1 && ((pathdiff < 0 && pathdiff > -6) || pathdiff > 10))
			desired_direction = 1;
		else if (direction == 1 && ((pathdiff > 0 && pathdiff < 6) || pathdiff < -10))
			desired_direction = -1;
	
		willboss->target.x = ai_path[closest_ai_path].x_pos + (direction * WILLBOSS_PATH_DISTANCE * phd_sin(ai_path[closest_ai_path].y_rot) >> W2V_SHIFT);
		willboss->target.z = ai_path[closest_ai_path].z_pos + (direction * WILLBOSS_PATH_DISTANCE * phd_cos(ai_path[closest_ai_path].y_rot) >> W2V_SHIFT);

		switch (item->current_anim_state)
		{
		case WILLBOSS_STOP:
		{
			willboss->maximum_turn = 0;
			willboss->flags = 0;

			if (direction != desired_direction)
				item->goal_anim_state = WILLBOSS_180;
			else if (in_fire_zone && info.ahead && will2junc_distance < WILLBOSS_FIRE_RANGE && lara_item->hit_points > 0)
				item->goal_anim_state = WILLBOSS_SHOOT;
			else if (info.bite && info.distance < WILLBOSS_LUNGE_RANGE)
				item->goal_anim_state = WILLBOSS_LUNGE;
			else item->goal_anim_state = WILLBOSS_WALK;

			break;
		}
		case WILLBOSS_WALK:
		{
			willboss->maximum_turn = WILLBOSS_TURN;
			willboss->flags = 0;

			if (direction != desired_direction)
				item->goal_anim_state = WILLBOSS_STOP;
			else if (in_fire_zone && info.ahead && will2junc_distance < WILLBOSS_FIRE_RANGE)
				item->goal_anim_state = WILLBOSS_STOP;
			else if (info.bite && info.distance < WILLBOSS_ATTACK_RANGE)
			{
				if ((GetRandomControl() & 3) == 1)
					item->goal_anim_state = WILLBOSS_STOP;
				else if (item->frame_number < anims[item->anim_number].frame_base + 30)
					item->goal_anim_state = WILLBOSS_WALKATAK2;
				else item->goal_anim_state = WILLBOSS_WALKATAK1;
			}

			break;
		}
		case WILLBOSS_180:
		{
			willboss->maximum_turn = 0;
			willboss->flags = 0;

			if (item->frame_number == anims[item->anim_number].frame_base + 51)
			{
				item->pos.y_rot += 0x8000;
				direction = -direction;
			}

			break;
		}
		case WILLBOSS_LUNGE:
		{
			willboss->target.x = lara_item->pos.x_pos;
			willboss->target.z = lara_item->pos.z_pos;
			willboss->maximum_turn = WILLBOSS_ATTACK_TURN;

			if (!willboss->flags && (item->touch_bits & WILLBOSS_TOUCH))
			{
				lara_item->hit_status = 1;
				lara_item->hit_points -= WILLBOSS_BITE_DAMAGE * 2;

				willboss->flags = 1;

				CreatureEffect(item, &willboss_bite_left, DoBloodSplat);
				CreatureEffect(item, &willboss_bite_right, DoBloodSplat);
			}

			break;
		}
		case WILLBOSS_WALKATAK1:
		case WILLBOSS_WALKATAK2:
		{
			if (!willboss->flags && (item->touch_bits & WILLBOSS_TOUCH))
			{
				lara_item->hit_status = 1;
				lara_item->hit_points -= WILLBOSS_BITE_DAMAGE;
				CreatureEffect(item, &willboss_bite_left, DoBloodSplat);
				CreatureEffect(item, &willboss_bite_right, DoBloodSplat);

				willboss->flags = 1;
			}

			if (in_fire_zone && info.bite && will2junc_distance < WILLBOSS_FIRE_RANGE)
				item->goal_anim_state = WILLBOSS_WALK;
			else if (info.bite && info.distance < WILLBOSS_ATTACK_RANGE)
				item->goal_anim_state = (item->current_anim_state == WILLBOSS_WALKATAK1 ? WILLBOSS_WALKATAK2 : WILLBOSS_WALKATAK1);
			else item->goal_anim_state = WILLBOSS_WALK;

			break;
		}
		case WILLBOSS_BIGKILL:
		{
			switch (item->frame_number - anims[item->anim_number].frame_base)
			{
			case 0:
			case 43:
			case 95:
			case 105:
				CreatureEffect(item, &willboss_bite_left, DoBloodSplat);
				break;
			case 61:
			case 91:
			case 101:
				CreatureEffect(item, &willboss_bite_right, DoBloodSplat);
				break;
			}

			break;
		}
		case WILLBOSS_SHOOT:
		{
			willboss->target.x = lara_item->pos.x_pos;
			willboss->target.z = lara_item->pos.z_pos;
			willboss->maximum_turn = WILLBOSS_ATTACK_TURN;

			if (item->frame_number - anims[item->anim_number].frame_base == 40 && lara_item->hit_points > 0)
			{
				PHD_VECTOR pos { -64, 410, 0 };

				GetJointAbsPosition(item, &pos, 20);
				TriggerWillbossPlasmaBall(&pos, item->room_number, item->pos.y_rot - 0x1000, 0);

				pos = { 64, 410, 0 };

				GetJointAbsPosition(item, &pos, 23);
				TriggerWillbossPlasmaBall(&pos, item->room_number, item->pos.y_rot + 0x1000, 0);
			}

			PHD_VECTOR pos {};

			int bright = item->frame_number - anims[item->anim_number].frame_base;

			if (bright > 16)
			{
				bright = anims[item->anim_number].frame_end - item->frame_number;

				if (bright > 16)
					bright = 16;
			}

			int rnd = GetRandomControl(),
				r = ((rnd & 7) * bright) >> 4,
				g = ((31 - ((rnd >> 4) & 3)) * bright) >> 4,
				b = ((24 - ((rnd >> 6) & 3)) * bright) >> 4;

			GetJointAbsPosition(item, &pos, 17);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 12, r, g, b);
			TriggerWillbossPlasma(item_number, SPN_WILLBOSSLPLASMA, bright << 2);
			TriggerWillbossPlasma(item_number, SPN_WILLBOSSRPLASMA, bright << 2);

			break;
		}
		}

		if (lara_alive && lara_item->hit_points <= 0)
		{
			CreatureKill(item, WILLBOSS_KILL_ANIM, WILLBOSS_BIGKILL, EXTRA_YETIKILL);
			willboss->maximum_turn = 0;

			return;
		}
	}

	CreatureAnimation(item_number, CreatureTurn(item, willboss->maximum_turn), 0);
}

void S_DrawWillBoss(ITEM_INFO* item)
{
	DrawAnimatingItem(item);
}

void WillBossDie(int16_t item_number)
{
	auto item = &items[item_number];

	item->collidable = 0;
	item->hit_points = DONT_TARGET;

	KillItem(item_number);
	DisableBaddieAI(item_number);

	item->flags |= ONESHOT;
}

void InitialiseWillBoss(int16_t item_number)
{
	closest_ai_path = NO_AI_PATH;
	items[item_number].item_flags[1] = 0;
	bossdata.dead = bossdata.dropped_icon = bossdata.explode_count = 0;
}

void ExplodeWillBoss(ITEM_INFO* item)
{
	if (bossdata.explode_count == 1 ||
		bossdata.explode_count == 15 ||
		bossdata.explode_count == 25 ||
		bossdata.explode_count == 35 ||
		bossdata.explode_count == 45 ||
		bossdata.explode_count == 55)
	{
		int x = item->pos.x_pos + ((GetRandomDraw() & 1023) - 512),
			y = item->pos.y_pos - (GetRandomDraw() & 1023) - 256,
			z = item->pos.z_pos + ((GetRandomDraw() & 1023) - 512);

		ExpRings[bossdata.ring_count].x = x;
		ExpRings[bossdata.ring_count].y = y;
		ExpRings[bossdata.ring_count].z = z;
		ExpRings[bossdata.ring_count].on = 1;
		bossdata.ring_count++;

		for (x = 0; x < 24; x += 3)
		{
			PHD_VECTOR pos {};

			GetJointAbsPosition(item, &pos, x);
			TriggerWillbossPlasmaBall(&pos, item->room_number, GetRandomControl() << 1, 4);
		}

		TriggerExplosionSparks(x, y, z, 3, -2, 2, 0);

		g_audio->play_sound(76, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}

	for (int i = 0; i < 5; ++i)
	{
		if (bossdata.explode_count < 128)
		{
			death_radii[i] = (dradii[i] >> 4) + (((dradii[i]) * bossdata.explode_count) >> 7);
			death_heights[i] = dheights2[i] + (((dheights1[i] - dheights2[i]) * bossdata.explode_count) >> 7);
		}
	}

	auto shptr = &WillBossShield[0];

	for (int i = 0; i < 5; ++i)
	{
		int y = death_heights[i],
			angle = (wibble << 3) & 511;

		for (int j = 0; j < 8; ++j, ++shptr)
		{
			shptr->x = (m_sin(angle << 1) * death_radii[i]) >> 11;
			shptr->y = y;
			shptr->z = (m_cos(angle << 1) * death_radii[i]) >> 11;

			if (i != 0 && i != 4 && bossdata.explode_count < 64)
			{
				int g = (GetRandomDraw() & 31) + 224,
					b = (g >> 1) + (GetRandomDraw() & 63),
					r = (GetRandomDraw() & 63);

				r = (r * (64 - bossdata.explode_count)) >> 6;
				g = (g * (64 - bossdata.explode_count)) >> 6;
				b = (b * (64 - bossdata.explode_count)) >> 6;

				shptr->rgb = r | (g << 8) | (b << 16);
			}
			else shptr->rgb = 0;

			angle += 512;
			angle &= 4095;
		}
	}
}

void TriggerWillbossPlasma(int16_t item_number, long node, long size)
{
	int dx = lara_item->pos.x_pos - items[item_number].pos.x_pos,
		dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	size += (GetRandomControl() & 15);

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sG = 255;
	sptr->sB = 48 + (GetRandomControl() & 31);
	sptr->sR = 48;
	sptr->dG = 192 + (GetRandomControl() & 63);
	sptr->dB = 128 + (GetRandomControl() & 63);
	sptr->dR = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = ((GetRandomControl() & 15) - 8);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 31) - 16);
	sptr->Yvel = (GetRandomControl() & 7) + 8;
	sptr->Zvel = ((GetRandomControl() & 31) - 16);
	sptr->Friction = 3;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;

	sptr->Gravity = (GetRandomControl() & 7) + 8;
	sptr->MaxYvel = (GetRandomControl() & 7) + 16;
	sptr->FxObj = item_number;
	sptr->NodeNumber = node;
	sptr->Def = uint8_t(objects[EXPLOSION1].mesh_ptr);
	sptr->Scalar = 1;
	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 2;
	sptr->dHeight = size >> 2;
}

void TriggerWillbossPlasmaBallFlame(int16_t fx_number, long type, long xv, long yv, long zv)
{
	int dx = lara_item->pos.x_pos - effects[fx_number].pos.x_pos,
		dz = lara_item->pos.z_pos - effects[fx_number].pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sG = 255;
	sptr->sB = 48 + (GetRandomControl() & 31);
	sptr->sR = 48;
	sptr->dG = 192 + (GetRandomControl() & 63);
	sptr->dB = 128 + (GetRandomControl() & 63);
	sptr->dR = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = ((GetRandomControl() & 15) - 8);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 15) - 8);
	sptr->Xvel = (xv + (GetRandomControl() & 255) - 128) << 1;
	sptr->Yvel = (GetRandomControl() & 511) - 256;
	sptr->Zvel = (zv + (GetRandomControl() & 255) - 128) << 1;
	sptr->Friction = 5 | (5 << 4);

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;

	sptr->FxObj = fx_number;
	sptr->Def = uint8_t(objects[EXPLOSION1].mesh_ptr);
	sptr->Gravity = sptr->MaxYvel = 0;

	int size;

	if (type >= 0)
	{
		sptr->Scalar = 3;
		size = type;
	}
	else
	{
		sptr->Scalar = (type < -2 ? 4 : 2);
		size = (GetRandomControl() & 15) + 16;
		sptr->Xvel = xv + (GetRandomControl() & 255) - 128;
		sptr->Yvel = yv;
		sptr->Zvel = zv + (GetRandomControl() & 255) - 128;
		sptr->Friction = 5;
	}

	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 3;
	sptr->dHeight = size >> 3;
}

void TriggerWillbossPlasmaBall(PHD_VECTOR* pos, int16_t room_number, int16_t angle, int16_t type)
{
	if (auto fx_number = CreateEffect(room_number); fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		fx->pos.x_pos = pos->x;
		fx->pos.y_pos = pos->y;
		fx->pos.z_pos = pos->z;
		fx->pos.x_rot = 0;
		fx->pos.y_rot = angle;
		fx->object_number = EXTRAFX2;
		fx->speed = 16 + (type) ? (GetRandomControl() & 31) + 16 : 0;
		fx->fallspeed = -(type << 4);
		fx->flag1 = type;
	}
}

void ControlWillbossPlasmaBall(int16_t fx_number)
{
	uint8_t radtab[5] = { 13, 7, 7, 7, 7 };

	auto fx = &effects[fx_number];

	int old_x = fx->pos.x_pos,
		old_y = fx->pos.y_pos,
		old_z = fx->pos.z_pos;

	if (fx->flag1 == 0)
	{
		int tx = lara_item->pos.x_pos,
			ty = lara_item->pos.y_pos - 256,
			tz = lara_item->pos.z_pos;

		const auto angles = phd_GetVectorAngles({ tx - fx->pos.x_pos, ty - fx->pos.y_pos, tz - fx->pos.z_pos });

		fx->pos.y_rot = angles.x;
		fx->pos.x_rot = angles.y;

		if (fx->speed < 512)
			fx->speed += (fx->speed >> 4) + 4;

		if (wibble & 4)
			TriggerWillbossPlasmaBallFlame(fx_number, fx->speed >> 1, 0, 0, 0);
	}
	else
	{
		fx->fallspeed += (fx->flag1 == 1) ? 1 : 2;

		if ((wibble & 12) == 0)
		{
			if (fx->speed)
				--fx->speed;

			TriggerWillbossPlasmaBallFlame(fx_number, -1 - fx->flag1, 0, -(GetRandomControl() & 31), 0);
		}
	}

	int speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;

	fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
	fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
	fx->pos.y_pos += -((fx->speed * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT) + fx->fallspeed;

	auto room_number = fx->room_number;
	auto floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

	if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) ||
		fx->pos.y_pos < GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos))
	{
		if (fx->flag1 == 0)
		{
			PHD_VECTOR pos { old_x, old_y, old_z };

			int r = 2 + (GetRandomControl() & 3);

			for (int i = 0; i < r; ++i)
				TriggerWillbossPlasmaBall(&pos, fx->room_number, (fx->pos.y_rot) + 0x6000 + (GetRandomControl() & 0x3fff), 1);
		}

		KillEffect(fx_number);

		return;
	}

	if (ItemNearLara(lara_item, &fx->pos, 200) && !fx->flag1)
	{
		for (int i = 14; i >= 0; i -= 2)
		{
			PHD_VECTOR pos {};

			GetJointAbsPosition(lara_item, &pos, i);
			TriggerWillbossPlasmaBall(&pos, fx->room_number, GetRandomControl() << 1, 1);
		}

		LaraBurn();

		lara_item->hit_points = -1;
		lara.burn_green = 1;

		KillEffect(fx_number);

		return;
	}

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, lara_item->room_number);

	if (radtab[fx->flag1])
	{
		int rnd = GetRandomControl(),
			g = 31 - ((rnd >> 4) & 3),
			b = 24 - ((rnd >> 6) & 3),
			r = rnd & 7;

		TriggerDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, radtab[fx->flag1], r, g, b);
	}
}