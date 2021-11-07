#pragma once

#include <shared/game/math.h>
#include <shared/scripting/obj_base.h>

#include "audios.h"
#include "samples.h"

using buf_src_id = unsigned int;

struct sample_info
{
	std::string file;

	sample_id id;
};

struct audio_info
{
	std::vector<sample_info> samples;

	std::string name;

	int id;

	float chances,
		  volume,
		  pitch,
		  range;

	audio_flags flags;

	bool sync;
};

class audio : public obj_base
{
private:

	int_vec3 position;

	audio_id hash;
	
	buf_src_id id,
			   buffer;

	audio_flags flags;

	float volume = 0.f,
		  pitch = 0.f,
		  range = 0.f,
		  volume_base = 0.f,
		  pitch_base = 0.f,
		  range_base = 0.f;

	bool was_3d = false,
		 is_3d = false,
		 looped = false,
		 one_shot_rewind = false,
		 one_shot_wait = false,
		 manually_stopped = false;

public:

	static constexpr buf_src_id INVALID_ID()	{ return ~(0l); }

	audio(audio_id hash, buf_src_id src, buf_src_id buffer) : hash(hash), id(src), buffer(buffer) { obj_type = OBJ_TYPE_AUDIO; }
	~audio();

	bool init(const int_vec3& pos, float master_vol, float pitch, audio_info* ai);

	void play();
	void stop(bool by_engine = true);
	void rewind(bool by_engine = true);
	void set_position(const int_vec3& v);
	void set_pitch(float v);
	void set_volume(float v, bool update = false);

	bool can_be_destroyed();
	bool is_playing();
	bool is_looped() const						{ return looped; }
	bool is_one_shot_rewind() const				{ return one_shot_rewind; }
	bool is_one_shot_wait() const				{ return one_shot_wait; }
	bool is_flagged_audio() const				{ return flags != AUDIO_FLAG_NORMAL; }

	float get_volume() const					{ return volume; }

	audio_id get_hash() const					{ return hash; }

	buf_src_id get_id() const					{ return id; }
	buf_src_id get_buffer() const				{ return buffer; }
	
};