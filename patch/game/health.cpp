#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "text.h"
#include "health.h"
#include "game.h"

#include <specific/output.h>
#include <specific/init.h>
#include <specific/standard.h>

#include <scripting/events.h>
#include <shared/scripting/resource_system.h>

#define MAX_TIME_ALLOWED	((59 * 60 * 30) + (59 * 30) + 27)
#define NUM_PU				1

struct DISPLAYPU
{
	int16_t duration,
		   sprnum;
};

DISPLAYPU pickups[NUM_PU];

TEXTSTRING* LpModeTS = 0;
int LnModeTSLife = 0;

int FlashIt()
{
	static int flash_state = 0,
			   flash_count = 0;

	if (flash_count)
		--flash_count;
	else
	{
		flash_state ^= 1;
		flash_count = 5;
	}

	return flash_state;
}

void draw_game_info()
{
	DrawAmmoInfo();
	DrawModeInfo();

	if (overlay_flag > 0)
	{
		int flash_state = FlashIt();

		DrawHealthBar(flash_state);
		DrawAirBar(flash_state);

		if (lara.dash < LARA_DASH_TIME)
			S_DrawDashBar((lara.dash * 100) / LARA_DASH_TIME);
	}

	T_DrawText();
}

void DrawHealthBar(int flash_state)
{
	static int old_hitpoints = 0;

	int hitpoints = (int)lara_item->hit_points;

	if (hitpoints < 0)					 hitpoints = 0;
	else if (hitpoints > LARA_HITPOINTS) hitpoints = LARA_HITPOINTS;

	if (old_hitpoints != hitpoints)
	{
		old_hitpoints = hitpoints;
		health_bar_timer = 40;
	}

	if (health_bar_timer < 0)
		health_bar_timer = 0;

	if (hitpoints <= LARA_HITPOINTS >> 2)
		S_DrawHealthBar(flash_state ? hitpoints / 10 : 0, false);
	else if (health_bar_timer > 0 || hitpoints <= 0 || lara.gun_status == LG_READY || lara.poisoned)
		S_DrawHealthBar(hitpoints / 10, lara.poisoned);
}

void DrawAirBar(int flash_state)
{
	if (lara.skidoo == NO_ITEM || items[lara.skidoo].object_number != UPV)
		if (lara.water_status != LARA_UNDERWATER && lara.water_status != LARA_SURFACE && (!(room[lara_item->room_number].flags & SWAMP) || !(lara.water_surface_dist < -775)))
			return;

	int air = (int)lara.air;

	if (air < 0)			 air = 0;
	else if (air > LARA_AIR) air = LARA_AIR;

	int value = (air * 100) / LARA_AIR;

	if (air <= LARA_AIR >> 2)
		S_DrawAirBar(flash_state ? value : 0);
	else S_DrawAirBar(value);
}

void MakeAmmoString(char* string)
{
	for (auto c = string; *c != 0; c++)
	{
		if (*c == 32)			continue;
		else if (*c - 'A' >= 0) *c += 12 - 'A';
		else					*c += 1 - '0';
	}
}

void RemoveAmmoText()
{
	if (ammo_text)
	{
		T_RemovePrint(ammo_text);
		ammo_text = nullptr;
	}
}

void DrawAmmoInfo()
{
	char ammostring[64] { 0 };

	if (lara.skidoo != NO_ITEM && items[lara.skidoo].object_number == UPV)
		sprintf_s(ammostring, "%5d", lara.harpoon.ammo);
	else
	{
		if (lara.gun_status != LG_READY || overlay_flag <= 0)
		{
			RemoveAmmoText();
			return;
		}

		switch (lara.gun_type)
		{
		case LG_PISTOLS:
			return;
		case LG_MAGNUMS:
			sprintf_s(ammostring, "%5d", lara.magnums.ammo);
			break;
		case LG_UZIS:
			sprintf_s(ammostring, "%5d", lara.uzis.ammo);
			break;
		case LG_SHOTGUN:
			sprintf_s(ammostring, "%5d", lara.shotgun.ammo / SHOTGUN_AMMO_CLIP);
			break;
		case LG_HARPOON:
			sprintf_s(ammostring, "%5d", lara.harpoon.ammo);
			break;
		case LG_M16:
			sprintf_s(ammostring, "%5d", lara.m16.ammo);
			break;
		case LG_ROCKET:
			sprintf_s(ammostring, "%5d", lara.rocket.ammo);
			break;
		case LG_GRENADE:
			sprintf_s(ammostring, "%5d", lara.grenade.ammo);
			break;
		default:
			return;
		}
	}

	RemoveAmmoText();

	ammo_text = T_Print(-10, 35, 0, ammostring);
	T_RightAlign(ammo_text, 1);
}

void InitialisePickUpDisplay()
{
	for (int i = 0; i < NUM_PU; ++i)
		pickups[i].duration = 0;
}

void AddDisplayPickup(int16_t objnum)
{
	if (objnum == SECRET_ITEM1 || objnum == SECRET_ITEM2 || objnum == SECRET_ITEM3)
	{
		// play sound
	}

	pickups[0].duration = 75;
	pickups[0].sprnum = objnum;
}

void DisplayModeInfo(char* szString)
{
	if (!szString)
	{
		T_RemovePrint(LpModeTS);
		LpModeTS = 0;
		return;
	}

	if (LpModeTS)
		T_ChangeText(LpModeTS, szString);
	else
	{
		LpModeTS = T_Print(-16, -16, 0, szString);
		T_RightAlign(LpModeTS, 1);
		T_BottomAlign(LpModeTS, 1);
	}

	LnModeTSLife = 75;
}

void DrawModeInfo()
{
	if (LpModeTS)
	{
		if (--LnModeTSLife == 0)
		{
			T_RemovePrint(LpModeTS);
			LpModeTS = 0;
		}
	}
}

void FinishLevel()
{
	if (level_complete)
		return;

	level_complete = g_resource->trigger_event(events::level::ON_LEVEL_FINISH);
}