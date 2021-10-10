#pragma once

class game_ms
{
private:

	int conn_attemps = 0;

public:

	game_ms();
	~game_ms();
	
	static constexpr int32_t VER_MAJOR = 1,
							 VER_MINOR = 0;

	bool connect();
	bool send_info(const std::string& ip, const std::string& name, const std::string& gamemode, const std::string& map, const std::string& players, int players_count, int max_players);
	bool verify_game_ownership(uint64_t steam_id);

	static constexpr uint32_t VERSION()				{ return (VER_MAJOR << 8) | VER_MINOR; }
	static constexpr uint32_t TYPE()				{ return 1; }
};