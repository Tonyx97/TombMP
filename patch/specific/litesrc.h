#pragma once

int16_t* calc_vertice_light(int16_t* objptr, int16_t*);

void S_CalculateLight(int32_t x, int32_t y, int32_t z, int16_t room_number, ITEM_LIGHT* itemlight);
void S_CalculateStaticLight(int16_t adder);

inline int32_t LightPos[12];
inline int32_t LightCol[12];
inline PHD_VECTOR LPos[3];

inline int smcr, smcg, smcb,
		   number_dynamics, sunset;

inline LIGHT_INFO dynamic[MAX_DYNAMICS];