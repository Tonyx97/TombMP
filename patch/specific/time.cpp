import prof;

#include "standard.h"
#include "global.h"

// sync remembers the last value of the system timer to allow frame rate to be calced

double LdSync, LdFreq;

void TIME_Reset()
{
	LARGE_INTEGER fq;

	QueryPerformanceCounter(&fq);

	LdSync = ((double)fq.LowPart + (double)fq.HighPart * (double)0xffffffff) / LdFreq;
}

bool time_init()
{
	prof::print(YELLOW, "time_init");

	if (LARGE_INTEGER fq; QueryPerformanceFrequency(&fq))
	{
		LdFreq = ((double)fq.LowPart + (double)fq.HighPart * (double)0xffffffff) / 60.0;

		TIME_Reset();

		return true;
	}
	else return false;
}

// Get the number of frames that has passed since the last call (60 per second)

int Sync()
{
	LARGE_INTEGER ct;

	QueryPerformanceCounter(&ct);

	// WARNING: inaccuracy as HighPart increases and double loses low bit precision?

	auto dCounter = ((double)ct.LowPart + (double)ct.HighPart * (double)0xffffffff) / LdFreq;

	// use the non-fractional bits to get the frame count

	auto nFrames = (long)dCounter - (long)LdSync;

	LdSync = dCounter;

	return nFrames;
}