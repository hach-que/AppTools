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

uint32_t tc_getsize(const char* filename)
{
	struct stat s;
	AppLib::FUSE::FuseLink::getattr(filename, &s);
	return s.st_size;
}

void tc_scale(uint32_t max)
{
	printf("0");
	printf("% 55i\n", max);
}

void tc_setup()
{
	for (int i = 0; i < 56; i += 1)
		printf(".");
	for (int i = 0; i < 56; i += 1)
		printf("\b");
}

void tc_setto(uint32_t current, uint32_t max, uint32_t iter_current, uint32_t iter_max)
{
	for (int i = 0; i < 56; i += 1)
	{
		if (floor((double)i / 56.0f * max) < current)
			printf("=");
		else if (floor((double)i / 56.0f * max) == current)
			printf("|");
		else if (floor((double)i / 56.0f * max) > current)
			printf(".");
	}
	printf(" %4i / %4i", iter_current, iter_max);
	for (int i = 0; i < 69; i += 1)
		printf("\b");
}