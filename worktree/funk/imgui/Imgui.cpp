#include "Imgui.h"

#include <cstring>
#include <assert.h>
#include <gl/glew.h>

#include "ImguiManager.h"
#include "ImguiWindow.h"

#include <gfx/Renderer.h>
#include <gfx/DrawPrimitives.h>
#include <gfx/LineGraph.h>
#include <math/CubicSpline2d.h>
#include <math/ColorUtil.h>
#include <common/Input.h>
#include <common/Timer.h>
#include <common/Window.h>
#include <common/Debug.h>

namespace funk
{
	int Imgui::FONT_WIDTH = 6;
	int Imgui::FONT_HEIGHT = 12;

	const int BUTTON_INSIDE_PADDING = 3;
	const int TAB_WIDTH = Imgui::FONT_WIDTH*2;

	const int WIDGET_PADDING = 4;
	const int WINDOW_INSIDE_PADDING = 6;
	const int TITLE_BAR_PADDING = 6;
	const int TITLE_BAR_HEIGHT = Imgui::FONT_HEIGHT+TITLE_BAR_PADDING*2;
	const int SCROLL_BAR_SIZE = 12;

	const int FILLBAR_WIDTH = 150;
	const int FILLBAR_TEXT_BUFFER = 2;
	const int FILLBAR_HEIGHT = Imgui::FONT_HEIGHT+FILLBAR_TEXT_BUFFER*2;

	const v3 COLOR_WINDOW_BG			= v3(0.15f);
	const v3 COLOR_SLIDER_BTN_BORDER	= v3(0.0f);
	const v3 COLOR_SLIDER_BG_BORDER		= v3(0.08f);
	const v3 COLOR_SLIDER				= v3(0.25f);
	const v3 COLOR_SLIDER_ACTIVE		= v3(0.40f);
	const v3 COLOR_BUTTON				= v3( 0.35f );
	const v3 COLOR_BUTTON_HOVER			= v3( 0.45f );
	const v3 COLOR_BUTTON_PRESS			= v3( 0.25f );
	const v3 COLOR_BAR					= v3(0.15f);
	const v3 COLOR_BAR_HOVER			= v3(0.30f);
	const v3 COLOR_FILLBAR				= v3(0.6f, 0.6f, 0.0f);
	const v3 COLOR_SEPARATOR			= v3(0.35f);
	const v3 COLOR_WHITE				= v3(1.0f);
	const v3 COLOR_BLACK				= v3(0.0f);

	const float MOUSEWHEEL_SCROLL_DELTA = 100.0f;

	enum ButtonState
	{
		BUTTON_NONE,
		BUTTON_HOVER,
		BUTTON_PRESS,
		BUTTON_DOWN
	};

	enum SpecialWidgetId
	{
		ID_SCROLL_X			= (1<<16)+0,
		ID_SCROLL_Y			= (1<<16)+1,
		ID_RESIZE_BUTTON	= (1<<16)+2,
	};

	inline v2 toV2( v2i v ) { return v2((float)v.x, (float)v.y); }
	inline v2i toV2i( v2 v ) { return v2i((int)v.x, (int)v.y); }

	inline StrongHandle<ImguiWindow> &ImguiWorkingWindow() { return ImguiManager::Get()->state.workingWindow; }
	inline int	ImguiTextWidth(const char* c) { return Imgui::FONT_WIDTH*strlen(c); }
	inline bool ImguiIsMinized() { assert(ImguiWorkingWindow()); return ImguiWorkingWindow()->minimized || ImguiWorkingWindow()->locked; }
	inline void ImguiPrint( const char* text, v2i pos ) { ImguiManager::Get()->GetFont().Print(text,toV2(pos)); }
	inline ImguiManager::State &ImguiState() { return  ImguiManager::Get()->state; }

	inline bool ImguiMouseDown()
	{
		return Input::Get()->IsMouseDown(1);
	}

	inline bool ImguiDidMouseJustGoDown()
	{
		return Input::Get()->DidMouseJustGoDown(1);
	}

	inline bool ImguiDidMouseJustGoUp()
	{
		return Input::Get()->DidMouseJustGoUp(1);
	}

	inline v2i ImguiGetMousePos()
	{
		return Input::Get()->GetMousePos() + ImguiState().scrollOffset;
	}

	bool ImguiMouseOver( v2i pos, v2i dimen )
	{
		if ( ImguiWorkingWindow()->locked ) return false;

		const v2i mousePos = ImguiGetMousePos();
		int mX = (int)mousePos.x;
		int mY = (int)mousePos.y;

		// mouse regions
		v2i regionPos = ImguiState().mouseRegionStart + ImguiState().scrollOffset;
		v2i regionDimen = ImguiState().mouseRegionDimen;

		return (mX >= pos.x) && (mX <= pos.x+dimen.x) && (mY <= pos.y) && (mY >= pos.y-dimen.y)
			&& (mX >= regionPos.x) && (mX <= regionPos.x+regionDimen.x) && (mY <= regionPos.y) && (mY >= regionPos.y-regionDimen.y);
	}

	inline bool ImguiMouseDoubleClick()
	{
		return Input::Get()->DidMouseDoubleClick();
	}

	inline bool ImguiIsWindowActive()
	{
		return ImguiWorkingWindow() == ImguiState().activeWindow;
	}

	inline bool ImguiIsWidgetActive(int id)
	{
		return ImguiIsWindowActive() && (ImguiManager::Get()->state.activeWidgetId == id );
	}

	inline int ImguiGenWidgetId()
	{
		return ++ImguiState().widgetCount;
	}

	inline void ImguiSetActiveWidgetId(int id)
	{
		if( id == ImguiManager::State::WIDGET_NULL )
		{
			// nothing
		}

		ImguiState().activeWidgetId = id;
	}

	inline void ImguiSetActiveWindow(StrongHandle<ImguiWindow> &window)
	{
		if ( window == NULL )
		{
			ImguiManager::Get()->ClearActiveWindow();
		}

		if ( ImguiState().activeWindow != window )
		{
			ImguiSetActiveWidgetId( ImguiManager::State::WIDGET_NULL );
		}

		ImguiState().activeWindow = window;
	}

	void MoveDrawPosBy( v2i dimen )
	{
		StrongHandle<ImguiWindow> &window = ImguiWorkingWindow();

		ImguiState().drawPosPrev = ImguiState().drawPos;
		ImguiState().drawPosPrev.x += dimen.x + WIDGET_PADDING;
		ImguiState().drawPos += v2i(dimen.x, -dimen.y);

		window->dimenAutosize.x = max( window->dimenAutosize.x, ImguiState().drawPos.x - window->pos.x );
		window->dimenAutosize.y = max( window->dimenAutosize.y, window->pos.y - ImguiState().drawPos.y );
	}

	inline void MoveDrawPosNextLine( v2i dimen )
	{
		StrongHandle<ImguiWindow> &window = ImguiWorkingWindow();

		MoveDrawPosBy( dimen + v2i(0,WIDGET_PADDING) );
		ImguiState().drawPos.x = window->pos.x + WINDOW_INSIDE_PADDING + TAB_WIDTH*ImguiState().numTabs;
	}

	inline void ImguiDrawRect( v2i pos, v2i dimen )
	{
		DrawRect( toV2(pos-v2i(0,dimen.y)), toV2(dimen) );
	}

	inline void ImguiDrawRectWire( v2i pos, v2i dimen )
	{
		DrawRectWire( toV2(pos-v2i(0,dimen.y)), toV2(dimen) );
	}

	inline void ImguiDrawLine( v2i start, v2i end )
	{
		DrawLine( toV2(start), toV2(end) );
	}

	inline void ImguiDrawPoint( v2i pt, float radius )
	{
		DrawCircle(toV2(pt), radius);
	}

	inline void ImguiColor( v3 color, float alpha = 1.0f )
	{
		glColor4f( color.x, color.y, color.z, alpha );
	}

	ButtonState ImguiButton( const char* name, int padding = BUTTON_INSIDE_PADDING )
	{
		if( ImguiIsMinized() ) return BUTTON_NONE;

		const v3 COLOR_BORDER = v3( 0.15f );
		const v3 COLOR_TEXT = COLOR_WHITE;

		ButtonState result = BUTTON_NONE;
		const int id = ImguiGenWidgetId();
		const int buttonHeight = Imgui::FONT_HEIGHT+padding*2;

		const int fontWidth = ImguiTextWidth(name);
		const v2i pos = ImguiState().drawPos;
		const v2i dimen = v2i(fontWidth+padding*2, buttonHeight);
		v3 buttonColor = COLOR_BUTTON;

		// mouse go down
		if ( ImguiMouseOver(pos,dimen) )
		{
			buttonColor = COLOR_BUTTON_HOVER;
			result = BUTTON_HOVER;

			// mouse goes down on button
			if ( ImguiDidMouseJustGoDown() && ImguiIsWindowActive()  ) 
			{
				ImguiSetActiveWidgetId(id);
			}

			// fully clicked on button!
			else if ( ImguiDidMouseJustGoUp() && ImguiIsWidgetActive(id) ) 
			{ 
				result = BUTTON_PRESS;
				ImguiSetActiveWidgetId(ImguiManager::State::WIDGET_NULL);
			}
		}

		// released mouse
		else if ( ImguiDidMouseJustGoUp() )
		{
			if ( ImguiIsWidgetActive(id) )
			{
				ImguiSetActiveWidgetId(ImguiManager::State::WIDGET_NULL);
			}
		}

		// held down
		if ( ImguiIsWidgetActive(id) )
		{
			result = BUTTON_DOWN;
			buttonColor = COLOR_BUTTON_PRESS;
		}

		// draw button
		ImguiColor(buttonColor);
		ImguiDrawRect( pos, dimen );
		ImguiColor(COLOR_BORDER);
		ImguiDrawRectWire( pos, dimen );
		ImguiColor(COLOR_TEXT);
		ImguiPrint(name, pos+v2i(padding,-padding) );

		MoveDrawPosNextLine(dimen);

		return result;
	}

	void ImguiTitleBar(StrongHandle<ImguiWindow> &window, const char* name, int id)
	{
		const int MINIZE_BUTTON_PADDING = WIDGET_PADDING;
		const v3 COLOR_WINDOW_TITLE = v3(0.1f, 0.6f, 0.3f);
		const v3 COLOR_WINDOW_TITLE_INACTIVE = v3(0.1f, 0.6f, 0.3f)*0.5f;
		const int BUTTON_HEIGHT = Imgui::FONT_HEIGHT+BUTTON_INSIDE_PADDING*2;

		bool isWindowActive = ImguiIsWindowActive();
		const v2i mousePos = ImguiGetMousePos();

		// draw title bar background
		v2i titleBarDimen = v2i( window->dimen.x, TITLE_BAR_HEIGHT );
		float titleBarAlpha = isWindowActive ? 1.0f : 0.7f;
		v3 titleBarColor = isWindowActive ? COLOR_WINDOW_TITLE : COLOR_WINDOW_TITLE_INACTIVE;
		ImguiColor( titleBarColor, titleBarAlpha );
		ImguiDrawRect( window->pos, titleBarDimen );

		// draw title font
		int fontWidth = ImguiTextWidth(name);
		float fontAlpha = isWindowActive ? 1.0f: 0.6f;
		ImguiColor(COLOR_WHITE, fontAlpha);
		ImguiPrint(name, window->pos + v2i(TITLE_BAR_PADDING, -TITLE_BAR_PADDING) );

		// find position for button
		int buttonWidth = Imgui::FONT_WIDTH + BUTTON_INSIDE_PADDING*2;
		ImguiState().drawPos = window->pos+v2i(window->dimen.x - buttonWidth-WIDGET_PADDING/2, -(TITLE_BAR_HEIGHT-BUTTON_HEIGHT)/2);

		// calculate how big min title bar
		window->titleBarMinWidth = fontWidth + TITLE_BAR_PADDING + buttonWidth + MINIZE_BUTTON_PADDING; 

		// handle minimized (need to unmiminimize to draw the button)
		bool minimized = window->minimized;
		window->minimized = false;
		ButtonState minBtn = ImguiButton(minimized ? "+" : "-");
		minimized = minimized^(minBtn == BUTTON_PRESS);
		window->minimized = minimized;

		// Handle title bar drag
		if ( minBtn == BUTTON_NONE )
		{
			if ( ImguiMouseOver(window->pos, titleBarDimen) )
			{
				if( ImguiDidMouseJustGoDown() && ImguiIsWindowActive() )
				{
					// double click
					if ( ImguiMouseDoubleClick() )
					{
						window->minimized = !window->minimized;
					}

					ImguiSetActiveWidgetId(id);
					ImguiState().data.ivec[0] = mousePos.x - window->pos.x;
					ImguiState().data.ivec[1] = mousePos.y - window->pos.y;
				}
			}
		}
	}

	void ImguiWindowBG(StrongHandle<ImguiWindow> &window)
	{
		const float ALPHA_INACTIVE	= 0.6f;
		const float ALPHA_HOVER		= 0.7f;
		const float ALPHA_ACTIVE	= 0.97f;

		float bgAlpha = ALPHA_INACTIVE;

		ImguiManager * imgui = ImguiManager::Get();

		// handle case where the bar is minimized
		v2i dimen = window->dimen;
		if ( window->minimized ) dimen.y = TITLE_BAR_HEIGHT;

		// hovering over
		if ( ImguiMouseOver(window->pos, dimen) )
		{
			bgAlpha = ALPHA_HOVER;

			// current active
			StrongHandle<ImguiWindow> & prevActiveWindow = ImguiState().activeWindow;

			// make sure the mouse is not active window
			if (  ImguiDidMouseJustGoDown() && 
				( !prevActiveWindow
				|| prevActiveWindow->framesSinceUpdate > 1
				|| (!ImguiMouseOver(prevActiveWindow->pos, prevActiveWindow->dimen))) ) 
			{
				ImguiSetActiveWindow(window);
			}

			// mouse scroll
			else if ( ImguiIsWindowActive() && !window->autosize )
			{
				const float scrollDelta = MOUSEWHEEL_SCROLL_DELTA / max(1,window->dimenAutosize.y);

				if ( Input::Get()->DidMouseWheelHit(Input::MOUSEWHEEL_DOWN) )
				{
					window->scrollPos.y = min( window->scrollPos.y+scrollDelta, 1.0f );
				}

				else if ( Input::Get()->DidMouseWheelHit(Input::MOUSEWHEEL_UP) )
				{
					window->scrollPos.y = max( window->scrollPos.y-scrollDelta, 0.0f );
				}
			}
		}

		if ( ImguiIsWindowActive() )
		{
			bgAlpha = ALPHA_ACTIVE;
		}

		if ( !ImguiIsMinized() )
		{
			ImguiColor(COLOR_WINDOW_BG, bgAlpha );
			ImguiDrawRect( window->pos, window->dimen );
		}
	}

	bool ImguiHorizScrollBar(StrongHandle<ImguiWindow> &window)
	{
		if( ImguiIsMinized() ) return false;
		const int id = ID_SCROLL_X;

		// calculate how much bar is shown
		int totalWidthAllWidgets = window->dimenAutosize.x;
		if ( totalWidthAllWidgets <= 0) totalWidthAllWidgets = 1;
		const int barWidth = window->dimen.x-1;
		const float percentShown = (float)barWidth/totalWidthAllWidgets;

		// don't show bar if not needed
		if ( percentShown >= 1.0f ) 
		{	
			window->scrollPos.x = 0.0f;
			return false;
		}

		if ( !ImguiIsWindowActive() ) return false;

		// bar
		v3  barColor = COLOR_BAR;
		v2i barPos = v2i( window->pos.x+1, window->pos.y-window->dimen.y );
		v2i barDimen = v2i( barWidth, SCROLL_BAR_SIZE );

		// slider
		v3	sliderColor = COLOR_SLIDER;
		v2i sliderDimen = v2i( (int)(barDimen.x*percentShown), SCROLL_BAR_SIZE );
		int sliderPosMinX = barPos.x;
		int sliderPosMaxX = barPos.x + barDimen.x - sliderDimen.x;
		v2i sliderPos = v2i( (int)lerp((float)sliderPosMinX, (float)sliderPosMaxX, window->scrollPos.x), barPos.y );

		const int mouseX = ImguiGetMousePos().x;

		// mouse over
		if ( ImguiMouseOver(barPos, barDimen) )
		{
			barColor = COLOR_BAR_HOVER;	

			if ( ImguiDidMouseJustGoDown() )
			{
				ImguiSetActiveWidgetId(id);

				if ( ImguiMouseOver(sliderPos, sliderDimen) ) // over slider
				{
					ImguiState().data.i = mouseX - sliderPos.x;
				}
				else // not over slider, chose half the position
				{
					ImguiState().data.i = sliderDimen.x >> 1;
				}
			}
		}

		if ( ImguiIsWidgetActive(id) )
		{
			// release mouse
			if ( ImguiDidMouseJustGoUp() )
			{
				ImguiSetActiveWidgetId(ImguiManager::State::WIDGET_NULL);
			}

			// bar position based on mouse pos
			else
			{
				int xMin = barPos.x + ImguiState().data.i;
				int xMax = barPos.x + barDimen.x - sliderDimen.x + ImguiState().data.i;

				window->scrollPos.x = saturate(((float)mouseX - xMin)/(xMax-xMin));		
			}
		}

		if ( ImguiIsWidgetActive(id) ) sliderColor = COLOR_SLIDER_ACTIVE;

		// draw background
		ImguiColor( barColor );
		ImguiDrawRect( barPos, barDimen );
		ImguiColor( COLOR_SLIDER_BG_BORDER );
		ImguiDrawRectWire( barPos, barDimen );

		// draw slider
		ImguiColor( sliderColor );
		ImguiDrawRect( sliderPos, sliderDimen );
		ImguiColor( COLOR_SLIDER_BTN_BORDER );
		ImguiDrawRectWire( sliderPos, sliderDimen );

		return true;
	}

	bool ImguiVertScrollBar(StrongHandle<ImguiWindow> &window)	
	{
		if( ImguiIsMinized() ) return false;
		const int id = ID_SCROLL_Y;

		// calculate how much bar is shown
		int totalHeightAllWidgets = window->dimenAutosize.y-TITLE_BAR_HEIGHT;
		if ( totalHeightAllWidgets <= 0) totalHeightAllWidgets = 1;
		const int barHeight = window->dimen.y-TITLE_BAR_HEIGHT;
		const float percentShown = (float)barHeight/totalHeightAllWidgets;

		// don't show bar if not needed
		if ( percentShown >= 1.0f ) 
		{	
			window->scrollPos.y = 0.0f;
			return false;
		}

		if ( !ImguiIsWindowActive() ) return false;

		// bar
		v3  barColor = COLOR_BAR;
		v2i barPos = v2i( window->pos.x+window->dimen.x, window->pos.y-TITLE_BAR_HEIGHT );
		v2i barDimen = v2i( SCROLL_BAR_SIZE, barHeight );

		// slider
		v3	sliderColor = COLOR_SLIDER;
		v2i sliderDimen = v2i( SCROLL_BAR_SIZE, (int)(barDimen.y*percentShown) );
		int sliderPosMinY = barPos.y;
		int sliderPosMaxY = barPos.y - barDimen.y + sliderDimen.y;
		v2i sliderPos = v2i( barPos.x, (int)lerp((float)sliderPosMinY, (float)sliderPosMaxY, window->scrollPos.y) );

		const int mouseY = ImguiGetMousePos().y;

		// mouse over
		if ( ImguiMouseOver(barPos, barDimen) )
		{
			barColor = COLOR_BAR_HOVER;	

			if ( ImguiDidMouseJustGoDown() )
			{
				ImguiSetActiveWidgetId(id);

				if ( ImguiMouseOver(sliderPos, sliderDimen ) ) // over slider
				{
					ImguiState().data.i = sliderPos.y - mouseY;
				}
				else // not over slider, chose half the position
				{
					ImguiState().data.i = sliderDimen.y >> 1;
				}
			}
		}
		
		if ( ImguiIsWidgetActive(id) )
		{
			// release mouse
			if ( ImguiDidMouseJustGoUp() )
			{
				ImguiSetActiveWidgetId(ImguiManager::State::WIDGET_NULL);
			}
			
			// bar position based on mouse pos
			else
			{
				int yMin = barPos.y - ImguiState().data.i;
				int yMax = barPos.y - barDimen.y + sliderDimen.y - ImguiState().data.i;

				window->scrollPos.y = saturate(((float)mouseY - yMin)/(yMax-yMin));		
			}
		}

		if ( ImguiIsWidgetActive(id) ) sliderColor = COLOR_SLIDER_ACTIVE;

		// draw background
		ImguiColor( barColor );
		ImguiDrawRect( barPos, barDimen );
		ImguiColor( COLOR_SLIDER_BG_BORDER );
		ImguiDrawRectWire( barPos, barDimen );

		// draw slider
		ImguiColor( sliderColor );
		ImguiDrawRect( sliderPos, sliderDimen );
		ImguiColor( COLOR_SLIDER_BTN_BORDER );
		ImguiDrawRectWire( sliderPos, sliderDimen );

		return true;
	}

	void ImguiResizeButton(StrongHandle<ImguiWindow> &window)	
	{
		if( ImguiIsMinized() || !ImguiIsWindowActive() ) return;
		const int id = ID_RESIZE_BUTTON;

		const v2i pos = window->pos + v2i(window->dimen.x, -window->dimen.y);
		const v2i dimen = v2i(SCROLL_BAR_SIZE);
		const v2i mousePos = ImguiGetMousePos();

		v3 color = COLOR_SLIDER;

		// mouse over
		if ( ImguiMouseOver(pos,dimen) )
		{
			color = COLOR_BUTTON_HOVER;

			// clicked on button
			if ( ImguiDidMouseJustGoDown() )
			{
				ImguiSetActiveWidgetId(id);

				// cache mouse pos and offset
				ImguiState().data.ivec[0] = mousePos.x;
				ImguiState().data.ivec[1] = mousePos.y;
				ImguiState().data.ivec[2] = mousePos.x - pos.x;
				ImguiState().data.ivec[3] = mousePos.y - pos.y;
			}
		}

		if ( ImguiIsWidgetActive(id) )
		{
			color = COLOR_BUTTON_PRESS;

			// release
			if ( ImguiDidMouseJustGoUp() )
			{
				ImguiSetActiveWidgetId(ImguiManager::State::WIDGET_NULL);
			}
			else
			{
				// resize window
				window->dimen.x = max( window->titleBarMinWidth+SCROLL_BAR_SIZE, mousePos.x - window->pos.x - ImguiState().data.ivec[2] );
				window->dimen.y = max( TITLE_BAR_HEIGHT+SCROLL_BAR_SIZE, window->pos.y - mousePos.y + ImguiState().data.ivec[3] );
			}
		}
		
		// background
		ImguiColor(color);
		ImguiDrawRect( pos, dimen );
		ImguiColor(COLOR_BLACK);
		ImguiDrawRectWire( pos, dimen );

		// line
		const int LINE_PADDING = 2;
		v2i linePosStart = pos + v2i(LINE_PADDING, -SCROLL_BAR_SIZE+LINE_PADDING);
		v2i linePosEnd = pos + v2i(SCROLL_BAR_SIZE-LINE_PADDING,-LINE_PADDING);
		ImguiColor(COLOR_WHITE, ImguiIsWidgetActive(id) ? 0.85f : 0.5f );
		ImguiDrawLine(linePosStart, linePosEnd);
	}

	void Imgui::Begin( const char* name, v2i pos, v2i dimen )
	{
		assert( name );
		CHECK( ImguiWorkingWindow() == NULL, "Imgui::Begin('%s') Error - Forgot to call Imgui::End() on window '%s'", name, ImguiState().workingWindowTitle.c_str() );

		const int id = ImguiGenWidgetId();
		const v2i mousePos = ImguiGetMousePos();
		const v2i windowSize = Window::Get()->Sizei();

		// setup render
		ImguiManager::Get()->GetCam().Begin();
		Renderer::Get()->BeginDefaultShader();
		glEnable(GL_BLEND);
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);
		glViewport(0,0, windowSize.x, windowSize.y);

		// get window
		ImguiManager * mngr = ImguiManager::Get();
		StrongHandle<ImguiWindow> window = mngr->GetWindow(name);
		window->framesSinceUpdate = 0;
		ImguiState().workingWindow = window;
		ImguiState().workingWindowTitle = name;

		// remove any tabbing
		ImguiState().numTabs = 0;

		// on first creation of window
		if ( window->isNewWindow )
		{
			// dimensions
			window->dimen = dimen;
			window->autosize = ( dimen == v2i(Imgui::AUTOSIZE) );

			// autosize
			if ( window->autosize ) window->dimen = v2i(0);

			window->pos = pos;
		}

		// handle dragging window
		if ( ImguiIsWidgetActive(id) )
		{
			if ( ImguiDidMouseJustGoUp() )
			{
				ImguiSetActiveWidgetId(ImguiManager::State::WIDGET_NULL);
			}
			else
			{
				v2i delta = v2i( ImguiState().data.ivec[0], ImguiState().data.ivec[1] );
				window->pos = mousePos-delta;
			}
		}

		ImguiWindowBG(window);
		ImguiTitleBar(window, name, id );		

		// show scroll bars
		if( !window->autosize )
		{
			bool showHorizScroll = ImguiHorizScrollBar(window);
			bool showVertScroll = ImguiVertScrollBar(window);

			// only show resize button if any scrollbars are visible
			//if ( showHorizScroll || showVertScroll )
			{
				ImguiResizeButton(window);
			}
		}

		// scissor around box (not the title bar or scrollers)
		glEnable(GL_SCISSOR_TEST);
		glScissor(window->pos.x, window->pos.y-window->dimen.y, max(0,window->dimen.x-1), max(0,window->dimen.y-TITLE_BAR_HEIGHT-1) );

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();

		// figure out translation offset
		int translateX = int(-(window->dimenAutosize.x-window->dimen.x) * window->scrollPos.x); 
		int translateY = int((window->dimenAutosize.y-window->dimen.y) * window->scrollPos.y);
		ImguiState().scrollOffset = -v2i(translateX, translateY);
		glTranslatef( (float)translateX, (float)translateY, 0.0f );

		// where the mouse can interact
		ImguiState().mouseRegionStart = window->pos - v2i(0, TITLE_BAR_HEIGHT);
		ImguiState().mouseRegionDimen = window->dimen - v2i(0, TITLE_BAR_HEIGHT);
		
		// determine size through imgui calls
		window->dimenAutosize = v2i(0);
		MoveDrawPosNextLine( v2i(0,WINDOW_INSIDE_PADDING-WIDGET_PADDING) );
	}

	void Imgui::End()
	{
		CHECK( ImguiWorkingWindow() != NULL, "Imgui::End() called without existing working window" );

		// editable region
		const v2i windowSize = Window::Get()->Sizei();
		ImguiState().mouseRegionStart = v2i(0, windowSize.y);
		ImguiState().mouseRegionDimen = windowSize;
		ImguiState().scrollOffset = v2i(0);

		glMatrixMode( GL_MODELVIEW );
		glPopMatrix();
		
		// end render state
		glDisable(GL_SCISSOR_TEST);
		ImguiManager * mngr = ImguiManager::Get();
		Renderer::Get()->EndDefaultShader();
		ImguiManager::Get()->GetCam().End();

		// disable working window
		StrongHandle<ImguiWindow> window = ImguiWorkingWindow();
		window->isNewWindow = false;

		// will use title bar width if that is bigger
		window->dimenAutosize.x = max( window->dimenAutosize.x, window->titleBarMinWidth );

		// add window padding
		window->dimenAutosize += v2i(WINDOW_INSIDE_PADDING, WINDOW_INSIDE_PADDING-WIDGET_PADDING);

		// if autosized
		if ( window->autosize ) window->dimen = window->dimenAutosize;

		ImguiState().workingWindow = NULL;
		ImguiState().widgetCount = 0;
		ImguiState().workingWindowTitle = "";
	}

	bool Imgui::Button( const char* name )
	{
		return ImguiButton(name) == BUTTON_PRESS;
	}

	bool Imgui::ButtonDown( const char* name )
	{
		return ImguiButton(name) == BUTTON_DOWN;
	}

	bool Imgui::ButtonHover( const char* name )
	{
		return ImguiButton(name) == BUTTON_HOVER;
	}

	bool Imgui::ButtonNoPadding( const char* name )
	{
		return ImguiButton(name, 0) == BUTTON_PRESS;
	}

	bool Imgui::ButtonDownNoPadding( const char* name )
	{
		return ImguiButton(name, 0) == BUTTON_DOWN;
	}

	bool Imgui::ButtonHoverNoPadding( const char* name )
	{
		return ImguiButton(name, 0) == BUTTON_HOVER;
	}

	void Imgui::Separator()
	{
		if( ImguiIsMinized() ) return;

		const int PADDING = WIDGET_PADDING;

		StrongHandle<ImguiWindow> window = ImguiWorkingWindow();

		v2i pos = ImguiState().drawPos - v2i(0, PADDING);
		v2i dimen = v2i(window->dimen.x, 0 );

		ImguiColor(COLOR_BLACK, 0.6f);
		ImguiDrawLine( pos, pos + v2i(window->dimen.x-WINDOW_INSIDE_PADDING*2, 0) );

		ImguiColor(COLOR_SEPARATOR);
		ImguiDrawLine( pos-v2i(0,1), pos -v2i(0,1) + v2i(window->dimen.x-WINDOW_INSIDE_PADDING*2, 0) );

		MoveDrawPosNextLine( v2i(1,dimen.y+PADDING*2+1) );
	}

	float ImguiSlider( int id, float percent )
	{
		const int SLIDER_WIDTH = 150;
		const int SLIDER_HEIGHT = Imgui::FONT_HEIGHT;
		const int BUTTON_WIDTH = 10;

		v2i pos = ImguiState().drawPos;
		v2i dimen = v2i( SLIDER_WIDTH, SLIDER_HEIGHT );

		// extend size if not autosized
		if( !ImguiWorkingWindow()->autosize )
		{
			dimen.x = max( dimen.x, ImguiWorkingWindow()->dimen.x - WIDGET_PADDING*2 - WINDOW_INSIDE_PADDING*2 - Imgui::FONT_WIDTH*15 );
		}

		v3 colorBar = COLOR_BAR;
		v3 colorButton = COLOR_SLIDER;

		// button position
		int xMin = pos.x+BUTTON_WIDTH/2;
		int xMax = pos.x+dimen.x-BUTTON_WIDTH/2;

		if ( ImguiMouseOver(pos,dimen) )
		{
			colorBar = COLOR_BAR_HOVER;

			// clicked down
			if ( ImguiDidMouseJustGoDown() && ImguiIsWindowActive() )
			{
				ImguiSetActiveWidgetId(id);
				ImguiState().data.f = 0.0f;
			}
		}

		if ( ImguiIsWidgetActive(id) )
		{
			// released
			if ( ImguiDidMouseJustGoUp() )
			{
				ImguiSetActiveWidgetId(ImguiManager::State::WIDGET_NULL);
			}

			colorButton = COLOR_SLIDER_ACTIVE;
		}

		// background
		ImguiColor( colorBar );
		ImguiDrawRect( pos, dimen );
		ImguiColor( COLOR_SLIDER_BG_BORDER );
		ImguiDrawRectWire( pos, dimen );

		// draw button
		int xPos = (int)lerp((float)xMin, (float)xMax, saturate(percent) );
		v2i buttonDimen = v2i(BUTTON_WIDTH, SLIDER_HEIGHT-1 );
		v2i buttonPos = v2i( xPos-BUTTON_WIDTH/2, pos.y );
		ImguiColor( colorButton );
		ImguiDrawRect( buttonPos, buttonDimen );
		ImguiColor( COLOR_SLIDER_BTN_BORDER );
		ImguiDrawRectWire( buttonPos, buttonDimen );

		// move draw cursor
		MoveDrawPosNextLine( dimen );

		// do this after draw call because some buttons need to snap
		if ( ImguiIsWidgetActive(id) )
		{
			int mouseX = ImguiGetMousePos().x;
			percent = saturate( float(mouseX - xMin)/(xMax-xMin) );
		}

		return percent;
	}

	float Imgui::SliderFloat( const char* name, float & val, float min, float max )
	{
		if( ImguiIsMinized() ) return val;
		const int id = ImguiGenWidgetId();

		Print(name);

		// calc value
		float sliderVal = min + ImguiSlider(id, (val-min)/(max-min) )*(max-min);
		if ( ImguiIsWidgetActive(id) ) val = sliderVal;

		// print text
		SameLine();
		char buffer[32];
		sprintf_s(buffer, "%0.4f", val);
		Print(buffer);
		
		return val;
	}

	int Imgui::SliderInt( const char* name, int & val, int min, int max )
	{
		if( ImguiIsMinized() ) return val;
		const int id = ImguiGenWidgetId();

		Print(name);

		// calc value
		int sliderVal = min + (int)(ImguiSlider(id, float(val-min)/(max-min) )*max);
        sliderVal = std::min<int>(sliderVal, max - 1);
		if ( ImguiIsWidgetActive(id) ) val = sliderVal;

		// print text
		SameLine();
		char buffer[32];
		sprintf_s(buffer, "%d", val);
		Print(buffer);

		return val;
	}

	funk::v2 Imgui::SliderV2( const char* name, v2 & val, v2 min, v2 max )
	{
		if( ImguiIsMinized() ) return val;
		const int id = ImguiGenWidgetId();

		Print(name);

		SliderFloat( NULL, val.x, min.x, max.x );
		SliderFloat( NULL, val.y, min.y, max.y );

		return val;
	}

	funk::v3 Imgui::SliderV3( const char* name, v3 & val, v3 min, v3 max )
	{
		if( ImguiIsMinized() ) return val;
		const int id = ImguiGenWidgetId();

		Print(name);

		SliderFloat( NULL, val.x, min.x, max.x );
		SliderFloat( NULL, val.y, min.y, max.y );
		SliderFloat( NULL, val.z, min.z, max.z );

		return val;
	}

	funk::v3 Imgui::SliderRGB( const char* name, v3 & val )
	{
		if( ImguiIsMinized() ) return val;
		const int id = ImguiGenWidgetId();

		Print(name);

		// draw color block
		SameLine();
		v2i pos = ImguiState().drawPos;
		v2i dimen = v2i(Imgui::FONT_HEIGHT*2, Imgui::FONT_HEIGHT);
		ImguiColor( val );
		ImguiDrawRect( pos, dimen );
		ImguiColor( COLOR_BLACK );
		ImguiDrawRectWire( pos, dimen );
		MoveDrawPosNextLine(dimen);

		const v3 min(0.0f);
		const v3 max(1.0f);

		SliderFloat( NULL, val.x, min.x, max.x );
		SliderFloat( NULL, val.y, min.y, max.y );
		SliderFloat( NULL, val.z, min.z, max.z );

		return val;
	}


	funk::v3 Imgui::SliderHSV( const char* name, v3 & rgb )
	{
		if( ImguiIsMinized() ) return rgb;
		const int id = ImguiGenWidgetId();

		Print(name);

		// draw color block
		SameLine();
		v2i pos = ImguiState().drawPos;
		v2i dimen = v2i(Imgui::FONT_HEIGHT*2, Imgui::FONT_HEIGHT);
		ImguiColor( rgb );
		ImguiDrawRect( pos, dimen );
		ImguiColor( COLOR_BLACK );
		ImguiDrawRectWire( pos, dimen );
		MoveDrawPosNextLine(dimen);

		const v3 min(0.0f);
		const v3 max(1.0f);

		v3 hsv = RGBtoHSV(rgb);
		SliderFloat( NULL, hsv.x, 0.0f, 2.0f*PI-0.01f );
		SliderFloat( NULL, hsv.y, min.y, max.y );
		SliderFloat( NULL, hsv.z, min.z, max.z );

		rgb = HSVtoRGB(hsv);
		return rgb;
	}

	void Imgui::Print( const char * text )
	{
		if( ImguiIsMinized() || !text ) return;

		int textWidth = ImguiTextWidth(text);
		ImguiColor(COLOR_WHITE);
		ImguiPrint(text, ImguiState().drawPos );
		MoveDrawPosNextLine( v2i(textWidth, Imgui::FONT_HEIGHT) );
	}

	void Imgui::PrintParagraph( const char * text )
	{
		if( ImguiIsMinized() || !text ) return;

		int srcLen = strlen(text);
		int offset = 0;
		int lineNumber = 1;
		const char * textPos = text;

		while( textPos-text < srcLen )
		{
			const char * endPos = strchr( textPos, '\n' );

			if ( endPos == NULL ) endPos = text+srcLen;

			// copy line
			char buffer[256];
			int len = min( (int)(endPos-textPos), (int)sizeof(buffer)-1);
			memcpy(buffer, textPos, len);
			buffer[len] = 0;
			textPos = endPos+1;

			Imgui::Print(buffer);
		}
	}

	void Imgui::SameLine()
	{
		if( ImguiIsMinized() ) return;

		ImguiState().drawPos = ImguiState().drawPosPrev;
	}

	bool Imgui::CheckBox( const char* name, bool &val )
	{
		if( ImguiIsMinized() ) return val;

		if ( ImguiButton(val ? " X " : "   ", 0) == BUTTON_PRESS )
		{
			val = !val;
		}

		SameLine();
		Print(name);

		return val;
	}

	int Imgui::Select( const char* names[], int &val, int numChoices )
	{
		if( ImguiIsMinized() ) return val;

		for( int i = 0; i < numChoices; ++i )
		{
			if ( ImguiButton( i==val ? " * " : "   ", 0) == BUTTON_PRESS )
			{
				val = i;
			}

			SameLine();
			Print(names[i]);
		}

		return val;
	}

	int	Imgui::SelectCustom( const char* names[], int* values, int &val, int numChoices )
	{
		if( ImguiIsMinized() ) return val;

		for( int i = 0; i < numChoices; ++i )
		{
			if ( ImguiButton( i==val ? " * " : "   ", 0) == BUTTON_PRESS )
			{
				val = values[i];
			}

			SameLine();
			Print(names[i]);
		}

		return val;
	}

	int Imgui::DropDown( const char* name, bool &val )
	{
		if( ImguiIsMinized() ) return val;

		if ( ImguiButton(val ? " V " : " > ", 0) == BUTTON_PRESS )
		{
			val = !val;
		}

		SameLine();
		Print(name);

		return val;
	}

	void ImguiFillBar(float percent)
	{		
		v2i pos = ImguiState().drawPos;
		v2i dimen = v2i( FILLBAR_WIDTH, FILLBAR_HEIGHT );

		ImguiColor( COLOR_BAR );
		ImguiDrawRect( pos, dimen );
		ImguiColor( COLOR_BLACK, 0.5f );
		ImguiDrawRectWire( pos, dimen );
		ImguiColor( COLOR_FILLBAR );
		ImguiDrawRect( pos, v2i((int)(dimen.x*saturate(percent)), dimen.y) );
	}

	void Imgui::FillBarFloat( const char* name, float val, float min, float max )
	{
		if( ImguiIsMinized() ) return;

		// title
		Imgui::Print( name );

		// bar
		float percent = (float)(val-min)/(max-min);
		ImguiFillBar(percent);

		v2i pos = ImguiState().drawPos;
		v2i dimen = v2i( FILLBAR_WIDTH, FILLBAR_HEIGHT );

		// text on top
		char buffer[32];
		sprintf_s(buffer, "%f", val);
		ImguiColor(COLOR_WHITE);
		ImguiPrint(buffer, pos+v2i(FILLBAR_TEXT_BUFFER,-FILLBAR_TEXT_BUFFER));

		MoveDrawPosNextLine( dimen );		
	}

	void Imgui::FillBarV2( const char* name, v2 val, v2 min, v2 max )
	{
		if( ImguiIsMinized() ) return;

		Imgui::Print( name );
		Imgui::FillBarFloat(NULL, val.x, min.x, max.x);
		Imgui::FillBarFloat(NULL, val.y, min.y, max.y);
	}

	void Imgui::FillBarV3( const char* name, v3 val, v3 min, v3 max )
	{
		if( ImguiIsMinized() ) return;

		Imgui::Print( name );
		Imgui::FillBarFloat(NULL, val.x, min.x, max.x);
		Imgui::FillBarFloat(NULL, val.y, min.y, max.y);
		Imgui::FillBarFloat(NULL, val.z, min.z, max.z);
	}

	void Imgui::FillBarInt( const char* name, int val, int min, int max )
	{
		if( ImguiIsMinized() ) return;

		// title
		Imgui::Print( name );

		// bar
		float percent = (float)(val-min)/(max-min);
		ImguiFillBar(percent);
		v2i pos = ImguiState().drawPos;
		v2i dimen = v2i( FILLBAR_WIDTH, FILLBAR_HEIGHT );

		// text on top
		char buffer[32];
		sprintf_s(buffer, "%d", val);
		ImguiColor(COLOR_WHITE);
		ImguiPrint(buffer, pos+v2i(FILLBAR_TEXT_BUFFER,-FILLBAR_TEXT_BUFFER));

		MoveDrawPosNextLine( dimen );	
	}

	int Imgui::TextInput( const char* name, std::string& val, int width )
	{
		const float KEYPRESS_COOLDOWN = 0.4f;
		const float KEYHOLD_COOLDOWN = 0.04f;

		// time
		static Timer timer;
		static float prevFrameTime = 0.0f;
		float currTime = timer.GetTimeSecs();
		float dt = currTime - prevFrameTime;
		prevFrameTime = currTime;

		if( ImguiIsMinized() ) return 0;
		const int id = ImguiGenWidgetId();

		// title
		Imgui::Print( name );

		v2i pos = ImguiState().drawPos;
		v2i dimen = v2i( width, FILLBAR_HEIGHT );
		v3 colorBar = COLOR_BAR;
		v3 colorBorder = COLOR_BLACK;
		int numCharSlots = (dimen.x - FILLBAR_TEXT_BUFFER*2) / Imgui::FONT_WIDTH;

		// get carret pos
		int & caretStart = ImguiState().data.ivec[0];
		int & caretPos = ImguiState().data.ivec[1];

		if ( ImguiIsWidgetActive(id) )
		{
			caretStart =  clamp( caretStart, 0, max(0, (int)val.size()-1) );
			if( (int)val.size() <= numCharSlots ) caretStart = 0;
			caretPos =  clamp( caretPos, caretStart, caretStart + min(numCharSlots-1, (int)val.size()) );
		}
		
		float & heldKeyCooldown = ImguiState().data.fvec[2];
		
		if ( ImguiMouseOver(pos,dimen) )
		{
			colorBar = COLOR_BAR_HOVER;

			// clicked down
			if ( ImguiDidMouseJustGoDown() && !ImguiIsWidgetActive(id) && ImguiIsWindowActive() )
			{
				ImguiSetActiveWidgetId(id);
				caretStart = max( (int)val.size() - numCharSlots + 1, 0 ); // start
				caretPos = caretStart + min(numCharSlots-1, (int)val.size()); // position
				heldKeyCooldown = 0.0f;
			}
		}

		// highlight if active
		if ( ImguiIsWidgetActive(id) )
		{
			colorBorder = COLOR_WHITE;

			int caretDelta = 0;

			// did press a key
			char c = Input::Get()->GetKeyDown();
			if ( c != 0 && c != Input::KEY_BACKSPACE )
			{
				if  ( c == Input::KEY_RETURN )
				{
					//ImguiSetActiveWidgetId(ImguiManager::State::WIDGET_NULL);
					return 1;
				}
				else // append character
				{
					val.insert( caretPos, 1, c );
					++caretDelta;
				}
			}

			// handle special cases
			else
			{
				// keys that can be held down
				const char * heldKey = NULL;
				const char* holdableKeys[] = {"LEFT", "RIGHT", "DELETE", "BACKSPACE"};
				for( int i = 0; i < sizeof(holdableKeys)/sizeof(holdableKeys[0]); ++i )
				{
					// just pressed down
					if( Input::Get()->DidKeyJustGoDown(holdableKeys[i]) ) // firstime press
					{
						heldKeyCooldown = KEYPRESS_COOLDOWN;
						heldKey = holdableKeys[i];
						break;
					}
					else if ( Input::Get()->IsKeyDown(holdableKeys[i]) ) // held down
					{
						heldKeyCooldown -= dt;

						if ( heldKeyCooldown <= 0.0f )
						{
							heldKeyCooldown = KEYHOLD_COOLDOWN;
							heldKey = holdableKeys[i];
							break;
						}
					}
				}

				if ( heldKey )
				{
					if( strcmp(heldKey, "LEFT") == 0 )
					{
						--caretDelta;
					}
					else if( strcmp(heldKey, "RIGHT") == 0 )
					{	
						++caretDelta;
					}
					else if( strcmp(heldKey, "DELETE") == 0 )
					{
						if ( caretPos < (int)val.size() ) val.erase(caretPos, 1);
					}
					else if( strcmp(heldKey, "BACKSPACE") == 0 )
					{
						if ( !val.empty() && caretPos-1 >= 0 ) 
						{
							val.erase( caretPos-1, 1 );
							--caretDelta;
						}
					}
				}
				else if ( Input::Get()->DidKeyJustGoDown("HOME"))
				{
					caretPos = caretStart = 0;
				}
				else if ( Input::Get()->DidKeyJustGoDown("END"))
				{
					caretStart = max( (int)val.size() - numCharSlots + 1, 0 ); // start
					caretPos = caretStart + min(numCharSlots-1, (int)val.size()); // position
				}
			}

			// move caret left
			if ( caretDelta == -1 )
			{
				caretPos = max(caretPos-1, 0);
				if ( caretPos < caretStart ) caretStart -= 1;

				// handle case where deleting
				if ( caretStart > 0 && caretPos >= (int)val.size() && (caretStart-caretPos)<numCharSlots )
				{
					--caretStart;
				}
			}

			// move caret right
			else if ( caretDelta == 1 )
			{
				if ( (int)val.size() > numCharSlots )
				{
					if( caretPos < (int)val.size() ) ++caretPos;
				}
				else if ( caretPos < (int)val.size() )	++caretPos;

				if ( caretPos-caretStart >= numCharSlots )
				{
					++caretStart;
				}
			}
		}

		// draw rect
		ImguiColor( colorBar );
		ImguiDrawRect(pos, dimen);
		ImguiColor( colorBorder, 0.5f );
		ImguiDrawRectWire(pos, dimen);
		
		// tex position
		v2i textPos = pos + v2i(FILLBAR_TEXT_BUFFER, -FILLBAR_TEXT_BUFFER);
		int characterStart = 0;
		int characterEnd = min( (int)val.size(), numCharSlots );

		// substr with caret positions
		if ( ImguiIsWidgetActive(id) )
		{
			characterStart = caretStart;
			characterEnd = caretStart + numCharSlots;
		}

		// draw text
		int numCharsPrint = min(characterEnd-characterStart, numCharSlots );
		ImguiColor( COLOR_WHITE );
		ImguiPrint( val.substr(characterStart,numCharsPrint).c_str(), textPos );

		// do "text_" blinking
		if ( ImguiIsWidgetActive(id) && fmod( timer.GetTimeSecs(), 1.0f ) < 0.5f )
		{
			v2i underlinePos = textPos + Imgui::FONT_WIDTH*v2i(caretPos-caretStart, 0);
			ImguiPrint( "_", underlinePos-v2i(0,1) );
		}

		MoveDrawPosNextLine( dimen );

		return 0;
	}

	void Imgui::Tab()
	{
		if( ImguiIsMinized() ) return;

		ImguiState().drawPos.x += TAB_WIDTH;
		++ImguiState().numTabs;
	}

	void Imgui::Untab()
	{
		if( ImguiIsMinized() ) return;

		if ( ImguiState().numTabs > 0 ) 
		{
			ImguiState().drawPos.x -= TAB_WIDTH;
			--ImguiState().numTabs;
		}
	}

	void Imgui::Minimize()
	{
		StrongHandle<ImguiWindow> window = ImguiWorkingWindow();
		if ( window->isNewWindow ) { window->minimized = true; }
	}

	void Imgui::Header( const char* name )
	{
		if( ImguiIsMinized() ) return;

		const int HEADER_PADDING = 3;
		const int HEADER_HEIGHT = Imgui::FONT_HEIGHT + HEADER_PADDING*3;
		const int HEADER_WIDTH = FILLBAR_WIDTH;
		const int HEADER_TOP_PADDING = 7;

		ImguiState().drawPos.x = ImguiWorkingWindow()->pos.x;
		ImguiState().drawPos.y -= HEADER_TOP_PADDING;
		v2i pos = ImguiState().drawPos;
		v2i dimen = v2i( ImguiWorkingWindow()->dimen.x, HEADER_HEIGHT );

		const v3 HEADER_COLOR = v3(0.1f, 0.25f, 0.29f );
		ImguiColor( COLOR_BAR_HOVER, ImguiIsWindowActive() ? 1.0f : 0.7f );
		ImguiDrawRect( pos, dimen );
		ImguiColor( COLOR_WHITE );
		ImguiPrint(name, pos+v2i(HEADER_PADDING*2, -HEADER_PADDING));

		// border around
		ImguiColor( COLOR_WHITE,0.2f );
		ImguiDrawRectWire( pos, dimen-v2i(0,1) );

		MoveDrawPosNextLine(dimen+v2i(-WINDOW_INSIDE_PADDING-WIDGET_PADDING,HEADER_PADDING));
	}

	void Imgui::Lock( const char * gui )
	{
		StrongHandle<ImguiWindow> window = ImguiManager::Get()->GetWindow(gui);
		assert(window);
		window->locked = true;

		if ( window == ImguiState().activeWindow )
		{
			ImguiManager::Get()->ClearActiveWindow();
		}
	}

	void Imgui::Unlock( const char * gui )
	{
		StrongHandle<ImguiWindow> window = ImguiManager::Get()->GetWindow(gui);
		assert(window);
		window->locked = false;
	}

	void Imgui::SetScrollX( float x )
	{
		if( ImguiIsMinized() ) return;
		ImguiWorkingWindow()->scrollPos.x = saturate(x);
	}

	void Imgui::SetScrollY( float y )
	{
		if( ImguiIsMinized() ) return;
		ImguiWorkingWindow()->scrollPos.y = saturate(y);
	}

	bool Imgui::IsWindowActive()
	{
		return ImguiWorkingWindow() && (ImguiWorkingWindow() == ImguiState().activeWindow );
	}

	int	Imgui::GetPrevWidgetId()
	{
		assert( ImguiState().workingWindow != NULL );
		return ImguiState().widgetCount;
	}

	bool Imgui::IsWidgetActive(int id)
	{
		assert( id > 0 );
		return IsWindowActive() && (ImguiState().activeWidgetId == id);
	}

	bool Imgui::IsWorkingWindowNew()
	{
		assert( ImguiState().workingWindow != NULL );
		return ImguiState().workingWindow->isNewWindow;
	}

	void Imgui::SetActiveWidgetId(int id)
	{
		assert( ImguiState().workingWindow != NULL );
		assert( id > 0 );

		ImguiState().activeWidgetId = id;
		ImguiState().activeWindow = ImguiState().workingWindow;
	}

	void Imgui::LineGraph( StrongHandle<funk::LineGraph> lineGraph )
	{
		assert(lineGraph);

		if( ImguiIsMinized() ) return;

		const v2i pos = ImguiState().drawPos;
		const v2i dimen = lineGraph->Dimen();

		lineGraph->DrawBG( toV2(pos) );
		lineGraph->Draw( toV2(pos) );
		MoveDrawPosNextLine( dimen );
	}

	void Imgui::CubicSpline2d( StrongHandle<funk::CubicSpline2d> spline, v2 minPos, v2 maxPos, v2i dimen )
	{
		assert(spline);

		if( ImguiIsMinized() ) return;
		const int id = ImguiGenWidgetId();

		const v2i pos = ImguiState().drawPos;
		const v2 posf = toV2(pos);
		const v2 dimenf = toV2(dimen);
		const bool isMouseOverWidget = ImguiMouseOver(pos, dimen);

		// drop box
		ImguiColor( v3(0.2f, 0.2f, 0.2f), 0.8f );
		ImguiDrawRect( pos, dimen );
		ImguiColor( v3(1.0f,1.0f,1.0f), isMouseOverWidget ? 1.0f : 0.6f );
		ImguiDrawRectWire( pos, dimen );

		// calc relative mouse pos, in spline space
		const v2 mousePos = toV2(ImguiGetMousePos());
		v2 mouseAlpha = (mousePos-posf+v2(0.0f,dimenf.y))/(dimenf);
		mouseAlpha = saturate(mouseAlpha);
		float newX = lerp(minPos.x, maxPos.x, mouseAlpha.x);
		float newY = lerp(minPos.y, maxPos.y, mouseAlpha.y);
		const v2 mousePosSpline = v2(newX, newY);

		// draw grid
		int numLines = 9;
		ImguiColor( COLOR_WHITE, 0.1f );
		const v2i boxMinPos = pos + v2i(0, -dimen.y);
		const v2i boxMaxPos = boxMinPos + dimen;
		for ( int i = 0; i <= numLines+1; ++i )
		{
			float alpha = (float)i/(numLines+1);

			int x = lerp( boxMinPos.x, boxMaxPos.x, alpha );
			int y = lerp( boxMinPos.y, boxMaxPos.y, alpha );

			ImguiDrawLine( v2i(x, boxMinPos.y), v2i(x, boxMaxPos.y));
			ImguiDrawLine( v2i(boxMinPos.x, y), v2i(boxMaxPos.x, y));
		}
		
		// draw curve
		ImguiColor( COLOR_WHITE, 0.3f );
		const int numCurvePts = 128;
		const float deltaT = 1.0f / numCurvePts;
		for( int i = 0; i < numCurvePts; ++i )
		{
			float t0 = deltaT*i;
			float t1 = deltaT*(i+1);
			const v2 pt0 = spline->CalcPosAt(t0);
			const v2 pt1 = spline->CalcPosAt(t1);

			// position in the widget
			const v2 alpha0 = (pt0 - minPos)/(maxPos-minPos);
			const v2 alpha1 = (pt1 - minPos)/(maxPos-minPos);
			const v2 relPos0 = posf + dimenf*alpha0 - v2(0, dimenf.y);
			const v2 relPos1 = posf + dimenf*alpha1 - v2(0, dimenf.y);

			ImguiDrawLine( toV2i(relPos0), toV2i(relPos1) );
		}

		// draw control points
		const int numControlPts = spline->NumControlPts();
		const int ptSize = 14;	
		int & selectedPtIndex = ImguiState().data.i;
		int mouseOverPtIndex = -1;
		for( int i = 0; i < numControlPts; ++i )
		{
			const v2 pt = spline->GetControlPt(i);
			const v2 alpha = (pt - minPos)/(maxPos-minPos);
			const v2 relPos = posf + dimenf*alpha - v2(0, dimenf.y);

			const v2i ptPos = toV2i(relPos)+v2i(-ptSize>>1, +ptSize>>1);
			const v2i ptDimen = v2i(ptSize);
			
			bool isMouseOverPt = ImguiMouseOver(ptPos,ptDimen);
			mouseOverPtIndex = ImguiIsWidgetActive(id) ? selectedPtIndex : mouseOverPtIndex != -1 ?mouseOverPtIndex: isMouseOverPt?i:-1;

			// draw box border around control pt
			if ( isMouseOverWidget )
			{
				// select color
				if ( ImguiIsWidgetActive(id) && selectedPtIndex == i )
				{
					ImguiColor(v3(1.0f,1.0f,1.0f) );
				}
				else if ( isMouseOverPt )
				{
					ImguiColor(v3(1.0f,0.7f,0.0f) );
				}
				else
				{
					ImguiColor(v3(1.0f,0.0f,0.0f) );
				}

				ImguiDrawRectWire(ptPos, ptDimen);
				
			}

			ImguiColor(v3(1.0f,1.0f,1.0f) );
			ImguiDrawPoint(toV2i(relPos), 2.0f);

			// handle mouse active
			if ( ImguiIsWidgetActive(id) )
			{
				// released
				if ( ImguiDidMouseJustGoUp() )
				{
					ImguiSetActiveWidgetId( ImguiManager::State::WIDGET_NULL );
				}
				else if ( i == selectedPtIndex )
				{
					spline->SetControlPt( i, mousePosSpline );
				}
			}
			else if ( isMouseOverPt && ImguiDidMouseJustGoDown() ) // click down
			{
				ImguiSetActiveWidgetId(id);
				ImguiState().data.i = i;
			}
		}
		MoveDrawPosNextLine( dimen );

		char buffer[64];
		ImguiColor(COLOR_WHITE);

		if ( mouseOverPtIndex != -1 )
		{
			const v2 ptPos = spline->GetControlPt(mouseOverPtIndex);

			sprintf_s( buffer, "(%.3f, %.3f)", ptPos.x, ptPos.y );
			ImguiPrint(buffer, ImguiGetMousePos()+v2i(-Imgui::FONT_WIDTH*5, Imgui::FONT_HEIGHT*2) );
		}
		else if ( isMouseOverWidget )
		{
			sprintf_s( buffer, "(%.3f, %.3f)", mousePosSpline.x, mousePosSpline.y );
			ImguiPrint(buffer, ImguiGetMousePos()+v2i(-Imgui::FONT_WIDTH*5, Imgui::FONT_HEIGHT*2) );
			ImguiDrawPoint(ImguiGetMousePos(), 4.0f);
		}
	}
}