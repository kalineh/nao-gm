#include "VertexShader.h"

#include <assert.h>
#include <gl/glew.h>
#include <iostream>

#include <gm/gmBind.h>

#include "GLutil.h"
#include "Renderer.h"

namespace funk
{
VertexShader::VertexShader( const char * file ) : m_file( file )
{
	assert( file );
	m_id = LoadProgramFile( m_file.c_str(), GL_VERTEX_SHADER );
	assert( m_id != 0 );
}

VertexShader::~VertexShader()
{
	glDeleteShader(m_id);
}

GM_REG_NAMESPACE(VertexShader)
{
	GM_MEMFUNC_CONSTRUCTOR(VertexShader)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(name, 0);
		GM_PUSH_USER_HANDLED( VertexShader, new VertexShader(name) );
		return GM_OK;
	}

	GM_GEN_MEMFUNC_INT_VOID( VertexShader, GetId )
}

GM_REG_MEM_BEGIN(VertexShader)
GM_REG_MEMFUNC( VertexShader, GetId )
GM_REG_HANDLED_DESTRUCTORS(VertexShader)
GM_REG_MEM_END()
GM_BIND_DEFINE(VertexShader)
}