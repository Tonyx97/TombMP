#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>

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

struct PHDTEXTURESTRUCT
{
	uint16_t drawtype,
			 tpage,
			 u1, v1,
			 u2, v2,
			 u3, v3,
			 u4, v4;
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
const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

void generateBitmapImage(unsigned char* image, int width, int height, char* imageFileName)
{
	int widthInBytes = width * BYTES_PER_PIXEL;

	unsigned char padding[3] = { 0, 0, 0 };
	int paddingSize = (4 - (widthInBytes) % 4) % 4;

	int stride = (widthInBytes)+paddingSize;

	FILE* imageFile;
	
	fopen_s(&imageFile, imageFileName, "wb");

	unsigned char* fileHeader = createBitmapFileHeader(height, stride);
	fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

	unsigned char* infoHeader = createBitmapInfoHeader(height, width);
	fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

	int i;
	for (i = 0; i < height; i++) {
		fwrite(image + (i * widthInBytes), BYTES_PER_PIXEL, width, imageFile);
		fwrite(padding, 1, paddingSize, imageFile);
	}

	fclose(imageFile);
}

unsigned char* createBitmapFileHeader(int height, int stride)
{
	int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

	static unsigned char fileHeader[] = {
		0,0,     /// signature
		0,0,0,0, /// image file size in bytes
		0,0,0,0, /// reserved
		0,0,0,0, /// start of pixel array
	}; 
	fileHeader[0] = (unsigned char)('B');
	fileHeader[1] = (unsigned char)('M');
	fileHeader[2] = (unsigned char)(fileSize);
	fileHeader[3] = (unsigned char)(fileSize >> 8);
	fileHeader[4] = (unsigned char)(fileSize >> 16);
	fileHeader[5] = (unsigned char)(fileSize >> 24);
	fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

	return fileHeader;
}

unsigned char* createBitmapInfoHeader(int height, int width)
{
	static unsigned char infoHeader[] = {
		0,0,0,0, /// header size
		0,0,0,0, /// image width
		0,0,0,0, /// image height
		0,0,     /// number of color planes
		0,0,     /// bits per pixel
		0,0,0,0, /// compression
		0,0,0,0, /// image size
		0,0,0,0, /// horizontal resolution
		0,0,0,0, /// vertical resolution
		0,0,0,0, /// colors in color table
		0,0,0,0, /// important color count
	};

	infoHeader[0] = (unsigned char)(INFO_HEADER_SIZE);
	infoHeader[4] = (unsigned char)(width);
	infoHeader[5] = (unsigned char)(width >> 8);
	infoHeader[6] = (unsigned char)(width >> 16);
	infoHeader[7] = (unsigned char)(width >> 24);
	infoHeader[8] = (unsigned char)(height);
	infoHeader[9] = (unsigned char)(height >> 8);
	infoHeader[10] = (unsigned char)(height >> 16);
	infoHeader[11] = (unsigned char)(height >> 24);
	infoHeader[12] = (unsigned char)(1);
	infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL * 8);

	return infoHeader;
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

	auto texture_pages = new uint8_t[num_textiles * 131072]();

	read<uint8_t>(texture_pages, num_textiles * 131072);

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

	// IGNORE STATIC OBJS

	auto NumSpriteTextures = read<int32_t>();

	g_offset += NumSpriteTextures * 32;	// StaticMeshes

	// IGNORE SPRITES

	auto NumStaticMeshes = read<int32_t>();

	g_offset += NumStaticMeshes * 16;	// SpriteTextures

	// IGNORE DYNAMIC SPRITES

	auto NumSpriteSequences = read<int32_t>();

	g_offset += NumSpriteSequences * 8;	// SpriteSequences

	// IGNORE CAMERAS

	auto NumCameras = read<int32_t>();

	g_offset += NumCameras * 16;	// Cameras

	// IGNORE SOUNDS

	auto NumSoundSources = read<int32_t>();

	g_offset += NumSoundSources * 16;	// SoundSources

	// IGNORE BOXES

	auto NumBoxes = read<int32_t>();

	g_offset += NumBoxes * 8;	// Boxes

	// IGNORE OVERLAPS

	auto NumOverlaps = read<int32_t>();

	g_offset += NumOverlaps * 2;	// Overlaps

	// IGNORE OVERLAPS

	g_offset += NumBoxes * 20;		// Zones

	// IGNORE ANIMATED TEXTURES

	auto NumAnimatedTextures = read<int32_t>();

	g_offset += NumAnimatedTextures * 2;	// AnimatedTextures

	// OBJECT TEXTURES INFO

	auto NumObjectTextures = read<int32_t>();
	auto text_info = new PHDTEXTURESTRUCT[NumObjectTextures]();

	read<PHDTEXTURESTRUCT>(text_info, NumObjectTextures);

	// adjust meshes pointers

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

		obj_id = 232;

		auto obj = &objects[obj_id];

		int generated_textures = 0;

		/*for (int i = 0; i < num_textiles; ++i)
		{
			std::vector<uint8_t> page_data;

			auto page_base = texture_pages + (i * 256 * 256 * 2);

			for (int y = 0; y < 256; ++y)
				for (int x = 0; x < 256; ++x)
				{
					auto pixel = (int16_t*)page_base + x + (256 - y) * 256;

					uint8_t r = (*pixel >> 7) & 0xf8;
					uint8_t g = (*pixel >> 2) & 0xf8;
					uint8_t b = (*pixel << 3) & 0xf8;

					page_data.push_back(b);
					page_data.push_back(g);
					page_data.push_back(r);
				}

			generateBitmapImage((unsigned char*)page_data.data(), 256, 256, (char*)("pages\\page_" + std::to_string(generated_textures++) + ".bmp").c_str());
		}*/

		float UVTable[65536];

		for (int i = 0; i < 65536; ++i)
			UVTable[i] = (float)(i + 1) * (1.f / 65536.f);

		auto get_texture_page = [&](int page)
		{
			return (int16_t*)texture_pages + (page * 256 * 256);
		};

		auto get_texture_part = [&](int16_t* page, int offset)
		{
			return &page[offset];
		};

		auto fix_and_gen_texture_part = [&](int16_t* face, bool gt4 = true)
		{
			auto texture_info = text_info + face[gt4 ? 4 : 3];
			auto page = get_texture_page(texture_info->tpage);

			auto u1_off = texture_info->u1,
				 v1_off = texture_info->v1,
				 u2_off = texture_info->u2,
				 v2_off = texture_info->v2,
				 u3_off = texture_info->u3,
				 v3_off = texture_info->v3,
				 u4_off = texture_info->u4,
				 v4_off = texture_info->v4;

			int16_t u1 = (u1_off / 256),
					v1 = (v1_off / 256),
					u2 = (u2_off / 256),
					v2 = (v2_off / 256),
					u3 = (u3_off / 256),
					v3 = (v3_off / 256),
					u4 = (u4_off / 256),
					v4 = (v4_off / 256);

			std::cout << "page(" << texture_info->tpage << ") "
					  << "u1(" << u1 << ") v1(" << v1 << ") | "
					  << "u2(" << u2 << ") v2(" << v2 << ") | "
					  << "u3(" << u3 << ") v3(" << v3 << ") | "
					  << "u4(" << u4 << ") v4(" << v4 << ")" << std::endl;

			int width = std::abs(u1 - u2),
				height = std::abs(v1 - v4);

			auto tu = UVTable[u1_off];
			auto tv = UVTable[v1_off];

			std::cout << "width: " << width << " height: " << height << std::endl;

			auto texture_start = get_texture_part(page, 0);

			std::vector<uint8_t> face_texture;

			int sx = 256,
				sy = 256;

			{
				auto row_id = u1;
				auto column_id = v1 + 3;

				auto row_start = u1_off & ~255;
				auto col_start = v1_off & ~255;

				auto pixel = &page[0];

				for (int i = 0; i < 256; ++i)
				{
					for (int j = 0; j < 256; ++j)
					{
						//pixel = &page[-(row_id * 256) - (column_id * 256) + j + i * 256];

						uint8_t r = (*pixel >> 7) & 0xf8;
						uint8_t g = (*pixel >> 2) & 0xf8;
						uint8_t b = (*pixel << 3) & 0xf8;

						face_texture.push_back(b);
						face_texture.push_back(g);
						face_texture.push_back(r);

						++pixel;
					}

					//pixel += 256;
				}
			}

			/*{
				for (int y = 256 - 1; y >= 0; --y)
				//for (int y = 0; y < 256; ++y)
					for (int x = 256 - 1; x >= 0; --x)
					//for (int x = 0; x < 256; ++x)
					{
						auto pixel = texture_start + x + y * 256;

						uint8_t r = (*pixel >> 7) & 0xf8;
						uint8_t g = (*pixel >> 2) & 0xf8;
						uint8_t b = (*pixel << 3) & 0xf8;

						face_texture.push_back(b);
						face_texture.push_back(g);
						face_texture.push_back(r);
					}
			}*/

			generateBitmapImage((unsigned char*)face_texture.data(), sx, sy, (char*)("mesh_" + std::to_string(generated_textures++) + ".bmp").c_str());

			/*{
				for (int y = 256 - 1; y >= 0; --y)
					for (int x = 0; x < 256; ++x)
					{
						auto pixel = texture_start + x + y * 256;

						uint8_t r = (*pixel >> 7) & 0xf8;
						uint8_t g = (*pixel >> 2) & 0xf8;
						uint8_t b = (*pixel << 3) & 0xf8;

						face_texture.push_back(b);
						face_texture.push_back(g);
						face_texture.push_back(r);
					}

				generateBitmapImage((unsigned char*)face_texture.data(), 256, 256, (char*)("test_mesh_" + std::to_string(generated_textures++) + ".bmp").c_str());
			}*/
		};

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

			int gt4_pixels_count = 0;

			for (auto face = gt4_faces; face < gt4_faces + gt4_faces_count * 5; face += 5)	fix_and_gen_texture_part(face);
			for (auto face = gt3_faces; face < gt3_faces + gt3_faces_count * 4; face += 4)	fix_and_gen_texture_part(face, false);
			for (auto face = g4_faces; face < g4_faces + g4_faces_count * 5; face += 5)		fix_and_gen_texture_part(face);
			for (auto face = g3_faces; face < g3_faces + g3_faces_count * 4; face += 4)		fix_and_gen_texture_part(face, false);

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

		//std::cin >> option;
		break;
	}

	ShellExecuteA(nullptr, nullptr, "mesh_2.bmp", nullptr, nullptr, SW_HIDE);

	// gl to free all the pools :risitas: (let windows kernel do its job)

	return 0;
}