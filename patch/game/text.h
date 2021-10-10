#pragma once

#include "types.h"

#define	MAX_TEXT_STRINGS 64
#define	MAX_STRING_SIZE  64
#define TEXT_HEIGHT	11

#define SCR_LEFT	0
#define	SCR_RIGHT	get_render_width()
#define SCR_TOP		0
#define	SCR_BOTTOM	get_render_height()

#define IT_DEFAULT 0
#define IT_GOLD 1
#define IT_GREY 2
#define IT_RED 3
#define IT_BLUE 4
#define IT_BRONZE 5

struct TEXTSTRING
{
	uint32_t flags;
	uint16_t textflags;
	uint16_t bgndflags;
	uint16_t outlflags;
	int16_t xpos;
	int16_t ypos;
	int16_t zpos;
	int16_t letterSpacing;
	int16_t wordSpacing;
	int16_t flashRate;
	int16_t flashCount;
	int16_t bgndColour;
	SG_COL* bgndGour;
	int16_t outlColour;
	SG_COL* outlGour;
	int16_t bgndSizeX;
	int16_t bgndSizeY;
	int16_t bgndOffX;
	int16_t bgndOffY;
	int16_t bgndOffZ;
	int32_t scaleH;
	int32_t scaleV;
	int16_t Colour;
	char* string;
};

enum T_flags
{
	T_TOPALIGN = 0,
	T_LEFTALIGN = 0,
	T_ACTIVE = (1 << 0),
	T_FLASH = (1 << 1),
	T_ROTATE_H = (1 << 2),
	T_ROTATE_V = (1 << 3),
	T_CENTRE_H = (1 << 4),
	T_CENTRE_V = (1 << 5),
	T_RIGHTALIGN = (1 << 7),
	T_BOTTOMALIGN = (1 << 8),
	T_ADDBACKGROUND = (1 << 9),
	T_ADDOUTLINE = (1 << 10),
	T_RIGHTJUSTIFY = (1 << 11)
};

// Transparency levels 1-4 (light to heavy)
enum D_flags
{
	D_TRANS1 = 1,
	D_TRANS2 = 2,
	D_TRANS3 = 3,
	D_TRANS4 = 4,
	D_NEXT = (1 << 3)
};

inline int save_toggle = 0;

void T_InitPrint();
TEXTSTRING* T_Print(int32_t xpos, int32_t ypos, int32_t zpos, const char* string);
void T_MovePrint(TEXTSTRING* textString, int32_t xpos, int32_t ypos, int32_t zpos);
void T_ChangeText(TEXTSTRING* textString, const char* string);
void T_SetFlags(TEXTSTRING* textString, uint16_t flags);
void T_SetBgndFlags(TEXTSTRING* textString, uint16_t flags);
void T_SetOutlFlags(TEXTSTRING* textString, uint16_t flags);
void T_SetScale(TEXTSTRING* textString, int32_t scaleH, int32_t scaleV);
void T_LetterSpacing(TEXTSTRING* textString, int16_t pixels);
void T_WordSpacing(TEXTSTRING* textString, int16_t pixels);
void T_FlashText(TEXTSTRING* textString, int16_t ok, int16_t rate);
int32_t T_GetTextWidth(TEXTSTRING* textString);

void T_AddBackground(TEXTSTRING* textString, int16_t xsize, int16_t ysize, int16_t xoff, int16_t yoff, int16_t zoff, int16_t colour, SG_COL* gourptr, uint16_t flags);
void T_RemoveBackground(TEXTSTRING* textString);
void T_AddOutline(TEXTSTRING* textString, int16_t ok, int16_t colour, SG_COL* gourptr, uint16_t flags);
void T_RemoveOutline(TEXTSTRING* textString);

void T_CentreH(TEXTSTRING* textString, int16_t ok);
void T_CentreV(TEXTSTRING* textString, int16_t ok);
void T_RightAlign(TEXTSTRING* textString, int16_t ok);
void T_BottomAlign(TEXTSTRING* textString, int16_t ok);

int T_RemovePrint(TEXTSTRING* tString);
void T_RemoveAllPrints(void);

void T_DrawText(void);
void T_DrawThisText(TEXTSTRING* textString);
int16_t T_GetStringLen(const char* string);

uint32_t GetTextScaleH(uint32_t scaleH);
uint32_t GetTextScaleV(uint32_t scaleV);