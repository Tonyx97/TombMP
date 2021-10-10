#pragma once

namespace sol { class state; }

namespace sf_entity
{
	void register_functions(sol::state* vm);
}

namespace sf_level
{
	void register_functions(sol::state* vm);
}

namespace sf_player
{
	void register_functions(sol::state* vm);
}

namespace sf_server
{
	void register_functions(sol::state* vm);
}