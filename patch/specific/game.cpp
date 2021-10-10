#include "standard.h"
#include "global.h"
#include "picture.h"
#include "frontend.h"
#include "file.h"
#include "input.h"
#include "output.h"
#include "global.h"

#include <game/invdata.h>
#include <game/health.h> 
#include <game/invfunc.h>
#include <game/inventry.h>
#include <game/gameflow.h>
#include <game/game.h>
#include <game/setup.h>
#include <game/camera.h>
#include <game/control.h>
#include <game/objects.h>
#include <game/lara.h>

#include <3dsystem/3d_gen.h>

uint32_t StartLevel = LV_FIRSTLEVEL;

int save_counter = 0;

static int32_t rand_1 = 0xd371f947;
static int32_t rand_2 = 0xd371f947;

int32_t GetRandomControl()
{
	rand_1 = rand_1 * 0x41c64e6d + 0x3039;
	return ((rand_1 >> 10) & 0x7fff);
}

void SeedRandomControl(int32_t seed)
{
	rand_1 = seed;
}

int32_t GetRandomDraw()
{
	rand_2 = rand_2 * 0x41c64e6d + 0x3039;
	return ((rand_2 >> 10) & 0x7fff);
}

void SeedRandomDraw(int32_t seed)
{
	rand_2 = seed;
}

int32_t StartGame(int level_number)
{
	InitialiseLevelFlags();

	if (!InitialiseLevel(level_number))
		return EXIT_GAME;

	return GameLoop();
}

int32_t GameLoop()
{
	// main game loop. Frame compensated by performing control phase repeatedly

	overlay_flag = 1;

	InitialiseResetCamera();

	int32_t nframes = 1;

	auto status = ControlPhase(nframes);

	while (status != LEVEL_COMPLETED && status != EXIT_GAME && status != LEVEL_CHANGE)
	{
		nframes = draw_phase_game();
		status = game_closedown ? EXIT_GAME : ControlPhase(nframes);
	}

	g_audio->stop_all();

	return status;
}