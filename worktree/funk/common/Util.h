#ifndef _INCLUDE_COMMON_UTIL_H_
#define _INCLUDE_COMMON_UTIL_H_

char * TextFileRead( const char * file, int * numBytes = 0 );

inline int IsLittleEndian() 
{
	union 
	{
		unsigned int i;
		char c[4];
	} bint = {0x01020304};

	return bint.c[0] == 1; 
}

#endif