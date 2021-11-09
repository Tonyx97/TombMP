#pragma once

void* game_malloc(int size, int type);
void game_free(int size, int type);
void S_InitialiseSystem();
void free_game_memory();
void init_game_malloc();
void CalculateWibbleTable();
void show_game_malloc_totals();

void refresh_gamma_ramp();
void InitZTable();
void InitUVTable();
void InitBuckets();

inline int SqrTable[1024];
inline unsigned int RColorTable[33][33][33];
inline unsigned int GColorTable[33][33][33];
inline unsigned int BColorTable[33][33][33];