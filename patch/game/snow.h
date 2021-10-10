#pragma once

#define MAX_SNOWFLAKES	256

struct SNOWFLAKE
{
	long x;
	long old_roomflags:1;
	long stopped:1;
	long y:30;
	long z;

	signed char xv;
	unsigned char yv;	// Also determines size of flake.
	signed char zv;
	unsigned char life;
};

void DoSnow();