#pragma once

/* Game options - bits determine what options appear on wheel */
#define NO_ACTION    0
#define GAME_ACTION  1
#define END_ACTION	 2
#define LOAD_ACTION  4
#define SAVE_ACTION  8
#define SOUND_ACTION 16
#define VIDEO_ACTION 32

#define DONT_TARGET	(-16384)

#define UNIT_SHADOW		256
#define UNIT_SHADOW_SHIFT 8

// For triggers
#define ATONESHOT		0x80		// Anti-trigger ONESHOT.
#define ONESHOT			0x100
#define CODE_BITS		0x3e00
#define REVERSE			0x4000
#define NOT_VISIBLE		ONESHOT
#define KILLED_ITEM		(-0x8000)
#define CLEAR_BODY		KILLED_ITEM

#define MAX_SECRETS		16
#define MAX_DYNAMICS	10
#define NO_CAMERA		-1

enum camera_type;

// Camera flags to modify basic behaviour
#define FOLLOW_CENTRE 1
#define NO_CHUNKY     2
#define CHASE_OBJECT  3
#define	NO_MINY	0xffffff

/* Include data types for particular object types + item specific prototypes */
#include "box.h"
#include "lara.h"
#include "traps.h"
#include "inventry.h"
#include "text.h"
#include "gameflow.h"