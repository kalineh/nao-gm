#include "MicrophoneRecorder.h"

#include <gm/gmBind.h>
#include <gm/gmBindFuncGen.h>
#include "SoundMngr.h"
#include <memory>

#include <common/Debug.h>

#define FMOD_ERRCHECK(res) if (res != 0) { MESSAGE_BOX("FMOD Error", "Error 0x%X, %s", res, #res); CHECK(false); }

namespace funk
{

MicrophoneRecorderFile::MicrophoneRecorderFile( const char * file )
    : m_fmodsnd(nullptr)
    , m_fmodsys(nullptr)
    , m_fileName(file)
    , m_fh(0)
    , m_dataLength(0)
{
	FMOD::System * fmodSys = SoundMngr::Get()->GetSys();

	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.numchannels      = 1;
	exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
	exinfo.defaultfrequency = 44100;
	exinfo.length           = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * 2;

	unsigned int flags = FMOD_SOFTWARE | FMOD_2D | FMOD_OPENUSER;
	FMOD_ERRCHECK(fmodSys->createSound(0, flags, &exinfo, &m_fmodsnd));

	m_fmodsys = fmodSys;
}

MicrophoneRecorderFile::~MicrophoneRecorderFile()
{
	if ( m_fmodsnd )
        m_fmodsnd->release();
}

void MicrophoneRecorderFile::RecordStart()
{
	char name[256];
	FMOD_ERRCHECK(m_fmodsys->getRecordDriverInfo(1, name, 256, 0));
	m_recordDriver = 0;

	FMOD_ERRCHECK(m_fmodsys->recordStart(m_recordDriver, m_fmodsnd, true));

	m_fh = fopen(m_fileName.c_str(), "wb");
	CHECK(m_fh != 0, "Error cannot open file %s", m_fileName.c_str() );

	WriteWavHeader(m_dataLength);

	FMOD_ERRCHECK(m_fmodsnd->getLength(&m_soundLength, FMOD_TIMEUNIT_PCM));

	m_lastRecordPosLength  = 0;
}

void MicrophoneRecorderFile::RecordEnd()
{
	WriteWavHeader(m_dataLength);
	FMOD_ERRCHECK(m_fmodsnd->release());
	fclose(m_fh);
}

void MicrophoneRecorderFile::Update()
{
	unsigned int recordpos = 0;
	m_fmodsys->getRecordPosition(m_recordDriver, &recordpos);

	int numChannels = 1;

	if (recordpos != m_lastRecordPosLength)        
	{
		void *ptr1, *ptr2;
		int blocklength;
		unsigned int len1, len2;

		blocklength = (int)recordpos - (int)m_lastRecordPosLength;
		if (blocklength < 0) blocklength += m_soundLength;

		// Lock the sound to get access to the raw data.
		m_fmodsnd->lock(m_lastRecordPosLength * numChannels * 2, blocklength * numChannels * 2, &ptr1, &ptr2, &len1, &len2);   /* * exinfo.numchannels * 2 = stereo 16bit.  1 sample = 4 bytes. */

		// Write it to disk.
		if (ptr1 && len1) m_dataLength += fwrite(ptr1, 1, len1, m_fh);
		if (ptr2 && len2) m_dataLength += fwrite(ptr2, 1, len2, m_fh);

		//Unlock the sound to allow FMOD to use it again.
		m_fmodsnd->unlock(ptr1, ptr2, len1, len2);
	}

	m_lastRecordPosLength = recordpos;
}

void MicrophoneRecorderFile::WriteWavHeader( unsigned int dataLength )
{
	int             channels, bits;
	float           rate;

	fseek(m_fh, 0, SEEK_SET);

	m_fmodsnd->getFormat  (0, 0, &channels, &bits);
	m_fmodsnd->getDefaults(&rate, 0, 0, 0);

	#if defined(WIN32) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
	#define __PACKED                         /* dummy */
	#else
	#define __PACKED __attribute__((packed)) /* gcc packed */
	#endif

	{
	#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
	#pragma pack(1)
	#endif

		/*
		WAV Structures
		*/
		typedef struct
		{
			signed char id[4];
			int 		size;
		} RiffChunk;

		struct
		{
			RiffChunk       chunk           __PACKED;
			unsigned short	wFormatTag      __PACKED;    /* format type  */
			unsigned short	nChannels       __PACKED;    /* number of channels (i.e. mono, stereo...)  */
			unsigned int	nSamplesPerSec  __PACKED;    /* sample rate  */
			unsigned int	nAvgBytesPerSec __PACKED;    /* for buffer estimation  */
			unsigned short	nBlockAlign     __PACKED;    /* block size of data  */
			unsigned short	wBitsPerSample  __PACKED;    /* number of bits per sample of mono data */
		} FmtChunk  = { {{'f','m','t',' '}, sizeof(FmtChunk) - sizeof(RiffChunk) }, 1, channels, (int)rate, (int)rate * channels * bits / 8, 1 * channels * bits / 8, bits } __PACKED;

		struct
		{
			RiffChunk   chunk;
		} DataChunk = { {{'d','a','t','a'}, dataLength } };

		struct
		{
			RiffChunk   chunk;
			signed char rifftype[4];
		} WavHeader = { {{'R','I','F','F'}, sizeof(FmtChunk) + sizeof(RiffChunk) + dataLength }, {'W','A','V','E'} };

	#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
	#pragma pack()
	#endif

		/*
		Write out the WAV header.
		*/
		fwrite(&WavHeader, sizeof(WavHeader), 1, m_fh);
		fwrite(&FmtChunk, sizeof(FmtChunk), 1, m_fh);
		fwrite(&DataChunk, sizeof(DataChunk), 1, m_fh);
	}
}

MicrophoneRecorder::MicrophoneRecorder(int frequency, int channels, FMOD_SOUND_FORMAT format)
    : m_fmodsnd(nullptr)
    , m_fmodsys(nullptr)
    , m_dataLength(0)
    , m_byteSize(0)
    , m_frequency(frequency)
    , m_channels(channels)
{
    switch (format)
    {
        case FMOD_SOUND_FORMAT_PCM8: m_byteSize = 1; break;
        case FMOD_SOUND_FORMAT_PCM16: m_byteSize = 2; break;
        case FMOD_SOUND_FORMAT_PCM24: m_byteSize = 3; break;
        case FMOD_SOUND_FORMAT_PCM32: m_byteSize = 4; break;
        case FMOD_SOUND_FORMAT_PCMFLOAT: m_byteSize = 4; break;
    }

    CHECK(m_byteSize > 0);

	FMOD::System * fmodSys = SoundMngr::Get()->GetSys();

	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.numchannels      = channels;
	exinfo.format           = format;
	exinfo.defaultfrequency = frequency;
	exinfo.length           = exinfo.defaultfrequency * m_byteSize * exinfo.numchannels * 2;

	unsigned int flags = FMOD_SOFTWARE | FMOD_2D | FMOD_OPENUSER;
	FMOD_ERRCHECK(fmodSys->createSound(0, flags, &exinfo, &m_fmodsnd));

	m_fmodsys = fmodSys;

    m_data.resize(channels);
}

MicrophoneRecorder::~MicrophoneRecorder()
{
	if ( m_fmodsnd )
        m_fmodsnd->release();
}

void MicrophoneRecorder::RecordStart()
{
    char name[256] = { 0 };
	FMOD_ERRCHECK(m_fmodsys->getRecordDriverInfo(1, name, 256, 0));
	m_recordDriver = 0;

	FMOD_ERRCHECK(m_fmodsys->recordStart(m_recordDriver, m_fmodsnd, true));
	FMOD_ERRCHECK(m_fmodsnd->getLength(&m_soundLength, FMOD_TIMEUNIT_PCM));

	m_lastRecordPosLength  = 0;
}

void MicrophoneRecorder::RecordEnd()
{
	FMOD_ERRCHECK(m_fmodsnd->release());
}

void MicrophoneRecorder::Update()
{
    // get the current position N in sound buffer
	unsigned int recordpos = 0;
	m_fmodsys->getRecordPosition(m_recordDriver, &recordpos);

    // did the current pos change from last time?
	if (recordpos != m_lastRecordPosLength)        
	{
		void *ptr1, *ptr2;
		int blocklength;
		unsigned int len1, len2;

        // the block length is the number of samples since the last record pos to the latest data
		blocklength = (int)recordpos - (int)m_lastRecordPosLength;

        // pull whole blocks if the recordpos is reset?
		if (blocklength < 0)
        {
            blocklength += m_soundLength;
        }

		// the buffer will be split into two parts when it wraps around the internal buffer size
		m_fmodsnd->lock(m_lastRecordPosLength * m_channels * m_byteSize, blocklength * m_channels * m_byteSize, &ptr1, &ptr2, &len1, &len2);

        // TODO: channel splitting
        std::vector<unsigned char>& data = m_data[0];

        data.resize(len1 + len2);

        // first block should always be valid if there is data
		if (ptr1 && len1)
        {
            memcpy(&data[0], ptr1, len1);
        }

        // second block if the data buffer wrapped
		if (ptr2 && len2)
        {
            memcpy(&data[0] + len1, ptr2, len2);
        }

		// unlock for fmod to use again
		m_fmodsnd->unlock(ptr1, ptr2, len1, len2);
	}

	m_lastRecordPosLength = recordpos;
}

void* MicrophoneRecorder::GetData(int channel)
{
    if (m_data[channel].empty())
        return nullptr;

    return &m_data[channel][0];
}

int MicrophoneRecorder::GetDataSize(int channel)
{
    return m_data[channel].size();
}

GM_REG_NAMESPACE(MicrophoneRecorderFile)
{
	GM_MEMFUNC_CONSTRUCTOR(MicrophoneRecorderFile)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM( file, 0 );

		StrongHandle<MicrophoneRecorderFile> p = new MicrophoneRecorderFile(file);
		GM_PUSH_USER_HANDLED( MicrophoneRecorderFile, p );
		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_VOID( MicrophoneRecorderFile, RecordStart )
	GM_GEN_MEMFUNC_VOID_VOID( MicrophoneRecorderFile, RecordEnd )
	GM_GEN_MEMFUNC_VOID_VOID( MicrophoneRecorderFile, Update )
}

GM_REG_MEM_BEGIN(MicrophoneRecorderFile)
GM_REG_MEMFUNC( MicrophoneRecorderFile, RecordStart )
GM_REG_MEMFUNC( MicrophoneRecorderFile, RecordEnd )
GM_REG_MEMFUNC( MicrophoneRecorderFile, Update )
GM_REG_HANDLED_DESTRUCTORS(MicrophoneRecorderFile)
GM_REG_MEM_END()
GM_BIND_DEFINE(MicrophoneRecorderFile)

GM_REG_NAMESPACE(MicrophoneRecorder)
{
	GM_MEMFUNC_CONSTRUCTOR(MicrophoneRecorder)
	{
		GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(frequency, 0);
        GM_CHECK_INT_PARAM(channels, 1);
        GM_CHECK_INT_PARAM(format, 2);

		StrongHandle<MicrophoneRecorder> p = new MicrophoneRecorder(frequency, channels, (FMOD_SOUND_FORMAT)format);
		GM_PUSH_USER_HANDLED( MicrophoneRecorder, p );
		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_VOID( MicrophoneRecorder, RecordStart )
	GM_GEN_MEMFUNC_VOID_VOID( MicrophoneRecorder, RecordEnd )
	GM_GEN_MEMFUNC_VOID_VOID( MicrophoneRecorder, Update )
}

GM_REG_MEM_BEGIN(MicrophoneRecorder)
GM_REG_MEMFUNC( MicrophoneRecorder, RecordStart )
GM_REG_MEMFUNC( MicrophoneRecorder, RecordEnd )
GM_REG_MEMFUNC( MicrophoneRecorder, Update )
GM_REG_HANDLED_DESTRUCTORS(MicrophoneRecorder)
GM_REG_MEM_END()
GM_BIND_DEFINE(MicrophoneRecorder)

}