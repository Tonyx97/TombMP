#include <shared/defs.h>

#include <server/server.h>

#include "entity.h"
#include "player.h"

game_entity::game_entity(int stype, int16_t id, SYNC_ID sid)
{
	type = ENTITY_TYPE_LEVEL;
	subtype = stype;
	sync_id = sid;
	item_id = id;
}

game_entity::~game_entity()
{
	if (auto streamer = get_streamer())
		streamer->transfer_entity_ownership(this, nullptr);
}