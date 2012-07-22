#ifndef _INCLUDE_SHADER_PROGRAM_H_
#define _INCLUDE_SHADER_PROGRAM_H_

#include <string>
#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>

namespace funk
{
	class FragShader;
	class VertexShader;

	class ShaderProgram : public HandledObj<ShaderProgram>
	{
	public:
		ShaderProgram();
		~ShaderProgram();

		// link the shaders to program
		void Link();

		void AttachVertexShader( const char * name );
		void AttachVertexShader( StrongHandle<VertexShader> vp );
		void AttachFragShader( const char * name );
		void AttachFragShader( StrongHandle<FragShader> fp );

		void Begin();
		void End();

		int GetUniformId( const char * name ) const;
		void SetUniform( int uniformId, float x, float y, float z, float w );
		void SetUniform( int uniformId, int x, int y, int z, int w );
		void SetUniform( int uniformId, int i );

		unsigned int GetId() const { return m_id; }

		GM_BIND_TYPEID(ShaderProgram);
		
	private:
	
		bool m_linked;
		bool m_bound;

		void CheckLinkable();

		StrongHandle<VertexShader> m_vp;
		StrongHandle<FragShader> m_fp;

		unsigned int m_id;
		std::string m_file;
	};

	GM_BIND_DECL( ShaderProgram );
}

#endif