#include "MicrophoneRecorder.h"

#include <gm/gmBind.h>
#include <gm/gmBindFuncGen.h>
#include "SoundMngr.h"
#include <memory>

#include <common/Debug.h>

#define FMOD_ERRCHECK(res) if (res != 0) { MESSAGE_BOX("FMOD Error", "Error 0x%X, %s", res, #res); assert(false); }

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

MicrophoneRecorder::MicrophoneRecorder()
    : m_fmodsnd(nullptr)
    , m_fmodsys(nullptr)
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

		// Write it to output.
		if (ptr1 && len1)
        {
            //left->resize(len1);
            //memcpy(&(*left)[0], ptr1, len1);
        }

		if (ptr2 && len2)
        {
            //right->resize(len2);
            //memcpy(&(*right)[0], ptr2, len2);
        }

		//Unlock the sound to allow FMOD to use it again.
		m_fmodsnd->unlock(ptr1, ptr2, len1, len2);
	}

	m_lastRecordPosLength = recordpos;
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
		GM_CHECK_NUM_PARAMS(0);

		StrongHandle<MicrophoneRecorder> p = new MicrophoneRecorder();
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