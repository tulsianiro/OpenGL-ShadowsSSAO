#include "pch.h"
#include "math.h"

float getRandom(float min, float max)
{
	static bool first = true;
	if (first)
	{
		first = false;
		srand(time(NULL));
	}

	float r = (float)rand() / (float)RAND_MAX;
	return min + r * (max - min);
}

float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}