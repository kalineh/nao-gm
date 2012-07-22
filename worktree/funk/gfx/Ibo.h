#ifndef _INCLUDE_IBO_H_
#define _INCLUDE_IBO_H_

#include <gl/glew.h>
#include <common/HandledObj.h>

namespace funk
{
	class Ibo : public HandledObj<Ibo>
	{
	public:
		Ibo(	const unsigned int* data, 
				unsigned int numElems, 
				unsigned int type = GL_STATIC_DRAW );
		~Ibo();

		void Bind();
		void Unbind();
		void Render(unsigned int renderPrimitive = GL_TRIANGLES, 
					unsigned int numElems = 0,
					unsigned int offset = 0 );

	private:
		unsigned int m_id;
		unsigned int m_numElems;
		unsigned int m_type;
	};
}
#endif