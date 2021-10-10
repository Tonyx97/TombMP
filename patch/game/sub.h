#pragma once

struct SUBINFO
{
	int32_t Vel;
	int32_t Rot;
	int32_t RotX;
	int16_t FanRot;
	char Flags;
	char WeaponTimer;
};

void SubInitialise(int16_t item_number);
int SubControl();
void SubCollision(int16_t item_number, ITEM_INFO* litem, COLL_INFO* coll);
void SubDraw(ITEM_INFO* item);
void SubEffects(int16_t item_number);