#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "anim.h"
#include "control.h"

#include <specific/init.h>
#include <specific/fn_stubs.h>

struct TMP_ANIM_STRUCT
{
	int16_t* frame_ptr;
	int16_t interpolation;
	int16_t current_anim_state;
	int32_t velocity;
	int32_t acceleration;
	int16_t frame_base;
	int16_t frame_end;
	int16_t jump_anim_num;
	int16_t jump_frame_num;
	int16_t number_changes;
	int16_t change_ptr;
	int16_t number_commands;
	int16_t command_ptr;
};

struct TMP_CHANGE_STRUCT
{
	int16_t goal_anim_state;
	int16_t number_ranges;
	int16_t range_ptr;
};

std::set<int16_t> g_free_custom_anims,
				  g_used_custom_anims;

void init_custom_animations_pools(int normal_anims_count)
{
	max_number_custom_anims = 1024;

	g_free_custom_anims.clear();
	g_used_custom_anims.clear();

	for (int i = 0; i < max_number_custom_anims; ++i)
		g_free_custom_anims.insert(normal_anims_count + i);
}

bool is_valid_anim(int16_t id)
{
	return (id >= 0 && id < number_anims + max_number_custom_anims);
}

bool unload_animation(int16_t id)
{
	if (g_used_custom_anims.empty())
		return false;

	if (!g_used_custom_anims.contains(id))
		return false;

	memcpy(&anims[id], &anims[STOP_A], sizeof(*anims));

	g_used_custom_anims.erase(id);
	g_free_custom_anims.insert(id);

	return true;
}

bool is_custom_anim_loaded(int16_t id)
{
	return g_used_custom_anims.contains(id);
}

int16_t load_animation(const std::string& filename)
{
	if (g_free_custom_anims.empty())
		return -1;

	auto anim_file = std::ifstream(filename, std::ios::binary);
	if (!anim_file)
		return -1;

	TMP_ANIM_STRUCT anim_info;

	int16_t size_of_frame,
			anim_len,
			commands_len;

	anim_file.read((char*)&anim_info, sizeof(anim_info));
	anim_file.read((char*)&size_of_frame, sizeof(size_of_frame));
	anim_file.read((char*)&anim_len, sizeof(anim_len));
	anim_file.read((char*)&commands_len, sizeof(commands_len));

	auto anim_frame_data = (char*)game_malloc(anim_len * size_of_frame);
	auto anim_command_data = (int16_t*)game_malloc(commands_len);

	anim_file.read(anim_frame_data, size_of_frame * anim_len);
	anim_file.read((char*)anim_command_data, commands_len);

	anim_file.close();

	auto anim_id = *g_free_custom_anims.begin();
	auto anim = &anims[anim_id];

	anim->frame_ptr = (int16_t*)anim_frame_data;
	anim->change_ptr = (int16_t*)&changes[(int16_t)anim_info.change_ptr];
	anim->command_ptr = anim_command_data;
	anim->interpolation = anim_info.interpolation;
	anim->current_anim_state = anim_info.current_anim_state;
	anim->velocity = anim_info.velocity;
	anim->acceleration = anim_info.acceleration;
	anim->frame_base = anim_info.frame_base;
	anim->frame_end = anim_info.frame_end;
	anim->jump_anim_num = anim_info.jump_anim_num;
	anim->jump_frame_num = anim_info.jump_frame_num;
	anim->number_changes = anim_info.number_changes;
	anim->number_commands = anim_info.number_commands;

	g_free_custom_anims.erase(anim_id);
	g_used_custom_anims.insert(anim_id);

	++number_custom_anims;

	return anim_id;
}