#ifndef _INCLUDE_CORE_H_
#define _INCLUDE_CORE_H_

#include <common/BaseEntityGroup.h>
#include <common/StrongHandle.h>
#include <gfx/LineGraph.h>

#define ASSERT(condition) \
    if (!condition) assert(false) //if (!condition) { __asm { int 3; } }

#define GM_AL_EXCEPTION_WRAPPER(code) \
    try { code ; } catch (const AL::ALError& e) { GM_EXCEPTION_MSG(e.what()); return GM_OK; }

namespace funk
{
    class LineGraph;
    class Texture;
    class Cam2d;
    class Cam3d;
    class Font;
    class VertShader;
    class FragShader;
    class Ibo;
    class Vbo;
    class IniReader;
    class Input;
    class LineGraph;
    class Perlin;
    class Particles2d;
    class Renderer;
    class ShaderProgram;
    class Sound;
    class SoundMngr;
    class Timer;

	class Core : public BaseEntityGroup
	{
	public:
		Core();

		void HandleArgs( int argc, char ** argv );
		void Init();
		void Deinit();
		void Update( float dt );
		void Render();
		void Run();

	private:

		void HandleInputs();

		void InitGuiGraphs();
		void UpdateGuiGraphs();
		void GuiGraphs();
		void GuiStats();

		bool m_showAnalyticsGui;
		bool m_gameRunning;
		int m_fps;

		float m_msUpdate;
		float m_msRender;
		float m_msGpu;
		float m_msTotal;
		float m_fpsFrame;

		bool m_showGraphsGui;
		StrongHandle<LineGraph> m_lineGraphRender;
		StrongHandle<LineGraph> m_lineGraphUpdate;
		StrongHandle<LineGraph> m_lineGraphTotal;
		StrongHandle<LineGraph> m_lineGraphGpu;
	};
}

#endif