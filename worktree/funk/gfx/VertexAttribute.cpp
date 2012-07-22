#include "VertexAttribute.h"

#include <common/Debug.h>

namespace funk
{
void VertexAttribute::Bind( unsigned int offset )
{
	size_t nElems = m_vertDesc.size();

	for ( size_t i = 0; i < nElems; ++i )
	{
		Descriptor & desc = m_vertDesc[i];
		GLuint dataType = GL_BYTE + (desc.dataType - ATTRIB_BYTE );
		unsigned totalOffset = offset + desc.offsetBytes;

		switch ( desc.type )
		{
			case ATTRIB_VERTEX: 
				glEnableClientState( GL_VERTEX_ARRAY );
				glVertexPointer( desc.numElemsPerVert, dataType, desc.strideBytes, (GLvoid*)( totalOffset ) );
				break;

			case ATTRIB_COLOR: 
				glEnableClientState( GL_COLOR_ARRAY );
				glColorPointer( desc.numElemsPerVert, dataType, desc.strideBytes, (GLvoid*)( totalOffset ) );
				break;

			case ATTRIB_NORMAL: 
				glEnableClientState( GL_NORMAL_ARRAY );
				glNormalPointer( dataType, desc.strideBytes, (GLvoid*)( totalOffset ) );
				break;

			case ATTRIB_TEXTURE_0: 
			case ATTRIB_TEXTURE_1: 
			case ATTRIB_TEXTURE_2: 
			case ATTRIB_TEXTURE_3: 
			{
				glEnableClientState( GL_TEXTURE_COORD_ARRAY );
				GLenum glActiveTexEnum = GL_TEXTURE0 + ( desc.type - ATTRIB_TEXTURE_0);
				glClientActiveTexture( glActiveTexEnum );
				glTexCoordPointer( desc.numElemsPerVert, dataType, desc.strideBytes, (GLvoid*)( totalOffset ) );
				break;
			}

			default: CHECK(false, "Vertex attribute not recognized"); break;
		}
	}
}

void VertexAttribute::AddAttrib( VertexType type, DataType dataType, unsigned int numElemsPerVert, unsigned int strideBytes, unsigned int offsetBytes )
{
	m_vertDesc.resize( m_vertDesc.size() + 1 );

	Descriptor & desc = m_vertDesc.back();
	desc.type = type;
	desc.dataType = dataType;
	desc.numElemsPerVert = numElemsPerVert;
	desc.strideBytes = strideBytes;
	desc.offsetBytes = offsetBytes;
}

void VertexAttribute::Unbind()
{
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
}
}