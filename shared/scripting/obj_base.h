#pragma once

enum e_obj_type
{
	OBJ_TYPE_TIMER,
	OBJ_TYPE_AUDIO,
	OBJ_TYPE_UNKNOWN,
};

class script;

class obj_base
{
protected:

	script* owner = nullptr;

	int obj_type = OBJ_TYPE_UNKNOWN;

	bool referenced = false;

	void remove_script_ref();

public:

	obj_base()  {}
	~obj_base();

	void set_owner(script* v)	{ owner = v; referenced = true; }

	int get_obj_type() const	{ return obj_type; }

	script* get_owner() const	{ return owner; }
};