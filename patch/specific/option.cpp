#include "standard.h"
#include "global.h"
#include "input.h"
#include "init.h"

#include <game/invdata.h>
#include <game/objects.h>
#include <game/inventry.h>
#include <game/gameflow.h>

#include <keycode/keycode.h>

#define SAWIDTH 140

TEXTSTRING* sa_text[3] = { nullptr };

void do_compass_option(INVENTORY_ITEM* inv_item);
void do_detail_option(INVENTORY_ITEM* inv_item);
void do_sound_option(INVENTORY_ITEM* inv_item);

void do_inventory_options(INVENTORY_ITEM* inv_item)
{
	switch (inv_item->object_number)
	{
	case GUN_OPTION:
	case SHOTGUN_OPTION:
	case MAGNUM_OPTION:
	case UZI_OPTION:
	case HARPOON_OPTION:
	case M16_OPTION:
	case ROCKET_OPTION:
		inv_input |= SELECT_ITEM;
		break;
	case GUN_AMMO_OPTION:
	case SG_AMMO_OPTION:
	case MAG_AMMO_OPTION:
	case UZI_AMMO_OPTION:
	case HARPOON_AMMO_OPTION:
	case M16_AMMO_OPTION:
	case ROCKET_AMMO_OPTION:
	case CONTROL_OPTION:
		break;
	case MEDI_OPTION:
	case BIGMEDI_OPTION:
	case PICKUP_OPTION1:
	case PICKUP_OPTION2:
	case PUZZLE_OPTION1:
	case PUZZLE_OPTION2:
	case PUZZLE_OPTION3:
	case PUZZLE_OPTION4:
	case KEY_OPTION1:
	case KEY_OPTION2:
	case KEY_OPTION3:
	case KEY_OPTION4:
		inv_input |= SELECT_ITEM;
		break;
	case MAP_OPTION:
		do_compass_option(inv_item);
		break;
	case DETAIL_OPTION:
		do_detail_option(inv_item);
		break;
	case SOUND_OPTION:
		do_sound_option(inv_item);
		break;
	default:
		if (inv_input & SELECT_ITEM || inv_input & DESELECT_ITEM)
		{
			inv_item->goal_frame = 0;
			inv_item->anim_direction = -1;
		}
	}
}

void do_detail_option(INVENTORY_ITEM* inv_item)
{
	std::string formatted_string;

	if (!sa_text[0])
	{
		if (f_gamma > 10.f)
			f_gamma = 10.f;

		formatted_string = std::format("{:.1f}", f_gamma);

		sa_text[0] = T_Print(0, 0, 0, formatted_string.c_str());

		T_AddBackground(sa_text[0], SAWIDTH - 12, 0, 0, 0, 8, C_BLACK, nullptr, 0);
		T_AddOutline(sa_text[0], 1, C_ORANGE, nullptr, 0);

		// the box

		sa_text[1] = T_Print(0, -32, 0, " ");

		T_AddBackground(sa_text[1], SAWIDTH, 60, 0, 0, 48, C_BLACK, nullptr, 0);
		T_AddOutline(sa_text[1], 1, C_BLUE, nullptr, 0);

		sa_text[2] = T_Print(0, -30, 0, GF_PCStrings[PCSTR_GAMMA]);
		T_AddBackground(sa_text[2], SAWIDTH - 4, 0, 0, 0, 8, C_BLACK, nullptr, 0);
		T_AddOutline(sa_text[2], 1, C_BLUE, nullptr, 0);

		for (int i = 0; i < 3; ++i)
		{
			T_CentreH(sa_text[i], 1);
			T_CentreV(sa_text[i], 1);
		}
	}

	if ((inv_input & DEC_HBAR) && f_gamma > 0.5f)
	{
		idelay = 1;
		idcount = 5;

		f_gamma = std::clamp(f_gamma - 0.5f, 0.f, 10.f);

		formatted_string = std::format("{:.1f}", f_gamma);

		T_ChangeText(sa_text[0], formatted_string.c_str());

		refresh_gamma_ramp();

		g_audio->play_sound(115);
	}
	else if ((inv_input & INC_HBAR) && f_gamma < 10.f)
	{
		idelay = 1;
		idcount = 5;

		f_gamma = std::clamp(f_gamma + 0.5f, 0.f, 10.f);

		formatted_string = std::format("{:.1f}", f_gamma);

		T_ChangeText(sa_text[0], formatted_string.c_str());

		refresh_gamma_ramp();

		g_audio->play_sound(115);
	}

	if (inv_input & (DESELECT_ITEM | SELECT_ITEM))
	{
		T_RemovePrint(sa_text[0]); sa_text[0] = nullptr;
		T_RemovePrint(sa_text[1]); sa_text[1] = nullptr;
		T_RemovePrint(sa_text[2]); sa_text[2] = nullptr;
	}
}

void do_sound_option(INVENTORY_ITEM* inv_item)
{
	std::string formatted_string;

	auto current_volume = g_audio->get_master_volume();

	if (!sa_text[0])
	{
		if (current_volume > 1.f)
			current_volume = 1.f;

		formatted_string = std::format("{:.1f}", current_volume * 10.f);

		sa_text[0] = T_Print(0, 0, 0, formatted_string.c_str());

		T_AddBackground(sa_text[0], SAWIDTH - 12, 0, 0, 0, 8, C_BLACK, nullptr, 0);
		T_AddOutline(sa_text[0], 1, C_ORANGE, nullptr, 0);

		// the box

		sa_text[1] = T_Print(0, -32, 0, " ");

		T_AddBackground(sa_text[1], SAWIDTH, 60, 0, 0, 48, C_BLACK, nullptr, 0);
		T_AddOutline(sa_text[1], 1, C_BLUE, nullptr, 0);

		sa_text[2] = T_Print(0, -30, 0, GF_PCStrings[PCSTR_SETVOLUME]);
		T_AddBackground(sa_text[2], SAWIDTH - 4, 0, 0, 0, 8, C_BLACK, nullptr, 0);
		T_AddOutline(sa_text[2], 1, C_BLUE, nullptr, 0);

		for (int i = 0; i < 3; ++i)
		{
			T_CentreH(sa_text[i], 1);
			T_CentreV(sa_text[i], 1);
		}
	}

	if ((inv_input & DEC_HBAR) && current_volume > 0.f)
	{
		idelay = 1;
		idcount = 5;

		current_volume = std::clamp(current_volume - 0.05f, 0.f, 1.f);

		formatted_string = std::format("{:.1f}", current_volume * 10.f);

		T_ChangeText(sa_text[0], formatted_string.c_str());

		g_audio->set_master_volume(current_volume);
		g_audio->play_sound(115);
	}
	else if ((inv_input & INC_HBAR) && current_volume < 1.f)
	{
		idelay = 1;
		idcount = 5;

		current_volume = std::clamp(current_volume + 0.05f, 0.f, 1.f);

		formatted_string = std::format("{:.1f}", current_volume * 10.f);

		T_ChangeText(sa_text[0], formatted_string.c_str());

		g_audio->set_master_volume(current_volume);
		g_audio->play_sound(115);
	}

	if (inv_input & (DESELECT_ITEM | SELECT_ITEM))
	{
		T_RemovePrint(sa_text[0]); sa_text[0] = nullptr;
		T_RemovePrint(sa_text[1]); sa_text[1] = nullptr;
		T_RemovePrint(sa_text[2]); sa_text[2] = nullptr;
	}
}

void do_compass_option(INVENTORY_ITEM* inv_item)
{
	int s = (10000 / 30) % 60,
		h = s / 3600,
		m = (s / 60) % 60;

	char time_str[32];

	sprintf_s(time_str, "%02d:%02d:%02d", h, m, s);

	if (inv_input & (DESELECT_ITEM | SELECT_ITEM))
	{
		inv_item->goal_frame = inv_item->frames_total - 1;
		inv_item->anim_direction = 1;
	}

	if (inv_item->misc_data[0] != inv_item->misc_data[1])
		g_audio->play_sound(69, {}, 5.f);
}

int get_render_height()
{
	return phd_winheight;
}

int get_render_width()
{
	return phd_winwidth;
}