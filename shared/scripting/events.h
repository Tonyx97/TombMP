#pragma once

namespace events
{
	namespace resource
	{
		static inline std::string ON_START = "onResourceStart";
		static inline std::string ON_STOP = "onResourceStop";
	}

	namespace player
	{
		static inline std::string ON_PLAYER_JOIN = "onPlayerJoin";
		static inline std::string ON_PLAYER_QUIT = "onPlayerQuit";
		static inline std::string ON_PLAYER_DIED = "onPlayerDied";
	}
}