import utils;
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>

#include <scripting/events.h>

#include <mp/client.h>

#define DR_WAV_IMPLEMENTATION

#include "dr_wav.h"

#include "json.hpp"

#include "audio_system.h"

using json = nlohmann::json;

bool audio_system::init()
{
	if (!switch_device())
		prof::print(RED, "Could not switch audio device");

	if (!std::filesystem::is_directory("data\\audio"))
		prof::print(RED, "Audio folder doesn't exist");

	if (!std::filesystem::is_directory("data\\audio\\audios"))
		prof::print(RED, "Audios folder doesn't exist");

	prof::print(RED, "OpenAL Initialized");

	alGenBuffers(MAX_BUFFERS, audio_buffers);
	alGenSources(MAX_SOURCES, audio_sources);

	for (int i = 0; i < MAX_BUFFERS; ++i) free_buffers.push(audio_buffers[i]);
	for (int i = 0; i < MAX_SOURCES; ++i) free_sources.push(audio_sources[i]);

	for (const auto& p : std::filesystem::directory_iterator("data\\audio\\audios\\"))
	{
		if (auto fullpath = p.path().string(); !load_audio(fullpath))
		{
			prof::print(RED, "Audio '{}' could not be loaded", fullpath.c_str());
			return false;
		}
	}

	std::ifstream out("data\\audio\\meta.json");

	json audio_list;

	out >> audio_list;

	auto base = audio_list["AudioInfo"];

	if (base.empty())
	{
		prof::print(RED, "meta.json is corrupted");
		return false;
	}

	for (const auto& audio : base)
	{
		std::string id = audio["Id"],
					name = fix_name(audio["Name"]),
					chances = audio["Chance"],
					range = audio["RangeInSectors"],
					volume = audio["Volume"],
					pitch = audio["PitchFactor"],
					flags = audio["LoopBehaviour"],
					sync = audio["Sync"];

		audio_id hash_id = utils::hash::JENKINS(name.c_str());

		auto ai = new audio_info
		{
			.name	 = name,
			.id		 = std::stoi(id),
			.chances = std::stof(chances),
			.volume  = std::stof(volume),
			.pitch	 = std::stof(pitch),
			.range	 = std::stof(range),
			.flags	 = utils::hash::JENKINS(flags.c_str()),
			.sync	 = utils::string::bool_from_str(sync)
		};

		for (const auto& samples : audio["Samples"])
		{
			std::string sample = samples["FileName"];

			ai->samples.push_back(
			{
				.file = sample,
				.id = utils::hash::JENKINS(fix_name(sample).c_str()),
			});
		}

		audios_info.insert({ hash_id, ai });
		audio_ids.insert({ ai->id, hash_id });
	}

	alDistanceModel(AL_EXPONENT_DISTANCE);

	return (initialized = true);
}

bool audio_system::switch_device()
{
	auto error = [&](const char* error)
	{
		if (device)
			alcCloseDevice(device);

		prof::print(RED, "Audio device switching error ({})", error);

		return false;
	};

	if (device && !alcCloseDevice(device))
		return error("Audio device could not be closed");

	if (context)
		alcDestroyContext(context);

	device = nullptr;
	context = nullptr;

	if (!(device = alcOpenDevice(0)))
		return error("Audio device could not be opened");

	if (!(context = alcCreateContext(device, nullptr)))
		return error("Audio context could not be created");

	if (!alcMakeContextCurrent(context))
		return error("Audio context could not be marked as 'current'");

	device_name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);

	prof::print(GREEN, "Audio Device '{}' opened", device_name.c_str());

	return true;
}

bool audio_system::load_audio(const std::string& filename)
{
	auto error = [&](const char* error)
	{
		prof::print(RED, "Fail loading audio ({})", error);
		return false;
	};

	if (free_buffers.empty())
		return error("There is no space to load this audio into");

	auto hash = utils::hash::JENKINS(fix_name(filename).c_str());

	if (buffers.contains(hash))
		return true;

	drwav wav;

	if (!drwav_init_file(&wav, filename.c_str(), nullptr))
		return error("Could not load wav file");

	auto sample_data = new int16_t[(int)wav.totalPCMFrameCount * wav.channels]();

	drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, sample_data);

	auto buffer = free_buffers.top();

	alBufferData(buffer, wav.channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, sample_data, (int)wav.dataChunkDataSize, wav.sampleRate);

	delete[] sample_data;

	if (auto err = alGetError(); err != AL_NO_ERROR)
		return error("An OpenAL error ocurred");

	free_buffers.pop();

	buffers.insert({ hash, buffer });

	return true;
}

bool audio_system::unload_audio(const std::string& filename)
{
	auto error = [&](const char* error)
	{
		prof::print(RED, "Fail unloading audio ({})", error);
		return false;
	};

	auto hash = utils::hash::JENKINS(fix_name(filename).c_str());
	
	if (auto it = buffers.find(hash); it != buffers.end())
	{
		free_buffers.push(it->second);

		buffers.erase(it);

		return true;
	}

	return error("Buffer does not exist");
}

audio* audio_system::create_sound(script* s, audio_id hash, const int_vec3& pos, float pitch)
{
	if (!s || free_sources.empty())
		return nullptr;
	
	auto buf_it = buffers.find(hash);
	if (buf_it == buffers.end())
		return nullptr;

	buf_src_id source = free_sources.top(),
			   buffer = buf_it->second;

	auto a = new audio(hash, source, buffer);

	if (!a->init(pos, master_volume, pitch, nullptr))
	{
		delete a;
		return nullptr;
	}

	if (!g_resource->trigger_event(events::audio::ON_AUDIO_PLAY, uint32_t(hash), pos.x, pos.y, pos.z))
	{
		delete a;
		return nullptr;
	}

	free_sources.pop();

	audios.insert(a);

	a->play();

	return s->add_obj(a);
}

audio* audio_system::create_sound(script* s, const std::string& file, const int_vec3& pos, float pitch)
{
	return (s ? create_sound(s, utils::hash::JENKINS(fix_name(file).c_str()), pos, pitch) : nullptr);
}

bool audio_system::play_sound(audio_id hash, const int_vec3& pos, float pitch, bool sync)
{
	if (free_sources.empty())
		return false;

	auto ai_it = audios_info.find(hash);
	if (ai_it == audios_info.end())
		return false;

	auto ai = ai_it->second;

	if (ai->samples.empty())
		return false;

	int total_samples = ai->samples.size();

	sample_info* sample = nullptr;

	if (total_samples == 1)
	{
		if (ai->chances < 100 && utils::rand::rand_int(0, 100) > ai->chances)
			return true;
		
		sample = &(*ai->samples.begin());
	}
	else sample = &ai->samples[utils::rand::rand_int(0, total_samples - 1)];
	
	auto buf_it = buffers.find(sample->id);
	if (buf_it == buffers.end())
		return false;

	bool looped = (ai->flags == AUDIO_FLAG_LOOPED),
		 one_shot_rewind = (ai->flags == AUDIO_FLAG_ONE_SHOT_REWIND),
		 one_shot_wait = (ai->flags == AUDIO_FLAG_ONE_SHOT_WAIT);
	
	if (auto existing_audio = get_flagged_audio(hash))
	{
		if (existing_audio->is_one_shot_rewind())
			existing_audio->rewind();

		existing_audio->set_position(pos);
		existing_audio->set_pitch(pitch);
		
		return true;
	}

	buf_src_id source = free_sources.top(),
			   buffer = buf_it->second;

	auto a = new audio(hash, source, buffer);

	if (!a->init(pos, master_volume, pitch, ai))
	{
		delete a;
		return false;
	}

	if (!g_resource->trigger_event(events::audio::ON_AUDIO_PLAY, uint32_t(sample->id), pos.x, pos.y, pos.z))
	{
		delete a;
		return false;
	}

	free_sources.pop();

	audios.insert(a);

	if (sync && ai->sync)
	{
		gns::audio::play info
		{
			.pos = pos,
			.hash = hash,
			.pitch = pitch
		};

		g_client->send_packet(ID_AUDIO_PLAY, info);
	}
	
	a->play();

	return true;
}

bool audio_system::play_sound(int id, const int_vec3& pos, float pitch, bool sync)
{
	auto it = audio_ids.find(id);
	return (it != audio_ids.end() ? play_sound(it->second, pos, pitch, sync) : false);
}

bool audio_system::stop_sound(audio_id hash, bool sync)
{	
	std::erase_if(audios, [&](audio* a)
	{
		if (a->get_hash() != hash)
			return false;

		destroy_audio(a);

		if (sync)
		{
			auto ai_it = audios_info.find(hash);
			if (ai_it == audios_info.end())
				return true;

			auto ai = ai_it->second;

			if (ai->sync)
			{
				gns::audio::stop info { .hash = hash };

				g_client->send_packet(ID_AUDIO_STOP, info);
			}
		}
		
		return true;
	});

	return true;
}

bool audio_system::stop_sound(int id, bool sync)
{
	auto it = audio_ids.find(id);
	return (it != audio_ids.end() ? stop_sound(it->second, sync) : false);
}

bool audio_system::stop_all()
{
	for (const auto& a : audios)
		a->stop();

	return true;
}

audio* audio_system::get_flagged_audio(audio_id hash)
{
	for (const auto& a : audios)
		if (a->get_hash() == hash && a->is_flagged_audio())
			return a;

	return nullptr;
}

std::string audio_system::fix_name(std::string name)
{
	if (auto b = *name.begin(); b >= '0' && b <= '9')
		name.insert(name.begin(), '_');

	if (auto i = name.find_last_of("\\/"))
		name = name.substr(i + 1, name.length());

	if (auto i = name.find_last_of("."))
		name = name.substr(0, i);

	name.erase(std::remove(name.begin(), name.end(), '!'), name.end());

	std::replace(name.begin(), name.end(), '&', '_');
	std::replace(name.begin(), name.end(), '-', '_');

	return name;
}

void audio_system::destroy_audio(audio* a, bool unlist)
{
	if (!has_audio(a))
		return;

	a->stop();

	free_sources.push(a->get_id());

	if (unlist)
		audios.erase(a);

	delete a;
}

void audio_system::update(const int_vec3& pos, float angle)
{
	float listener_direction[] = { std::sinf(angle), 0.f, std::cosf(angle), 0.f, -1.f, 0.f };

	float x = float(pos.x) / 449.f,
		  y = float(pos.y) / 449.f,
		  z = float(pos.z) / 449.f;

	alListener3f(AL_POSITION, x, y, z);
	alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
	alListenerfv(AL_ORIENTATION, listener_direction);

	std::erase_if(audios, [&](audio* a)
	{
		if (!a->can_be_destroyed())
			return false;

		destroy_audio(a);
		
		return true;
	});

	if (master_volume != old_master_volume)
	{
		for (const auto& a : audios)
			a->set_volume(master_volume * a->get_volume());

		old_master_volume = master_volume;
	}
}

void audio_system::destroy()
{
	if (!initialized)
		return;

	for (const auto& [audio_id, ai] : audios_info)
		delete ai;

	alDeleteSources(sizeof(audio_sources), audio_sources);
	alDeleteBuffers(sizeof(audio_buffers), audio_buffers);

	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
}