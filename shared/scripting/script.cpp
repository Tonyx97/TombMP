#include <shared/defs.h>

#include <shared/timer_system/timer_system.h>

#ifdef GAME_CLIENT
#include <audio/audio_system.h>
#endif

#include "script.h"
#include "resource.h"
#include "resource_system.h"
#include "globals.h"
#include "obj_base.h"

#include "functions/shf_defs.h"

script::script(resource* owner, const std::string& subpath, const std::string& lookup_name, script_type type) :
			   owner(owner), subpath(subpath), lookup_name(lookup_name), type(type)
{
	fullpath = resource_system::RESOURCES_PATH + owner->get_folder() + '\\' + subpath;
}

script::~script()
{
	stop();
}

void script::invalidate_vm()
{
	destroy_objs();

	delete vm;
	vm = nullptr;
}

void script::destroy_objs()
{
	can_remove_refs = false;

	for (auto it = objs_references.begin(); it != objs_references.end(); ++it)
	{
		auto obj = *it;

		switch (obj->get_obj_type())
		{
		case OBJ_TYPE_TIMER: g_timer->destroy_timer(static_cast<timer*>(obj)); break;
#ifdef GAME_CLIENT
		case OBJ_TYPE_AUDIO: g_audio->destroy_audio(static_cast<audio*>(obj), true); break;
#endif
		}
	}

	objs_references.clear();

	can_remove_refs = true;
}

void script::dispatch_error(const sol::error& err)
{
	owner->set_last_error(err.what());

	if (auto err_callback = g_resource->get_err_callback())
		err_callback(this, err.what());
}

void script::register_shared_globals()
{
	vm->set(scripting::globals::THIS_RESOURCE, owner);
	vm->set(scripting::globals::RESOURCE_NAME, owner->get_folder());
	vm->set(scripting::globals::SCRIPT_INSTANCE, (uintptr_t)this);
}

void script::register_shared_functions()
{
	shf_resource::register_functions(vm);
	shf_util::register_functions(vm);
}

void script::call_fn_va(const std::string& name, const std::vector<std::any>& va)
{
	if (!is_running())
		return;

	if (sol::safe_function fn = (*vm)[name]; fn.valid())
	{
		sol::protected_function_result res = (va.empty() ? fn() : fn(sol::as_args(va)));

		if (!res.valid())
			dispatch_error(res);
	}
}

script_action_result script::start()
{
	if (!vm)
	{
		vm = new sol::state();

		vm->open_libraries();

		if (const auto& setup_fn = g_resource->get_script_setup_fn())
			setup_fn(vm);

		register_shared_globals();
		register_shared_functions();

		if (auto res = vm->safe_script_file(fullpath); res.valid())
			return SCRIPT_ACTION_OK;
		else dispatch_error(res);

		invalidate_vm();

		return SCRIPT_ACTION_SCRIPT_ERROR;
	}

	return SCRIPT_ACTION_ALREADY_RUNNING;
}

script_action_result script::stop()
{
	if (is_running())
	{
		invalidate_vm();
		return SCRIPT_ACTION_OK;
	}
	
	return SCRIPT_ACTION_NOT_RUNNING;
}

script_action_result script::restart()
{
	if (auto stop_res = stop(); stop_res != SCRIPT_ACTION_OK)
		return stop_res;

	return start();
}

std::string script::get_global_string(const sol::this_state& s, const char* name)
{
	return sol::state_view(s).get<std::string>(name);
}

script* script::get_global_script(const sol::this_state& s)
{
	return (script*)(sol::state_view(s).get<uintptr_t>(scripting::globals::SCRIPT_INSTANCE));
}