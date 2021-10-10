#pragma once

struct KAYAKINFO
{
	int32_t Vel;
	int32_t Rot;
	int32_t FallSpeedF;
	int32_t FallSpeedL;
	int32_t FallSpeedR;
	int32_t Water;
	PHD_3DPOS OldPos;
	char Turn;
	char Forward;
	char TrueWater;
	char Flags;
};

void KayakCollision(int16_t item_number, ITEM_INFO* litem, COLL_INFO* coll);
int KayakControl();
void KayakDraw(ITEM_INFO*);
void KayakInitialise(int16_t);
void LaraRapidsDrown();