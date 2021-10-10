#pragma once

#include <shared/game/math.h>

#include "entity_base.h"

enum game_entity_flags
{
	GAME_ENTITY_FLAG_ALTER_HEIGHT_BLOCK,
	GAME_ENTITY_FLAG_NONE,
};

class game_entity : public game_entity_base
{
private:

	int flags = GAME_ENTITY_FLAG_NONE;

	bool level_entity = false;

public:

	game_entity(int stype, int16_t id, SYNC_ID sid);
	game_entity(int16_t obj_id, int stype, SYNC_ID sid, const game_vec3d& vec);
	~game_entity();

	void sync(bool ignore_streamer = false) override;
	void force_sync(bool ignore_streamer = false) override;
	void update_by_subtype(const subtype_update_info& info);

	bool spawn() override;

	void set_as_level_entity()								{ level_entity = true; }

	bool is_level_entity() const							{ return level_entity; }

	static bool check_class(game_entity_base* i)			{ return i->get_type() == ENTITY_TYPE_LEVEL; }
};