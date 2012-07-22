#include "gmUtilEx.h"

#include <time.h>
#include <fstream>
#include <time.h>

#include <common/Util.h>
#include <common/Debug.h>
#include <vm/VirtualMachine.h>

#include "gmMachine.h"
#include "gmTableObject.h"
#include "gmStreamBuffer.h"

#include <math/v2.h>
#include <math/v3.h>

using namespace funk;

int gmCompileStr( gmMachine *vm, const char* file )
{
	// If using bytecode
	if ( VirtualMachine::Get()->IsUsingByteCode() )
	{
		int threadId;

		int numBytes;
		char * code = TextFileRead(file, &numBytes);

		gmStreamBufferStatic readBuffer;
		readBuffer.Open(code, numBytes );
		vm->ExecuteLib(readBuffer, &threadId, true );

		delete [] code;
		return threadId;
	}

	// Not using bytecode, compiling
	while(true)
	{
		int threadId;
		char * code = TextFileRead(file);

		// failed
		if ( !code ) return 0;

		if ( vm->GetDebugMode() ) vm->AddSourceCode( code, file );
		int err = vm->CheckSyntax( code );
		
		if ( !err ) 
		{
			printf("Compiled '%s'\n", file );
			vm->ExecuteString(code, &threadId);
			delete [] code;
			return threadId;
		}

		delete [] code;

		gmLog & compileLog = vm->GetLog();
		bool first = true;
		const char *msg = compileLog.GetEntry(first);
		if(msg)	
		{
			//printf("Compile error '%s': %s\n", file, msg );
			MESSAGE_BOX("GM Compile Error", "'%s': %s\n", file, msg ); 
		}
		compileLog.Reset();

		/*
		// wait
		clock_t goal = 1000+clock(); 
		while( goal > clock() ) {;}*/
	}
}

void OutputTableNode( std::ofstream &fh, gmTableObject * table, gmVariable & key, int level )
{
	// check not infinite loop
	const int recursionThreshold = 10;
	CHECK( level < recursionThreshold, "Saving table node file exceeded %d levels!", recursionThreshold );

	const char * TAB = "    ";

	for( int i = 0; i < level; ++i ) fh << TAB;

	gmVariable & var = table->Get(key);
	gmType type = var.m_type;

	// output table key
	if ( key.IsString() ) fh << key.GetCStringSafe() << " = ";

	switch( type )
	{
		case GM_NULL: fh << "null"; break;
		case GM_INT: fh << var.GetInt(); break;
		case GM_FLOAT: fh << var.GetFloat(); break;
		case GM_VEC2: 
		{
			funk::v2 vec = var.GetVec2();
			fh << "v2(" << vec.x << ", " << vec.y << ")";
			break;
		}
		case GM_VEC3: 
		{
			funk::v3 vec = var.GetVec3();
			fh << "v3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
			break;
		}
		case GM_STRING: fh << '"' << var.GetCStringSafe() << '"'; break;

		// invalid ones
		case GM_FUNCTION: fh << "null, // function"; break;
		case GM_USER: fh << "null, // user-type"; break;

		case GM_TABLE:
		{
			if ( key.IsString() ) 
			{
				fh << std::endl;
				for( int i = 0; i < level; ++i ) fh << TAB;
			}
			fh << "{" << std::endl;

			gmTableObject * childTable = var.GetTableObjectSafe();
			std::vector<gmVariable*> tableKeys;

			gmSortTableKeys( childTable, tableKeys );
			for( size_t i = 0; i < tableKeys.size(); ++i )
			{
				OutputTableNode( fh, childTable, *tableKeys[i], level+1 );
			}

			break;
		}
	};

	if ( type == GM_TABLE )
	{
		for( int i = 0; i < level; ++i ) fh << TAB;
		fh << "}";
	}
	
	if ( type != GM_FUNCTION && type != GM_USER ) fh << ",";
	fh << std::endl;
}

int gmSaveTableToFile( gmTableObject * table, const char * file )
{
	assert(table);

	std::ofstream fh(file);
	fh.precision(6);
	std::showpoint(fh);

	if (!fh.is_open() )
	{
		CHECK( fh.is_open(), "Failed to load file '%s'", file );
		return 0;
	}
	
	// output time
	char buffer[256];
	const char * format = "%Y-%m-%d, %I:%M:%S %p";
	time_t lt;
    time(&lt);
	struct tm * ct = localtime((time_t *) &lt);
	strftime(buffer, 256, format, ct);
		
	fh << "// generated: " << buffer << std::endl;


	fh << "global g_fileData =" << std::endl << "{" << std::endl;

	std::vector<gmVariable*> tableKeys;
	gmSortTableKeys( table, tableKeys );
	for( size_t i = 0; i < tableKeys.size(); ++i )
	{
		OutputTableNode( fh, table, *tableKeys[i], 1 );
	}

	fh << "};";
	fh.close();

	printf("Saved table to file: '%s'\n", file );

	return 0;
}

bool gmVariableCmp( const gmVariable & v0, const gmVariable & v1 )
{
	if ( v0.m_type == v1.m_type )
	{
		gmType type = v0.m_type;

		switch(type)
		{
			case GM_INT: return v0.GetInt() < v1.GetInt();
			case GM_FLOAT: return v0.GetFloat() < v1.GetFloat();
			case GM_STRING: return strcmp( v0.GetCStringSafe(), v1.GetCStringSafe() ) < 0;
			default: return true;
		}
	}
	else
	{
		return v0.m_type < v1.m_type;
	}	

	assert(false);
	return false;
}

void gmSortTableKeys( gmTableObject * table, std::vector<gmVariable*> & result )
{
	int tableSize = table->Count();

	result.clear();
	result.reserve(tableSize);

	gmTableIterator it;
	gmTableNode * node = table->GetFirst( it );
	while ( !table->IsNull(it) )
	{
		result.push_back( &(node->m_key) );
		node = table->GetNext(it);
	}

	// sort keys
	for( int i = 0; i < tableSize-1; ++i )
	{
		for( int j = i+1; j < tableSize; ++j )
		{
			if ( !gmVariableCmp( *result[i], *result[j] ) )
			{
				gmVariable * temp = result[i];
				result[i] = result[j];
				result[j] = temp;
			}
		}
	}
}