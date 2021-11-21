#pragma once

#include <shared/game/math.h>

#include "typedefs.h"
#include "file_info.h"

namespace GNS_GLOBALS
{
	inline constexpr size_t MAX_CHAT_MSG_LENGTH = 256ull;

	inline constexpr int MAX_PLAYERS = 32;
	inline constexpr int MIN_NAME_LENGTH = 2;
	inline constexpr int MAX_NAME_LENGTH = 32;
	inline constexpr int MAX_RESOURCE_NAME_LENGTH = 64;
	inline constexpr int MAX_LEVEL_FILENAME_LENGTH = MAX_RESOURCE_NAME_LENGTH * 2;
	inline constexpr int MAX_USERNAME_LENGTH = 32;
	inline constexpr int MAX_CHAT_NOTIFICATION_LENGTH = 256;
	inline constexpr int MAX_SCRIPT_ERROR_LENGTH = 512;
	inline constexpr int MAX_EVENT_NAME_LENGTH = 64;
}

template <typename T, int S>
struct gns_string
{
	T data[S] = { 0 };

	const T* operator * () { return data; }

	gns_string() {}
	gns_string(const T* v)
	{
		*this = v;
	}

	template <typename Tx = T, std::enable_if_t<std::is_same_v<Tx, char>>* = nullptr>
	void operator = (const Tx* v)
	{
		strcpy_s(data, v);
	}

	template <typename Tx = T, std::enable_if_t<std::is_same_v<Tx, wchar_t>>* = nullptr>
	void operator = (const Tx* v)
	{
		wcscpy_s(data, v);
	}
};

template <typename T, int S>
struct gns_vector
{
	T data[S] = {};

	int count = 0;

	void add(const T& v)	{ data[count++] = v; }

	auto begin()			{ return data; }
	auto end()				{ return data + count; }
};

/*
* SERVER
*/

namespace gns
{
	namespace server
	{
		struct game_settings
		{
			bool player_info = false,
				 friendly_fire = true,
				 flip_map_sync = true;
		};

		struct trigger_event
		{
			gns_string<char, GNS_GLOBALS::MAX_EVENT_NAME_LENGTH> name {};
		};
	}

	namespace file
	{
		struct file_transfer_data
		{
			gns_string<char, GNS_GLOBALS::MAX_RESOURCE_NAME_LENGTH> filename {};

			FILE_HASH file_hash;

			int file_size,
				file_bytes_transferred,
				delta,
				total_size,
				total_files,
				bytes_sent,
				files_sent;

			// data here (read from the bitstream using 'size' and 'bytes_sent')
		};
	}

	namespace resource
	{
		struct info
		{
			gns_string<char, GNS_GLOBALS::MAX_RESOURCE_NAME_LENGTH> rsrc {},
																	name {};

			FILE_HASH hash;
		};
		
		struct action
		{
			gns_string<char, GNS_GLOBALS::MAX_RESOURCE_NAME_LENGTH> name {};

			int action;
		};
	}

	namespace net_player
	{
		struct connect
		{
			PLAYER_ID id;

			SYNC_ID sid;
		};

		struct join
		{
			gns_string<char, GNS_GLOBALS::MAX_NAME_LENGTH> name {};

			PLAYER_ID id;

			bool ready = false;
		};

		struct quit
		{
			gns_string<char, GNS_GLOBALS::MAX_NAME_LENGTH> name {};

			PLAYER_ID id;

			bool timed_out = false;
		};

		struct name_bc
		{
			gns_string<char, GNS_GLOBALS::MAX_NAME_LENGTH> old_name {},
														   new_name {};
		};

		struct chat
		{
			gns_string<wchar_t, GNS_GLOBALS::MAX_CHAT_MSG_LENGTH> message {};
			gns_string<char, GNS_GLOBALS::MAX_NAME_LENGTH> player_name {};
		};

		struct register_login
		{
			gns_string<char, GNS_GLOBALS::MAX_USERNAME_LENGTH> user {},
															   pass {};
		};

		struct net_flags
		{
			gns_string<char, GNS_GLOBALS::MAX_USERNAME_LENGTH> user {};

			uint64_t flags = 0;
		};

		struct notification
		{
			gns_string<char, GNS_GLOBALS::MAX_CHAT_NOTIFICATION_LENGTH> text {};

			uint64_t color = 0;
		};

		struct script_error
		{
			gns_string<char, GNS_GLOBALS::MAX_SCRIPT_ERROR_LENGTH> error {};
		};
	}

	namespace level
	{
		struct info
		{
			gns_string<char, GNS_GLOBALS::MAX_LEVEL_FILENAME_LENGTH> name {};
		};

		struct load
		{
			gns_string<char, GNS_GLOBALS::MAX_LEVEL_FILENAME_LENGTH> name {};

			bool restart;
		};
	}

	namespace player
	{
		struct initial_sync
		{
			gns_string<char, GNS_GLOBALS::MAX_NAME_LENGTH> name {};

			PLAYER_ID id;

			SYNC_ID sid;
		};

		struct spawn
		{
			gns_string<char, GNS_GLOBALS::MAX_NAME_LENGTH> name {};

			int_vec3 position;

			short_vec3 rotation;

			PLAYER_ID id;

			SYNC_ID sid;

			int16_t health;

			bool set_position,
				 set_rotation;
		};

		struct respawn
		{
			int_vec3 position;

			short_vec3 rotation;
	
			PLAYER_ID id;

			SYNC_ID sid;

			int16_t room,
					health;

			bool set_position;
		};

		struct info
		{
			struct arm
			{
				short_vec3 rotation;

				uint32_t frame_base;

				int16_t anim,
						frame,
						flash_gun;
			};

			arm left_arm,
				right_arm;

			int_vec3 position;

			short_vec3 rotation,
					   head_rotation,
					   torso_rotation;

			PLAYER_ID id;

			SYNC_ID vehicle;

			int32_t entity_flags,
					floor,
					ping,
					smoke_weapon,
					smoke_count_l,
					smoke_count_r;

			uint32_t touch_bits,
					 mesh_bits,
					 meshes_offsets[15];

			int16_t anim,
					anim_frame,
					room,
					speed,
					fallspeed,
					health,
					back_gun,
					hit_direction,
					hit_frame,
					gun_status,
					gun_type,
					flare_age,
					weapon_item_current_anim_state,
					water_status;

			uint8_t fire_r,
					fire_g,
					fire_b;

			bool ducked,
				 underwater,
				 burning,
				 electric,
				 respawn,
				 flare_in_hand,
				 collidable,
				 hair_enabled;
		};
	}

	namespace projectile
	{
		struct create
		{
			game_vec3d vec;

			int16_t obj,
					speed,
					fallspeed,
					health,
					current_anim_state,
					goal_anim_state,
					required_anim_state,
					flags0;
		};
	}

	namespace fx
	{
		struct gun_smoke
		{
			int x, y, z,
				vx, vy, vz,
				initial, weapon,
				count;

			int16_t room;
		};

		struct gunshell
		{
			int x, y, z, ry,
				shelltype,
				weapon;

			int16_t room;

			bool left;
		};
	}

	namespace audio
	{
		struct play
		{
			int_vec3 pos;

			uint32_t hash;

			float pitch;
		};

		struct stop
		{
			uint32_t hash;
		};
	}

	namespace entity
	{
		struct hit_damage
		{
			struct
			{
				int x, y, z;

				int16_t speed,
						angle,
						room;

				bool create;
			} blood;

			SYNC_ID attacker_sid,
					sid;

			int weapon;

			int16_t damage;
		};

		struct explode
		{
			SYNC_ID sid;
		};

		struct spawn
		{
			game_vec3d vec;

			SYNC_ID sid;

			int16_t obj_id;
		};

		struct attach
		{
			SYNC_ID a_sid,
					b_sid;

			int_vec3 local_pos;

			short_vec3 local_rot;
		};
	}

	namespace sync
	{
		struct level_entity_initial_basic_sync
		{
			SYNC_ID sid;

			int subtype;

			int16_t id;
		};

		struct stream_sync_info
		{
			SYNC_ID sid;
			
			int distance;
		};

		struct stream_info
		{
			SYNC_ID sid;

			bool add;
		};

		struct ownership
		{
			SYNC_ID sid;

			int timeout;

			bool acquire;
		};

		struct base
		{
			SYNC_ID sid;

			bool ignore_streamer;
		};

		struct base_info : public base
		{
			int_vec3 pos,
					 local_pos;

			short_vec3 rot,
					   local_rot;

			uint32_t mesh_bits,
					 touch_bits;

			int16_t obj_id,
					room,
					hp,
					anim,
					frame,
					current_anim_state,
					goal_anim_state,
					timer,
					flags,
					flags0,
					shade;

			uint16_t active,
					 status,
					 gravity_status;
		};

		struct block : public base_info
		{
		};

		struct ai : public base_info
		{
		};

		struct vehicle : public base_info
		{
		};

		struct interactive : public base_info
		{
		};

		struct trap : public base_info
		{
		};
	}
}