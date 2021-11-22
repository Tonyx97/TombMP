import prof;

#include "standard.h"
#include "global.h"
#include "init.h"
#include "output.h"

#include <3dsystem/3d_gen.h>

#include <specific/fn_stubs.h>

std::unordered_set<void*> g_game_mem;

unsigned short GetRandom(WATERTAB* w, int n)
{
	do {
		const uint16_t random = rand() & 0xfc;

		int i = 0;

		for (; i < n; i++)
			if (w[i].random == random)
				break;

		if (i == n)
			return random;

	} while (true);

	return 0;
}

void init_water_table()
{
	srand(121197);

	for (int i = 0; i < 64; ++i)
	{
		short sin = m_sin(i << 7);

		// underwater/swamp table

		WaterTable[0][i].shimmer = (sin * 63) >> (12 + 3);
		WaterTable[0][i].choppy = (sin * 16) >> 12;
		WaterTable[0][i].random = (uint8_t)GetRandom(&WaterTable[0][0], i);
		WaterTable[0][i].abs = 0;

		// mist tables

		WaterTable[1][i].shimmer = (sin * 32) >> (12 + 3);
		WaterTable[1][i].choppy = 0;
		WaterTable[1][i].random = (uint8_t)GetRandom(&WaterTable[1][0], i);
		WaterTable[1][i].abs = -3;

		WaterTable[2][i].shimmer = (sin * 64) >> (12 + 3);
		WaterTable[2][i].choppy = 0;
		WaterTable[2][i].random = (uint8_t)GetRandom(&WaterTable[2][0], i);
		WaterTable[2][i].abs = 0;

		WaterTable[3][i].shimmer = (sin * 96) >> (12 + 3);
		WaterTable[3][i].choppy = 0;
		WaterTable[3][i].random = (uint8_t)GetRandom(&WaterTable[3][0], i);
		WaterTable[3][i].abs = 4;

		WaterTable[4][i].shimmer = (sin * 127) >> (12 + 3);
		WaterTable[4][i].choppy = 0;
		WaterTable[4][i].random = (uint8_t)GetRandom(&WaterTable[4][0], i);
		WaterTable[4][i].abs = 8;

		// shimmer/ripple tables

		for (int j = 0, k = 5; j < 4; ++j, k += 4)
		{
			for (int l = 0; l < 4; ++l)
			{
				static unsigned char off[4] = { 4, 8, 12, 16 };
				static short shim[4] = { 31, 63, 95, 127 };
				static short chop[4] = { 16, 53, 90, 127 };

				WaterTable[k + l][i].shimmer = -((sin * shim[l]) >> 15);
				WaterTable[k + l][i].choppy = (sin * chop[j]) >> 12;
				WaterTable[k + l][i].random = (uint8_t)GetRandom(&WaterTable[k + l][0], i);
				WaterTable[k + l][i].abs = off[l];
			}
		}
	}
}

void S_InitialiseSystem()
{
	S_SeedRandom();

	auto [sw, sh] = g_window->get_resolution();

	game_setup.dump_x = 0;
	game_setup.dump_y = 0;
	game_setup.dump_width = sw;
	game_setup.dump_height = sh;

	init_water_table();

	CalculateWibbleTable();

	InitZTable();
	InitUVTable();

	for (int i = 0; i < 1024; ++i)
		SqrTable[i] = (int)sqrt((float)i);

	for (int i = 1; i < 33; ++i)
	{
		for (int j = 1; j < 33; ++j)
		{
			for (int k = 1; k < 33; ++k)
			{
				RColorTable[i][j][k] = (((k * (i - j)) / i)) << 10;
				GColorTable[i][j][k] = (((k * (i - j)) / i)) << 5;
				BColorTable[i][j][k] = (((k * (i - j)) / i));
			}
		}
	}
}

void free_game_memory()
{
	for (auto ptr : g_game_mem)
		free(ptr);

	g_game_mem.clear();
}

void* game_malloc(int size, int type)
{
	auto ptr = calloc(1, size);

	g_game_mem.insert(ptr);

	return ptr;
}

void game_free(void* ptr, int type)
{
	if (auto it = g_game_mem.find(ptr); it != g_game_mem.end())
	{
		free(ptr);

		g_game_mem.erase(it);
	}
}

void CalculateWibbleTable()
{
	// calculate the wibbling for the water effect

	for (int i = 0; i < WIBBLE_SIZE; ++i)
	{
		auto sn = phd_sin((i * 65536) / WIBBLE_SIZE);

		wibble_table[i] = (float)(sn * MAX_WIBBLE >> W2V_SHIFT);
		shade_table[i] = sn * MAX_SHADE >> W2V_SHIFT;
		rand_table[i] = (GetRandomDraw() >> 5) - 0x01ff;

		// wibble tables for dynamic lighting of rooms
		for (int j = 0; j < 31; ++j)
			wibble_light[i][j] = ((j - 16) * i << 9) / (WIBBLE_SIZE - 1);
	}
}

void S_SeedRandom()
{
	auto lt = time(nullptr);
	tm tm;

	localtime_s(&tm, &lt);

	SeedRandomControl(tm.tm_sec + (tm.tm_min * 57) + (tm.tm_hour * 3543));
	SeedRandomDraw(tm.tm_sec + (tm.tm_min * 43) + (tm.tm_hour * 3477));
}