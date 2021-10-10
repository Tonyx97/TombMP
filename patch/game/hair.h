#pragma once

void InitialiseHair(vec3d* data, int_vec3* hair_vel);
void HairControl(ITEM_INFO* item, vec3d* data, int_vec3* hair_vel);
void DrawHair(vec3d* data);