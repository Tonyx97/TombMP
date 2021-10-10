#include "objects.h"
#include "lara.h"
#include "camera.h"
#include "gameflow.h"
#include "lot.h"

#include <specific/init.h>
#include <specific/standard.h>
#include <specific/global.h>

#include <mp/client.h>
#include <mp/game/entity_base.h>
#include <mp/game/level.h>

CREATURE_INFO* non_lot_slots;

int nonlot_slots_used = 0;

void InitialiseLOTarray()
{
	auto creature = baddie_slots = (CREATURE_INFO*)game_malloc(NUM_SLOTS * sizeof(CREATURE_INFO), CREATURE_DATA);

	for (int i = 0; i < NUM_SLOTS; ++i, ++creature)
	{
		creature->item_num = NO_ITEM;
		creature->LOT.node = (BOX_NODE*)game_malloc(sizeof(BOX_NODE) * number_boxes, CREATURE_LOT);
	}

	slots_used = 0;

	creature = non_lot_slots = (CREATURE_INFO*)game_malloc(NUM_NONLOT_SLOTS * sizeof(CREATURE_INFO), CREATURE_DATA);

	for (int i = 0; i < NUM_NONLOT_SLOTS; ++i, ++creature)
		creature->item_num = NO_ITEM;

	nonlot_slots_used = 0;
}

void DisableBaddieAI(int16_t item_num)
{
	CREATURE_INFO* creature = nullptr;

	auto item = &items[item_num];

	if (item_num == lara.item_number)
	{
		creature = lara.creature;
		lara.creature = nullptr;
	}
	else
	{
		creature = (CREATURE_INFO*)item->data;
		item->data = nullptr;
	}

	if (creature)
	{
		creature->item_num = NO_ITEM;

		if (objects[item->object_number].non_lot)
			--nonlot_slots_used;
		else --slots_used;
	}
}

int EnableBaddieAI(int16_t item_number, int Always)
{
	auto item = &items[item_number];

	auto acquire_entity_control = [&]()
	{
		g_level->request_entity_ownership(g_level->get_entity_by_item(item), true, 2);

		return 1;
	};

	CREATURE_INFO* creature = nullptr;

	if (lara.item_number == item_number)
	{
		if (lara.creature)
			return acquire_entity_control();
	}
	else if (item->data)
		return acquire_entity_control();
	else if (objects[item->object_number].non_lot)
		return (EnableNonLotAI(item_number, Always) == 1 ? acquire_entity_control() : 0);

	if (slots_used < NUM_SLOTS)
	{
		creature = baddie_slots;

		for (int i = 0; i < NUM_SLOTS; ++i, ++creature)
		{
			if (creature->item_num == NO_ITEM)
			{
				InitialiseSlot(item_number, i);
				return acquire_entity_control();
			}
		}
	}

	int worstdist;

	if (!Always)
	{
		int x = (item->pos.x_pos - camera.pos.x) >> 8,
			y = (item->pos.y_pos - camera.pos.y) >> 8,
			z = (item->pos.z_pos - camera.pos.z) >> 8;

		worstdist = (x * x) + (y * y) + (z * z);
	}
	else worstdist = 0;

	int worstslot = -1;

	creature = baddie_slots;

	for (int i = 0; i < NUM_SLOTS; ++i, ++creature)
	{
		auto item = &items[creature->item_num];

		int x = (item->pos.x_pos - camera.pos.x) >> 8,
			y = (item->pos.y_pos - camera.pos.y) >> 8,
			z = (item->pos.z_pos - camera.pos.z) >> 8,
			dist = (x * x) + (y * y) + (z * z);

		if (dist > worstdist)
		{
			worstdist = dist;
			worstslot = i;
		}
	}

	if (worstslot >= 0)
	{
		items[baddie_slots[worstslot].item_num].status = INVISIBLE;
		//DisableBaddieAI(baddie_slots[worstslot].item_num);
		//InitialiseSlot(item_number, worstslot);

		return acquire_entity_control();
	}

	return 1;
}

void InitialiseSlot(int16_t item_number, int slot)
{
	auto creature = &baddie_slots[slot];
	auto item = &items[item_number];

	if (item_number == lara.item_number)
		lara.creature = creature;
	else item->data = (void*)creature;

	creature->item_num = item_number;
	creature->mood = BORED_MOOD;

	for (int i = 0; i < 4; ++i)
		creature->joint_rotation[i] = 0;

	creature->maximum_turn = ONE_DEGREE;
	creature->flags = 0;
	creature->enemy = nullptr;
	creature->alerted = 0;
	creature->head_left = 0;
	creature->head_right = 0;
	creature->reached_goal = 0;
	creature->hurt_by_lara = 0;
	creature->patrol2 = 0;

	creature->LOT.step = STEP_L;
	creature->LOT.drop = -STEP_L * 2;
	creature->LOT.block_mask = BLOCKED;
	creature->LOT.fly = NO_FLYING;

	switch (item->object_number)
	{
	case LARA:
		creature->LOT.step = WALL_L * 20;
		creature->LOT.drop = -WALL_L * 20;
		creature->LOT.fly = STEP_L;
		break;
	case DIVER:
	case WHALE:
	case VULTURE:
	case CROW:
	case MUTANT1:
	case CROCODILE:
	{
		creature->LOT.step = WALL_L * 20;
		creature->LOT.drop = -WALL_L * 20;
		creature->LOT.fly = STEP_L / 16;

		if (item->object_number == WHALE)
			creature->LOT.block_mask = BLOCKABLE;

		break;
	}
	case LIZARD_MAN:
	case BOB:
	case PUNK1:
	case CIVVIE:
	case MONKEY:
	case MP1:
	case WILLARD_BOSS:
		creature->LOT.step = WALL_L;
		creature->LOT.drop = -WALL_L;
		break;
	case SHIVA:
	case TREX:
		creature->LOT.block_mask = BLOCKABLE;
		break;
	case LON_BOSS:
		creature->LOT.step = WALL_L;
		creature->LOT.drop = -STEP_L * 3;
		break;
	}

	ClearLOT(&creature->LOT);

	if (item_number != lara.item_number)
		CreateZone(item);

	++slots_used;
}

void CreateZone(ITEM_INFO* item)
{
	auto creature = (CREATURE_INFO*)item->data;
	auto r = &room[item->room_number];

	item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;

	auto node = creature->LOT.node;

	creature->LOT.zone_count = 0;

	if (creature->LOT.fly == NO_FLYING)
	{
		auto zone = ground_zone[ZONE(creature->LOT.step)][0],
			 flip = ground_zone[ZONE(creature->LOT.step)][1];

		auto zone_number = zone[item->box_number],
			 flip_number = flip[item->box_number];

		for (int i = 0; i < number_boxes; ++i, ++zone, ++flip)
			if (*zone == zone_number || *flip == flip_number)
			{
				node->box_number = i;
				++node;
				++creature->LOT.zone_count;
			}
	}
	else
	{
		for (int i = 0; i < number_boxes; ++i)
		{
			node->box_number = i;
			++node;
			++creature->LOT.zone_count;
		}
	}
}

void ClearLOT(LOT_INFO* LOT)
{
	LOT->head = LOT->tail = NO_BOX;
	LOT->search_number = 0;
	LOT->target_box = NO_BOX;
	LOT->required_box = NO_BOX;
	
	auto node = LOT->node;

	for (int i = 0; i < number_boxes; ++i, ++node)
	{
		node->exit_box = node->next_expansion = NO_BOX;
		node->search_number = 0;
	}
}

int EnableNonLotAI(int16_t item_number, int Always)
{
	auto creature = non_lot_slots;

	if (nonlot_slots_used < NUM_NONLOT_SLOTS)
	{
		for (int i = 0; i < NUM_NONLOT_SLOTS; ++i, ++creature)
		{
			if (creature->item_num == NO_ITEM)
			{
				InitialiseNonLotAI(item_number, i);
				return 1;
			}
		}
	}

	int worstdist;

	if (!Always)
	{
		auto item = &items[item_number];

		int x = (item->pos.x_pos - camera.pos.x) >> 8,
			y = (item->pos.y_pos - camera.pos.y) >> 8,
			z = (item->pos.z_pos - camera.pos.z) >> 8;

		worstdist = (x * x) + (y * y) + (z * z);
	}
	else worstdist = 0;

	int worstslot = -1;

	creature = non_lot_slots;

	for (int i = 0; i < NUM_NONLOT_SLOTS; ++i, ++creature)
	{
		auto item = &items[creature->item_num];

		int x = (item->pos.x_pos - camera.pos.x) >> 8,
			y = (item->pos.y_pos - camera.pos.y) >> 8,
			z = (item->pos.z_pos - camera.pos.z) >> 8,
			dist = (x * x) + (y * y) + (z * z);

		if (dist > worstdist)
		{
			worstdist = dist;
			worstslot = i;
		}
	}

	if (worstslot >= 0)
	{
		items[non_lot_slots[worstslot].item_num].status = INVISIBLE;

		DisableBaddieAI(non_lot_slots[worstslot].item_num);
		InitialiseNonLotAI(item_number, worstslot);

		return 1;
	}

	return 0;
}

void InitialiseNonLotAI(int16_t item_number, int slot)
{
	auto creature = &non_lot_slots[slot];
	auto item = &items[item_number];

	if (item_number == lara.item_number)
		lara.creature = creature;
	else item->data = (void*)creature;

	creature->item_num = item_number;
	creature->mood = BORED_MOOD;

	for (int i = 0; i < 4; ++i)
		creature->joint_rotation[i] = 0;

	creature->maximum_turn = ONE_DEGREE;
	creature->flags = 0;
	creature->enemy = nullptr;
	creature->alerted = 0;
	creature->head_left = 0;
	creature->head_right = 0;
	creature->reached_goal = 0;
	creature->hurt_by_lara = 0;
	creature->patrol2 = 0;

	creature->LOT.step = STEP_L;
	creature->LOT.drop = -STEP_L * 2;
	creature->LOT.block_mask = BLOCKED;
	creature->LOT.fly = NO_FLYING;

	switch (item->object_number)
	{
	case LARA:
		creature->LOT.step = WALL_L * 20;
		creature->LOT.drop = -WALL_L * 20;
		creature->LOT.fly = STEP_L;
		break;
	case DIVER:
	case WHALE:
	case VULTURE:
	case CROW:
	{
		creature->LOT.step = WALL_L * 20;
		creature->LOT.drop = -WALL_L * 20;
		creature->LOT.fly = STEP_L / 16;

		if (item->object_number == WHALE)
			creature->LOT.block_mask = BLOCKABLE;

		break;
	}
	case LIZARD_MAN:
	case MP1:
		creature->LOT.step = WALL_L;
		creature->LOT.drop = -WALL_L;
		break;
	}

	++nonlot_slots_used;
}