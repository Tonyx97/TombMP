#pragma once

#include <shared/game_net/typedefs.h>
#include <shared/game_net/entity_type.h>
#include <shared/game/obj_types.h>

class net_player;
class game_entity_base;
class game_entity;
class game_player;

class game_level
{
private:

	struct attach_info
	{
		game_entity* a,
				   * b;

		int_vec3 local_pos;

		short_vec3 local_rot;
	};

	std::unordered_map<SYNC_ID, game_entity_base*> all_entities;
	std::unordered_map<PLAYER_ID, game_player*> players;
	std::unordered_map<int, level_entity_type> subtypes;
	std::unordered_set<int16_t> loaded_objects;
	std::vector<attach_info> attachments;

	struct
	{
		std::unordered_set<game_player*> players;
		std::unordered_set<game_entity*> level_entities;
	} instances;

	std::vector<game_vec3d> player_spawns;

	std::string filename = "none",
				name = "none";

	std::mt19937_64 mt;

	bool level_entities_synced = false;

	game_entity_base* add_entity_base(game_entity_base* base);

public:

	game_level();
	~game_level();

	game_player* add_player(net_player* n_player);
	game_entity* add_level_entity(int subtype, int16_t item_id = -1);
	game_entity* spawn_level_entity(int16_t obj_id, const game_vec3d& vec);

	void sync_entity(game_entity_base* entity_base);
	void destroy_entity(game_entity* entity);
	void remove_entity(game_entity_base* base);
	void remove_all_level_entities();
	void reset_level_data();
	void update_entity_streamers();
	void send_initial_level_entities_info(game_player* player);
	void sync_attachments_for_player(game_player* player);
	void add_player_spawn(const game_vec3d& v)			{ player_spawns.push_back(v); }
	void set_object_loaded(int16_t obj_id)				{ loaded_objects.insert(obj_id); }
	void set_level_entities_synced()					{ level_entities_synced = true; }
	void add_attachment(game_entity* a, game_entity* b, const int_vec3& local_pos, const short_vec3& local_rot);
	
	template <typename Tx>
	void register_entity_type(Tx type)				{}

	template <typename Tx, typename Ty, typename... A>
	void register_entity_type(Tx type, Ty obj_id, A... obj_ids)
	{
		static_assert(std::is_same_v<Tx, level_entity_type>, "Tx should be 'level_entity_type'");
		static_assert(std::is_same_v<Ty, object_types>, "Ty should be 'object_types'");

		subtypes.insert({ int(obj_id), type });

		register_entity_type(type, obj_ids...);
	}

	// spawns a synced entity to a player that didn't have it spawned before
	// because the player just joined
	//
	bool spawn_entity_for_player(net_player* n_player, game_entity* entity);
	bool set_level(const std::string& v, bool restart = false);

	template <typename T>
	void for_each_player(const T& fn)
	{
		for (auto player : instances.players)
			fn(player);
	}
	
	template <typename T>
	void for_each_level_entity(const T& fn)
	{
		for (auto entity : instances.level_entities)
			fn(entity);
	}

	bool has_player(game_player* player);
	bool is_entity_streamed_by(SYNC_ID entity_sid, game_player* player);
	bool is_entity_streamed_by(game_entity_base* entity, game_player* player);
	bool is_object_loaded(int16_t obj_id) const			{ return loaded_objects.contains(obj_id); }
	bool are_level_entities_synced() const				{ return level_entities_synced; }
	bool has_player(PLAYER_ID id) const 				{ return players.contains(id); }
	bool has_entity(game_entity* entity) const 			{ return instances.level_entities.contains(entity); }

	level_entity_type get_level_entity_subtype_from_obj_id(int obj_id);

	SYNC_ID generate_sync_id();

	game_entity_base* get_entity_by_sid(SYNC_ID sid);

	game_player* get_player_by_id(PLAYER_ID id);
	game_player* get_random_player(bool joined);

	const auto& get_instanced_players()					{ return instances.players; }
	const auto& get_instanced_level_entities()			{ return instances.level_entities; }
	const auto& get_player_spawns()						{ return player_spawns; }
	
	size_t get_instanced_players_count()				{ return instances.players.size(); }
	size_t get_instanced_level_entities_count()			{ return instances.level_entities.size(); }

	const std::string& get_filename() const				{ return filename; }
	const std::string& get_name() const					{ return name; }
};

inline std::unique_ptr<game_level> g_level;