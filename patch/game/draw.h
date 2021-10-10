#pragma once

#include "items.h"

struct vec3d;

inline int16_t rooms_to_draw[100];

inline int camera_underwater,
		   number_draw_rooms = 0;

inline bool bRoomOn = true,
			bObjectOn = true,
			bAObjectOn = true,
			bEffectOn = true;

int32_t draw_phase_game();
void draw_rooms(int16_t current_room);
void DrawDummyItem(ITEM_INFO* item);
void draw_lara(ITEM_INFO* item, vec3d* hair_data, short weapon_current_anim_state, bool remote);
void draw_lara_interpolation(ITEM_INFO* item, vec3d* hair_data, int16_t* frame1, int16_t* frame2, int frac, int rate, short weapon_current_anim_state, bool remote);
void DrawEffect(int16_t i);
void DrawAnimatingItem(ITEM_INFO* item);
void DrawGunFlash(int weapon_type, int clip);
void CalculateObjectLighting(ITEM_INFO* item, int16_t* frame);
void CalculateObjectLightingLara();
void phd_PutPolygons_I(int16_t* ptr, int clip);
int GetFrames(ITEM_INFO* item, int16_t* frm[], int* rate);
int16_t* GetBestFrame(ITEM_INFO* item);
int16_t* GetBoundsAccurate(ITEM_INFO* item);

void InitInterpolate(int frac, int rate);
void phd_PopMatrix_I(void);
void phd_PushMatrix_I(void);
void phd_TranslateRel_I(int32_t x, int32_t y, int32_t z);
void phd_TranslateRel_ID(int32_t x, int32_t y, int32_t z, int32_t x2, int32_t y2, int32_t z2);
void phd_RotY_I(int16_t ang);
void phd_RotX_I(int16_t ang);
void phd_RotZ_I(int16_t ang);
void phd_RotYXZ_I(int16_t y, int16_t x, int16_t z);
void gar_RotYXZsuperpack_I(int16_t** pprot1, int16_t** pprot2, int skip);
void ClipRoom(ROOM_INFO* r);
void gar_RotYXZsuperpack(int16_t** pprot, int skip);
void InterpolateMatrix(void);
void InterpolateArmMatrix(void);