#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "effect2.h"
#include "lot.h"
#include "sphere.h"
#include "people.h"
#include "text.h"
#include "control.h"
#include "laraanim.h"
#include "laraelec.h"
#include "pickup.h"
#include "traps.h"
#include "setup.h"

#include <specific/fn_stubs.h>

#define	SMALL_FLASH 				10
#define	BIG_FLASH 					16
#define	BOLT_SPEED					384
#define	LONDONBOSS_VAULT2_ANIM		9
#define	LONDONBOSS_VAULT3_ANIM		18
#define	LONDONBOSS_VAULT4_ANIM		15
#define	LONDONBOSS_GODOWN_ANIM		21
#define	LONDONBOSS_STND2SUM_ANIM	1
#define LONDONBOSS_SUMMON_ANIM		2
#define	LONDONBOSS_GODOWN_ANIM		21
#define LONDONBOSS_VAULT_SHIFT		96
#define LONDONBOSS_AWARE_DISTANCE	SQUARE(WALL_L)
#define LONDONBOSS_WALK_TURN		(ONE_DEGREE * 4)
#define LONDONBOSS_RUN_TURN			(ONE_DEGREE * 7)
#define LONDONBOSS_WALK_RANGE		SQUARE(WALL_L)
#define LONDONBOSS_WALK_CHANCE		0x100
#define LONDONBOSS_LAUGH_CHANCE		0x100
#define LONDONBOSS_TURN 			(ONE_DEGREE * 2)
#define LONDONBOSS_DIE_ANIM 		17
#define LONDONBOSS_FINAL_HEIGHT		-11776
#define BIGZAP_TIMER				600

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
	short life;
	short speed;
	short radius;
	short xrot;
	short zrot;
	long x;
	long y;
	long z;
	EXPLOSION_VERTS	verts[16];
};

enum
{
	NOTHING,
	SUMMONING,
	KNOCKBACK
};

enum
{
	NORMAL_BOLT,
	LARGE_BOLT,
	SUMMON_BOLT
};

enum
{
	RIGHT_PRONG,
	ICONPOS,
	LEFT_PRONG
};

enum
{
	ATTACK_HEAD,
	ATTACK_HAND1,
	ATTACK_HAND2
};

enum londonboss_anims
{
	LONDONBOSS_EMPTY,
	LONDONBOSS_STAND,
	LONDONBOSS_WALK,
	LONDONBOSS_RUN,
	LONDONBOSS_SUMMON,
	LONDONBOSS_BIGZAP,
	LONDONBOSS_DEATH,
	LONDONBOSS_LAUGH,
	LONDONBOSS_LILZAP,
	LONDONBOSS_VAULT2,
	LONDONBOSS_VAULT3,
	LONDONBOSS_VAULT4,
	LONDONBOSS_GODOWN
};

static constexpr char links[20][4] =
{
	{ 0, 0, 1, 2 },
	{ 0, 0, 2, 3 },
	{ 0, 0, 3, 4 },
	{ 0, 0, 4, 1 },

	{ 1, 2, 5, 6 },
	{ 2, 3, 6, 7 },
	{ 3, 4, 7, 8 },
	{ 4, 1, 8, 5 },

	{ 1 + 4, 2 + 4, 5 + 4, 6 + 4 },
	{ 2 + 4, 3 + 4, 6 + 4, 7 + 4 },
	{ 3 + 4, 4 + 4, 7 + 4, 8 + 4 },
	{ 4 + 4, 1 + 4, 8 + 4, 5 + 4 },

	{ 1 + 8, 2 + 8, 5 + 8, 6 + 8 },
	{ 2 + 8, 3 + 8, 6 + 8, 7 + 8 },
	{ 3 + 8, 4 + 8, 7 + 8, 8 + 8 },
	{ 4 + 8, 1 + 8, 8 + 8, 5 + 8 },

	{ 1 + 12, 2 + 12, 5 + 12, 5 + 12 },
	{ 2 + 12, 3 + 12, 5 + 12, 5 + 12 },
	{ 3 + 12, 4 + 12, 5 + 12, 5 + 12 },
	{ 4 + 12, 1 + 12, 5 + 12, 5 + 12 }
};

static constexpr char sumlinks[8][4] =
{
	{ 0, 0, 1, 2 },
	{ 0, 0, 2, 3 },
	{ 0, 0, 3, 4 },
	{ 0, 0, 4, 1 },

	{ 1, 2, 5, 5 },
	{ 2, 3, 5, 5 },
	{ 3, 4, 5, 5 },
	{ 4, 1, 5, 5 },
};

long death_radii_lb[5];
long death_heights_lb[5];
long radii_lb[5] = { 200, 400, 500, 500, 475 };
long heights_lb[5] = { -1536, -1280, -832, -384,0 };
long dradii_lb[5] = { 100 << 4, 350 << 4, 400 << 4, 350 << 4, 100 << 4 };
long dheights1_lb[5] = { -1536 - (768 << 3), -1152 - (384 << 3), -768, -384 + (384 << 3), 768 << 3 };
long dheights2_lb[5] = { -1536,-1152,-768,-384,0 };

BITE_INFO londonboss_points[3] =
{
	{ 16, 56, 356, 10 },	// Right prong.
	{ -28, 48, 304, 10 },	// Icon.
	{ -72, 48, 356, 10 },	// Left prong.
};

SHIELD_POINTS LondonBossShield[40];
EXPLOSION_RING ExpRingsLondonBoss[6];
EXPLOSION_RING KBRings[3];

void TriggerPlasmaBallLondonBoss(ITEM_INFO* item, long type, PHD_VECTOR* pos1, int16_t room_number, int16_t angle);
void TriggerLaserBolt(PHD_VECTOR* pos, ITEM_INFO* item, long type, int16_t yang);
void LondonBossDie(int16_t item_number);
void ExplodeLondonBoss(ITEM_INFO* item);
long KnockBackCollision(EXPLOSION_RING* erptr);

void LondonBossControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto londonboss = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   angle = 0,
		   tilt = 0,
		   torso_x = 0,
		   torso_y = 0;

	if (item->item_flags[2])
	{
		if (item->item_flags[2] == 1)
			item->hit_points = 0;

		if (item->item_flags[2] >= 12)
			TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, (GetRandomControl() & 3) + 8, 0, (GetRandomControl() & 7) + 8, (GetRandomControl() & 7) + 16);
		else TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 25 - (item->item_flags[2] << 1) + (GetRandomControl() & 1), (16 - item->item_flags[2]) + (GetRandomControl() & 7), 32 - item->item_flags[2], 31);
	}

	PHD_VECTOR trident[3];

	for (int i = 0; i < 3; ++i)
	{
		trident[i] = { londonboss_points[i].x,  londonboss_points[i].y,  londonboss_points[i].z };

		GetJointAbsPosition(item, &trident[i], londonboss_points[i].mesh_num);
	}

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != LONDONBOSS_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + LONDONBOSS_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LONDONBOSS_DEATH;
		}

		if (anims[item->anim_number].frame_end - item->frame_number == 1)
		{
			item->frame_number = anims[item->anim_number].frame_end - 1;
			item->mesh_bits = 0;

			if (bossdata.explode_count == 0)
			{
				bossdata.ring_count = 0;

				for (int i = 0; i < 6; ++i)
				{
					ExpRingsLondonBoss[i].on = 0;
					ExpRingsLondonBoss[i].life = 32;
					ExpRingsLondonBoss[i].radius = 512;
					ExpRingsLondonBoss[i].speed = 128 + (i << 5);
					ExpRingsLondonBoss[i].xrot = ((GetRandomControl() & 511) - 256) << 4;
					ExpRingsLondonBoss[i].zrot = ((GetRandomControl() & 511) - 256) << 4;
				}

				if (bossdata.dropped_icon == 0)
				{
					BossDropIcon(item_number);

					bossdata.dropped_icon = 1;
				}
			}

			if (bossdata.explode_count < 256)
				++bossdata.explode_count;

			if (bossdata.explode_count > 64 && bossdata.ring_count == 6/* && ExpRingsLondonBoss[5].life == 0*/)
			{
				LondonBossDie(item_number);

				bossdata.dead = 1;
			}
			else ExplodeLondonBoss(item);
			
			return;
		}
	}
	else
	{
		if (item->ai_bits)
			GetAITarget(londonboss);

		AI_INFO info,
				lara_info;

		CreatureAIInfo(item, &info);

		if (londonboss->enemy == lara_item)
		{
			lara_info.angle = info.angle;
			lara_info.distance = info.distance;
			lara_info.x_angle = info.x_angle;
		}
		else
		{
			int lara_dz = lara_item->pos.z_pos - item->pos.z_pos,
				lara_dx = lara_item->pos.x_pos - item->pos.x_pos,
				lara_dy = item->pos.y_pos - lara_item->pos.y_pos;

			lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;

			lara_info.x_angle = (abs(lara_dx) > abs(lara_dz) ? phd_atan(abs(lara_dx) + (abs(lara_dz) >> 1), lara_dy)
															 : phd_atan(abs(lara_dz) + (abs(lara_dx) >> 1), lara_dy));
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, londonboss->maximum_turn);

		auto enemy = londonboss->enemy;

		londonboss->enemy = lara_item;
	
		if (item->hit_status || lara_info.distance < LONDONBOSS_AWARE_DISTANCE || TargetVisible(item, &lara_info) || lara_item->pos.y_pos < item->pos.y_pos)
			AlertAllGuards(item_number);

		londonboss->enemy = enemy;

		if (lara_item->pos.y_pos < item->pos.y_pos)
			londonboss->hurt_by_lara = 1;

		if (item->timer > 0)
			--item->timer;

		item->hit_points = 300;

		switch (item->current_anim_state)
		{
		case LONDONBOSS_LAUGH:
		{
			if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN) item->pos.y_rot += lara_info.angle;
			else if (lara_info.angle < 0)					 item->pos.y_rot -= LONDONBOSS_WALK_TURN;
			else											 item->pos.y_rot += LONDONBOSS_WALK_TURN;

			if (londonboss->alerted)
			{
				item->goal_anim_state = LONDONBOSS_STAND;
				break;
			}
		}
		case LONDONBOSS_STAND:
		{
			londonboss->flags = 0;
			londonboss->maximum_turn = 0;

			if (londonboss->reached_goal)
			{
				londonboss->reached_goal = 0;

				item->ai_bits |= AMBUSH;
				item->item_flags[3] += 0x2000;
			}

			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				if ((head < -0x3000 || head > 0x3000) && item->pos.y_pos > LONDONBOSS_FINAL_HEIGHT)
				{
					item->goal_anim_state = LONDONBOSS_WALK;
					londonboss->maximum_turn = LONDONBOSS_WALK_TURN;
				}

				head = AIGuard(londonboss);

				if (!(GetRandomControl() & 0xF))
					item->goal_anim_state = LONDONBOSS_LAUGH;

				break;
			}
			else if ((item->pos.y_pos <= LONDONBOSS_FINAL_HEIGHT || item->pos.y_pos < lara_item->pos.y_pos) && !(GetRandomControl() & 0xF) && !bossdata.charged && item->timer)
				item->goal_anim_state = LONDONBOSS_LAUGH;
			else if (londonboss->reached_goal || lara_item->pos.y_pos > item->pos.y_pos || item->pos.y_pos <= LONDONBOSS_FINAL_HEIGHT)
			{
				if (bossdata.charged)
					item->goal_anim_state = LONDONBOSS_BIGZAP;
				else if (!item->timer)
					item->goal_anim_state = LONDONBOSS_SUMMON;
				else item->goal_anim_state = LONDONBOSS_LILZAP;
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = LONDONBOSS_WALK;
			else if (londonboss->mood == ESCAPE_MOOD || item->pos.y_pos > lara_item->pos.y_pos)
				item->goal_anim_state = LONDONBOSS_RUN;
			else if (londonboss->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (londonboss->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
			{
				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.ahead)
					item->goal_anim_state = LONDONBOSS_STAND;
				else item->goal_anim_state = LONDONBOSS_RUN;
			}
			else if (info.bite && info.distance < LONDONBOSS_WALK_RANGE)
				item->goal_anim_state = LONDONBOSS_WALK;
			else item->goal_anim_state = LONDONBOSS_RUN;

			break;
		}
		case LONDONBOSS_WALK:
		{
			londonboss->flags = 0;
			londonboss->maximum_turn = LONDONBOSS_WALK_TURN;

			head = lara_info.angle;

			if (item->ai_bits & GUARD || (londonboss->reached_goal && !(item->ai_bits & FOLLOW)))
				item->goal_anim_state = LONDONBOSS_STAND;
			else if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = LONDONBOSS_WALK;
				head = 0;
			}
			else if (londonboss->mood == ESCAPE_MOOD)
				item->goal_anim_state = LONDONBOSS_RUN;
			else if (londonboss->mood == BORED_MOOD)
			{
				if (GetRandomControl() < LONDONBOSS_LAUGH_CHANCE)
				{
					item->required_anim_state = LONDONBOSS_LAUGH;
					item->goal_anim_state = LONDONBOSS_STAND;
				}
			}
			else if (info.distance > LONDONBOSS_WALK_RANGE)
				item->goal_anim_state = LONDONBOSS_RUN;

			break;
		}
		case LONDONBOSS_RUN:
		{
			londonboss->maximum_turn = LONDONBOSS_RUN_TURN;

			if (info.ahead)
				head = info.angle;

			tilt = angle / 2;

			if (item->ai_bits & GUARD || (londonboss->reached_goal && !(item->ai_bits & FOLLOW)))
				item->goal_anim_state = LONDONBOSS_STAND;
			else if (londonboss->mood == ESCAPE_MOOD)
				break;
			else if ((item->ai_bits & FOLLOW) && (londonboss->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
				item->goal_anim_state = LONDONBOSS_STAND;
			else if (londonboss->mood == BORED_MOOD)
				item->goal_anim_state = LONDONBOSS_WALK;
			else if (info.ahead && info.distance < LONDONBOSS_WALK_RANGE)
				item->goal_anim_state = LONDONBOSS_WALK;

			break;
		}
		case LONDONBOSS_SUMMON:
		{
			head = lara_info.angle;

			if (londonboss->reached_goal)
			{
				londonboss->reached_goal = 0;
				item->ai_bits = AMBUSH;
				item->item_flags[3] += 0x2000;
			}

			if (item->anim_number == objects[item->object_number].anim_index + LONDONBOSS_STND2SUM_ANIM)
			{
				if (item->frame_number == anims[item->anim_number].frame_base)
				{
					bossdata.hp_counter = item->hit_points;

					item->timer = BIGZAP_TIMER;
				}
				else if (item->hit_status && item->goal_anim_state != LONDONBOSS_STAND)
				{
					g_audio->stop_sound(352);

					g_audio->play_sound(353, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					g_audio->play_sound(355, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

					item->goal_anim_state = LONDONBOSS_STAND;
				}
			}
			else if (item->anim_number == objects[item->object_number].anim_index + LONDONBOSS_SUMMON_ANIM && item->frame_number == anims[item->anim_number].frame_end)
				bossdata.charged = 1;

			if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
				item->pos.y_rot += lara_info.angle;
			else if (lara_info.angle < 0)
				item->pos.y_rot -= LONDONBOSS_WALK_TURN;
			else item->pos.y_rot += LONDONBOSS_WALK_TURN;

			if ((wibble & 7) == 0)
			{
				PHD_VECTOR pos1 { item->pos.x_pos, (GetRandomControl() & 511) - 256, item->pos.z_pos };

				TriggerLaserBolt(&pos1, item, SUMMON_BOLT, 0);

				for (int i = 0; i < 6; ++i)
				{
					if (!ExpRingsLondonBoss[i].on)
					{
						int rnd = (GetRandomControl() & 1023);

						ExpRingsLondonBoss[i].on = 1;
						ExpRingsLondonBoss[i].life = 64;
						ExpRingsLondonBoss[i].speed = (GetRandomControl() & 15) + 16;
						ExpRingsLondonBoss[i].x = item->pos.x_pos;
						ExpRingsLondonBoss[i].y = item->pos.y_pos + 128 - rnd;
						ExpRingsLondonBoss[i].z = item->pos.z_pos;
						ExpRingsLondonBoss[i].xrot = ((GetRandomControl() & 511) - 256) << 4;
						ExpRingsLondonBoss[i].zrot = ((GetRandomControl() & 511) - 256) << 4;
						ExpRingsLondonBoss[i].radius = (1024 + (1024 - (abs(rnd - 512))));

						break;
					}
				}
			}

			londonboss->maximum_turn = 0;

			break;
		}
		case LONDONBOSS_BIGZAP:
		{
			londonboss->maximum_turn = 0;

			if (londonboss->reached_goal)
			{
				londonboss->reached_goal = 0;
				item->ai_bits = AMBUSH;
				item->item_flags[3] += 0x2000;
			}

			bossdata.charged = 0;

			if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
				item->pos.y_rot += lara_info.angle;
			else if (lara_info.angle < 0)
				item->pos.y_rot -= LONDONBOSS_WALK_TURN;
			else item->pos.y_rot += LONDONBOSS_WALK_TURN;

			torso_y = lara_info.angle;
			torso_x = lara_info.x_angle;

			if (item->frame_number == anims[item->anim_number].frame_base + 36)
			{
				TriggerLaserBolt(&trident[RIGHT_PRONG], item, NORMAL_BOLT, item->pos.y_rot + 0x200);
				TriggerLaserBolt(&trident[ICONPOS], item, LARGE_BOLT, item->pos.y_rot);
				TriggerLaserBolt(&trident[LEFT_PRONG], item, NORMAL_BOLT, item->pos.y_rot - 0x200);
			}

			break;
		}
		case LONDONBOSS_LILZAP:
		{
			londonboss->maximum_turn = 0;

			if (londonboss->reached_goal)
			{
				londonboss->reached_goal = 0;
				item->ai_bits = AMBUSH;
				item->item_flags[3] += 0x2000;
			}

			if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
				item->pos.y_rot += lara_info.angle;
			else if (lara_info.angle < 0)
				item->pos.y_rot -= LONDONBOSS_WALK_TURN;
			else item->pos.y_rot += LONDONBOSS_WALK_TURN;

			torso_y = lara_info.angle;
			torso_x = lara_info.x_angle;

			if (item->frame_number == anims[item->anim_number].frame_base + 14)
			{
				TriggerLaserBolt(&trident[RIGHT_PRONG], item, NORMAL_BOLT, item->pos.y_rot + 0x200);
				TriggerLaserBolt(&trident[LEFT_PRONG], item, NORMAL_BOLT, item->pos.y_rot - 0x200);
			}
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	if ((item->current_anim_state < LONDONBOSS_VAULT2 || item->current_anim_state > LONDONBOSS_GODOWN) && item->current_anim_state != LONDONBOSS_DEATH)
	{
		switch (CreatureVault(item_number, angle, 2, LONDONBOSS_VAULT_SHIFT))
		{
		case 2:
			londonboss->maximum_turn = 0;
			item->anim_number = objects[LON_BOSS].anim_index + LONDONBOSS_VAULT2_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LONDONBOSS_VAULT2;
			break;
		case 3:
			londonboss->maximum_turn = 0;
			item->anim_number = objects[LON_BOSS].anim_index + LONDONBOSS_VAULT3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LONDONBOSS_VAULT3;
			break;
		case 4:
			londonboss->maximum_turn = 0;
			item->anim_number = objects[LON_BOSS].anim_index + LONDONBOSS_VAULT4_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LONDONBOSS_VAULT4;
			break;
		case -4:
			londonboss->maximum_turn = 0;
			item->anim_number = objects[LON_BOSS].anim_index + LONDONBOSS_GODOWN_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LONDONBOSS_GODOWN;
		}
	}
	else
	{
		londonboss->maximum_turn = 0;

		CreatureAnimation(item_number, angle, 0);
	}

	{
		int g = abs(m_sin(item->item_flags[1] << 7, 7)) + (GetRandomControl() & 7);

		if (g > 31)
			g = 31;

		int b = g >> 1;

		TriggerDynamicLight(trident[ICONPOS].x, trident[ICONPOS].y, trident[ICONPOS].z, 10, 0, g >> 1, b >> 1);

		item->item_flags[1]++;
		item->item_flags[1] &= 63;
	}

	if (item->hit_points > 0 && item->item_flags[0] != KNOCKBACK && lara_item->hit_points > 0)
	{
		int dx = lara_item->pos.x_pos - item->pos.x_pos,
			dy = lara_item->pos.y_pos - 256 - item->pos.y_pos,
			dz = lara_item->pos.z_pos - item->pos.z_pos;

		if (dx > 8000 || dx < -8000 || dy > 8000 || dy < -8000 || dz > 8000 || dz < -8000)
			dx = 0xFFF;
		else
		{
			dx = SQUARE(dx);
			dy = SQUARE(dy);
			dz = SQUARE(dz);
			dx = phd_sqrt(dx + dy + dz);
		}

		if (dx < 0xB00)
		{
			item->item_flags[0] = KNOCKBACK;

			for (int i = 0; i < 3; ++i)
			{
				KBRings[i].on = 1;
				KBRings[i].life = 32;
				KBRings[i].speed = 64 + ((i == 1) << 4);
				KBRings[i].x = item->pos.x_pos;
				KBRings[i].y = item->pos.y_pos - 384 - 128 + (i << 7);
				KBRings[i].z = item->pos.z_pos;
				KBRings[i].xrot = KBRings[i].zrot = 0;
				KBRings[i].radius = 512 + ((i == 1) << 8);
			}
		}
	}
	else if (KBRings[0].on == 0 && KBRings[1].on == 0 && KBRings[2].on == 0)
		item->item_flags[0] = 0;
}

void S_DrawLondonBoss(ITEM_INFO* item)
{
	DrawAnimatingItem(item);

	if (item->hit_points <= 0 && bossdata.explode_count == 0)
	{
		UpdateElectricityPoints();
		LaraElectricDeath(0, item);
		LaraElectricDeath(1, item);
	}
}

void LondonBossDie(int16_t item_number)
{
	auto item = &items[item_number];

	item->collidable = 0;
	item->hit_points = DONT_TARGET;

	KillItem(item_number);
	DisableBaddieAI(item_number);

	item->flags |= ONESHOT;
}

void InitialiseLondonBoss(int16_t item_number)
{
	bossdata.explode_count = bossdata.ring_count = bossdata.dead = bossdata.dropped_icon = 0;

	auto shptr = &LondonBossShield[0];

	for (int i = 0; i < 5; ++i)
	{
		int y = heights_lb[i],
			angle = 0;

		for (int j = 0; j < 8; ++j, ++shptr)
		{
			shptr->x = (m_sin(angle << 1) * radii_lb[i]) >> 11;
			shptr->y = y;
			shptr->z = (m_cos(angle << 1) * radii_lb[i]) >> 11;
			shptr->rgb = 0;

			angle += 512;
		}
	}
}

void ExplodeLondonBoss(ITEM_INFO* item)
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

		ExpRingsLondonBoss[bossdata.ring_count].x = x;
		ExpRingsLondonBoss[bossdata.ring_count].y = y;
		ExpRingsLondonBoss[bossdata.ring_count].z = z;
		ExpRingsLondonBoss[bossdata.ring_count].on = 1;
		bossdata.ring_count++;

		TriggerExplosionSparks(x, y, z, 3, -2, 2, 0);

		for (int i = 0; i < 2; ++i)
			TriggerExplosionSparks(x, y, z, 3, -1, 2, 0);

		g_audio->play_sound(76, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}

	for (int i = 0; i < 5; ++i)
	{
		if (bossdata.explode_count < 128)
		{
			death_radii_lb[i] = (dradii_lb[i] >> 4) + (((dradii_lb[i]) * bossdata.explode_count) >> 7);
			death_heights_lb[i] = dheights2_lb[i] + (((dheights1_lb[i] - dheights2_lb[i]) * bossdata.explode_count) >> 7);
		}
	}

	auto shptr = &LondonBossShield[0];

	for (int i = 0; i < 5; ++i)
	{
		int y = death_heights_lb[i],
			angle = (wibble << 3) & 511;

		for (int j = 0; j < 8; ++j, ++shptr)
		{
			shptr->x = (m_sin(angle << 1) * death_radii_lb[i]) >> 11;
			shptr->y = y;
			shptr->z = (m_cos(angle << 1) * death_radii_lb[i]) >> 11;

			if (i != 0 && i != 4 && bossdata.explode_count < 64)
			{
				int g = (GetRandomDraw() & 31) + 224,
					b = (g >> 2) + (GetRandomDraw() & 63),
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

void TriggerLaserBolt(PHD_VECTOR* pos, ITEM_INFO* item, long type, int16_t yang)
{
	if (auto item_number = CreateItem(); item_number != NO_ITEM)
	{
		auto bolt_item = &items[item_number];

		bolt_item->object_number = EXTRAFX4;
		bolt_item->room_number = item->room_number;
		bolt_item->pos.x_pos = pos->x;
		bolt_item->pos.y_pos = pos->y;
		bolt_item->pos.z_pos = pos->z;

		InitialiseItem(item_number);

		if (type == SUMMON_BOLT)
		{
			bolt_item->pos.y_pos += item->pos.y_pos - 384;
			bolt_item->pos.x_rot = -pos->y << 5;
			bolt_item->pos.y_rot = GetRandomControl() << 1;
		}
		else
		{
			const auto angles = phd_GetVectorAngles({ lara_item->pos.x_pos - pos->x,lara_item->pos.y_pos - 256 - pos->y,lara_item->pos.z_pos - pos->z });

			bolt_item->pos.x_rot = angles.y;
			bolt_item->pos.y_rot = yang;
			bolt_item->pos.z_rot = 0;
		}

		if (type != LARGE_BOLT)
		{
			bolt_item->speed = 16;
			bolt_item->item_flags[0] = -24;
			bolt_item->item_flags[1] = 4;

			if (type == SUMMON_BOLT)
				bolt_item->item_flags[2] = 1;
		}
		else
		{
			bolt_item->speed = 24;
			bolt_item->item_flags[0] = 31;
			bolt_item->item_flags[1] = 16;
		}

		AddActiveItem(item_number);
	}
}

void ControlLaserBolts(int16_t item_number)
{
	auto item = &items[item_number];
	auto oldroom = item->room_number;

	PHD_VECTOR oldpos { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos };

	int speed = (item->speed * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;

	item->pos.z_pos += (speed * phd_cos(item->pos.y_rot)) >> W2V_SHIFT;
	item->pos.x_pos += (speed * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;
	item->pos.y_pos += -((item->speed * phd_sin(item->pos.x_rot)) >> W2V_SHIFT);

	if (item->speed < BOLT_SPEED)
		item->speed += (item->speed >> 3) + 2;

	if (item->item_flags[2] && item->speed > (BOLT_SPEED >> 1))
		if (++item->item_flags[3] >= 16)
			return KillItem(item_number);

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	if (!item->item_flags[2])
	{
		bool hitlara = ItemNearLara(lara_item, &item->pos, 400);

		item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (hitlara || item->pos.y_pos >= item->floor || item->pos.y_pos <= GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos))
		{
			g_audio->play_sound(105, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			int rad = (item->item_flags[0] < 0 ? 2 : 3);

			TriggerExplosionSparks(oldpos.x, oldpos.y, oldpos.z, rad, -2, 2, item->room_number);

			for (int i = 0; i < rad; ++i)
				TriggerExplosionSparks(oldpos.x, oldpos.y, oldpos.z, 2, -1, 2, item->room_number);

			++rad;

			for (int i = 0; i < rad; ++i)
				TriggerPlasmaBallLondonBoss(item, 1, &oldpos, oldroom, item->pos.y_rot);

			if (hitlara)
			{
				lara_item->hit_points -= 30 + ((item->item_flags[0] >= 0) << 9);
				lara_item->hit_status = 1;
			}
			else
			{
				int dx = SQUARE(lara_item->pos.x_pos - item->pos.x_pos),
					dy = SQUARE(lara_item->pos.y_pos - 256 - item->pos.y_pos),
					dz = SQUARE(lara_item->pos.z_pos - item->pos.z_pos);

				if ((dx = phd_sqrt(dx + dy + dz)) < 0x400)
				{
					lara_item->hit_points -= (0x400 - dx) >> (6 - ((item->item_flags[0] >= 0) << 1));
					lara_item->hit_status = 1;
				}
			}
			
			return KillItem(item_number);
		}
	}

	int g = 31 - (GetRandomControl() & 7),
		b = g >> 1;

	int rad;

	if (item->item_flags[0] < 0)
	{
		if (item->item_flags[2])
		{
			g = (g * (16 - item->item_flags[3])) >> 4;
			b = (b * (16 - item->item_flags[3])) >> 4;
		}

		if ((rad = -item->item_flags[0]) > SMALL_FLASH)
		{
			++item->item_flags[0];
			item->item_flags[1] += 2;
		}
	}
	else if ((rad = item->item_flags[0]) > BIG_FLASH)
	{
		--item->item_flags[0];
		item->item_flags[1] += 4;
	}

	TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, rad, 0, g, b);
}

void TriggerPlasmaBallFlameLB(int16_t fx_number, long type, long xv, long yv, long zv)
{
	int dx = lara_item->pos.x_pos - effects[fx_number].pos.x_pos,
		dz = lara_item->pos.z_pos - effects[fx_number].pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int	size = (GetRandomControl() & 31) + 64;

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
	sptr->Xvel = xv + (GetRandomControl() & 255) - 128;
	sptr->Yvel = yv;
	sptr->Zvel = zv + (GetRandomControl() & 255) - 128;
	sptr->Friction = 5;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;

	sptr->FxObj = fx_number;
	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 1;
	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 2;
	sptr->dHeight = size >> 2;
	sptr->Gravity = sptr->MaxYvel = 0;
}

void TriggerPlasmaBallLondonBoss(ITEM_INFO* item, long type, PHD_VECTOR* pos1, int16_t room_number, int16_t angle)
{
	PHD_VECTOR pos { pos1->x, pos1->y, pos1->z };

	int speed = (GetRandomControl() & 31) + 64;

	int16_t angles[2] { int16_t(angle + GetRandomControl() + 0x4000), int16_t(0x2000) };

	if (auto fx_number = CreateEffect(room_number); fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		fx->pos.x_pos = pos.x;
		fx->pos.y_pos = pos.y;
		fx->pos.z_pos = pos.z;
		fx->pos.y_rot = angles[0];
		fx->pos.x_rot = angles[1];
		fx->object_number = EXTRAFX5;
		fx->speed = speed;
		fx->fallspeed = 0;
		fx->flag1 = 1;
		fx->flag2 = (type == 2 ? 1 : 0);
	}
}

void ControlLondBossPlasmaBall(int16_t fx_number)
{
	auto fx = &effects[fx_number];

	int old_y = fx->pos.y_pos;

	++fx->fallspeed;

	if (fx->speed > 8)			 fx->speed -= 2;
	if (fx->pos.x_rot > -0x3c00) fx->pos.x_rot -= 0x100;

	int speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;

	fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
	fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
	fx->pos.y_pos += -((fx->speed * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT) + fx->fallspeed;

	if ((wibble & 15) == 0)
		TriggerPlasmaBallFlameLB(fx_number, 0, 0, abs(old_y - fx->pos.y_pos) << 3, 0);

	auto room_number = fx->room_number;
	auto floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

	if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) ||
		fx->pos.y_pos < GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos))
	{
		return KillEffect(fx_number);
	}

	if (room[room_number].flags & UNDERWATER)
		return KillEffect(fx_number);

	if (!fx->flag2 && ItemNearLara(lara_item, &fx->pos, 200))
	{
		lara_item->hit_points -= 25;
		lara_item->hit_status = 1;
		
		return KillEffect(fx_number);
	}

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, lara_item->room_number);

	int rnd = GetRandomControl(),
		g = 31 - ((rnd >> 4) & 3),
		b = 24 - ((rnd >> 6) & 3),
		r = rnd & 7;

	static constexpr uint8_t radtab[2] = { 13, 7 };

	TriggerDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, radtab[fx->flag1], r, g, b);
}

long KnockBackCollision(EXPLOSION_RING* erptr)
{
	int radius = erptr->radius,
		x = lara_item->pos.x_pos - erptr->x,
		z = lara_item->pos.z_pos - erptr->z,
		dist = (x > 16000 || x < -16000 || z > 16000 || z < -16000 ? 0x7FFF : SQUARE(x) + SQUARE(z));

	if (dist < SQUARE(radius))
	{
		lara_item->hit_points -= 200;
		lara_item->hit_status = 1;

		int16_t angle = phd_atan(z, x),
			   diff = lara_item->pos.y_rot - angle;

		if (ABS(diff) < 0x4000)
		{
			lara_item->speed = 75;
			lara_item->pos.y_rot = angle;
		}
		else
		{
			lara_item->pos.y_rot = (int16_t)(angle - 0x8000);
			lara_item->speed = -75;
		}

		lara_item->gravity_status = 1;
		lara_item->fallspeed = -50;
		lara_item->pos.x_rot = lara_item->pos.z_rot = 0;
		lara_item->anim_number = FALLDOWN_A;
		lara_item->frame_number = FALLDOWN_F;
		lara_item->current_anim_state = AS_FORWARDJUMP;
		lara_item->goal_anim_state = AS_FORWARDJUMP;

		TriggerExplosionSparks(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, 3, -2, 2, lara_item->room_number);

		for (int i = 0; i < 3; ++i)
			TriggerPlasmaBallLondonBoss(lara_item, 2, (PHD_VECTOR*)&lara_item->pos, lara_item->room_number, GetRandomControl() << 1);

		return 1;
	}

	return 0;
}