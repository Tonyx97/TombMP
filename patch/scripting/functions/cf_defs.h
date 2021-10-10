#pragma once

namespace sol { class state; }

namespace cf_audio
{
	void register_functions(sol::state* vm);
}

namespace cf_camera
{
	void register_functions(sol::state* vm);
}

namespace cf_dx
{
	void register_functions(sol::state* vm);
}

namespace cf_entity
{
	void register_functions(sol::state* vm);
}

namespace cf_item
{
	void register_functions(sol::state* vm);
}

namespace cf_key
{
	void register_functions(sol::state* vm);
}

namespace cf_level
{
	void register_functions(sol::state* vm);
}

namespace cf_physics
{
	void register_functions(sol::state* vm);
}

namespace cf_player
{
	void register_functions(sol::state* vm);
}

namespace cf_server
{
	void register_functions(sol::state* vm);
}

namespace cf_weapon
{
	void register_functions(sol::state* vm);
}

namespace cf_fx
{
	void register_functions(sol::state* vm);
}