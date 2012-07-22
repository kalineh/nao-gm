#ifndef _INCLUDE_VBO_H_
#define _INCLUDE_VBO_H_

#include <gl/glew.h>

#include <common/StrongHandle.h>
#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>

#include "VertexAttribute.h"

namespace funk
{
	class Vbo : public HandledObj<Vbo>
	{
	public:
		Vbo( unsigned char * data,
			 unsigned int numElems, 
			 unsigned int strideBytes, 
			 StrongHandle< VertexAttribute > vertAttrib,
			 unsigned int type = GL_STATIC_DRAW );

		~Vbo();

		void SubData( unsigned char* data, unsigned int numBytes, unsigned int vboBytesOffset = 0 );
		void Bind( unsigned int offset = 0 );
		void Unbind();
		void Render( unsigned int renderPrimitive = GL_TRIANGLES, unsigned int offset = 0, unsigned int numElems = 0 );

		GM_BIND_TYPEID(Vbo);

	private:
		bool		 m_bound;
		unsigned int m_id;
		unsigned int m_stride;
		unsigned int m_numElems;
		unsigned int m_numBytesTotal;
		unsigned int m_dataType; // static/dynamic

		StrongHandle< VertexAttribute > m_vertAttrib;
	};

	GM_BIND_DECL(Vbo);
}

#endif