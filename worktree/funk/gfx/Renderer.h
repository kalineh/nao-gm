#ifndef _INCLUDE_RENDERER_H_
#define _INCLUDE_RENDERER_H_

#include <common/Singleton.h>
#include "ShaderProgram.h"
#include <common/StrongHandle.h>

namespace funk
{
	class Renderer : public Singleton<Renderer>
	{
	public:

		void Update();
		void Present();

		// Shader
		bool IsShaderBound() const { return m_defaultShader || m_shaderProgram; }
		void BeginDefaultShader();
		void EndDefaultShader();
		unsigned int GetShaderProgramId() const { return m_shaderProgram->GetId(); }

		void GetInfo( char * buffer, int len );

		// Error
		void HandleErrorDraw();

	private:
		friend Singleton<Renderer>;
		Renderer();
		~Renderer();

		bool m_defaultShader;
		friend class ShaderProgram;
		StrongHandle<ShaderProgram> m_shaderProgram;

		// Error handling
		bool m_bCheckerror;
	};
}

#endif