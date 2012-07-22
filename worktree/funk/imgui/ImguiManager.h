#ifndef _INCLUDE_IMGUI_MANAGER_H_
#define _INCLUDE_IMGUI_MANAGER_H_

#include <map>
#include <string>

#include <common/Singleton.h>
#include <common/StrongHandle.h>

#include <gfx/Font.h>
#include <gfx/Cam2d.h>
#include "ImguiWindow.h"

namespace funk
{
	class ImguiManager : public Singleton<ImguiManager>
	{
	public:

		struct State
		{
			union
			{
				float			f;
				int				i;
				unsigned int	uint;
				struct { float fvec[4]; };
				struct { int   ivec[4]; };
			} data;

			v2i	drawPos;
			v2i	drawPosPrev;
			v2i scrollOffset;
			int numTabs;	// tabs spacing for drawing

			// where the mouse can interact
			v2i mouseRegionStart;
			v2i mouseRegionDimen;

			// widget
			static const int WIDGET_NULL = -1;			
			int		activeWidgetId;
			int		widgetCount;
			
			StrongHandle<ImguiWindow> activeWindow;
			StrongHandle<ImguiWindow> workingWindow;
			std::string workingWindowTitle;

			void Reset();

		} state;

		void CleanUp(); // clean up unused windows
		void ClearActiveWindow();
		void ClearAllWindows();

		Font & GetFont() { return m_font; }
		Cam2d & GetCam() { return m_cam; }

		StrongHandle<ImguiWindow> GetWindow( const char* name );

	private:
		friend Singleton<ImguiManager>;
		ImguiManager();
		~ImguiManager();

		// all possible windows
		std::map< std::string, StrongHandle<ImguiWindow> > m_windowMap;

		Cam2d m_cam;
		Font m_font;
	};
}

#endif