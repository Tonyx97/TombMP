#pragma once

#include <game/room.h>

#define SIPL_DONT_CLEAR_SCREEN 0
#define SIPL_CLEAR_SCREEN 1

typedef struct
{
	signed char shimmer;
	signed char choppy;
	unsigned char random;
	unsigned char abs;
} WATERTAB;

inline WATERTAB	WaterTable[22][64];
inline int wibble_light[WIBBLE_SIZE][32];

void S_InitialisePolyList(int tClearScreen);
int32_t dump_screen();
void ScreenPartialDump();
void ScreenDump();
void S_ClearScreen();
void S_PrintShadow(int16_t size, int16_t* bptr, class ITEM_INFO* iptr, int unknown);

void S_LightRoom(ROOM_INFO* r);

void S_CalculateStaticLight(int16_t shade);

#define S_CalculateStaticMeshLight(x, y, z, shade, shadeB, r) S_CalculateStaticLight(shade)

void S_InitialiseScreen();
void output_polylist();
int S_GetObjectBounds(int16_t* bptr);
void S_DrawHealthBar(int percent, bool poisoned);
void S_DrawHealthBar3D(int32_t wx, int32_t wy, int32_t wz, int percent, bool poisoned);
void S_DrawAirBar(int percent);
void S_DrawDashBar(int);
void S_InsertBackPolygon(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y, int colour);
void animate_textures(int nframes);
void S_SetupBelowWater(int underwater);
void S_SetupAboveWater(int underwater);
void S_DrawSparks();
void TriggerDynamicLight(long x, long y, long z, long falloff, long r, long g, long b);
void ClearDynamics();
void S_DrawFootPrints();
void S_DrawDarts(ITEM_INFO* item);

void mCalcPoint(long dx, long dy, long dz, long* result);
void ProjectPCoord(long x, long y, long z, long* result, long cx, long cy, long fov);
bool world_to_screen(int x, int y, int z, int& ox, int& oy);

void ScreenClear(bool tWindow = false);