#include "SoundMngr.h"

#include <iostream>
#include <assert.h>
#include <common/Debug.h>
#include <imgui/Imgui.h>

#include "Sound.h"
#include "MicrophoneRecorder.h"

#define FMOD_ERRCHECK(res) if (res != 0) { MESSAGE_BOX("FMOD Error", "Error 0x%X, %s", res, #res); assert(false); }

namespace funk
{
typedef std::map< std::string, Sound* >::iterator SoundIter;

SoundMngr::SoundMngr()
{
	FMOD_RESULT result;
	unsigned int version;
	int numdrivers;
	int numHardwareChannels;
	FMOD_SPEAKERMODE speakermode;
	FMOD_CAPS caps;
	char name[256];

	//Create a System object and initialize.
	FMOD_ERRCHECK(FMOD::System_Create(&m_system));
	FMOD_ERRCHECK(m_system->getVersion(&version));
	FMOD_ERRCHECK(m_system->getHardwareChannels(&numHardwareChannels));
	FMOD_ERRCHECK(m_system->getNumDrivers(&numdrivers));

	if (version < FMOD_VERSION)
	{
		printf("Error! You are using an old version of FMOD %08x. This program requires %08x\n", version, FMOD_VERSION);
	}
	
	if (numdrivers == 0)
	{
		result = m_system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
		FMOD_ERRCHECK(result);
	}
	else
	{
		FMOD_ERRCHECK(m_system->getDriverCaps(0, &caps, 0, &speakermode));

		//Set the user selected speaker mode.
		FMOD_ERRCHECK(m_system->setSpeakerMode(speakermode));
		if (caps & FMOD_CAPS_HARDWARE_EMULATED)
		{
			/*
			The user has the 'Acceleration' slider set to off! This is really bad
			for latency! You might want to warn the user about this.
			*/
			result = m_system->setDSPBufferSize(1024, 10);
			FMOD_ERRCHECK(result);
		}
		FMOD_ERRCHECK(m_system->getDriverInfo(0, name, 256, 0));
		if (strstr(name, "SigmaTel"))
		{
			/*
			Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
			PCM floating point output seems to solve it.
			*/
			FMOD_ERRCHECK(m_system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR));
		}
	}

	printf("FMOD version 0x%X\n\tNum hardware channels: %d\n\tNum drivers: %d\n", version, numHardwareChannels, numdrivers);
	result = m_system->init(100, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
	{
		/*
		Ok, the speaker mode selected isn't supported by this soundcard. Switch it
		back to stereo...
		*/
		result = m_system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
		FMOD_ERRCHECK(result);
		/*
		... and re-init.
		*/
		result = m_system->init(100, FMOD_INIT_NORMAL, 0);
	}
	FMOD_ERRCHECK(result);
}

SoundMngr::~SoundMngr()
{
	m_mapSound.clear();

	m_system->close();
	m_system->release();	
}

StrongHandle<Sound> SoundMngr::GetSound( const char * filename, bool streaming, bool loop )
{
	assert( filename );

	SoundIter it = m_mapSound.find( filename );

	// not found
	if ( it == m_mapSound.end() || it->second == NULL )
	{
		StrongHandle<Sound> pSound = new Sound(filename, streaming, loop);
		AddSound( filename, pSound );
		return pSound;
	}

	return it->second;
}

void SoundMngr::Update()
{
	FMOD_RESULT result = m_system->update();
	FMOD_ERRCHECK( result );
}

void SoundMngr::Release( const char * filename )
{
	assert( filename );

	SoundIter it = m_mapSound.find( filename );
	CHECK( it != m_mapSound.end(), "Unable to release sound '%s'", filename );

	m_mapSound.erase( it );
}

void SoundMngr::AddSound( const char * filename, StrongHandle<Sound> snd )
{
	SoundIter it = m_mapSound.find( filename );
	assert( it == m_mapSound.end() );
	m_mapSound[filename] = snd;
}

void SoundMngr::GuiStats()
{
	int numChannelsPlaying;
	int usageMemory;
	float usageDSP;
	float usageStream;
	float usageGeometry;
	float usageUpdate;
	float usageTotal;

	// system info
	m_system->getChannelsPlaying( &numChannelsPlaying );
	m_system->getCPUUsage( &usageDSP, &usageStream, &usageGeometry, &usageUpdate, &usageTotal );

	// system memory
	unsigned int mem;
	m_system->getMemoryInfo( FMOD_MEMBITS_ALL, FMOD_EVENT_MEMBITS_ALL, &mem, NULL );
	usageMemory = mem;

	Imgui::Header("FMOD");
	Imgui::FillBarInt("Num Sounds", (int)m_mapSound.size(), 0, 128 );
	Imgui::FillBarInt("Mem Usage (Bytes)", usageMemory, 0, 64000000 );
	Imgui::FillBarInt("Channels", numChannelsPlaying, 0, 64 );
	Imgui::FillBarFloat("DSP", usageDSP, 0.0f, 16.0f );
	Imgui::FillBarFloat("Stream", usageStream, 0.0f, 16.0f );
	Imgui::FillBarFloat("Geometry", usageGeometry, 0.0f, 16.0f );
	Imgui::FillBarFloat("Update", usageUpdate, 0.0f, 16.0f );
	Imgui::FillBarFloat("Total", usageTotal, 0.0f, 16.0f );
}

}