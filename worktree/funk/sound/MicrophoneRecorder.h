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

    class MicrophoneRecorderFile
        : public HandledObj<MicrophoneRecorderFile>
	{
	public:
		void RecordStart();
		void RecordEnd();
		void Update();

		GM_BIND_TYPEID(MicrophoneRecorderFile);

		explicit MicrophoneRecorderFile( const char * file );
		~MicrophoneRecorderFile();

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

	GM_BIND_DECL( MicrophoneRecorderFile );

	class MicrophoneRecorder
        : public HandledObj<MicrophoneRecorder>
	{
	public:
		void RecordStart();
		void RecordEnd();

		void Update();

        void* GetData(int channel);
        int GetDataSize(int channel);

		GM_BIND_TYPEID(MicrophoneRecorder);

		MicrophoneRecorder(int frequency, int channels, FMOD_SOUND_FORMAT format);
		~MicrophoneRecorder();

	private:
		FMOD::Sound   *m_fmodsnd;
		FMOD::System  *m_fmodsys;

        // TODO: can we extract some from fmod instead of local copy?
        unsigned int m_frequency;
        unsigned int m_channels;
        unsigned int m_byteSize;
		unsigned int m_dataLength;
		unsigned int m_soundLength;
		unsigned int m_lastRecordPosLength;

        std::vector< std::vector<unsigned char> > m_data; 

		int m_recordDriver;

		void WriteWavHeader( unsigned int dataLength );		
	};

	GM_BIND_DECL( MicrophoneRecorder );
}
#endif