#include "Fbo.h"

#include <assert.h>
#include <common/Debug.h>
#include <common/Window.h>

#include <gm/gmBind.h>
#include <gm/gmBindFuncGen.h>

#include "Texture.h"

namespace funk
{
Fbo::Fbo( int width, int height ) 
: m_width(width), m_height(height), m_fbo(0), 
m_fboDepth(0),  m_boundAsTarget(false)
{
	InitDepthBuffer();
	InitTexture();
	InitFrameBuffer();

	GLint err = glGetError();
	CHECK( err == GL_NO_ERROR, "Creating FBO texture failed, GLerror: %d!", err );
}

Fbo::~Fbo()
{
	CHECK( !m_boundAsTarget, "Deleting FBO that is currently bound!" );

	if ( m_fbo ) glDeleteFramebuffers(1, &m_fbo);
	if ( m_fboDepth ) glDeleteRenderbuffers(1, &m_fboDepth );
}

void Fbo::InitDepthBuffer()
{
	glGenRenderbuffersEXT(1, &m_fboDepth); // Generate one render buffer and store the ID in fbo_depth  
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_fboDepth); // Bind the fbo_depth render buffer  
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, m_width, m_height); // Set the render buffer storage to be a depth component, with a width and height of the window
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0); // Unbind the render buffer 

	GLint err = glGetError();
	CHECK( err == GL_NO_ERROR, "Creating FBO depth buffer failed, GLerror: 0x%X!", err );
}

void Fbo::InitTexture()
{
	m_tex = new Texture(m_width, m_height);
}

void Fbo::InitFrameBuffer()
{
	glGenFramebuffersEXT(1, &m_fbo); // Generate one frame buffer and store the ID in fbo
	assert( m_fbo != 0 );

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo); // Bind our frame buffer
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_tex->Id(), 0); // Attach the texture fbo_texture to the color buffer in our frame buffer
	
	if ( m_fboDepth ) glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_fboDepth); // Attach the depth buffer fbo_depth to our frame buffer

	// Error check
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	CHECK(status == GL_FRAMEBUFFER_COMPLETE_EXT, "Unable to create FBO!"); // If the frame buff

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // Unbind our frame buffer
}

void Fbo::SetColorTex( StrongHandle<Texture> tex )
{
	CHECK( tex != NULL, "FBO trying to set NULL texure as color buffer!");
	m_tex = tex;

	CHECK( !m_boundAsTarget, "Cannot bind next texture. Fbo currently bound!");

	Begin();
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_tex->Id(), 0);
	End();
}

void Fbo::Begin()
{
	CHECK( !m_boundAsTarget, "Fbo currently bound!");
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	m_boundAsTarget = true;

	// set viewport as FBO tex size
	const v2i dimen = m_tex->Sizei();
	glViewport( 0, 0, dimen.x, dimen.y );
}

void Fbo::End()
{
	CHECK( m_boundAsTarget, "Fbo not bound!");
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	m_boundAsTarget = false;

	// set viewport to screen
	v2i dimen =  Window::Get()->Sizei();
	glViewport( 0, 0, dimen.x, dimen.y );
}

GM_REG_NAMESPACE(Fbo)
{
	GM_MEMFUNC_CONSTRUCTOR(Fbo)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_FLOAT_OR_INT_PARAM(width, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(height, 1);

		StrongHandle<Fbo> p = new Fbo((int)width, (int)height);
		GM_PUSH_USER_HANDLED( Fbo, p.Get() );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(GetTex)
	{
		GM_CHECK_NUM_PARAMS(0);	

		GM_GET_THIS_PTR(Fbo, ptr);
		GM_PUSH_USER_HANDLED( Texture, ptr->GetTex().Get() );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetColorTex)
	{
		GM_CHECK_NUM_PARAMS(1);	
		GM_CHECK_USER_PARAM_PTR( Texture, tex, 0 );
		GM_GET_THIS_PTR(Fbo, ptr);

		ptr->SetColorTex(tex);
		
		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_VOID( Fbo, Begin )
	GM_GEN_MEMFUNC_VOID_VOID( Fbo, End )
	GM_GEN_MEMFUNC_INT_VOID( Fbo, Width )
	GM_GEN_MEMFUNC_INT_VOID( Fbo, Height)

}

GM_REG_MEM_BEGIN(Fbo)
GM_REG_MEMFUNC( Fbo, Begin )
GM_REG_MEMFUNC( Fbo, End )
GM_REG_MEMFUNC( Fbo, Width )
GM_REG_MEMFUNC( Fbo, Height )
GM_REG_MEMFUNC( Fbo, GetTex )
GM_REG_MEMFUNC( Fbo, SetColorTex )
GM_REG_HANDLED_DESTRUCTORS(Fbo)
GM_REG_MEM_END()
GM_BIND_DEFINE(Fbo)

}