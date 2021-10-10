#pragma once

struct BOAT_INFO
{
	int boat_turn;
	int left_fallspeed, right_fallspeed;
	int16_t tilt_angle, extra_rotation;
	int water, pitch;
	int16_t prop_rot;
};

void InitialiseBoat(int16_t item_number);
void BoatCollision(int16_t item_number, ITEM_INFO *litem, COLL_INFO *coll);
void BoatControl(int16_t item_number);
void DrawBoat(ITEM_INFO *item);