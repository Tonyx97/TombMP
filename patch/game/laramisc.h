#pragma once

void (*lara_control_routines[])(ITEM_INFO* item, COLL_INFO* coll);
void (*extra_control_routines[])(ITEM_INFO* item, COLL_INFO* coll);
void (*lara_collision_routines[])(ITEM_INFO* item, COLL_INFO* coll);

void LaraControl();
void InitialiseLara();
void LaraInitialiseMeshes();
void ResetLaraInfo();
void InitialiseLaraLoad(int16_t item_num);
void InitialiseLaraInventory();
void UseItem(int16_t object_num);
void ControlLaraExtra(int16_t item_num);
void LaraDinoDeath(ITEM_INFO* item);
void AnimateLara(ITEM_INFO* item);
void LaraCheatGetStuff();