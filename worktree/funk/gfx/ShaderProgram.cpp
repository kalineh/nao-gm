#include "ShaderProgram.h"

#include <gl/glew.h>

#include <gm/gmBind.h>
#include <common/Debug.h>

#include "VertexShader.h"
#include "FragShader.h"
#include "Renderer.h"

namespace funk
{
ShaderProgram::ShaderProgram()
: m_linked(false), m_bound(false), m_vp(NULL), m_fp(NULL)
{
	m_id = glCreateProgram();
	CHECK( m_id != 0, "Error creating shader program!" );
}

ShaderProgram::~ShaderProgram()
{
	if ( m_vp ) glDetachShader(m_id, m_vp->GetId());
	if ( m_fp ) glDetachShader(m_id, m_fp->GetId());

	glDeleteProgram(m_id);
}

void ShaderProgram::Begin()
{
	CHECK(m_linked, "Shaders not linked!");

	glUseProgram(m_id);
	Renderer::Get()->m_shaderProgram = this;
	m_bound = true;
}

void ShaderProgram::End()
{
	CHECK(m_linked, "Shaders not linked!");

	glUseProgram(0);
	Renderer::Get()->m_shaderProgram = NULL;
	m_bound = false;
}

void ShaderProgram::AttachVertexShader( StrongHandle<VertexShader> vp )
{
	assert( vp != NULL );
	assert( vp->GetId() != 0 );
	assert( m_vp == NULL );

	m_vp = vp;
}

void ShaderProgram::AttachVertexShader( const char * name )
{
	assert(name);

	StrongHandle<VertexShader> vp = new VertexShader(name);
	AttachVertexShader(vp);
}

void ShaderProgram::AttachFragShader( StrongHandle<FragShader> fp )
{
	assert( fp != NULL );
	assert( fp->GetId() != 0 );
	assert( m_fp == NULL );

	m_fp = fp;
}

void ShaderProgram::AttachFragShader( const char * name )
{
	assert(name);

	StrongHandle<FragShader> fp = new FragShader(name);
	AttachFragShader(fp);
}

void ShaderProgram::Link()
{
	assert( m_fp != NULL || m_vp != NULL );

	if ( m_fp ) glAttachShader(m_id, m_fp->GetId() );
	if ( m_vp ) glAttachShader(m_id, m_vp->GetId() );

	glLinkProgram(m_id);

#ifdef _DEBUG
	GLint result = 0;
	glGetProgramiv(m_id, GL_LINK_STATUS, &result);
	assert( result == GL_TRUE );
	glGetProgramiv(m_id, GL_ATTACHED_SHADERS, &result);
	assert( result >= 2 );
#endif

	m_linked = true;
}

int ShaderProgram::GetUniformId( const char * name ) const
{
	CHECK(m_linked, "Shaders not linked!");
	GLint id = glGetUniformLocation( m_id, name );
	WARN( id >= 0, "%s uniform not found!", name );
	return id;
}

void ShaderProgram::SetUniform( int uniformId, float x, float y, float z, float w )
{
	CHECK(m_linked, "Shaders not linked!");
	assert( uniformId >= 0 );
	glUniform4f(uniformId, x, y, z, w);
}

void ShaderProgram::SetUniform( int uniformId, int i )
{
	CHECK(m_bound, "Shaders not bound!");
	assert( uniformId >= 0 );
	glUniform1i(uniformId, i);
}

void ShaderProgram::SetUniform( int uniformId, int x, int y, int z, int w )
{
	CHECK(m_bound, "Shaders not bound!");
	assert( uniformId >= 0 );
	glUniform4i(uniformId, x, y, z, w);
}

GM_REG_NAMESPACE(ShaderProgram)
{
	GM_MEMFUNC_CONSTRUCTOR(ShaderProgram)
	{
		GM_CHECK_NUM_PARAMS(0);
		GM_PUSH_USER_HANDLED( ShaderProgram, new ShaderProgram );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(AttachVertexShader)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM_PTR( funk::VertexShader, vp, 0 );
		GM_GET_THIS_PTR(ShaderProgram, ptr);
		
		ptr->AttachVertexShader(vp);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(AttachFragShader)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM_PTR( funk::FragShader, fp, 0 );
		GM_GET_THIS_PTR(ShaderProgram, ptr);

		ptr->AttachFragShader(fp);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetUniformInt)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_INT_PARAM( id, 0 );
		GM_CHECK_INT_PARAM( val, 1 );

		GM_GET_THIS_PTR(ShaderProgram, ptr);
		ptr->SetUniform(id, val);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetUniformInt4)
	{
		GM_CHECK_NUM_PARAMS(5);
		GM_CHECK_INT_PARAM( id, 0 );
		GM_CHECK_INT_PARAM( val0, 1 );
		GM_CHECK_INT_PARAM( val1, 2 );
		GM_CHECK_INT_PARAM( val2, 3 );
		GM_CHECK_INT_PARAM( val3, 4 );

		GM_GET_THIS_PTR(ShaderProgram, ptr);
		ptr->SetUniform(id, val0, val1, val2, val3);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetUniformFloat4)
	{
		GM_CHECK_NUM_PARAMS(5);
		GM_CHECK_INT_PARAM( id, 0 );
		GM_CHECK_FLOAT_OR_INT_PARAM( val0, 1 );
		GM_CHECK_FLOAT_OR_INT_PARAM( val1, 2 );
		GM_CHECK_FLOAT_OR_INT_PARAM( val2, 3 );
		GM_CHECK_FLOAT_OR_INT_PARAM( val3, 4 );

		GM_GET_THIS_PTR(ShaderProgram, ptr);
		ptr->SetUniform(id, val0, val1, val2, val3);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(GetShaderUniforms)
	{
		gmMachine * machine = a_thread->GetMachine();
		gmTableObject * table = a_thread->GetMachine()->AllocTableObject();

		GM_GET_THIS_PTR(ShaderProgram, ptr);
		unsigned int programId = ptr->GetId();

		int count = 0;
		glGetProgramiv( programId, GL_ACTIVE_UNIFORMS, &count ); 

		// go thru each uniform
		for ( int i = 0; i < count; ++i )
		{
			int nameLen = -1;
			int size = -1;
			GLenum type = GL_ZERO;

			char name[64];

			// uniform data
			glGetActiveUniform( programId, GLuint(i), sizeof(name)-1, &nameLen, &size, &type, name );
			name[nameLen] = 0;
			GLuint uniformId = glGetUniformLocation( programId, name );

			enum UniformType
			{
				SAMPLER	= 0,
				INT4	= 1,
				FLOAT4	= 2,
			};
			UniformType utype;

			switch(type)
			{
			case GL_SAMPLER_2D: utype = SAMPLER; break;
			case GL_INT_VEC4: utype = INT4; break;
			case GL_FLOAT_VEC4: utype = FLOAT4; break;
			default: printf("GetShaderUniforms: Uniform '%s' type unrecognized!\n", name); continue;
			};

			// create table
			gmTableObject* uniformTable = a_thread->GetMachine()->AllocTableObject();
			uniformTable->Set( machine, "id", gmVariable((int)uniformId) );
			uniformTable->Set( machine, "type", gmVariable(utype) );

			table->Set( machine, name, gmVariable(uniformTable) );
		}

		a_thread->PushTable(table);
		return GM_OK;
	}


	GM_GEN_MEMFUNC_VOID_VOID( ShaderProgram, Begin )
	GM_GEN_MEMFUNC_VOID_VOID( ShaderProgram, End )
	GM_GEN_MEMFUNC_VOID_VOID( ShaderProgram, Link )
}

GM_REG_MEM_BEGIN(ShaderProgram)
GM_REG_MEMFUNC( ShaderProgram, Begin )
GM_REG_MEMFUNC( ShaderProgram, End )
GM_REG_MEMFUNC( ShaderProgram, Link )
GM_REG_MEMFUNC( ShaderProgram, AttachVertexShader )
GM_REG_MEMFUNC( ShaderProgram, AttachFragShader )
GM_REG_MEMFUNC( ShaderProgram, GetShaderUniforms )
GM_REG_MEMFUNC( ShaderProgram, SetUniformInt )
GM_REG_MEMFUNC( ShaderProgram, SetUniformInt4 )
GM_REG_MEMFUNC( ShaderProgram, SetUniformFloat4 )
GM_REG_HANDLED_DESTRUCTORS(ShaderProgram)
GM_REG_MEM_END()
GM_BIND_DEFINE(ShaderProgram)
}