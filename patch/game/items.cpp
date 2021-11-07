import prof;

#include "effect2.h"

#include <specific/standard.h>
#include <specific/global.h>

#include <game/objects.h>

#include <mp/client.h>
#include <mp/game/entity_base.h>
#include <mp/game/level.h>

void ITEM_INFO::set_rotation_x(PHD_ANGLE v)
{
	if (parent) local_pos.x_rot = v;
	else		pos.x_rot = v;
}

void ITEM_INFO::set_rotation_y(PHD_ANGLE v)
{
	if (parent) local_pos.y_rot = v;
	else		pos.y_rot = v;
}

void ITEM_INFO::set_rotation_z(PHD_ANGLE v)
{
	if (parent) local_pos.z_rot = v;
	else		pos.z_rot = v;
}

PHD_ANGLE ITEM_INFO::get_rotation_x(bool skip_local)
{
	return (parent && !skip_local ? local_pos.x_rot : pos.x_rot);
}

PHD_ANGLE ITEM_INFO::get_rotation_y(bool skip_local)
{
	return (parent && !skip_local ? local_pos.y_rot : pos.y_rot);
}

PHD_ANGLE ITEM_INFO::get_rotation_z(bool skip_local)
{
	return (parent && !skip_local ? local_pos.z_rot : pos.z_rot);
}

void InitialiseItemArray(int numitems)
{
	body_bag = NO_ITEM;
	next_item_active = NO_ITEM;
	next_item_free = level_items;

	auto item = &items[level_items];

	for (int i = level_items + 1; i < numitems; ++i, ++item)
	{
		item->next_item = i;
		item->active = 0;
		item->il.init = 0;
	}

	item->next_item = NO_ITEM;
}

void KillItem(int16_t item_num, bool sync)
{
	auto item = &items[item_num];

	if (sync)
		if (auto entity = g_level->get_entity_by_item(item))
		{
			g_client->send_packet(ID_SYNC_KILL, entity->get_sync_id());
			g_level->remove_entity(entity);
			
			return;
		}

	DetachSpark(item_num, SP_ITEM);

	item->really_active = 0;
	item->active = 0;
	item->status = INVISIBLE;

	auto linknum = next_item_active;

	if (linknum == item_num)
		next_item_active = item->next_active;
	else
	{
		for (; linknum != NO_ITEM; linknum = items[linknum].next_active)
		{
			if (items[linknum].next_active == item_num)
			{
				items[linknum].next_active = item->next_active;
				break;
			}
		}
	}

	if (item->room_number != NO_ROOM)
	{
		if ((linknum = room[item->room_number].item_number) == item_num)
			room[item->room_number].item_number = item->next_item;
		else
		{
			for (; linknum != NO_ITEM; linknum = items[linknum].next_item)
			{
				if (items[linknum].next_item == item_num)
				{
					items[linknum].next_item = item->next_item;
					break;
				}
			}
		}
	}

	if (item == lara.target)
		lara.target = nullptr;

	if (item_num < level_items)
		item->flags |= KILLED_ITEM;
	else
	{
		item->next_item = next_item_free;
		next_item_free = item_num;
	}
}

int16_t CreateItem()
{
	if (int16_t item_num = next_item_free; item_num == NO_ITEM)
		return NO_ITEM;
	else
	{
		auto item = &items[item_num];

		item->flags = 0;
		item->il.init = 0;
		item->parent = nullptr;
		item->id = item_num;

		next_item_free = item->next_item;

		return item_num;
	}
} 

void InitialiseItem(int16_t item_num)
{
	auto item = &items[item_num];

	item->anim_number = objects[item->object_number].anim_index;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
	item->required_anim_state = 0;
	item->pos.x_rot = item->pos.z_rot = 0;
	item->speed = item->fallspeed = 0;
	item->status = NOT_ACTIVE;
	item->active = 0;
	item->gravity_status = item->hit_status = item->looked_at = item->really_active = item->ai_bits = item->dynamic_light = 0;
	item->item_flags[0] = item->item_flags[1] = item->item_flags[2] = item->item_flags[3] = 0;
	item->hit_points = objects[item->object_number].hit_points;
	item->collidable = 1;
	item->clear_body = 0;
	item->timer = 0;
	item->shade = int16_t(0x4210 | 0x8000);
	item->mesh_bits = 0xffffffff;
	item->touch_bits = 0;
	item->after_death = 0;
	item->il.init = 0;
	item->fired_weapon = 0;
	item->data = nullptr;
	item->parent = nullptr;
	item->id = item_num;

	if (item->flags & NOT_VISIBLE)
	{
		item->status = INVISIBLE;
		item->flags -= NOT_VISIBLE;
	}
	else if (objects[item->object_number].intelligent)
		item->status = INVISIBLE;

	if (item->flags & CLEAR_BODY)
	{
		item->clear_body = 1;
		item->flags -= CLEAR_BODY;
	}

	if ((item->flags & CODE_BITS) == CODE_BITS)
	{
		item->flags -= CODE_BITS;
		item->flags |= REVERSE;

		AddActiveItem(item_num);

		item->status = ACTIVE;
	}

	auto r = &room[item->room_number];
	auto floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];

	item->next_item = r->item_number;
	item->floor = floor->floor << 8;
	item->box_number = floor->box;

	r->item_number = item_num;

	if (objects[item->object_number].initialise)
		(*objects[item->object_number].initialise)(item_num);
}

void BasicSetupItem(int16_t item_num)
{
	auto item = &items[item_num];

	item->anim_number = objects[item->object_number].anim_index;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
	item->required_anim_state = 0;
	item->speed = item->fallspeed = 0;
	item->status = NOT_ACTIVE;
	item->active = 0;
	item->gravity_status = item->hit_status = item->looked_at = item->really_active = item->ai_bits = item->dynamic_light = 0;
	item->item_flags[0] = item->item_flags[1] = item->item_flags[2] = item->item_flags[3] = 0;
	item->hit_points = objects[item->object_number].hit_points;
	item->collidable = 1;
	item->clear_body = 0;
	item->timer = 0;
	item->shade = int16_t(0x4210 | 0x8000);
	item->mesh_bits = 0xffffffff;
	item->touch_bits = 0;
	item->after_death = 0;
	item->il.init = 0;
	item->fired_weapon = 0;
	item->carried_item = NO_ITEM;
	item->data = nullptr;
	item->parent = nullptr;
	item->id = item_num;

	if (item->flags & NOT_VISIBLE)
	{
		item->status = INVISIBLE;
		item->flags -= NOT_VISIBLE;
	}
	else if (objects[item->object_number].intelligent)
		item->status = INVISIBLE;

	if (item->flags & CLEAR_BODY)
	{
		item->clear_body = 1;
		item->flags -= CLEAR_BODY;
	}

	auto r = &room[item->room_number];
	auto floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
	item->floor = floor->floor << 8;

	item->box_number = floor->box;

	if (objects[item->object_number].initialise)
		(*objects[item->object_number].initialise)(item_num);
}

void RemoveActiveItem(int16_t item_num)
{
	auto item = &items[item_num];

	if (!item->active)
		return;

	item->active = 0;

	if (int16_t linknum = next_item_active; linknum == item_num)
		next_item_active = item->next_active;
	else
	{
		for (; linknum != NO_ITEM; linknum = items[linknum].next_active)
		{
			if (items[linknum].next_active == item_num)
			{
				items[linknum].next_active = item->next_active;
				return;
			}
		}
	}
}

void RemoveDrawnItem(int16_t item_num)
{
	auto item = &items[item_num];

	if (int16_t linknum = room[item->room_number].item_number; linknum == item_num)
		room[item->room_number].item_number = item->next_item;
	else
	{
		for (; linknum != NO_ITEM; linknum = items[linknum].next_item)
		{
			if (items[linknum].next_item == item_num)
			{
				items[linknum].next_item = item->next_item;
				return;
			}
		}
	}
}

void AddActiveItem(int16_t item_num)
{
	auto item = &items[item_num];

	if (item->active)
		return;

	if (!objects[item->object_number].control)
	{
		item->status = NOT_ACTIVE;
		return;
	}

	item->active = ACTIVE;
	item->next_active = next_item_active;

	next_item_active = item_num;
}

void ItemNewRoom(int16_t item_num, int16_t room_number)
{
	auto item = &items[item_num];

	if (item->room_number == room_number)
		return;

	if (item->room_number != NO_ROOM)
	{
		auto r = &room[item->room_number];

		if (auto linknum = r->item_number; linknum == item_num)
			r->item_number = item->next_item;
		else
		{
			for (; linknum != NO_ITEM; linknum = items[linknum].next_item)
			{
				if (items[linknum].next_item == item_num)
				{
					items[linknum].next_item = item->next_item;
					break;
				}
			}
		}
	}

	auto r = &room[room_number];

	item->room_number = room_number;
	item->next_item = r->item_number;

	r->item_number = item_num;
}

int GlobalItemReplace(int in_objnum, int out_objnum)
{
	auto r = room;

	int changed = 0;

	for (int i = 0; i < number_rooms; ++i, ++r)
	{
		for (auto item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item)
		{
			if (items[item_num].object_number == in_objnum)
			{
				items[item_num].object_number = out_objnum;
				++changed;
			}
		}
	}
	return changed;
}

bool IsValidItem(ITEM_INFO* item)
{
	return ((uintptr_t)item >= (uintptr_t)items && (uintptr_t)item < (uintptr_t)(items + NUMBER_ITEMS));
}

void InitialiseFXArray()
{
	auto fx = effects;

	next_fx_active = NO_ITEM;
	next_fx_free = 0;

	for (int i = 1; i < NUM_EFFECTS; ++i, ++fx)
		fx->next_fx = i;

	fx->next_fx = NO_ITEM;
}

int16_t CreateEffect(int16_t room_num)
{
	int16_t fx_num = next_fx_free;

	if (fx_num == NO_ITEM)
		return NO_ITEM;

	auto fx = &effects[fx_num];
	auto r = &room[room_num];

	next_fx_free = fx->next_fx;

	fx->room_number = room_num;
	fx->next_fx = r->fx_number;
	fx->next_active = next_fx_active;
	fx->shade = 0x4210;
	fx->item = NO_ITEM;

	r->fx_number = fx_num;

	next_fx_active = fx_num;

	return fx_num;
}

void KillEffect(int16_t fx_num)
{
	DetachSpark(fx_num, SP_FX);

	auto fx = &effects[fx_num];
	auto linknum = next_fx_active;

	if (linknum == fx_num)
		next_fx_active = fx->next_active;
	else
	{
		for (; linknum != NO_ITEM; linknum = effects[linknum].next_active)
		{
			if (effects[linknum].next_active == fx_num)
			{
				effects[linknum].next_active = fx->next_active;
				break;
			}
		}
	}

	if ((linknum = room[fx->room_number].fx_number) == fx_num)
		room[fx->room_number].fx_number = fx->next_fx;
	else
	{
		for (; linknum != NO_ITEM; linknum = effects[linknum].next_fx)
		{
			if (effects[linknum].next_fx == fx_num)
			{
				effects[linknum].next_fx = fx->next_fx;
				break;
			}
		}
	}

	fx->next_fx = next_fx_free;

	next_fx_free = fx_num;
}

void EffectNewRoom(int16_t fx_num, int16_t room_number)
{
	auto fx = &effects[fx_num];
	auto r = &room[fx->room_number];
	auto linknum = r->fx_number;

	if (linknum == fx_num)
		r->fx_number = fx->next_fx;
	else
	{
		for (; linknum != NO_ITEM; linknum = effects[linknum].next_fx)
		{
			if (effects[linknum].next_fx == fx_num)
			{
				effects[linknum].next_fx = fx->next_fx;
				break;
			}
		}
	}

	r = &room[room_number];

	fx->room_number = room_number;
	fx->next_fx = r->fx_number;

	r->fx_number = fx_num;
}

void ClearBodyBag()
{
	if (body_bag == NO_ITEM)
		return;

	auto item_number = body_bag;

	while (item_number != NO_ITEM)
	{
		auto item = &items[item_number];

		KillItem(item_number);

		item_number = item->next_active;
		item->next_active = NO_ITEM;
	}

	body_bag = NO_ITEM;
}

void attach_entities(ITEM_INFO* attached, ITEM_INFO* target, const int_vec3& offset, const short_vec3& rotation)
{
	attached->local_pos.x_pos = offset.x;
	attached->local_pos.y_pos = offset.y;
	attached->local_pos.z_pos = offset.z;

	attached->local_pos.x_rot = rotation.x;
	attached->local_pos.y_rot = rotation.y;
	attached->local_pos.z_rot = rotation.z;

	attached->parent = target;

	if (auto it = g_attachments.find(target); it != g_attachments.end())
		it->second.push_back(attached);
	else g_attachments.insert({ target, { attached } });
}

void dispatch_entities_attachments()
{
	for (auto& [target, attachments] : g_attachments)
	{
		phd_PushUnitMatrix();

		*(phd_mxptr + M03) = 0;
		*(phd_mxptr + M13) = 0;
		*(phd_mxptr + M23) = 0;

		phd_TranslateRel(target->pos.x_pos, target->pos.y_pos, target->pos.z_pos);
		phd_RotYXZ(target->pos.y_rot, target->pos.x_rot, target->pos.z_rot);

		for (auto& attached_item : attachments)
		{
			phd_TranslateRel(-attached_item->local_pos.x_pos, -attached_item->local_pos.y_pos, -attached_item->local_pos.z_pos);
			phd_RotYXZ(attached_item->local_pos.y_rot, attached_item->local_pos.x_rot, attached_item->local_pos.z_rot);

			ItemNewRoom(attached_item->id, target->room_number);

			auto pos = phd_GetMatrixPosition();
			auto angles = phd_GetMatrixAngles();

			attached_item->pos.x_pos = pos.x;
			attached_item->pos.y_pos = pos.y;
			attached_item->pos.z_pos = pos.z;
			attached_item->pos.x_rot = angles.x;
			attached_item->pos.y_rot = angles.y;
			attached_item->pos.z_rot = angles.z;
		}

		phd_PopMatrix();
	}
}