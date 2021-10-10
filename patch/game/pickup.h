#pragma once

void PickUpCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void SwitchCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void SwitchCollision2(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void DetonatorCollision(int16_t item_number, ITEM_INFO* laraitem, COLL_INFO* coll);
void KeyHoleCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void PuzzleHoleCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void SwitchControl(int16_t item_number);
int SwitchTrigger(int16_t item_num, int16_t timer);
int KeyTrigger(int16_t item_num);
int PickupTrigger(int16_t item_num);
void AnimatingPickUp(int16_t item_number);
void BossDropIcon(int16_t item_number);