#pragma once

#include <shared/game_net/typedefs.h>

class game_entity_base;
class game_entity;
class game_player;

class ITEM_INFO;

enum level_entity_type;
enum object_types;

class game_level
{
private:

	std::unordered_map<SYNC_ID, game_entity_base*> all_entities;			// localplayer stays out of this because the sid is not received at connection time
	std::unordered_map<ITEM_INFO*, game_entity_base*> entities_by_item;		// localplayer stays out of this because 'lara_item' changes
	std::unordered_map<PLAYER_ID, game_player*> players;
	std::unordered_map<int, level_entity_type> subtypes;

	std::unordered_set<game_entity_base*> local_streamed_entities;

	struct
	{
		std::unordered_set<game_player*> players;
		std::unordered_set<game_entity*> level_entities;
	} instances;

	std::vector<game_vec3d> player_spawns;

	std::string filename;

	game_vec3d default_spawn;

	bool change = false,
		 loaded = false;

	static inline game_player* localplayer = nullptr;

	game_entity_base* add_entity_base(game_entity_base* base);

	void clear();

public:

	game_level()												{}
	~game_level();

	static game_player* LOCALPLAYER()							{ return localplayer; }

	game_player* add_localplayer();
	game_player* add_player(PLAYER_ID id, SYNC_ID sid);
	game_entity* add_level_entity(int subtype, int16_t item_id, SYNC_ID sid);
	game_entity* spawn_level_entity(int16_t obj_id, SYNC_ID sid, const game_vec3d& vec);

	void remove_entity(game_entity_base* base);

	void set_level(const std::string& v, bool restart = false);
	void add_streamed_entity(SYNC_ID sid);
	void remove_streamed_entity(SYNC_ID sid);
	void request_entity_ownership(game_entity_base* entity, bool acquire, int timeout = -1);
	void add_player_spawn(const game_vec3d& v)					{ player_spawns.push_back(v); }
	void set_default_spawn(const game_vec3d& v)					{ default_spawn = v; }
	void set_as_loaded()										{ loaded = true; change = false; }

	template <typename Tx>
	void register_entity_type(Tx type) {}

	template <typename Tx, typename Ty, typename... A>
	void register_entity_type(Tx type, Ty obj_id, A... obj_ids)
	{
		static_assert(std::is_same_v<Tx, level_entity_type>, "Tx should be 'level_entity_type'");
		static_assert(std::is_same_v<Ty, object_types>, "Ty should be 'object_types'");

		subtypes.insert({ int(obj_id), type });

		register_entity_type(type, obj_ids...);
	}

	bool is_entity_streamed(game_entity_base* entity);

	level_entity_type get_level_entity_subtype_from_obj_id(int obj_id);

	const auto& get_instanced_players()							{ return instances.players; }
	const auto& get_instanced_level_entities()					{ return instances.level_entities; }
	const auto& get_player_spawns()								{ return player_spawns; }
	
	size_t get_instanced_players_count()						{ return instances.players.size(); }
	size_t get_instanced_level_entities_count()					{ return instances.level_entities.size(); }

	game_entity_base* get_entity_by_sid(SYNC_ID sid);
	game_entity_base* get_entity_by_item(ITEM_INFO* item);

	game_player* get_player_by_id(PLAYER_ID id);
	game_player* get_player_by_item(ITEM_INFO* item);
	game_player* get_player_by_name(const std::string& name);

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

	template <typename T>
	void for_each_streamed_entity(const T& fn)
	{
		for (auto entity : local_streamed_entities)
			fn(entity);
	}
	
	bool is_change_requested() const							{ return change; }
	bool is_loaded() const										{ return loaded; }
	bool has_entity(game_entity* entity) const;
	bool has_player(game_player* player) const;
	bool has_player(PLAYER_ID id) const;
	bool has_player(ITEM_INFO* player_item);

	const game_vec3d& get_default_spawn() const					{ return default_spawn; }

	const std::string& get_filename() const						{ return filename; }
};

inline std::unique_ptr<game_level> g_level;