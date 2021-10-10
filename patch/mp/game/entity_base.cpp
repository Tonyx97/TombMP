#include <shared/defs.h>

#include <mp/client.h>

#include <specific/standard.h>

#include "entity_base.h"

game_entity_base::~game_entity_base()
{
	if (item_id != NO_ITEM && lara.item_number != item_id)
	{
		KillItem(item_id, false);

		item_id = NO_ITEM;
	}
}

void game_entity_base::update_linked_lists()
{
	if (new_room != old_room)
	{
		ItemNewRoom(item_id, new_room);

		old_room = new_room;
	}

	if (old_active != new_active)
	{
		if (old_active)
			RemoveActiveItem(item_id);
		else AddActiveItem(item_id);

		old_active = new_active;
	}
}