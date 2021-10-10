#pragma once

#define NO_CAMERA		-1
#define FOLLOW_CENTRE 	1
#define NO_CHUNKY     	2
#define CHASE_OBJECT  	3
#define NO_MINY			0xffffff

enum camera_type
{
	CHASE_CAMERA,
	FIXED_CAMERA,
	LOOK_CAMERA,
	COMBAT_CAMERA,
	CINEMATIC_CAMERA,
	HEAVY_CAMERA,
	TARGET_CAMERA,
};

struct CAMERA_INFO
{
	GAME_VECTOR pos;
	GAME_VECTOR target;
	camera_type type;
	camera_type old_type;
	int32_t shift, flags, fixed_camera;
	int32_t number_frames, bounce, underwater;
	int32_t target_distance, target_square;
	int16_t target_angle, actual_angle;
	int16_t target_elevation;
	int16_t box;
	int16_t number, last, timer, speed;
	long min_ypos;
	ITEM_INFO* item, * last_item;
	OBJECT_VECTOR* fixed;
	int mike_at_lara;
	PHD_ANGLE fov, roll;
	PHD_VECTOR mike_pos;
};

inline CAMERA_INFO camera;

inline char UseForcedFixedCamera = 0;
inline GAME_VECTOR ForcedFixedCamera;

long mgLOS(GAME_VECTOR* start, GAME_VECTOR* target, long push);
long CameraCollisionBounds(GAME_VECTOR* ideal, long push, long yfirst);

void InitialiseResetCamera();
void CalculateCamera();
void MoveCamera(GAME_VECTOR* ideal, int speed);
void ChaseCamera(ITEM_INFO* item);
void CombatCamera(ITEM_INFO* item);
void LookCamera(ITEM_INFO* item);
void FixedCamera();