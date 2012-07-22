#ifndef _INCLUDE_SOUND_H_
#define _INCLUDE_SOUND_H_

#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>
#include <fmod.hpp>
#include <string>

namespace funk
{
	class SoundMngr;

	class Sound : public HandledObj<Sound>
	{
	public:

		void Play();
		void Stop();
		void SetVolume(float vol);
		void SetPan(float pan);
		void SetPause( bool pause );

		bool IsPlaying() const;
		
		// fft data
		void GetSpectrum(  float *arr, int numVals ) const;

		// sound position
		unsigned int GetTotalLenMillisec() const;
		unsigned int GetCurrPosMillisec() const;
		void SetPosMillisec( unsigned int pos );

		void Reload();

		const std::string & FileName() const { return m_filename; }

		GM_BIND_TYPEID(Sound);
		~Sound();

	private:
		
		float m_volume;
		std::string m_filename;
		unsigned int m_flags;

		FMOD::Sound   *m_fmodsnd;
		FMOD::Channel *m_fmodchn;

		void Init();
		void Release();

		friend class SoundMngr;
		Sound( const char * fileName, bool streaming, bool loop );
	};

	GM_BIND_DECL( Sound );
}
#endif