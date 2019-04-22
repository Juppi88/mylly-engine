#include "utils.h"
#include <time.h>
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

void Utils::Initialize(void)
{
	// Seed random number generator.
	srand(time(NULL));
}

float Utils::Random(float min, float max)
{
	return min + ((float)rand() / RAND_MAX) * (max - min);
}

int Utils::Random(int min, int max)
{
	return min + (rand() % (max - min));
}

bool Utils::FlipCoin(void)
{
	return ((rand() & 1) == 0);
}
