#include <specific/standard.h>

#include "control.h"
#include "effect2.h"
#include "lara.h"
#include "footprint.h"
#include "laraanim.h"

static constexpr char Sounds[] =
{
	0,	// mud
	5,	// snow
	3,	// sand
	2,	// gravel
	1,	// ice
	9,	// water
	9,	// stone
	4,	// wood
	6,	// metal
	9,	// marble
	3,	// grass
	9,	// concrete
	4,	// old wood
	6	// old metal
};

void InitFootPrints()
{
	for (int i = 0; i < MAX_FOOTPRINTS; ++i)
		FootPrint[i].Active = 0;

	FootPrintNum = 0;
}

void AddFootprint(ITEM_INFO* item)
{
	PHD_VECTOR pos {};

	get_lara_bone_pos(lara_item, &pos, FXType == SFX_LANDONLY ? FOOT_L : FOOT_R);

	auto room_num = item->room_number;
	auto floor = GetFloor(pos.x, pos.y, pos.z, &room_num);

	if (floor->fx != 6 && floor->fx != 5 && floor->fx != 9 && floor->fx != 11)
		g_audio->play_sound(288 + Sounds[floor->fx], { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

	if (floor->fx <= 4 && !OnObject)
	{
		auto f = &FootPrint[FootPrintNum];

		f->x = pos.x;
		f->y = GetHeight(floor, pos.x, pos.y, pos.z);
		f->z = pos.z;
		f->YRot = item->pos.y_rot;
		f->Active = 0x200;

		FootPrintNum = (FootPrintNum + 1) & (MAX_FOOTPRINTS - 1);
	}
}