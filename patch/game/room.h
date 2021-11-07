#pragma once

#define MAX_ROOMS		1024	// should this be this high ?
#define MAX_FLIPMAPS 	10

#define UNDERWATER	1
#define OUTSIDE		8
#define DYNAMIC_LIT 16
#define NOT_INSIDE	32	// OUTSIDE means sky room, NOT_INSIDE just means in the wind/mist etc
#define INSIDE		64	// INSIDE means room *not adjacent* to an OUTSIDE room!!
#define SWAMP		128

#define END_BIT     0x8000
#define VALUE_BITS  0x3ff
#define DATA_TYPE	0x1f

#define WALL_L		1024
#define WALL_SHIFT	10
#define STEP_L		(WALL_L / 4)

#define NO_ROOM 	0xff
#define NO_HEIGHT	(-0x7f00)

struct FLOOR_INFO
{
	uint16_t index;
	uint16_t fx : 4;
	uint16_t box : 11;
	uint16_t stopper : 1;			// Used for pushable blocks in conjunction with the electric cleaner.
	unsigned char pit_room;
	signed char floor;
	unsigned char sky_room;
	signed char ceiling;
};

struct SUNLIGHT
{
	int16_t nx, ny, nz;
	int16_t pad;
};

struct SPOTLIGHT
{
	int32_t intensity;
	int32_t falloff;
};

struct LIGHT_INFO
{
	int32_t x;
	int32_t y;
	int32_t z;
	unsigned char r, g, b, type;
	union
	{
		SUNLIGHT sun;
		SPOTLIGHT spot;
	}l;
};

struct MESH_INFO
{
	int32_t x;
	int32_t y;
	int32_t z;
	PHD_ANGLE y_rot;
	int16_t shade, shadeB;
	int16_t static_number; // STATIC_INFO structure that contains bounding info
};

struct ROOM_INFO
{
	int16_t* data;
	int16_t* door;
	FLOOR_INFO* floor;
	LIGHT_INFO* light;
	MESH_INFO* mesh;
	int32_t x, y, z;
	int32_t minfloor, maxceiling;
	int16_t x_size, y_size;
	int16_t ambient, lighting;
	int16_t num_lights;
	int16_t num_meshes;
	int16_t ReverbType;
	char MeshEffect;
	char bound_active;
	int16_t left;
	int16_t right;
	int16_t top;
	int16_t bottom;
	int16_t test_left, test_right, test_top, test_bottom; // 14/8/97: better bounds testing?
	int16_t item_number;
	int16_t fx_number;
	int16_t flipped_room;	// -1 normally, unless alternative room available
	uint16_t flags;
};

inline ROOM_INFO* room = nullptr;
inline int16_t number_rooms = 0;

inline int16_t* floor_data;
inline int32_t floor_data_count = 0;