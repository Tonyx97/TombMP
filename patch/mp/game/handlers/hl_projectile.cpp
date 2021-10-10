import utils;

#include <shared/defs.h>

#include <mp/client.h>
#include <mp/game/level.h>

#include <game/items.h>
#include <game/objects.h>
#include <game/control.h>
#include <game/effects.h>
#include <game/effect2.h>
#include <game/larafire.h>

#include "hl_defs.h"

void projectile_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_PROJECTILE_CREATE: return on_projectile_create();
	}
}

void projectile_handlers::on_projectile_create()
{
	gns::projectile::create info; g_client->read_packet_ex(info);

	if (auto item_number = CreateItem(); item_number != NO_ITEM)
	{
		auto item = &items[item_number];

		item->shade = int16_t(0x4210 | 0x8000);
		item->object_number = info.obj;
		item->room_number = info.vec.room;
		item->pos.x_pos = info.vec.pos.x;
		item->pos.y_pos = info.vec.pos.y;
		item->pos.z_pos = info.vec.pos.z;

		InitialiseItem(item_number);

		item->pos.x_rot = info.vec.rot.x;
		item->pos.y_rot = info.vec.rot.y;
		item->pos.z_rot = 0;
		item->fallspeed = info.fallspeed;
		item->speed = info.speed;
		item->hit_points = info.health;
		item->item_flags[1] = 1;			// created remotely (used by lara1gun.cpp to properly sync damage and entities explosions)

		if (info.obj == ROCKET)
		{
			item->item_flags[0] = info.flags0;

			for (int i = 0; i < 5; ++i)
				TriggerGunSmoke(info.vec.pos.x, info.vec.pos.y, info.vec.pos.z, 0, 0, 0, 1, LG_ROCKET, 32);

			phd_PushUnitMatrix();

			*(phd_mxptr + M03) = 0;
			*(phd_mxptr + M13) = 0;
			*(phd_mxptr + M23) = 0;

			phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
			phd_PushMatrix();
			phd_TranslateRel(0, 0, -128);

			int wx = (*(phd_mxptr + M03) >> W2V_SHIFT),
				wy = (*(phd_mxptr + M13) >> W2V_SHIFT),
				wz = (*(phd_mxptr + M23) >> W2V_SHIFT),
				xv, yv, zv;

			phd_PopMatrix();

			for (int i = 0; i < 8; ++i)
			{
				phd_PushMatrix();
				{
					phd_TranslateRel(0, 0, -(GetRandomControl() & 2047));

					xv = (*(phd_mxptr + M03) >> W2V_SHIFT);
					yv = (*(phd_mxptr + M13) >> W2V_SHIFT);
					zv = (*(phd_mxptr + M23) >> W2V_SHIFT);
				}
				phd_PopMatrix();

				TriggerRocketFlame(wx, wy, wz, xv - wx, yv - wy, zv - wz, item_number);
			}

			phd_PopMatrix();
		}
		else if (info.obj == GRENADE)
		{
			item->current_anim_state = info.current_anim_state;
			item->goal_anim_state = info.goal_anim_state;
			item->required_anim_state = info.required_anim_state;
		}

		AddActiveItem(item_number);
	}
}