#pragma once

inline int alloc_multiplier = 1;

void* game_malloc(int size, int type = 0);
void* game_realloc(void* ptr, int size);
void game_free(void* ptr, int type = 0);
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