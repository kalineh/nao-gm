#ifndef _INCLUDE_FBO_H_
#define _INCLUDE_FBO_H_

#include <gl/glew.h>
#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>

namespace funk
{
	// fwd decl
	class Texture;

	class Fbo : public HandledObj<Fbo>
	{
	public:
		Fbo( int width, int height );
		~Fbo();

		int Width() const { return m_width; }
		int Height() const { return m_height; }

		void Begin();
		void End();

		void SetColorTex( StrongHandle<Texture> tex );
		StrongHandle<Texture> GetTex() const { return m_tex; }

		GM_BIND_TYPEID(Fbo);
		
	private:

		bool m_boundAsTarget; // bound as render target
		int m_width, m_height;

		void InitDepthBuffer();
		void InitTexture();
		void InitFrameBuffer();

		unsigned int m_fbo;
		unsigned int m_fboDepth;

		StrongHandle<Texture> m_tex;
	};

	GM_BIND_DECL(Fbo);
}

#endif