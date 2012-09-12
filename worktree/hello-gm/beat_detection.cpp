//
// beat_detection.cpp
//

#include "beat_detection.h"

ALSoundProcessing::ALSoundProcessing(boost::shared_ptr<AL::ALBroker> broker, std::string name)
    : AL::ALSoundExtractor(broker, name)
{
    setModuleDescription("Collect raw microphone data to a local double buffer.");
}

ALSoundProcessing::~ALSoundProcessing()
{
}

void ALSoundProcessing::init()
{
    const int request_interleaving = 1;
    audioDevice->callVoid(
        "setClientPreferences",
        getName(),
        48000,
        int(AL::ALLCHANNELS),
        request_interleaving
    );
}

// sample code
void average_sound_data(const int& channels, const int& samples, const AL_SOUND_FORMAT* buffer, const AL::ALValue& timestamp)
{
    // average data per channel
    std::vector<float> energy;

    for (int i = 0; i < channels; ++i)
        energy.push_back(0.0f);

    // audio root-mean-square power

    for (int i = 0; i < channels; ++i)
    {
        const signed short* channel = buffer + samples;

        for (int j = 0; j < samples; ++j)
        {
            const signed short value = channel[j];
            const float e = float(value); // normalize? value / SHORT_MAX?

            energy[i] += e * e;
        }

        energy[i] /= float(samples);
        energy[i] = sqrtf(energy[i]);
    }
}

void ALSoundProcessing::process(const int& channels, const int& samples, const AL_SOUND_FORMAT* buffer, const AL::ALValue& timestamp)
{
    const int length = channels * samples;

    _buffer.clear();
    _buffer.resize(length);

    for (int i = 0; i < length; ++i)
        _buffer[i] = float(buffer[i]);
}

GMAudioStream::GMAudioStream(const char* name, const char* ip, int port)
    : _active(false)
    //, _proxy(std::string(ip), port)
    , _name(name)
    , _subscriber_id(name)
{
}

void GMAudioStream::SetActive(bool active)
{
    if (active && !_active)
    {
        Subscribe();
        _active = active;
        return;
    }

    if (!active && _active)
    {
        //_proxy.unsubscribe(_subscriber_id);        
        _active = active;
        _subscriber_id = _name;
    }
}

void GMAudioStream::Update()
{
    if (!_active)
        return;

    GetRemoteAudioData();
}

void GMAudioStream::BeatFFT()
{
    // TODO!
}

void GMAudioStream::BeatEnergy()
{
}

void GMAudioStream::Subscribe()
{
    try
    {
        //_proxy.unsubscribe(_subscriber_id);
    }
    catch (const AL::ALError&)
    {
        // ignore, just attempting to avoid hanging subscriptions
    }

    //_subscriber_id = _proxy.subscribe(_name);

    //_proxy.setParam(AL::kCameraVFlipID, 0);
}

void GMAudioStream::GetRemoteAudioData()
{
}

GM_REG_NAMESPACE(GMAudioStream)
{
	GM_MEMFUNC_DECL(CreateGMAudioStream)
	{
		GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_STRING_PARAM(name, 0);
        GM_CHECK_STRING_PARAM(ip, 1);
        GM_CHECK_INT_PARAM(port, 2);
		GM_AL_EXCEPTION_WRAPPER(GM_PUSH_USER_HANDLED( GMAudioStream, new GMAudioStream(name, ip, port) ));
		return GM_OK;
	}

    GM_MEMFUNC_DECL(SetActive)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(active, 0);

		GM_GET_THIS_PTR(GMAudioStream, self);
		GM_AL_EXCEPTION_WRAPPER(self->SetActive(active != 0));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(Update)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->Update());
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMAudioStream)
GM_REG_MEMFUNC( GMAudioStream, SetActive )
GM_REG_MEMFUNC( GMAudioStream, Update )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMAudioStream);
