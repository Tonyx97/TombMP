#pragma once

#include "text.h"

#define MAIN_RING			0
#define	OPTION_RING			1
#define	KEYS_RING			2

#define MAX_INV_OBJ 30

#define	DESELECT_ITEM	IN_DESELECT
#define	SELECT_ITEM		IN_SELECT
#define	ROTATE_RIGHT	IN_RIGHT
#define	ROTATE_LEFT		IN_LEFT
#define	EXIT_INVENTORY	(IN_DESELECT|IN_OPTION)

#define PASSPORT_2FRONT	IN_LEFT
#define PASSPORT_2BACK	IN_RIGHT

#define	DEC_HBAR		IN_LEFT
#define INC_HBAR		IN_RIGHT
#define	PREV_HBAR		IN_FORWARD
#define	NEXT_HBAR		IN_BACK

enum INV_ORDER
{
	MAP_POS,
	GUN_POS,
	SGN_POS,
	MAG_POS,
	UZI_POS,
	M16_POS,
	ROCKET_POS,
	GRENADE_POS,
	HARPOON_POS,
	FLR_POS,
	EXP_POS,
	BGM_POS,
	MED_POS,
	SGC_POS
};

enum INV_ORDER2
{
	LBAR_POS = 100,
	KY1_POS,
	KY2_POS,
	KY3_POS,
	KY4_POS,
	PZ4_POS,
	PZ3_POS,
	PZ2_POS,
	PZ1_POS,
	SCN_POS,
	PK2_POS,
	PK1_POS,
	IC1_POS,
	IC2_POS,
	IC3_POS,
	IC4_POS,
};

enum INV_TEXT
{
	IT_NAME,
	IT_QTY,
	IT_NUMBEROF
};

/* Inventory Static Values */
#define	OPEN_FRAMES	32
#define	CLOSE_FRAMES 32
#define OPEN_ROTATION -0x8000
#define CLOSE_ROTATION -0x8000
#define	ROTATE_DURATION	24
#define RINGSWITCH_FRAMES 48
#define	SELECTING_FRAMES 16

#define	CAMERA_STARTHEIGHT	-0x600
#define CAMERA_HEIGHT -0x100
#define	CAMERA_2_RING	0x256
#define RING_RADIUS 0x2b0
#define	CAMERA_YOFFSET -0x60
#define	ITEM_TILT 0x0

/* 'ls_adder' lighting settings      */
#define LOW_LIGHT	0x1400
#define HIGH_LIGHT	0x1000

/* Inventory Status Codes */
enum STATUS_CODES
{
	RNG_OPENING,
	RNG_OPEN,
	RNG_CLOSING,
	RNG_MAIN2OPTION,
	RNG_MAIN2KEYS,
	RNG_KEYS2MAIN,
	RNG_OPTION2MAIN,
	RNG_SELECTING,
	RNG_SELECTED,
	RNG_DESELECTING,
	RNG_DESELECT,
	RNG_CLOSING_ITEM,
	RNG_EXITING_INVENTORY,
	RNG_DONE
};

/* Item orientation when selected */
/* MAP ORIENTATIONS */
#define	MAP_PT_XROT		0x1100
#define MAP_ZTRANS		320
#define	MAP_YTRANS		-170
#define	MAP_X_ROT		-1536
#define MAP_Y_ROT		0
#define MAP_MESH		0xffffffff

/* GUN ORIENTATIONS */
#define	GUNS_PT_XROT	0xc80
#define GUNS_ZTRANS		352
#define	GUNS_YTRANS		38
#define	GUNS_X_ROT		2848
#define GUNS_Y_ROT		-32768
#define GUNS_MESH		0xffffffff

/* SHOTGUN ORIENTATIONS */
#define	SGUN_PT_XROT	0xc80
#define SGUN_ZTRANS		228
#define	SGUN_YTRANS		0
#define	SGUN_X_ROT		5120
#define SGUN_Y_ROT		30720
#define SGUN_MESH		0xffffffff

/* FLARE ORIENTATIONS */
#define	FLAR_PT_XROT	0xc80
#define FLAR_ZTRANS		0x128
#define	FLAR_YTRANS		0
#define	FLAR_X_ROT		0
#define FLAR_Y_ROT		-0x2000
#define FLAR_MESH		0xffffffff

/* AUTOMATIC PISTOLS (was MAGNUM) ORIENTATIONS */
#define	MAGN_PT_XROT	0xc80
#define MAGN_ZTRANS		362
#define	MAGN_YTRANS		0
#define	MAGN_X_ROT		3360
#define MAGN_Y_ROT		-32768
#define MAGN_MESH		0xffffffff

/* UZI ORIENTATIONS */
#define	UZI_PT_XROT		0xc80
#define UZI_ZTRANS		322
#define	UZI_YTRANS		56
#define	UZI_X_ROT		2336
#define UZI_Y_ROT		-32768
#define UZI_MESH		0xffffffff

/* HARPOON ORIENTATIONS */
#define HARPOON_PT_XROT	0xc80
#define HARPOON_ZTRANS  296
#define HARPOON_YTRANS  58
#define HARPOON_X_ROT   -736
#define HARPOON_Y_ROT   -19456
#define HARPOON_MESH    0xffffffff

/* M16 ORIENTATIONS */
#define M16_PT_XROT		0xc80
#define M16_ZTRANS		296
#define M16_YTRANS		84
#define M16_X_ROT		-224
#define M16_Y_ROT		-18432
#define M16_MESH		0xffffffff

/* MINI ROCKET GUN ORIENTATIONS */
#define ROCKET_PT_XROT 0xc80
#define ROCKET_ZTRANS  296
#define ROCKET_YTRANS  56
#define ROCKET_X_ROT   -224
#define ROCKET_Y_ROT   14336
#define ROCKET_MESH    0xffffffff

/* MINI GRENADE GUN ORIENTATIONS */
#define GRENADE_PT_XROT 0xc80
#define GRENADE_ZTRANS  296
#define GRENADE_YTRANS  56
#define GRENADE_X_ROT   -224
#define GRENADE_Y_ROT   14336
#define GRENADE_MESH    0xffffffff

/* GUN AMMO ORIENTATIONS */
#define	GAMO_PT_XROT	0xc80
#define GAMO_ZTRANS		0x128
#define	GAMO_YTRANS		0
#define	GAMO_X_ROT		-0xee0
#define GAMO_Y_ROT		0
#define GAMO_MESH		0xffffffff

/* SHOTGUN AMMO ORIENTATIONS */
#define	SGAM_PT_XROT	0xc80
#define SGAM_ZTRANS		0x128
#define	SGAM_YTRANS		0
#define	SGAM_X_ROT		-0xee0
#define SGAM_Y_ROT		0
#define SGAM_MESH		0xffffffff

/* MAGNUM AMMO ORIENTATIONS */
#define	MGAM_PT_XROT	0xc80
#define MGAM_ZTRANS		0x128
#define	MGAM_YTRANS		0
#define	MGAM_X_ROT		-0xee0
#define MGAM_Y_ROT		0
#define MGAM_MESH		0xffffffff

/* UZI AMMO ORIENTATIONS */
#define	UZAM_PT_XROT	0xc80
#define UZAM_ZTRANS		0x128
#define	UZAM_YTRANS		0
#define	UZAM_X_ROT		-0xee0
#define UZAM_Y_ROT		0
#define UZAM_MESH		0xffffffff

/* HARPOON AMMO ORIENTATIONS */
#define HAM_PT_XROT 0xc80
#define HAM_ZTRANS  0x128
#define HAM_YTRANS  0
#define HAM_X_ROT   -0xee0
#define HAM_Y_ROT   0
#define HAM_MESH    0xffffffff

/* M16 AMMO ORIENTATIONS */
#define M16AM_PT_XROT	0xc80
#define M16AM_ZTRANS		0x128
#define M16AM_YTRANS		0
#define M16AM_X_ROT		-0xee0
#define M16AM_Y_ROT		0
#define M16AM_MESH		0xffffffff

/* ROCKET AMMO ORIENTATIONS */
#define RAM_PT_XROT	0xc80
#define RAM_ZTRANS	0x128
#define RAM_YTRANS	0
#define RAM_X_ROT		-0xee0
#define RAM_Y_ROT		0
#define RAM_MESH		0xffffffff

/* GRENADE AMMO ORIENTATIONS */
#define GRAM_PT_XROT	0xc80
#define GRAM_ZTRANS	0x128
#define GRAM_YTRANS	0
#define GRAM_X_ROT		-0xee0
#define GRAM_Y_ROT		0
#define GRAM_MESH		0xffffffff

/* MEDI ORIENTATIONS */
#define	MEDI_PT_XROT	0xfc0
#define MEDI_ZTRANS		0xd8
#define	MEDI_YTRANS		0
#define	MEDI_X_ROT		-0x1c80
#define MEDI_Y_ROT		-0x1000
#define MEDI_MESH		0xffffffff

/* BIGMEDI ORIENTATIONS */
#define	BMED_PT_XROT	0xe20
#define BMED_ZTRANS		0x160
#define	BMED_YTRANS		0
#define	BMED_X_ROT		-0x1fe0
#define BMED_Y_ROT		-0x1000
#define BMED_MESH		0xffffffff

/* LEAD BAR ORIENTATIONS */
#define	LBAR_PT_XROT	0xe20
#define LBAR_ZTRANS		0x160
#define	LBAR_YTRANS		0
#define	LBAR_X_ROT		-0x1fe0
#define LBAR_Y_ROT		-0x1000
#define LBAR_MESH		0xffffffff

/* PICKUP1 ORIENTATIONS */
#define	PCK1_PT_XROT	0x1c20
#define PCK1_ZTRANS		0x100
#define	PCK1_YTRANS		0
#define	PCK1_X_ROT		-0x1100
#define PCK1_Y_ROT		0
#define PCK1_MESH		0xffffffff

/* PICKUP2 ORIENTATIONS */
#define	PCK2_PT_XROT	0x1c20
#define PCK2_ZTRANS		0x100
#define	PCK2_YTRANS		0
#define	PCK2_X_ROT		-0x1100
#define PCK2_Y_ROT		0
#define PCK2_MESH		0xffffffff

/* SCION ORIENTATIONS */
#define	SCON_PT_XROT	0x1c20
#define SCON_ZTRANS		0x100
#define	SCON_YTRANS		0
#define	SCON_X_ROT		-0x1100
#define SCON_Y_ROT		0
#define SCON_MESH		0xffffffff

/* PUZZLE1 ORIENTATIONS */
#define	PUZ1_PT_XROT	0x1c20
#define PUZ1_ZTRANS		0x100
#define	PUZ1_YTRANS		0
#define	PUZ1_X_ROT		-0x1100
#define PUZ1_Y_ROT		0
#define PUZ1_MESH		0xffffffff

/* PUZZLE2 ORIENTATIONS */
#define	PUZ2_PT_XROT	0x1c20
#define PUZ2_ZTRANS		0x100
#define	PUZ2_YTRANS		0
#define	PUZ2_X_ROT		-0x1100
#define PUZ2_Y_ROT		0
#define PUZ2_MESH		0xffffffff

/* PUZZLE3 ORIENTATIONS */
#define	PUZ3_PT_XROT	0x1c20
#define PUZ3_ZTRANS		0x100
#define	PUZ3_YTRANS		0
#define	PUZ3_X_ROT		-0x1100
#define PUZ3_Y_ROT		0
#define PUZ3_MESH		0xffffffff

/* PUZZLE4 ORIENTATIONS */
#define	PUZ4_PT_XROT	0x1c20
#define PUZ4_ZTRANS		0x100
#define	PUZ4_YTRANS		0
#define	PUZ4_X_ROT		-0x1100
#define PUZ4_Y_ROT		0
#define PUZ4_MESH		0xffffffff

/* KEY1 ORIENTATIONS */
#define	KEY1_PT_XROT	0x1c20
#define KEY1_ZTRANS		0x100
#define	KEY1_YTRANS		0
#define	KEY1_X_ROT		-0x1100
#define KEY1_Y_ROT		0
#define KEY1_MESH		0xffffffff

/* KEY2 ORIENTATIONS */
#define	KEY2_PT_XROT	0x1c20
#define KEY2_ZTRANS		0x100
#define	KEY2_YTRANS		0
#define	KEY2_X_ROT		-0x1100
#define KEY2_Y_ROT		0
#define KEY2_MESH		0xffffffff

/* KEY3 ORIENTATIONS */
#define	KEY3_PT_XROT	0x1c20
#define KEY3_ZTRANS		0x100
#define	KEY3_YTRANS		0
#define	KEY3_X_ROT		-0x1100
#define KEY3_Y_ROT		0
#define KEY3_MESH		0xffffffff

/* KEY4 ORIENTATIONS */
#define	KEY4_PT_XROT	0x1c20
#define KEY4_ZTRANS		0x100
#define	KEY4_YTRANS		0
#define	KEY4_X_ROT		-0x1100
#define KEY4_Y_ROT		0
#define KEY4_MESH		0xffffffff

/* DETAIL ORIENTATIONS */
#define	DETL_PT_XROT	0x1080
#define DETL_ZTRANS		444
#define	DETL_YTRANS		16
#define	DETL_X_ROT		-7232
#define DETL_Y_ROT		0
#define DETL_MESH		0xffffffff

/* SOUND ORIENTATIONS */
#define	SND_PT_XROT		0x12e0
#define SND_ZTRANS		350
#define	SND_YTRANS		-2
#define	SND_X_ROT		-5408
#define SND_Y_ROT		-3072
#define SND_MESH		0xffffffff

/* CONTROL ORIENTATIONS */
#define	CTRL_PT_XROT	0x1580
#define CTRL_ZTRANS		508
#define	CTRL_YTRANS		46
#define	CTRL_X_ROT		-2560
#define CTRL_Y_ROT		13312
#define CTRL_MESH		0xffffffff

#define GAMMA_MODIFIER	8
#define	MIN_GAMMA_LEVEL -127
#define MAX_GAMMA_LEVEL 127

#define	SOUND_MODIFIER	8
#define	MIN_SOUND_LEVEL 0
#define	MAX_SOUND_LEVEL 255

#define DETAIL_MODIFIER	8
#define	MIN_DETAIL_LEVEL 0
#define	MAX_DETAIL_LEVEL 255

struct IMOTION_INFO
{
	int16_t count;					//  Frames to perform motion in
	int16_t status;					//  Current status of motion  ie 'OPENING', 'CLOSING'
	int16_t status_target;			//  Target status of motion ie 'OPEN', 'SELECTED', 'EXITING_INVENTORY'
	int16_t radius_target;			//  Ring target radius after motion
	int16_t radius_rate;				//	Ring radius adjust rate per frame
	int16_t camera_ytarget;			//  Camera target Y position after motion
	int16_t camera_yrate;			//	Camera adjust rate per frame
	int16_t camera_pitch_target;
	int16_t camera_pitch_rate;
	int16_t rotate_target;			//	Ring rotation target after motion
	int16_t rotate_rate;				//	Ring rotate rate thru motion
	PHD_ANGLE item_ptxrot_target;	//	Selected item's pre-translation x rot target after motion
	PHD_ANGLE item_ptxrot_rate;		//	Selected item's Pre-translation x rot rate thru motion
	PHD_ANGLE item_xrot_target;		//	Selected item's target x rot after motion
	PHD_ANGLE item_xrot_rate;		//	Selected item's x rot rate thru motion
	int32_t item_ytrans_target;		//	Selected item's target y translation after motion
	int32_t item_ytrans_rate;		//	Selected item's y translate rate thru motion
	int32_t item_ztrans_target;		//	Selected item's target z translation after motion
	int32_t item_ztrans_rate;		//	Selected item's z translation rate thru motion
	int32_t misc;					//  Misc data
};

struct INVENTORY_SPRITE
{
	int16_t shape;
	int16_t x;
	int16_t y;
	int16_t z;
	int32_t param1;
	int32_t param2;
	SG_COL* grdptr;
	int16_t sprnum;
};

struct INVENTORY_ITEM
{
	char* itemText;
	int16_t object_number;
	int16_t frames_total;
	int16_t current_frame;
	int16_t goal_frame;
	int16_t open_frame;
	int16_t anim_direction;
	int16_t anim_speed;
	int16_t anim_count;
	PHD_ANGLE pt_xrot_sel;
	PHD_ANGLE pt_xrot;
	PHD_ANGLE x_rot_sel;
	PHD_ANGLE x_rot_nosel;
	PHD_ANGLE x_rot;
	PHD_ANGLE y_rot_sel;
	PHD_ANGLE y_rot;
	int32_t ytrans_sel;
	int32_t ytrans;
	int32_t ztrans_sel;
	int32_t ztrans;
	uint32_t which_meshes;
	uint32_t drawn_meshes;
	int16_t inv_pos;
	INVENTORY_SPRITE** sprlist;
	int32_t misc_data[4];
};

struct RING_INFO
{
	INVENTORY_ITEM** list;
	int16_t type;
	int16_t radius;
	int16_t camera_pitch;
	int16_t rotating;
	int16_t rot_count;
	int16_t current_object;
	int16_t target_object;
	int16_t number_of_objects;
	int16_t angle_adder;
	int16_t rot_adder;
	int16_t rot_adderL;
	int16_t rot_adderR;
	PHD_3DPOS ringpos;
	PHD_3DPOS camera;
	PHD_VECTOR light;
	IMOTION_INFO* imo;
};

enum R_flags
{
	R_CENTRE = 0,
	R_USE = (1 << 0),
	R_LEFTALIGN = (1 << 1),
	R_RIGHTALIGN = (1 << 2),
	R_HEADING = (1 << 3),
	R_BEST_TIME = (1 << 4),    // For gym stats text.
	R_NORMAL_TIME = (1 << 5),
	R_NO_TIME = (1 << 6),
};

extern INVENTORY_ITEM icompass_option;
extern INVENTORY_ITEM igun_option;
extern INVENTORY_ITEM iflare_option;
extern INVENTORY_ITEM ishotgun_option;
extern INVENTORY_ITEM imagnum_option;
extern INVENTORY_ITEM iuzi_option;
extern INVENTORY_ITEM iharpoon_option;
extern INVENTORY_ITEM im16_option;
extern INVENTORY_ITEM irocket_option;
extern INVENTORY_ITEM igrenade_option;

extern INVENTORY_ITEM igunammo_option;
extern INVENTORY_ITEM isgunammo_option;
extern INVENTORY_ITEM imagammo_option;
extern INVENTORY_ITEM iuziammo_option;
extern INVENTORY_ITEM iharpoonammo_option;
extern INVENTORY_ITEM im16ammo_option;
extern INVENTORY_ITEM irocketammo_option;
extern INVENTORY_ITEM igrenadeammo_option;

extern INVENTORY_ITEM iexplosiv_option;
extern INVENTORY_ITEM imedi_option;
extern INVENTORY_ITEM ibigmedi_option;
extern INVENTORY_ITEM ipickup1_option;
extern INVENTORY_ITEM ipickup2_option;
extern INVENTORY_ITEM ipuzzle1_option;
extern INVENTORY_ITEM ipuzzle2_option;
extern INVENTORY_ITEM ipuzzle3_option;
extern INVENTORY_ITEM ipuzzle4_option;
extern INVENTORY_ITEM ikey1_option;
extern INVENTORY_ITEM ikey2_option;
extern INVENTORY_ITEM ikey3_option;
extern INVENTORY_ITEM ikey4_option;
extern INVENTORY_ITEM icon1_option;
extern INVENTORY_ITEM icon2_option;
extern INVENTORY_ITEM icon3_option;
extern INVENTORY_ITEM icon4_option;
extern INVENTORY_ITEM sgcrystal_option;

extern INVENTORY_ITEM idetail_option;
extern INVENTORY_ITEM isound_option;

extern int16_t inv_main_objects;
extern int16_t inv_main_current;
extern INVENTORY_ITEM* inv_main_list[];
extern int16_t inv_main_qtys[];

extern int16_t inv_keys_objects;
extern int16_t inv_keys_current;
extern INVENTORY_ITEM* inv_keys_list[];
extern int16_t inv_keys_qtys[];

extern int16_t inv_option_objects;
extern int16_t inv_option_current;
extern INVENTORY_ITEM* inv_option_list[];

extern TEXTSTRING* Inv_itemText[];
extern TEXTSTRING* Inv_levelText;
extern TEXTSTRING* Inv_tagText;
extern TEXTSTRING* Inv_ringText;
extern TEXTSTRING* Inv_upArrow1;
extern TEXTSTRING* Inv_upArrow2;
extern TEXTSTRING* Inv_downArrow1;
extern TEXTSTRING* Inv_downArrow2;

inline INVENTORY_ITEM* inv_item = nullptr;

void Inv_InsertItem(INVENTORY_ITEM* inv_item);
void RingIsOpen(RING_INFO* ring);
void RingIsNotOpen(RING_INFO* ring);
void RingNotActive(INVENTORY_ITEM* inv_item);
void RingActive();
void RemoveInventoryText();

void Inv_RingInit(RING_INFO* ring, int16_t type, INVENTORY_ITEM** list, int16_t qty, int16_t current, IMOTION_INFO* imo);
void Inv_RingGetView(RING_INFO* ring, PHD_3DPOS* viewer);
void Inv_RingLight(RING_INFO* ring, int16_t object_number);
void Inv_RingCalcAdders(RING_INFO* ring, int16_t rotation_duration);
void Inv_RingDoMotions(RING_INFO* ring);
void Inv_RingRotateLeft(RING_INFO* ring);
void Inv_RingRotateRight(RING_INFO* ring);

void Inv_RingMotionInit(RING_INFO* ring, int16_t frames, int16_t status, int16_t status_target);
void Inv_RingMotionSetup(RING_INFO* ring, int16_t status, int16_t status_target, int16_t frames);
void Inv_RingMotionRadius(RING_INFO* ring, int16_t target);
void Inv_RingMotionRotation(RING_INFO* ring, int16_t rotation, int16_t target);
void Inv_RingMotionCameraPos(RING_INFO* ring, int16_t target);
void Inv_RingMotionCameraPitch(RING_INFO* ring, int16_t target);
void Inv_RingMotionItemSelect(RING_INFO* ring, INVENTORY_ITEM* inv_item);
void Inv_RingMotionItemDeselect(RING_INFO* ring, INVENTORY_ITEM* inv_item);

void do_inventory_options(INVENTORY_ITEM* inv_item);

int get_render_height();
int get_render_width();