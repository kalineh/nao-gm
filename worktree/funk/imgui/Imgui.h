#ifndef _INCLUDE_IMGUI_H_
#define _INCLUDE_IMGUI_H_

#include <math/v2i.h>
#include <math/v2i.h>
#include <math/v2.h>
#include <math/v3.h>
#include <common/StrongHandle.h>
#include <string>

namespace funk
{
	class LineGraph;
	class CubicSpline2d;

	struct Imgui
	{
		static const int AUTOSIZE = -1;

		static void		Begin( const char* name, v2i pos = v2i(500), v2i dimen = v2i(Imgui::AUTOSIZE) );
		static void		End();

		static void		Header( const char* name );
		static void		SameLine();
		static void		Separator();
		static void		Tab();
		static void		Untab();
		static void		Minimize();

		static bool		Button( const char* name );
		static bool		ButtonDown( const char* name );
		static bool		ButtonHover( const char* name );
		static bool		ButtonNoPadding( const char* name );
		static bool		ButtonDownNoPadding( const char* name );
		static bool		ButtonHoverNoPadding( const char* name );

		static bool		CheckBox( const char* name, bool &val );
		static int		Select( const char* names[], int &val, int numChoices ); // returns the index of the choices
		static int		SelectCustom( const char* names[], int* values, int &val, int numChoices ); // returns val inside "values"
		static int		DropDown( const char* name, bool &val );

		static float	SliderFloat( const char* name, float & val, float min, float max );
		static int		SliderInt( const char* name, int & val, int min, int max );
		static v2		SliderV2( const char* name, v2 & val, v2 min, v2 max );
		static v3		SliderV3( const char* name, v3 & val, v3 min, v3 max );
		static v3		SliderRGB( const char* name, v3 & val);
		static v3		SliderHSV( const char* name, v3 & rgb);

		static void		FillBarFloat( const char* name, float val, float min, float max );
		static void		FillBarInt( const char* name, int val, int min, int max );
		static void		FillBarV2( const char* name, v2 val, v2 min, v2 max );
		static void		FillBarV3( const char* name, v3 val, v3 min, v3 max );

		static int		TextInput( const char* name, std::string& val, int width = 150 );
		static void		Print( const char * text );
		static void		PrintParagraph( const char * text );

		static void		LineGraph( StrongHandle<LineGraph> lineGraph );
		static void		CubicSpline2d( StrongHandle<CubicSpline2d> lineGraph, v2 minPos, v2 maxPos, v2i dimen=v2i(300) );

		// advanced features
		static void		SetScrollX( float x /*[0,1]*/);
		static void		SetScrollY( float y /*[0,1]*/);
		static bool		IsWindowActive();
		static bool		IsWidgetActive(int id);
		static bool		IsWorkingWindowNew();
		static int		GetPrevWidgetId();
		static void		SetActiveWidgetId(int id); // dangerous
		static void		Lock( const char * gui );
		static void		Unlock( const char * gui );

		static int FONT_WIDTH;
		static int FONT_HEIGHT;
	};
}

#endif