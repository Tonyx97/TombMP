#pragma once

void draw_shotgun(int weapon_type);
void undraw_shotgun(int weapon_type);
void draw_shotgun_meshes(int weapon_type);
void undraw_shotgun_meshes(int weapon_type);
void ready_shotgun(int weapon_type);
void RifleHandler(int weapon_type);
void animate_shotgun(int weapon_type);
void FireShotgun();
void FireHarpoon();
void FireM16(int running);
void FireRocket();
void FireGrenade();
void ControlHarpoonBolt(int16_t item_number);
void ControlRocket(int16_t item_number);
void ControlGrenade(int16_t item_number);
void TriggerUnderwaterExplosion(ITEM_INFO* item);