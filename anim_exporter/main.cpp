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

std::ifstream g_level;
std::ofstream g_out_anim;

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
	g_out_anim.write((char*)data, size);
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

	auto NumMeshData = read<int32_t>();

	g_offset += NumMeshData * 2; // mesh_base

	auto NumMeshPointers = read<int32_t>();

	g_offset += NumMeshPointers * 4;	// MeshPointers / meshes

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
	
	// IGNORING BONES (MeshTree) HERE

	auto NumMeshTrees = read<int32_t>();

	g_offset += NumMeshTrees * 4;	// MeshTrees

	// FRAMES

	auto NumFrames = read<int32_t>();

	auto frames = new int16_t[NumFrames]();

	read<int16_t>(frames, NumFrames);

	// MODELS (used to get frame base or something)

	auto NumModels = read<int32_t>();

	// read tr_model here

	struct
	{
		int16_t num_meshes;
		int16_t anim_index;
		int16_t frame_base;
	} objects[1000];

	for (int i = 0; i < NumModels; ++i)
	{
		auto id = read<int32_t>();
		auto num_meshes = read<int16_t>();
		auto mesh_index = read<int16_t>();
		auto bone_index = read<int32_t>();

		auto size = read<int32_t>();
		auto anim_index = read<int16_t>();

		objects[id] = { .num_meshes = num_meshes, .anim_index = anim_index, .frame_base = int16_t(size) };
	}

	// remap anim pointers

	for (int i = 0; i < NumAnimations; ++i)
	{
		auto anim = &animations[i];

		anim->frame_ptr = (int16_t*)((uintptr_t)frames + (uintptr_t)anim->frame_ptr);
	}

	// close level and create anim file

	g_level.close();

	char option = 'y';

	while (option == 'y')
	{
		int16_t obj_id = -1,
				obj_target_id = -1,
				anim_id = -1;

		std::cout << "Anim Object Location: ";

		std::cin >> obj_id;

		if (obj_id == -1)
			return 0;

		std::cout << "To Relative Object: ";

		std::cin >> obj_target_id;

		if (obj_target_id == -1)
			return 0;

		std::cout << "Anim ID: ";

		std::cin >> anim_id;

		if (anim_id == -1)
			return 0;

		std::cout << "Output File Name: ";

		std::string anim_name;

		std::cin >> anim_name;

		if (anim_name.empty())
			anim_name = "default";

		anim_name += ".anim";

		g_out_anim = std::ofstream(anim_name, std::ios::binary | std::ios::trunc);

		auto obj_anim_index = objects[obj_id].anim_index;
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
		write(&commands[anim->command_index], commands_len);		// write command data

		g_out_anim.close();

		std::cout << "Animation " << anim_id << " exported to file " << anim_name << std::endl;
		std::cout << "Export more? (y/n)" << std::endl;

		std::cin >> option;
	}

	// gl to free all the pools :risitas: (let windows kernel do its job)

	return 0;
}