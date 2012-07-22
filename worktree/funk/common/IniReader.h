#ifndef _INCLUDE_INIREADER_H_
#define _INCLUDE_INIREADER_H_

#include <map>
#include <string>

namespace funk
{
	class IniReader
	{
	public:
		IniReader( const char * file );

		std::string		GetStr( const char* section, const char* key );
		const char*		GetCStr( const char* section, const char* key );
		int				GetInt( const char* section, const char* key );
		float			GetFloat( const char* section, const char* key );

	private:

		void checkValid( const char* section, const char* key ) const;
		bool validValue( const char* section, const char* key ) const;
		void readFile();

		bool processComment( const std::string & str );
		bool processDataRow( const std::string & str, const std::string & section );
		bool processSection( const std::string & str, std::string & section );

		typedef std::map< std::string, std::string > DataMap;
		typedef std::map< std::string, DataMap > SectionMap;

		SectionMap m_sectionMap;
		std::string m_fileName;
	};
}

#endif