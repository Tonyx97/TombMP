#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "control.h"
#include "camera.h"
#include "sphere.h"
#include "lot.h"
#include "types.h"
#include "missile.h"

#include <specific/fn_stubs.h>

#include <shared/defs.h>

#include <mp/client.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#define CLIP_LEFT				1
#define CLIP_RIGHT				2
#define CLIP_TOP				4
#define CLIP_BOTTOM				8
#define ALL_CLIP				(CLIP_LEFT | CLIP_RIGHT | CLIP_TOP | CLIP_BOTTOM)
#define SECONDARY_CLIP			16
#define BIFF					(WALL_L >> 1)
#define BIFF_AVOID_TURN			0x800
#define MAX_X_ROT				(ONE_DEGREE * 20)
#define CREATURE_FLOAT_SPEED	(WALL_L / 32)
#define HEAD_ARC				0x3000
#define JOINT_ARC				0x3000
#define FEELER_DISTANCE			512
#define FEELER_ANGLE			(45 * ONE_DEGREE)
#define REACHED_GOAL_RADIUS		(STEP_L * 3)
#define GUNFIRE_ALERT_RADIUS	8000
#define PREDICTIVE_SCALE_FACTOR 14
#define NULL_OBJECT				-1
#define STALK_DIST				(WALL_L * 3)
#define ESCAPE_DIST				(WALL_L * 5)
#define ATTACK_RANGE			SQUARE(WALL_L * 3)
#define ESCAPE_CHANCE			0x800
#define RECOVER_CHANCE			0x100

int StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, int16_t box_number)
{
	if (!enemy)
		return 0;

	auto box = &boxes[box_number];

	int z = (((uint32_t)box->left + (uint32_t)box->right) << (WALL_SHIFT - 1)) - enemy->pos.z_pos,
		x = (((uint32_t)box->top + (uint32_t)box->bottom) << (WALL_SHIFT - 1)) - enemy->pos.x_pos,
		zrange = STALK_DIST + (((uint32_t)box->right - (uint32_t)box->left) << WALL_SHIFT),
		xrange = STALK_DIST + (((uint32_t)box->bottom - (uint32_t)box->top) << WALL_SHIFT);

	if (x > xrange || x < -xrange || z > zrange || z < -zrange)
		return 0;

	int enemy_quad = (enemy->pos.y_rot >> 14) + 2,
		box_quad = (z > 0 ? ((x > 0) ? 2 : 1) : ((x > 0) ? 3 : 0));

	if (enemy_quad == box_quad)
		return 0;

	int baddie_quad = (item->pos.z_pos > enemy->pos.z_pos ? ((item->pos.x_pos > enemy->pos.x_pos) ? 2 : 1)
														  : ((item->pos.x_pos > enemy->pos.x_pos) ? 3 : 0));

	return (enemy_quad != baddie_quad || abs(enemy_quad - box_quad) != 2);
}

void InitialiseCreature(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->object_number != ELECTRIC_CLEANER && item->object_number != SHIVA && item->object_number != TARGETS)
		item->pos.y_rot += (GetRandomControl() - 0x4000) >> 1;

	item->collidable = 1;
	item->data = nullptr;
}

int CreatureActive(int16_t item_number)
{
	if (auto item = &items[item_number]; item->status == INVISIBLE)
	{
		if (!EnableBaddieAI(item_number, 0))
			return 0;

		item->status = ACTIVE;
	}

	return 1;
}

void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info)
{
	if (!item->data)
		return;

	auto creature = (CREATURE_INFO*)item->data;
	auto enemy = creature->enemy;

	if (!enemy)
	{
		enemy = lara_item;
		creature->enemy = lara_item;
	}

	auto zone = (creature->LOT.fly == NO_FLYING ? ground_zone[ZONE(creature->LOT.step)][flip_status]
												: ground_zone[ZONE(1)][0]);
	
	auto r = &room[item->room_number];

	item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;
	info->zone_number = (creature->LOT.fly == NO_FLYING ? zone[item->box_number] : FLY_ZONE);

	r = &room[enemy->room_number];
	enemy->box_number = r->floor[((enemy->pos.z_pos - r->z) >> WALL_SHIFT) + ((enemy->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;

	info->enemy_zone = (creature->LOT.fly == NO_FLYING ? zone[enemy->box_number] : FLY_ZONE);

	if (!objects[item->object_number].non_lot)
	{
		if (boxes[enemy->box_number].overlap_index & creature->LOT.block_mask)										   info->enemy_zone |= BLOCKED;
		else if (creature->LOT.node[item->box_number].search_number == (creature->LOT.search_number | BLOCKED_SEARCH)) info->enemy_zone |= BLOCKED;
	}

	auto object = &objects[item->object_number];

	int x, z;

	if (enemy == lara_item)
	{
		z = (enemy->pos.z_pos + (enemy->speed * PREDICTIVE_SCALE_FACTOR * phd_cos(lara.move_angle) >> W2V_SHIFT)) - (item->pos.z_pos + (object->pivot_length * phd_cos(item->pos.y_rot) >> W2V_SHIFT));
		x = (enemy->pos.x_pos + (enemy->speed * PREDICTIVE_SCALE_FACTOR * phd_sin(lara.move_angle) >> W2V_SHIFT)) - (item->pos.x_pos + (object->pivot_length * phd_sin(item->pos.y_rot) >> W2V_SHIFT));
	}
	else
	{
		z = (enemy->pos.z_pos + (enemy->speed * PREDICTIVE_SCALE_FACTOR * phd_cos(enemy->pos.y_rot) >> W2V_SHIFT)) - (item->pos.z_pos + (object->pivot_length * phd_cos(item->pos.y_rot) >> W2V_SHIFT));
		x = (enemy->pos.x_pos + (enemy->speed * PREDICTIVE_SCALE_FACTOR * phd_sin(enemy->pos.y_rot) >> W2V_SHIFT)) - (item->pos.x_pos + (object->pivot_length * phd_sin(item->pos.y_rot) >> W2V_SHIFT));
	}

	int y = item->pos.y_pos - enemy->pos.y_pos,
		angle = phd_atan(z, x);

	if (z > 32000 || z < -32000 || x>32000 || x < -32000)
		info->distance = 0x7fffffff;
	else info->distance = (creature->enemy ? z * z + x * x : 0x7fffffff);

	info->angle = angle - item->pos.y_rot;
	info->enemy_facing = 0x8000 + angle - enemy->pos.y_rot;

	z = abs(z);
	x = abs(x);

	if (enemy == lara_item)
	{
		int16_t lara_anim = lara_item->current_anim_state;

		if (lara_anim == AS_DUCK ||
			lara_anim == AS_DUCKROLL ||
			lara_anim == AS_ALL4S ||
			lara_anim == AS_CRAWL ||
			lara_anim == AS_ALL4TURNL ||
			lara_anim == AS_ALL4TURNR)
		{
			y -= (STEP_L * 3 / 2);
		}
	}

	info->x_angle = (x > z ? phd_atan(x + (z >> 1), y) : phd_atan(z + (x >> 1), y));
	info->ahead = (info->angle > -FRONT_ARC && info->angle < FRONT_ARC);
	info->bite = (info->ahead && enemy->hit_points > 0 && ABS(enemy->pos.y_pos - item->pos.y_pos) <= STEP_L * 2);
}

int SearchLOT(LOT_INFO* LOT, int expansion)
{
	int16_t* zone,
			search_zone;

	if (LOT->fly == NO_FLYING)
	{
		zone = ground_zone[ZONE(LOT->step)][flip_status];
		search_zone = zone[LOT->head];
	}
	else
	{
		zone = ground_zone[ZONE(1)][0];
		search_zone = FLY_ZONE;
	}

	for (int i = 0; i < expansion; ++i)
	{
		if (LOT->head == NO_BOX)
		{
			LOT->tail = NO_BOX;
			return 0;
		}

		auto node = &LOT->node[LOT->head];
		auto box = &boxes[LOT->head];

		int index = box->overlap_index & OVERLAP_INDEX,
			done = 0;

		do {
			int box_number = overlap[index++];
			if (box_number & BOX_END_BIT)
			{
				done = 1;
				box_number &= BOX_NUMBER;
			}

			if (LOT->fly == NO_FLYING && search_zone != zone[box_number])
				continue;

			int change = boxes[box_number].height - box->height;
			if (change > LOT->step || change < LOT->drop)
				continue;

			auto expand = &LOT->node[box_number];

			if ((node->search_number & SEARCH_NUMBER) < (expand->search_number & SEARCH_NUMBER))
				continue;

			if (node->search_number & BLOCKED_SEARCH)
			{
				if ((node->search_number & SEARCH_NUMBER) == (expand->search_number & SEARCH_NUMBER))
					continue;

				expand->search_number = node->search_number;
			}
			else
			{
				if ((node->search_number & SEARCH_NUMBER) == (expand->search_number & SEARCH_NUMBER) && !(expand->search_number & BLOCKED_SEARCH))
					continue;

				if (boxes[box_number].overlap_index & LOT->block_mask)
					expand->search_number = node->search_number | BLOCKED_SEARCH;
				else
				{
					expand->search_number = node->search_number;
					expand->exit_box = LOT->head;
				}
			}

			if (expand->next_expansion == NO_BOX && box_number != LOT->tail)
			{
				LOT->node[LOT->tail].next_expansion = box_number;
				LOT->tail = box_number;
			}
		} while (!done);

		LOT->head = node->next_expansion;

		node->next_expansion = NO_BOX;
	}

	return 1;
}

int UpdateLOT(LOT_INFO* LOT, int expansion)
{
	if (LOT->required_box != NO_BOX && LOT->required_box != LOT->target_box)
	{
		LOT->target_box = LOT->required_box;

		auto expand = &LOT->node[LOT->target_box];

		if (expand->next_expansion == NO_BOX && LOT->tail != LOT->target_box)
		{
			expand->next_expansion = LOT->head;

			if (LOT->head == NO_BOX)
				LOT->tail = LOT->target_box;

			LOT->head = LOT->target_box;
		}

		expand->search_number = ++LOT->search_number;
		expand->exit_box = NO_BOX;
	}

	return SearchLOT(LOT, expansion);
}

void TargetBox(LOT_INFO* LOT, int16_t box_number)
{
	box_number &= BOX_NUMBER;

	auto box = &boxes[box_number];

	LOT->target.z = (((uint32_t)box->left << WALL_SHIFT)) + GetRandomControl() * (((uint32_t)box->right - (uint32_t)box->left - 1) >> (15 - WALL_SHIFT)) + WALL_L / 2;
	LOT->target.x = ((uint32_t)box->top << WALL_SHIFT) + GetRandomControl() * (((uint32_t)box->bottom - (uint32_t)box->top - 1) >> (15 - WALL_SHIFT)) + WALL_L / 2;
	LOT->required_box = box_number;

	if (LOT->fly == NO_FLYING)
		LOT->target.y = box->height;
	else LOT->target.y = box->height - STEP_L * 3 / 2;
}

int EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, int16_t box_number)
{
	auto box = &boxes[box_number];

	int z = (((uint32_t)box->left + (uint32_t)box->right) << (WALL_SHIFT - 1)) - enemy->pos.z_pos,
		x = (((uint32_t)box->top + (uint32_t)box->bottom) << (WALL_SHIFT - 1)) - enemy->pos.x_pos;

	if (x > -ESCAPE_DIST && x < ESCAPE_DIST && z > -ESCAPE_DIST && z < ESCAPE_DIST)
		return 0;

	if ((z > 0) ^ (item->pos.z_pos > enemy->pos.z_pos) &&
		(x > 0) ^ (item->pos.x_pos > enemy->pos.x_pos))
		return 0;

	return 1;
}

int ValidBox(ITEM_INFO* item, int16_t zone_number, int16_t box_number)
{
	auto creature = (CREATURE_INFO*)item->data;

	auto zone = (creature->LOT.fly == NO_FLYING ? ground_zone[ZONE(creature->LOT.step)][flip_status]
												: ground_zone[ZONE(1)][0]);

	if (creature->LOT.fly == NO_FLYING && zone[box_number] != zone_number)
		return 0;

	auto box = &boxes[box_number];

	if (box->overlap_index & creature->LOT.block_mask)
		return 0;

	return (item->pos.z_pos <= ((int32_t)box->left << WALL_SHIFT) ||
			item->pos.z_pos >= ((int32_t)box->right << WALL_SHIFT) ||
			item->pos.x_pos <= ((int32_t)box->top << WALL_SHIFT) ||
			item->pos.x_pos >= ((int32_t)box->bottom << WALL_SHIFT));
}

void GetCreatureMood(ITEM_INFO* item, AI_INFO* info, int violent)
{
	if (!item->data)
		return;

	auto creature = (CREATURE_INFO*)item->data;
	auto enemy = creature->enemy;
	auto LOT = &creature->LOT;

	if (LOT->node[item->box_number].search_number == (LOT->search_number | BLOCKED_SEARCH))
		LOT->required_box = NO_BOX;

	if (creature->mood != ATTACK_MOOD && LOT->required_box != NO_BOX)
	{
		if (!ValidBox(item, info->zone_number, LOT->target_box))
		{
			if (info->zone_number == info->enemy_zone)
				creature->mood = BORED_MOOD;

			LOT->required_box = NO_BOX;
		}
	}

	auto mood = creature->mood;

	if (!enemy)
	{
		creature->mood = BORED_MOOD;
		enemy = lara_item;
	}
	else if (enemy->hit_points <= 0 && enemy == lara_item)
		creature->mood = BORED_MOOD;
	else if (violent)
	{
		switch (creature->mood)
		{
		case ATTACK_MOOD:
		{
			if (info->zone_number != info->enemy_zone)
				creature->mood = BORED_MOOD;

			break;
		}
		case BORED_MOOD:
		case STALK_MOOD:
			creature->mood = (info->zone_number == info->enemy_zone ? ATTACK_MOOD : ESCAPE_MOOD);
			break;
		case ESCAPE_MOOD:
		{
			if (info->zone_number == info->enemy_zone)
				creature->mood = ATTACK_MOOD;
		}
		}
	}
	else
	{
		switch (creature->mood)
		{
		case ATTACK_MOOD:
		{
			if (item->hit_status && (GetRandomControl() < ESCAPE_CHANCE || info->zone_number != info->enemy_zone))
				creature->mood = STALK_MOOD;
			else if (info->zone_number != info->enemy_zone && info->distance > (WALL_L * 6))
				creature->mood = BORED_MOOD;

			break;
		}
		case BORED_MOOD:
		case STALK_MOOD:
		{
			if (creature->alerted && info->zone_number != info->enemy_zone)
				creature->mood = (info->distance > (WALL_L * 3) ? STALK_MOOD : BORED_MOOD);
			else if (info->zone_number == info->enemy_zone)
				creature->mood = (info->distance < ATTACK_RANGE || (creature->mood == STALK_MOOD && LOT->required_box == NO_BOX) ? ATTACK_MOOD : STALK_MOOD);

			break;
		}
		case ESCAPE_MOOD:
		{
			if (info->zone_number == info->enemy_zone && GetRandomControl() < RECOVER_CHANCE)
				creature->mood = STALK_MOOD;
		}
		}
	}

	if (mood != creature->mood)
	{
		if (mood == ATTACK_MOOD)
			TargetBox(LOT, LOT->target_box);

		LOT->required_box = NO_BOX;
	}
}

void CreatureMood(ITEM_INFO* item, AI_INFO* info, int violent)
{
	int16_t* bounds;

	if (!item->data)
		return;

	auto creature = (CREATURE_INFO*)item->data;
	auto enemy = creature->enemy;
	auto LOT = &creature->LOT;

	switch (creature->mood)
	{
	case ATTACK_MOOD:
	{
		LOT->target.x = enemy->pos.x_pos;
		LOT->target.y = enemy->pos.y_pos;
		LOT->target.z = enemy->pos.z_pos;
		LOT->required_box = enemy->box_number;

		if (LOT->fly != NO_FLYING)
		{
			auto object = &objects[enemy->object_number];

			if (lara.water_status == LARA_ABOVEWATER)
			{
				bounds = GetBestFrame(enemy);

				LOT->target.y += bounds[2];
			}
		}

		break;
	}
	case BORED_MOOD:
	{
		auto box_number = LOT->node[GetRandomControl() * LOT->zone_count >> 15].box_number;

		if (ValidBox(item, info->zone_number, box_number))
		{
			if (StalkBox(item, enemy, box_number) && enemy->hit_points > 0 && creature->enemy)
			{
				TargetBox(LOT, box_number);

				creature->mood = BORED_MOOD;
			}
			else if (LOT->required_box == NO_BOX)
				TargetBox(LOT, box_number);
		}

		break;
	}
	case STALK_MOOD:
	{
		if (LOT->required_box == NO_BOX || !StalkBox(item, enemy, LOT->required_box))
		{
			auto box_number = LOT->node[GetRandomControl() * LOT->zone_count >> 15].box_number;

			if (ValidBox(item, info->zone_number, box_number))
			{
				if (StalkBox(item, enemy, box_number))
					TargetBox(LOT, box_number);
				else if (LOT->required_box == NO_BOX)
				{
					TargetBox(LOT, box_number);

					if (info->zone_number != info->enemy_zone)
						creature->mood = BORED_MOOD;
				}
			}
		}

		break;
	}
	case ESCAPE_MOOD:
	{
		auto box_number = LOT->node[GetRandomControl() * LOT->zone_count >> 15].box_number;

		if (ValidBox(item, info->zone_number, box_number))
		{
			if (LOT->required_box == NO_BOX)
			{
				if (EscapeBox(item, enemy, box_number))
					TargetBox(LOT, box_number);
				else if (info->zone_number == info->enemy_zone && StalkBox(item, enemy, box_number) && !violent)
				{
					TargetBox(LOT, box_number);

					creature->mood = STALK_MOOD;
				}
			}
		}
	}
	}

	if (LOT->target_box == NO_BOX)
		TargetBox(LOT, item->box_number);

	CalculateTarget(&creature->target, item, &creature->LOT);
}

TARGET_TYPE CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT)
{
	UpdateLOT(LOT, MAX_EXPANSION);

	target->x = item->pos.x_pos;
	target->y = item->pos.y_pos;
	target->z = item->pos.z_pos;

	auto box_number = item->box_number;

	if (box_number == NO_BOX)
		return NO_TARGET;

	auto box = &boxes[box_number];

	int box_left = (int32_t)box->left << WALL_SHIFT,
		box_right = ((int32_t)box->right << WALL_SHIFT) - 1,
		box_top = (int32_t)box->top << WALL_SHIFT,
		box_bottom = ((int32_t)box->bottom << WALL_SHIFT) - 1,
		left = box_left,
		right = box_right,
		top = box_top,
		bottom = box_bottom;

	int prime_free = ALL_CLIP;

	do {
		box = &boxes[box_number];

		if (LOT->fly == NO_FLYING)
		{
			if (target->y > box->height)
				target->y = box->height;
		}
		else if (target->y > box->height - WALL_L)
			target->y = box->height - WALL_L;

		box_left = (int32_t)box->left << WALL_SHIFT;
		box_right = ((int32_t)box->right << WALL_SHIFT) - 1;
		box_top = (int32_t)box->top << WALL_SHIFT;
		box_bottom = ((int32_t)box->bottom << WALL_SHIFT) - 1;

		if (item->pos.z_pos >= box_left && item->pos.z_pos <= box_right &&
			item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom)
		{
			left = box_left;
			right = box_right;
			top = box_top;
			bottom = box_bottom;
		}
		else
		{
			if (item->pos.z_pos < box_left)
			{
				if ((prime_free & CLIP_LEFT) && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom)
				{
					if (target->z < box_left + BIFF) target->z = box_left + BIFF;
					if (prime_free & SECONDARY_CLIP) return SECONDARY_TARGET;
					if (box_top > top)				 top = box_top;
					if (box_bottom < bottom)		 bottom = box_bottom;

					prime_free = CLIP_LEFT;
				}
				else if (prime_free != CLIP_LEFT)
				{
					target->z = right - BIFF;

					if (prime_free != ALL_CLIP)
						return (SECONDARY_TARGET);

					prime_free |= SECONDARY_CLIP;
				}
			}
			else if (item->pos.z_pos > box_right)
			{
				if ((prime_free & CLIP_RIGHT) && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom)
				{
					if (target->z > box_right - BIFF) target->z = box_right - BIFF;
					if (prime_free & SECONDARY_CLIP)  return SECONDARY_TARGET;
					if (box_top > top)				  top = box_top;
					if (box_bottom < bottom)		  bottom = box_bottom;

					prime_free = CLIP_RIGHT;
				}
				else if (prime_free != CLIP_RIGHT)
				{
					target->z = left + BIFF;

					if (prime_free != ALL_CLIP)
						return (SECONDARY_TARGET);

					prime_free |= SECONDARY_CLIP;
				}
			}

			if (item->pos.x_pos < box_top)
			{
				if ((prime_free & CLIP_TOP) && item->pos.z_pos >= box_left && item->pos.z_pos <= box_right)
				{
					if (target->x < box_top + BIFF)  target->x = box_top + BIFF;
					if (prime_free & SECONDARY_CLIP) return (SECONDARY_TARGET);
					if (box_left > left)			 left = box_left;
					if (box_right < right)			 right = box_right;

					prime_free = CLIP_TOP;
				}
				else if (prime_free != CLIP_TOP)
				{
					target->x = bottom - BIFF;

					if (prime_free != ALL_CLIP)
						return (SECONDARY_TARGET);

					prime_free |= SECONDARY_CLIP;
				}
			}
			else if (item->pos.x_pos > box_bottom)
			{
				if ((prime_free & CLIP_BOTTOM) && item->pos.z_pos >= box_left && item->pos.z_pos <= box_right)
				{
					if (target->x > box_bottom - BIFF)  target->x = box_bottom - BIFF;
					if (prime_free & SECONDARY_CLIP)	return SECONDARY_TARGET;
					if (box_left > left)				left = box_left;
					if (box_right < right)				right = box_right;

					prime_free = CLIP_BOTTOM;
				}
				else if (prime_free != CLIP_BOTTOM)
				{
					target->x = top + BIFF;

					if (prime_free != ALL_CLIP)
						return (SECONDARY_TARGET);

					prime_free |= SECONDARY_CLIP;
				}
			}
		}

		if (box_number == LOT->target_box)
		{
			if (prime_free & (CLIP_LEFT | CLIP_RIGHT))
				target->z = LOT->target.z;
			else if (!(prime_free & SECONDARY_CLIP))
			{
				if (target->z < box_left + BIFF)	   target->z = box_left + BIFF;
				else if (target->z > box_right - BIFF) target->z = box_right - BIFF;
			}

			if (prime_free & (CLIP_TOP | CLIP_BOTTOM))
				target->x = LOT->target.x;
			else if (!(prime_free & SECONDARY_CLIP))
			{
				if (target->x < box_top + BIFF)			target->x = box_top + BIFF;
				else if (target->x > box_bottom - BIFF) target->x = box_bottom - BIFF;
			}

			target->y = LOT->target.y;

			return PRIME_TARGET;
		}

		box_number = LOT->node[box_number].exit_box;

		if (box_number != NO_BOX && (boxes[box_number].overlap_index & LOT->block_mask))
			break;

	} while (box_number != NO_BOX);

	if (prime_free & (CLIP_LEFT | CLIP_RIGHT))
		target->z = box_left + WALL_L / 2 + (GetRandomControl() * (box_right - box_left - WALL_L) >> 15);
	else if (!(prime_free & SECONDARY_CLIP))
	{
		if (target->z < box_left + BIFF)	   target->z = box_left + BIFF;
		else if (target->z > box_right - BIFF) target->z = box_right - BIFF;
	}

	if (prime_free & (CLIP_TOP | CLIP_BOTTOM))
		target->x = box_top + WALL_L / 2 + (GetRandomControl() * (box_bottom - box_top - WALL_L) >> 15);
	else if (!(prime_free & SECONDARY_CLIP))
	{
		if (target->x < box_top + BIFF)			target->x = box_top + BIFF;
		else if (target->x > box_bottom - BIFF) target->x = box_bottom - BIFF;
	}

	if (LOT->fly == NO_FLYING) target->y = box->height;
	else					   target->y = box->height - STEP_L * 3 / 2;

	return NO_TARGET;
}

int CreatureCreature(int16_t item_number)
{
	auto item = &items[item_number];

	int x = item->pos.x_pos,
		y = item->pos.y_pos,
		z = item->pos.z_pos,
		y_rot = item->pos.y_rot,
		radius = objects[item->object_number].radius;

	auto r = &room[item->room_number];

	int16_t link = r->item_number;

	do {
		item = &items[link];

		if (link == item_number)
			return 0;

		if (item != lara_item && item->status == ACTIVE && item->hit_points > 0)
		{
			int xdistance = abs(item->pos.x_pos - x),
				zdistance = abs(item->pos.z_pos - z),
				distance = (xdistance > zdistance ? xdistance + (zdistance >> 1) : zdistance + (xdistance >> 1));

			if (distance < radius + objects[item->object_number].radius)
				return (phd_atan(item->pos.z_pos - z, item->pos.x_pos - x) - y_rot);
		}

		link = item->next_item;
	} while (link != NO_ITEM);

	return 0;
}

int BadFloor(int32_t x, int32_t y, int32_t z, int32_t box_height, int32_t next_height, int16_t room_number, LOT_INFO* LOT)
{
	auto floor = GetFloor(x, y, z, &room_number);

	if (floor->box == NO_BOX)
		return 1;

	if (boxes[floor->box].overlap_index & LOT->block_mask)
		return 1;

	int height = boxes[floor->box].height;

	if (box_height - height > LOT->step || box_height - height < LOT->drop)
		return 1;

	if (box_height - height < -LOT->step && height > next_height)
		return 1;

	if (LOT->fly != NO_FLYING && y > height + LOT->fly)
		return 1;

	return 0;
}

void CreatureDie(int16_t item_number, bool explode, bool is_player, bool sync_explode)
{
	if (is_player && !g_client->get_game_settings().friendly_fire)
		return;

	auto item = &items[item_number];
	
	item->collidable = 0;
	item->hit_points = DONT_TARGET;

	if (explode)
	{
		auto localplayer = game_level::LOCALPLAYER();

		if (sync_explode)
		{
			SYNC_ID sid;

			if (auto entity = g_level->get_entity_by_item(item))
				sid = entity->get_sync_id();
			else if (item == lara_item)
			{
				localplayer->set_health(DONT_TARGET);
				localplayer->set_entity_flags(ENTITY_FLAG_INVISIBLE);

				sid = localplayer->get_sync_id();
			}

			g_client->send_packet(ID_ENTITY_EXPLODE, gns::entity::explode
			{
				.sid = sid
			});
		}

		ExplodingDeath(item_number, 0xffffffff, 0);

		if (!is_player)
			KillItem(item_number, sync_explode);
		else
		{
			if (auto player = g_level->get_player_by_item(item))
				player->add_entity_flags(ENTITY_FLAG_INVISIBLE);
			else if (localplayer->get_item() == item)
				localplayer->add_entity_flags(ENTITY_FLAG_INVISIBLE);

			item->ai_bits |= EXPLODED;
		}
	}
	else RemoveActiveItem(item_number);

	if (!is_player)
	{
		DisableBaddieAI(item_number);

		item->flags |= ONESHOT;
	}

	if (item->clear_body)
	{
		item->next_active = body_bag;
		body_bag = item_number;
	}

	auto pickup_number = item->carried_item;

	while (pickup_number != NO_ITEM)
	{
		auto pickup = &items[pickup_number];

		pickup->pos.x_pos = (item->pos.x_pos & ~1023) | 512;
		pickup->pos.z_pos = (item->pos.z_pos & ~1023) | 512;

		auto room_number = item->room_number;
		auto floor = GetFloor(pickup->pos.x_pos, item->pos.y_pos, pickup->pos.z_pos, &room_number);

		pickup->pos.y_pos = GetHeight(floor, pickup->pos.x_pos, item->pos.y_pos, pickup->pos.z_pos);

		ItemNewRoom(pickup_number, item->room_number);

		pickup_number = pickup->carried_item;
	}
}

int CreatureAnimation(int16_t item_number, int16_t angle, int16_t tilt)
{
	auto item = &items[item_number];

	if (!item->data)
		return 0;

	auto creature = (CREATURE_INFO*)item->data;
	auto LOT = &creature->LOT;

	PHD_VECTOR old { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos };

	int box_height = boxes[item->box_number].height;

	auto zone = (LOT->fly == NO_FLYING ? ground_zone[ZONE(LOT->step)][flip_status] : ground_zone[ZONE(1)][0]);

	if (!objects[item->object_number].water_creature)
	{
		auto room_number = item->room_number;

		GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (room_number != item->room_number)
			ItemNewRoom(item_number, room_number);
	}

	AnimateItem(item);

	if (item->status == DEACTIVATED)
	{
		CreatureDie(item_number, false);

		return 0;
	}

	auto bounds = GetBoundsAccurate(item);

	int y = item->pos.y_pos + bounds[2];

	auto room_number = item->room_number;

	GetFloor(old.x, y, old.z, &room_number);

	auto floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);

	int height = boxes[floor->box].height,
		next_box;

	if (!objects[item->object_number].non_lot)
		next_box = LOT->node[floor->box].exit_box;
	else
	{
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
		height = boxes[floor->box].height;
		next_box = floor->box;
	}

	int next_height = height;

	if (next_box != NO_BOX)
		next_height = boxes[next_box].height;

	if (floor->box == NO_BOX || (LOT->fly == NO_FLYING && zone[item->box_number] != zone[floor->box]) ||
		box_height - height > LOT->step || box_height - height < LOT->drop)
	{
		int pos_x = item->pos.x_pos >> WALL_SHIFT,
			pos_z = item->pos.z_pos >> WALL_SHIFT;

		int shift_x = old.x >> WALL_SHIFT,
			shift_z = old.z >> WALL_SHIFT;

		if (pos_x < shift_x)	  item->pos.x_pos = old.x & (~(WALL_L - 1));
		else if (pos_x > shift_x) item->pos.x_pos = old.x | (WALL_L - 1);

		if (pos_x < shift_z)	  item->pos.z_pos = old.z & (~(WALL_L - 1));
		else if (pos_x > shift_z) item->pos.z_pos = old.z | (WALL_L - 1);

		floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
		height = boxes[floor->box].height;

		if (!objects[item->object_number].non_lot)
			next_box = LOT->node[floor->box].exit_box;
		else
		{
			floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
			height = boxes[floor->box].height;
			next_box = floor->box;
		}

		next_height = (next_box != NO_BOX ? boxes[next_box].height : height);
	}

	int x = item->pos.x_pos,
		z = item->pos.z_pos,
		pos_x = x & (WALL_L - 1),
		pos_z = z & (WALL_L - 1);

	int radius = objects[item->object_number].radius,
		shift_x = 0,
		shift_z = 0;

	if (pos_z < radius)
	{
		if (BadFloor(x, y, z - radius, height, next_height, room_number, LOT))
			shift_z = radius - pos_z;

		if (pos_x < radius)
		{
			if (BadFloor(x - radius, y, z, height, next_height, room_number, LOT))
				shift_x = radius - pos_x;
			else if (!shift_z && BadFloor(x - radius, y, z - radius, height, next_height, room_number, LOT))
			{
				if (item->pos.y_rot > -0x6000 && item->pos.y_rot < 0x2000)
					shift_z = radius - pos_z;
				else shift_x = radius - pos_x;
			}
		}
		else if (pos_x > WALL_L - radius)
		{
			if (BadFloor(x + radius, y, z, height, next_height, room_number, LOT))
				shift_x = WALL_L - radius - pos_x;
			else if (!shift_z && BadFloor(x + radius, y, z - radius, height, next_height, room_number, LOT))
			{
				if (item->pos.y_rot > -0x2000 && item->pos.y_rot < 0x6000)
					shift_z = radius - pos_z;
				else shift_x = WALL_L - radius - pos_x;
			}
		}
	}
	else if (pos_z > WALL_L - radius)
	{
		if (BadFloor(x, y, z + radius, height, next_height, room_number, LOT))
			shift_z = WALL_L - radius - pos_z;

		if (pos_x < radius)
		{
			if (BadFloor(x - radius, y, z, height, next_height, room_number, LOT))
				shift_x = radius - pos_x;
			else if (!shift_z && BadFloor(x - radius, y, z + radius, height, next_height, room_number, LOT))
			{
				if (item->pos.y_rot > -0x2000 && item->pos.y_rot < 0x6000)
					shift_x = radius - pos_x;
				else shift_z = WALL_L - radius - pos_z;
			}
		}
		else if (pos_x > WALL_L - radius)
		{
			if (BadFloor(x + radius, y, z, height, next_height, room_number, LOT))
				shift_x = WALL_L - radius - pos_x;
			else if (!shift_z && BadFloor(x + radius, y, z + radius, height, next_height, room_number, LOT))
			{
				if (item->pos.y_rot > -0x6000 && item->pos.y_rot < 0x2000)
					shift_x = WALL_L - radius - pos_x;
				else shift_z = WALL_L - radius - pos_z;
			}
		}
	}
	else if (pos_x < radius)
	{
		if (BadFloor(x - radius, y, z, height, next_height, room_number, LOT))
			shift_x = radius - pos_x;
	}
	else if (pos_x > WALL_L - radius && BadFloor(x + radius, y, z, height, next_height, room_number, LOT))
		shift_x = WALL_L - radius - pos_x;

	item->pos.x_pos += shift_x;
	item->pos.z_pos += shift_z;

	if (shift_x || shift_z)
	{
		floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
		
		item->pos.y_rot += angle;

		if (tilt)
			CreatureTilt(item, (int16_t)(tilt * 2));
	}

	if (int biff_angle = (item->object_number != TREX && item->speed && item->hit_points > 0 ? CreatureCreature(item_number) : 0))
	{
		if (abs(biff_angle) < BIFF_AVOID_TURN) item->pos.y_rot -= biff_angle;
		else if (biff_angle > 0)			   item->pos.y_rot -= BIFF_AVOID_TURN;
		else								   item->pos.y_rot += BIFF_AVOID_TURN;

		return 1;
	}

	if (LOT->fly != NO_FLYING)
	{
		int dy = creature->target.y - item->pos.y_pos;

		if (dy > LOT->fly)		 dy = LOT->fly;
		else if (dy < -LOT->fly) dy = -LOT->fly;

		if (item->pos.y_pos + dy > (height = GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos)))
		{
			if (item->pos.y_pos > height)
			{
				item->pos.x_pos = old.x;
				item->pos.z_pos = old.z;

				dy = -LOT->fly;
			}
			else
			{
				dy = 0;

				item->pos.y_pos = height;
			}
		}
		else if (!objects[item->object_number].water_creature)
		{
			floor = GetFloor(item->pos.x_pos, y + STEP_L, item->pos.z_pos, &room_number);

			if (room[room_number].flags & (UNDERWATER | SWAMP))
				dy = -LOT->fly;
		}
		else
		{
			int ceiling = GetCeiling(floor, item->pos.x_pos, y, item->pos.z_pos);

			int top = (item->object_number == WHALE ? STEP_L / 2 : bounds[2]);

			if (item->pos.y_pos + top + dy < ceiling)
			{
				if (item->pos.y_pos + top < ceiling)
				{
					item->pos.x_pos = old.x;
					item->pos.z_pos = old.z;
					dy = LOT->fly;
				}
				else
					dy = 0;
			}
		}

		item->pos.y_pos += dy;
		floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
		item->floor = GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos);

		angle = (item->speed) ? phd_atan(item->speed, -dy) : 0;

		if (angle < -MAX_X_ROT)		angle = -MAX_X_ROT;
		else if (angle > MAX_X_ROT) angle = MAX_X_ROT;

		if (angle < item->pos.x_rot - ONE_DEGREE)	   item->pos.x_rot -= ONE_DEGREE;
		else if (angle > item->pos.x_rot + ONE_DEGREE) item->pos.x_rot += ONE_DEGREE;
		else										   item->pos.x_rot = angle;
	}
	else
	{
		floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);

		int ceiling = GetCeiling(floor, item->pos.x_pos, y, item->pos.z_pos),
			top = (item->object_number == TREX || item->object_number == SHIVA || item->object_number == MUTANT2 ? STEP_L * 3 : bounds[2]);

		if (item->pos.y_pos + top < ceiling)
		{
			item->pos.x_pos = old.x;
			item->pos.z_pos = old.z;
			item->pos.y_pos = old.y;
		}

		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
		item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (item->pos.y_pos > item->floor)					 item->pos.y_pos = item->floor;
		else if (item->floor - item->pos.y_pos > STEP_L / 4) item->pos.y_pos += STEP_L / 4;
		else if (item->pos.y_pos < item->floor)				 item->pos.y_pos = item->floor;

		item->pos.x_rot = 0;
	}

	if (!objects[item->object_number].water_creature)
	{
		GetFloor(item->pos.x_pos, item->pos.y_pos - (STEP_L * 2), item->pos.z_pos, &room_number);

		if (room[room_number].flags & UNDERWATER)
			item->hit_points = 0;
	}

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	return 1;
}

int16_t CreatureTurn(ITEM_INFO* item, int16_t maximum_turn)
{
	if (!item->data || maximum_turn == 0)
		return 0;

	auto creature = (CREATURE_INFO*)item->data;

	int z = creature->target.z - item->pos.z_pos,
		x = creature->target.x - item->pos.x_pos,
		range = (item->speed << 14) / maximum_turn;

	int16_t angle = phd_atan(z, x) - item->pos.y_rot;

	if ((angle > FRONT_ARC || angle < -FRONT_ARC) && x * x + z * z < SQUARE(range))
		maximum_turn >>= 1;

	if (angle > maximum_turn)		angle = maximum_turn;
	else if (angle < -maximum_turn) angle = -maximum_turn;

	item->pos.y_rot += angle;

	return angle;
}

void CreatureTilt(ITEM_INFO* item, int16_t angle)
{
	item->pos.z_rot += std::clamp((angle << 2) - item->pos.z_rot, -MAX_TILT, MAX_TILT);
}

void CreatureJoint(ITEM_INFO* item, int16_t joint, int16_t required)
{
	if (!item->data)
		return;

	auto creature = (CREATURE_INFO*)item->data;

	int16_t change = std::clamp(required - creature->joint_rotation[joint], -MAX_JOINT_CHANGE, MAX_JOINT_CHANGE);

	creature->joint_rotation[joint] = std::clamp(creature->joint_rotation[joint] + change, -JOINT_ARC, JOINT_ARC);
}

void CreatureFloat(int16_t item_number)
{
	auto item = &items[item_number];

	item->hit_points = DONT_TARGET;
	item->pos.x_rot = 0;

	int water_level = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

	if (item->pos.y_pos > water_level)
		item->pos.y_pos -= CREATURE_FLOAT_SPEED;

	if (item->pos.y_pos < water_level)
		item->pos.y_pos = water_level;

	AnimateItem(item);

	auto room_number = item->room_number;
	auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);
}

void CreatureUnderwater(ITEM_INFO* item, int32_t depth)
{
	if (int water_level = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
		item->pos.y_pos < water_level + depth)
	{
		item->pos.y_pos = water_level + depth;

		if (item->pos.x_rot > 2 * ONE_DEGREE) item->pos.x_rot -= 2 * ONE_DEGREE;
		else if (item->pos.x_rot > 0)		  item->pos.x_rot = 0;
	}
}

int16_t CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, int16_t(*generate)(int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE yrot, int16_t room_number))
{
	PHD_VECTOR pos { bite->x, bite->y, bite->z };

	GetJointAbsPosition(item, &pos, bite->mesh_num);

	return ((*generate)(pos.x, pos.y, pos.z, item->speed, item->pos.y_rot, item->room_number));
}

int CreatureVault(int16_t item_number, int16_t angle, int vault, int shift)
{
	auto item = &items[item_number];

	int xx = item->pos.z_pos >> WALL_SHIFT,
		yy = item->pos.x_pos >> WALL_SHIFT,
		y = item->pos.y_pos;

	auto room_number = item->room_number;

	CreatureAnimation(item_number, angle, 0);

	if (item->floor > y + STEP_L * 7 / 2)										vault = -4;
	else if (item->floor > y + STEP_L * 5 / 2 && item->object_number == MONKEY) vault = -3;
	else if (item->floor > y + STEP_L * 3 / 2 && item->object_number == MONKEY) vault = -2;
	else if (item->pos.y_pos > y - STEP_L * 3 / 2)								return 0;
	else if (item->pos.y_pos > y - STEP_L * 5 / 2)								vault = 2;
	else if (item->pos.y_pos > y - STEP_L * 7 / 2)								vault = 3;
	else																		vault = 4;

	int x_floor = item->pos.z_pos >> WALL_SHIFT,
		y_floor = item->pos.x_pos >> WALL_SHIFT;

	if (xx == x_floor)
	{
		if (yy == y_floor)
			return 0;

		if (yy < y_floor)
		{
			item->pos.x_pos = (y_floor << WALL_SHIFT) - shift;
			item->pos.y_rot = 0x4000;
		}
		else
		{
			item->pos.x_pos = (yy << WALL_SHIFT) + shift;
			item->pos.y_rot = -0x4000;
		}
	}
	else if (yy == y_floor)
	{
		if (xx < x_floor)
		{
			item->pos.z_pos = (x_floor << WALL_SHIFT) - shift;
			item->pos.y_rot = 0;
		}
		else
		{
			item->pos.z_pos = (xx << WALL_SHIFT) + shift;
			item->pos.y_rot = -0x8000;
		}
	}

	item->pos.y_pos = item->floor = y;

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);

	return vault;
}

void CreatureKill(ITEM_INFO* item, int kill_anim, int kill_state, int lara_kill_state)
{
	item->anim_number = objects[item->object_number].anim_index + kill_anim;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = kill_state;

	lara_item->anim_number = objects[LARA_EXTRA].anim_index;
	lara_item->frame_number = anims[lara_item->anim_number].frame_base;
	lara_item->current_anim_state = EXTRA_BREATH;
	lara_item->goal_anim_state = lara_kill_state;
	lara_item->pos.x_pos = item->pos.x_pos;
	lara_item->pos.y_pos = item->pos.y_pos;
	lara_item->pos.z_pos = item->pos.z_pos;
	lara_item->pos.y_rot = item->pos.y_rot;
	lara_item->pos.x_rot = item->pos.x_rot;
	lara_item->pos.z_rot = item->pos.z_rot;
	lara_item->fallspeed = 0;
	lara_item->gravity_status = 0;
	lara_item->speed = 0;

	if (item->room_number != lara_item->room_number)
		ItemNewRoom(lara.item_number, item->room_number);

	AnimateItem(lara_item);

	lara.extra_anim = 1;
	lara.gun_status = LG_HANDSBUSY;
	lara.gun_type = LG_UNARMED;
	lara.hit_direction = -1;
	lara.air = -1;

	camera.pos.room_number = lara_item->room_number;

	camera.type = CHASE_CAMERA;
	camera.flags = FOLLOW_CENTRE;
	camera.target_angle = 170 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
}

void AlertAllGuards(int16_t item_number)
{
	auto cinfo = baddie_slots;
	auto item = &items[item_number];
	auto obj_number = item->object_number;

	for (int i = 0; i < NUM_SLOTS; ++i, ++cinfo)
	{
		if (cinfo->item_num == NO_ITEM)
			continue;

		auto target = &items[cinfo->item_num];

		if (obj_number == target->object_number && target->status == ACTIVE)
			cinfo->alerted = 1;
	}
}

void AlertNearbyGuards(ITEM_INFO* item)
{
	auto cinfo = baddie_slots;

	for (int i = 0; i < NUM_SLOTS; ++i, ++cinfo)
	{
		if (cinfo->item_num == NO_ITEM)
			continue;

		auto target = &items[cinfo->item_num];

		if (target->room_number == item->room_number)
		{
			cinfo->alerted = 1;
			continue;
		}

		int x = (target->pos.x_pos - item->pos.x_pos) >> 6,
			y = (target->pos.y_pos - item->pos.y_pos) >> 6,
			z = (target->pos.z_pos - item->pos.z_pos) >> 6,
			distance = x * x + y * y + z * z;

		if (distance < GUNFIRE_ALERT_RADIUS)
			cinfo->alerted = 1;
	}
}

int16_t AIGuard(CREATURE_INFO* creature)
{
	if (items[creature->item_num].ai_bits & MODIFY)
		return 0;
	
	if (int random = GetRandomControl(); random < 0x100)
	{
		creature->head_left = 1;
		creature->head_right = 1;
	}
	else if (random < 0x180)
	{
		creature->head_left = 1;
		creature->head_right = 0;
	}
	else if (random < 0x200)
	{
		creature->head_left = 0;
		creature->head_right = 1;
	}

	if (creature->head_left && creature->head_right) return 0;
	else if (creature->head_left)					 return -0x4000;
	else if (creature->head_right)					 return 0x4000;

	return 0;
}

void GetAITarget(CREATURE_INFO* creature)
{
	if (!creature)
		return;

	auto enemy = creature->enemy;

	int enemy_object = NULL_OBJECT;

	if (enemy)
		enemy_object = enemy->object_number;

	auto item = &items[creature->item_num];

	auto ai_bits = item->ai_bits;

	if (ai_bits & GUARD)
	{
		creature->enemy = lara_item;

		if (creature->alerted)
		{
			item->ai_bits &= ~GUARD;

			if (ai_bits & AMBUSH)
				item->ai_bits |= MODIFY;
		}
	}
	else if (ai_bits & PATROL1)
	{
		if (creature->alerted || creature->hurt_by_lara)
		{
			item->ai_bits &= ~PATROL1;

			if (ai_bits & AMBUSH)
				item->ai_bits |= MODIFY;
		}
		else if (!creature->patrol2 && enemy_object != AI_PATROL1)
		{
			auto target_item = &items[0];

			for (int i = 0; i < level_items; ++i, ++target_item)
				if (target_item->object_number == AI_PATROL1 && target_item->room_number != NO_ROOM)
					if (SameZone(creature, target_item) && target_item->pos.y_rot == item->item_flags[3])
					{
						creature->enemy = target_item;
						break;
					}

		}
		else if (creature->patrol2 && enemy_object != AI_PATROL2)
		{
			auto target_item = &items[0];

			for (int i = 0; i < level_items; ++i, ++target_item)
				if (target_item->object_number == AI_PATROL2 && target_item->room_number != NO_ROOM)
					if (SameZone(creature, target_item) && target_item->pos.y_rot == item->item_flags[3])
					{
						creature->enemy = target_item;
						break;
					}
		}
		else if (ABS(enemy->pos.x_pos - item->pos.x_pos) < REACHED_GOAL_RADIUS &&
				 ABS(enemy->pos.z_pos - item->pos.z_pos) < REACHED_GOAL_RADIUS &&
				 ABS(enemy->pos.y_pos - item->pos.y_pos) < REACHED_GOAL_RADIUS)
		{
			auto floor = GetFloor(enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos, &(enemy->room_number));

			GetHeight(floor, enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos);
			TestTriggers(trigger_index, 1);

			creature->patrol2 = ~creature->patrol2;
		}
	}
	else if (ai_bits & AMBUSH)
	{
		if (!(ai_bits & MODIFY) && !creature->hurt_by_lara)
			creature->enemy = lara_item;
		else if (enemy_object != AI_AMBUSH)
		{
			auto target_item = &items[0];

			for (int i = 0; i < level_items; ++i, ++target_item)
			{
				if (target_item->object_number == AI_AMBUSH && target_item->room_number != NO_ROOM)
				{
					if (SameZone(creature, target_item) && (target_item->pos.y_rot == item->item_flags[3] || item->object_number == MONKEY))
					{
						creature->enemy = target_item;
						break;
					}
				}
			}
		}
		else if (item->object_number == MONKEY)
			return;
		else if (ABS(enemy->pos.x_pos - item->pos.x_pos) < REACHED_GOAL_RADIUS &&
				 ABS(enemy->pos.z_pos - item->pos.z_pos) < REACHED_GOAL_RADIUS &&
				 ABS(enemy->pos.y_pos - item->pos.y_pos) < REACHED_GOAL_RADIUS)
		{
			auto floor = GetFloor(enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos, &(enemy->room_number));

			GetHeight(floor, enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos);
			TestTriggers(trigger_index, 1);

			creature->reached_goal = 1;
			creature->enemy = lara_item;

			item->ai_bits &= ~AMBUSH;
			item->ai_bits &= ~MODIFY;
			item->ai_bits |= GUARD;

			creature->alerted = 0;
		}
	}
	else if (ai_bits & FOLLOW)
	{
		if (creature->hurt_by_lara)
		{
			creature->enemy = lara_item;
			creature->alerted = 1;

			item->ai_bits &= ~FOLLOW;
		}
		else if (item->hit_status)
			item->ai_bits &= ~FOLLOW;
		else if (enemy_object != AI_FOLLOW)
		{
			auto target_item = &items[0];

			for (int i = 0; i < level_items; ++i, ++target_item)
			{
				if (target_item->object_number == AI_FOLLOW && target_item->room_number != NO_ROOM)
				{
					if (SameZone(creature, target_item) && target_item->pos.y_rot == item->item_flags[3])
					{
						creature->enemy = target_item;
						break;
					}
				}
			}
		}
		else if (ABS(enemy->pos.x_pos - item->pos.x_pos) < REACHED_GOAL_RADIUS &&
				 ABS(enemy->pos.z_pos - item->pos.z_pos) < REACHED_GOAL_RADIUS &&
				 ABS(enemy->pos.y_pos - item->pos.y_pos) < REACHED_GOAL_RADIUS)
		{
			creature->reached_goal = 1;

			item->ai_bits &= ~FOLLOW;
		}

	}
	else if (item->object_number == MONKEY && item->carried_item == NO_ITEM)
	{
		if (item->ai_bits != MODIFY)
		{
			if (enemy_object != MEDI_ITEM)
			{
				auto target_item = &items[0];

				for (int i = 0; i < level_items; ++i, ++target_item)
				{
					if (target_item->object_number == MEDI_ITEM && target_item->room_number != NO_ROOM && !target_item->ai_bits && target_item->status != INVISIBLE && !(target_item->flags & KILLED_ITEM))
					{
						if (SameZone(creature, target_item))
						{
							creature->enemy = target_item;
							break;
						}
					}
				}
			}
		}
		else
		{
			if (enemy_object != KEY_ITEM4)
			{
				auto target_item = &items[0];

				for (int i = 0; i < level_items; ++i, ++target_item)
				{
					if (target_item->object_number == KEY_ITEM4 && target_item->room_number != NO_ROOM && !target_item->ai_bits && target_item->status != INVISIBLE && !(target_item->flags & KILLED_ITEM))
					{
						if (SameZone(creature, target_item))
						{
							creature->enemy = target_item;
							break;
						}
					}
				}
			}
		}
	}
}

int16_t SameZone(CREATURE_INFO* creature, ITEM_INFO* target_item)
{
	if (creature->LOT.fly != NO_FLYING)
		return 1;
	
	auto zone = ground_zone[ZONE(creature->LOT.step)][flip_status];
	auto item = &items[creature->item_num];
	auto r = &room[item->room_number];

	item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;

	auto zone_number = zone[item->box_number];

	r = &room[target_item->room_number];

	target_item->box_number = r->floor[((target_item->pos.z_pos - r->z) >> WALL_SHIFT) + ((target_item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;

	auto target_zone = zone[target_item->box_number];

	return (zone_number == target_zone);
}

void AdjustStopperFlag(ITEM_INFO* item, long dir, long set)
{
	AdjustStopperFlag(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, dir, set);
}

void AdjustStopperFlag(int x, int y, int z, int16_t room_id, long dir, long set)
{
	auto r = &room[room_id];

	r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper = set;

	int nx = x + ((1024 * phd_sin(dir)) >> W2V_SHIFT),
		nz = z + ((1024 * phd_cos(dir)) >> W2V_SHIFT);

	GetFloor(nx, y, nz, &room_id);

	r = &room[room_id];
	r->floor[((nz - r->z) >> WALL_SHIFT) + ((nx - r->x) >> WALL_SHIFT) * r->x_size].stopper = set;
}