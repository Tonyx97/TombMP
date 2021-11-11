#include <specific/standard.h>
#include <specific/input.h>

#include "objects.h"
#include "lara.h"
#include "control.h"
#include "inventry.h"
#include "invfunc.h"
#include "invdata.h"
#include "health.h"
#include "laraanim.h"
#include "game.h"

#include <specific/fn_stubs.h>
#include <specific/frontend.h>
#include <specific/litesrc.h>

#include <ui/ui.h>

#define LIT_MESHES		0x7e
#define	LONDON			2
#define	INDIA			4
#define	PERU			8
#define	NEVADA			16
#define	SPAC			32
#define	ANTARC			64
#define NUM_ASS_TIMES	10

void SelectMeshes(INVENTORY_ITEM* inv_item);

void InitColours()
{
	inv_colours[C_BLACK] = S_COLOUR(0, 0, 0);
	inv_colours[C_GREY] = S_COLOUR(64, 64, 64);
	inv_colours[C_WHITE] = S_COLOUR(255, 255, 255);
	inv_colours[C_RED] = S_COLOUR(255, 0, 0);
	inv_colours[C_ORANGE] = S_COLOUR(255, 128, 0);
	inv_colours[C_YELLOW] = S_COLOUR(255, 255, 0);
	inv_colours[C_DARKGREEN] = S_COLOUR(0, 128, 0);
	inv_colours[C_GREEN] = S_COLOUR(0, 255, 0);
	inv_colours[C_CYAN] = S_COLOUR(0, 255, 255);
	inv_colours[C_BLUE] = S_COLOUR(0, 0, 255);
	inv_colours[C_MAGENTA] = S_COLOUR(255, 0, 255);
}

void RingIsOpen(RING_INFO* ring)
{
	if (!Inv_ringText)
	{
		switch (ring->type)
		{
		case MAIN_RING:		   Inv_ringText = T_Print(0, 26, IT_BRONZE, GF_GameStrings[GT_MAIN_HEADING]); break;
		case OPTION_RING:	   Inv_ringText = T_Print(0, 26, IT_BRONZE, GF_GameStrings[GT_OPTION_HEADING]); break;
		case KEYS_RING:		   Inv_ringText = T_Print(0, 26, IT_BRONZE, GF_GameStrings[GT_KEYS_HEADING]); break;
		}

		T_CentreH(Inv_ringText, 1);
	}

	if (Inventory_Mode == INV_KEYS_MODE)
		return;

	if (!Inv_upArrow1)
	{
		if (ring->type == OPTION_RING || (ring->type == MAIN_RING && inv_keys_objects))
		{
			Inv_upArrow1 = T_Print(20, 28, IT_GREY, "[");
			Inv_upArrow2 = T_Print(-20, 28, IT_GREY, "[");

			T_RightAlign(Inv_upArrow2, 1);
		}
	}

	if (!Inv_downArrow1)
	{
		if ((ring->type == MAIN_RING && !gameflow.lockout_optionring) || ring->type == KEYS_RING)
		{
			Inv_downArrow1 = T_Print(20, -15, IT_GREY, "]");

			T_BottomAlign(Inv_downArrow1, 1);

			Inv_downArrow2 = T_Print(-20, -15, IT_GREY, "]");

			T_BottomAlign(Inv_downArrow2, 1);
			T_RightAlign(Inv_downArrow2, 1);
		}
	}
}

void RingIsNotOpen(RING_INFO* ring)
{
	T_RemovePrint(Inv_tagText);		Inv_tagText = nullptr;
	T_RemovePrint(Inv_ringText);	Inv_ringText = nullptr;
	T_RemovePrint(Inv_upArrow1);	Inv_upArrow1 = nullptr;
	T_RemovePrint(Inv_upArrow2);	Inv_upArrow2 = nullptr;
	T_RemovePrint(Inv_downArrow1);	Inv_downArrow1 = nullptr;
	T_RemovePrint(Inv_downArrow2);	Inv_downArrow2 = nullptr;
}

void RingNotActive(INVENTORY_ITEM* inv_item)
{
	if (!Inv_itemText[IT_NAME])
	{
		switch (inv_item->object_number)
		{
		case PUZZLE_OPTION1: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Puzzle Key 1"); break;
		case PUZZLE_OPTION2: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Puzzle Key 2"); break;
		case PUZZLE_OPTION3: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Puzzle Key 3"); break;
		case PUZZLE_OPTION4: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Puzzle Key 4"); break;
		case PICKUP_OPTION1: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Pickup 1"); break;
		case PICKUP_OPTION2: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Pickup 2"); break;
		case KEY_OPTION1: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Key 1"); break;
		case KEY_OPTION2: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Key 2"); break;
		case KEY_OPTION3: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Key 3"); break;
		case KEY_OPTION4: Inv_itemText[IT_NAME] = T_Print(0, -16, 0, "Key 4"); break;
		case GAMMA_OPTION:
			break;
		default:
			Inv_itemText[IT_NAME] = T_Print(0, -16, IT_BRONZE, inv_item->itemText);
			break;
		}

		if (Inv_itemText[IT_NAME])
		{
			T_BottomAlign(Inv_itemText[IT_NAME], 1);
			T_CentreH(Inv_itemText[IT_NAME], 1);
		}
	}

	std::string temp_text;

	auto tempInt = Inv_RequestItem(inv_item->object_number);

	switch (inv_item->object_number)
	{
	case SHOTGUN_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", int(lara.shotgun.ammo / SHOTGUN_AMMO_CLIP));

		break;
	}
	case MAGNUM_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", lara.magnums.ammo);

		break;
	}
	case UZI_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", lara.uzis.ammo);

		break;
	}
	case HARPOON_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", lara.harpoon.ammo);

		break;
	}
	case M16_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", lara.m16.ammo);

		break;
	}
	case ROCKET_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", lara.rocket.ammo);

		break;
	}
	case GRENADE_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", lara.grenade.ammo);

		break;
	}
	case SG_AMMO_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", tempInt * NUM_SG_SHELLS);

		break;
	}
	case MAG_AMMO_OPTION:
	case UZI_AMMO_OPTION:
	case M16_AMMO_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", tempInt * 2);

		break;
	}
	case HARPOON_AMMO_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", lara.harpoon.ammo);

		break;
	}
	case ROCKET_AMMO_OPTION:
	case GRENADE_AMMO_OPTION:
	case FLAREBOX_OPTION:
	{
		if (Inv_itemText[IT_QTY])
			return;

		temp_text = std::format("{}", tempInt);

		break;
	}
	case MEDI_OPTION:
	case BIGMEDI_OPTION:
		health_bar_timer = 40;
		DrawHealthBar(FlashIt());
	case PUZZLE_OPTION1:
	case PUZZLE_OPTION2:
	case PUZZLE_OPTION3:
	case PUZZLE_OPTION4:
	case PICKUP_OPTION1:
	case PICKUP_OPTION2:
	case KEY_OPTION1:
	case KEY_OPTION2:
	case KEY_OPTION3:
	case KEY_OPTION4:
	case ICON_PICKUP1_OPTION:
	case ICON_PICKUP2_OPTION:
	case ICON_PICKUP3_OPTION:
	case ICON_PICKUP4_OPTION:
	case SAVEGAME_CRYSTAL_OPTION:
	{
		if (Inv_itemText[IT_QTY] || tempInt <= 1)
			return;

		temp_text = std::format("{}", tempInt);

		break;
	}
	default: return;
	}

	if (!Inv_itemText[IT_QTY])
	{
		Inv_itemText[IT_QTY] = T_Print(64, -56, 3, temp_text.c_str());

		T_BottomAlign(Inv_itemText[IT_QTY], 1);
		T_CentreH(Inv_itemText[IT_QTY], 1);
	}
}

void RingActive()
{
	T_RemovePrint(Inv_itemText[IT_NAME]); Inv_itemText[IT_NAME] = nullptr;
	T_RemovePrint(Inv_itemText[IT_QTY]);  Inv_itemText[IT_QTY] = nullptr;
}

int	Inv_AddItem(int itemNum)
{
	int itemNumOption = Inv_GetItemOption(itemNum),
		found = 0,
		n = 0,
		m = 0;

	if (!objects[itemNumOption].loaded)
		return 0;

	for (; n < inv_main_objects; ++n)
		if (auto inv_item = inv_main_list[n]; inv_item->object_number == itemNumOption)
		{
			found = 1;
			break;
		}

	for (; m < inv_keys_objects; ++m)
		if (auto inv_item = inv_keys_list[m]; inv_item->object_number == itemNumOption)
		{
			found = 2;
			break;
		}

	if (found == 1)
	{
		if (itemNum == FLAREBOX_ITEM)		   inv_main_qtys[n] += FLARE_AMMO_BOX;
		else if (itemNum == HARPOON_AMMO_ITEM) lara.harpoon.ammo += HARPOON_AMMO_QTY;
		else								   ++inv_main_qtys[n];

		return 1;
	}
	else if (found == 2)
	{
		++inv_keys_qtys[m];
		return 1;
	}
	else
	{
		switch (itemNum)
		{
		case MAP_CLOSED:
		case MAP_OPTION:
			Inv_InsertItem(&icompass_option);
			return 1;
		case GUN_ITEM:
		case GUN_OPTION:
		{
			Inv_InsertItem(&igun_option);

			if (lara.last_gun_type == LG_UNARMED)
			{
				lara.last_gun_type = LG_PISTOLS;
				lara.mesh_ptrs[THIGH_L] = objects[PISTOLS].mesh_ptr[THIGH_L];
				lara.mesh_ptrs[THIGH_R] = objects[PISTOLS].mesh_ptr[THIGH_R];
			}

			return 1;
		}
		case SHOTGUN_ITEM:
		case SHOTGUN_OPTION:
		{
			if ((found = Inv_RequestItem(SG_AMMO_ITEM)))
			{
				for (int i = 0; i < found; ++i)
				{
					Inv_RemoveItem(SG_AMMO_ITEM);

					lara.shotgun.ammo += SHOTGUN_AMMO_QTY;
				}
			}

			lara.shotgun.ammo += SHOTGUN_AMMO_QTY;

			Inv_InsertItem(&ishotgun_option);

			if (lara.last_gun_type == LG_UNARMED) 	lara.last_gun_type = LG_SHOTGUN;
			if (!lara.back_gun)						lara.back_gun = SHOTGUN;

			GlobalItemReplace(SHOTGUN_ITEM, SG_AMMO_ITEM);

			break;
		}
		case MAGNUM_ITEM:
		case MAGNUM_OPTION:
		{
			if ((found = Inv_RequestItem(MAG_AMMO_ITEM)))
			{
				for (int i = 0; i < found; ++i)
				{
					Inv_RemoveItem(MAG_AMMO_ITEM);

					lara.magnums.ammo += MAGNUM_AMMO_QTY;
				}
			}

			GlobalItemReplace(MAGNUM_ITEM, MAG_AMMO_ITEM);

			lara.magnums.ammo += MAGNUM_AMMO_QTY;

			Inv_InsertItem(&imagnum_option);

			break;
		}
		case UZI_ITEM:
		case UZI_OPTION:
		{
			if ((found = Inv_RequestItem(UZI_AMMO_ITEM)))
			{
				for (int i = 0; i < found; ++i)
				{
					Inv_RemoveItem(UZI_AMMO_ITEM);

					lara.uzis.ammo += UZI_AMMO_QTY;
				}
			}

			GlobalItemReplace(UZI_ITEM, UZI_AMMO_ITEM);

			lara.uzis.ammo += UZI_AMMO_QTY;

			Inv_InsertItem(&iuzi_option);

			break;
		}
		case HARPOON_ITEM:
		case HARPOON_OPTION:
		{
			if ((found = Inv_RequestItem(HARPOON_AMMO_ITEM)))
				for (int i = 0; i < found; ++i)
					Inv_RemoveItem(HARPOON_AMMO_ITEM);

			GlobalItemReplace(HARPOON_ITEM, HARPOON_AMMO_ITEM);

			lara.harpoon.ammo += HARPOON_AMMO_QTY;

			Inv_InsertItem(&iharpoon_option);

			break;
		}
		case M16_ITEM:
		case M16_OPTION:
		{
			if ((found = Inv_RequestItem(M16_AMMO_ITEM)))
			{
				for (int i = 0; i < found; ++i)
				{
					Inv_RemoveItem(M16_AMMO_ITEM);
					lara.m16.ammo += M16_AMMO_QTY;
				}
			}

			GlobalItemReplace(M16_ITEM, M16_AMMO_ITEM);

			lara.m16.ammo += M16_AMMO_QTY;

			Inv_InsertItem(&im16_option);

			break;
		}
		case ROCKET_GUN_ITEM:
		case ROCKET_OPTION:
		{
			if ((found = Inv_RequestItem(ROCKET_AMMO_ITEM)))
			{
				for (int i = 0; i < found; ++i)
				{
					Inv_RemoveItem(ROCKET_AMMO_ITEM);

					lara.rocket.ammo += ROCKET_AMMO_QTY;
				}
			}

			GlobalItemReplace(ROCKET_GUN_ITEM, ROCKET_AMMO_ITEM);

			lara.rocket.ammo += ROCKET_AMMO_QTY;

			Inv_InsertItem(&irocket_option);

			break;
		}
		case GRENADE_GUN_ITEM:
		case GRENADE_OPTION:
		{
			if ((found = Inv_RequestItem(GRENADE_AMMO_ITEM)))
			{
				for (int i = 0; i < found; ++i)
				{
					Inv_RemoveItem(GRENADE_AMMO_ITEM);
					lara.grenade.ammo += GRENADE_AMMO_QTY;
				}
			}

			GlobalItemReplace(GRENADE_GUN_ITEM, GRENADE_AMMO_ITEM);

			lara.grenade.ammo += GRENADE_AMMO_QTY;

			Inv_InsertItem(&igrenade_option);

			break;
		}
		case SG_AMMO_ITEM:
		case SG_AMMO_OPTION:
		{
			if (Inv_RequestItem(SHOTGUN_ITEM))
				lara.shotgun.ammo += SHOTGUN_AMMO_QTY;
			else Inv_InsertItem(&isgunammo_option);

			break;
		}
		case MAG_AMMO_ITEM:
		case MAG_AMMO_OPTION:
		{
			if (Inv_RequestItem(MAGNUM_ITEM))
				lara.magnums.ammo += MAGNUM_AMMO_QTY;
			else Inv_InsertItem(&imagammo_option);

			break;
		}
		case UZI_AMMO_ITEM:
		case UZI_AMMO_OPTION:
		{
			if (Inv_RequestItem(UZI_ITEM))
				lara.uzis.ammo += UZI_AMMO_QTY;
			else Inv_InsertItem(&iuziammo_option);

			break;
		}
		case HARPOON_AMMO_ITEM:
		case HARPOON_AMMO_OPTION:
		{
			lara.harpoon.ammo += HARPOON_AMMO_QTY;

			if (!Inv_RequestItem(HARPOON_ITEM) && !Inv_RequestItem(HARPOON_AMMO_ITEM))
				Inv_InsertItem(&iharpoonammo_option);

			break;
		}
		case M16_AMMO_ITEM:
		case M16_AMMO_OPTION:
		{
			if (Inv_RequestItem(M16_ITEM))
				lara.m16.ammo += M16_AMMO_QTY;
			else Inv_InsertItem(&im16ammo_option);

			break;
		}
		case ROCKET_AMMO_ITEM:
		case ROCKET_AMMO_OPTION:
		{
			if (Inv_RequestItem(ROCKET_GUN_ITEM))
				lara.rocket.ammo += ROCKET_AMMO_QTY;
			else Inv_InsertItem(&irocketammo_option);

			break;
		}
		case GRENADE_AMMO_ITEM:
		case GRENADE_AMMO_OPTION:
		{
			if (Inv_RequestItem(GRENADE_GUN_ITEM))
				lara.grenade.ammo += GRENADE_AMMO_QTY;
			else Inv_InsertItem(&igrenadeammo_option);

			break;
		}
		case FLAREBOX_ITEM:
		case FLAREBOX_OPTION:
		{
			Inv_InsertItem(&iflare_option);

			for (int i = 0; i < (FLARE_AMMO_BOX - 1); ++i)
				Inv_AddItem(FLARE_ITEM);

			return 1;
		}
		case FLARE_ITEM:
			Inv_InsertItem(&iflare_option);
			return 1;
		case MEDI_ITEM:
		case MEDI_OPTION:
			Inv_InsertItem(&imedi_option);
			return 1;
		case BIGMEDI_ITEM:
		case BIGMEDI_OPTION:
			Inv_InsertItem(&ibigmedi_option);
			return 1;
		case SECRET_ITEM1:
			return 1;
		case SECRET_ITEM2:
			return 1;
		case SECRET_ITEM3:
			return 1;
		case PICKUP_ITEM1:
		case PICKUP_OPTION1:
			Inv_InsertItem(&ipickup1_option);
			return 1;
		case PICKUP_ITEM2:
		case PICKUP_OPTION2:
			Inv_InsertItem(&ipickup2_option);
			return 1;
		case PUZZLE_ITEM1:
		case PUZZLE_OPTION1:
			Inv_InsertItem(&ipuzzle1_option);
			return 1;
		case PUZZLE_ITEM2:
		case PUZZLE_OPTION2:
			Inv_InsertItem(&ipuzzle2_option);
			return 1;
		case PUZZLE_ITEM3:
		case PUZZLE_OPTION3:
			Inv_InsertItem(&ipuzzle3_option);
			return 1;
		case PUZZLE_ITEM4:
		case PUZZLE_OPTION4:
			Inv_InsertItem(&ipuzzle4_option);
			return 1;
		case KEY_ITEM1:
		case KEY_OPTION1:
			Inv_InsertItem(&ikey1_option);
			return 1;
		case KEY_ITEM2:
		case KEY_OPTION2:
			Inv_InsertItem(&ikey2_option);
			return 1;
		case KEY_ITEM3:
		case KEY_OPTION3:
			Inv_InsertItem(&ikey3_option);
			return 1;
		case KEY_ITEM4:
		case KEY_OPTION4:
			Inv_InsertItem(&ikey4_option);
			return 1;
		case ICON_PICKUP1_ITEM:
		case ICON_PICKUP1_OPTION:
			Inv_InsertItem(&icon1_option);
			return 1;
		case ICON_PICKUP2_ITEM:
		case ICON_PICKUP2_OPTION:
			Inv_InsertItem(&icon2_option);
			return 1;
		case ICON_PICKUP3_ITEM:
		case ICON_PICKUP3_OPTION:
			Inv_InsertItem(&icon3_option);
			return 1;
		case ICON_PICKUP4_ITEM:
		case ICON_PICKUP4_OPTION:
			Inv_InsertItem(&icon4_option);
			return 1;
		case SAVEGAME_CRYSTAL_ITEM:
		case SAVEGAME_CRYSTAL_OPTION:
			Inv_InsertItem(&sgcrystal_option);
			return 1;
		default: return 0;
		}
	}

	return 0;
}

void Inv_InsertItem(INVENTORY_ITEM* inv_item)
{
	if (inv_item->inv_pos < 100)
	{
		int n = 0;

		for (; n < inv_main_objects; ++n)
			if (auto inv_place = inv_main_list[n]; inv_place->inv_pos > inv_item->inv_pos)
				break;

		if (n == inv_main_objects)
		{
			inv_main_list[inv_main_objects] = inv_item;
			inv_main_qtys[inv_main_objects++] = 1;
		}
		else
		{
			for (int m = inv_main_objects; m > (n - 1); --m)
			{
				inv_main_list[m + 1] = inv_main_list[m];
				inv_main_qtys[m + 1] = inv_main_qtys[m];
			}

			inv_main_list[n] = inv_item;
			inv_main_qtys[n] = 1;

			++inv_main_objects;
		}
	}
	else
	{
		int n = 0;

		for (; n < inv_keys_objects; ++n)
			if (auto inv_place = inv_keys_list[n]; inv_place->inv_pos > inv_item->inv_pos)
				break;

		if (n == inv_keys_objects)
		{
			inv_keys_list[inv_keys_objects] = inv_item;
			inv_keys_qtys[inv_keys_objects++] = 1;
		}
		else
		{
			for (int m = inv_keys_objects; m > (n - 1); --m)
			{
				inv_keys_list[m + 1] = inv_keys_list[m];
				inv_keys_qtys[m + 1] = inv_keys_qtys[m];
			}

			inv_keys_list[n] = inv_item;
			inv_keys_qtys[n] = 1;

			++inv_keys_objects;
		}
	}
}

int Inv_RequestItem(int	itemNum)
{
	int itemNumOption = Inv_GetItemOption(itemNum);

	for (int i = 0; i < inv_main_objects; ++i)
		if (auto inv_item = inv_main_list[i]; inv_item->object_number == itemNumOption)
			return inv_main_qtys[i];

	for (int i = 0; i < inv_keys_objects; ++i)
		if (auto inv_item = inv_keys_list[i]; inv_item->object_number == itemNumOption)
			return inv_keys_qtys[i];

	return 0;
}

void Inv_RemoveAllItems()
{
	inv_main_objects = inv_main_current = 0;
	inv_keys_objects = inv_keys_current = 0;
}

int	Inv_RemoveItem(int itemNum)
{
	int itemNumOption = Inv_GetItemOption(itemNum);

	for (int i = 0; i < inv_main_objects; ++i)
	{
		if (auto inv_item = inv_main_list[i]; inv_item->object_number == itemNumOption)
		{
			if (--inv_main_qtys[i] <= 0)
			{
				--inv_main_objects;

				for (int j = i; j < inv_main_objects; ++j)
				{
					inv_main_list[j] = inv_main_list[j + 1];
					inv_main_qtys[j] = inv_main_qtys[j + 1];
				}
			}

			return 1;
		}
	}

	for (int i = 0; i < inv_keys_objects; ++i)
	{
		if (auto inv_item = inv_keys_list[i]; inv_item->object_number == itemNumOption)
		{
			if (--inv_keys_qtys[i] <= 0)
			{
				--inv_keys_objects;

				for (int j = i; j < inv_keys_objects; ++j)
				{
					inv_keys_list[j] = inv_keys_list[j + 1];
					inv_keys_qtys[j] = inv_keys_qtys[j + 1];
				}
			}

			return 1;
		}
	}

	return 0;
}

int	Inv_GetCount()
{
	int qty = 0;

	for (int i = 0; i < inv_main_objects; ++i)
		qty += inv_main_qtys[i];

	return qty;
}

int	Inv_GetItemOption(int itemNum)
{
	switch (itemNum)
	{
	case GUN_ITEM:
	case GUN_OPTION:				return GUN_OPTION;
	case FLARE_ITEM:
	case FLAREBOX_ITEM:
	case FLAREBOX_OPTION:			return FLAREBOX_OPTION;
	case SHOTGUN_ITEM:
	case SHOTGUN_OPTION:			return SHOTGUN_OPTION;
	case MAGNUM_ITEM:
	case MAGNUM_OPTION:				return MAGNUM_OPTION;
	case UZI_ITEM:
	case UZI_OPTION:				return UZI_OPTION;
	case M16_ITEM:
	case M16_OPTION:				return M16_OPTION;
	case ROCKET_GUN_ITEM:
	case ROCKET_OPTION:				return ROCKET_OPTION;
	case GRENADE_GUN_ITEM:
	case GRENADE_OPTION:			return GRENADE_OPTION;
	case HARPOON_ITEM:
	case HARPOON_OPTION:			return HARPOON_OPTION;
	case SG_AMMO_ITEM:
	case SG_AMMO_OPTION:			return SG_AMMO_OPTION;
	case MAG_AMMO_ITEM:
	case MAG_AMMO_OPTION:			return MAG_AMMO_OPTION;
	case HARPOON_AMMO_ITEM:
	case HARPOON_AMMO_OPTION:		return HARPOON_AMMO_OPTION;
	case M16_AMMO_ITEM:
	case M16_AMMO_OPTION:			return M16_AMMO_OPTION;
	case ROCKET_AMMO_ITEM:
	case ROCKET_AMMO_OPTION:		return ROCKET_AMMO_OPTION;
	case GRENADE_AMMO_ITEM:
	case GRENADE_AMMO_OPTION:		return GRENADE_AMMO_OPTION;
	case UZI_AMMO_ITEM:
	case UZI_AMMO_OPTION:			return UZI_AMMO_OPTION;
	case MEDI_ITEM:
	case MEDI_OPTION:				return MEDI_OPTION;
	case BIGMEDI_ITEM:
	case BIGMEDI_OPTION:			return BIGMEDI_OPTION;
	case PUZZLE_ITEM1:
	case PUZZLE_OPTION1:			return PUZZLE_OPTION1;
	case PUZZLE_ITEM2:
	case PUZZLE_OPTION2:			return PUZZLE_OPTION2;
	case PUZZLE_ITEM3:
	case PUZZLE_OPTION3:			return PUZZLE_OPTION3;
	case PUZZLE_ITEM4:
	case PUZZLE_OPTION4:			return PUZZLE_OPTION4;
	case PICKUP_ITEM1:
	case PICKUP_OPTION1:			return PICKUP_OPTION1;
	case PICKUP_ITEM2:
	case PICKUP_OPTION2:			return PICKUP_OPTION2;
	case KEY_ITEM1:
	case KEY_OPTION1:				return KEY_OPTION1;
	case KEY_ITEM2:
	case KEY_OPTION2:				return KEY_OPTION2;
	case KEY_ITEM3:
	case KEY_OPTION3:				return KEY_OPTION3;
	case KEY_ITEM4:
	case KEY_OPTION4:				return KEY_OPTION4;
	case ICON_PICKUP1_ITEM:
	case ICON_PICKUP1_OPTION:		return ICON_PICKUP1_OPTION;
	case ICON_PICKUP2_ITEM:
	case ICON_PICKUP2_OPTION:		return ICON_PICKUP2_OPTION;
	case ICON_PICKUP3_ITEM:
	case ICON_PICKUP3_OPTION:		return ICON_PICKUP3_OPTION;
	case ICON_PICKUP4_ITEM:
	case ICON_PICKUP4_OPTION:		return ICON_PICKUP4_OPTION;
	case SAVEGAME_CRYSTAL_ITEM:
	case SAVEGAME_CRYSTAL_OPTION:	return SAVEGAME_CRYSTAL_OPTION;
	}

	return -1;
}

void RemoveInventoryText()
{
	for (int i = 0; i < IT_NUMBEROF; ++i)
	{
		T_RemovePrint(Inv_itemText[i]); Inv_itemText[i] = nullptr;
	}
}

void Inv_RingInit(RING_INFO* ring, int16_t type, INVENTORY_ITEM** list, int16_t qty, int16_t current, IMOTION_INFO* imo)
{
	ring->type = type;
	ring->radius = 0;
	ring->list = list;
	ring->number_of_objects = qty;
	ring->current_object = current;
	ring->angle_adder = 0x10000 / qty;
	ring->camera_pitch = 0;
	ring->rotating = 0;
	ring->rot_count = 0;
	ring->target_object = 0;
	ring->rot_adder = 0;
	ring->rot_adderL = 0;
	ring->rot_adderR = 0;
	ring->imo = imo;
	ring->camera.x_pos = 0;
	ring->camera.y_pos = CAMERA_STARTHEIGHT;
	ring->camera.z_pos = 0x380;
	ring->camera.x_rot = 0;
	ring->camera.y_rot = 0;
	ring->camera.z_rot = 0;

	Inv_RingMotionInit(ring, OPEN_FRAMES, RNG_OPENING, RNG_OPEN);
	Inv_RingMotionRadius(ring, RING_RADIUS);
	Inv_RingMotionCameraPos(ring, CAMERA_HEIGHT);
	Inv_RingMotionRotation(ring, OPEN_ROTATION, (int16_t)(0xc000 - (ring->current_object * ring->angle_adder)));

	ring->ringpos.x_pos = 0;
	ring->ringpos.y_pos = 0;
	ring->ringpos.z_pos = 0;
	ring->ringpos.x_rot = 0;
	ring->ringpos.y_rot = imo->rotate_target - OPEN_ROTATION;
	ring->ringpos.z_rot = 0;
	ring->light.x = -0x600;
	ring->light.y = 0x100;
	ring->light.z = 0x400;
}

void Inv_RingGetView(RING_INFO* ring, PHD_3DPOS* viewer)
{
	const auto angles = phd_GetVectorAngles({ 0 - ring->camera.x_pos, CAMERA_YOFFSET - ring->camera.y_pos, ring->radius - ring->camera.z_pos });

	viewer->x_pos = ring->camera.x_pos;
	viewer->y_pos = ring->camera.y_pos;
	viewer->z_pos = ring->camera.z_pos;
	viewer->x_rot = angles.y + ring->camera_pitch;
	viewer->y_rot = angles.x;
	viewer->z_rot = 0;
}

void Inv_RingLight(RING_INFO* ring, int16_t object_number)
{
	LightCol[M00] = 0xcf0;
	LightCol[M10] = 0x680;
	LightCol[M20] = 0x000;

	LightCol[M01] = 0xcf0;
	LightCol[M11] = 0xcf0;
	LightCol[M21] = 0xcf0;

	LightCol[M02] = 0x0;
	LightCol[M12] = 0x0;
	LightCol[M22] = 0xc00;

	LPos[0].x = 4096 << 2;
	LPos[0].y = -4096 << 2;
	LPos[0].z = 3072 << 2;

	LPos[1].x = -4096 << 2;
	LPos[1].y = -4096 << 2;
	LPos[1].z = 3072 << 2;

	LPos[2].x = 0 << 2;
	LPos[2].y = 2048 << 2;
	LPos[2].z = 3072 << 2;

	smcr = smcg = smcb = 32;

	auto mptr = w2v_matrix;

	int x = (mptr[M00] * LPos[0].x + mptr[M01] * LPos[0].y + mptr[M02] * LPos[0].z) >> W2V_SHIFT,
		y = (mptr[M10] * LPos[0].x + mptr[M11] * LPos[0].y + mptr[M12] * LPos[0].z) >> W2V_SHIFT,
		z = (mptr[M20] * LPos[0].x + mptr[M21] * LPos[0].y + mptr[M22] * LPos[0].z) >> W2V_SHIFT;

	LPos[0] = { x, y, z };

	x = (mptr[M00] * LPos[1].x + mptr[M01] * LPos[1].y + mptr[M02] * LPos[1].z) >> W2V_SHIFT;
	y = (mptr[M10] * LPos[1].x + mptr[M11] * LPos[1].y + mptr[M12] * LPos[1].z) >> W2V_SHIFT;
	z = (mptr[M20] * LPos[1].x + mptr[M21] * LPos[1].y + mptr[M22] * LPos[1].z) >> W2V_SHIFT;

	LPos[1] = { x, y, z };

	x = (mptr[M00] * LPos[2].x + mptr[M01] * LPos[2].y + mptr[M02] * LPos[2].z) >> W2V_SHIFT;
	y = (mptr[M10] * LPos[2].x + mptr[M11] * LPos[2].y + mptr[M12] * LPos[2].z) >> W2V_SHIFT;
	z = (mptr[M20] * LPos[2].x + mptr[M21] * LPos[2].y + mptr[M22] * LPos[2].z) >> W2V_SHIFT;

	LPos[2] = { x, y, z };
}

void Inv_RingCalcAdders(RING_INFO* ring, int16_t rotation_duration)
{
	if (ring->number_of_objects == 0)
		return;

	ring->angle_adder = 0x10000 / ring->number_of_objects;
	ring->rot_adderL = ring->angle_adder / rotation_duration;
	ring->rot_adderR = -ring->rot_adderL;
}

void Inv_RingDoMotions(RING_INFO* ring)
{
	auto imo = ring->imo;

	if (imo->count)
	{
		ring->radius += imo->radius_rate;
		ring->camera.y_pos += imo->camera_yrate;
		ring->ringpos.y_rot += imo->rotate_rate;
		ring->camera_pitch += imo->camera_pitch_rate;

		auto inv_item = *(ring->list + ring->current_object);

		inv_item->pt_xrot += imo->item_ptxrot_rate;
		inv_item->x_rot += imo->item_xrot_rate;
		inv_item->ytrans += imo->item_ytrans_rate;
		inv_item->ztrans += imo->item_ztrans_rate;

		if (--imo->count == 0)
		{
			imo->status = imo->status_target;

			if (imo->radius_rate)
			{
				imo->radius_rate = 0;
				ring->radius = imo->radius_target;
			}

			if (imo->camera_yrate)
			{
				imo->camera_yrate = 0;
				ring->camera.y_pos = imo->camera_ytarget;
			}

			if (imo->rotate_rate)
			{
				imo->rotate_rate = 0;
				ring->ringpos.y_rot = imo->rotate_target;
			}

			if (imo->item_ptxrot_rate)
			{
				imo->item_ptxrot_rate = 0;
				inv_item->pt_xrot = imo->item_ptxrot_target;
			}

			if (imo->item_xrot_rate)
			{
				imo->item_xrot_rate = 0;
				inv_item->x_rot = imo->item_xrot_target;
			}

			if (imo->item_ytrans_rate)
			{
				imo->item_ytrans_rate = 0;
				inv_item->ytrans = imo->item_ytrans_target;
			}

			if (imo->item_ztrans_rate)
			{
				imo->item_ztrans_rate = 0;
				inv_item->ztrans = imo->item_ztrans_target;
			}

			if (imo->camera_pitch_rate)
			{
				imo->camera_pitch_rate = 0;
				ring->camera_pitch = imo->camera_pitch_target;
			}
		}
	}

	if (ring->rotating)
	{
		ring->ringpos.y_rot += ring->rot_adder;

		if (--ring->rot_count == 0)
		{
			ring->current_object = ring->target_object;
			ring->ringpos.y_rot = 0xc000 - (ring->current_object * ring->angle_adder);
			ring->rotating = 0;
		}
	}
}

void Inv_RingRotateLeft(RING_INFO* ring)
{
	ring->rotating = 1;
	ring->target_object = ring->current_object - 1;

	if (ring->target_object < 0)
		ring->target_object = ring->number_of_objects - 1;

	ring->rot_count = ROTATE_DURATION;
	ring->rot_adder = ring->rot_adderL;
}

void Inv_RingRotateRight(RING_INFO* ring)
{
	ring->rotating = 1;
	ring->target_object = ring->current_object + 1;

	if (ring->target_object >= ring->number_of_objects)
		ring->target_object = 0;

	ring->rot_count = ROTATE_DURATION;
	ring->rot_adder = ring->rot_adderR;
}

void Inv_RingMotionInit(RING_INFO* ring, int16_t frames, int16_t status, int16_t status_target)
{
	auto imo = ring->imo;

	imo->count = frames;
	imo->status = status;
	imo->status_target = status_target;
	imo->radius_target = 0;
	imo->radius_rate = 0;
	imo->camera_ytarget = 0;
	imo->camera_yrate = 0;
	imo->camera_pitch_target = 0;
	imo->camera_pitch_rate = 0;
	imo->rotate_target = 0;
	imo->rotate_rate = 0;
	imo->item_ptxrot_target = 0;
	imo->item_ptxrot_rate = 0;
	imo->item_xrot_target = 0;
	imo->item_xrot_rate = 0;
	imo->item_ytrans_target = 0;
	imo->item_ytrans_rate = 0;
	imo->item_ztrans_target = 0;
	imo->item_ztrans_rate = 0;
	imo->misc = 0;
}

void Inv_RingMotionSetup(RING_INFO* ring, int16_t status, int16_t status_target, int16_t frames)
{
	auto imo = ring->imo;

	imo->count = frames;
	imo->status = status;
	imo->status_target = status_target;
	imo->radius_rate = 0;
	imo->camera_yrate = 0;
}

void Inv_RingMotionRadius(RING_INFO* ring, int16_t target)
{
	auto imo = ring->imo;

	imo->radius_target = target;
	imo->radius_rate = (target - ring->radius) / imo->count;
}

void Inv_RingMotionRotation(RING_INFO* ring, int16_t rotation, int16_t target)
{
	auto imo = ring->imo;

	imo->rotate_rate = rotation / imo->count;
	imo->rotate_target = target;
}

void Inv_RingMotionCameraPos(RING_INFO* ring, int16_t target)
{
	auto imo = ring->imo;

	imo->camera_ytarget = target;
	imo->camera_yrate = (target - ring->camera.y_pos) / imo->count;
}

void Inv_RingMotionCameraPitch(RING_INFO* ring, int16_t target)
{
	auto imo = ring->imo;

	imo->camera_pitch_target = target;
	imo->camera_pitch_rate = target / imo->count;
}

void Inv_RingMotionItemSelect(RING_INFO* ring, INVENTORY_ITEM* inv_item)
{
	auto imo = ring->imo;

	imo->item_ptxrot_target = inv_item->pt_xrot_sel;
	imo->item_ptxrot_rate = inv_item->pt_xrot_sel / imo->count;
	imo->item_xrot_target = inv_item->x_rot_sel;
	imo->item_xrot_rate = (inv_item->x_rot_sel - inv_item->x_rot_nosel) / imo->count;
	imo->item_ytrans_target = inv_item->ytrans_sel;
	imo->item_ytrans_rate = inv_item->ytrans_sel / imo->count;
	imo->item_ztrans_target = inv_item->ztrans_sel;
	imo->item_ztrans_rate = inv_item->ztrans_sel / imo->count;
}

void Inv_RingMotionItemDeselect(RING_INFO* ring, INVENTORY_ITEM* inv_item)
{
	auto imo = ring->imo;

	imo->item_ptxrot_target = 0;
	imo->item_ptxrot_rate = 0 - inv_item->pt_xrot_sel / imo->count;
	imo->item_xrot_target = inv_item->x_rot_nosel;
	imo->item_xrot_rate = (inv_item->x_rot_nosel - inv_item->x_rot_sel) / imo->count;
	imo->item_ytrans_target = 0;
	imo->item_ytrans_rate = 0 - inv_item->ytrans_sel / imo->count;
	imo->item_ztrans_target = 0;
	imo->item_ztrans_rate = 0 - inv_item->ztrans_sel / imo->count;
}