#include <shared/defs.h>

#include <sol/sol.hpp>

#include "global.h"
#include "globals.h"
#include "functions/cf_defs.h"

#include <mp/game/player.h>
#include <mp/game/level.h>

#include <specific/standard.h>

void scripting::init_functions_and_globals(sol::state* vm)
{
	auto localplayer = game_level::LOCALPLAYER();

	vm->set(globals::LOCALPLAYER_ITEM, localplayer->get_item());
	vm->set(globals::LOCALPLAYER, localplayer);

	cf_key::register_functions(vm);
	cf_physics::register_functions(vm);
	cf_dx::register_functions(vm);
	cf_player::register_functions(vm);
	cf_camera::register_functions(vm);
	cf_item::register_functions(vm);
	cf_server::register_functions(vm);
	cf_level::register_functions(vm);
	cf_audio::register_functions(vm);
	cf_entity::register_functions(vm);
	cf_weapon::register_functions(vm);
	cf_fx::register_functions(vm);
}