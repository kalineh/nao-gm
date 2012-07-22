#include "ImguiGM.h"

#include <gm/gmThread.h>
#include <gm/gmMachine.h>
#include <gm/gmHelpers.h>
#include <gm/gmBind.h>
#include <gm/gmUtilEx.h>
#include <gm/gmArrayLib.h>

#include <gfx/LineGraph.h>
#include <math/CubicSpline2d.h>

#include <vm/VirtualMachine.h>

#include "ImguiManager.h"
#include "Imgui.h"


namespace funk
{

void OutputGmTable( gmTableObject* table, gmUserArray* selectArr, int level, bool showFunctions )
{
	if( level >= selectArr->Size() ) return;

	std::vector<gmVariable*> keys;
	gmSortTableKeys(table, keys);

	const int charsPerTab = 2;
	const int dotPadding = 64 - level*charsPerTab;

	for( size_t i = 0; i < keys.size(); ++i )
	{
		gmVariable & key = *keys[i];
		gmVariable val = table->Get(key);
		int getLevelVal = selectArr->GetAt(level).GetInt();
		bool selected = getLevelVal == i;

		if ( !showFunctions && val.IsFunction() ) continue;

		char buffer[256];
		const char* strKey = key.AsString( &VirtualMachine::Get()->GetVM(), buffer, sizeof(buffer) );

		if ( val.IsTable() )
		{
			bool dropDownSelected = selected;
			dropDownSelected = Imgui::DropDown( strKey, dropDownSelected ) != 0;

			// clicked down
			if ( dropDownSelected != selected )
			{
				selectArr->SetAt(level, gmVariable(dropDownSelected ? (int)i : -1 ));

				for( int j = level+1; j < selectArr->Size(); ++j )
				{
					selectArr->SetAt(j, gmVariable(-1) );
				}
			}

			selected = dropDownSelected;
		}
		else
		{
			Imgui::Print(" - ");
			Imgui::SameLine();
			Imgui::Print(strKey);
		}

		// dots
		int keyLen = strlen(strKey);
		int numDots = dotPadding-keyLen;
		memset( buffer, '.', numDots );
		buffer[numDots] = 0;

		Imgui::SameLine();
		Imgui::Print(buffer);
		Imgui::SameLine();
		const char* strVal = val.AsString( &VirtualMachine::Get()->GetVM(), buffer, sizeof(buffer) );
		Imgui::Print( strVal );

		if ( val.IsTable() )
		{
			gmTableObject * childTable = val.GetTableObjectSafe();

			// print table count
			char tableCountBuffer[8];
			sprintf( tableCountBuffer, "(%d)", childTable->Count() );
			Imgui::SameLine();
			Imgui::Print(tableCountBuffer);

			// if table, print it
			if ( selected )
			{
				Imgui::Tab();
				OutputGmTable(childTable, selectArr, level+1, showFunctions);
				Imgui::Untab();
			}
		}
	}
}

void ImguiOutputTable(gmTableObject* table, gmUserArray* selectArr, bool showFunctions)
{
	OutputGmTable( table, selectArr, 0, showFunctions );
}

struct gmfImguiLib
{
	GM_MEMFUNC_DECL(Begin)
	{
		GM_CHECK_STRING_PARAM(name, 0);
		GM_FLOAT_OR_INT_PARAM(posX, 1, 500);
		GM_FLOAT_OR_INT_PARAM(posY, 2, 500);
		GM_FLOAT_OR_INT_PARAM(dimenX, 3, (float)Imgui::AUTOSIZE);
		GM_FLOAT_OR_INT_PARAM(dimenY, 4, (float)Imgui::AUTOSIZE);

		Imgui::Begin(name, v2i((int)posX, (int)posY), v2i((int)dimenX, (int)dimenY) );

		return GM_OK;
	}

	GM_MEMFUNC_DECL(End)
	{
		GM_CHECK_NUM_PARAMS(0);
		Imgui::End();
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Button)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );
		a_thread->PushInt( Imgui::Button(str) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(ButtonDown)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );
		a_thread->PushInt( Imgui::ButtonDown(str) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(ButtonHover)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );
		a_thread->PushInt( Imgui::ButtonHover(str) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(ButtonNoPadding)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );
		a_thread->PushInt( Imgui::ButtonNoPadding(str) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(ButtonDownNoPadding)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );
		a_thread->PushInt( Imgui::ButtonDownNoPadding(str) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(ButtonHoverNoPadding)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );
		a_thread->PushInt( Imgui::ButtonHoverNoPadding(str) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(CheckBox)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_INT_PARAM( val, 1 );

		bool enabled = val != 0;
		a_thread->PushInt( Imgui::CheckBox(str, enabled) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(DropDown)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_INT_PARAM( val, 1 );

		bool enabled = val != 0;
		a_thread->PushInt( Imgui::DropDown(str, enabled) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Select)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_TABLE_PARAM(table, 0);
		GM_CHECK_INT_PARAM(choice, 1);

		const int MAX_CHOICES = 128;
		const char * selections[MAX_CHOICES];
		int numChoices = table->Count();

		// gather all strings
		gmTableIterator it;
		gmTableNode * childNode = table->GetFirst( it );
		int i = 0;
		while ( !table->IsNull(it) )
		{
			selections[i] = childNode->m_value.GetCStringSafe();	
			++i;
			childNode = table->GetNext(it);
		}

		a_thread->PushInt( Imgui::Select(selections, choice, numChoices) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SelectCustom)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_TABLE_PARAM(table, 0);
		GM_CHECK_INT_PARAM(choice, 1);

		const int MAX_CHOICES = 64;

		const char * selections[MAX_CHOICES];
		int values[MAX_CHOICES];
		int numChoices = table->Count();
		int index;

		// gather all strings
		gmTableIterator it;
		gmTableNode * childNode = table->GetFirst( it );
		int i = 0;
		while ( !table->IsNull(it) )
		{
			selections[i] = childNode->m_key.GetCStringSafe();	
			values[i] = childNode->m_value.GetIntSafe();	

			if ( values[i] == choice ) index = i;

			++i;
			childNode = table->GetNext(it);
		}

		
		Imgui::Select(selections, index, numChoices);
		a_thread->PushInt( values[index] );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SliderFloat)
	{
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_FLOAT_PARAM( val, 1 );
		GM_CHECK_FLOAT_PARAM( min, 2 );
		GM_CHECK_FLOAT_PARAM( max, 3 );

		str = strlen(str)==0? NULL : str;
		a_thread->PushFloat( Imgui::SliderFloat(str, val, min,max) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SliderInt)
	{
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_INT_PARAM( val, 1 );
		GM_CHECK_INT_PARAM( min, 2 );
		GM_CHECK_INT_PARAM( max, 3 );

		str = strlen(str)==0? NULL : str;
		a_thread->PushInt( Imgui::SliderInt(str, val, min, max) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SliderV2)
	{
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_VEC2_PARAM( val, 1 );
		GM_CHECK_VEC2_PARAM( min, 2 );
		GM_CHECK_VEC2_PARAM( max, 3 );

		str = strlen(str)==0? NULL : str;
		a_thread->PushVec2( Imgui::SliderV2(str, val, min, max) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SliderV3)
	{
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_VEC3_PARAM( val, 1 );
		GM_CHECK_VEC3_PARAM( min, 2 );
		GM_CHECK_VEC3_PARAM( max, 3 );

		str = strlen(str)==0? NULL : str;
		a_thread->PushVec3( Imgui::SliderV3(str, val, min, max) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SliderRGB)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_VEC3_PARAM( val, 1 );

		str = strlen(str)==0? NULL : str;
		a_thread->PushVec3( Imgui::SliderRGB(str, val) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SliderHSV)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_VEC3_PARAM( val, 1 );

		str = strlen(str)==0? NULL : str;
		a_thread->PushVec3( Imgui::SliderHSV(str, val) );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(FillBarFloat)
	{
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_FLOAT_PARAM( val, 1 );
		GM_CHECK_FLOAT_PARAM( min, 2 );
		GM_CHECK_FLOAT_PARAM( max, 3 );

		str = strlen(str)==0? NULL : str;
		Imgui::FillBarFloat(str, val, min,max) ;
		return GM_OK;
	}

	GM_MEMFUNC_DECL(FillBarV2)
	{
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_VEC2_PARAM( val, 1 );
		GM_CHECK_VEC2_PARAM( min, 2 );
		GM_CHECK_VEC2_PARAM( max, 3 );

		str = strlen(str)==0? NULL : str;
		Imgui::FillBarV2(str, val, min,max) ;
		return GM_OK;
	}

	GM_MEMFUNC_DECL(FillBarV3)
	{
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_VEC3_PARAM( val, 1 );
		GM_CHECK_VEC3_PARAM( min, 2 );
		GM_CHECK_VEC3_PARAM( max, 3 );

		str = strlen(str)==0? NULL : str;
		Imgui::FillBarV3(str, val, min,max) ;
		return GM_OK;
	}

	GM_MEMFUNC_DECL(FillBarInt)
	{
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_STRING_PARAM( str, 0 );
		GM_CHECK_INT_PARAM( val, 1 );
		GM_CHECK_INT_PARAM( min, 2 );
		GM_CHECK_INT_PARAM( max, 3 );

		str = strlen(str)==0? NULL : str;
		Imgui::FillBarInt(str, val, min,max) ;
		return GM_OK;
	}

	GM_MEMFUNC_DECL(TextInput)
	{
		GM_CHECK_STRING_PARAM( title, 0 );
		GM_CHECK_STRING_PARAM( str, 1 );
		GM_INT_PARAM( width, 2, 150 );

		static std::string text;
		text = str;
		Imgui::TextInput(title, text, width);

		a_thread->PushNewString(text.c_str(), text.size());
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Print)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );

		Imgui::Print(str);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Header)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );

		Imgui::Header(str);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Tab)
	{
		GM_CHECK_NUM_PARAMS(0);
		Imgui::Tab();
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Untab)
	{
		GM_CHECK_NUM_PARAMS(0);
		Imgui::Untab();
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Minimize)
	{
		GM_CHECK_NUM_PARAMS(0);
		Imgui::Minimize();
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Separator)
	{
		GM_CHECK_NUM_PARAMS(0);
		Imgui::Separator();
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SameLine)
	{
		GM_CHECK_NUM_PARAMS(0);
		Imgui::SameLine();
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Lock)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );
		Imgui::Lock(str);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(Unlock)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( str, 0 );
		Imgui::Unlock(str);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(IsWindowActive)
	{
		GM_CHECK_NUM_PARAMS(0);
		a_thread->PushInt(Imgui::IsWindowActive());
		return GM_OK;
	}

	GM_MEMFUNC_DECL(DisplayTable)
	{
		GM_CHECK_TABLE_PARAM(table, 0);
		GM_CHECK_USER_PARAM(gmUserArray*, GM_ARRAY, selectArr, 1 );
		GM_INT_PARAM(showFunctions, 2, 0);

		ImguiOutputTable( table, selectArr, showFunctions>0 );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(LineGraph)
	{
		GM_CHECK_NUM_PARAMS(0);
		GM_CHECK_USER_PARAM_PTR( LineGraph, lineGraph, 0 );

		Imgui::LineGraph( lineGraph );

		return GM_OK;
	}

	GM_MEMFUNC_DECL(CubicSpline2d)
	{
		GM_CHECK_USER_PARAM_PTR( CubicSpline2d, spline, 0 );
		GM_CHECK_VEC2_PARAM( minPos, 1 );
		GM_CHECK_VEC2_PARAM( maxPos, 2 );
		GM_INT_PARAM( width, 3, 300 );
		GM_INT_PARAM( height, 4, 300 );

		Imgui::CubicSpline2d( spline, minPos, maxPos, v2i(width,height) );

		return GM_OK;
	}
};

static gmFunctionEntry s_gmImguiLib[] = 
{
	GM_LIBFUNC_ENTRY(Begin, Imgui)
	GM_LIBFUNC_ENTRY(End, Imgui)

	GM_LIBFUNC_ENTRY(Button, Imgui)
	GM_LIBFUNC_ENTRY(ButtonDown, Imgui)
	GM_LIBFUNC_ENTRY(ButtonHover, Imgui)
	GM_LIBFUNC_ENTRY(ButtonNoPadding, Imgui)
	GM_LIBFUNC_ENTRY(ButtonDownNoPadding, Imgui)
	GM_LIBFUNC_ENTRY(ButtonHoverNoPadding, Imgui)
	GM_LIBFUNC_ENTRY(CheckBox, Imgui)
	GM_LIBFUNC_ENTRY(DropDown, Imgui)
	GM_LIBFUNC_ENTRY(Select, Imgui)
	GM_LIBFUNC_ENTRY(SelectCustom, Imgui)
	GM_LIBFUNC_ENTRY(SliderFloat, Imgui)
	GM_LIBFUNC_ENTRY(SliderInt, Imgui)
	GM_LIBFUNC_ENTRY(SliderV2, Imgui)
	GM_LIBFUNC_ENTRY(SliderV3, Imgui)
	GM_LIBFUNC_ENTRY(SliderRGB, Imgui)
	GM_LIBFUNC_ENTRY(SliderHSV, Imgui)
	GM_LIBFUNC_ENTRY(FillBarFloat, Imgui)
	GM_LIBFUNC_ENTRY(FillBarInt, Imgui)
	GM_LIBFUNC_ENTRY(FillBarV2, Imgui)
	GM_LIBFUNC_ENTRY(FillBarV3, Imgui)
	GM_LIBFUNC_ENTRY(TextInput, Imgui)
	GM_LIBFUNC_ENTRY(LineGraph, Imgui)
	GM_LIBFUNC_ENTRY(CubicSpline2d, Imgui)
	GM_LIBFUNC_ENTRY(Header, Imgui)
	GM_LIBFUNC_ENTRY(Print, Imgui)
	GM_LIBFUNC_ENTRY(Tab, Imgui)
	GM_LIBFUNC_ENTRY(Untab, Imgui)
	GM_LIBFUNC_ENTRY(Minimize, Imgui)
	GM_LIBFUNC_ENTRY(Separator, Imgui)
	GM_LIBFUNC_ENTRY(SameLine, Imgui)
	GM_LIBFUNC_ENTRY(Lock, Imgui)
	GM_LIBFUNC_ENTRY(Unlock, Imgui)
	GM_LIBFUNC_ENTRY(IsWindowActive, Imgui)
	GM_LIBFUNC_ENTRY(DisplayTable, Imgui)
};

void gmBindImguiLib( gmMachine * a_machine )
{
	a_machine->RegisterLibrary(s_gmImguiLib, sizeof(s_gmImguiLib) / sizeof(s_gmImguiLib[0]), "Gui" );
}
}