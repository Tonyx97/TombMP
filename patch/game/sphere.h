#pragma once

struct SPHERE
{
	int32_t x, y, z, r;
};

bool TestCollision(ITEM_INFO* item, ITEM_INFO* laraitem);
int GetSpheres(ITEM_INFO* item, SPHERE* ptr, int WorldSpace);
void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* vec, int joint);