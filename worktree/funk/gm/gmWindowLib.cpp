#include "gmWindowLib.h"

#include "gmThread.h"
#include "gmMachine.h"
#include "gmHelpers.h"

#include <gm/gmBind.h>
#include <common/Window.h>
#include <math/v2.h>

namespace funk
{
struct gmfWindowLib
{
	static int GM_CDECL gmfGetDimen(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		a_thread->PushVec2( Window::Get()->Sizef() );

		return GM_OK;
	}

	static int GM_CDECL gmfGetMonitorDimen(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);

		v2i dimen = Window::Get()->MonitorSizei();
		a_thread->PushVec2( v2( (float)dimen.x, (float)dimen.y ) );

		return GM_OK;
	}

	static int GM_CDECL gmfIsMouseFocus(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		a_thread->PushInt( Window::Get()->IsMouseFocus() ? 1:0 );
		return GM_OK;
	}

	static int GM_CDECL gmfIsKeyboardFocus(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		a_thread->PushInt( Window::Get()->IsKeyboardFocus() ? 1:0 );
		return GM_OK;
	}

	static int GM_CDECL gmfIsAppActive(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		a_thread->PushInt( Window::Get()->IsAppActive() ? 1:0 );
		return GM_OK;
	}

	static int GM_CDECL gmfIsFullScreen(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		a_thread->PushInt( Window::Get()->IsFullScreen() ? 1:0 );
		return GM_OK;
	}

	static int GM_CDECL gmfSetTitle(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( title, 0 );
		Window::Get()->SetTitle(title);

		return GM_OK;
	}
};

static gmFunctionEntry s_gmWindowLib[] = 
{
	GM_LIBFUNC_ENTRY(GetDimen, Window)
	GM_LIBFUNC_ENTRY(GetMonitorDimen, Window)
	GM_LIBFUNC_ENTRY(IsMouseFocus, Window)
	GM_LIBFUNC_ENTRY(IsKeyboardFocus, Window)
	GM_LIBFUNC_ENTRY(IsAppActive, Window)
	GM_LIBFUNC_ENTRY(IsFullScreen, Window)
	GM_LIBFUNC_ENTRY(SetTitle, Window)	
};

void gmBindWindowLib( gmMachine * a_machine )
{
	a_machine->RegisterLibrary(s_gmWindowLib, sizeof(s_gmWindowLib) / sizeof(s_gmWindowLib[0]), "Window" );
}

}