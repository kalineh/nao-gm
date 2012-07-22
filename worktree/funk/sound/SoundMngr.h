#ifndef _INCLUDE_SOUND_MNGR_H_
#define _INCLUDE_SOUND_MNGR_H_

#include <common/Singleton.h>
#include <common/StrongHandle.h>

#include <fmod.hpp>
#include <map>
#include <string>

namespace funk
{
	class Sound;
	class MicrophoneRecorder;

	class SoundMngr : public Singleton<SoundMngr>
	{
	public:

		StrongHandle<Sound> GetSound( const char * filename, bool streaming = false, bool loop = false );		
		void Update();
		void GuiStats();

		FMOD::System * GetSys() { return m_system; }

	private:
		std::map< std::string, Sound* > m_mapSound;

		FMOD::System *m_system;

		friend Sound;
		friend MicrophoneRecorder;
		void Release( const char * filename );
		void AddSound( const char * filename, StrongHandle<Sound> snd );

		friend Singleton<SoundMngr>;
		SoundMngr();
		~SoundMngr();
	};
}

#endif