#pragma once

namespace sol { class state; }

namespace scripting
{
	void init_functions_and_globals(sol::state* vm);
}