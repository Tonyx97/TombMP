#pragma once

#include <shared/game/obj_types.h>
#include <specific/stypes.h>
#include <string>

void ControlAnimating_1_4(short item_number);

void init_custom_objects_pools(int mesh_data_size, int normal_objs_count);

bool unload_obj(int16_t id);

int16_t load_obj(const std::string& filename);

inline int16_t** g_custom_meshes = nullptr;

inline int max_number_custom_objs = 0,
		   max_number_custom_mesh_data = 0;