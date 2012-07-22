#ifndef _INCLUDE_INPUTMANAGER_H_
#define _INCLUDE_INPUTMANAGER_H_

#include <string>
#include <map>
#include <math/v2i.h>

#include <common/Singleton.h>

#include "SDL.h"

namespace funk
{
	class Input : public Singleton<Input>
	{
	public:
		static const char KEY_BACKSPACE = 8;
		static const char KEY_RETURN = 13;
		static const int  MOUSEWHEEL_UP = 0;
		static const int  MOUSEWHEEL_DOWN = 1;

		void Update();
		void HandleEvent(SDL_Event &event);

		// Mouse
		inline bool IsMouseDown( int btn ) const;
		inline bool DidMouseJustGoDown( int btn ) const;
		inline bool DidMouseJustGoUp( int btn ) const;
		void		SetMousePos( int x, int y );
		v2i			GetMousePos() const { return m_mousePos; }
		v2i			GetMousePosRel() const { return m_mouseRel; }
		inline bool DidMouseWheelHit( int btn ) const;
		inline bool DidMouseDoubleClick() const { return m_doubleClicked; }

		// Key
		inline bool IsKeyDown( const char* key ) const;
		inline bool DidKeyJustGoDown( const char* key ) const;
		inline bool DidKeyJustGoUp( const char* key ) const;
		inline char GetKeyDown() const { return m_keyDown; }
		inline char GetKeyUp() const { return m_keyUp; }

	private:
		friend Singleton<Input>;
		Input();
		~Input();

		void BuildMapStringKey();
		inline bool IsKeyDownInState( int iState, unsigned int key ) const;

		void DetectDoubleClick();
		bool m_doubleClicked;

		// Key
		static const int MAX_KEYS = 323;
		Uint8  m_arrayKeyStates[ 2 ][MAX_KEYS];
		std::map< std::string, unsigned int > m_mapStringKey;
		char m_keyUp, m_keyDown;

		int m_iCurrentState;

		// mouse
		bool  m_mouseWheelHit[2];
		Uint8 m_mouseState[2];
		v2i m_mousePos;
		v2i m_mouseRel;

		inline bool IsMouseDownInState( int iState, int mouseKey ) const;

	};

#include "Input.inl"
}

#endif