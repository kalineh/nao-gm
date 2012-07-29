#ifndef _INCLUDE_TEXTURE_H_
#define _INCLUDE_TEXTURE_H_

#include <string>

#include <math/v2.h>
#include <math/v2i.h>
#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>
#include <gfx/TextureParams.h>

namespace funk
{
	class Texture : public HandledObj<Texture>
	{
	public:

		Texture( int width, int height, TexParams params = TexParams() );

		void Bind( int shaderSlot = 0);
		void Unbind();

		void GenMipMaps();

		inline v2i Sizei() const { return m_dimen; }
		inline v2  Sizef() const { return v2((float)m_dimen.x, (float)m_dimen.y); }

		unsigned int Id() const { return m_id; }
		const std::string & Filename() const { return m_filename; }

		void ReadPixels( int x, int y, int width, int height, void* data );
		void SubData( void* data, int width, int height, int offsetX = 0, int offsetY = 0, int level = 0);		
		void GetTexImage( void* output );

		void SetWrap( int uType, int vType );
		void SetFilter( int minFilter, int magFilter );

		GM_BIND_TYPEID(Texture);
		~Texture();

	private:

		// FBO
		bool m_bManaged;
		bool m_bound;

		TexParams m_params;

		// texture manager
		friend class TextureManager;
		Texture();
		void Init( const char * file );
		void Init( const char * name, unsigned char * data, int width, int height, int bitsPerPixel = 32 );

		std::string m_filename;

		unsigned int m_id;
		v2i m_dimen;
		int m_texSlot;
	};

	GM_BIND_DECL(Texture);
}
#endif