#pragma once

enum block_states
{
	STILL = 1,
	PUSH,
	PULL
};

void InitialiseMovingBlock(int16_t item_num);
void ClearMovableBlockSplitters(int32_t x, int32_t y, int32_t z, int16_t room_number);
void MovableBlock(int16_t item_number);
void MovableBlockCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
int TestBlockPush(ITEM_INFO* item, int blokhite, uint16_t quadrant);
int TestBlockPull(ITEM_INFO* item, int blokhite, uint16_t quadrant);
void AlterFloorHeight(ITEM_INFO* item, int height);
void AlterFloorHeight(int x, int y, int z, int16_t room_id, int height);
void DrawMovableBlock(ITEM_INFO* item);

void DrawUnclippedItem(ITEM_INFO* item);