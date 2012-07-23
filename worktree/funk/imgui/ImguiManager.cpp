#include "ImguiManager.h"
#include "Imgui.h"

#include <common/ResourcePath.h>
#include <common/Window.h>
#include <assert.h>

namespace funk
{

// how many frames until dead
const int DEAD_WINDOW_NUM_FRAMES = 10000;

ImguiManager::ImguiManager()
{
	m_font.Init( RESOURCE_PATH("common/img/font/droid_10.png"), 10 );
	Imgui::FONT_WIDTH = m_font.GetCharWidth('B');
	Imgui::FONT_HEIGHT = m_font.GetHeight();

	m_cam.InitScreenSpace();

	state.Reset();

	const v2i windowSize = Window::Get()->Sizei();
	state.mouseRegionStart = v2i(0, windowSize.y);
	state.mouseRegionDimen = windowSize;
}

ImguiManager::~ImguiManager()
{
}

StrongHandle<ImguiWindow> ImguiManager::GetWindow( const char* name )
{
	assert(name);

	std::map< std::string, StrongHandle<ImguiWindow> >::iterator it = m_windowMap.find(name);

	if ( it == m_windowMap.end() )
	{
		StrongHandle<ImguiWindow> result = new ImguiWindow();
		m_windowMap[name] = result;
		return result;
	}

	return it->second;
}

void ImguiManager::CleanUp()
{
	std::map< std::string, StrongHandle<ImguiWindow> >::iterator it = m_windowMap.begin();

	while( it != m_windowMap.end() )
	{
		// if not updated, kill it!
		if ( it->second->framesSinceUpdate > 1 )
		{
			// make sure it is not same window
			if ( state.activeWindow == it->second )
			{
				ClearActiveWindow();
			}

			if ( it->second->framesSinceUpdate > DEAD_WINDOW_NUM_FRAMES )
			{
				m_windowMap.erase(it++);
				continue;
			}
		}

		++it->second->framesSinceUpdate;
		++it;
	}
}

void ImguiManager::ClearActiveWindow()
{
	state.activeWindow  = NULL;
	state.widgetCount = 0;
}

void ImguiManager::ClearAllWindows()
{
	ClearActiveWindow();
	State().Reset();
	m_windowMap.clear();
}

void ImguiManager::State::Reset()
{
	activeWidgetId = State::WIDGET_NULL;
	memset( &data, 0, sizeof(data) );
	activeWindow = NULL;
	widgetCount = 0;
}

}