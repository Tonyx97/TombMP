#include "lua_any_pusher.h"

#ifdef GAME_SERVER
#include <game/player.h>
#include <game/entity.h>
#else
#include <mp/game/player.h>
#include <mp/game/entity.h>
#endif

int sol_lua_push(const sol::types<std::any>&, lua_State* L, const std::any& v)
{
	int amount = 0;

	if (v.type() == typeid(int32_t))			amount = sol::stack::push(L, std::any_cast<int>(v));
	else if (v.type() == typeid(double))		amount = sol::stack::push(L, std::any_cast<double>(v));
	else if (v.type() == typeid(std::string))	amount = sol::stack::push(L, std::any_cast<std::string>(v));
	else if (v.type() == typeid(bool))			amount = sol::stack::push(L, std::any_cast<bool>(v));
	else if (v.type() == typeid(const char*))	amount = sol::stack::push(L, std::any_cast<const char*>(v));
	else if (v.type() == typeid(float))			amount = sol::stack::push(L, std::any_cast<float>(v));
	else if (v.type() == typeid(game_player*))	amount = sol::stack::push(L, std::any_cast<game_player*>(v));
	else if (v.type() == typeid(game_entity*))	amount = sol::stack::push(L, std::any_cast<game_entity*>(v));
	else if (v.type() == typeid(uint32_t))		amount = sol::stack::push(L, std::any_cast<uint32_t>(v));
	else if (v.type() == typeid(uint8_t))		amount = sol::stack::push(L, std::any_cast<uint8_t>(v));
	else if (v.type() == typeid(int8_t))		amount = sol::stack::push(L, std::any_cast<int8_t>(v));
	else if (v.type() == typeid(uint16_t))		amount = sol::stack::push(L, std::any_cast<uint16_t>(v));
	else if (v.type() == typeid(int16_t))		amount = sol::stack::push(L, std::any_cast<int16_t>(v));
	else if (v.type() == typeid(uint64_t))		amount = sol::stack::push(L, std::any_cast<uint64_t>(v));
	else if (v.type() == typeid(int64_t))		amount = sol::stack::push(L, std::any_cast<int64_t>(v));

	return amount;
}