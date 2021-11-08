#pragma once

#define CF_MESH	1
#define CF_TURNINGL	2
#define CF_TURNINGR	4
#define CF_RDIR	8
#define CF_CONTROL	16
#define CF_STOPPED	32
#define CF_NOANIM	64
#define CF_DEAD	128

struct CARTINFO
{
	int32_t Speed;
	int32_t MidPos;
	int32_t FrontPos;
	int32_t TurnX;
	int32_t TurnZ;
	int16_t TurnLen;
	int16_t TurnRot;
	int16_t YVel;
	int16_t Gradient;
	char Flags;
	char StopDelay;
};

void MineCartInitialise(int16_t item_number);
void MineCartCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
int MineCartControl();

inline int minecart_turn_extra_blocks = 0;