#pragma once

#include <unordered_set>

#include <shared/game/math.h>
#include <shared/game_net/structs.h>
#include <shared/game_net/typedefs.h>

#include "entity_base.h"

class net_player;

class game_player : public game_entity_base
{
private:

	std::unordered_map<SYNC_ID, gns::sync::stream_sync_info> streaming_info;

	std::unordered_set<game_entity_base*> streamed_entities,
										  removed_streamed_entities,
										  added_streamed_entities;

	net_player* net = nullptr;

	int ping = -1;

	bool streaming_synced = false;

public:
	
	game_player(net_player* net, SYNC_ID sid);
	~game_player();

	void spawn() override										{}
	void send_stream_info();
	void set_ping(int v)										{ ping = v; }
	void set_streaming_synced()									{ streaming_synced = true; }
	void add_stream_info(gns::sync::stream_sync_info& v)		{ streaming_info.insert({ v.sid, std::move(v) }); }
	void add_streamed_entity(game_entity_base* v)				{ streamed_entities.insert(v); added_streamed_entities.insert(v); }
	void remove_streamed_entity(game_entity_base* v)			{ streamed_entities.erase(v); removed_streamed_entities.insert(v); }
	void clear_stream_info(bool reset_streaming_info = true);
	void clear_streamed_entities();

	bool transfer_entity_ownership(game_entity_base* entity, game_player* new_player, int timeout = -1);
	bool acquire_entity_ownership(game_entity_base* entity, int timeout = -1);
	bool spawn(const int_vec3& pos, const short_vec3& rot, int16_t hp = 1000);
	bool spawn(const int_vec3& pos, int16_t hp = 1000);
	bool spawn(int16_t hp);
	bool is_streaming_synced() const							{ return streaming_synced; }

	int get_ping() const										{ return ping; }

	net_player* get_net()										{ return net; }

	PLAYER_ID get_id() const;

	gns::sync::stream_sync_info* get_stream_info_for_entity(SYNC_ID sid);

	const std::string& get_name() const;

	static bool check_class(game_entity_base* i)				{ return i->get_type() == ENTITY_TYPE_PLAYER; }
};