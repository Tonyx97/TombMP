#pragma once

void S_InsertBackground(int16_t* objptr);
void S_InsertRoom(int16_t* objptr, int outside);

inline int32_t outside = 0;
inline bool fix_skybox = true;

inline int CurrentRoom = -1;