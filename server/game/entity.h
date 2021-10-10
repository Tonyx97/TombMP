#pragma once

#include <shared/game/math.h>
#include <shared/game_net/typedefs.h>

#include "entity_base.h"

class game_entity : public game_entity_base
{
private:

	bool level_entity = false;

public:
	
	game_entity(int stype, int16_t id, SYNC_ID sid);
	~game_entity();

	void set_as_level_entity()						{ level_entity = true; }
	void spawn() override							{ spawned = true; }

	bool is_level_entity() const					{ return level_entity; }

	static bool check_class(game_entity_base* i)	{ return i->get_type() == ENTITY_TYPE_LEVEL; }
};