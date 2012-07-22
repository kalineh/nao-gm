#ifndef _INCLUDE_FONT_H_
#define _INCLUDE_FONT_H_

#include "Texture.h"

#include <math/v2.h>
#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>
#include <gfx/Vbo.h>

namespace funk
{
	class Font : public HandledObj<Font>
	{
	public:
		Font();

		void Init( const char * file, int fontSize );
		void Print( const char * text, v2 pos );

		int GetWidth( const char * text ) const;
		int GetCharWidth( unsigned char c ) const { return m_charWidths[c]; }
		int GetHeight() const { return m_fontSize; };
		
		StrongHandle<Texture> GetTex() const { return m_tex; }

		GM_BIND_TYPEID(Font);

	private:

		void InitVbo();

		StrongHandle<Texture> m_tex;
		StrongHandle<Vbo> m_vbo;

		struct Vert
		{
			v2 pos;
			v2 uv;
		};
		std::vector<Vert> m_vertsBuffer;
		int m_vboIndex; // index where to push data

		static const int NUM_CHARS = 256;

		// character map data
		v2				m_charSizeUVs[NUM_CHARS];
		v2				m_deltaUV;
		unsigned char	m_charWidths[NUM_CHARS];
		int				m_fontSize;
	};

	GM_BIND_DECL( Font );
}
#endif