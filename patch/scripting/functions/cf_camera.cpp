#include <shared/defs.h>

#include <sol/sol.hpp>

#include "cf_defs.h"

#include <specific/standard.h>

#include <game/camera.h>

void cf_camera::register_functions(sol::state* vm)
{
	vm->set_function("resetCamera", [&]()
	{
		InitialiseResetCamera();
	});

	vm->set_function("getCameraPosition", [&]() -> std::tuple<int, int, int, uint16_t>
	{
		return { camera.pos.x, camera.pos.y, camera.pos.z, camera.pos.room_number };
	});

	vm->set_function("setCameraPosition", [&](int x, int y, int z)
	{
		camera.pos.x = x;
		camera.pos.y = y;
		camera.pos.z = z;
	});

	vm->set_function("getCameraRoom", [&]()				{ return camera.pos.room_number; });
	vm->set_function("setCameraRoom", [&](int16_t room) { camera.pos.room_number = room; });

	vm->set_function("getCameraFOV", [&]()				{ return camera.fov; });
	vm->set_function("setCameraFOV", [&](int16_t fov)	{ AlterFOV(fov); });

	vm->set_function("getCameraRoll", [&]()				{ return camera.roll; });
	vm->set_function("setCameraRoll", [&](int16_t roll) { camera.roll = roll; });

	vm->set_function("getCameraType", [&]()			{ return int(camera.type); });
	vm->set_function("setCameraType", [&](int type) { camera.type = camera_type(type); });

	vm->set_function("getCameraItem", [&]()					{ return camera.item; });
	vm->set_function("setCameraItem", [&](ITEM_INFO* item)	{ camera.item = item; });

	vm->set_function("getCameraTarget", [&]() -> std::tuple<int, int, int, uint16_t>
	{
		return { camera.target.x, camera.target.y, camera.target.z, camera.target.room_number };
	});

	vm->set_function("setCameraTarget", [&](int x, int y, int z, int16_t room)
	{
		camera.target = { x, y, z, room };
	});
}