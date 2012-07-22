#include "TextureManager.h"

#include <assert.h>
#include <common/Debug.h>
#include "Texture.h"

namespace funk
{
typedef std::map< std::string, Texture* >::iterator TexIter;

TextureManager::TextureManager()
{;}

TextureManager::~TextureManager()
{;}

StrongHandle<Texture> TextureManager::GetTex( const char * filename )
{
	assert( filename );

	TexIter it = m_mapTex.find( filename );

	// not found
	if ( it == m_mapTex.end() || it->second == NULL )
	{
		StrongHandle<Texture> pTex(new Texture);
		pTex->Init( filename );
		m_mapTex[ filename ] = pTex;
		return pTex;
	}

	return it->second;
}

StrongHandle<Texture> TextureManager::CreateTexFromData( const char * name, unsigned char * data, int width, int height, int bitsPerPixel )
{
	assert( name && data );

	TexIter it = m_mapTex.find( name );

	// not found
	if ( it == m_mapTex.end() || it->second == NULL )
	{
		StrongHandle<Texture> pTex = new Texture;
		pTex->Init( name, data, width, height, bitsPerPixel );
		m_mapTex[ name ] = pTex;
		return pTex;
	}

	return it->second;
}

void TextureManager::ReleaseTexture( const char * filename )
{
	assert( filename );

	TexIter it = m_mapTex.find( filename );
	CHECK( it != m_mapTex.end(), "Unable to release texture '%s'", filename );

	m_mapTex.erase( it );
}
}