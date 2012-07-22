#include "SoundRecorder.h"

#include <gm/gmBind.h>
#include <gm/gmBindFuncGen.h>
#include "SoundMngr.h"
#include <memory>

#include <common/Debug.h>
#include <common/Util.h>

#define FMOD_ERRCHECK(res) assert(res == 0)

namespace funk
{
SoundRecorder::SoundRecorder() : 
m_fmodsys(0), m_fh(0), m_dataLength(0)
{
	FMOD::System * fmodSys = SoundMngr::Get()->GetSys();
	m_fmodsys = fmodSys;
}

SoundRecorder::~SoundRecorder()
{
	if ( m_fh ) fclose(m_fh);
}

void SoundRecorder::RecordStart( const char * file )
{
	m_dataLength = 0;

	m_fh = fopen(file, "wb");
	CHECK(m_fh != 0, "Error cannot open file %s", file );

	WriteWavHeader(m_dataLength);
}

void SoundRecorder::RecordEnd()
{
	if ( m_fh == 0 ) return;

	WriteWavHeader(m_dataLength);

	fclose(m_fh);
	m_fh = 0;
}

void SoundRecorder::Update()
{
	static float time = 0.0f;
	time += 1.0f / 60.0f;

	if ( m_fh == 0 ) return;

	const unsigned int samplesPerSec = 48000;
	const unsigned int fps = 60;
	const int numSamplesPerFrame = samplesPerSec / fps;
	float data[2][numSamplesPerFrame];
	short conversion[numSamplesPerFrame*2];

	m_fmodsys->getWaveData( &data[0][0], numSamplesPerFrame, 0 ); // left channel
	m_fmodsys->getWaveData( &data[1][0], numSamplesPerFrame, 1 ); // right channel

	int littleEndian = IsLittleEndian();

	const float freq = 420.0f;

	for ( int i = 0; i < numSamplesPerFrame; ++i )
	{
		float val =  sinf(2.0f * 3.14159f * freq * 0.016666f * i / (numSamplesPerFrame-1));

		// left channel
		float coeff_left = val;//data[0][i];
		short val_left =  (short)(coeff_left * 0x7FFF);

		// right channel
		float coeff_right = val;//data[1][i];
		short val_right =  (short)(coeff_right * 0x7FFF);

		// handle endianness
		if ( !littleEndian )
		{
			//val_left = ((val_left & 0xff) << 8) | (val_left >> 8);
			//val_right = ((val_right & 0xff) << 8) | (val_right >> 8);
		}
		
		conversion[i*2+0] = val_left;
		conversion[i*2+1] = val_right;
	}

	fwrite((void*)&conversion[0], sizeof(conversion[0]), numSamplesPerFrame*2, m_fh);
	m_dataLength += sizeof(conversion);
}

void SoundRecorder::WriteWavHeader( unsigned int dataLength )
{
	if ( m_fh == 0 ) return;

	// fmod format
	int					rate;
	FMOD_SOUND_FORMAT	format;
	int					channels;
	int					maxInputChannels;
	FMOD_DSP_RESAMPLER	resampleMethod;
	int					bits;

	m_fmodsys->getSoftwareFormat( &rate, &format, &channels, &maxInputChannels, &resampleMethod, &bits );

	// TEmp!!!
	bits = 16;

	fseek(m_fh, 0, SEEK_SET);

	#if defined(WIN32) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
	#define __PACKED
	#else
	#define __PACKED __attribute__((packed))
	#endif

	{
		#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
		#pragma pack(1)
		#endif

		// WAV Structures
		typedef struct
		{
			signed char id[4];
			int 		size;
		} RiffChunk;

		struct
		{
			RiffChunk       chunk           __PACKED;
			unsigned short	wFormatTag      __PACKED; 
			unsigned short	nChannels       __PACKED;
			unsigned int	nSamplesPerSec  __PACKED;
			unsigned int	nAvgBytesPerSec __PACKED;
			unsigned short	nBlockAlign     __PACKED;
			unsigned short	wBitsPerSample  __PACKED;
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

		void * wavPtr = &WavHeader;
		void * fmtPtr = &FmtChunk;
		void * datPtr = &DataChunk;

		//Write out the WAV header.
		fwrite(&WavHeader, sizeof(WavHeader), 1, m_fh);
		fwrite(&FmtChunk, sizeof(FmtChunk), 1, m_fh);
		fwrite(&DataChunk, sizeof(DataChunk), 1, m_fh);
	}	
}

GM_REG_NAMESPACE(SoundRecorder)
{
	GM_MEMFUNC_CONSTRUCTOR(SoundRecorder)
	{
		GM_CHECK_NUM_PARAMS(0);

		StrongHandle<SoundRecorder> p = new SoundRecorder();
		GM_PUSH_USER_HANDLED( SoundRecorder, p );
		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_STRING( SoundRecorder, RecordStart )
	GM_GEN_MEMFUNC_VOID_VOID( SoundRecorder, RecordEnd )
	GM_GEN_MEMFUNC_VOID_VOID( SoundRecorder, Update )
};

GM_REG_MEM_BEGIN(SoundRecorder)
GM_REG_MEMFUNC( SoundRecorder, RecordStart )
GM_REG_MEMFUNC( SoundRecorder, RecordEnd )
GM_REG_MEMFUNC( SoundRecorder, Update )
GM_REG_HANDLED_DESTRUCTORS(SoundRecorder)
GM_REG_MEM_END()
GM_BIND_DEFINE(SoundRecorder)

}