#pragma once

import utils;

#include <shared/game_net/typedefs.h>
#include <shared/game_net/entity_type.h>
#include <shared/game/math.h>

class game_player;

class game_entity_base
{
protected:

	struct
	{
		game_player* streamer = nullptr;

		int32_t timeout = -1;

		bool lock = false;

		void reset()
		{
			streamer = nullptr;
			timeout = -1;
			lock = false;
		}
	} streaming {};

	struct
	{
		int_vec3 position,
				 local_position;

		short_vec3 rotation,
				   local_rotation;

		int32_t floor = 0,
				touch_bits = 0,
				mesh_bits = 0;

		int16_t obj_id = 0,
				anim_id = 0,
				anim_frame = 0,
				speed = 0,
				fallspeed = 0,
				room = 0,
				health = 0,
				current_anim_state = 0,
				goal_anim_state = 0,
				flags = 0,
				timer = 0,
				item_flags[4] = { 0 },
				shade = 0,
				status = 0,
				gravity_status = 0;

		bool active = false;
	} item_info;

	SYNC_ID sync_id = 0;

	int32_t type = ENTITY_TYPE_UNKNOWN,
			subtype = ENTITY_LEVEL_TYPE_NONE,
			entity_flags = 0;

	// available only for default level entities
	//
	int16_t item_id = -1;
	
	bool spawned = false;

public:

	virtual ~game_entity_base() = 0;
	virtual void spawn() = 0;
	
	void set_entity_flags(int32_t v)				{ entity_flags = v; }
	void add_entity_flags(int32_t v)				{ entity_flags |= v; }
	void remove_entity_flags(int32_t v)				{ entity_flags &= ~v; }
	void reset_streaming_info()						{ streaming.reset(); }
	void set_streamer(game_player* v)				{ streaming.streamer = v; }
	void set_streaming_timeout(int v)				{ streaming.timeout = v; }
	void set_streaming_lock(bool v)					{ streaming.lock = v; }
	void consume_streaming_timeout()				{ --streaming.timeout; }
	void despawn()									{ spawned = false; }
	void set_sync_id(SYNC_ID v)						{ sync_id = v; }
	void set_floor(int32_t v)						{ item_info.floor = v; }
	void set_touch_bits(uint32_t v)					{ item_info.touch_bits = v; }
	void set_mesh_bits(uint32_t v)					{ item_info.mesh_bits = v; }
	void set_obj_id(int16_t v)						{ item_info.obj_id = v; }
	void set_anim_id(int16_t v)						{ item_info.anim_id = v; }
	void set_anim_frame(int16_t v)					{ item_info.anim_frame = v; }
	void set_room(int16_t v)						{ item_info.room = v; }
	void set_speed(int16_t v)						{ item_info.speed = v; }
	void set_fallspeed(int16_t v)					{ item_info.fallspeed = v; }
	void set_health(int16_t v)						{ item_info.health = v; }
	void set_current_anim_state(int16_t v)			{ item_info.current_anim_state = v; }
	void set_goal_anim_state(int16_t v)				{ item_info.goal_anim_state = v; }
	void set_flags(int16_t v)						{ item_info.flags = v; }
	void set_timer(int16_t v)						{ item_info.timer = v; }
	void set_shade(int16_t v)						{ item_info.shade = v; }
	void set_item_flags(int i, int16_t v)			{ item_info.item_flags[i] = v; }
	void set_status(uint16_t v)						{ item_info.status = v; }
	void set_gravity_status(uint16_t v)				{ item_info.gravity_status = v; }
	void set_active(bool v)							{ item_info.active = v; }
	void set_position(const int_vec3& v)			{ item_info.position = v; }
	void set_rotation(const short_vec3& v)			{ item_info.rotation = v; }
	void set_local_position(const int_vec3& v)		{ item_info.local_position = v; }
	void set_local_rotation(const short_vec3& v)	{ item_info.local_rotation = v; }

	bool is_spawned() const							{ return spawned; }
	bool is_active() const							{ return item_info.active; }
	bool is_streamed_by(game_player* player)		{ return streaming.streamer == player; }
	bool is_streaming_locked() const				{ return streaming.lock; }
	
	int16_t get_item_id() const						{ return item_id; }
	int16_t get_obj_id() const						{ return item_info.obj_id; }
	int16_t get_room() const						{ return item_info.room; }
	int16_t get_health() const						{ return item_info.health; }
	int16_t get_anim_id() const						{ return item_info.anim_id; }
	int16_t get_anim_frame() const					{ return item_info.anim_frame; }
	int16_t get_current_anim_state() const			{ return item_info.current_anim_state; }
	int16_t get_goal_anim_state() const				{ return item_info.goal_anim_state; }
	int16_t get_timer() const						{ return item_info.timer; }
	int16_t get_flags() const						{ return item_info.flags; }
	int16_t get_shade() const						{ return item_info.shade; }
	int16_t get_item_flags(int i) const				{ return item_info.item_flags[i]; }
	
	uint16_t get_status() const						{ return item_info.status; }
	uint16_t get_gravity_status() const				{ return item_info.gravity_status; }

	int32_t get_floor() const						{ return item_info.floor; }
	int32_t get_streaming_timeout() const			{ return streaming.timeout; }
	int32_t get_type() const						{ return type; }
	int32_t get_subtype() const						{ return subtype; }
	int32_t get_entity_flags() const				{ return entity_flags; }
	
	uint32_t get_mesh_bits() const					{ return item_info.mesh_bits; }
	uint32_t get_touch_bits() const					{ return item_info.touch_bits; }

	SYNC_ID get_sync_id() const						{ return sync_id; }

	game_player* get_streamer() const				{ return streaming.streamer; }

	const short_vec3& get_rotation() const			{ return item_info.rotation; }
	const short_vec3& get_local_rotation() const	{ return item_info.local_rotation; }
	
	const int_vec3& get_position() const			{ return item_info.position; }
	const int_vec3& get_local_position() const		{ return item_info.local_position; }
};