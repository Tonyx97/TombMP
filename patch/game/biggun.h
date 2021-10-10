#pragma once

struct BIGGUNINFO
{
	int16_t RotX;
	int16_t StartRotY;
	char Flags;
	signed char FireCount;
};

void BigGunInitialise(int16_t item_number);
int BigGunControl(COLL_INFO *coll);
void BigGunCollision(int16_t item_number, ITEM_INFO *litem, COLL_INFO *coll);
void BigGunDraw(ITEM_INFO *v);