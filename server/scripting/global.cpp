#include <shared/defs.h>

#include "global.h"
#include "globals.h"

#include "functions/sf_defs.h"

void scripting::init_functions_and_globals(sol::state* vm)
{
	sf_server::register_functions(vm);
	sf_level::register_functions(vm);
	sf_player::register_functions(vm);
	sf_entity::register_functions(vm);
}