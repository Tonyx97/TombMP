#pragma once

#include "items.h"
#include "collide.h"

int LaraTestClimbPos(ITEM_INFO* item, int front, int right, int origin, int height, int* shift);
int16_t GetClimbTrigger(int32_t x, int32_t y, int32_t z, int16_t room_number);