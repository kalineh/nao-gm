#include "ImguiWindow.h"

namespace funk
{

ImguiWindow::ImguiWindow()
{
	framesSinceUpdate = 0;
	titleBarMinWidth = 0;
	minimized = false;
	isNewWindow = true;
	autosize = false;
	locked = false;
	scrollPos = v2(0);
	dimenAutosize = v2i(0);
}
}