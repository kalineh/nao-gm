#ifndef _INCLUDE_TEXTUREMANAGER_H_
#define _INCLUDE_TEXTUREMANAGER_H_

#include <map>
#include <string>

#include <common/Singleton.h>
#include <common/StrongHandle.h>

namespace funk
{
	class Texture;

	class TextureManager : public Singleton<TextureManager>
	{
	public:
		StrongHandle<Texture> GetTex( const char * filename );
		StrongHandle<Texture> CreateTexFromData( const char * name, unsigned char * data, int width, int height, int bitsPerPixel = 32 );

	private:
		friend Texture;
		void ReleaseTexture( const char * filename );

		friend Singleton<TextureManager>;
		TextureManager();
		~TextureManager();

		std::map< std::string, Texture* > m_mapTex;
	};
}

#endif