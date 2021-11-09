#pragma once

#include <3dsystem/3dglodef.h>

// maximum number of frame compensation

#define MAX_FRAMES		10
#define MALLOC_SIZE		(1024 * 1024 * 100)		// 3.5 MB
#define ONE_DEGREE		182

struct SETUP
{
	short sc_width, sc_height;
	short dump_x, dump_y, dump_width, dump_height;
	void* scrn_ptr;
};