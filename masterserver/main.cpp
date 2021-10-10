import prof;

#include <shared/defs.h>

#include "ms/ms.h"

int main()
{
	prof::init("TombMP MasterServer");

#ifdef _DEBUG
	ShowWindow(GetConsoleWindow(), SW_MINIMIZE);
#endif

	ms* sv = new ms();

	if (!sv->init())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		return 0;
	}

	if (!sv->run())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		return 0;
	}

	delete sv;

	return 0;
}