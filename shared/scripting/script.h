#pragma once

#include "lua_any_pusher.h"
#include "obj_base.h"

class resource;

enum script_type
{
	SCRIPT_TYPE_CLIENT = 0xb6043d78,
	SCRIPT_TYPE_SERVER = 0x22ab63cd,
	SCRIPT_TYPE_SHARED = 0x8d17348,
	SCRIPT_TYPE_UNKNOWN = 0xFFFFFFFF,
};

enum script_load_result
{
	SCRIPT_OK,
	SCRIPT_FAILED,
	SCRIPT_EXISTS,
	SCRIPT_INVALID_PLATFORM,
};

enum script_action_result
{
	SCRIPT_ACTION_NOT_RUNNING,
	SCRIPT_ACTION_ALREADY_RUNNING,
	SCRIPT_ACTION_SCRIPT_ERROR,
	SCRIPT_ACTION_NOT_FOUND,
	SCRIPT_ACTION_FAIL,
	SCRIPT_ACTION_OK,
};

class script
{
private:

	std::unordered_set<obj_base*> objs_references;

	using obj_ref_it = decltype(objs_references)::const_iterator;

	std::string fullpath,
				subpath,
				lookup_name;

	sol::state* vm = nullptr;

	resource* owner = nullptr;

	script_type type = SCRIPT_TYPE_UNKNOWN;

	bool can_remove_refs = true;

	void register_shared_globals();
	void register_shared_functions();

public:

	static constexpr auto FILE_EXT = ".lua";

	script(resource* owner, const std::string& subpath, const std::string& lookup_name, script_type type);
	~script();

	void invalidate_vm();
	void destroy_objs();
	void dispatch_error(const sol::error& err);

	template <typename T>
	T* add_obj(T* obj)
	{
		if (obj)
		{
			obj->set_owner(this);

			objs_references.insert(static_cast<obj_base*>(obj));
		}

		return obj;
	}

	template <typename T>
	bool remove_obj(T* obj)
	{
		if (!can_remove_refs)
			return false;

		return !!objs_references.erase(static_cast<obj_base*>(obj));
	}

	template <typename T>
	void update_global(const std::string& name, const T& value)
	{
		if (!is_running())
			return;

		vm->set(name, value);
	}

	template <typename... A>
	void call_fn(const std::string& name, A&&... args)
	{
		if (!is_running())
			return;

		if (sol::safe_function fn = (*vm)[name]; fn.valid())
			if (auto res = fn(args...); !res.valid())
				dispatch_error(res);
	}

	void call_fn_va(const std::string& name, const std::vector<std::any>& va);

	bool is_running() const				{ return !!vm; }

	script_action_result start();
	script_action_result stop();
	script_action_result restart();

	script_type get_type() const		{ return type; }

	resource* get_owner() const			{ return owner; }

	sol::state* get_vm() const			{ return vm; }

	std::string get_subpath() const		{ return subpath; }
	std::string get_lookup_name() const { return lookup_name; }

	static std::string get_global_string(const sol::this_state& s, const char* name);
	static script* get_global_script(const sol::this_state& s);
};