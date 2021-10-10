#pragma once

class net_player;

namespace audio_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_audio_play(net_player*);
	void on_audio_stop(net_player*);
}

namespace fx_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_fx_gun_smoke(net_player*);
	void on_fx_gunshell(net_player*);
}

namespace level_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_level_flip_map(net_player*);
}

namespace player_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_player_sync_with_players(net_player*);
	void on_player_info(net_player*);
	void on_player_death(net_player*);
}

namespace projectile_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_projectile_create(net_player*);
}

namespace entity_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_entity_hit_damage(net_player*);
	void on_entity_explode(net_player*);
}

namespace sync_handlers
{
	void handle_packet(net_player*, uint16_t);

	void on_level_entities(net_player*);
	void on_spawned_entities(net_player*);
	void on_request_stream_info(net_player*);
	void on_ownership(net_player*);
	void on_block(net_player*);
	void on_ai(net_player*);
	void on_vehicle(net_player*);
	void on_interactive(net_player*);
	void on_others(net_player*);
	void on_attachments(net_player*);
	void on_trigger_to_object(net_player*);
	void on_kill(net_player*);
}