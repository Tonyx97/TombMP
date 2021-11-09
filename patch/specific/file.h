#pragma once

bool load_level();
bool load_gameflow(const char* filename);
void unload_level();

int16_t load_animation(const std::string& filename);