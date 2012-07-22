#include "Sound.h"

#include <gm/gmBind.h>
#include <gm/gmBindFuncGen.h>
#include <common/Debug.h>
#include "SoundMngr.h"
#include <memory>

namespace funk
{
Sound::Sound( const char * filename, bool streaming, bool loop) : 
m_fmodsnd(0), m_fmodchn(0), m_filename( filename ), m_volume(1.0f)
{
	m_flags = FMOD_SOFTWARE | FMOD_2D;
	m_flags |= ( streaming ? FMOD_CREATESTREAM : FMOD_CREATESAMPLE );
	m_flags |= ( loop ? FMOD_LOOP_NORMAL : 0x0 );

	Init();
}

Sound::~Sound()
{
	SoundMngr::Get()->Release( FileName().c_str() );
	Release();
}

void Sound::Init()
{
	FMOD::System * fmodSys = SoundMngr::Get()->GetSys();
	FMOD_RESULT result = fmodSys->createSound(m_filename.c_str(), m_flags, 0, &m_fmodsnd);
	CHECK( result == 0, "Unable to load sound: '%s'!\n", m_filename.c_str() );
}

void Sound::Release()
{
	if ( m_fmodsnd ) m_fmodsnd->release();
}

void Sound::Reload()
{
	Release();
	Init();
}

void Sound::Play()
{
	FMOD::System * fmodSys = SoundMngr::Get()->GetSys();
	FMOD_RESULT result = fmodSys->playSound(FMOD_CHANNEL_FREE, m_fmodsnd, false, &m_fmodchn);
	SetVolume(m_volume);
}

void Sound::Stop()
{
	if ( !m_fmodchn ) return;
	m_fmodchn->stop();
}

void Sound::SetVolume( float vol )
{
	m_volume = vol;
	m_fmodchn->setVolume( vol );
}

unsigned int Sound::GetTotalLenMillisec() const
{
	unsigned int ms;
	m_fmodsnd->getLength( &ms, FMOD_TIMEUNIT_MS );
	return ms;
}

void Sound::SetPosMillisec( unsigned int pos )
{
	m_fmodchn->setPosition( pos, FMOD_TIMEUNIT_MS );
}

void Sound::GetSpectrum( float *arr, int numVals ) const
{
	if ( IsPlaying() && GetCurrPosMillisec() < GetTotalLenMillisec() )
	{
		FMOD_RESULT result = m_fmodchn->getSpectrum( arr, numVals, 0, FMOD_DSP_FFT_WINDOW_BLACKMAN ); 	
	}
	else
	{
		memset( arr, 0, numVals * sizeof(float) );
	}
}

void Sound::SetPan(float pan)
{
	m_fmodchn->setPan( pan );
}

bool Sound::IsPlaying() const
{
	if ( !m_fmodchn ) return false;

	bool playing;
	m_fmodchn->getPaused( &playing );
	return !playing;
}

unsigned int Sound::GetCurrPosMillisec() const
{
	unsigned int ms;
	m_fmodchn->getPosition( &ms, FMOD_TIMEUNIT_MS );
	return ms;
}

void Sound::SetPause( bool pause )
{
	if ( !m_fmodchn ) return;
	m_fmodchn->setPaused( pause );
}

GM_REG_NAMESPACE(Sound)
{
	GM_MEMFUNC_CONSTRUCTOR(Sound)
	{
		int numParams = a_thread->GetNumParams();
		bool loop = 0;

		GM_CHECK_STRING_PARAM( file, 0 );
		GM_INT_PARAM( looping, 1, 0 );
		GM_INT_PARAM( streaming, 2, 0 );

		StrongHandle<Sound> p = SoundMngr::Get()->GetSound(file, streaming != 0, looping != 0 );
		GM_PUSH_USER_HANDLED( Sound, p.Get() );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetPosMillisec)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM( pos_millisec, 0 );
		GM_GET_THIS_PTR(Sound, ptr);
		ptr->SetPosMillisec(pos_millisec);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(GetTotalLenMillisec)
	{
		GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(Sound, ptr);
		a_thread->PushInt( ptr->GetTotalLenMillisec());
		return GM_OK;
	}

	GM_MEMFUNC_DECL(GetCurrPosMillisec)
	{
		GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(Sound, ptr);
		a_thread->PushInt( ptr->GetCurrPosMillisec());
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetPause)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM( pause, 0 );
		GM_GET_THIS_PTR(Sound, ptr);
		ptr->SetPause(pause != 0);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(GetSpectrumValue)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM( band, 0 );

		GM_GET_THIS_PTR(Sound, ptr);
		
		const int numVals = 64;
		float buffer[numVals];
		assert( band >=0 && band < numVals );
		ptr->GetSpectrum( buffer, numVals ); 
		
		a_thread->PushFloat( buffer[band] );
		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_VOID( Sound, Play )
	GM_GEN_MEMFUNC_VOID_FLOAT( Sound, SetVolume )
	GM_GEN_MEMFUNC_VOID_FLOAT( Sound, SetPan )
	GM_GEN_MEMFUNC_INT_VOID( Sound, IsPlaying )
	GM_GEN_MEMFUNC_VOID_VOID( Sound, Stop )
	GM_GEN_MEMFUNC_VOID_VOID( Sound, Reload )
}

GM_REG_MEM_BEGIN(Sound)
GM_REG_MEMFUNC( Sound, Play )
GM_REG_MEMFUNC( Sound, Stop )
GM_REG_MEMFUNC( Sound, SetVolume )
GM_REG_MEMFUNC( Sound, SetPause )
GM_REG_MEMFUNC( Sound, SetPan )
GM_REG_MEMFUNC( Sound, GetTotalLenMillisec )
GM_REG_MEMFUNC( Sound, GetCurrPosMillisec )
GM_REG_MEMFUNC( Sound, GetSpectrumValue )
GM_REG_MEMFUNC( Sound, SetPosMillisec )
GM_REG_MEMFUNC( Sound, IsPlaying )
GM_REG_MEMFUNC( Sound, Reload )
GM_REG_HANDLED_DESTRUCTORS(Sound)
GM_REG_MEM_END()
GM_BIND_DEFINE(Sound)

}