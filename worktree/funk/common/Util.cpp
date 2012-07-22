#include "Util.h"

#include "Debug.h"
#include <fstream>

using namespace funk;

char * TextFileRead( const char * fileName, int * numBytes )
{
	std::ifstream file( fileName, std::ios::binary );

	if (  !file.is_open() )
	{
		CHECK( false, "Cannot open file '%s'\n", fileName );
		return NULL;
	}

	file.seekg( 0, std::ios::end );
	size_t size = file.tellg();
	file.seekg( 0, std::ios::beg );

	char* data = new char[ size + 1 ];
	file.read( data, size );
	file.close();

	data[size] = '\0';

	if ( numBytes ) *numBytes = size;

	return data;
}