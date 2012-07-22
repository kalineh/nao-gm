#ifndef _INCLUDE_FRAG_SHADER_H_
#define _INCLUDE_FRAG_SHADER_H_

#include <string>
#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>

namespace funk
{
	class FragShader : public HandledObj<FragShader>
	{
	public:
		FragShader( const char * file );
		~FragShader();

		unsigned int GetId() const { return m_id; }

		GM_BIND_TYPEID(FragShader);
		
	private:

		unsigned int m_id;
		std::string m_file;
	};

	GM_BIND_DECL( FragShader );
}

#endif