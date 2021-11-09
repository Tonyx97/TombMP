import prof;

#include <specific/standard.h>
#include <specific/global.h>

#include "objects.h"
#include "lara.h"
#include "inventry.h"
#include "invdata.h"
#include "invfunc.h"
#include "laramisc.h"
#include "health.h"
#include "setup.h"
#include "game.h"

#include <specific/file.h>
#include <specific/frontend.h>

uint32_t GF_ScriptVersion = 0;

int16_t GF_SunsetEnabled = 0,
	   GF_NoFloor = 0,
	   GF_NumSecrets = 3;

char GF_Add2InvItems[ADDINV_NUMBEROF];
char GF_SecretInvItems[ADDINV_NUMBEROF];

int GF_LoadScriptFile(const char* fname)
{
	GF_SunsetEnabled = 0;

	if (!load_gameflow(fname))
		return 0;

#ifdef _DEBUG
	gameflow.cheat_enable = 1;
#endif

	icompass_option.itemText = GF_GameStrings[GT_STOPWATCH];
	igun_option.itemText = GF_GameStrings[GT_PISTOLS];
	iflare_option.itemText = GF_GameStrings[GT_FLARE];
	ishotgun_option.itemText = GF_GameStrings[GT_SHOTGUN];
	imagnum_option.itemText = GF_GameStrings[GT_AUTOPISTOLS];
	iuzi_option.itemText = GF_GameStrings[GT_UZIS];
	iharpoon_option.itemText = GF_GameStrings[GT_HARPOON];
	im16_option.itemText = GF_GameStrings[GT_M16];
	irocket_option.itemText = GF_GameStrings[GT_ROCKETLAUNCHER];
	igrenade_option.itemText = GF_GameStrings[GT_GRENADELAUNCHER];
	igunammo_option.itemText = GF_GameStrings[GT_PISTOLCLIPS];
	isgunammo_option.itemText = GF_GameStrings[GT_SHOTGUNSHELLS];
	imagammo_option.itemText = GF_GameStrings[GT_AUTOPISTOLCLIPS];
	iuziammo_option.itemText = GF_GameStrings[GT_UZICLIPS];
	iharpoonammo_option.itemText = GF_GameStrings[GT_HARPOONBOLTS];
	im16ammo_option.itemText = GF_GameStrings[GT_M16CLIPS];
	irocketammo_option.itemText = GF_GameStrings[GT_ROCKETS];
	igrenadeammo_option.itemText = GF_GameStrings[GT_GRENADES];

	imedi_option.itemText = GF_GameStrings[GT_SMALLMEDI];
	ibigmedi_option.itemText = GF_GameStrings[GT_LARGEMEDI];
	ipickup1_option.itemText = GF_GameStrings[GT_PICKUP];
	ipickup2_option.itemText = GF_GameStrings[GT_PICKUP];
	ipuzzle1_option.itemText = GF_GameStrings[GT_PUZZLE];
	ipuzzle2_option.itemText = GF_GameStrings[GT_PUZZLE];
	ipuzzle3_option.itemText = GF_GameStrings[GT_PUZZLE];
	ipuzzle4_option.itemText = GF_GameStrings[GT_PUZZLE];
	ikey1_option.itemText = GF_GameStrings[GT_KEY];
	ikey2_option.itemText = GF_GameStrings[GT_KEY];
	ikey3_option.itemText = GF_GameStrings[GT_KEY];
	ikey4_option.itemText = GF_GameStrings[GT_KEY];

	icon1_option.itemText = GF_GameStrings[GT_ICON1];
	icon2_option.itemText = GF_GameStrings[GT_ICON2];
	icon3_option.itemText = GF_GameStrings[GT_ICON3];
	icon4_option.itemText = GF_GameStrings[GT_ICON4];
	sgcrystal_option.itemText = GF_GameStrings[GT_CRYSTAL];
	idetail_option.itemText = GF_PCStrings[PCSTR_DETAILLEVEL];
	isound_option.itemText = GF_PCStrings[PCSTR_SOUND];

	prof::print(GREEN, "GF: Schedule successfully initiallised!\n");

	return 1;
}

int GF_InterpretSequence(int16_t* ptr)
{
	GF_SunsetEnabled = GF_NoFloor = 0;
	GF_NumSecrets = 3;

	memset(GF_Add2InvItems, 0, sizeof(GF_Add2InvItems));
	memset(GF_SecretInvItems, 0, sizeof(GF_SecretInvItems));

	int option = EXIT_GAME;

	while (*ptr != GFE_END_SEQ)
	{
		switch (*ptr)
		{
		case GFE_STARTLEVEL:
		{
			ptr += 2;

			return StartGame(*(ptr - 1));
		}
		case GFE_ADD2INV:
		{
			ptr += 2;

			if (auto ptr_val = *(ptr - 1); ptr_val < 1000)
				++GF_SecretInvItems[ptr_val];
			else ++GF_Add2InvItems[ptr_val - 1000];

			break;
		}
		case GFE_SUNSET:
		{
			++ptr;

			GF_SunsetEnabled = 1;

			break;
		}
		case GFE_NOFLOOR:
		{
			ptr += 2;

			GF_NoFloor = *(ptr - 1);

			break;
		}
		case GFE_NUMSECRETS:
		{
			ptr += 2;

			GF_NumSecrets = *(ptr - 1);

			break;
		}
		case GFE_STARTANIM:
		case GFE_DEMOPLAY:
		case GFE_CUTSCENE:
		case GFE_CUTANGLE:
		case GFE_JUMPTO_SEQ:
		case GFE_PICTURE:
		case GFE_SETTRACK:
		case GFE_LOADINGPIC:
		{
			ptr += 2;
			break;
		}
		case GFE_REMOVE_WEAPONS:
		case GFE_REMOVE_AMMO:
		case GFE_LEVCOMPLETE:
		case GFE_GAMECOMPLETE:
		case GFE_KILL2COMPLETE:
		case GFE_DEADLY_WATER:
		case GFE_LIST_END:
		case GFE_LIST_START:
		{
			++ptr;
			break;
		}
		case GFE_PLAYFMV:
		{
			ptr += 2;

			if (*ptr == GFE_PLAYFMV)
				ptr += 2;

			break;
		}
		default:
			return OPENING_GAME;
		}
	}

	return option;
}

void GF_ModifyInventory()
{
	auto add_weapon = [&](object_types weapon, add_inv_types weapon_type, object_types ammo_obj, add_inv_types ammo_type, int amount)
	{
		int total_ammo = 0;
		
		if (Inv_RequestItem(weapon))
			total_ammo += GF_Add2InvItems[ammo_type] * amount;
		else if (GF_Add2InvItems[weapon_type])
		{
			Inv_AddItem(weapon);

			total_ammo += GF_Add2InvItems[ammo_type] * amount;
		}
		else
		{
			for (int i = 0; i < GF_Add2InvItems[ammo_type]; ++i)
				Inv_AddItem(ammo_obj);
		}

		return total_ammo;
	};

	if (GF_Add2InvItems[ADDINV_PISTOLS])
		Inv_AddItem(GUN_ITEM);

	lara.shotgun.ammo	+= add_weapon(SHOTGUN_ITEM, ADDINV_SHOTGUN, SG_AMMO_ITEM, ADDINV_SHOTGUN_AMMO, SHOTGUN_AMMO_QTY);
	lara.magnums.ammo	+= add_weapon(MAGNUM_ITEM, ADDINV_AUTOPISTOLS, MAG_AMMO_ITEM, ADDINV_AUTOPISTOLS_AMMO, MAGNUM_AMMO_QTY);
	lara.uzis.ammo		+= add_weapon(UZI_ITEM, ADDINV_UZIS, UZI_AMMO_ITEM, ADDINV_UZI_AMMO, UZI_AMMO_QTY);
	lara.harpoon.ammo	+= add_weapon(HARPOON_ITEM, ADDINV_HARPOON, HARPOON_AMMO_ITEM, ADDINV_HARPOON_AMMO, HARPOON_AMMO_QTY);
	lara.m16.ammo		+= add_weapon(M16_ITEM, ADDINV_M16, M16_AMMO_ITEM, ADDINV_M16_AMMO, M16_AMMO_QTY);
	lara.rocket.ammo	+= add_weapon(ROCKET_GUN_ITEM, ADDINV_ROCKET, ROCKET_AMMO_ITEM, ADDINV_ROCKET_AMMO, ROCKET_AMMO_QTY);
	lara.grenade.ammo	+= add_weapon(GRENADE_GUN_ITEM, ADDINV_GRENADE, GRENADE_AMMO_ITEM, ADDINV_GRENADE_AMMO, GRENADE_AMMO_QTY);

	for (int i = 0; i < GF_Add2InvItems[ADDINV_FLARES]; ++i)  Inv_AddItem(FLARE_ITEM);
	for (int i = 0; i < GF_Add2InvItems[ADDINV_MEDI]; ++i)    Inv_AddItem(MEDI_ITEM);
	for (int i = 0; i < GF_Add2InvItems[ADDINV_BIGMEDI]; ++i) Inv_AddItem(BIGMEDI_ITEM);

	for (int i = 0; i < GF_Add2InvItems[ADDINV_PICKUP1]; ++i) Inv_AddItem(PICKUP_ITEM1);
	for (int i = 0; i < GF_Add2InvItems[ADDINV_PICKUP2]; ++i) Inv_AddItem(PICKUP_ITEM2);

	for (int i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE1]; ++i) Inv_AddItem(PUZZLE_ITEM1);
	for (int i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE2]; ++i) Inv_AddItem(PUZZLE_ITEM2);
	for (int i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE3]; ++i) Inv_AddItem(PUZZLE_ITEM3);
	for (int i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE4]; ++i) Inv_AddItem(PUZZLE_ITEM4);

	for (int i = 0; i < GF_Add2InvItems[ADDINV_KEY1]; ++i) Inv_AddItem(KEY_ITEM1);
	for (int i = 0; i < GF_Add2InvItems[ADDINV_KEY2]; ++i) Inv_AddItem(KEY_ITEM2);
	for (int i = 0; i < GF_Add2InvItems[ADDINV_KEY3]; ++i) Inv_AddItem(KEY_ITEM3);
	for (int i = 0; i < GF_Add2InvItems[ADDINV_KEY4]; ++i) Inv_AddItem(KEY_ITEM4);

	for (int i = 0; i < GF_Add2InvItems[ADDINV_SAVEGAME_CRYSTAL]; ++i) Inv_AddItem(SAVEGAME_CRYSTAL_ITEM);

	for (int i = 0; i < ADDINV_NUMBEROF; ++i) GF_Add2InvItems[i] = 0;
}