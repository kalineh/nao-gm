#include "Renderer.h"

#include <SDL.h>
#include <gl/glew.h>
#include <il/il.h>
#include <il/ilu.h>

#include <common/IniReader.h>
#include <common/Debug.h>
#include <common/Window.h>

namespace funk
{

const int SHADER_NULL = 0;
const int SHADER_DEFAULT = -1;

Renderer::Renderer()
{
	IniReader reader( "common/ini/main.ini" );
	m_bCheckerror = reader.GetInt( "Window", "GLCheckError" ) != 0;

	GLenum err = glewInit();
	CHECK( err == GLEW_OK, "GLEW unable to initialize!" );

	// print info
	/*char buffer[256];
	GetInfo(buffer, sizeof(buffer));
	printf("%s\n", buffer);*/

	ilInit();
	iluInit();
	ilEnable(IL_ORIGIN_SET);
	ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_LOWER_LEFT);

	m_shaderProgram = NULL;
}

Renderer::~Renderer()
{
}

void Renderer::GetInfo( char * buffer, int len )
{
	// get version
	sprintf_s( buffer, len,
		"OpenGL version %s\n\tShading model: %s\n\tVendor: %s\n\tRenderer: %s\n", 
		glGetString(GL_VERSION), 
		glGetString(GL_SHADING_LANGUAGE_VERSION),
		glGetString(GL_VENDOR),
		glGetString(GL_RENDERER));
}

void Renderer::Present()
{
	SDL_GL_SwapBuffers();
}

void Renderer::BeginDefaultShader()
{
	WARN( !IsShaderBound(), "Cannot set default shader, shader already bound!" );
	
	m_defaultShader = true;
	glUseProgram(0);
}

void Renderer::EndDefaultShader()
{
	WARN( IsShaderBound(), "Cannot end default shader, no shaders bound!" );
	m_defaultShader = false;
}

void Renderer::HandleErrorDraw()
{
	GLint err = glGetError();
	CHECK( err == GL_NO_ERROR, "GLerror: 0x%X!", err );

	printf("ERROR detected: HandleErrorDraw()\n");

	// end shader
	if ( IsShaderBound() )
	{
		m_shaderProgram = SHADER_NULL;
		glUseProgram(0);		
	}

	// end: Fbo, Ibo, Vbo
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// disable vertex stuff
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );

	// reset viewport
	const v2i dimen =  Window::Get()->Sizei();
	glViewport( 0, 0, dimen.x, dimen.y );

	// TODO: cameras?
}

void Renderer::Update()
{
	if ( m_bCheckerror )
	{	
		GLint err = glGetError();
		CHECK( err == GL_NO_ERROR, "GLerror: 0x%X!", err );
	}
}
}