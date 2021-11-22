#pragma once

#include <shared/game/math.h>

#include "entity_base.h"

class game_player : public game_entity_base
{
public:

	struct arm
	{
		short_vec3 rotation;

		uint32_t frame_base;

		int16_t anim,
				frame,
				flash_gun;
	};

private:

	std::string name;

	vec3d hair[7];

	int_vec3 hair_vel[7];

	arm left_arm,
		right_arm;

	short_vec3 head_rotation,
			   torso_rotation;

	game_entity_base* vehicle = nullptr;

	int32_t smoke_weapon = 0,
			smoke_count_l = 0,
			smoke_count_r = 0;
		
	uint32_t meshes_offsets[15] = { 0 };

	int ping = -1;

	int16_t	back_gun = 0,
			hit_direction = -1,
			hit_frame = 0,
			gun_status = 0,
			gun_type = 0,
			flare_age = 0,
			weapon_item_current_anim_state = -1,
			water_status = 0,
			skin = -1;

	uint8_t fire_r = 0,
			fire_g = 0,
			fire_b = 0;

	bool local = false,
		 ducked = false,
		 underwater = false,
		 burning = false,
		 electric = false,
		 flare_in_hand = false,
		 hair_enabled = false;

	PLAYER_ID id;

public:

	game_player(PLAYER_ID id, SYNC_ID sid);
	game_player(PLAYER_ID id, SYNC_ID sid, const game_vec3d& spawn_vec);
	~game_player();

	void sync(bool ignore_streamer = false) override			{}
	void force_sync(bool ignore_streamer = false) override		{}
	void update_by_subtype(const subtype_update_info&) override	{}

	bool spawn() override;

	void update_localplayer_instance_info(bool clear = false);
	void set_local()											{ local = true; }
	void set_name(const std::string& v)							{ name = v; }
	void set_ping(int v)										{ ping = v; }
	void set_vehicle(game_entity_base* v)						{ vehicle = v; }
	void set_smoke_weapon(int v)								{ smoke_weapon = v; }
	void set_smoke_count_l(int v)								{ smoke_count_l = v; }
	void set_smoke_count_r(int v)								{ smoke_count_r = v; }
	void set_head_rotation(const short_vec3& v)					{ head_rotation = v; }
	void set_torso_rotation(const short_vec3& v)				{ torso_rotation = v; }
	void set_left_arm_info(void* v)								{ memcpy(&left_arm, v, sizeof(left_arm)); }
	void set_right_arm_info(void* v)							{ memcpy(&right_arm, v, sizeof(right_arm)); }
	void set_back_gun(int16_t v)								{ back_gun = v; }
	void set_hit_direction(int16_t v)							{ hit_direction = v; }
	void set_hit_frame(int16_t v)								{ hit_frame = v; }
	void set_gun_status(int16_t v)								{ gun_status = v; }
	void set_gun_type(int16_t v)								{ gun_type = v; }
	void set_flare_age(int16_t v)								{ flare_age = v; }
	void set_skin(int16_t v)									{ skin = v; }
	void set_fire_color(uint8_t r, uint8_t g, uint8_t b)		{ fire_r = r; fire_g = g; fire_b = b; }
	void set_ducked(bool v)										{ ducked = v; }
	void set_underwater(bool v)									{ underwater = v; }
	void set_burning(bool v)									{ burning = v; }
	void set_electric(bool v)									{ electric = v; }
	void set_flare_in_hand(bool v)								{ flare_in_hand = v; }
	void set_hair_enabled(bool v)								{ hair_enabled = v; }
	void set_weapon_item_current_anim_state(int16_t v)			{ weapon_item_current_anim_state = v; }
	void set_water_status(int16_t v)							{ water_status = v; }
	void set_meshes_offsets(uint32_t* v)						{ memcpy(meshes_offsets, v, sizeof(meshes_offsets)); }
	
	bool is_local() const										{ return local; }
	bool is_ducked() const										{ return ducked; }
	bool is_underwater() const									{ return underwater; }
	bool is_burning() const										{ return burning; }
	bool is_electric() const									{ return electric; }
	bool is_flare_in_hand() const								{ return flare_in_hand; }
	bool is_hair_enabled() const								{ return hair_enabled; }
	
	int16_t get_back_gun() const								{ return back_gun; }
	int16_t get_hit_direction() const							{ return hit_direction; }
	int16_t get_hit_frame() const								{ return hit_frame; }
	int16_t get_gun_status() const								{ return gun_status; }
	int16_t get_gun_type() const								{ return gun_type; }
	int16_t get_flare_age() const								{ return flare_age; }
	int16_t get_weapon_item_current_anim_state() const			{ return weapon_item_current_anim_state; }
	int16_t get_water_status() const							{ return water_status; }
	int16_t get_skin() const									{ return skin; }

	int get_ping() const										{ return ping; }
	int get_smoke_weapon() const								{ return smoke_weapon; }
	int get_smoke_count_l() const								{ return smoke_count_l; }
	int get_smoke_count_r() const								{ return smoke_count_r; }

	uint32_t* get_meshes_offsets() 								{ return meshes_offsets; }
	
	vec3d* get_hair_data() 										{ return hair; }
	int_vec3* get_hair_vel() 									{ return hair_vel; }

	game_entity_base* get_vehicle() const						{ return vehicle; }

	PLAYER_ID get_id() const									{ return id; }
	
	u8_vec3 get_fire_color() const								{ return { fire_r, fire_g, fire_b }; }
	const short_vec3& get_head_rotation() const					{ return head_rotation; }
	const short_vec3& get_torso_rotation() const				{ return torso_rotation; }
	const arm& get_left_arm_info() const						{ return left_arm; }
	const arm& get_right_arm_info() const						{ return right_arm; }
		
	const std::string& get_name() const							{ return name; }
	
	static bool check_class(game_entity_base* i)				{ return i->get_type() == ENTITY_TYPE_PLAYER; }
};