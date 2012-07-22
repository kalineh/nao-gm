#include "Input.h"

#include "SDL_keyboard.h"

#include <common/Window.h>
#include <common/Timer.h>

namespace funk
{
	Input::Input() : m_iCurrentState(0), m_mousePos(0,0), m_mouseRel(0,0), m_doubleClicked(false)
	{
		BuildMapStringKey();

		Update();
		Update();
	}

	Input::~Input()
	{;}

	void Input::Update()
	{
		// Key state
		Uint8* keyState = SDL_GetKeyState( NULL );
		memcpy( m_arrayKeyStates[m_iCurrentState], keyState, MAX_KEYS);
		m_keyDown = m_keyUp = 0;

		// Mouse state
		int x, y;
		m_mouseState[m_iCurrentState] = SDL_GetMouseState( &x, &y );
		memset( m_mouseWheelHit, 0, sizeof(m_mouseWheelHit) );
		v2i mousePos;
		mousePos.x = x;
		mousePos.y = Window::Get()->Sizei().y - y;

		m_mouseRel = mousePos - m_mousePos;
		m_mousePos = mousePos;		

		m_iCurrentState ^= 1;

		DetectDoubleClick();
	}

	void Input::DetectDoubleClick()
	{
		const float DOUBLE_CLICK_THRESHOLD_SECS = 0.35f;
		const float DOUBLE_CLICK_DIST_THRESHOLD = 5.0f;
		static Timer clickTimer;

		m_doubleClicked = false;

		if ( DidMouseJustGoDown(1) && length(m_mouseRel) < DOUBLE_CLICK_DIST_THRESHOLD  )
		{
			float time = clickTimer.GetTimeSecs();
			m_doubleClicked = ( time < DOUBLE_CLICK_THRESHOLD_SECS );
			clickTimer.Start();
		}
	}

	void Input::BuildMapStringKey()
	{
		m_mapStringKey["FIRST"]=0;
		m_mapStringKey["BACKSPACE"]=8;
		m_mapStringKey["TAB"]=9;
		m_mapStringKey["CLEAR"]=12;
		m_mapStringKey["RETURN"]=13;
		m_mapStringKey["PAUSE"]=19;
		m_mapStringKey["ESCAPE"]=27;
		m_mapStringKey["SPACE"]=32;
		m_mapStringKey["EXCLAIM"]=33;
		m_mapStringKey["QUOTEDBL"]=34;
		m_mapStringKey["HASH"]=35;
		m_mapStringKey["DOLLAR"]=36;
		m_mapStringKey["AMPERSAND"]=38;
		m_mapStringKey["QUOTE"]=39;
		m_mapStringKey["LEFTPAREN"]=40;
		m_mapStringKey["RIGHTPAREN"]=41;
		m_mapStringKey["ASTERISK"]=42;
		m_mapStringKey["PLUS"]=43;
		m_mapStringKey["COMMA"]=44;
		m_mapStringKey["MINUS"]=45;
		m_mapStringKey["PERIOD"]=46;
		m_mapStringKey["SLASH"]=47;
		m_mapStringKey["0"]=48;
		m_mapStringKey["1"]=49;
		m_mapStringKey["2"]=50;
		m_mapStringKey["3"]=51;
		m_mapStringKey["4"]=52;
		m_mapStringKey["5"]=53;
		m_mapStringKey["6"]=54;
		m_mapStringKey["7"]=55;
		m_mapStringKey["8"]=56;
		m_mapStringKey["9"]=57;
		m_mapStringKey["COLON"]=58;
		m_mapStringKey["SEMICOLON"]=59;
		m_mapStringKey["LESS"]=60;
		m_mapStringKey["EQUALS"]=61;
		m_mapStringKey["GREATER"]=62;
		m_mapStringKey["QUESTION"]=63;
		m_mapStringKey["AT"]=64;
		m_mapStringKey["LEFTBRACKET"]=91;
		m_mapStringKey["BACKSLASH"]=92;
		m_mapStringKey["RIGHTBRACKET"]=93;
		m_mapStringKey["CARET"]=94;
		m_mapStringKey["UNDERSCORE"]=95;
		m_mapStringKey["BACKQUOTE"]=96;
		m_mapStringKey["a"]=m_mapStringKey["A"]=97;
		m_mapStringKey["b"]=m_mapStringKey["B"]=98;
		m_mapStringKey["c"]=m_mapStringKey["C"]=99;
		m_mapStringKey["d"]=m_mapStringKey["D"]=100;
		m_mapStringKey["e"]=m_mapStringKey["E"]=101;
		m_mapStringKey["f"]=m_mapStringKey["F"]=102;
		m_mapStringKey["g"]=m_mapStringKey["G"]=103;
		m_mapStringKey["h"]=m_mapStringKey["H"]=104;
		m_mapStringKey["i"]=m_mapStringKey["I"]=105;
		m_mapStringKey["j"]=m_mapStringKey["J"]=106;
		m_mapStringKey["k"]=m_mapStringKey["K"]=107;
		m_mapStringKey["l"]=m_mapStringKey["L"]=108;
		m_mapStringKey["m"]=m_mapStringKey["M"]=109;
		m_mapStringKey["n"]=m_mapStringKey["N"]=110;
		m_mapStringKey["o"]=m_mapStringKey["O"]=111;
		m_mapStringKey["p"]=m_mapStringKey["P"]=112;
		m_mapStringKey["q"]=m_mapStringKey["Q"]=113;
		m_mapStringKey["r"]=m_mapStringKey["R"]=114;
		m_mapStringKey["s"]=m_mapStringKey["S"]=115;
		m_mapStringKey["t"]=m_mapStringKey["T"]=116;
		m_mapStringKey["u"]=m_mapStringKey["U"]=117;
		m_mapStringKey["v"]=m_mapStringKey["V"]=118;
		m_mapStringKey["w"]=m_mapStringKey["W"]=119;
		m_mapStringKey["x"]=m_mapStringKey["X"]=120;
		m_mapStringKey["y"]=m_mapStringKey["Y"]=121;
		m_mapStringKey["z"]=m_mapStringKey["Z"]=122;
		m_mapStringKey["DELETE"]=127;
		m_mapStringKey["WORLD_0"]=160;
		m_mapStringKey["WORLD_1"]=161;
		m_mapStringKey["WORLD_2"]=162;
		m_mapStringKey["WORLD_3"]=163;
		m_mapStringKey["WORLD_4"]=164;
		m_mapStringKey["WORLD_5"]=165;
		m_mapStringKey["WORLD_6"]=166;
		m_mapStringKey["WORLD_7"]=167;
		m_mapStringKey["WORLD_8"]=168;
		m_mapStringKey["WORLD_9"]=169;
		m_mapStringKey["WORLD_10"]=170;
		m_mapStringKey["WORLD_11"]=171;
		m_mapStringKey["WORLD_12"]=172;
		m_mapStringKey["WORLD_13"]=173;
		m_mapStringKey["WORLD_14"]=174;
		m_mapStringKey["WORLD_15"]=175;
		m_mapStringKey["WORLD_16"]=176;
		m_mapStringKey["WORLD_17"]=177;
		m_mapStringKey["WORLD_18"]=178;
		m_mapStringKey["WORLD_19"]=179;
		m_mapStringKey["WORLD_20"]=180;
		m_mapStringKey["WORLD_21"]=181;
		m_mapStringKey["WORLD_22"]=182;
		m_mapStringKey["WORLD_23"]=183;
		m_mapStringKey["WORLD_24"]=184;
		m_mapStringKey["WORLD_25"]=185;
		m_mapStringKey["WORLD_26"]=186;
		m_mapStringKey["WORLD_27"]=187;
		m_mapStringKey["WORLD_28"]=188;
		m_mapStringKey["WORLD_29"]=189;
		m_mapStringKey["WORLD_30"]=190;
		m_mapStringKey["WORLD_31"]=191;
		m_mapStringKey["WORLD_32"]=192;
		m_mapStringKey["WORLD_33"]=193;
		m_mapStringKey["WORLD_34"]=194;
		m_mapStringKey["WORLD_35"]=195;
		m_mapStringKey["WORLD_36"]=196;
		m_mapStringKey["WORLD_37"]=197;
		m_mapStringKey["WORLD_38"]=198;
		m_mapStringKey["WORLD_39"]=199;
		m_mapStringKey["WORLD_40"]=200;
		m_mapStringKey["WORLD_41"]=201;
		m_mapStringKey["WORLD_42"]=202;
		m_mapStringKey["WORLD_43"]=203;
		m_mapStringKey["WORLD_44"]=204;
		m_mapStringKey["WORLD_45"]=205;
		m_mapStringKey["WORLD_46"]=206;
		m_mapStringKey["WORLD_47"]=207;
		m_mapStringKey["WORLD_48"]=208;
		m_mapStringKey["WORLD_49"]=209;
		m_mapStringKey["WORLD_50"]=210;
		m_mapStringKey["WORLD_51"]=211;
		m_mapStringKey["WORLD_52"]=212;
		m_mapStringKey["WORLD_53"]=213;
		m_mapStringKey["WORLD_54"]=214;
		m_mapStringKey["WORLD_55"]=215;
		m_mapStringKey["WORLD_56"]=216;
		m_mapStringKey["WORLD_57"]=217;
		m_mapStringKey["WORLD_58"]=218;
		m_mapStringKey["WORLD_59"]=219;
		m_mapStringKey["WORLD_60"]=220;
		m_mapStringKey["WORLD_61"]=221;
		m_mapStringKey["WORLD_62"]=222;
		m_mapStringKey["WORLD_63"]=223;
		m_mapStringKey["WORLD_64"]=224;
		m_mapStringKey["WORLD_65"]=225;
		m_mapStringKey["WORLD_66"]=226;
		m_mapStringKey["WORLD_67"]=227;
		m_mapStringKey["WORLD_68"]=228;
		m_mapStringKey["WORLD_69"]=229;
		m_mapStringKey["WORLD_70"]=230;
		m_mapStringKey["WORLD_71"]=231;
		m_mapStringKey["WORLD_72"]=232;
		m_mapStringKey["WORLD_73"]=233;
		m_mapStringKey["WORLD_74"]=234;
		m_mapStringKey["WORLD_75"]=235;
		m_mapStringKey["WORLD_76"]=236;
		m_mapStringKey["WORLD_77"]=237;
		m_mapStringKey["WORLD_78"]=238;
		m_mapStringKey["WORLD_79"]=239;
		m_mapStringKey["WORLD_80"]=240;
		m_mapStringKey["WORLD_81"]=241;
		m_mapStringKey["WORLD_82"]=242;
		m_mapStringKey["WORLD_83"]=243;
		m_mapStringKey["WORLD_84"]=244;
		m_mapStringKey["WORLD_85"]=245;
		m_mapStringKey["WORLD_86"]=246;
		m_mapStringKey["WORLD_87"]=247;
		m_mapStringKey["WORLD_88"]=248;
		m_mapStringKey["WORLD_89"]=249;
		m_mapStringKey["WORLD_90"]=250;
		m_mapStringKey["WORLD_91"]=251;
		m_mapStringKey["WORLD_92"]=252;
		m_mapStringKey["WORLD_93"]=253;
		m_mapStringKey["WORLD_94"]=254;
		m_mapStringKey["WORLD_95"]=255;
		m_mapStringKey["KP0"]=256;
		m_mapStringKey["KP1"]=257;
		m_mapStringKey["KP2"]=258;
		m_mapStringKey["KP3"]=259;
		m_mapStringKey["KP4"]=260;
		m_mapStringKey["KP5"]=261;
		m_mapStringKey["KP6"]=262;
		m_mapStringKey["KP7"]=263;
		m_mapStringKey["KP8"]=264;
		m_mapStringKey["KP9"]=265;
		m_mapStringKey["KP_PERIOD"]=266;
		m_mapStringKey["KP_DIVIDE"]=267;
		m_mapStringKey["KP_MULTIPLY"]=268;
		m_mapStringKey["KP_MINUS"]=269;
		m_mapStringKey["KP_PLUS"]=270;
		m_mapStringKey["KP_ENTER"]=271;
		m_mapStringKey["KP_EQUALS"]=272;
		m_mapStringKey["UP"]=273;
		m_mapStringKey["DOWN"]=274;
		m_mapStringKey["RIGHT"]=275;
		m_mapStringKey["LEFT"]=276;
		m_mapStringKey["INSERT"]=277;
		m_mapStringKey["HOME"]=278;
		m_mapStringKey["END"]=279;
		m_mapStringKey["PAGEUP"]=280;
		m_mapStringKey["PAGEDOWN"]=281;
		m_mapStringKey["F1"]=282;
		m_mapStringKey["F2"]=283;
		m_mapStringKey["F3"]=284;
		m_mapStringKey["F4"]=285;
		m_mapStringKey["F5"]=286;
		m_mapStringKey["F6"]=287;
		m_mapStringKey["F7"]=288;
		m_mapStringKey["F8"]=289;
		m_mapStringKey["F9"]=290;
		m_mapStringKey["F10"]=291;
		m_mapStringKey["F11"]=292;
		m_mapStringKey["F12"]=293;
		m_mapStringKey["F13"]=294;
		m_mapStringKey["F14"]=295;
		m_mapStringKey["F15"]=296;
		m_mapStringKey["NUMLOCK"]=300;
		m_mapStringKey["CAPSLOCK"]=301;
		m_mapStringKey["SCROLLOCK"]=302;
		m_mapStringKey["RSHIFT"]=303;
		m_mapStringKey["LSHIFT"]=304;
		m_mapStringKey["RCTRL"]=305;
		m_mapStringKey["LCTRL"]=306;
		m_mapStringKey["RALT"]=307;
		m_mapStringKey["LALT"]=308;
		m_mapStringKey["RMETA"]=309;
		m_mapStringKey["LMETA"]=310;
		m_mapStringKey["LSUPER"]=311;
		m_mapStringKey["RSUPER"]=312;
		m_mapStringKey["MODE"]=313;
		m_mapStringKey["COMPOSE"]=314;
		m_mapStringKey["HELP"]=315;
		m_mapStringKey["PRINT"]=316;
		m_mapStringKey["SYSREQ"]=17;
		m_mapStringKey["BREAK"]=318;
		m_mapStringKey["MENU"]=319;
		m_mapStringKey["POWER"]=320;
		m_mapStringKey["EURO"]=321;
		m_mapStringKey["UNDO"]=322;
	}

	void Input::SetMousePos( int x, int y )
	{
		SDL_WarpMouse( x, Window::Get()->Sizei().y - y );
	}

	void Input::HandleEvent( SDL_Event &event )
	{
		// handle keys
		// grab key
		char character = 0;
		uint16_t c = event.key.keysym.unicode;
		if ( c && (c & 0xFF80) == 0 ) character = c & 0x7f;

		if ( event.type == SDL_KEYDOWN ) 
		{
			m_keyDown = character;
		}
		else if ( event.type == SDL_KEYUP )
		{
			m_keyUp = character;
		}
		else if ( event.type == SDL_MOUSEBUTTONDOWN  )
		{
			// handle mousewheel
			if (event.button.button == SDL_BUTTON_WHEELUP ) m_mouseWheelHit[MOUSEWHEEL_UP] = true;
			else if (event.button.button == SDL_BUTTON_WHEELDOWN ) m_mouseWheelHit[MOUSEWHEEL_DOWN] = true;
			
		}
	}
}