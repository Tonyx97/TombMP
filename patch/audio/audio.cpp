import utils;

#include <shared/defs.h>

#include <al/al.h>
#include <al/alc.h>

#include "audio.h"

bool audio::init(const int_vec3& pos, float master_vol, float pitch, audio_info* ai)
{
	if (ai)
	{
		this->pitch = pitch + ((pitch_base = ai->pitch) == 100.f ? 0.f : ai->pitch / 100.f);
		volume = (master_vol * ((volume_base = ai->volume) / 100.f));
		range = range_base = ai->range;
		flags = ai->flags;
		looped = (ai->flags == AUDIO_FLAG_LOOPED);
		one_shot_rewind = (ai->flags == AUDIO_FLAG_ONE_SHOT_REWIND);
		one_shot_wait = (ai->flags == AUDIO_FLAG_ONE_SHOT_WAIT);
	}
	else
	{
		this->pitch = pitch_base = pitch;
		volume = volume_base = master_vol;
		range = range_base = 10.f;
		flags = AUDIO_FLAG_NORMAL;
	}

	alSourcei(id, AL_BUFFER, buffer);
	alSourcei(id, AL_LOOPING, looped ? AL_TRUE : AL_FALSE);
	alSourcef(id, AL_PITCH, this->pitch);
	alSourcef(id, AL_GAIN, volume);
	alSourcef(id, AL_MIN_GAIN, 0.f);
	alSourcef(id, AL_MAX_GAIN, 1.f);
	alSourcef(id, AL_REFERENCE_DISTANCE, range);
	alSourcef(id, AL_MAX_DISTANCE, range * 1.5f);
	alSourcef(id, AL_ROLLOFF_FACTOR, 5.f);

	set_position(pos);

	return (alGetError() == AL_NO_ERROR);
}

void audio::play()
{
	alSourcePlay(id);

	manually_stopped = false;
}

void audio::stop(bool by_engine)
{
	alSourceStop(id);

	if (!by_engine)
		manually_stopped = true;
}

void audio::rewind(bool by_engine)
{
	stop(by_engine);

	alSourceRewind(id);

	play();
}

void audio::set_position(const int_vec3& v)
{
	position = v;
	is_3d = (v.x != 0 || v.y != 0 || v.z != 0);

	if (is_3d != was_3d)
		alSourcei(id, AL_SOURCE_RELATIVE, (was_3d = is_3d) ? AL_FALSE : AL_TRUE);

	alSource3f(id, AL_POSITION, float(v.x) / 449.f, float(v.y) / 449.f, float(v.z) / 449.f);
}

void audio::set_pitch(float v)
{
	alSourcef(id, AL_PITCH, v);
}

void audio::set_volume(float v, bool update)
{
	if (update)
		volume = v;

	alSourcef(id, AL_GAIN, v);
}

bool audio::can_be_destroyed()
{
	return (!manually_stopped && !is_playing());
}

bool audio::is_playing()
{
	int state = 0;
	alGetSourcei(id, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
}