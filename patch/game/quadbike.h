#pragma once

#define QF_DEAD	0x80
#define QF_FALLING	0x40

struct QUADINFO
{
	int32_t Velocity;
	int16_t FrontRot;	// front wheel rotation
	int16_t RearRot;		// rear wheel rotation
	int32_t Revs;      	// For handbrake starts.
	int32_t EngineRevs;
	int16_t track_mesh;
	int skidoo_turn;
	int left_fallspeed,
		right_fallspeed;
	int16_t momentum_angle,
		extra_rotation;
	int pitch;
	char Flags;
};

void InitialiseQuadBike(int16_t item_number);
int QuadBikeControl();
void QuadBikeCollision(int16_t item_number, ITEM_INFO* litem, COLL_INFO* coll);
void QuadBikeDraw(ITEM_INFO* item);