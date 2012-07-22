#ifndef _INCLUDE_VERTEXATTRIBUTE_H_
#define _INCLUDE_VERTEXATTRIBUTE_H_

#include <gl/glew.h>
#include <vector>
#include <common/HandledObj.h>

namespace funk
{
	class VertexAttribute : public HandledObj<VertexAttribute>
	{
	public:

		enum VertexType
		{
			ATTRIB_VERTEX,
			ATTRIB_COLOR,
			ATTRIB_NORMAL,
			ATTRIB_TEXTURE_0,
			ATTRIB_TEXTURE_1,
			ATTRIB_TEXTURE_2,
			ATTRIB_TEXTURE_3,
			ATTRIB_TEXTURE_4,
			ATTRIB_TEXTURE_5,
			ATTRIB_TEXTURE_6
		};

		enum DataType
		{
			ATTRIB_BYTE,
			ATTRIB_UNSIGNED_BYTE,
			ATTRIB_SHORT,
			ATTRIB_UNSIGNED_SHORT,
			ATTRIB_INT,
			ATTRIB_UNSIGNED_INT,
			ATTRIB_FLOAT
		};

		void Bind( unsigned int offset = 0);
		void Unbind();

		void AddAttrib(	VertexType type, 
						DataType dataType, 
						unsigned int numElems, 
						unsigned int strideBytes, 
						unsigned int offsetBytes );
	
	private:
		struct Descriptor 
		{
			VertexType type;
			DataType dataType;
			unsigned int numElemsPerVert;
			unsigned int strideBytes;
			unsigned int offsetBytes;
		};

		std::vector< Descriptor > m_vertDesc;
	};
}

#endif