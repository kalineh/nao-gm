#ifndef _INCLUDE_WINDOW_H_
#define _INCLUDE_WINDOW_H_

#include <SDL.h>

#include <common/Singleton.h>
#include <math/v2.h>
#include <math/v2i.h>

namespace funk
{
	class Window : public Singleton<Window>
	{
	public:
		v2i Sizei() const { return m_dimen; }
		v2 Sizef() const { return funk::v2((float)m_dimen.x, (float)m_dimen.y); }

		v2i MonitorSizei() const { return m_dimenMonitor; }
		
		bool IsMouseFocus() const;
		bool IsKeyboardFocus() const;
		bool IsAppActive() const;
		bool IsFullScreen() const { return m_bFullScreen; }

		void SetTitle( const char * title );

	private:
		friend Singleton<Window>;
		Window();
		~Window();

		void InitIcon();

		v2i m_dimen;
		v2i m_dimenMonitor;
		bool m_bFullScreen;

		SDL_Surface* m_screen;
	};
}

#endif