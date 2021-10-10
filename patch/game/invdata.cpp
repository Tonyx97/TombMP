#include <specific/standard.h>
#include <specific/global.h>

#include "objects.h"
#include "invdata.h"
#include "inventry.h"

#define	GAMM_PT_XROT	0
#define GAMM_ZTRANS		0x100
#define	GAMM_YTRANS		-0xb0
#define	GAMM_X_ROT		0
#define GAMM_Y_ROT		0
#define GAMM_MESH		0xfffffff7

SG_COL inv_colours[C_NUMBER_COLOURS];

SG_COL is_gour1[] =
{
	S_GOURAUD_COL(255, 255, 255, 255), 	// Top & Left Bevel Edge (HIGHLIGHTED)
	S_GOURAUD_COL(160, 160, 160, 160)
};

SG_COL is_gour2[] =
{
	S_GOURAUD_COL(96, 96, 96, 96),		// Bottom & Right Bevel Edge (HIGHLIGHTED)
	S_GOURAUD_COL(32, 32, 32, 32)
};

SG_COL is_gour3[] =
{
	S_GOURAUD_COL(160, 160, 160, 160), 	// Gauge Surround		(HIGHLIGHTED)
	S_GOURAUD_COL(128, 128, 128, 128),
	S_GOURAUD_COL(96, 96, 96, 96),
	S_GOURAUD_COL(128, 128, 128, 128)
};

SG_COL is_gour4[] =
{
	S_GOURAUD_COL(0, 0, 0, 0), 		// Moving Gauge (BLACK>RED)
	S_GOURAUD_COL(255, 0, 0, 255),
	S_GOURAUD_COL(255, 0, 0, 255),
	S_GOURAUD_COL(0, 0, 0, 0)
};

SG_COL is_gour5[] =
{
	S_GOURAUD_COL(0, 0, 0, 0), 		// Moving Gauge (BLACK>ORANGE)
	S_GOURAUD_COL(255, 128, 0, 255),
	S_GOURAUD_COL(255, 128, 0, 255),
	S_GOURAUD_COL(0, 0, 0, 0)
};

SG_COL is_gour6[] =
{
	S_GOURAUD_COL(0, 0, 0, 0), 		// Moving Gauge (BLACK>BLUE)
	S_GOURAUD_COL(0, 128, 255, 255),
	S_GOURAUD_COL(0, 128, 255, 255),
	S_GOURAUD_COL(0, 0, 0, 0)
};

SG_COL is_gour7[] =
{
	S_GOURAUD_COL(0, 0, 0, 0), 		// Moving Gauge (BLACK>MAGENTA)
	S_GOURAUD_COL(255, 0, 255, 255),
	S_GOURAUD_COL(255, 0, 255, 255),
	S_GOURAUD_COL(0, 0, 0, 0)
};

SG_COL is_gour8[] =
{
	S_GOURAUD_COL(223, 223, 223, 223), 	// Top & Left Bevel Edge (DULL)
	S_GOURAUD_COL(128, 128, 128, 128)
};

SG_COL is_gour9[] =
{
	S_GOURAUD_COL(64, 64 ,64, 64),		// Bottom & Right Bevel Edge (DULL)
	S_GOURAUD_COL(0, 0, 0, 0)
};

SG_COL is_gour10[] =
{
	S_GOURAUD_COL(128, 128, 128, 128), 	// Gauge Surround		(DULL)
	S_GOURAUD_COL(96, 96, 96, 96),
	S_GOURAUD_COL(64, 64, 64, 64),
	S_GOURAUD_COL(96, 96, 96, 96)
};
SG_COL is_gour11[] =
{
	S_GOURAUD_COL(0, 0, 0, 0), 		// Moving Gauge (DK ORANGE)
	S_GOURAUD_COL(128, 64, 0, 128),
	S_GOURAUD_COL(128, 64, 0, 128),
	S_GOURAUD_COL(0, 0, 0, 0)
};
SG_COL is_gour12[] =
{
	S_GOURAUD_COL(0, 0, 0, 0), 		// Moving Gauge (DK TURQUOISE)
	S_GOURAUD_COL(0, 64, 128, 128),
	S_GOURAUD_COL(0, 64, 128, 128),
	S_GOURAUD_COL(0, 0, 0, 0)
};

INVENTORY_ITEM icompass_option	= { nullptr/*compass_name*/, MAP_OPTION,   1,0,0, 0,1,1,0, MAP_PT_XROT, 0, MAP_X_ROT, 0,0, MAP_Y_ROT, 0, MAP_YTRANS, 0, MAP_ZTRANS, 0, MAP_MESH,  MAP_MESH,  MAP_POS, nullptr };
INVENTORY_ITEM igun_option		= { nullptr/*pistols_name*/, GUN_OPTION,    12,0,0,11,1,1,0, GUNS_PT_XROT,0, GUNS_X_ROT,0,0, GUNS_Y_ROT,0, GUNS_YTRANS,0, GUNS_ZTRANS,0, GUNS_MESH, GUNS_MESH, GUN_POS, nullptr };

/* Items sometimes in Main List */
INVENTORY_ITEM iflare_option	= { nullptr/*flare_name*/,		FLAREBOX_OPTION,	31,0,0,30,1,1,0, FLAR_PT_XROT,0, FLAR_X_ROT,0,0, FLAR_Y_ROT,0, FLAR_YTRANS,0, FLAR_ZTRANS,0, FLAR_MESH, FLAR_MESH, FLR_POS, nullptr };
INVENTORY_ITEM ishotgun_option	= { nullptr/*shotgun_name*/,	SHOTGUN_OPTION,		13,0,0,12,1,1,0, SGUN_PT_XROT,0, SGUN_X_ROT,0,0, SGUN_Y_ROT,0, SGUN_YTRANS,0, SGUN_ZTRANS,0, SGUN_MESH, SGUN_MESH, SGN_POS, nullptr };
INVENTORY_ITEM imagnum_option	= { nullptr/*magnum_name*/,		MAGNUM_OPTION,		12,0,0,11,1,1,0, MAGN_PT_XROT,0, MAGN_X_ROT,0,0, MAGN_Y_ROT,0, MAGN_YTRANS,0, MAGN_ZTRANS,0, MAGN_MESH, MAGN_MESH, MAG_POS, nullptr };
INVENTORY_ITEM iuzi_option		= { nullptr/*uzi_name*/,   		UZI_OPTION,			13,0,0,12,1,1,0, UZI_PT_XROT, 0, UZI_X_ROT, 0,0,  UZI_Y_ROT, 0, UZI_YTRANS, 0, UZI_ZTRANS, 0, UZI_MESH,  UZI_MESH,  UZI_POS, nullptr };
INVENTORY_ITEM iharpoon_option	= { nullptr/*harpoon_name*/,	HARPOON_OPTION,		12,0,0,11,1,1,0, HARPOON_PT_XROT,0,HARPOON_X_ROT,0,0, HARPOON_Y_ROT, 0, HARPOON_YTRANS, 0, HARPOON_ZTRANS, 0, HARPOON_MESH,  HARPOON_MESH,  HARPOON_POS, nullptr };
INVENTORY_ITEM im16_option		= { nullptr/*m16_name*/,		M16_OPTION,			12,0,0,11,1,1,0, M16_PT_XROT, 0, M16_X_ROT, 0,0, M16_Y_ROT, 0, M16_YTRANS, 0, M16_ZTRANS, 0, M16_MESH,  M16_MESH,  M16_POS, nullptr };
INVENTORY_ITEM irocket_option	= { nullptr/*rocket_name*/,		ROCKET_OPTION,		12,0,0,11,1,1,0, ROCKET_PT_XROT,0,ROCKET_X_ROT,0,0, ROCKET_Y_ROT, 0, ROCKET_YTRANS, 0, ROCKET_ZTRANS, 0, ROCKET_MESH,  ROCKET_MESH,  ROCKET_POS, nullptr };
INVENTORY_ITEM igrenade_option	= { nullptr/*grenade_name*/,	GRENADE_OPTION,		12,0,0,11,1,1,0, GRENADE_PT_XROT,0,GRENADE_X_ROT,0,0, GRENADE_Y_ROT, 0, GRENADE_YTRANS, 0, GRENADE_ZTRANS, 0, GRENADE_MESH,  GRENADE_MESH,  GRENADE_POS, nullptr };

INVENTORY_ITEM igunammo_option		= { nullptr/*gun_ammo_name*/, 	GUN_AMMO_OPTION, 	  1,0,0, 0,1,1,0, GAMO_PT_XROT,0, GAMO_X_ROT,0,0, GAMO_Y_ROT,0, GAMO_YTRANS,0, GAMO_ZTRANS,0, GAMO_MESH, GAMO_MESH, GUN_POS, nullptr };
INVENTORY_ITEM isgunammo_option		= { nullptr/*sg_ammo_name*/, 	SG_AMMO_OPTION, 	  1,0,0, 0,1,1,0, SGAM_PT_XROT,0, SGAM_X_ROT,0,0, SGAM_Y_ROT,0, SGAM_YTRANS,0, SGAM_ZTRANS,0, SGAM_MESH, SGAM_MESH, SGN_POS, nullptr };
INVENTORY_ITEM imagammo_option		= { nullptr/*mag_ammo_name*/, 	MAG_AMMO_OPTION, 	  1,0,0, 0,1,1,0, MGAM_PT_XROT,0, MGAM_X_ROT,0,0, MGAM_Y_ROT,0, MGAM_YTRANS,0, MGAM_ZTRANS,0, MGAM_MESH, MGAM_MESH, MAG_POS, nullptr };
INVENTORY_ITEM iuziammo_option		= { nullptr/*uzi_ammo_name*/,   UZI_AMMO_OPTION, 	  1,0,0, 0,1,1,0, UZAM_PT_XROT,0, UZAM_X_ROT,0,0, UZAM_Y_ROT,0, UZAM_YTRANS,0, UZAM_ZTRANS,0, UZAM_MESH, UZAM_MESH, UZI_POS, nullptr };
INVENTORY_ITEM iharpoonammo_option	= { nullptr/*harpoon_a_nme*/,	HARPOON_AMMO_OPTION,  1,0,0, 0,1,1,0, HAM_PT_XROT, 0, HAM_X_ROT, 0,0, HAM_Y_ROT,0, HAM_YTRANS,0, HAM_ZTRANS,0, HAM_MESH, HAM_MESH, HARPOON_POS, nullptr };
INVENTORY_ITEM im16ammo_option		= { nullptr/*m16_ammo_name*/,	M16_AMMO_OPTION, 	  1,0,0, 0,1,1,0, M16AM_PT_XROT,0,M16AM_X_ROT,0,0, M16AM_Y_ROT,0, M16AM_YTRANS,0, M16AM_ZTRANS,0, M16AM_MESH, M16AM_MESH, M16_POS, nullptr };
INVENTORY_ITEM irocketammo_option	= { nullptr/*rocket_am_name*/,	ROCKET_AMMO_OPTION,   1,0,0, 0,1,1,0, RAM_PT_XROT, 0, RAM_X_ROT, 0,0, RAM_Y_ROT,0, RAM_YTRANS,0, RAM_ZTRANS,0, RAM_MESH, RAM_MESH, ROCKET_POS, nullptr };
INVENTORY_ITEM igrenadeammo_option	= { nullptr/*grenade_am_name*/,	GRENADE_AMMO_OPTION,  1,0,0, 0,1,1,0, GRAM_PT_XROT, 0, GRAM_X_ROT, 0,0, GRAM_Y_ROT,0, GRAM_YTRANS,0, GRAM_ZTRANS,0, GRAM_MESH, GRAM_MESH, GRENADE_POS, nullptr };

INVENTORY_ITEM imedi_option		= { nullptr/*medi_name*/,		MEDI_OPTION, 	 		26,0,0,25,1,1,0, MEDI_PT_XROT,0, MEDI_X_ROT,0,0, MEDI_Y_ROT,0, MEDI_YTRANS,0, MEDI_ZTRANS,0, MEDI_MESH, MEDI_MESH, MED_POS, nullptr };
INVENTORY_ITEM ibigmedi_option	= { nullptr/*bigmedi_name*/,	BIGMEDI_OPTION, 		20,0,0,19,1,1,0, BMED_PT_XROT,0, BMED_X_ROT,0,0, BMED_Y_ROT,0, BMED_YTRANS,0, BMED_ZTRANS,0, BMED_MESH, BMED_MESH, BGM_POS, nullptr };
INVENTORY_ITEM ipickup1_option	= { nullptr/*pickup_name*/,		PICKUP_OPTION1, 		 1,0,0, 0,1,1,0, PCK1_PT_XROT,0, PCK1_PT_XROT,0,0, (PHD_ANGLE)(PCK1_Y_ROT + 0x8000),0, PCK1_YTRANS,0, PCK1_ZTRANS,0, PCK1_MESH, PCK1_MESH, PK1_POS, nullptr };
INVENTORY_ITEM ipickup2_option	= { nullptr/*pickup_name*/,		PICKUP_OPTION2, 		 1,0,0, 0,1,1,0, PCK2_PT_XROT,0, PCK2_X_ROT,0,0, PCK2_Y_ROT,0, PCK2_YTRANS,0, PCK2_ZTRANS,0, PCK2_MESH, PCK2_MESH, PK2_POS, nullptr };
INVENTORY_ITEM ipuzzle1_option	= { nullptr/*puzzle_name*/,		PUZZLE_OPTION1, 		 1,0,0, 0,1,1,0, PUZ1_PT_XROT,0, PUZ1_X_ROT,0,0, PUZ1_Y_ROT,0, PUZ1_YTRANS,0, PUZ1_ZTRANS,0, PUZ1_MESH, PUZ1_MESH, PZ1_POS, nullptr };
INVENTORY_ITEM ipuzzle2_option	= { nullptr/*puzzle_name*/,		PUZZLE_OPTION2, 		 1,0,0, 0,1,1,0, PUZ2_PT_XROT,0, PUZ2_X_ROT,0,0, PUZ2_Y_ROT,0, PUZ2_YTRANS,0, PUZ2_ZTRANS,0, PUZ2_MESH, PUZ2_MESH, PZ2_POS, nullptr };
INVENTORY_ITEM ipuzzle3_option	= { nullptr/*puzzle_name*/,		PUZZLE_OPTION3, 		 1,0,0, 0,1,1,0, PUZ3_PT_XROT,0, PUZ3_X_ROT,0,0, PUZ3_Y_ROT,0, PUZ3_YTRANS,0, PUZ3_ZTRANS,0, PUZ3_MESH, PUZ3_MESH, PZ3_POS, nullptr };
INVENTORY_ITEM ipuzzle4_option	= { nullptr/*puzzle_name*/,		PUZZLE_OPTION4, 		 1,0,0, 0,1,1,0, PUZ4_PT_XROT,0, PUZ4_X_ROT,0,0, PUZ4_Y_ROT,0, PUZ4_YTRANS,0, PUZ4_ZTRANS,0, PUZ4_MESH, PUZ4_MESH, PZ4_POS, nullptr };
INVENTORY_ITEM ikey1_option		= { nullptr/*key_name*/,		KEY_OPTION1, 			 1,0,0, 0,1,1,0, KEY1_PT_XROT,0, KEY1_X_ROT,0,0, KEY1_Y_ROT,0, KEY1_YTRANS,0, KEY1_ZTRANS,0, KEY1_MESH, KEY1_MESH, KY1_POS, nullptr };
INVENTORY_ITEM ikey2_option		= { nullptr/*key_name*/,		KEY_OPTION2, 			 1,0,0, 0,1,1,0, KEY2_PT_XROT,0, KEY2_X_ROT,0,0, KEY2_Y_ROT,0, KEY2_YTRANS,0, KEY2_ZTRANS,0, KEY2_MESH, KEY2_MESH, KY2_POS, nullptr };
INVENTORY_ITEM ikey3_option		= { nullptr/*key_name*/,		KEY_OPTION3, 			 1,0,0, 0,1,1,0, KEY3_PT_XROT,0, KEY3_X_ROT,0,0, KEY3_Y_ROT,0, KEY3_YTRANS,0, KEY3_ZTRANS,0, KEY3_MESH, KEY3_MESH, KY3_POS, nullptr };
INVENTORY_ITEM ikey4_option		= { nullptr/*key_name*/,		KEY_OPTION4, 			 1,0,0, 0,1,1,0, KEY4_PT_XROT,0, KEY4_X_ROT,0,0, KEY4_Y_ROT,0, KEY4_YTRANS,0, KEY4_ZTRANS,0, KEY4_MESH, KEY4_MESH, KY4_POS, nullptr };
INVENTORY_ITEM icon1_option		= { nullptr/*icon_name*/,		ICON_PICKUP1_OPTION,	 1,0,0, 0,1,1,0, KEY1_PT_XROT,0, KEY1_X_ROT,0,0, KEY1_Y_ROT,0, KEY1_YTRANS,0, KEY1_ZTRANS,0, KEY1_MESH, KEY1_MESH, IC1_POS, nullptr };
INVENTORY_ITEM icon2_option		= { nullptr/*icon_name*/,		ICON_PICKUP2_OPTION,	 1,0,0, 0,1,1,0, KEY1_PT_XROT,0, KEY1_X_ROT,0,0, KEY1_Y_ROT,0, KEY1_YTRANS,0, KEY1_ZTRANS,0, KEY1_MESH, KEY1_MESH, IC2_POS, nullptr };
INVENTORY_ITEM icon3_option		= { nullptr/*icon_name*/,		ICON_PICKUP3_OPTION,	 1,0,0, 0,1,1,0, KEY1_PT_XROT,0, KEY1_X_ROT,0,0, KEY1_Y_ROT,0, KEY1_YTRANS,0, KEY1_ZTRANS,0, KEY1_MESH, KEY1_MESH, IC3_POS, nullptr };
INVENTORY_ITEM icon4_option		= { nullptr/*icon_name*/,		ICON_PICKUP4_OPTION,	 1,0,0, 0,1,1,0, KEY1_PT_XROT,0, KEY1_X_ROT,0,0, KEY1_Y_ROT,0, KEY1_YTRANS,0, KEY1_ZTRANS,0, KEY1_MESH, KEY1_MESH, IC4_POS, nullptr };
INVENTORY_ITEM sgcrystal_option = { nullptr/*savegame_name*/,	SAVEGAME_CRYSTAL_OPTION, 1,0,0, 0,1,1,0, KEY1_PT_XROT,0, KEY1_X_ROT,0,0, KEY1_Y_ROT,0, KEY1_YTRANS,0, KEY1_ZTRANS,0, KEY1_MESH, KEY1_MESH, SGC_POS, nullptr };

INVENTORY_ITEM idetail_option	= { nullptr/*detail_name*/,		DETAIL_OPTION, 	     1,0,0, 0,1,1,0, DETL_PT_XROT,0, DETL_X_ROT,0,0, DETL_Y_ROT,0, DETL_YTRANS - 32,0, DETL_ZTRANS,0, DETL_MESH, DETL_MESH, 		 1, nullptr };
INVENTORY_ITEM isound_option	= { nullptr/*sound_name*/,		SOUND_OPTION, 	     1,0,0, 0,1,1,0, SND_PT_XROT, 0, SND_X_ROT, 0,0, SND_Y_ROT, 0, SND_YTRANS - 16, 0, SND_ZTRANS, 0, SND_MESH, 	SND_MESH, 		 2, nullptr };

int16_t inv_main_objects = 8;
int16_t inv_main_current = 0;
int16_t inv_main_select = -1;
int16_t inv_main_qtys[23] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };

INVENTORY_ITEM* inv_main_list[23] =
{
	&icompass_option,
	&iflare_option,
	&igun_option,
	&ishotgun_option,
	&imagnum_option,
	&iuzi_option,
	&im16_option,
	&irocket_option,
	&igrenade_option,
	&iharpoon_option,
	&ibigmedi_option,
	&imedi_option,
	&sgcrystal_option,
};

int16_t inv_keys_objects = 0;
int16_t inv_keys_current = 0;
int16_t inv_keys_select = -1;
int16_t inv_keys_qtys[23] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

INVENTORY_ITEM* inv_keys_list[23] =
{
	&ipuzzle1_option,
	&ipuzzle2_option,
	&ipuzzle3_option,
	&ipuzzle4_option,
	&ikey1_option,
	&ikey2_option,
	&ikey3_option,
	&ikey4_option,
	&ipickup1_option,
	&ipickup2_option,
	&icon1_option,
	&icon2_option,
	&icon3_option,
	&icon4_option,
};

int16_t inv_option_objects = 2;
int16_t inv_option_current = 0;

INVENTORY_ITEM* inv_option_list[] =
{
	&isound_option,
	&idetail_option
};

TEXTSTRING* Inv_itemText[IT_NUMBEROF] = { nullptr };
TEXTSTRING* Inv_levelText = nullptr;
TEXTSTRING* Inv_ringText = nullptr;
TEXTSTRING* Inv_tagText = nullptr;
TEXTSTRING* Inv_upArrow1 = nullptr;
TEXTSTRING* Inv_upArrow2 = nullptr;
TEXTSTRING* Inv_downArrow1 = nullptr;
TEXTSTRING* Inv_downArrow2 = nullptr;