#include <vcruntime_string.h>

#include "objects.h"
#include "anim.h"
#include "camera.h"
#include "health.h"
#include "text.h"
#include "invdata.h"

#include <specific/frontend.h>

#include <stdio.h>

#define	SECRETS_WIDTH	16
#define SECRET_CODE1	127
#define SECRET_CODE2	128
#define SECRET_CODE3	129

TEXTSTRING T_textStrings[MAX_TEXT_STRINGS];
int16_t T_numStrings;

char T_theStrings[MAX_TEXT_STRINGS * MAX_STRING_SIZE];

char T_textSpacing[] =
{
	14/*A*/, 13/*B*/, 14/*C*/, 14/*D*/, 13/*E*/,
	12/*F*/, 15/*G*/, 15/*H*/,  9/*I*/, 12/*J*/,
	15/*K*/, 14/*L*/, 17/*M*/, 14/*N*/, 15/*O*/,
	13/*P*/, 15/*Q*/, 14/*R*/, 11/*S*/, 13/*T*/,
	13/*U*/, 15/*V*/, 17/*W*/, 14/*X*/, 14/*Y*/, 13/*Z*/,

	10/*a*/, 11/*b*/,  9/*c*/, 10/*d*/,  9/*e*/,
	 9/*f*/, 10/*g*/, 11/*h*/,  6/*i*/,  7/*j*/,
	12/*k*/,  7/*l*/, 16/*m*/, 11/*n*/, 10/*o*/,
	11/*p*/, 11/*q*/,  9/*r*/,  8/*s*/,  8/*t*/,
	11/*u*/, 10/*v*/, 15/*w*/, 10/*x*/, 10/*y*/, 10/*z*/,

   12/*0*/,   7/*1*/, 10/*2*/, 10/*3*/, 10/*4*/,
   10/*5*/,  10/*6*/,  9/*7*/, 10/*8*/, 10/*9*/,

	5/*.*/,  5/*,*/, 5/*!*/, 11/*?*/,  9/*"*/,
   10/*"*/,  8/**/,  6/*(*/,  6/*)*/,  7/*-*/,
	7/*=*/,  3/*:*/,11/*%*/,  8/*+*/, 13/*(c)*/,
   16/*tm*/, 9/*&*/, 4/*'*/,
   12, 12,
   7, 5, 7, 7, 7, 7, 7, 7, 7, 7,
   16, 14, 14, 14, 14, 14, 14, 14, 14,
   12, 14,
   8, 8, 8, 8, 8, 8, 8
};

char T_remapASCII[] =
{
	  0/* */,
	 64/*!*/, 66/*"*/, 78/*#*/, 77/*$*/, 74/*%*/, 78/*&*/, 79/*'*/, 69/*(*/,
	 70/*)*/, 92/***/, 72/*+*/, 63/*,*/, 71/*-*/, 62/*.*/, 68/**/,  52/*0*/,
	 53/*1*/, 54/*2*/, 55/*3*/, 56/*4*/, 57/*5*/, 58/*6*/, 59/*7*/, 60/*8*/,
	 61/*9*/, 73/*:*/, 73/*;*/, 66/*<*/, 74/*=*/, 75/*>*/, 65/*?*/,  0/**/,
	  0/*A*/,  1/*B*/,  2/*C*/,  3/*D*/,  4/*E*/,  5/*F*/,  6/*G*/, 7/*H*/,
	  8/*I*/,  9/*J*/, 10/*K*/, 11/*L*/, 12/*M*/, 13/*N*/, 14/*O*/, 15/*P*/,
	 16/*Q*/, 17/*R*/, 18/*S*/, 19/*T*/, 20/*U*/, 21/*V*/, 22/*W*/, 23/*X*/,
	 24/*Y*/, 25/*Z*/, 80/*[*/, 76/*\*/, 81/*]*/, 97/*^*/, 98/*_*/, 77/*`*/,
	 26/*a*/, 27/*b*/, 28/*c*/, 29/*d*/, 30/*e*/, 31/*f*/, 32/*g*/, 33/*h*/,
	 34/*i*/, 35/*j*/, 36/*k*/, 37/*l*/, 38/*m*/, 39/*n*/, 40/*o*/, 41/*p*/,
	 42/*q*/, 43/*r*/, 44/*s*/, 45/*t*/, 46/*u*/, 47/*v*/, 48/*w*/, 49/*x*/,
	 50/*y*/, 51/*z*/,100/*{*/,101/*|*/,102/*}*/, 67/*~*/
};

void T_InitPrint()
{
	auto textString = T_textStrings;

	DisplayModeInfo(0);

	for (int n = 0; n < MAX_TEXT_STRINGS; ++n)
	{
		textString->flags = 0;
		++textString;
	}

	T_numStrings = 0;
}

TEXTSTRING* T_Print(int32_t xpos, int32_t ypos, int32_t zpos, const char* string)
{
	auto textString = T_textStrings;

	if (!string || T_numStrings >= MAX_TEXT_STRINGS)
		return 0;

	int n = 0;

	for (; n < MAX_TEXT_STRINGS; n++)
	{
		if (!(textString->flags & T_ACTIVE))
			break;

		++textString;
	}

	if (n >= MAX_TEXT_STRINGS)
		return 0;

	auto length = T_GetStringLen(string);
	if (length >= MAX_STRING_SIZE)
		length = MAX_STRING_SIZE - 1;

	textString->string = &T_theStrings[n * MAX_STRING_SIZE];

	auto height = get_render_height();
	if (height < 480)
		height = 480;

	auto width = get_render_width();
	if (width < 640)
		width = 640;

	textString->scaleV = (height << 16) / 480;
	textString->scaleH = (width << 16) / 640;
	textString->scaleH = 0x10000;
	textString->scaleV = 0x10000;
	textString->xpos = (int16_t)((xpos * GetTextScaleH(0x10000)) >> 16);
	textString->ypos = (int16_t)((ypos * GetTextScaleV(0x10000)) >> 16);
	textString->zpos = textString->Colour = zpos;
	textString->letterSpacing = 1;
	textString->wordSpacing = 6;

	memcpy(textString->string, string, length + 1);

	textString->flags = T_ACTIVE;
	textString->textflags = 0;
	textString->outlflags = 0;
	textString->bgndflags = 0;

	textString->bgndSizeX = 0;
	textString->bgndSizeY = 0;
	textString->bgndOffX = 0;
	textString->bgndOffY = 0;
	textString->bgndOffZ = 0;

	++T_numStrings;

	return textString;
}

void T_MovePrint(TEXTSTRING* textString, int32_t xpos, int32_t ypos, int32_t zpos)
{
	textString->xpos = (int16_t)xpos;
	textString->ypos = (int16_t)ypos;
	textString->zpos = (int16_t)zpos;
}

void T_ChangeText(TEXTSTRING* textString, const char* string)
{
	if (!string || !textString)
		return;

	if (textString->flags & T_ACTIVE)
	{
		if (T_GetStringLen(string) > MAX_STRING_SIZE)
			*(char*)(string + MAX_STRING_SIZE - 1) = 0;

		memcpy(textString->string, string, MAX_STRING_SIZE);
	}
}

void T_SetFlags(TEXTSTRING* textString, uint16_t flags)
{
	if (textString)
		textString->flags = flags;
}

void T_SetBgndFlags(TEXTSTRING* textString, uint16_t flags)
{
	if (textString)
		textString->bgndflags = flags;
}

void T_SetOutlFlags(TEXTSTRING* textString, uint16_t flags)
{
	if (textString)
		textString->outlflags = flags;
}

void T_SetScale(TEXTSTRING* textString, int32_t scaleH, int32_t scaleV)
{
	if (textString)
	{
		textString->scaleH = scaleH;
		textString->scaleV = scaleV;
	}
}

void T_LetterSpacing(TEXTSTRING* textString, int16_t pixels)
{
	if (textString)
		textString->letterSpacing = pixels * 8;
}

void T_WordSpacing(TEXTSTRING* textString, int16_t pixels)
{
	if (textString)
		textString->wordSpacing = pixels * 8;
}

void T_FlashText(TEXTSTRING* textString, int16_t ok, int16_t	rate)
{
	if (!textString)
		return;

	if (ok)
	{
		textString->flags |= T_FLASH;
		textString->flashRate = rate;
		textString->flashCount = rate;
	}
	else textString->flags &= ~T_FLASH;
}

void T_AddBackground(TEXTSTRING* textString, int16_t xsize, int16_t ysize, int16_t xoff, int16_t yoff, int16_t zoff, int16_t colour, SG_COL* gourptr, uint16_t flags)
{
	uint32_t scaleH = GetTextScaleH(textString->scaleH),
		   scaleV = GetTextScaleV(textString->scaleV);

	if (textString)
	{
		textString->flags |= T_ADDBACKGROUND;
		textString->bgndSizeX = (int16_t)((xsize * scaleH) >> 16);
		textString->bgndSizeY = (int16_t)((ysize * scaleV) >> 16);
		textString->bgndOffX = (int16_t)((xoff * scaleH) >> 16);
		textString->bgndOffY = (int16_t)((yoff * scaleV) >> 16);
		textString->bgndOffZ = zoff;
		textString->bgndColour = colour;
		textString->bgndGour = gourptr;
		textString->bgndflags = flags;
	}
}

void T_RemoveBackground(TEXTSTRING* textString)
{
	if (textString)
		textString->flags &= ~T_ADDBACKGROUND;
}

void T_AddOutline(TEXTSTRING* textString, int16_t ok, int16_t colour, SG_COL* gourptr, uint16_t flags)
{
	if (textString)
	{
		textString->flags |= T_ADDOUTLINE;
		textString->outlColour = colour;
		textString->outlGour = gourptr;
		textString->outlflags = flags;
	}
}

void T_RemoveOutline(TEXTSTRING* textString)
{
	if (textString)
		textString->flags &= ~T_ADDOUTLINE;
}

void T_CentreH(TEXTSTRING* textString, int16_t ok)
{
	if (!textString)
		return;

	if (ok) textString->flags |= T_CENTRE_H;
	else	textString->flags &= ~T_CENTRE_H;
}

void T_CentreV(TEXTSTRING* textString, int16_t ok)
{
	if (!textString)
		return;

	if (ok) textString->flags |= T_CENTRE_V;
	else	textString->flags &= ~T_CENTRE_V;
}

void T_RightAlign(TEXTSTRING* textString, int16_t ok)
{
	if (!textString)
		return;

	if (ok) textString->flags |= T_RIGHTALIGN;
	else	textString->flags &= ~T_RIGHTALIGN;
}

void T_BottomAlign(TEXTSTRING* textString, int16_t ok)
{
	if (!textString)
		return;

	if (ok) textString->flags |= T_BOTTOMALIGN;
	else	textString->flags &= ~T_BOTTOMALIGN;
}

int32_t T_GetTextWidth(TEXTSTRING* textString)
{
	uint32_t letter = 0,
		   scaleV = GetTextScaleH(textString->scaleH),
		   scaleH = GetTextScaleV(textString->scaleV);

	int width = 0;

	auto string = (uint8_t*)textString->string;

	while (*string)
	{
		letter = *(string++);

		if (letter == 17 || letter == 18)
		{
			width += 14;
			continue;
		}

		if ((letter > 129) || (letter > 10 && letter < 32) || (letter == '(' || letter == ')' || letter == '$' || letter == '~'))
			continue;

		if (letter == 32)
		{
			width += (scaleH == 0x10000 ? textString->wordSpacing : ((textString->wordSpacing * scaleH) >> 16));
			continue;
		}

		if (letter >= SECRET_CODE1 && letter <= SECRET_CODE3)
		{
			width += (scaleH == 0x10000 ? SECRETS_WIDTH : ((SECRETS_WIDTH * scaleH) >> 16));
			continue;
		}

		if (letter < 11)
			letter += 81;
		else if (letter < 16)
			letter += 91;
		else letter = T_remapASCII[letter - 32];

		if (letter >= '0' && letter <= '9')
			width += ((12 * scaleH) >> 16);
		else width += (scaleH == 0x10000 ? (T_textSpacing[letter] + textString->letterSpacing)
										 : ((((int32_t)T_textSpacing[letter] + textString->letterSpacing) * scaleH) >> 16));
	}

	width -= textString->letterSpacing;
	width &= 0xfffe;

	return width;
}

int	T_RemovePrint(TEXTSTRING* textString)
{
	if (!textString || !(textString->flags & T_ACTIVE))
		return 0;

	textString->flags &= ~T_ACTIVE;

	--T_numStrings;

	return 1;
}

void T_RemoveAllPrints()
{
	T_InitPrint();
}

int16_t T_GetStringLen(const char* string)
{
	int16_t len = 1;

	while (*string++)
		if (++len > MAX_STRING_SIZE)
			return MAX_STRING_SIZE;

	return len;
}

void T_DrawText()
{
	auto textString = T_textStrings;

	for (int i = 0; i < MAX_TEXT_STRINGS; ++i, ++textString)
		if (textString && (textString->flags & T_ACTIVE))
		{
			T_DrawThisText(textString);
		}
}

void draw_border(int32_t xpos, int32_t ypos, int32_t zpos, int32_t width, int32_t height)
{
#ifdef GAMEDEBUG
	if (!objects[BORDERS].loaded)
		return;
#endif

	width -= 8;
	height -= 8;
	xpos += 4;
	ypos += 4;

	S_DrawScreenSprite2d(xpos, ypos, zpos, 0x10000, 0x10000, (int16_t)(objects[BORDERS].mesh_index + 0), 7, -1);
	S_DrawScreenSprite2d(xpos + width, ypos, zpos, 0x10000, 0x10000, (int16_t)(objects[BORDERS].mesh_index + 1), 7, -1);
	S_DrawScreenSprite2d(xpos + width, ypos + height, zpos, 0x10000, 0x10000, (int16_t)(objects[BORDERS].mesh_index + 2), 7, -1);
	S_DrawScreenSprite2d(xpos, ypos + height, zpos, 0x10000, 0x10000, (int16_t)(objects[BORDERS].mesh_index + 3), 7, -1);
	S_DrawScreenSprite2d(xpos, ypos, zpos, width * 0x2000, 0x10000, (int16_t)(objects[BORDERS].mesh_index + 4), 7, -1);
	S_DrawScreenSprite2d(xpos + width, ypos, zpos, 0x10000, height * 0x2000, (int16_t)(objects[BORDERS].mesh_index + 5), 7, -1);
	S_DrawScreenSprite2d(xpos, ypos + height, zpos, width * 0x2000, 0x10000, (int16_t)(objects[BORDERS].mesh_index + 6), 7, -1);
	S_DrawScreenSprite2d(xpos, ypos, zpos, 0x10000, height * 0x2000, (int16_t)(objects[BORDERS].mesh_index + 7), 7, -1);
}

void T_RightJustify(TEXTSTRING* textString)
{
	textString->flags |= T_RIGHTJUSTIFY;
}

void T_DrawThisText(TEXTSTRING* textString)
{
	if (textString->flags & T_FLASH)
	{
		textString->flashCount -= (int16_t)camera.number_frames;

		if (textString->flashCount <= -textString->flashRate)
			textString->flashCount = textString->flashRate;
		else if (textString->flashCount < 0)
			return;
	}

	auto string = (uint8_t*)textString->string;
	
	uint32_t scaleH = GetTextScaleH(textString->scaleH),
		   scaleV = GetTextScaleV(textString->scaleV);

	int xpos = textString->xpos + SCR_LEFT,
		ypos = textString->ypos + SCR_TOP,
		zpos = textString->zpos,
		pixWidth = T_GetTextWidth(textString),
		textheight = (TEXT_HEIGHT * scaleV) >> 16;

	if (textString->flags & T_CENTRE_H)
		xpos += (((SCR_RIGHT - SCR_LEFT) - pixWidth) / 2);
	else if (textString->flags & T_RIGHTALIGN)
		xpos += ((SCR_RIGHT - SCR_LEFT) - pixWidth);
	else if (textString->flags & T_RIGHTJUSTIFY)
		xpos -= pixWidth;

	if (textString->flags & T_CENTRE_V)
		ypos += ((SCR_BOTTOM - SCR_TOP) / 2);
	else if (textString->flags & T_BOTTOMALIGN)
		ypos += (SCR_BOTTOM - SCR_TOP);

	int bxpos = xpos - ((2 * scaleH) >> 16) + textString->bgndOffX,
		bypos = ypos - ((4 * scaleV) >> 16) - textheight + textString->bgndOffY;

	while (*string)
	{
		auto letter = *(string++);

		if (letter > 18 && letter < 32)
			continue;

		if (letter == 32)
		{
			xpos += ((textString->wordSpacing * scaleH) >> 16);
			continue;
		}

		if (letter >= SECRET_CODE1 && letter <= SECRET_CODE3)
		{
			xpos += ((SECRETS_WIDTH * scaleH) >> 16);
			continue;
		}

		if (letter >= SECRET_CODE1)
			continue;

		int sprite;

		if (letter < 11)
			sprite = letter + 81;
		else if (letter <= 18)
			sprite = letter + 91;
		else sprite = T_remapASCII[letter - 32];

		if (letter >= '0' && letter <= '9')
			xpos += ((((12 - T_textSpacing[sprite]) / 2) * scaleH) >> 16);

		if (xpos > 0 && xpos < SCR_RIGHT && ypos > 0 && ypos < SCR_BOTTOM)
			S_DrawScreenSprite2d(xpos, ypos, zpos, scaleH, scaleV, (int16_t)(objects[ALPHABET].mesh_index + sprite), textString->Colour, textString->textflags);

		if (letter == '(' || letter == ')' || letter == '$' || letter == '~')
			continue;

		if (letter >= '0' && letter <= '9')
			xpos += (((12 - (12 - T_textSpacing[sprite]) / 2) * scaleH) >> 16);
		else
		{
			if (scaleH == 0x10000)
				xpos += (sprite == 108 || sprite == 109 ? 14 : (T_textSpacing[sprite] + textString->letterSpacing));
			else xpos += ((((int32_t)T_textSpacing[sprite] + textString->letterSpacing) * scaleH) >> 16);
		}
	}

	int32_t bwidth = 0,
		   bheight = 0;

	if (textString->flags & T_ADDBACKGROUND || textString->flags & T_ADDOUTLINE)
	{
		if (textString->bgndSizeX)
		{
			bxpos += (pixWidth / 2);
			pixWidth = textString->bgndSizeX;
			bxpos -= (pixWidth / 2);
		}

		bwidth = pixWidth + 4;
		bheight = (textString->bgndSizeY ? textString->bgndSizeY : ((TEXT_HEIGHT + 5) * scaleV) >> 16);
	}

	if (textString->flags & T_ADDBACKGROUND)
	{
		if (textString->bgndGour)
		{
			int hw = bwidth / 2,
				hh = bheight / 2,
				hw2 = bwidth - hw,
				hh2 = bheight - hh;

			S_DrawScreenFBox(bxpos, bypos, zpos + textString->bgndOffZ + 2, hw + hw2, hh + hh2, textString->bgndColour, textString->bgndGour, textString->bgndflags);
		}
		else S_DrawScreenFBox(bxpos, bypos, zpos + textString->bgndOffZ + 2, bwidth, bheight, textString->bgndColour, textString->bgndGour, textString->bgndflags);
	}

	if (textString->flags & T_ADDOUTLINE)
		draw_border(bxpos, bypos, 0, bwidth, bheight);
}

uint32_t GetTextScaleH(uint32_t scaleH)
{
	int width = get_render_width();
	if (width < 640)
		width = 640;

	uint32_t scale = (width << 16) / width;

	return ((scaleH >> 8) * (scale >> 8));
}

uint32_t GetTextScaleV(uint32_t scaleV)
{
	int height = get_render_height();
	if (height < 480)
		height = 480;

	uint32_t scale = (height << 16) / height;

	return ((scaleV >> 8) * (scale >> 8));
}