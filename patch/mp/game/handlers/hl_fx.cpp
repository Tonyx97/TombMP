import utils;

#include <shared/defs.h>

#include <mp/client.h>
#include <mp/game/level.h>

#include <game/effect2.h>
#include <game/larafire.h>

#include "hl_defs.h"

void fx_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_FX_GUN_SMOKE:	return on_fx_gun_smoke();
	case ID_FX_GUNSHELL:	return on_fx_gunshell();
	}
}

void fx_handlers::on_fx_gun_smoke()
{
	gns::fx::gun_smoke info; g_client->read_packet_ex(info);

	if (info.weapon == LG_SHOTGUN)
	{
		for (int i = 0; i < 7; ++i)  TriggerGunSmoke(info.x, info.y, info.z, info.vx, info.vy, info.vz, info.initial, info.weapon, info.count, info.room);
		for (int i = 0; i < 12; ++i) TriggerShotgunSparks(info.x, info.y, info.z, info.vx << 1, info.vy << 1, info.vz << 1);
	}
	else TriggerGunSmoke(info.x, info.y, info.z, info.vx, info.vy, info.vz, info.initial, info.weapon, info.count, info.room);
}

void fx_handlers::on_fx_gunshell()
{
	gns::fx::gunshell info; g_client->read_packet_ex(info);

	TriggerGunShell(info.x, info.y, info.z, info.ry, info.shelltype, info.weapon, info.left, info.room, false);
}