#include "Window.h"

#include <gl/glew.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <common/ResourcePath.h>
#include <common/IniReader.h>

namespace funk
{
	Window::Window()
	{
		IniReader reader( RESOURCE_PATH("common/ini/main.ini") );
		m_dimen.x = reader.GetInt( "Window", "WindowWidth" );
		m_dimen.y = reader.GetInt( "Window", "WindowHeight" );
		int fps = reader.GetInt( "Window", "FPS" );
		m_bFullScreen = reader.GetInt( "Window", "Fullscreen" ) != 0;

		#ifdef _WIN32
		int showConsole = reader.GetInt( "Window", "ShowConsole" );
		if ( !showConsole ) ShowWindow( GetConsoleWindow(), SW_HIDE );
		#endif

		int result = SDL_Init( SDL_INIT_VIDEO );
		assert( result == 0 );
		SDL_WM_SetCaption( reader.GetCStr("Window","Title"), 0 );
		InitIcon();

		// monitor info
		char buffer[64];
		const SDL_VideoInfo* info = SDL_GetVideoInfo();
		assert(info);
		m_dimenMonitor.x = info->current_w;
		m_dimenMonitor.y = info->current_h;
		SDL_VideoDriverName( buffer, sizeof(buffer));

		// set fullscreen to window dimen
		if ( m_bFullScreen ) m_dimen = m_dimenMonitor; 

		printf( "%dx%d @ %d hz. Driver: %s\n", m_dimen.x, m_dimen.y, fps, buffer );

		// Window surface format
		Uint32 flags = SDL_OPENGL | SDL_DOUBLEBUF | SDL_HWSURFACE ;
		if ( m_bFullScreen ) flags |= SDL_FULLSCREEN;
		result = SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
		assert( result == 0 );

		m_screen = SDL_SetVideoMode( m_dimen.x, m_dimen.y, 0, flags );
		glViewport( 0, 0, m_dimen.x, m_dimen.y );
	}

	Window::~Window()
	{
		SDL_Quit();
	}

	bool Window::IsMouseFocus() const
	{
		return (SDL_GetAppState() & SDL_APPMOUSEFOCUS) != 0;
	}

	bool Window::IsKeyboardFocus() const
	{
		return (SDL_GetAppState() & SDL_APPINPUTFOCUS) != 0;
	}

	bool Window::IsAppActive() const
	{
		return (SDL_GetAppState() & SDL_APPACTIVE) != 0;
	}

	void Window::SetTitle( const char * title )
	{
		assert(title);
		SDL_WM_SetCaption( title, 0 );
	}

	void Window::InitIcon()
	{
		const int iconWidth = 32;

		SDL_Surface* image = SDL_LoadBMP(RESOURCE_PATH("common/img/icon.bmp"));
		assert(image);
		
		// mask
		unsigned char mask[iconWidth*iconWidth];
		memset(mask, 0x0, sizeof(mask) );

		// magenta color key
		const uint32_t colorKeyUnit = 0xff00ff00; 
		
		SDL_LockSurface(image);

		// create mask
		for( int i = 0; i < iconWidth*iconWidth; ++i )
		{
			unsigned char* pixelColor = (unsigned char*)(image->pixels) + image->format->BytesPerPixel*i;
			const uint32_t colorUint = (pixelColor[2] << 24) | (pixelColor[1] << 16) | (pixelColor[0] << 8);

			if ( colorUint != colorKeyUnit ) 
			{
				int maskIndex = i>>3;
				int bit = 7-i%8;
				mask[maskIndex] |= 1<<(bit);
			}
		}

		SDL_UnlockSurface(image);
		SDL_WM_SetIcon( image, mask );
	}
}