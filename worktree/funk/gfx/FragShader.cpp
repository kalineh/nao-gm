#include "FragShader.h"

#include <assert.h>
#include <gl/glew.h>
#include <iostream>

#include <gm/gmBind.h>

#include "GLutil.h"
#include "Renderer.h"

namespace funk
{
FragShader::FragShader( const char * file ) : m_file(file)
{
	assert( file );
	m_id = LoadProgramFile( m_file.c_str(), GL_FRAGMENT_SHADER );
	assert( m_id != 0 );
}

FragShader::~FragShader()
{
	glDeleteShader(m_id);
}

GM_REG_NAMESPACE(FragShader)
{
	GM_MEMFUNC_CONSTRUCTOR(FragShader)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(name, 0);
		GM_PUSH_USER_HANDLED( FragShader, new FragShader(name) );
		return GM_OK;
	}

	GM_GEN_MEMFUNC_INT_VOID( FragShader, GetId )
}

GM_REG_MEM_BEGIN(FragShader)
GM_REG_MEMFUNC( FragShader, GetId )
GM_REG_HANDLED_DESTRUCTORS(FragShader)
GM_REG_MEM_END()
GM_BIND_DEFINE(FragShader)
}