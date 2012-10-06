#ifndef _INCLUDE_MICROPHONE_RECORDER_H_
#define _INCLUDE_MICROPHONE_RECORDER_H_

#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>
#include <fmod.hpp>
#include <string>
#include <vector>

namespace funk
{
	class SoundMngr;

	class MicrophoneRecorder : public HandledObj<MicrophoneRecorder>
	{
	public:
		void RecordStart();
		void RecordEnd();
		void Update();

		void RecordStartNoFile();
		void RecordEndNoFile();
        void UpdateNoFile(std::vector<unsigned short>* left, std::vector<unsigned short>* right);

		GM_BIND_TYPEID(MicrophoneRecorder);
        // TODO: no-file ctor: replace with difference/subclass
		MicrophoneRecorder();

		MicrophoneRecorder( const char * file );
		~MicrophoneRecorder();

	private:
		FMOD::Sound   *m_fmodsnd;
		FMOD::System  *m_fmodsys;

		std::string m_fileName;
		FILE * m_fh;
		unsigned int m_dataLength;
		unsigned int m_soundLength;
		unsigned int m_lastRecordPosLength;

		int m_recordDriver;

		void WriteWavHeader( unsigned int dataLength );		
	};

	GM_BIND_DECL( MicrophoneRecorder );
}
#endif