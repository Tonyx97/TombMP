#include <shared/defs.h>

#include <sol/sol.hpp>

#include <specific/standard.h>

#include <game/effect2.h>

#include "cf_defs.h"

void cf_fx::register_functions(sol::state* vm)
{
	vm->set_function("fxAddBlood", DoBloodSplatEx);
	vm->set_function("fxAddBloodLots", DoLotsOfBlood);

	vm->set_function("fxAddBubble", [&](int x, int y, int z, int16_t room_id, int size, int sizerange)
	{
		PHD_3DPOS pos { x, y, z };

		CreateBubble(&pos, room_id, size, sizerange);
	});

	vm->set_function("fxAddRicochet", [&](int x, int y, int z, int angle, int size)
	{
		GAME_VECTOR pos { x, y, z };

		TriggerRicochetSpark(&pos, angle, size);
	});

	vm->set_function("fxAddFlareSparks", [&](int x, int y, int z, int xv, int yv, int zv, int smoke)
	{
		TriggerFlareSparks(x, y, z, xv, yv, zv, smoke);
	});

	vm->set_function("fxAddExplosionSparks", [&](int x, int y, int z, int extra_trigs, int dyn, int uw, int16_t room_id)
	{
		TriggerExplosionSparks(x, y, z, extra_trigs, dyn, uw, room_id);
	});

	vm->set_function("fxAddExplosionSmokeEnd", [&](int x, int y, int z, int uw)
	{
		TriggerExplosionSmokeEnd(x, y, z, uw);
	});

	vm->set_function("fxAddExplosionSmoke", [&](int x, int y, int z, int uw)
	{
		TriggerExplosionSmoke(x, y, z, uw);
	});

	vm->set_function("fxAddFireSmoke", [&](int x, int y, int z, int body_part, int type)
	{
		TriggerFireSmoke(x, y, z, body_part, type);
	});

	vm->set_function("fxAddFireFlame", [&](int x, int y, int z, int body_part, int type)
	{
		TriggerFireFlame(x, y, z, body_part, type);
	});

	vm->set_function("fxAddStaticFlame", [&](int x, int y, int z, int size)
	{
		TriggerStaticFlame(x, y, z, size);
	});

	vm->set_function("fxAddAlertLight", [&](int x, int y, int z, int r, int g, int b, int angle, int16_t room_id)
	{
		TriggerAlertLight(x, y, z, r, g, b, angle, room_id);
	});

	vm->set_function("fxAddGunShell", [&](int x, int y, int z, int ry, int type, int weapon, bool left, int16_t room_id)
	{
		TriggerGunShell(x, y, z, ry, type, weapon, left, room_id, false);
	});
	
	vm->set_function("fxAddGunSmoke", [&](int x, int y, int z, int xv, int yv, int zv, int initial, int weapon, int count, int16_t room_id)
	{
		TriggerGunSmoke(x, y, z, xv, yv, zv, initial, weapon, count, room_id);
	});
	
	vm->set_function("fxAddExplosionBubble", [&](int x, int y, int z, int16_t room_id)
	{
		TriggerExplosionBubble(x, y, z, room_id);
	});
	
	vm->set_function("fxAddWaterfallMist", [&](int x, int y, int z, int angle)
	{
		TriggerWaterfallMist(x, y, z, angle);
	});
	
	vm->set_function("fxAddShotgunSparks", [&](int x, int y, int z, int xv, int yv, int zv)
	{
		TriggerShotgunSparks(x, y, z, xv, yv, zv);
	});
	
	vm->set_function("fxAddRocketFlame", [&](int x, int y, int z, int xv, int yv, int zv, int item_id)
	{
		TriggerRocketFlame(x, y, z, xv, yv, zv, item_id);
	});
	
	vm->set_function("fxAddRocketSmoke", [&](int x, int y, int z, int body_part)
	{
		TriggerRocketSmoke(x, y, z, body_part);
	});
	
	vm->set_function("fxAddDarkSmoke", [&](int x, int y, int z, int xv, int zv, int hit)
	{
		TriggerDartSmoke(x, y, z, xv, zv, hit);
	});
	
	vm->set_function("fxAddBreath", [&](int x, int y, int z, int xv, int yv, int zv)
	{
		TriggerBreath(x, y, z, xv, yv, zv);
	});
}