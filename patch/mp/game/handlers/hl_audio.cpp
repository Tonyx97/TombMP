import utils;

#include <shared/defs.h>

#include <mp/client.h>

#include <audio/audio_system.h>

#include "hl_defs.h"

void audio_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_AUDIO_PLAY:		return on_audio_play();
	case ID_AUDIO_STOP:		return on_audio_stop();
	}
}

void audio_handlers::on_audio_play()
{
	gns::audio::play info; g_client->read_packet_ex(info);

	g_audio->play_sound(info.hash, info.pos, info.pitch, false);
}

void audio_handlers::on_audio_stop()
{
	gns::audio::stop info; g_client->read_packet_ex(info);

	g_audio->stop_sound(info.hash, false);
}