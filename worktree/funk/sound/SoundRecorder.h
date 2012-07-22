#ifndef _INCLUDE_SOUND_RECORDER_H_
#define _INCLUDE_SOUND_RECORDER_H_

#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>
#include <fmod.hpp>
#include <string>

namespace funk
{
	class SoundMngr;

	class SoundRecorder : public HandledObj<SoundRecorder>
	{
	public:
		void RecordStart( const char * file );
		void RecordEnd();
		void Update();

		GM_BIND_TYPEID(SoundRecorder);
		SoundRecorder();
		~SoundRecorder();

	private:
		FMOD::System  *m_fmodsys;
		FILE * m_fh;

		unsigned int m_dataLength;

		void WriteWavHeader( unsigned int dataLength );		
	};

	GM_BIND_DECL( SoundRecorder );
}
#endif