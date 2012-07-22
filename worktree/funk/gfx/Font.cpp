#include "Font.h"

#include <stdio.h>
#include <gl/glew.h>

#include <gfx/TextureManager.h>
#include <gm/gmBind.h>

namespace funk
{

const int MAX_CHARS = 4096;
const int MAX_VERTS_PER_CHAR = 4;
const int MAX_VERTS = MAX_CHARS*MAX_VERTS_PER_CHAR;

inline void GetCharRowCol( unsigned char c, int &row, int &col )
{
	row = c >> 4;
	col = c % 16;
}

Font::Font()
{}

void Font::Init( const char * file, int fontSize )
{
	// for some reason, need to pad by this
	fontSize += 2;

	// load char map data
	char dataMap[128];
	sprintf_s(dataMap, "%s.dat", file);	
	FILE * fh = fopen(dataMap,"rb");
	assert( fh != NULL );
	fread(m_charWidths, sizeof(m_charWidths), 1, fh);
	fclose(fh);

	// texture size
	m_tex = TextureManager::Get()->GetTex( file );
	v2i texDimen = m_tex->Sizei();
	m_fontSize = fontSize;

	// calc uv tex
	float uvY = (float)m_fontSize / texDimen.y;
	for ( int i = 0; i < NUM_CHARS; ++i )
	{
		float u = (float)m_charWidths[i] / texDimen.x;
		m_charSizeUVs[i] = v2(u,uvY);
	}

	InitVbo();
}

void Font::InitVbo()
{
	m_vboIndex = 0;

	StrongHandle< VertexAttribute > attrib = new VertexAttribute;
	attrib->AddAttrib( VertexAttribute::ATTRIB_VERTEX, VertexAttribute::ATTRIB_FLOAT, 2, sizeof( Vert ), 0 );
	attrib->AddAttrib( VertexAttribute::ATTRIB_TEXTURE_0, VertexAttribute::ATTRIB_FLOAT, 2, sizeof( Vert ), offsetof(Vert, uv) );

	m_vbo = new Vbo( NULL, MAX_VERTS, sizeof(Vert), attrib, GL_DYNAMIC_DRAW );
}

void Font::Print( const char * text, v2 pos )
{
	if ( text == NULL ) return;
	const unsigned int len = strlen( text );
	if ( len == 0 ) return;

	// make sure has enough verts to handle
	assert(len < MAX_CHARS);

	pos.x = floorf(pos.x);
	pos.y = floorf(pos.y);	

	// build verts
	const int batchNumVerts = len*MAX_VERTS_PER_CHAR;
	m_vertsBuffer.resize(batchNumVerts);

	const float uvDivisor = 1.0f / 16.0f;
	float xOffset = 0.0f;
	
	for ( unsigned int i = 0; i < len; ++i )
	{
		const unsigned char c = text[i];
		const v2 charDimen = v2( (float)m_charWidths[c], (float)m_fontSize );
		const v2 deltaUV = m_charSizeUVs[c];

		// calc start uv
		int row, col;
		GetCharRowCol( c, row, col );

		const v2 uv = v2( col*uvDivisor, 1.0f-row*uvDivisor );
		const int vertIndex = i * MAX_VERTS_PER_CHAR;

		// build verts
		m_vertsBuffer[vertIndex+0].uv = v2(uv.x, uv.y);
		m_vertsBuffer[vertIndex+0].pos = v2(pos.x + xOffset, pos.y);
		m_vertsBuffer[vertIndex+1].uv = v2( uv.x+deltaUV.x,  uv.y );
		m_vertsBuffer[vertIndex+1].pos = v2( pos.x + xOffset + charDimen.x, pos.y );
		m_vertsBuffer[vertIndex+2].uv = v2( uv.x+deltaUV.x, uv.y-deltaUV.y );
		m_vertsBuffer[vertIndex+2].pos = v2( pos.x + xOffset + charDimen.x, pos.y-charDimen.y );
		m_vertsBuffer[vertIndex+3].uv = v2( uv.x, uv.y-deltaUV.y );
		m_vertsBuffer[vertIndex+3].pos = v2( pos.x + xOffset, pos.y-charDimen.y );

		xOffset += charDimen.x;
	}

	// wrap around if not enough room
	if ( m_vboIndex + batchNumVerts > MAX_CHARS ) m_vboIndex = 0;

	// send and render
	m_tex->Bind();
	m_vbo->Bind();
	m_vbo->SubData( (unsigned char*)&m_vertsBuffer[0], batchNumVerts*sizeof(Vert), m_vboIndex*sizeof(Vert) );
	m_vbo->Render( GL_QUADS, m_vboIndex, batchNumVerts );
	m_vbo->Unbind();
	m_tex->Unbind();

	// move index pointer along
	m_vboIndex += batchNumVerts;
}

int Font::GetWidth( const char * text ) const
{
	int width = 0;

	int len = strlen(text);
	for( int i = 0; i < len; ++i )
	{
		unsigned char c = text[i];
		width += m_charWidths[c];
	}

	return width;
}

GM_REG_NAMESPACE(Font)
{
	GM_MEMFUNC_CONSTRUCTOR(Font)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(filename, 0);
		GM_CHECK_INT_PARAM(fontSize, 1);

		StrongHandle<Font> font = new Font();
		font->Init(filename, fontSize );
		GM_PUSH_USER_HANDLED( Font, font.Get() );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Print)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(str, 0);
		GM_CHECK_VEC2_PARAM(pos, 1);

		GM_GET_THIS_PTR(Font, ptr);
		ptr->Print(str, pos);
		
		return GM_OK;
	}

	GM_MEMFUNC_DECL(GetDimen)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(str, 0);

		GM_GET_THIS_PTR(Font, ptr);
		float x = (float)ptr->GetWidth(str);
		float y = (float)ptr->GetHeight();
		a_thread->PushVec2( v2(x,y) );

		return GM_OK;
	}

	GM_MEMFUNC_DECL(GetWidth)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(str, 0);

		GM_GET_THIS_PTR(Font, ptr);
		a_thread->PushInt( ptr->GetWidth(str) );

		return GM_OK;
	}

	GM_GEN_MEMFUNC_INT_VOID(Font, GetHeight)
}

GM_REG_MEM_BEGIN(Font)
GM_REG_MEMFUNC( Font, Print )
GM_REG_MEMFUNC( Font, GetDimen )
GM_REG_MEMFUNC( Font, GetHeight )
GM_REG_MEMFUNC( Font, GetWidth )
GM_REG_HANDLED_DESTRUCTORS(Font)
GM_REG_MEM_END()
GM_BIND_DEFINE(Font)

}