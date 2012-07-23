#ifndef _INCLUDE_CORE_H_
#define _INCLUDE_CORE_H_

#include <common/BaseEntityGroup.h>
#include <common/StrongHandle.h>
#include <gfx/LineGraph.h>

namespace funk
{
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