#ifndef _INCLUDE_COMMON_UTIL_H_
#define _INCLUDE_COMMON_UTIL_H_

#include <string.h>

namespace funk
{

inline unsigned int HashFNV(const char* str)
{
	const size_t length = strlen(str) + 1;
	unsigned int hash = 2166136261u;

	for (size_t i=0; i<length; ++i)
	{
		hash ^= *str++;
		hash *= 16777619u;
	}

	return hash;
}
}

#endif