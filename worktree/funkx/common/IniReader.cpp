#include "IniReader.h"

#include <assert.h>
#include <fstream>

#include <common/Debug.h>

namespace funk
{

IniReader::IniReader( const char * file )
: m_fileName( file )
{
	assert( file );
	readFile();
}

void IniReader::readFile()
{
	std::fstream fh( m_fileName.c_str(), std::ios::in );

	CHECK( fh.is_open(), "INI file '%s' does not exist!", m_fileName.c_str() );

	char line[128];
	std::string currSection = "";

	while( !fh.eof() )
	{
		fh.getline( line, sizeof(line) );

		std::string str = line;

		if ( processComment(str) ) continue;		
		else if ( processSection(str, currSection) ) continue;
		else if ( processDataRow(str, currSection) ) continue;
	}

	fh.close();
}

bool IniReader::processComment( const std::string & str )
{
	if( str.empty() ) return true;

	size_t nonblank = str.find_first_not_of(" \t");
	if ( nonblank == std::string::npos ) return true;

	// TODO blank lines or ;
	size_t firstChar = str.find_first_not_of( " \t;" );
	size_t firstCommentChr = str.find_first_of( ";" );

	if ( firstCommentChr != std::string::npos )
	{
		return firstChar > firstCommentChr;
	}

	return false;
}

bool IniReader::processDataRow( const std::string & str, const std::string & section )
{
	assert( !section.empty() && m_sectionMap.find(section) != m_sectionMap.end() );

	size_t eqPos = str.find_first_of( "=" );	

	if ( eqPos == std::string::npos ) return false;

	size_t keyStart = str.find_first_not_of( " \t" );
	size_t keyEnd = str.find_last_not_of( " \t", eqPos-1 );
	std::string key = str.substr( keyStart, keyEnd - keyStart+1 );

	size_t valStart = str.find_first_not_of( " \t", eqPos+1 );
	size_t valEnd = str.find_last_not_of( " \t"  );
	std::string val = str.substr( valStart, valEnd - valStart+1 );

	// make sure key doesnt exist
	assert( m_sectionMap[ section ].find( key ) == m_sectionMap[ section ].end() );

	(m_sectionMap[ section ])[key] = val;

	return true;
}

bool IniReader::processSection( const std::string & str, std::string & section )
{
	size_t begBracket = str.find_first_of( "[" );
	size_t endBracket = str.find_first_of( "]" );

	// Brackets must be closed
	assert (!( (begBracket != std::string::npos && endBracket == std::string::npos) ||
		(begBracket == std::string::npos && endBracket != std::string::npos) ));

	// found
	if ( begBracket != std::string::npos && endBracket != std::string::npos )
	{
		section = str.substr( begBracket+1, endBracket - begBracket-1 );

		// Section exists?
		assert( m_sectionMap.find(section) == m_sectionMap.end() );

		DataMap datamap;
		m_sectionMap[section] = datamap;
	}

	return false;
}

std::string IniReader::GetStr( const char* section, const char* key )
{
	checkValid( section, key );
	return m_sectionMap[section][key];
}

int IniReader::GetInt( const char* section, const char* key )
{
	checkValid( section, key );
	return atoi( m_sectionMap[section][key].c_str() );
}

float IniReader::GetFloat( const char* section, const char* key )
{
	checkValid( section, key );
	return (float)atof( m_sectionMap[section][key].c_str() );
}

const char* IniReader::GetCStr( const char* section, const char* key )
{
	checkValid( section, key );
	return m_sectionMap[section][key].c_str();
}

bool IniReader::validValue( const char* section, const char* key ) const
{
	return ( m_sectionMap.find(section) != m_sectionMap.end() &&
		(m_sectionMap.find(section)->second).find( key ) != (m_sectionMap.find(section)->second).end() );
}

void IniReader::checkValid( const char* section, const char* key ) const
{
	CHECK( validValue( section, key ), 
		"INI file '%s.ini' cound not find section '%s', key '%s'", 
		m_fileName.c_str(), section, key );
}

}