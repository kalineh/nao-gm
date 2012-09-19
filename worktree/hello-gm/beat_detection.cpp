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
    , _ip(ip)
    , _port(port)
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

/*
Output:  complex values   
   
   
        which are the Fourier Transform of the input.
BitReverseData(data)
mmax = 1
while (n > mmax)
   istep = 2 * mmax
   theta = 3.14159265358979323846 * b / mmax
   wp = cos(theta) + i * sin(theta)
   w = 1
for (m = 1; m <= mmax; m = m + 1)      for (k = m - 1; k < n; k = k + istep)
         j = k + mmax
temp = w * x[j]
x[j] = x[k] - temp
         x[k] = x[k] + temp
      endfor
   w = w*wp
endfor
mmax = istep
endwhile
ScaleData(data)
*/

#define PI 3.14159265


#include <complex>
#include <vector>

class FFT
{
    public:
        typedef std::complex<double> Complex;
        
        /* Initializes FFT. n must be a power of 2. */
        FFT(int n, bool inverse = false);

        /* Computes Discrete Fourier Transform of given buffer. */
        std::vector<Complex> transform(const std::vector<Complex>& buf);

        static double getIntensity(Complex c);
        static double getPhase(Complex c);
        
    private:
        int n, lgN;
        bool inverse;
        std::vector<Complex> omega;
        std::vector<Complex> result;
        
        void bitReverseCopy(const std::vector<Complex>& src,
                std::vector<Complex>& dest) const;
};

FFT::FFT(int n, bool inverse)
    : n(n)
    , inverse(inverse)
    , result(std::vector<Complex>(n))
{
    lgN = 0;
    for (int i = n; i > 1; i >>= 1)
        ++lgN;

    omega.resize(lgN);

    int m = 1;
    for (int s = 0; s < lgN; ++s)
    {
        m <<= 1;
        if (inverse)
            omega[s] = exp(Complex(0, 2.0 * PI / m));
        else
            omega[s] = exp(Complex(0, -2.0 * PI / m));
    }
}

std::vector<FFT::Complex> FFT::transform(const std::vector<Complex>& buf)
{
    bitReverseCopy(buf, result);
    int m = 1;
    for (int s = 0; s < lgN; ++s)
    {
        m <<= 1;
        for (int k = 0; k < n; k += m)
        {
            Complex current_omega = 1;
            for (int j = 0; j < (m >> 1); ++j)
            {
                Complex t = current_omega * result[k + j + (m >> 1)];
                Complex u = result[k + j];
                result[k + j] = u + t;
                result[k + j + (m >> 1)] = u - t;
                current_omega *= omega[s];
            }
        }
    }
    if (inverse == false)
        for (int i = 0; i < n; ++i)
            result[i] /= n;
    return result;
}

double FFT::getIntensity(Complex c)
{
    return abs(c);
}

double FFT::getPhase(Complex c)
{
    return arg(c);
}

void FFT::bitReverseCopy(const std::vector<Complex>& src, std::vector<Complex>& dest)
        const
{
    for (int i = 0; i < n; ++i)
    {
        int index = i, rev = 0;
        for (int j = 0; j < lgN; ++j)
        {
            rev = (rev << 1) | (index & 1);
            index >>= 1;
        }
        dest[rev] = src[i];
    }
}

void GMAudioStream::CalcBeatFFT(int channel)
{
    const int count = _data[channel].size();

    std::vector<FFT::Complex> values(count);

    for (int i = 0; i < count; ++i)
    {
        values[i] = FFT::Complex(_data[channel][i], 0.0f);
    }

    FFT fft(values.size(), true);

    std::vector<FFT::Complex> results = fft.transform(values);

    _transformed.resize(count);
    for (int i = 0; i < count; ++i)
    {
        _transformed[i] = float(results[i].real());
    }
}

void GMAudioStream::CalcBeatEnergy(int channel)
{
}

void GMAudioStream::DrawWaveform(int channel, v3 color, float alpha)
{
    DrawWaveformImpl(_data[channel], color, alpha);
}

void GMAudioStream::DrawBeatWaveform(int channel, v3 color, float alpha)
{
    DrawWaveformImpl(_transformed, color, alpha);
}

void GMAudioStream::DrawWaveformImpl(const Channel& channel, v3 color, float alpha)
{
    const int count = channel.size();

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = v2(1024.0f, 768.0f);

    const float step = (tr.x - bl.x) / float(channel.size());
    const float range = (tr.y - bl.y) / 32768.0f / 4.0f;

    //glColor(

    for (int i = 1; i < count; ++i)
    {
        const float value0 = channel[i - 1];
        const float value1 = channel[i - 0];
        const v2 a = bl + v2( step * float(i), range * value0 );
        const v2 b = bl + v2( step * float(i), range * value1 );

        DrawLine(a, b);
    }
}

void GMAudioStream::Subscribe()
{
    if ( !_sound_processing )
    {
        AL::ALBroker::Ptr broker = AL::ALBroker::createBroker("main", "0.0.0.0", 54000, _ip, _port);
        _sound_processing = AL::ALModule::createModule<ALSoundProcessing>(broker, "ALSoundProcessing");
    }

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

    _data.resize(channels);

    for (int i = 0; i < channels; ++i)
    {
        _sound_processing->ExtractChannel(i, _data[i]);
    }
}

void GMAudioStream::GenerateSineWave(int frequency)
{
    const int channels = 4;

    _data.resize(channels);

    for (int i = 0; i < channels; ++i)
    {
        const int entries = 1024;

        _data[i].resize(entries);

        for (int j = 0; j < entries; ++j)
        {
            const float t = float(j * frequency);
            const float s = std::sinf(t) * 0.5f + 1.0f;
            const float r = 32768.0f / 4.0f;
            _data[i][j] = s * r;
        }
    }
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

    GM_MEMFUNC_DECL(DrawBeatWaveform)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(channel, 0);
        GM_CHECK_VEC3_PARAM(color, 1);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawBeatWaveform(channel, color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcBeatFFT)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(channel, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcBeatFFT(channel));
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMAudioStream)
GM_REG_MEMFUNC( GMAudioStream, SetActive )
GM_REG_MEMFUNC( GMAudioStream, Update )
GM_REG_MEMFUNC( GMAudioStream, GenerateSineWave )
GM_REG_MEMFUNC( GMAudioStream, CalcBeatFFT )
GM_REG_MEMFUNC( GMAudioStream, DrawWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawBeatWaveform )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMAudioStream);
