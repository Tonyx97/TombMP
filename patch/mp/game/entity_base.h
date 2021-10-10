#pragma once

import utils;

#include <shared/game_net/typedefs.h>
#include <shared/game_net/entity_type.h>

#include <game/lara.h>
#include <game/hair.h>
#include <game/room.h>

class game_entity_base
{
protected:

	ITEM_INFO* item = nullptr;

	SYNC_ID sync_id = 0;

	int_vec3 next_position;

	double_quat next_rotation;

	int16_t item_id = NO_ITEM,
			old_room = NO_ROOM,
			new_room = NO_ROOM,
			next_frame = 0;

	int32_t type = ENTITY_TYPE_UNKNOWN,
			subtype = ENTITY_LEVEL_TYPE_NONE,
			entity_flags = 0;

	bool spawned = false,
		 block_default_sync = false,
		 old_active = false,
		 new_active = false;

public:

	~game_entity_base();

	virtual void sync(bool ignore_streamer = false) = 0;
	virtual void force_sync(bool ignore_streamer = false) = 0;
	virtual void update_by_subtype(const subtype_update_info& info) = 0;
	virtual bool spawn() = 0;

	/*
	* updates the linked list dependencies such as items in a room, active items etc.
	*/
	void update_linked_lists();
	
	void set_block_default_sync(bool v)				{ block_default_sync = v; }
	void set_new_room(int16_t v)					{ new_room = v; }
	void set_new_active(bool v)						{ new_active = v; }
	void set_entity_flags(int32_t v)				{ entity_flags = v; }
	void add_entity_flags(int32_t v)				{ entity_flags |= v; }
	void remove_entity_flags(int32_t v)				{ entity_flags &= ~v; }
	void set_floor(int32_t v)						{ item->floor = v; }
	void set_touch_bits(uint32_t v)					{ item->touch_bits = v; }
	void set_mesh_bits(uint32_t v)					{ item->mesh_bits = v; }
	void set_obj_id(int16_t v)						{ item->object_number = v; }
	void set_anim_id(int16_t v)						{ item->anim_number = v; }
	void set_anim_frame(int16_t v)					{ item->frame_number = v; }
	void set_current_anim_state(int16_t v)			{ item->current_anim_state = v; }
	void set_goal_anim_state(int16_t v)				{ item->goal_anim_state = v; }
	void set_shade(int16_t v)						{ item->shade = v; }
	void set_speed(int16_t v)						{ item->speed = v; }
	void set_fallspeed(int16_t v)					{ item->fallspeed = v; }
	void set_health(int16_t v)						{ item->hit_points = v; }
	void set_flags(int16_t v)						{ item->flags = v; }
	void set_timer(int16_t v)						{ item->timer = v; }
	void set_item_flags(int i, int16_t v)			{ item->item_flags[i] = v; }
	void set_status(uint16_t v)						{ item->status = v; }
	void set_gravity_status(uint16_t v)				{ item->gravity_status = v; }
	void set_collidable(uint16_t v)					{ item->collidable = v; }
	void set_position(const int_vec3& v)			{ utils::mem::move(item->pos.x_pos, v); }
	void set_rotation(const short_vec3& v)			{ utils::mem::move(item->pos.x_rot, v); }
	void set_local_position(const int_vec3& v)		{ utils::mem::move(item->local_pos.x_pos, v); }
	void set_local_rotation(const short_vec3& v)	{ utils::mem::move(item->local_pos.x_rot, v); }
	void set_next_anim_frame(int16_t v)				{ next_frame = v; }
	void set_next_position(const int_vec3& v)		{ next_position = v; }
	void set_next_rotation(const double_quat& v)	{ next_rotation = v; }

	bool is_spawned() const							{ return spawned; }
	bool was_active() const							{ return old_active; }
	bool is_active() const							{ return new_active; }
	bool get_active() const							{ return item->active; }
	bool is_dead() const							{ return item->hit_points <= 0; }
	
	int16_t get_item_id() const						{ return item_id; }
	int16_t get_obj_id() const						{ return item->object_number; }
	int16_t get_room() const						{ return item->room_number; }
	int16_t get_health() const						{ return item->hit_points; }
	int16_t get_flags() const						{ return item->flags; }
	int16_t get_anim_id() const						{ return item->anim_number; }
	int16_t get_anim_frame() const					{ return item->frame_number; }
	int16_t get_current_anim_state() const			{ return item->current_anim_state; }
	int16_t get_goal_anim_state() const				{ return item->goal_anim_state; }
	int16_t get_item_flags(int i) const				{ return item->item_flags[i]; }
	int16_t get_timer() const						{ return item->timer; }
	int16_t get_shade() const						{ return item->shade; }
	int16_t get_old_room() const					{ return old_room; }
	int16_t get_new_room() const					{ return new_room; }
	int16_t get_next_frame() const					{ return next_frame; }

	uint16_t get_status() const						{ return item->status; }
	uint16_t get_gravity_status() const				{ return item->gravity_status; }
	uint16_t is_collidable() const					{ return item->collidable; }

	int32_t get_floor() const						{ return item->floor; }
	int32_t get_type() const						{ return type; }
	int32_t get_subtype() const						{ return subtype; }
	int32_t get_entity_flags() const				{ return entity_flags; }
	
	uint32_t get_mesh_bits() const					{ return item->mesh_bits; }
	uint32_t get_touch_bits() const					{ return item->touch_bits; }
	
	SYNC_ID get_sync_id() const						{ return sync_id; }
	
	short_vec3 get_rotation() const					{ return utils::mem::as<short_vec3>(item->pos.x_rot); }
	short_vec3 get_local_rotation() const			{ return utils::mem::as<short_vec3>(item->local_pos.x_rot); }
	const double_quat& get_next_rotation() const	{ return next_rotation; }

	ITEM_INFO* get_item() const						{ return item; }
	
	int_vec3 get_position() const					{ return utils::mem::as<int_vec3>(item->pos.x_pos); }
	int_vec3 get_local_position() const				{ return utils::mem::as<int_vec3>(item->local_pos.x_pos); }
	const int_vec3& get_next_position() const		{ return next_position; }
};