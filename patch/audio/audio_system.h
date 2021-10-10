#pragma once

#include <shared/game/math.h>

#include <al/al.h>
#include <al/alc.h>

#include <unordered_map>
#include <tuple>

#include "audio.h"

class audio_system
{
public:

	static constexpr int MAX_BUFFERS = 1024;
	static constexpr int MAX_SOURCES = 256;

private:

	std::string device_name;

	ALCdevice* device = nullptr;
	ALCcontext* context = nullptr;

	buf_src_id audio_buffers[MAX_BUFFERS],
			   audio_sources[MAX_SOURCES];

	std::stack<buf_src_id> free_buffers,
						   free_sources;

	std::unordered_map<audio_id, buf_src_id> buffers;
	std::unordered_map<audio_id, audio_info*> audios_info;

	std::unordered_map<int, audio_id> audio_ids;

	std::unordered_set<audio*> audios;

	float master_volume = 1.f,
		  old_master_volume = master_volume;

	bool initialized = false;

	void destroy();

public:

	audio_system()									{}
	~audio_system()									{ destroy(); }

	bool init();
	bool switch_device();
	bool load_audio(const std::string& filename);
	bool unload_audio(const std::string& filename);
	audio* create_sound(class script* s, audio_id hash, const int_vec3& pos = {}, float pitch = 1.f);
	audio* create_sound(class script* s, const std::string& file, const int_vec3& pos = {}, float pitch = 1.f);
	bool play_sound(audio_id hash, const int_vec3& pos = {}, float pitch = 1.f, bool sync = true);
	bool play_sound(int id, const int_vec3& pos = {}, float pitch = 1.f, bool sync = true);
	bool stop_sound(audio_id hash, bool sync = true);
	bool stop_sound(int id, bool sync = true);
	bool stop_all();
	bool has_audio(audio* a) const					{ return audios.contains(a); }

	audio* get_flagged_audio(audio_id hash);

	void destroy_audio(audio* a, bool unlist = false);
	void update(const int_vec3& pos, float angle);

	void set_master_volume(float val)				{ master_volume = val; }
	void increase_master_volume(float val)			{ master_volume += val; }
	void decrease_master_volume(float val)			{ master_volume -= val; }

	float get_master_volume()						{ return master_volume; }

	std::string fix_name(std::string name);
};

inline std::unique_ptr<audio_system> g_audio;