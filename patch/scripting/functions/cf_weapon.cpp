#include <shared/defs.h>

#include <sol/sol.hpp>

#include <audio/audio_system.h>

#include <game/lara.h>
#include <game/larafire.h>

#include "cf_defs.h"

void cf_weapon::register_functions(sol::state* vm)
{
	vm->set_function("getWeaponNameFromID", [&](int id)
	{
		switch (id)
		{
		case GUN_ITEM:			return "Pistols";
		case MAGNUM_ITEM:		return "Desert Eagle";
		case UZI_ITEM:			return "Uzis";
		case SHOTGUN_ITEM:		return "Shotgun";
		case M16_ITEM:			return "HK";
		case ROCKET_GUN_ITEM:	return "RPG";
		case GRENADE_GUN_ITEM:	return "Grenade Launcher";
		case HARPOON_ITEM:		return "Harpoon";
		default:				return "No weapon";
		}
	});

	vm->set_function("setWeaponAmmo", [&](int id, int ammo)
	{
		switch (id)
		{
		case GUN_ITEM:			lara.pistols.ammo = ammo; break;
		case MAGNUM_ITEM:		lara.magnums.ammo = ammo; break;
		case UZI_ITEM:			lara.uzis.ammo = ammo; break;
		case SHOTGUN_ITEM:		lara.shotgun.ammo = ammo; break;
		case M16_ITEM:			lara.m16.ammo = ammo; break;
		case ROCKET_GUN_ITEM:	lara.rocket.ammo = ammo; break;
		case GRENADE_GUN_ITEM:	lara.grenade.ammo = ammo; break;
		case HARPOON_ITEM:		lara.harpoon.ammo = ammo; break;
		}
	});

	vm->set_function("getWeaponAmmo", [&](int id)
	{
		switch (id)
		{
		case GUN_ITEM:			return lara.pistols.ammo;
		case MAGNUM_ITEM:		return lara.magnums.ammo;
		case UZI_ITEM:			return lara.uzis.ammo;
		case SHOTGUN_ITEM:		return lara.shotgun.ammo;
		case M16_ITEM:			return lara.m16.ammo;
		case ROCKET_GUN_ITEM:	return lara.rocket.ammo;
		case GRENADE_GUN_ITEM:	return lara.grenade.ammo;
		case HARPOON_ITEM:		return lara.harpoon.ammo;
		}

		return -1;
	});

	vm->set_function("setSilencedHK", [&](bool enabled) { g_silenced_hk = enabled; });
}