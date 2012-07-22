#include "Ibo.h"

#include <common/Debug.h>

namespace funk
{
Ibo::Ibo( const unsigned int* data, unsigned int numElems, unsigned int type )
: m_numElems( numElems ), m_type( type )
{
	CHECK( data != 0, "No IBO data set" );

	// load data onto CPU
	glGenBuffers( 1, &m_id );

	// Bind the buffer and send data
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_id );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_numElems * sizeof( unsigned int ), data, m_type );
}

Ibo::~Ibo()
{
	glDeleteBuffers( 1, & m_id );
}

void Ibo::Bind()
{
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_id );
}

void Ibo::Render( unsigned int renderPrimitive, unsigned int numElems, unsigned int offset )
{
	numElems = numElems == 0 ? m_numElems : numElems;

	CHECK( offset + numElems <= m_numElems );
	CHECK( offset >= 0 );
	CHECK( numElems >= 0 );

	glDrawRangeElements( renderPrimitive, 0, numElems, numElems, GL_UNSIGNED_INT, (GLvoid*)((offset) * sizeof(unsigned int)) );
}

void Ibo::Unbind()
{
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}
}