#pragma once

#include <any>

#include <sol/sol.hpp>

int sol_lua_push(const sol::types<std::any>&, lua_State* L, const std::any& v);