#ifndef _INCLUDE_VERTEX_SHADER_H_
#define _INCLUDE_VERTEX_SHADER_H_

#include <string>
#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>

namespace funk
{
	class VertexShader : public HandledObj<VertexShader>
	{
	public:

		VertexShader( const char * file );
		~VertexShader();

		unsigned int GetId() const { return m_id; }

		GM_BIND_TYPEID(VertexShader);
		
	private:
		unsigned int m_id;
		std::string m_file;
	};

	GM_BIND_DECL( VertexShader );
}

#endif