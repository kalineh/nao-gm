#include "Texture.h"

#include <gm/gmBind.h>
#include <gm/gmBindFuncGen.h>
#include <common/Debug.h>

#include <gl/glew.h>
#include <il/il.h>
#include <il/ilu.h>

#include "TextureManager.h"

namespace funk
{
Texture::Texture(): m_id(0), m_texSlot(-1), m_bManaged(true), m_bound(false)
{;}

Texture::Texture( int width, int height, TexParams params ) : m_bManaged(false), m_params(params), m_bound(false)
{
	m_dimen = v2i(width, height);

	glGenTextures(1, &m_id); // Generate one texture

	glBindTexture(GL_TEXTURE_2D, m_id); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, params.internalFormat, m_dimen.x, m_dimen.y, 0, params.format, params.dataType, 0);

	// Setup the basic texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);

	GLint err = glGetError();
	CHECK( err == GL_NO_ERROR, "Creating FBO texture failed, GLerror: %d!", err );
}

Texture::~Texture()
{
	if ( m_bManaged ) TextureManager::Get()->ReleaseTexture(m_filename.c_str() );

	// free from mem
	if ( m_id > 0 ) glDeleteTextures(1, &m_id);
}

void Texture::Init( const char * file )
{
	CHECK( m_id == 0, "Texture '%s' already initialized!", file );
	m_filename = file;

	ILuint ih;
	ilGenImages(1, &ih);
	ilBindImage(ih);
	ilLoadImage(file);
	
	ILuint errorid = ilGetError() ;
	CHECK( errorid == IL_NO_ERROR, "Texture '%s' load error: %s", file, iluErrorString(errorid) ) ;

	m_dimen.x = (int)ilGetInteger( IL_IMAGE_WIDTH );
	m_dimen.y = (int)ilGetInteger( IL_IMAGE_HEIGHT );
	
	ILuint format = ilGetInteger ( IL_IMAGE_FORMAT );
	ILuint dataType = ilGetInteger ( IL_IMAGE_TYPE );
	
	// Find the GL image format and type
	GLint glFormat = GL_RGB + ( format - IL_RGB );
	GLint glType = GL_BYTE + (dataType-IL_BYTE);

	int channels = ( glFormat == GL_RGBA ) ? 4: 3;

	glGenTextures( 1, &m_id );
	glBindTexture( GL_TEXTURE_2D, m_id );
	glTexImage2D(GL_TEXTURE_2D, 0, channels, m_dimen.x, m_dimen.y, 0, glFormat, glType, ilGetData() );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glBindTexture( GL_TEXTURE_2D, 0 );

	ilDeleteImages(1, &ih);
}

void Texture::SetWrap( int uType, int vType )
{
	CHECK( m_bound, "Texture not bound!");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vType);
}

void Texture::SetFilter( int minFilter, int magFilter )
{
	CHECK( m_bound, "Texture not bound!");

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, minFilter );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, magFilter );
}

void Texture::Init( const char * name, unsigned char * data, int width, int height, int bitsPerPixel )
{
	m_filename = name;

	CHECK( m_id == 0, "Texture already initialized!" );
	CHECK( bitsPerPixel == 32 || bitsPerPixel == 24, "Texture data can only be 24 or 32-bit!");

	m_dimen.x = width;
	m_dimen.y = height;

	glGenTextures( 1, &m_id );
	glBindTexture( GL_TEXTURE_2D, m_id );

	GLint glFormat = bitsPerPixel == 32 ? GL_RGBA :  GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, 3, m_dimen.x, m_dimen.y, 0, glFormat, GL_UNSIGNED_BYTE, data );
}

void Texture::Bind( int texSlot )
{
	CHECK( m_id != 0, "Texture not initialized!" );
	WARN( !m_bound, "Texture already bound!");

	glEnable( GL_TEXTURE_2D );
	glActiveTexture(GL_TEXTURE0 + texSlot );
	glBindTexture(GL_TEXTURE_2D, m_id );

	m_texSlot = texSlot;
	m_bound = true;
}

void Texture::Unbind()
{
	WARN( m_bound, "Texture not bound!");
	glDisable( GL_TEXTURE_2D );

	glActiveTexture(GL_TEXTURE0 + m_texSlot );
	glBindTexture(GL_TEXTURE_2D, 0 );

	m_texSlot = -1;
	m_bound = false;
}

void Texture::SubData( void* data, int width, int height, int offsetX, int offsetY, int level /*= 0*/ )
{
	CHECK( m_bound, "Texture not bound!");

	assert( width+offsetX <= m_dimen.x && width >= 0 );
	assert( height+offsetY <= m_dimen.y && height >= 0 );

	glTexSubImage2D( GL_TEXTURE_2D, level, offsetX, offsetY, width, height, m_params.format, m_params.dataType, data );
}

void Texture::ReadPixels( int x, int y, int width, int height, void* data )
{
	CHECK( m_bound, "Texture not bound!");

	assert( width+x <= m_dimen.x && x >= 0 );
	assert( height+y <= m_dimen.y && y >= 0 );

	Bind();
	glReadPixels( x, y, width, height, m_params.format, m_params.dataType, data );
	Unbind();
}

void Texture::GenMipMaps()
{
	CHECK( m_bound, "Texture not bound!");

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
}

GM_REG_NAMESPACE(Texture)
{
	GM_MEMFUNC_CONSTRUCTOR(Texture)
	{
		GM_CHECK_NUM_PARAMS(1);

		// get texture
		if ( GM_IS_PARAM_STRING(0) )
		{
			GM_STRING_PARAM( file, 0, "ERROR" );
			StrongHandle<Texture> p = TextureManager::Get()->GetTex(file);
			GM_PUSH_USER_HANDLED( Texture, p.Get() );

			return GM_OK;
		}

		// surface
		else if ( GM_IS_PARAM_VEC2(0) )
		{
			GM_VEC2_PARAM( size, 0);
			StrongHandle<Texture> p = new Texture( int(size.x), int(size.y) );
			GM_PUSH_USER_HANDLED( Texture, p.Get() );

			return GM_OK;
		}

		return GM_EXCEPTION;
	}

	GM_MEMFUNC_DECL(Filename)
	{
		GM_CHECK_NUM_PARAMS(0);

		GM_GET_THIS_PTR(Texture, ptr);
		a_thread->PushNewString( ptr->Filename().c_str(), ptr->Filename().size() );

		return GM_OK;
	}

	GM_MEMFUNC_DECL(Width)
	{
		GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(Texture, ptr);
		a_thread->PushInt(ptr->Sizei().x);

		return GM_OK;
	}

	GM_MEMFUNC_DECL(Height)
	{
		GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(Texture, ptr);
		a_thread->PushInt(ptr->Sizei().y);

		return GM_OK;
	}


	GM_MEMFUNC_DECL(Dimen)
	{
		GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(Texture, ptr);
		a_thread->PushVec2(v2( (float)ptr->Sizei().x, (float)ptr->Sizei().y));

		return GM_OK;
	}

	GM_MEMFUNC_DECL(Bind)
	{
		GM_INT_PARAM( texSlot, 0, 0 );
		GM_GET_THIS_PTR(Texture, ptr);
		ptr->Bind(texSlot);

		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_VOID( Texture, Unbind )
	GM_GEN_MEMFUNC_INT_VOID( Texture, Id )
	GM_GEN_MEMFUNC_VOID_VOID( Texture, GenMipMaps )

	GM_GEN_MEMFUNC_VOID_INT_INT( Texture, SetWrap )
	GM_GEN_MEMFUNC_VOID_INT_INT( Texture, SetFilter )
}

GM_REG_MEM_BEGIN(Texture)
GM_REG_MEMFUNC( Texture, Bind )
GM_REG_MEMFUNC( Texture, Unbind )
GM_REG_MEMFUNC( Texture, Dimen )
GM_REG_MEMFUNC( Texture, Width )
GM_REG_MEMFUNC( Texture, Height )
GM_REG_MEMFUNC( Texture, Id )
GM_REG_MEMFUNC( Texture, GenMipMaps )
GM_REG_MEMFUNC( Texture, Filename )
GM_REG_MEMFUNC( Texture, SetWrap )
GM_REG_MEMFUNC( Texture, SetFilter )
GM_REG_HANDLED_DESTRUCTORS(Texture)
GM_REG_MEM_END()
GM_BIND_DEFINE(Texture)

}