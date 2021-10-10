#pragma once

namespace audio_handlers
{
	void handle_packet(uint16_t);

	void on_audio_play();
	void on_audio_stop();
}

namespace fx_handlers
{
	void handle_packet(uint16_t);

	void on_fx_gun_smoke();
	void on_fx_gunshell();
}

namespace level_handlers
{
	void handle_packet(uint16_t);

	void on_level_info();
	void on_level_load();
	void on_level_flip_map();
}

namespace player_handlers
{
	void handle_packet(uint16_t);

	void on_player_sync_with_players();
	void on_player_spawn();
	void on_player_info();
	void on_player_death();
	void on_player_respawn();
}

namespace projectile_handlers
{
	void handle_packet(uint16_t);

	void on_projectile_create();
}

namespace entity_handlers
{
	void handle_packet(uint16_t);

	void on_entity_hit_damage();
	void on_entity_explode();
	void on_entity_spawn();
	void on_entity_attach();
}

namespace sync_handlers
{
	void handle_packet(uint16_t);

	void on_level_entities();
	void on_request_stream_info();
	void on_stream_info();
	void on_initial_info();
	void on_block();
	void on_ai();
	void on_vehicle();
	void on_interactive();
	void on_others();
	void on_attachments();
	void on_kill();
}