//
// beat_detection.cpp
//

#include "beat_detection.h"

#include <gfx/DrawPrimitives.h>

ALSoundProcessing::ALSoundProcessing(boost::shared_ptr<AL::ALBroker> broker, std::string name)
    : AL::ALSoundExtractor(broker, name)
    , _process_mutex(AL::ALMutex::createALMutex())
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

    AL::ALCriticalSection section(_process_mutex);

    _buffer.clear();
    _buffer.resize(length);

    for (int i = 0; i < length; ++i)
        _buffer[i] = float(buffer[i]);
}

void ALSoundProcessing::ExtractChannel(int channel, std::vector<float>& results)
{
    AL::ALCriticalSection section(_process_mutex);

    const int channels = 4;
    const int samples = _buffer.size() / channels;

    const int start = samples * channel;

    results.resize(samples);
    for (int i = 0; i < samples; ++i)
        results[i] = _buffer[start + i];
}

GMAudioStream::GMAudioStream(const char* name, const char* ip, int port)
    : _active(false)
    , _name(name)
    , _subscriber_id(name)
{
    AL::ALBroker::Ptr broker = AL::ALBroker::createBroker("main", "0.0.0.0", 54000, ip, port);
    _sound_processing = AL::ALModule::createModule<ALSoundProcessing>(broker, "ALSoundProcessing");
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
        _sound_processing->unsubscribe(_subscriber_id);
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

void GMAudioStream::CalcBeatFFT()
{
    // TODO!
}

void GMAudioStream::CalcBeatEnergy()
{
}

void GMAudioStream::DrawWaveform(int channel, v3 color, float alpha)
{
    const std::vector<float>& data = _data[channel];
    const int count = data.size();

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = v2(640.0f, 480.0f);

    const float step = (tr.x - bl.x) / float(data.size());
    const float range = (tr.y - bl.y) / 32768.0f;

    //glColor(

    for (int i = 1; i < count; ++i)
    {
        const float value0 = data[i - 1];
        const float value1 = data[i - 0];
        const v2 a = bl + v2( step * float(i), range * value0 );
        const v2 b = bl + v2( step * float(i), range * value1 );

        DrawLine(a, b);
    }
}

void GMAudioStream::Subscribe()
{
    try
    {
        _sound_processing->unsubscribe(_subscriber_id);
    }
    catch (const AL::ALError&)
    {
        // ignore, just attempting to avoid hanging subscriptions
    }

    try
    {
        _sound_processing->subscribe(_subscriber_id);
    }
    catch (const AL::ALError&)
    {
    }

    // TODO: does subscribe call init?
    //_sound_processing->init();

    //const int request_interleaving = 1;
    //audioDevice->callVoid(
        //"setClientPreferences",
        //getName(),
        //48000,
        //int(AL::ALLCHANNELS),
        //request_interleaving
    //);


}

void GMAudioStream::GetRemoteAudioData()
{
    const int channels = 4;

    _data.resize(4);

    for (int i = 0; i < channels; ++i)
    {
        _sound_processing->ExtractChannel(i, _data[i]);
        break;
    }
}

void GMAudioStream::GenerateSineWave(int frequency)
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

    GM_MEMFUNC_DECL(GenerateSineWave)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(frequency, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->GenerateSineWave(frequency));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawWaveform)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(channel, 0);
        GM_CHECK_VEC3_PARAM(color, 1);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawWaveform(channel, color, alpha));
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMAudioStream)
GM_REG_MEMFUNC( GMAudioStream, SetActive )
GM_REG_MEMFUNC( GMAudioStream, Update )
GM_REG_MEMFUNC( GMAudioStream, GenerateSineWave )
GM_REG_MEMFUNC( GMAudioStream, DrawWaveform )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMAudioStream);
