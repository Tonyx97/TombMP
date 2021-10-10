#include <shared/defs.h>

#include <sol/sol.hpp>

#include "cf_defs.h"

#include <specific/standard.h>

#include <game/camera.h>

void cf_camera::register_functions(sol::state* vm)
{
	vm->set_function("getCameraPosition", [&]() -> std::tuple<int, int, int, uint16_t>
	{
		return { camera.pos.x, camera.pos.y, camera.pos.z, camera.pos.room_number };
	});

	vm->set_function("getCameraFOV", [&]() { return camera.fov; });

	vm->set_function("setCameraFOV", [&](int16_t fov)
	{
		AlterFOV(fov);
	});

	vm->set_function("getCameraRoll", [&]()				{ return camera.roll; });
	vm->set_function("setCameraRoll", [&](int16_t roll) { camera.roll = roll; });
}