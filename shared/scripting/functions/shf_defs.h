#pragma once

namespace sol { class state; }

namespace shf_resource
{
	void register_functions(sol::state* vm);
}

namespace shf_util
{
	void register_functions(sol::state* vm);
}