#include <windows.h>
#include <iostream>
#include <fstream>

struct tr_model  // 18 bytes
{
	uint32_t ID;           // Type Identifier (matched in Entities[])
	uint16_t NumMeshes;    // Number of meshes in this object
	uint16_t StartingMesh; // Stating mesh (offset into MeshPointers[])
	uint32_t MeshTree;     // Offset into MeshTree[]
	uint32_t FrameOffset;  // Byte offset into Frames[] (divide by 2 for Frames[i])
	uint16_t Animation;    // Offset into Animations[]
};

struct ANIM_STRUCT
{
	int16_t* frame_ptr;
	int16_t interpolation;
	int16_t current_anim_state;
	int32_t velocity;
	int32_t acceleration;
	int16_t frame_base;
	int16_t frame_end;
	int16_t jump_anim_num;
	int16_t jump_frame_num;
	int16_t number_changes;
	int16_t change_index;
	int16_t number_commands;
	int16_t command_index;
};

struct CHANGE_STRUCT
{
	int16_t goal_anim_state;
	int16_t number_ranges;
	int16_t range_index;
};

struct RANGE_STRUCT
{
	int16_t start_frame;
	int16_t end_frame;
	int16_t link_anim_num;
	int16_t link_frame_num;
};

struct BOUNDING_BOX
{
	int16_t MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
};

struct ANIM_FRAME
{
	BOUNDING_BOX box;
	int16_t off_x, off_y, off_z;
	uint16_t* angle_sets;
};

struct ITEM_INFO;
struct COLL_INFO;

struct OBJECT_INFO
{
	int16_t nmeshes;
	int16_t** mesh_ptr;
	int32_t* bone_ptr;
	int16_t* frame_base;
	void (*initialise)(int16_t item_number);
	void (*control)(int16_t item_number);
	void (*floor)(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
	void (*ceiling)(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
	void (*draw_routine)(ITEM_INFO* item);
	void (*collision)(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
	int16_t anim_index;
	int16_t hit_points;
	int16_t pivot_length;		// replacement for 'head_size': distance from centre to neck rotation
	int16_t radius;
	int16_t shadow_size;			// size of shadow ( -1 if none )
	uint16_t bite_offset;			// offset into table of BITE_INFO structures for enemies that fire weapons. (Table in DRAW.C, set up bite_offset in SETUP.C).
	uint16_t loaded;			// is this object loaded on this level
	uint16_t intelligent;		// does this object need AI info??
	uint16_t non_lot;			// does this creature not use LOT system (e.g. Compys)
	uint16_t semi_transparent;	// is sprite object semi transparent
	uint16_t water_creature;	// is this is water based baddie? needed for SFX in shallow water
};

std::ifstream g_level;
std::ofstream g_out_obj;

uint32_t g_offset = 0;

template <typename T>
T read(uint32_t off = g_offset)
{
	T value;
	g_level.seekg(off);
	g_level.read((char*)&value, sizeof(T));
	g_offset += sizeof(T);
	return value;
}

template <typename T>
void read(void* data, int elements, uint32_t off = g_offset)
{
	auto size = elements * sizeof(T);
	g_level.seekg(off);
	g_level.read((char*)data, size);
	g_offset += size;
}

void write(void* data, int size)
{
	g_out_obj.write((char*)data, size);
}

int main(int argc, char** argv)
{
	g_level = std::ifstream(argc > 1 ? argv[1] : "level.tr2", std::ios::binary);
	if (!g_level)
		return MessageBoxA(nullptr, "Could not open level file", "Error", MB_ICONERROR);

	g_offset += 4;		// Version
	g_offset += 768;	// Palette
	g_offset += 1024;	// Palette16

	auto num_textiles = read<int32_t>();

	g_offset += num_textiles * 65536;	// Textile8
	g_offset += num_textiles * 131072;	// Textile16

	g_offset += 4;	// Unused

	auto num_rooms = read<int16_t>();

	for (int i = 0; i < num_rooms; ++i)
	{
		g_offset += 16;	// info

		{	// tr_room_data
			auto NumDataWords = read<int32_t>();

			g_offset += NumDataWords * sizeof(uint16_t);
		}

		auto NumPortals = read<int16_t>();

		g_offset += NumPortals * 32;

		auto NumZsectors = read<int16_t>();
		auto NumXsectors = read<int16_t>();

		g_offset += NumZsectors * NumXsectors * 8;	// NumXsectors

		g_offset += 2;	// AmbientIntensity
		g_offset += 2;	// LightMode

		auto NumLights = read<int16_t>();

		g_offset += NumLights * 24;	// Lights

		auto NumStaticMeshes = read<int16_t>();

		g_offset += NumStaticMeshes * 18;	// StaticMeshes

		g_offset += 2;	// AlternateRoom
		g_offset += 2;	// Flags
		g_offset += 1;	// WaterScheme
		g_offset += 1;	// ReverbInfo
		g_offset += 1;	// Filler
	}

	auto NumFloorData = read<int32_t>();

	g_offset += NumFloorData * 2;	// FloorData

	// MESHES

	auto NumMeshData = read<int32_t>();
	auto meshes_base = new int16_t[NumMeshData]();

	read<int16_t>(meshes_base, NumMeshData);

	// MESH POINTERS

	auto NumMeshPointers = read<int32_t>();
	auto meshes = new int16_t*[NumMeshPointers]();

	read<int16_t*>(meshes, NumMeshPointers);

	// ANIMATIONS

	auto NumAnimations = read<int32_t>();
	auto animations = new ANIM_STRUCT[NumAnimations]();

	read<ANIM_STRUCT>(animations, NumAnimations);

	// STATE CHANGES

	auto NumStateChanges = read<int32_t>();
	auto changes = new CHANGE_STRUCT[NumStateChanges]();

	read<CHANGE_STRUCT>(changes, NumStateChanges);

	// RANGES

	auto NumAnimDispatches = read<int32_t>();
	auto ranges = new RANGE_STRUCT[NumAnimDispatches]();

	read<RANGE_STRUCT>(ranges, NumAnimDispatches);

	// COMMANDS

	auto NumAnimCommands = read<int32_t>();
	auto commands = new int16_t[NumAnimCommands]();

	read<int16_t>(commands, NumAnimCommands);

	// BONES

	auto NumMeshTrees = read<int32_t>();
	auto bones = new int32_t[NumMeshTrees]();

	read<int32_t>(bones, NumMeshTrees);

	// FRAMES

	auto NumFrames = read<int32_t>();
	auto frames = new int16_t[NumFrames]();

	read<int16_t>(frames, NumFrames);

	// MODELS (used to get frame base or something)

	auto NumModels = read<int32_t>();
	auto objects = new OBJECT_INFO[1024]();

	// read tr_model here

	for (int i = 0; i < NumModels; ++i)
	{
		auto id = read<int32_t>();
		auto num_meshes = read<int16_t>();
		auto mesh_index = read<int16_t>();
		auto bone_index = read<int32_t>();

		auto size = read<int32_t>();
		auto anim_index = read<int16_t>();

		auto obj = &objects[id];

		obj->nmeshes = num_meshes;
		obj->anim_index = anim_index;
		obj->frame_base = &frames[size / 2];
		obj->mesh_ptr = &meshes[mesh_index];
		obj->bone_ptr = &bones[bone_index];
		obj->loaded = 1;
	}

	for (int i = 0; i < NumMeshPointers; ++i)
		meshes[i] = meshes_base + (int)meshes[i] / 2;

	// remap anim pointers

	for (int i = 0; i < NumAnimations; ++i)
	{
		auto anim = &animations[i];

		anim->frame_ptr = &frames[int32_t(anim->frame_ptr) / 2];
	}

	// close level and create obj file

	g_level.close();

	char option = 'y';

	while (option == 'y')
	{
		int16_t obj_id = -1;

		std::cout << "Object ID: ";

		//std::cin >> obj_id;

		//if (obj_id == -1)
		//	return 0;

		std::cout << "Output File Name: ";

		std::string obj_name;

		//std::cin >> obj_name;

		if (obj_name.empty())
			obj_name = "default";

		obj_name += ".obj";

		g_out_obj = std::ofstream("..\\patch\\" + obj_name, std::ios::binary | std::ios::trunc);

		obj_id = 353;

		auto obj = &objects[obj_id];

		auto process_mesh_array = [&](int16_t* mesh_ptr, int32_t* mesh_size)
		{
			auto curr_ptr = mesh_ptr;			curr_ptr += 5;

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

			// reset faces texture info

			for (auto face = gt4_faces; face < gt4_faces + gt4_faces_count * 5; face += 5) face[4] = 0;
			for (auto face = gt3_faces; face < gt3_faces + gt3_faces_count * 4; face += 4) face[3] = 0;
			for (auto face = g4_faces; face < g4_faces + g4_faces_count * 5; face += 5)	face[4] = 0;
			for (auto face = g3_faces; face < g3_faces + g3_faces_count * 4; face += 4)	face[3] = 0;

			auto size = *mesh_size = (curr_ptr - mesh_ptr) * sizeof(int16_t);
			auto mesh_data = new uint8_t[size]();

			memcpy(mesh_data, mesh_ptr, size);

			return mesh_data;
		};

		write(&obj->nmeshes, sizeof(obj->nmeshes));

		auto mesh_array = new int16_t*[obj->nmeshes]();

		for (int i = 0; i < obj->nmeshes; ++i)
		{
			int32_t mesh_array_size = 0;

			auto mesh_array = process_mesh_array(obj->mesh_ptr[i], &mesh_array_size);

			write(&mesh_array_size, sizeof(mesh_array_size));
			write(mesh_array, mesh_array_size);
		}

		/*auto obj_anim_index = objects[obj_id].anim_index;
		auto target_obj_anim_index = objects[obj_target_id].anim_index;
		auto anim = &animations[obj_anim_index + anim_id];
		auto size_of_frame = int16_t(sizeof(ANIM_STRUCT) + (objects[obj_id].num_meshes * sizeof(int16_t) * 2) - sizeof(int16_t));
		auto anim_len = int16_t((anim->frame_end - anim->frame_base) + 1);
		auto next_command_index = anim_id == NumAnimations - 1 ? NumAnimCommands : animations[anim_id + 1].command_index;
		auto commands_len = int16_t((next_command_index - animations[anim_id].command_index) + 1);

		anim->jump_anim_num = target_obj_anim_index + (anim->jump_anim_num - obj_anim_index);
		anim->jump_frame_num = objects[obj_target_id].frame_base + anim->jump_frame_num;

		write(anim, sizeof(ANIM_STRUCT));							// write info
		write(&size_of_frame, sizeof(size_of_frame));				// write size of frame
		write(&anim_len, sizeof(anim_len));							// write anim length
		write(&commands_len, sizeof(commands_len));					// write command length
		write(anim->frame_ptr, anim_len * size_of_frame);			// write frame data
		write(&commands[anim->command_index], commands_len);		// write command data*/

		g_out_obj.close();

		std::cout << "Object " << obj_id << " exported to file " << obj_name << std::endl;
		std::cout << "Export more? (y/n)" << std::endl;

		std::cin >> option;
	}

	// gl to free all the pools :risitas: (let windows kernel do its job)

	return 0;
}