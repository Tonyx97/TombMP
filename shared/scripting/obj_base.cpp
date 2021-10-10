import utils;

#include <shared/defs.h>

#include "obj_base.h"
#include "script.h"

obj_base::~obj_base()
{
	if (referenced)
		remove_script_ref();
}

void obj_base::remove_script_ref()
{
	owner->remove_obj(this);
}