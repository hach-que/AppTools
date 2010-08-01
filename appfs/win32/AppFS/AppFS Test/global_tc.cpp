// Utility functions for the truncate section of the test suite.
#include <math.h>
#include <sys/stat.h>
#include "fuselink.h"

bool tc_verify(const char* filename, int intended_size)
{
	struct stat s;
	AppLib::FUSE::FuseLink::getattr(filename, &s);
	return (s.st_size == intended_size);
}

void tc_scale(uint32_t max)
{
	printf("0");
	printf("% 68i\n", max);
}

void tc_setup()
{
	for (int i = 0; i < 69; i += 1)
		printf(".");
	for (int i = 0; i < 69; i += 1)
		printf("\b");
}

void tc_setto(uint32_t current, uint32_t max, uint32_t iter_current, uint32_t iter_max)
{
	for (int i = 0; i < 60; i += 1)
	{
		if (floor((double)i / 60.0f * max) < current)
			printf("=");
		else if (floor((double)i / 60.0f * max) == current)
			printf("|");
		else if (floor((double)i / 60.0f * max) > current)
			printf(".");
	}
	printf("% 3i / % 3i", iter_current, iter_max);
	for (int i = 0; i < 69; i += 1)
		printf("\b");
}