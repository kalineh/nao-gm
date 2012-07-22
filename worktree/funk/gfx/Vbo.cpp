#include "Vbo.h"

#include <gm/gmBind.h>
#include <gm/gmBindFuncGen.h>

#include <common/Debug.h>

namespace funk
{
	Vbo::Vbo( unsigned char * data, unsigned int numElems, unsigned int vertSizeBytes, StrongHandle< VertexAttribute > vertAttrib, unsigned int type )
		: m_stride( vertSizeBytes ), m_numElems( numElems ), m_id(0),
		m_vertAttrib( vertAttrib ), m_bound(false), m_dataType(type)
	{
		CHECK( m_vertAttrib != NULL, "Vertex attribute not set for VBO" );

		// load data onto CPU
		glGenBuffers( 1, &m_id );

		m_numBytesTotal = m_stride * m_numElems;

		// Bind the buffer and send data
		glBindBuffer( GL_ARRAY_BUFFER, m_id );
		glBufferData( GL_ARRAY_BUFFER, m_numBytesTotal, data, type );

		CHECK( glGetError() == GL_NO_ERROR, "Creating VBO failed, GLerror: %d!", glGetError() );
	}

	Vbo::~Vbo()
	{
		glDeleteBuffers( 1, &m_id );
	}

	void Vbo::Bind( unsigned int offset )
	{
		CHECK( !m_bound, "Vbo already bound!" );
		CHECK( offset < m_numElems, "Offset %d out of bounds (num elemes: %d)", offset, m_numElems );

		glBindBuffer( GL_ARRAY_BUFFER, m_id );
		m_vertAttrib->Bind( offset );
		m_bound = true;
	}

	void Vbo::Unbind()
	{
		CHECK( m_bound, "Vbo not bound!" );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		m_vertAttrib->Unbind();
		m_bound = false;
	}

	void Vbo::Render( unsigned int renderPrimitive, unsigned int offset, unsigned int numElems )
	{
		CHECK( m_bound, "Cannot render. Vbo not bound!" );

		numElems = numElems == 0 ? m_numElems : numElems;

		CHECK( offset + numElems <= m_numElems );
		CHECK( offset >= 0 );
		CHECK( numElems >= 0 );

		glDrawArrays( renderPrimitive, offset, numElems );
	}

	void Vbo::SubData( unsigned char* data, unsigned int numBytes, unsigned int vboBytesOffset )
	{
		CHECK( m_dataType <= GL_DYNAMIC_COPY && m_dataType >= GL_DYNAMIC_DRAW );
		CHECK( numBytes <= m_numBytesTotal, "Vbo subdata exceeded byte size!" );
		CHECK( m_bound, "Cannot sub datas, need to bind VBO!");
		glBufferSubData( GL_ARRAY_BUFFER, vboBytesOffset, numBytes, data );
	}
	
	
GM_REG_NAMESPACE(Vbo)
{
	GM_MEMFUNC_DECL(Render)
	{
		GM_CHECK_INT_PARAM( renderPrimitive, 0 );
		GM_INT_PARAM( offset, 1, 0 );
		GM_INT_PARAM( numElems, 2, 0 );

		GM_GET_THIS_PTR(Vbo, ptr);
		ptr->Render( renderPrimitive, offset, numElems );

		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_INT( Vbo, Bind )
	GM_GEN_MEMFUNC_VOID_VOID( Vbo, Unbind )
}

GM_REG_MEM_BEGIN_NO_CONSTRUCTOR(Vbo)
GM_REG_MEMFUNC( Vbo, Bind )
GM_REG_MEMFUNC( Vbo, Unbind )
GM_REG_MEM_END()
GM_BIND_DEFINE(Vbo)

}