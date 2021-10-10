import prof;

#include <specific/standard.h>
#include <specific/global.h>

#include "lara.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "effect2.h"

#define COM_SQUARE			SQUARE(WALL_L / 2)
#define MIN_SQUARE			SQUARE(WALL_L / 3)
#define MIN_CAM_ANGLE		(ONE_DEGREE / 2)
#define GROUND_SHIFT		(STEP_L)
#define MAX_ELEVATION		(ONE_DEGREE * 85)
#define NORMAL_ELEVATION 	-(ONE_DEGREE * 10)
#define COMBAT_ELEVATION 	-(ONE_DEGREE * 15)
#define MAX_CAMERA_PUSH		200
#define MIN_CAMERA_SNAP		768
#define MAX_HEIGHT_CHANGE	1536
#define MAX_LOOKX_ROTATION	-(75 * ONE_DEGREE)
#define MIN_LOOKX_ROTATION	(55 * ONE_DEGREE)
#define MAX_LOOKY_ROTATION	(80 * ONE_DEGREE)
#define MAX_HEAD_ROTATION 	(50 * ONE_DEGREE)
#define MAX_HEAD_TILT	  	(85 * ONE_DEGREE)
#define MIN_HEAD_TILT 	  	(-85 * ONE_DEGREE)
#define HEAD_TURN 		  	(4 * ONE_DEGREE)
#define CHASE_SPEED			10
#define COMBAT_SPEED		8
#define COMBAT_DISTANCE		(WALL_L * 3 / 2)
#define LOOK_SPEED			4
#define	MAX_CAMERA_MOVE		384
#define	MAX_LOOKCAMERA_MOVE	224

struct OLD_CAMERA
{
	int16_t current_anim_state;
	int16_t goal_anim_state;
	PHD_3DPOS pos;
	PHD_3DPOS pos2;
};

int32_t oldtx, oldty, oldtz;
int32_t TooFarCount = 0;
int32_t CombatShift = 0;

uint8_t camerasnaps = 0;

GAME_VECTOR static_lookcamp;
GAME_VECTOR static_lookcamt;
OLD_CAMERA old_cam;
GAME_VECTOR last_ideal;
GAME_VECTOR last_target;

void InitialiseResetCamera()
{
	camera.target.x = last_target.x = lara_item->pos.x_pos;
	camera.target.y = last_target.y = camera.shift = lara_item->pos.y_pos - WALL_L;
	camera.target.z = last_target.z = lara_item->pos.z_pos;
	camera.target.room_number = last_target.room_number = lara_item->room_number;
	camera.pos.x = camera.target.x;
	camera.pos.y = camera.target.y;
	camera.pos.z = camera.target.z - 100;
	camera.pos.room_number = camera.target.room_number;
	camera.target_distance = WALL_L * 3 / 2;
	camera.item = nullptr;
	camera.number_frames = 1;

	if (!lara.extra_anim)
		camera.type = CHASE_CAMERA;

	camera.speed = 1;
	camera.flags = 0;
	camera.bounce = 0;
	camera.number = NO_CAMERA;
	camera.fixed_camera = 0;
	camera.roll = 0;

	UseForcedFixedCamera = 0;

	AlterFOV(g_window->get_fov());
	CalculateCamera();
}

void MoveCamera(GAME_VECTOR* ideal, int speed)
{
	if (old_cam.pos.x_rot != lara_item->pos.x_rot ||
		old_cam.pos.y_rot != lara_item->pos.y_rot ||
		old_cam.pos.z_rot != lara_item->pos.z_rot ||
		old_cam.pos2.x_rot != lara.head_x_rot ||
		old_cam.pos2.y_rot != lara.head_y_rot ||
		old_cam.pos2.x_pos != lara.torso_x_rot ||
		old_cam.pos2.y_pos != lara.torso_y_rot ||
		old_cam.pos.x_pos != lara_item->pos.x_pos ||
		old_cam.pos.y_pos != lara_item->pos.y_pos ||
		old_cam.pos.z_pos != lara_item->pos.z_pos ||
		old_cam.current_anim_state != lara_item->current_anim_state ||
		old_cam.goal_anim_state != lara_item->goal_anim_state ||
		camera.old_type != camera.type)
	{
		old_cam.pos.x_rot = lara_item->pos.x_rot;
		old_cam.pos.y_rot = lara_item->pos.y_rot;
		old_cam.pos.z_rot = lara_item->pos.z_rot;
		old_cam.pos2.x_rot = lara.head_x_rot;
		old_cam.pos2.y_rot = lara.head_y_rot;
		old_cam.pos2.x_pos = lara.torso_x_rot;
		old_cam.pos2.y_pos = lara.torso_y_rot;
		old_cam.pos.x_pos = lara_item->pos.x_pos;
		old_cam.pos.y_pos = lara_item->pos.y_pos;
		old_cam.pos.z_pos = lara_item->pos.z_pos;
		old_cam.current_anim_state = lara_item->current_anim_state;
		old_cam.goal_anim_state = lara_item->goal_anim_state;

		last_ideal.x = ideal->x;
		last_ideal.y = ideal->y;
		last_ideal.z = ideal->z;
		last_ideal.room_number = ideal->room_number;
	}
	else
	{
		ideal->x = last_ideal.x;
		ideal->y = last_ideal.y;
		ideal->z = last_ideal.z;
		ideal->room_number = last_ideal.room_number;
	}

	camera.pos.x += (ideal->x - camera.pos.x) / speed;
	camera.pos.y += (ideal->y - camera.pos.y) / speed;
	camera.pos.z += (ideal->z - camera.pos.z) / speed;
	camera.pos.room_number = ideal->room_number;

	if (camera.bounce)
	{
		if (camera.bounce > 0)
		{
			camera.pos.y += camera.bounce;
			camera.target.y += camera.bounce;
			camera.bounce = 0;
		}
		else
		{
			int shake = (GetRandomControl() - 0x4000) * camera.bounce / 0x7fff;

			camera.pos.x += shake;
			camera.target.y += shake;

			shake = (GetRandomControl() - 0x4000) * camera.bounce / 0x7fff;

			camera.pos.y += shake;
			camera.target.y += shake;

			shake = (GetRandomControl() - 0x4000) * camera.bounce / 0x7fff;

			camera.pos.z += shake;
			camera.target.z += shake;
			camera.bounce += 5;
		}
	}

	{
		int wx = camera.pos.x,
			wy = camera.pos.y,
			wz = camera.pos.z;

		auto room_number = camera.pos.room_number;
		auto floor = GetFloor(wx, wy + 256, wz, &room_number);

		if (room[room_number].flags & SWAMP)
		{
			wy = room[room_number].y - 256;
			floor = GetFloor(wx, wy, wz, &camera.pos.room_number);
		}

		room_number = camera.pos.room_number;
		floor = GetFloor(wx, wy, wz, &room_number);

		int height = GetHeight(floor, wx, wy, wz),
			ceiling = GetCeiling(floor, wx, wy, wz);

		if (wy < ceiling || wy > height)
		{
			mgLOS(&camera.target, &camera.pos, 0);

			int dx = abs(camera.pos.x - ideal->x),
				dy = abs(camera.pos.y - ideal->y),
				dz = abs(camera.pos.z - ideal->z);

			if (dx < MIN_CAMERA_SNAP && dy < MIN_CAMERA_SNAP && dz < MIN_CAMERA_SNAP)
			{
				GAME_VECTOR	temp1 { camera.pos.x, camera.pos.y, camera.pos.z, camera.pos.room_number },
							temp2 { ideal->x, ideal->y, ideal->z, ideal->room_number };

				if (!mgLOS(&temp2, &temp1, 0))
				{
					if (++camerasnaps >= 8)
					{
						camera.pos = { ideal->x, ideal->y, ideal->z, ideal->room_number };

						camerasnaps = 0;
					}
				}
			}
		}
	}

	{
		int wx = camera.pos.x,
			wy = camera.pos.y,
			wz = camera.pos.z;

		auto room_number = camera.pos.room_number;
		auto floor = GetFloor(wx, wy, wz, &room_number);

		int height = GetHeight(floor, wx, wy, wz),
			ceiling = GetCeiling(floor, wx, wy, wz);

		if (wy - 255 < ceiling && wy + 255 > height && ceiling < height && ceiling != NO_HEIGHT && height != NO_HEIGHT)
			camera.pos.y = (height + ceiling) >> 1;
		else if (wy + 255 > height && ceiling < height && ceiling != NO_HEIGHT && height != NO_HEIGHT)
			camera.pos.y = height - 255;
		else if (wy - 255 < ceiling && ceiling < height && ceiling != NO_HEIGHT && height != NO_HEIGHT)
			camera.pos.y = ceiling + 255;
		else if (ceiling >= height || height == NO_HEIGHT || ceiling == NO_HEIGHT)
			camera.pos = { ideal->x, ideal->y, ideal->z, ideal->room_number };
	}

	GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);

	phd_LookAt({ camera.pos.x, camera.pos.y, camera.pos.z }, { camera.target.x, camera.target.y, camera.target.z }, camera.roll);

	if (camera.mike_at_lara)
	{
		camera.actual_angle = lara_item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
		camera.mike_pos = { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos };
	}
	else
	{
		camera.actual_angle = phd_atan(camera.target.z - camera.pos.z, camera.target.x - camera.pos.x);
		camera.mike_pos =
		{
			camera.pos.x + ((phd_persp * phd_sin(camera.actual_angle)) >> W2V_SHIFT),
			camera.pos.y,
			camera.pos.z + ((phd_persp * phd_cos(camera.actual_angle)) >> W2V_SHIFT),
		};
	}

	camera.old_type = camera.type;
}

void ChaseCamera(ITEM_INFO* item)
{
	if (!camera.target_elevation)
		camera.target_elevation = NORMAL_ELEVATION;

	camera.target_elevation += item->pos.x_rot;

	if (camera.target_elevation > MAX_ELEVATION)
		camera.target_elevation = MAX_ELEVATION;
	else if (camera.target_elevation < -MAX_ELEVATION)
		camera.target_elevation = -MAX_ELEVATION;

	int distance = camera.target_distance * phd_cos(camera.target_elevation) >> W2V_SHIFT;

	auto room_number = camera.target.room_number;
	auto floor = GetFloor(camera.target.x, camera.target.y + 256, camera.target.z, &room_number);

	if (room[room_number].flags & SWAMP)
		camera.target.y = room[room_number].maxceiling - 256;

	floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);

	int h = GetHeight(floor, camera.target.x, camera.target.y, camera.target.z),
		c = GetCeiling(floor, camera.target.x, camera.target.y, camera.target.z);

	if (c + 16 > h - 16 && h != NO_HEIGHT && c != NO_HEIGHT)
	{
		camera.target.y = (c + h) >> 1;
		camera.target_elevation = 0;
	}
	else if (camera.target.y > h - 16 && h != NO_HEIGHT)
	{
		camera.target.y = h - 16;
		camera.target_elevation = 0;
	}
	else if (camera.target.y < c + 16 && c != NO_HEIGHT)
	{
		camera.target.y = c + 16;
		camera.target_elevation = 0;
	}

	floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);

	int wx = camera.target.x,
		wy = camera.target.y,
		wz = camera.target.z;

	room_number = camera.target.room_number;
	floor = GetFloor(wx, wy, wz, &room_number);
	h = GetHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);

	if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT)
		camera.target = { last_target.x, last_target.y, last_target.z, last_target.room_number };

	GAME_VECTOR	ideals[5],
				temp[2];

	for (int i = 0; i < 5; ++i)
		ideals[i].y = camera.target.y + (camera.target_distance * phd_sin(camera.target_elevation) >> W2V_SHIFT);

	int farthest = 0x7fffffff,
		farthestnum = 0;

	for (int i = 0; i < 5; ++i)
	{
		int angle = (i == 0 ? item->pos.y_rot + camera.target_angle : (i - 1) * 0x4000);

		ideals[i].x = camera.target.x - ((distance * phd_sin(angle)) >> W2V_SHIFT);
		ideals[i].z = camera.target.z - ((distance * phd_cos(angle)) >> W2V_SHIFT);
		ideals[i].room_number = camera.target.room_number;

		if (mgLOS(&camera.target, &ideals[i], MAX_CAMERA_PUSH))
		{
			temp[0] = { ideals[i].x, ideals[i].y, ideals[i].z, ideals[i].room_number };
			temp[1] = { camera.pos.x, camera.pos.y, camera.pos.z, camera.pos.room_number };

			if (i == 0 || mgLOS(&temp[0], &temp[1], 0))
			{
				if (i == 0)
				{
					farthestnum = 0;
					break;
				}
				
				if (int dx = SQUARE(camera.pos.x - ideals[i].x) + SQUARE(camera.pos.z - ideals[i].z); dx < farthest)
				{
					farthest = dx;
					farthestnum = i;
				}
			}
		}
		else if (i == 0)
		{
			temp[0] = { ideals[i].x, ideals[i].y, ideals[i].z, ideals[i].room_number };
			temp[1] = { camera.pos.x, camera.pos.y, camera.pos.z, camera.pos.room_number };

			if (i == 0 || mgLOS(&temp[0], &temp[1], 0))
			{
				int dx = SQUARE(camera.target.x - ideals[i].x),
					dz = SQUARE(camera.target.z - ideals[i].z);

				if ((dx + dz) > SQUARE(768))
				{
					farthestnum = 0;
					break;
				}
			}
		}
	}

	GAME_VECTOR ideal { ideals[farthestnum].x, ideals[farthestnum].y, ideals[farthestnum].z, ideals[farthestnum].room_number };

	CameraCollisionBounds(&ideal, 384, 1);

	if (camera.old_type == FIXED_CAMERA)
		camera.speed = 1;

	MoveCamera(&ideal, camera.speed);
}

void CombatCamera(ITEM_INFO* item)
{
	camera.target.z = item->pos.z_pos;
	camera.target.x = item->pos.x_pos;

	if (lara.target)
	{
		camera.target_angle = item->pos.y_rot + lara.target_angles[0];
		camera.target_elevation = item->pos.x_rot + lara.target_angles[1];
	}
	else
	{
		camera.target_angle = item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
		camera.target_elevation = item->pos.x_rot + lara.head_x_rot + lara.torso_x_rot + COMBAT_ELEVATION;
	}

	auto room_number = camera.target.room_number;
	auto floor = GetFloor(camera.target.x, camera.target.y + 256, camera.target.z, &room_number);

	if (room[room_number].flags & SWAMP)
		camera.target.y = room[room_number].y - 256;

	floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);

	int h = GetHeight(floor, camera.target.x, camera.target.y, camera.target.z),
		c = GetCeiling(floor, camera.target.x, camera.target.y, camera.target.z);

	if (c + 64 > h - 64 && h != NO_HEIGHT && c != NO_HEIGHT)
	{
		camera.target.y = (c + h) >> 1;
		camera.target_elevation = 0;
	}
	else if (camera.target.y > h - 64 && h != NO_HEIGHT)
	{
		camera.target.y = h - 64;
		camera.target_elevation = 0;
	}
	else if (camera.target.y < c + 64 && c != NO_HEIGHT)
	{
		camera.target.y = c + 64;
		camera.target_elevation = 0;
	}

	floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);

	int wx = camera.target.x,
		wy = camera.target.y,
		wz = camera.target.z;

	room_number = camera.target.room_number;
	floor = GetFloor(wx, wy, wz, &room_number);
	h = GetHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);

	if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT)
		camera.target = { last_target.x, last_target.y, last_target.z, last_target.room_number };

	camera.target_distance = COMBAT_DISTANCE;

	int distance = camera.target_distance * phd_cos(camera.target_elevation) >> W2V_SHIFT;

	GAME_VECTOR	ideals[9],
				temp[2];

	for (int i = 0; i < 5; ++i)
		ideals[i].y = camera.target.y + (camera.target_distance * phd_sin(camera.target_elevation) >> W2V_SHIFT);

	int farthest = 0x7fffffff,
		farthestnum = 0;

	for (int i = 0; i < 5; ++i)
	{
		int angle = (i == 0 ? camera.target_angle : (i - 1) * 0x4000);

		ideals[i].x = camera.target.x - ((distance * phd_sin(angle)) >> W2V_SHIFT);
		ideals[i].z = camera.target.z - ((distance * phd_cos(angle)) >> W2V_SHIFT);
		ideals[i].room_number = camera.target.room_number;

		if (mgLOS(&camera.target, &ideals[i], MAX_CAMERA_PUSH))
		{
			temp[0] = { ideals[i].x, ideals[i].y, ideals[i].z, ideals[i].room_number };
			temp[1] = { camera.pos.x, camera.pos.y, camera.pos.z, camera.pos.room_number };

			if (i == 0 || mgLOS(&temp[0], &temp[1], 0))
			{
				if (i == 0)
				{
					farthestnum = 0;
					break;
				}

				int dx = SQUARE(camera.pos.x - ideals[i].x) + SQUARE(camera.pos.z - ideals[i].z);
				if (dx < farthest)
				{
					farthest = dx;
					farthestnum = i;
				}
			}
		}
		else if (i == 0)
		{
			temp[0] = { ideals[i].x, ideals[i].y, ideals[i].z, ideals[i].room_number };
			temp[1] = { camera.pos.x, camera.pos.y, camera.pos.z, camera.pos.room_number };

			if (i == 0 || mgLOS(&temp[0], &temp[1], 0))
			{
				int dx = SQUARE(camera.target.x - ideals[i].x),
					dz = SQUARE(camera.target.z - ideals[i].z);

				if ((dx + dz) > SQUARE(768))
				{
					farthestnum = 0;
					break;
				}
			}
		}
	}

	GAME_VECTOR ideal { ideals[farthestnum].x, ideals[farthestnum].y, ideals[farthestnum].z, ideals[farthestnum].room_number };

	CameraCollisionBounds(&ideal, 384, 1);

	if (camera.old_type == FIXED_CAMERA)
		camera.speed = 1;

	MoveCamera(&ideal, camera.speed);
}

void LookCamera(ITEM_INFO* item)
{
	int hxrot = lara.head_x_rot,
		hyrot = lara.head_y_rot,
		txrot = lara.torso_x_rot,
		tyrot = lara.torso_y_rot;

	bool clipped = false;

	lara.torso_x_rot = 0;
	lara.torso_y_rot = 0;
	lara.head_x_rot <<= 1;
	lara.head_y_rot <<= 1;

	if (lara.head_x_rot > MIN_LOOKX_ROTATION)	   lara.head_x_rot = MIN_LOOKX_ROTATION;
	else if (lara.head_x_rot < MAX_LOOKX_ROTATION) lara.head_x_rot = MAX_LOOKX_ROTATION;

	if (lara.head_y_rot < -MAX_LOOKY_ROTATION)	   lara.head_y_rot = -MAX_LOOKY_ROTATION;
	else if (lara.head_y_rot > MAX_LOOKY_ROTATION) lara.head_y_rot = MAX_LOOKY_ROTATION;

	PHD_VECTOR pos1 { 0, 16, 64 };

	get_lara_bone_pos(lara_item, &pos1, HEAD);

	auto room_number = lara_item->room_number;
	auto floor = GetFloor(pos1.x, pos1.y + 256, pos1.z, &room_number);

	if (room[room_number].flags & SWAMP)
	{
		pos1.y = room[room_number].maxceiling - 256;
		floor = GetFloor(pos1.x, pos1.y, pos1.z, &room_number);
	}
	else floor = GetFloor(pos1.x, pos1.y, pos1.z, &room_number);

	int h = GetHeight(floor, pos1.x, pos1.y, pos1.z),
		c = GetCeiling(floor, pos1.x, pos1.y, pos1.z);

	if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || pos1.y > h || pos1.y < c)
		clipped = true;

	if (clipped)
	{
		pos1.x = pos1.z = 0;
		pos1.y = 16;

		get_lara_bone_pos(lara_item, &pos1, HEAD);

		clipped = 0;
		room_number = lara_item->room_number;
		floor = GetFloor(pos1.x, pos1.y + 256, pos1.z, &room_number);

		if (room[room_number].flags & SWAMP)
		{
			pos1.y = room[room_number].y - 256;
			floor = GetFloor(pos1.x, pos1.y, pos1.z, &room_number);
		}
		else floor = GetFloor(pos1.x, pos1.y, pos1.z, &room_number);

		h = GetHeight(floor, pos1.x, pos1.y, pos1.z);
		c = GetCeiling(floor, pos1.x, pos1.y, pos1.z);

		if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || pos1.y > h || pos1.y < c)
			clipped = true;

		if (clipped)
		{
			pos1.x = 0;
			pos1.y = 16;
			pos1.z = -64;

			get_lara_bone_pos(lara_item, &pos1, HEAD);
		}
	}

	PHD_VECTOR pos2 { 0, 0, -1024 };

	get_lara_bone_pos(lara_item, &pos2, HEAD);

	PHD_VECTOR pos3 { 0, 0, 2048 };

	get_lara_bone_pos(lara_item, &pos3, HEAD);

	int dx = (pos2.x - pos1.x) >> 3,
		dy = (pos2.y - pos1.y) >> 3,
		dz = (pos2.z - pos1.z) >> 3,
		wx = pos1.x,
		wy = pos1.y,
		wz = pos1.z;

	room_number = lara_item->room_number;

	auto room_number2 = room_number;
	
	int i = 0;

	for (; i < 8; ++i, room_number = room_number2)
	{
		floor = GetFloor(wx, wy + 256, wz, &room_number2);

		if (room[room_number2].flags & SWAMP)
		{
			wy = room[room_number2].maxceiling - 256;
			break;
		}

		floor = GetFloor(wx, wy, wz, &room_number2);
		h = GetHeight(floor, wx, wy, wz);
		c = GetCeiling(floor, wx, wy, wz);

		if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy > h || wy < c)
			break;

		wx += dx;
		wy += dy;
		wz += dz;
	}

	if (i)
	{
		wx -= dx;
		wy -= dy;
		wz -= dz;
	}

	GAME_VECTOR ideal { wx, wy, wz, room_number2 };

	if (old_cam.pos.x_rot == lara.head_x_rot &&
		old_cam.pos.y_rot == lara.head_y_rot &&
		old_cam.pos.x_pos == lara_item->pos.x_pos &&
		old_cam.pos.y_pos == lara_item->pos.y_pos &&
		old_cam.pos.z_pos == lara_item->pos.z_pos &&
		old_cam.current_anim_state == lara_item->current_anim_state &&
		old_cam.goal_anim_state == lara_item->goal_anim_state &&
		camera.old_type == LOOK_CAMERA)
	{
		ideal.x = static_lookcamp.x;
		ideal.y = static_lookcamp.y;
		ideal.z = static_lookcamp.z;
		ideal.room_number = static_lookcamp.room_number;
		pos3.x = static_lookcamt.x;
		pos3.y = static_lookcamt.y;
		pos3.z = static_lookcamt.z;
	}
	else
	{
		old_cam.pos.x_rot = lara.head_x_rot;
		old_cam.pos.y_rot = lara.head_y_rot;
		old_cam.pos.x_pos = lara_item->pos.x_pos;
		old_cam.pos.y_pos = lara_item->pos.y_pos;
		old_cam.pos.z_pos = lara_item->pos.z_pos;
		old_cam.current_anim_state = lara_item->current_anim_state;
		old_cam.goal_anim_state = lara_item->goal_anim_state;
		static_lookcamp.x = ideal.x;
		static_lookcamp.y = ideal.y;
		static_lookcamp.z = ideal.z;
		static_lookcamp.room_number = ideal.room_number;
		static_lookcamt.x = pos3.x;
		static_lookcamt.y = pos3.y;
		static_lookcamt.z = pos3.z;
	}

	CameraCollisionBounds(&ideal, 224, 1);

	if (camera.old_type == FIXED_CAMERA)
	{
		camera.pos.x = ideal.x;
		camera.pos.y = ideal.y;
		camera.pos.z = ideal.z;
		camera.target.x = pos3.x;
		camera.target.y = pos3.y;
		camera.target.z = pos3.z;
		camera.target.room_number = lara_item->room_number;
	}
	else
	{
		camera.pos.x += (ideal.x - camera.pos.x) >> 2;
		camera.pos.y += (ideal.y - camera.pos.y) >> 2;
		camera.pos.z += (ideal.z - camera.pos.z) >> 2;
		camera.target.x += (pos3.x - camera.target.x) >> 2;
		camera.target.y += (pos3.y - camera.target.y) >> 2;
		camera.target.z += (pos3.z - camera.target.z) >> 2;
		camera.target.room_number = lara_item->room_number;
	}

	if (camera.bounce && camera.type == camera.old_type)
	{
		if (camera.bounce > 0)
		{
			camera.pos.y += camera.bounce;
			camera.target.y += camera.bounce;
			camera.bounce = 0;
		}
		else
		{
			int shake = camera.bounce * (GetRandomControl() - 0x4000) >> 15;

			camera.pos.x += shake;
			camera.target.y += shake;

			shake = camera.bounce * (GetRandomControl() - 0x4000) >> 15;

			camera.pos.y += shake;
			camera.target.y += shake;

			shake = camera.bounce * (GetRandomControl() - 0x4000) >> 15;

			camera.pos.z += shake;
			camera.target.z += shake;
			camera.bounce += 5;
		}
	}

	GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);

	wx = camera.pos.x;
	wy = camera.pos.y;
	wz = camera.pos.z;
	room_number = camera.pos.room_number;
	floor = GetFloor(wx, wy, wz, &room_number);
	h = GetHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);

	if (wy - 255 < c && wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)  camera.pos.y = (h + c) >> 1;
	else if (wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)				camera.pos.y = h - 255;
	else if (wy - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)				camera.pos.y = c + 255;

	wx = camera.pos.x;
	wy = camera.pos.y;
	wz = camera.pos.z;
	room_number = camera.pos.room_number;
	floor = GetFloor(wx, wy, wz, &room_number);
	h = GetHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);

	if (room[room_number].flags & SWAMP)									 camera.pos.y = room[room_number].y - 256;
	else if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT) mgLOS(&camera.target, &camera.pos, 0);

	wx = camera.pos.x;
	wy = camera.pos.y;
	wz = camera.pos.z;
	room_number = camera.pos.room_number;
	floor = GetFloor(wx, wy, wz, &room_number);
	h = GetHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);

	if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT || (room[room_number].flags & SWAMP))
		camera.pos = { pos1.x, pos1.y, pos1.z, lara_item->room_number};

	GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);
	phd_LookAt({ camera.pos.x, camera.pos.y, camera.pos.z }, { camera.target.x, camera.target.y, camera.target.z }, camera.roll);

	if (camera.mike_at_lara)
	{
		camera.actual_angle = lara_item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
		camera.mike_pos = { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos };
	}
	else
	{
		camera.actual_angle = phd_atan(camera.target.z - camera.pos.z, camera.target.x - camera.pos.x);
		camera.mike_pos =
		{
			camera.pos.x + ((phd_persp * phd_sin(camera.actual_angle)) >> W2V_SHIFT),
			camera.pos.y,
			camera.pos.z + ((phd_persp * phd_cos(camera.actual_angle)) >> W2V_SHIFT),
		};
	}

	camera.old_type = camera.type;

	lara.head_x_rot = hxrot;
	lara.head_y_rot = hyrot;
	lara.torso_x_rot = txrot;
	lara.torso_y_rot = tyrot;
}

void FixedCamera()
{
	GAME_VECTOR ideal;

	if (UseForcedFixedCamera)
		ideal = { ForcedFixedCamera.x, ForcedFixedCamera.y, ForcedFixedCamera.z, ForcedFixedCamera.room_number };
	else
	{
		auto fixed = &camera.fixed[camera.number];

		ideal = { fixed->x, fixed->y, fixed->z, fixed->data };
	}

	camera.fixed_camera = 1;

	MoveCamera(&ideal, 1);

	if (camera.timer)
		if (--camera.timer == 0)
			camera.timer = -1;
}

void TargetCamera()
{
	camera.fixed_camera = 1;

	phd_LookAt({ camera.pos.x, camera.pos.y, camera.pos.z }, { camera.target.x, camera.target.y, camera.target.z }, camera.roll);
}

void CalculateCamera()
{
	oldtx = camera.target.x;
	oldty = camera.target.y;
	oldtz = camera.target.z;

	if (UseForcedFixedCamera)
	{
		camera.type = FIXED_CAMERA;
		camera.speed = 1;
	}

	if (room[camera.pos.room_number].flags & UNDERWATER)
	{
		g_audio->play_sound(60);

		if (!camera.underwater)
			camera.underwater = 1;
	}
	else if (camera.underwater)
	{
		g_audio->stop_all();

		camera.underwater = 0;
	}

	if (camera.type == CINEMATIC_CAMERA)
		return;

	if (camera.flags != NO_CHUNKY)
		chunky_flag = 0;

	int fixed_camera = (camera.item && (camera.type == FIXED_CAMERA || camera.type == HEAVY_CAMERA));

	auto item = fixed_camera ? camera.item : lara_item;
	auto object = &objects[item->object_number];
	auto bounds = GetBoundsAccurate(item);

	int y = item->pos.y_pos + (fixed_camera ? (bounds[2] + bounds[3]) / 2 : bounds[3] + ((bounds[2] - bounds[3]) * 3 >> 2));
	
	if (camera.item && !fixed_camera)
	{
		bounds = GetBoundsAccurate(camera.item);

		int shift = phd_sqrt(SQUARE(camera.item->pos.z_pos - item->pos.z_pos) + SQUARE(camera.item->pos.x_pos - item->pos.x_pos)),
			angle = phd_atan(camera.item->pos.z_pos - item->pos.z_pos, camera.item->pos.x_pos - item->pos.x_pos) - item->pos.y_rot,
			tilt = phd_atan(shift, y - (camera.item->pos.y_pos + (bounds[2] + bounds[3]) / 2));

		angle >>= 1;
		tilt >>= 1;

		if (angle > -MAX_HEAD_ROTATION && angle < MAX_HEAD_ROTATION && tilt > MIN_HEAD_TILT && tilt < MAX_HEAD_TILT)
		{
			int change = angle - lara.head_y_rot;

			if (change > HEAD_TURN)		  lara.head_y_rot += HEAD_TURN;
			else if (change < -HEAD_TURN) lara.head_y_rot -= HEAD_TURN;
			else						  lara.head_y_rot += change;

			lara.torso_y_rot = lara.head_y_rot;

			change = tilt - lara.head_x_rot;

			if (change > HEAD_TURN)		  lara.head_x_rot += HEAD_TURN;
			else if (change < -HEAD_TURN) lara.head_x_rot -= HEAD_TURN;
			else						  lara.head_x_rot += change;

			lara.torso_x_rot = lara.head_x_rot;

			camera.type = LOOK_CAMERA;
			camera.item->looked_at = 1;
		}
	}

	if (camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA)
	{
		if (camera.type == COMBAT_CAMERA)
		{
			last_target.x = camera.target.x;
			last_target.y = camera.target.y;
			last_target.z = camera.target.z;
			last_target.room_number = camera.target.room_number;
		}

		y -= STEP_L;

		camera.target.room_number = item->room_number;

		if (camera.fixed_camera)
		{
			camera.target.y = y;
			camera.speed = 1;
		}
		else
		{
			camera.target.y += ((y - camera.target.y) >> 2);
			camera.speed = (camera.type == LOOK_CAMERA) ? LOOK_SPEED : COMBAT_SPEED;
		}

		camera.fixed_camera = 0;

		if (camera.type == LOOK_CAMERA)
			LookCamera(item);
		else CombatCamera(item);
	}
	else
	{
		last_target.x = camera.target.x;
		last_target.y = camera.target.y;
		last_target.z = camera.target.z;
		last_target.room_number = camera.target.room_number;

		camera.target.room_number = item->room_number;

		if (camera.type != TARGET_CAMERA)
		{
			camera.target.x = item->pos.x_pos;
			camera.target.z = item->pos.z_pos;

			if (camera.flags == FOLLOW_CENTRE)
			{
				int shift = (bounds[4] + bounds[5]) / 2;

				camera.target.z += (phd_cos(item->pos.y_rot) * shift) >> W2V_SHIFT;
				camera.target.x += (phd_sin(item->pos.y_rot) * shift) >> W2V_SHIFT;
			}

			if (camera.fixed_camera ^ fixed_camera)
			{
				camera.target.y = y;
				camera.fixed_camera = camera.speed = 1;
			}
			else
			{
				camera.target.y = y;
				camera.fixed_camera = 0;
			}

			if (camera.speed != 1 && camera.old_type != LOOK_CAMERA)
			{
				camera.target.x = oldtx + ((camera.target.x - oldtx) >> 2);
				camera.target.y = oldty + ((camera.target.y - oldty) >> 2);
				camera.target.z = oldtz + ((camera.target.z - oldtz) >> 2);
			}
		}

		auto floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);

		if (camera.type == CHASE_CAMERA || camera.flags == CHASE_OBJECT)
			ChaseCamera(item);
		else if (camera.type == TARGET_CAMERA)
			TargetCamera();
		else FixedCamera();
	}

	camera.last = camera.number;
	camera.fixed_camera = fixed_camera;

	if (camera.type != HEAVY_CAMERA || camera.timer == -1)
	{
		if (camera.type != TARGET_CAMERA)
		{
			camera.type = CHASE_CAMERA;
			camera.speed = CHASE_SPEED;
			camera.number = NO_CAMERA;
			camera.last_item = camera.item;
			camera.item = nullptr;
			camera.target_angle = camera.target_elevation = 0;
			camera.target_distance = WALL_L * 3 / 2;
			camera.flags = 0;
		}
	}

	chunky_flag = 0;
}

long CameraCollisionBounds(GAME_VECTOR* ideal, long push, long yfirst)
{
	int wx = ideal->x,
		wy = ideal->y,
		wz = ideal->z;

	if (yfirst)
	{
		auto room_number = ideal->room_number;
		auto floor = GetFloor(wx, wy, wz, &room_number);

		int h = GetHeight(floor, wx, wy, wz),
			c = GetCeiling(floor, wx, wy, wz);

		if (wy - 255 < c && wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)  wy = (h + c) >> 1;
		else if (wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)				wy = h - 255;
		else if (wy - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)				wy = c + 255;
	}

	auto room_number = ideal->room_number;
	auto floor = GetFloor(wx - push, wy, wz, &room_number);

	int h = GetHeight(floor, wx - push, wy, wz),
		c = GetCeiling(floor, wx - push, wy, wz);

	if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
		wx = (wx & (~1023)) + push;

	room_number = ideal->room_number;
	floor = GetFloor(wx, wy, wz - push, &room_number);
	h = GetHeight(floor, wx, wy, wz - push);
	c = GetCeiling(floor, wx, wy, wz - push);

	if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
		wz = (wz & (~1023)) + push;

	room_number = ideal->room_number;
	floor = GetFloor(wx + push, wy, wz, &room_number);
	h = GetHeight(floor, wx + push, wy, wz);
	c = GetCeiling(floor, wx + push, wy, wz);

	if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
		wx = (wx | 1023) - push;

	room_number = ideal->room_number;
	floor = GetFloor(wx, wy, wz + push, &room_number);
	h = GetHeight(floor, wx, wy, wz + push);
	c = GetCeiling(floor, wx, wy, wz + push);

	if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
		wz = (wz | 1023) - push;

	if (!yfirst)
	{
		room_number = ideal->room_number;
		floor = GetFloor(wx, wy, wz, &room_number);
		h = GetHeight(floor, wx, wy, wz);
		c = GetCeiling(floor, wx, wy, wz);

		if (wy - 255 < c && wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)  wy = (h + c) >> 1;
		else if (wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)				wy = h - 255;
		else if (wy - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)				wy = c + 255;
	}

	room_number = ideal->room_number;
	floor = GetFloor(wx, wy, wz, &room_number);
	h = GetHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);

	if (wy > h || wy < c || h == NO_HEIGHT || c == NO_HEIGHT || c >= h)
		return 1;

	floor = GetFloor(wx, wy, wz, &ideal->room_number);

	ideal->x = wx;
	ideal->y = wy;
	ideal->z = wz;

	return 0;
}

long mgLOS(GAME_VECTOR* start, GAME_VECTOR* target, long push)
{
	bool clipped = false,
		 nc = false;

	int dx = (target->x - start->x) >> 3,
		dy = (target->y - start->y) >> 3,
		dz = (target->z - start->z) >> 3;

	int x = start->x,
		y = start->y,
		z = start->z;

	auto room_number = start->room_number;
	auto room_number2 = room_number;

	int i = 0;

	for (; i < 8; ++i, room_number = room_number2)
	{
		auto floor = GetFloor(x, y, z, &room_number2);

		if (room[room_number2].flags & SWAMP)
		{
			clipped = true;
			break;
		}

		int h = GetHeight(floor, x, y, z),
			c = GetCeiling(floor, x, y, z);

		if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h)
		{
			if (!nc)
			{
				x += dx;
				y += dy;
				z += dz;

				continue;
			}

			clipped = true;

			break;
		}

		int hdiff = 0,
			cdiff = 0;

		if (y > h)
		{
			if ((hdiff = y - h) < push)
				y = h;
			else
			{
				clipped = true;
				break;
			}
		}

		if (y < c)
		{
			if ((cdiff = c - y) < push)
				y = c;
			else
			{
				clipped = true;
				break;
			}
		}

		nc = 1;

		x += dx;
		y += dy;
		z += dz;
	}

	if (i)
	{
		x -= dx;
		y -= dy;
		z -= dz;
	}

	target->x = x;
	target->y = y;
	target->z = z;

	GetFloor(target->x, target->y, target->z, &room_number);

	target->room_number = room_number;

	return !clipped;
}