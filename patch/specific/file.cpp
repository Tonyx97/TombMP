import utils;
import prof;

#include "standard.h"
#include "global.h"
#include "drawprimitive.h"
#include "texture.h"
#include "init.h"
#include "hwrender.h"
#include "file.h"
#include "picture.h"
#include "input.h"

#include <shared/scripting/resource_system.h>

#include <scripting/events.h>

#include <game/game.h>
#include <game/invfunc.h>
#include <game/setup.h>
#include <game/objects.h>
#include <game/camera.h>
#include <game/effects.h>
#include <game/box.h>

#include <ui/ui.h>

#include <mp/client.h>
#include <mp/chat.h>
#include <mp/game/level.h>

#include <scripting/debugger.h>

#include <main.h>

#include <3dsystem/hwinsert.h>

#define THIS_SCRIPT_VERSION 3

uint8_t LabTextureUVFlag[MAX_TEXTURES];

int16_t LGF_offsets[200];

int texture_infos;

BOOL MyReadFile(HANDLE hFile, void* pBuffer, DWORD nNumberOfBytesToRead, DWORD* pNumberOfBytesRead = nullptr, OVERLAPPED* pOverlapped = nullptr)
{
	return ReadFile(hFile, pBuffer, nNumberOfBytesToRead, pNumberOfBytesRead, pOverlapped);
}

bool LoadTexturePages(HANDLE file)
{
	int num_pages;
	
	MyReadFile(file, &num_pages, sizeof(int));

	int nBytesPerPage = 0x20000,
		nAllocAmount = num_pages * nBytesPerPage;
	
	SetFilePointer(file, num_pages * 65536, 0, FILE_CURRENT);

	auto base = new char[nAllocAmount];

	for (int i = 0; i < num_pages; ++i)
		MyReadFile(file, base + (i * nBytesPerPage), nBytesPerPage);

	HWR_LoadTexturePages(num_pages, base);

	delete[] base;

	return true;
}

bool LoadRooms(HANDLE file)
{
	// load room data from file. Return 1 if OK, 0 if failure

	int16_t ndoors;
	int32_t size;

	// read in actual room data

	MyReadFile(file, &number_rooms, sizeof(number_rooms));

	if (number_rooms < 0 || number_rooms > MAX_ROOMS)
	{
		prof::print(DARK_RED, "LoadRoom(): Too many rooms {}", number_rooms);
		return false;
	}

	// allocate memory for rooms

	if (!(room = (ROOM_INFO*)game_malloc(number_rooms * sizeof(ROOM_INFO), ROOM_INFOS)))
	{
		prof::print(DARK_RED, "LoadRoom(): Could not allocate memory for rooms");
		return false;
	}

	auto r = room;

	for (int i = 0; i < number_rooms; ++i, ++r)
	{
		// room position

		MyReadFile(file, &r->x, sizeof(int32_t));
		r->y = 0;
		MyReadFile(file, &r->z, sizeof(int32_t));

		// room floor/ceiling bounds

		MyReadFile(file, &r->minfloor, sizeof(int32_t));
		MyReadFile(file, &r->maxceiling, sizeof(int32_t));

		// room mesh

		MyReadFile(file, &size, sizeof(int32_t));
		r->data = (int16_t*)game_malloc(size * sizeof(int16_t), ROOM_MESH);
		MyReadFile(file, r->data, sizeof(int16_t) * size);

		// doors

		MyReadFile(file, &ndoors, sizeof(int16_t));
		if (ndoors)
		{
			r->door = (int16_t*)game_malloc((ndoors * 16 + 1) * sizeof(int16_t), ROOM_DOOR);
			r->door[0] = ndoors;

			MyReadFile(file, &r->door[1], sizeof(int16_t) * ndoors * 16);
		}
		else r->door = nullptr;

		// floor data

		MyReadFile(file, &r->x_size, sizeof(int16_t));
		MyReadFile(file, &r->y_size, sizeof(int16_t));
		size = (int)r->x_size * r->y_size;
		r->floor = (FLOOR_INFO*)game_malloc(size * sizeof(FLOOR_INFO), ROOM_FLOOR);
		MyReadFile(file, r->floor, sizeof(FLOOR_INFO) * size);

		// light data

		MyReadFile(file, &r->ambient, sizeof(int16_t));
		MyReadFile(file, &r->lighting, sizeof(int16_t));
		MyReadFile(file, &r->num_lights, sizeof(int16_t));

		if (r->num_lights)
		{
			r->light = (LIGHT_INFO*)game_malloc(r->num_lights * sizeof(LIGHT_INFO), ROOM_LIGHTS);

			MyReadFile(file, r->light, sizeof(LIGHT_INFO) * r->num_lights);
		}
		else r->light = nullptr;

		// static mesh objects

		MyReadFile(file, &r->num_meshes, sizeof(int16_t));
		if (r->num_meshes)
		{
			r->mesh = (MESH_INFO*)game_malloc(r->num_meshes * sizeof(MESH_INFO), ROOM_STATIC_MESH_INFOS);
			MyReadFile(file, r->mesh, sizeof(MESH_INFO) * r->num_meshes);
		}
		else r->mesh = nullptr;

		// load in flipped room

		MyReadFile(file, &r->flipped_room, sizeof(int16_t));

		// load in flags (for underwater etc)

		MyReadFile(file, &r->flags, sizeof(uint16_t));

		// load Mesh Effect

		MyReadFile(file, &r->MeshEffect, sizeof(char));

		char unused = 0;

		MyReadFile(file, &unused, sizeof(char));
		MyReadFile(file, &unused, sizeof(char));

		// initialise room variables

		r->bound_active = 0;
		r->left = phd_winxmax;
		r->top = phd_winymax;
		r->right = r->bottom = 0;
		r->item_number = NO_ITEM;
		r->fx_number = NO_ITEM;
	}

	BuildOutsideTable();

	// load in 'floor_data' part of wad

	MyReadFile(file, &floor_data_count, sizeof(int32_t));
	floor_data = (int16_t*)game_malloc(floor_data_count * sizeof(int16_t), FLOOR_DATA);
	MyReadFile(file, floor_data, sizeof(int16_t) * floor_data_count);

	return true;
}

void AdjustTextureUVs(bool tNew)
{
	if (tNew)
		App.nUVAdd = 0;

	auto pTS = phdtextinfo;
	auto pUVF = LabTextureUVFlag;

	int nTI = texture_infos,
		nAdd = 128 - App.nUVAdd;

	for (; --nTI; ++pTS)
	{
		auto pUV = (uint16_t*)&(pTS->u1);
		auto bByte = *pUVF++;

		for (int nBit = 0; nBit < 8; ++pUV, ++nBit)
		{
			if (bByte & 1) *pUV -= nAdd;
			else		   *pUV += nAdd;

			bByte >>= 1;
		}
	}

	App.nUVAdd += nAdd;
}

bool LoadObjects(HANDLE file)
{
	// load actual mesh data

	int32_t mesh_base_size = 0;

	MyReadFile(file, &mesh_base_size, sizeof(int32_t));
	meshes_base = (int16_t*)game_malloc(mesh_base_size * sizeof(int16_t), MESHES);
	MyReadFile(file, meshes_base, sizeof(int16_t) * mesh_base_size);

	// load meshes (offset pointers first)

	MyReadFile(file, &number_meshes, sizeof(int32_t));
	meshes = (int16_t**)game_malloc(number_meshes * sizeof(int16_t*), MESH_POINTERS);
	MyReadFile(file, meshes, sizeof(int16_t*) * number_meshes);

	// adjust mesh offsets using base pointer

	for (int i = 0; i < number_meshes; ++i)
		meshes[i] = meshes_base + (int)meshes[i] / 2;

	// load in animation arrays

	MyReadFile(file, &number_anims, sizeof(int32_t));

	init_custom_animations_pools(number_anims);

	anims = (ANIM_STRUCT*)game_malloc((number_anims + max_number_custom_anims) * sizeof(ANIM_STRUCT), ANIMS);

	for (int i = 0; i < number_anims; ++i)
	{
		auto anim = &anims[i];

		int16_t change_index, command_index;

		MyReadFile(file, anim, offsetof(ANIM_STRUCT, change_ptr) - sizeof(change_index));
		MyReadFile(file, &change_index, sizeof(int16_t));
		MyReadFile(file, &anim->number_commands, sizeof(int16_t));
		MyReadFile(file, &command_index, sizeof(int16_t));

		anim->change_ptr = (CHANGE_STRUCT*)change_index;
		anim->command_ptr = (int16_t*)command_index;
	}

	MyReadFile(file, &number_anim_changes, sizeof(int32_t));
	changes = (CHANGE_STRUCT*)game_malloc(number_anim_changes * sizeof(CHANGE_STRUCT), STRUCTS);

	for (int i = 0; i < number_anim_changes; ++i)
	{
		auto change = &changes[i];

		int16_t range_index;
		
		MyReadFile(file, change, offsetof(CHANGE_STRUCT, range_ptr));
		MyReadFile(file, &range_index, sizeof(int16_t));

		change->range_ptr = (RANGE_STRUCT*)range_index;
	}

	MyReadFile(file, &number_anim_ranges, sizeof(int32_t));
	ranges = (RANGE_STRUCT*)game_malloc(number_anim_ranges * sizeof(RANGE_STRUCT), RANGES);
	MyReadFile(file, ranges, sizeof(RANGE_STRUCT) * number_anim_ranges);

	// load in animation data wodges

	MyReadFile(file, &number_anim_commands, sizeof(int32_t));
	commands = (int16_t*)game_malloc(number_anim_commands * sizeof(int16_t), COMMANDS);
	MyReadFile(file, commands, sizeof(int16_t) * number_anim_commands);

	// bones

	MyReadFile(file, &number_bones, sizeof(int32_t));
	bones = (int32_t*)game_malloc(number_bones * sizeof(int32_t), BONES);
	MyReadFile(file, bones, sizeof(int32_t) * number_bones);

	// frames

	MyReadFile(file, &number_anim_frames, sizeof(int32_t));
	frames = (int16_t*)game_malloc(number_anim_frames * sizeof(int16_t), FRAMES);
	MyReadFile(file, frames, sizeof(int16_t) * number_anim_frames);

	// remap anim pointers
	// what this does is basically assign the actual frame pointer without
	// any offset calculation to avoid the calculation in the future

	for (int i = 0; i < number_anims; ++i)
	{
		auto anim = &anims[i];

		anim->frame_ptr = &frames[int32_t(anim->frame_ptr) / 2];
		anim->change_ptr = &changes[int32_t(anim->change_ptr)];
		anim->command_ptr = &commands[int32_t(anim->command_ptr)];
	}

	for (int i = 0; i < number_anim_changes; ++i)
	{
		auto change = &changes[i];

		change->range_ptr = &ranges[int32_t(change->range_ptr)];
	}

	// load in normal animating objects (NumModels)

	int32_t objects_count;

	MyReadFile(file, &objects_count, sizeof(int32_t));

	objects = (OBJECT_INFO*)game_malloc((NUMBER_OBJECTS + 128) * sizeof(OBJECT_INFO), OBJECTS);

	for (int i = 0; i < objects_count; ++i)
	{
		int j, size;

		MyReadFile(file, &j, sizeof(int32_t));						// read normal object number

		auto obj = &objects[j];

		int32_t bone_index;
		int16_t mesh_index;

		MyReadFile(file, &obj->nmeshes, sizeof(int16_t));
		MyReadFile(file, &mesh_index, sizeof(int16_t));
		MyReadFile(file, &bone_index, sizeof(int32_t));
		MyReadFile(file, &size, sizeof(int32_t));
		MyReadFile(file, &obj->anim_index, sizeof(int16_t));

		obj->frame_base = &frames[size / 2];
		obj->mesh_ptr = &meshes[mesh_index];
		obj->bone_ptr = &bones[bone_index];
		obj->loaded = 1;														// flag object as loaded
	}

	// initialise objects: must come after bones have been set up, but before items loaded

	InitialiseObjects();

	// load in static objects

	MyReadFile(file, &number_static_objects, sizeof(int32_t));

	static_objects = (STATIC_INFO*)game_malloc((NUMBER_STATIC_OBJECTS + number_static_objects) * sizeof(STATIC_INFO));

	for (int i = 0; i < number_static_objects; ++i)
	{
		int j;

		MyReadFile(file, &j, sizeof(int32_t));					// read static object number

		auto obj = &static_objects[j];

		MyReadFile(file, &obj->mesh_number, sizeof(int16_t));
		MyReadFile(file, &obj->x_minp, sizeof(int16_t) * 6);	// read physical bound info
		MyReadFile(file, &obj->x_minc, sizeof(int16_t) * 6);	// read collide bound info
		MyReadFile(file, &obj->flags, sizeof(int16_t));
	}

	return true;
}

bool LoadSprites(HANDLE file)
{
	int number;

	// Sprite info pointers

	MyReadFile(file, &number, sizeof(int32_t));
	MyReadFile(file, phdsprinfo, sizeof(PHDSPRITESTRUCT) * number);

	// Load object information (for non-static sprites)

	MyReadFile(file, &number, sizeof(int32_t));

	for (int i = 0; i < number; ++i)
	{
		// only some objects are represented in each wad: this is the object number

		int j;

		MyReadFile(file, &j, sizeof(int32_t));

		auto obj = &objects[j];

		// Wad file may contain information about static objects too (useful for editors,
		// but game code doesn't want to know) which needs skipping across

		if (j < NUMBER_OBJECTS)
		{
			// 'nmeshes' is a negative value for sprites, and stores number of animated frames

			MyReadFile(file, &obj->nmeshes, sizeof(int16_t));
			MyReadFile(file, &obj->mesh_ptr, sizeof(int16_t));

			obj->mesh_ptr = (int16_t**)(phdsprinfo + (uintptr_t)obj->mesh_ptr);
			obj->loaded = 1;
		}
		else
		{
			// NOTE: Following routine is just a hack to load in mesh number into
			// the STATIC_INFO structure - doesn't get bounding info or anything
			// load in data for STATIC_INFO structures
			
			j -= NUMBER_OBJECTS;

			SetFilePointer(file, sizeof(int16_t), nullptr, FILE_CURRENT);
			MyReadFile(file, &static_objects[j].mesh_number, sizeof(int16_t));
		}
	}

	return true;
}

int16_t create_custom_item(int16_t id)
{
	auto flare_item = CreateItem();

	if (flare_item != NO_ITEM)
	{
		auto flare = &items[flare_item];

		flare->object_number = id;
		flare->room_number = lara_item->room_number;
		flare->pos.x_pos = 48055;
		flare->pos.y_pos = 1024;
		flare->pos.z_pos = 23912;
		flare->room_number = 0;

		InitialiseItem(flare_item);

		flare->pos.z_rot = 0;
		flare->pos.y_rot = 0;
		flare->pos.x_rot = 0;
	}

	return flare_item;
}

bool LoadItems(HANDLE file)
{
	// allocate and load in all the starting positions of movable items from room data

	MyReadFile(file, &level_items, sizeof(int32_t));

	// actual final game will crash and burn if 'number_items' is 0, but may be during testing

	if (level_items == 0)
		return true;

	if (level_items > NUMBER_ITEMS)
	{
		prof::print(DARK_RED, "LoadItems(): Too Many Items being Loaded!!");
		return false;
	}

	g_attachments.clear();

	if (!(items = (ITEM_INFO*)game_malloc(sizeof(ITEM_INFO) * NUMBER_ITEMS, ITEMS)))
	{
		prof::print(DARK_RED, "LoadItems(): Unable to allocate memory for 'items'");
		return false;
	}

	InitialiseItemArray(NUMBER_ITEMS);

	for (int i = 0; i < level_items; ++i)
	{
		auto item = &items[i];

		MyReadFile(file, &item->object_number, sizeof(int16_t));
		MyReadFile(file, &item->room_number, sizeof(int16_t));
		MyReadFile(file, &item->pos.x_pos, sizeof(int32_t));
		MyReadFile(file, &item->pos.y_pos, sizeof(int32_t));
		MyReadFile(file, &item->pos.z_pos, sizeof(int32_t));
		MyReadFile(file, &item->pos.y_rot, sizeof(int16_t));
		MyReadFile(file, &item->shade, sizeof(int16_t));
		MyReadFile(file, &item->shadeB, sizeof(int16_t)); // two shades for item_info
		MyReadFile(file, &item->flags, sizeof(int16_t));

		if (item->object_number < 0 || item->object_number >= NUMBER_OBJECTS)
		{
			prof::print(DARK_RED, "LoadItems(): Bad Object number ({}) on Item {}", item->object_number, i);
			return false;
		}

		// NOTE: any extra memory allocation for item is done via InitialiseItem now

		InitialiseItem(i);
	}

	// loading custom object...

	if (true)
	{
		// we gonna test by copying the tr2 m16 (id 401)

		auto custom_meshes = (int16_t**)game_malloc(1024 * sizeof(int16_t*));

		auto old_obj_id = 62;
		auto old_obj = &objects[old_obj_id];

		auto obj_id = NUMBER_OBJECTS + 100;
		auto obj = &objects[obj_id];

		std::ifstream obj_file("banana.obj", std::ios::binary);

		int16_t num_meshes = 0;
		int32_t mesh_array_size = 0;
		int32_t mesh_texs_info_size = 0;
		int32_t pages_len = 0;

		obj_file.read((char*)&num_meshes, sizeof(num_meshes));

		int tex_info_added = 0;

		for (int i = 0; i < num_meshes; ++i)
		{
			obj_file.read((char*)&mesh_array_size, sizeof(mesh_array_size));

			auto new_mesh_ptr = (int16_t*)game_malloc(mesh_array_size);

			obj_file.read((char*)new_mesh_ptr, mesh_array_size);

			custom_meshes[i] = new_mesh_ptr;

			{
				auto curr_ptr = new_mesh_ptr;		curr_ptr += 5;

				auto mesh_data_size = *curr_ptr;	curr_ptr += 1 + mesh_data_size * 3;
				auto mesh_light_size = *curr_ptr;	curr_ptr += 1 + mesh_light_size * 3;
				auto gt4_faces_count = *curr_ptr;
				auto gt4_faces = curr_ptr + 1;		curr_ptr = gt4_faces + gt4_faces_count * 5;
				auto gt3_faces_count = *curr_ptr;
				auto gt3_faces = curr_ptr + 1;		curr_ptr = gt3_faces + gt3_faces_count * 4;
				auto g4_faces_count = *curr_ptr;
				auto g4_faces = curr_ptr + 1;		curr_ptr = g4_faces + g4_faces_count * 5;
				auto g3_faces_count = *curr_ptr;
				auto g3_faces = curr_ptr + 1;		curr_ptr = g3_faces + g3_faces_count * 4;

				obj_file.read((char*)&mesh_texs_info_size, sizeof(mesh_texs_info_size));

				auto texs_info = new PHDTEXTURESTRUCT[mesh_texs_info_size];
				auto text_info_dst = phdtextinfo + texture_infos + tex_info_added;
				auto text_info_base_index = texture_infos + tex_info_added;

				obj_file.read((char*)texs_info, mesh_texs_info_size * sizeof(PHDTEXTURESTRUCT));

				memcpy(text_info_dst, texs_info, mesh_texs_info_size * sizeof(PHDTEXTURESTRUCT));

				for (int i = 0; i < mesh_texs_info_size; ++i)
					text_info_dst[i].tpage += texture_pages_count;

				// fix face texture indices

				for (auto face = gt4_faces; face < gt4_faces + gt4_faces_count * 5; face += 5) face[4] += text_info_base_index;
				for (auto face = gt3_faces; face < gt3_faces + gt3_faces_count * 4; face += 4) face[3] += text_info_base_index;
				for (auto face = g4_faces; face < g4_faces + g4_faces_count * 5; face += 5) face[4] += text_info_base_index;
				for (auto face = g3_faces; face < g3_faces + g3_faces_count * 4; face += 4) face[3] += text_info_base_index;

				tex_info_added += mesh_texs_info_size;
			}
		}

		obj_file.read((char*)&pages_len, sizeof(pages_len));

		for (int i = 0; i < pages_len; ++i)
		{
			auto page_data = new uint16_t[256 * 256]();

			obj_file.read((char*)page_data, 256 * 256 * 2);

			auto handle = DXTextureAdd(256, 256, page_data, DXTextureList, 555);
			LanTextureHandle[texture_pages_count + i] = (handle >= 0 ? handle : -1);

			delete[] page_data;
		}

		HWR_GetAllTextureHandles();

		obj->nmeshes = num_meshes;
		//obj->mesh_ptr = old_obj->mesh_ptr;
		obj->bone_ptr = old_obj->bone_ptr;
		obj->mesh_ptr = &custom_meshes[0];
		//obj->bone_ptr = new_bone_ptr;
		obj->anim_index = 0;	// todo, we have to import the anim data and the only frame.
		//obj->anim_index = old_obj->anim_index;
		obj->initialise = nullptr;
		obj->collision = nullptr;
		obj->control = nullptr;
		obj->draw_routine = DrawAnimatingItem;
		obj->floor = obj->ceiling = nullptr;
		obj->pivot_length = 0;
		obj->radius = 100;
		obj->shadow_size = 0;
		obj->hit_points = DONT_TARGET;
		obj->intelligent = obj->water_creature = 0;
		obj->loaded = 1;	// we have to notify the server about this if we ever get this working

		create_custom_item(obj_id);
	}

	return true;
}

bool LoadDepthQ(HANDLE file)
{
	MyReadFile(file, depthq_table, 32 * 256);

	// force colour 0 to black

	for (int i = 0; i < 33; ++i)
		depthq_table[i][0] = 0;

	memcpy(depthq_table[32], depthq_table[24], 256);

	for (int i = 0; i < 32; ++i)
		for (int j = 0; j < 256; ++j)
			gouraud_table[j][i] = depthq_table[i][j];

	// calculate water palette

	for (int i = 0; i < 256 * 3; i += 3)
	{
		const int j = (int)((unsigned int)game_palette[i + 1] * 2) / 3;

		water_palette[i] = (int)((unsigned int)game_palette[i] * 2) / 3;
		water_palette[i + 1] = j;
		water_palette[i + 2] = game_palette[i + 2];
	}

	return true;
}

bool LoadPalette(HANDLE file)
{
	MyReadFile(file, game_palette, 256 * 3);

	// force colour 0 to black

	game_palette[0] = game_palette[1] = game_palette[2] = 0;

	for (int i = 3; i < 768; ++i)
		game_palette[i] <<= 2;

	MyReadFile(file, G_GouraudPalette, 256 * 4);

	return true;
}

bool LoadCameras(HANDLE file)
{
	DWORD number_cameras;

	MyReadFile(file, &number_cameras, sizeof(int32_t));

	if (number_cameras == 0)
		return true;

	if (!(camera.fixed = (OBJECT_VECTOR*)game_malloc(sizeof(OBJECT_VECTOR) * number_cameras, CAMERAS)))
		return false;

	MyReadFile(file, camera.fixed, sizeof(OBJECT_VECTOR) * number_cameras);

	return true;
}

bool LoadSoundEffects(HANDLE file)
{
	MyReadFile(file, &number_sound_effects, sizeof(int32_t));

	if (number_sound_effects == 0)
		return true;

	if (!(sound_effects = (OBJECT_VECTOR*)game_malloc(sizeof(OBJECT_VECTOR) * number_sound_effects, SOUND_FX)))
		return false;

	MyReadFile(file, sound_effects, sizeof(OBJECT_VECTOR) * number_sound_effects);

	return true;
}

bool LoadBoxes(HANDLE file)
{
	int32_t number_overlaps;
	DWORD read;

	MyReadFile(file, &number_boxes, sizeof(int32_t));
	boxes = (BOX_INFO*)game_malloc(sizeof(BOX_INFO) * number_boxes, BOXES);
	MyReadFile(file, boxes, sizeof(BOX_INFO) * number_boxes, &read);

	if (read != sizeof(BOX_INFO) * number_boxes)
	{
		prof::print(DARK_RED, "LoadBoxes(): Unable to load boxes");
		return false;
	}

	// box overlaps

	MyReadFile(file, &number_overlaps, sizeof(int32_t));
	overlap = (uint16_t*)game_malloc(sizeof(uint16_t) * number_overlaps, OVERLAPS);
	MyReadFile(file, overlap, sizeof(uint16_t) * number_overlaps, &read);

	if (read != sizeof(uint16_t) * number_overlaps)
	{
		prof::print(DARK_RED, "LoadBoxes(): Unable to load box overlaps");
		return false;
	}

	// zones (version for flipped/unflipped versions of map)

	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			ground_zone[j][i] = (int16_t*)game_malloc(sizeof(int16_t) * number_boxes, GROUNDZONE);
			MyReadFile(file, ground_zone[j][i], sizeof(int16_t) * number_boxes, &read);

			if (read != sizeof(int16_t) * number_boxes)
			{
				prof::print(DARK_RED, "LoadBoxes(): Unable to load 'ground_zone'");
				return false;
			}
		}

		fly_zone[i] = (int16_t*)game_malloc(sizeof(int16_t) * number_boxes, FLYZONE);
		MyReadFile(file, fly_zone[i], sizeof(int16_t) * number_boxes, &read);

		if (read != sizeof(int16_t) * number_boxes)
		{
			prof::print(DARK_RED, "LoadBoxes(): Unable to load 'fly_zone'");
			return false;
		}
	}

	return true;
}

bool LoadAnimatedTextures(HANDLE file)
{
	int size;

	MyReadFile(file, &size, sizeof(int32_t));

	anim_tex_ranges = (int16_t*)game_malloc(size * sizeof(int16_t), ANIMATING_TEXTURE_RANGES);

	MyReadFile(file, anim_tex_ranges, sizeof(int16_t) * size);

	return true;
}

bool LoadObjectsTextures(HANDLE file)
{
	MyReadFile(file, &texture_infos, sizeof(int32_t));
	MyReadFile(file, phdtextinfo, sizeof(PHDTEXTURESTRUCT) * texture_infos);

	return true;

	// (TESTING) I think the code from below is just useless xd

	for (int i = 0; i < texture_infos; ++i)
	{
		auto pUV = (uint16_t*)&(phdtextinfo[i].u1);

		uint8_t bByte = 0;

		for (int nBit = 0; nBit < 8; ++pUV, ++nBit)
		{
			if (*pUV & 0x80)
			{
				*pUV |= 0xff;
				bByte |= (1 << nBit);
			}
			else *pUV &= 0xff00;
		}

		LabTextureUVFlag[i] = bByte;
	}

	AdjustTextureUVs(true);

	return true;
}

int load_level_file(const char* filename)
{
	auto file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

	if (file == INVALID_HANDLE_VALUE)
	{
		prof::print(DARK_RED, "Could not open level '{}' {:#x}", filename, GetLastError());

#ifdef _DEBUG
		std::this_thread::sleep_for(std::chrono::seconds(5));
#endif

		return false;
	}

	// check version number

	int version, level_dummy;

	MyReadFile(file, &version, sizeof(int));

	// load palette

	SetFilePointer(file, 0, 0, FILE_CURRENT);

	if (!LoadPalette(file))
		return false;

	// load all texture pages

	SetFilePointer(file, 0, 0, FILE_CURRENT);

	if (!LoadTexturePages(file))
		return false;

	// now load level number and see if it is correct

	MyReadFile(file, &level_dummy, sizeof(int));

	// load rooms and associated data

	if (!LoadRooms(file))
		return false;

	// load objects

	if (!LoadObjects(file))
		return false;

	// load sprite infos and data

	if (!LoadSprites(file))
		return false;

	// load camera position data

	if (!LoadCameras(file))
		return false;

	// load sound effect emitter info

	if (!LoadSoundEffects(file))
		return false;

	// load box information just before very, very end of room wad

	if (!LoadBoxes(file))
		return false;

	// load ranges for room texture animation slightly after just before very, very end of room wad

	if (!LoadAnimatedTextures(file))
		return false;

	// load objects textures

	if (!LoadObjectsTextures(file))
		return false;

	// load non-static items from the end of the room wad

	if (!LoadItems(file))
		return false;

	// load depth cue table at very, very end of room wad

	SetFilePointer(file, 0, 0, FILE_CURRENT);

	if (!LoadDepthQ(file))
		return false;

	// ignore cinematic and demo data

	int tmp_size;

	MyReadFile(file, &tmp_size, sizeof(int16_t));

	if (tmp_size)
		SetFilePointer(file, sizeof(int16_t) * 8 * tmp_size, 0, FILE_CURRENT);

	MyReadFile(file, &tmp_size, sizeof(int16_t));

	if (tmp_size)
		SetFilePointer(file, tmp_size, 0, FILE_CURRENT);

	CloseHandle(file);

	return true;
}

bool load_level()
{
	g_chat->disable();
	g_debugger->disable();

	unload_level();

	auto random_load_pic = rand() % 2;

	FadePictureUp(random_load_pic);

	auto dispatch_input_and_net = [&]()
	{
		if (update_input())
			return false;

		g_ui->begin();
		g_client->dispatch_packets(16);
		g_ui->end();

		return true;
	};

	while (!g_client->can_load() && !g_client->is_downloading())
		dispatch_input_and_net();

	while (g_client->is_downloading())
		if (!dispatch_input_and_net())
			return false;

	auto level_filename = g_level->get_filename();

	prof::print(YELLOW, "Loading level '{}'... ", level_filename.c_str());

	g_level->set_as_loaded();

	if (!load_level_file(level_filename.c_str()))
		return false;

	FadePictureDown(random_load_pic);

#ifdef _DEBUG
	g_debugger->enable();
#endif

	g_chat->enable();

	return true;
}

void unload_level()
{
	g_resource->trigger_event(events::level::ON_LEVEL_UNLOAD);

	HWR_FreeTexturePages();

	texture_infos = 0;
	number_custom_anims = 0;

	g_attachments.clear();

	free_game_memory();
}

int	Read_Strings(int num_strings, char** strings, char** buffer, DWORD* read, HANDLE file)
{
	int16_t size;

	MyReadFile(file, LGF_offsets, num_strings * sizeof(int16_t), read, nullptr);
	MyReadFile(file, &size, sizeof(int16_t), read, nullptr);

	*buffer = (char*)GlobalAlloc(GMEM_FIXED, size);

	if (!buffer)
		return false;

	MyReadFile(file, *buffer, size, read, nullptr);

	if (gameflow.cyphered_strings)
	{
		auto strptr = *buffer;

		for (int i = 0; i < size; ++i)
			*(strptr + i) ^= gameflow.cypher_code;
	}

	// setup string ptrs

	for (int i = 0; i < num_strings; ++i)
		strings[i] = *buffer + LGF_offsets[i];

	return true;
}

bool load_gameflow(const char* filename)
{
	DWORD read;
	int16_t number, size;

#ifdef GAMEDEBUG
	prof::print(YELLOW, "GF:	Loading GameFlow Data");
#endif

	auto file = CreateFile(filename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
	{
		prof::print(RED, "GF:	ERROR: Could not open file '{}'", filename);
		return false;
	}

	// read script version number

	MyReadFile(file, &GF_ScriptVersion, sizeof(uint32_t));

	if (GF_ScriptVersion != THIS_SCRIPT_VERSION)
	{
		prof::print(RED, "GF:	ERROR: Incorrect script version {} ({})", GF_ScriptVersion, THIS_SCRIPT_VERSION);
		return false;
	}

	MyReadFile(file, GF_Description, 256);

	prof::print(YELLOW, GF_Description);

	// read in GAMEFLOW_INFO

	MyReadFile(file, &size, sizeof(int16_t));

	if (size != sizeof(GAMEFLOW_INFO))
		return false;

	MyReadFile(file, &gameflow, sizeof(GAMEFLOW_INFO));

#ifdef GAMEDEBUG
	prof::print(YELLOW, "GF:	Cyphered Strings:{}", gameflow.cyphered_strings);
	prof::print(YELLOW, "GF:	Cypher Code:{:#x}", gameflow.cypher_code);

	prof::print(YELLOW, "GF:	{} level(s)", gameflow.num_levels);
	prof::print(YELLOW, "GF:	{} pic file(s)", gameflow.num_picfiles);
	prof::print(YELLOW, "GF:	{} title file(s)", gameflow.num_titlefiles);
	prof::print(YELLOW, "GF:	{} fmv file(s)", gameflow.num_fmvfiles);
	prof::print(YELLOW, "GF:	{} cut file(s)", gameflow.num_cutfiles);
	prof::print(YELLOW, "GF:	{} demo(s)", gameflow.num_demos);
	prof::print(YELLOW, "GF:	Title Track: {}", gameflow.title_track);
	prof::print(YELLOW, "GF:	Single Playable: {}", gameflow.singlelevel);

	prof::print(YELLOW, "GF:	Title Enabled? {}", gameflow.title_disabled);
	prof::print(YELLOW, "GF:	Title Replace: {:#x}", gameflow.title_replace);
	prof::print(YELLOW, "GF:	First Option: {:#x}", gameflow.firstOption);
	prof::print(YELLOW, "GF:	OnDeath in demoplay: {:#x}", gameflow.ondeath_demo_mode);
	prof::print(YELLOW, "GF:	OnDeath in gameplay: {:#x}", gameflow.ondeath_ingame);
	prof::print(YELLOW, "GF:	OnDemo User Interrupt: {:#x}", gameflow.on_demo_interrupt);
	prof::print(YELLOW, "GF:	OnDemo End: {:#x}", gameflow.on_demo_end);
	prof::print(YELLOW, "GF:	NoInput time: {} seconds", gameflow.noinput_time / 30);
	prof::print(YELLOW, "GF:	Noinput TimeOut? {}", gameflow.noinput_timeout);

	prof::print(YELLOW, "GF:	CheatmodeCheck Disabled? {}", gameflow.cheatmodecheck_disabled);
	prof::print(YELLOW, "GF:	Load/Save Disabled? {}", gameflow.loadsave_disabled);
	prof::print(YELLOW, "GF:	ScreenSizing Disabled? {}", gameflow.screensizing_disabled);
	prof::print(YELLOW, "GF:	LockOut OptionRing? {}", gameflow.lockout_optionring);
	prof::print(YELLOW, "GF:	'Dozy' Cheat? {}", gameflow.dozy_cheat_enabled);
#endif

	// level name/filename strings

	if (!(GF_Level_Names = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Level_Names, &GF_levelnames_buffer, &read, file))
		return false;

	if (!(GF_picfilenames = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_picfiles * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_picfiles, GF_picfilenames, &GF_picfilenames_buffer, &read, file))
		return false;

	if (!(GF_titlefilenames = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_titlefiles * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_titlefiles, GF_titlefilenames, &GF_titlefilenames_buffer, &read, file))
		return false;

	if (!(GF_fmvfilenames = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_fmvfiles * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_fmvfiles, GF_fmvfilenames, &GF_fmvfilenames_buffer, &read, file))
		return false;

	if (!(GF_levelfilenames = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_levelfilenames, &GF_levelfilenames_buffer, &read, file))
		return false;

	if (!(GF_cutscenefilenames = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_cutfiles * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_cutfiles, GF_cutscenefilenames, &GF_cutscenefilenames_buffer, &read, file))
		return false;

	// read-in offsets into seq buffer for each sequence

	MyReadFile(file, &LGF_offsets, sizeof(int16_t) * (gameflow.num_levels + 1));
	MyReadFile(file, &size, sizeof(int16_t));

	// read-in sequence data

	if (!(GF_sequence_buffer = (int16_t*)GlobalAlloc(GMEM_FIXED, size)))
		return false;

	MyReadFile(file, GF_sequence_buffer, size);

	// frontend sequence

	for (int i = 0; i < gameflow.num_levels; ++i)
		GF_level_sequence_list[i] = GF_sequence_buffer + LGF_offsets[i + 1] / 2;

	// read level number that are actually demos

	if (gameflow.num_demos)
	{
		int16_t dummy[24];
		MyReadFile(file, dummy, sizeof(int16_t) * gameflow.num_demos);
	}

	// GameText strings

	MyReadFile(file, &number, sizeof(int16_t));

	if (number != GT_NUM_GAMESTRINGS)
		return false;

	if (!(GF_GameStrings = (char**)GlobalAlloc(GMEM_FIXED, GT_NUM_GAMESTRINGS * sizeof(char*))))
		return false;

	if (!Read_Strings(GT_NUM_GAMESTRINGS, GF_GameStrings, &GF_GameStrings_buffer, &read, file))
		return false;

	if (!(GF_PCStrings = (char**)GlobalAlloc(GMEM_FIXED, PCSTR_NUM_STRINGS * sizeof(char*))))
		return false;

	if (!Read_Strings(PCSTR_NUM_STRINGS, GF_PCStrings, &GF_PCStrings_buffer, &read, file))
		return false;

	// puzzle1 strings
	
	if (!(GF_Puzzle1Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Puzzle1Strings, &GF_Puzzle1Strings_buffer, &read, file))
		return false;

	// puzzle2 strings

	if (!(GF_Puzzle2Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Puzzle2Strings, &GF_Puzzle2Strings_buffer, &read, file))
		return false;

	// puzzle3 strings

	if (!(GF_Puzzle3Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Puzzle3Strings, &GF_Puzzle3Strings_buffer, &read, file))
		return false;

	// puzzle4 strings

	if (!(GF_Puzzle4Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Puzzle4Strings, &GF_Puzzle4Strings_buffer, &read, file))
		return false;
	
	// pickup1 strings

	if (!(GF_Pickup1Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Pickup1Strings, &GF_Pickup1Strings_buffer, &read, file))
		return false;

	// pickup2 strings

	if (!(GF_Pickup2Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Pickup2Strings, &GF_Pickup2Strings_buffer, &read, file))
		return false;

	// key1 strings

	if (!(GF_Key1Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Key1Strings, &GF_Key1Strings_buffer, &read, file))
		return false;

	// key2 strings

	if (!(GF_Key2Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Key2Strings, &GF_Key2Strings_buffer, &read, file))
		return false;

	// key3 strings

	if (!(GF_Key3Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Key3Strings, &GF_Key3Strings_buffer, &read, file))
		return false;

	// key4 strings

	if (!(GF_Key4Strings = (char**)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char*))))
		return false;

	if (!Read_Strings(gameflow.num_levels, GF_Key4Strings, &GF_Key4Strings_buffer, &read, file))
		return false;

	CloseHandle(file);

	return true;
}