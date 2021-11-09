#pragma once

void* game_malloc(int size, int type);
void game_free(void* ptr, int type);
void free_game_memory();

void S_InitialiseSystem();
void CalculateWibbleTable();

void refresh_gamma_ramp();
void InitZTable();
void InitUVTable();
void InitBuckets();

inline int SqrTable[1024];
inline unsigned int RColorTable[33][33][33];
inline unsigned int GColorTable[33][33][33];
inline unsigned int BColorTable[33][33][33];