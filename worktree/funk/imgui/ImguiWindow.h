#ifndef _INCLUDE_IMGUI_WINDOW_H_
#define _INCLUDE_IMGUI_WINDOW_H_

#include <math/v2i.h>
#include <math/v2.h>
#include <common/HandledObj.h>

namespace funk
{
	struct ImguiWindow : public HandledObj<ImguiWindow>
	{
		v2i		pos;
		v2i		dimen;
		v2i		dimenAutosize;		// autosize calculated
		v2		scrollPos;
		int		titleBarMinWidth;	// minimum title bar width
		
		bool	autosize;
		bool	minimized;
		bool	isNewWindow; // first time
		bool	locked;
		int	framesSinceUpdate; // >1 means non-active

		ImguiWindow();
	};
}

#endif