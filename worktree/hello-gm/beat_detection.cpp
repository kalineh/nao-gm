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
    {
        results[i] = _buffer[start + i] / USHRT_MAX;
    }
}

GMAudioStream::GMAudioStream(const char* name, const char* ip, int port)
    : _active(false)
    , _name(name)
    , _subscriber_id(name)
    , _ip(ip)
    , _port(port)
    , _rolling_ft_min(0.0f)
    , _rolling_ft_max(0.0000001f)
    , _average_energy_index(0)
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

#define PI 3.14159265f

#include <complex>
#include <vector>

class FFT
{
    public:
        typedef std::complex<float> Complex;
        
        /* Initializes FFT. n must be a power of 2. */
        FFT(int n, bool inverse = false);

        /* Computes Discrete Fourier Transform of given buffer. */
        std::vector<Complex> transform(const std::vector<Complex>& buf);

        static float getIntensity(Complex c);
        static float getPhase(Complex c);
        
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
            omega[s] = exp(Complex(0.0f, 2.0f * PI / m));
        else
            omega[s] = exp(Complex(0.0f, -2.0f * PI / m));
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
            result[i] /= float(n);
    return result;
}

float FFT::getIntensity(Complex c)
{
    return abs(c);
}

float FFT::getPhase(Complex c)
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


// good for verifying results
// http://home.fuse.net/clymer/graphs/fourier.html
//
//
void DFT(int count, float* inreal, float* inimag, float* outreal, float* outimag)
{
    // DFT only evaluates frequencies up to N / 2

    // for each candidate frequency
    //    for each data point
    //        sum sin/cos complex

    const int n = count;

    memset(outreal, 0, sizeof(float) * n);
    memset(outimag, 0, sizeof(float) * n);

    for (int i = 0; i < n / 2; ++i)
    {
        float real = 0.0f;
        float imag = 0.0f;

        for (int j = 0; j < n; ++j)
        {
            const float f = inreal[j];

            real += f * std::sinf(i * j * 2 * PI / n);
            imag += f * std::cosf(i * j * 2 * PI / n);
        }

        outreal[i] = real;
        outimag[i] = imag;
    }
    
    for (int i = 0; i < n / 2; ++i)
    {
        outreal[i] /= n / 2;
        outimag[i] /= n / 2;

        outreal[i] = std::powf( outreal[i] * outreal[i] * outimag[i] * outimag[i], 0.5f );
        outimag[i] = 0.0f;
    }
}

// reference implementation
void DFTref(int count, float* inreal, float* inimag, float* outreal, float* outimag)
{
    int n = count;
    for (int k = 0; k < n; k++)
    {
        float sumreal = 0;
        float sumimag = 0;

        // equalize contribution
        const float divisor = float(k) / float(n);

        for (int t = 0; t < n; t++)
        {
            sumreal +=  inreal[t]*cosf(2*PI * t * k / n) + inimag[t]*sinf(2*PI * t * k / n);
            sumimag += -inreal[t]*sinf(2*PI * t * k / n) + inimag[t]*cosf(2*PI * t * k / n);
            //sumreal +=  inreal[t]*cosf(2*PI * t * k / n) + inimag[t]*sinf(2*PI * t * k / n);
            //sumimag += -inreal[t]*sinf(2*PI * t * k / n) + inimag[t]*cosf(2*PI * t * k / n);
        }

        outreal[k] = sumreal;
        outimag[k] = sumimag;
    }
}

#define FFT_ENTRIES 256

void GMAudioStream::CalcBeatDFT(int channel)
{
    int count = FFT_ENTRIES;

    if (_data.empty() || _data[channel].empty())
        return;

    std::vector<float> ir(count);
    std::vector<float> ii(count);
    std::vector<float> or(count);
    std::vector<float> oi(count);

    for (int i = 0; i < count; ++i)
    {
        ir[i] = _data[channel][i];
        ii[i] = 0.0f;
    }

    DFT(count, &ir[0], &ii[0], &or[0], &oi[0]);

    _transformed.resize(count);

    for (int i = 0; i < count; ++i)
    {
        _transformed[i] = std::complex<float>(or[i], oi[i]);
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
        _transformed[i] = std::complex<float>(results[i].real(), results[i].imag());
    }
}

void GMAudioStream::CalcAverageEnergies()
{
    // TODO: improve timing, match mic data
    const int FrameCount = 60;

    // push latest energy values

    _energy_history.resize(FrameCount);

    std::vector<float>& data = _energy_history[_average_energy_index];

    data.resize(_transformed.size());

    for (int i = 0; i < int(_transformed.size()); ++i)
    {
        data[i] = _transformed[i].real();
    }

    _average_energy_index += 1;
    _average_energy_index %= FrameCount;

    // calc average

    _average_energy.clear();
    _average_energy.resize(_transformed.size());

    for (int i = 0; i < FrameCount; ++i)
    {
        const int count = _energy_history[i].size();
        for (int j = 0; j < count; ++j)
        {
            _average_energy[j] += _energy_history[i][j];
        }
    }

    for (int i = 0; i < int(_average_energy.size()); ++i)
    {
        _average_energy[i] /= float(FrameCount);
    }

    // calc instant energy difference

    _energy_differences.resize(_transformed.size());

    for (int i = 0; i < int(data.size()); ++i)
    {
        const float d = data[i];
        const float a = _average_energy[i];

        _energy_differences[i] = std::abs(d - a);
    }
}

void GMAudioStream::DrawWaveform(int channel, v3 color, float alpha)
{
    DrawWaveformImpl(_data[channel], 1.0f, color, alpha);
}

void GMAudioStream::DrawBeatWaveform(int channel, v3 color, float alpha)
{
    std::vector<float> phases(_transformed.size());
    for (int i = 0; i < (int)phases.size(); ++i)
        phases[i] = _transformed[i].real();

    // normalize

    float min = 0.0f;
    float max = 0.0f;

    for (int i = 0; i < (int)phases.size(); ++i)
    {
        min = std::min<float>(phases[i], min);
        max = std::max<float>(phases[i], max);
    }

    //_rolling_ft_min = std::min<float>(_rolling_ft_min, min);
    //_rolling_ft_max = std::max<float>(_rolling_ft_max, max);

    //min = _rolling_ft_min;
    //max = _rolling_ft_max;

    const float range = max - min;
    for (int i = 0; i < (int)phases.size(); ++i)
    {
        phases[i] += min;
        phases[i] /= range;
    }

    _last_phase_min = min;
    _last_phase_max = max;

    DrawWaveformImpl(phases, 1.0f, color, alpha);
}

void GMAudioStream::DrawEnergyDifferenceWaveform(v3 color, float alpha)
{
    for (int i = 0; i < int(_average_energy.size()); ++i)
    {
        _average_energy[i] *= 10000000.0f;
        _average_energy[i] += 0.1f;
        _energy_differences[i] *= 10000000.0f;
        _energy_differences[i] += 0.1f;
    }
    //const float scale = 1.0f / (_last_phase_max - _last_phase_min); 
    //const float scale = 10000000000000.0f;
    const float scale = 1.0f;
    DrawWaveformImpl(_average_energy, scale, color, alpha);
    DrawWaveformImpl(_energy_differences, scale, v3(1.0f,1.0f,0.0f), alpha);
}

void GMAudioStream::DrawWaveformImpl(const Channel& channel, float scale, v3 color, float alpha)
{
    const int count = channel.size();

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = v2(1024.0f, 768.0f);

    const float step = (tr.x - bl.x) / float(channel.size());
    const float range = (tr.y - bl.y) / scale;

	glColor4f( color.x, color.y, color.z, alpha );

    for (int i = 1; i < count; ++i)
    {
        const float value0 = channel[i - 1];
        const float value1 = channel[i - 0];
        const v2 a = bl + v2( step * float(i - 1), range * value0 );
        const v2 b = bl + v2( step * float(i - 0), range * value1 );

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

void GMAudioStream::ClearSineWave()
{
    _data.clear();
}

void GMAudioStream::AddSineWave(int frequency)
{
    const int channels = 4;

    _data.resize(channels);

    for (int i = 0; i < channels; ++i)
    {
        const int entries = FFT_ENTRIES;

        _data[i].resize(entries);

        for (int j = 0; j < entries; ++j)
        {
            const float d = float(j) / float(entries) * PI * 2.0f;
            const float t = float(d * frequency);
            const float s = std::sinf(t) * 0.5f + 0.5f;
            _data[i][j] += s;
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

    GM_MEMFUNC_DECL(ClearSineWave)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->ClearSineWave());
        return GM_OK;
    }

    GM_MEMFUNC_DECL(AddSineWave)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(frequency, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->AddSineWave(frequency));
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

    GM_MEMFUNC_DECL(DrawEnergyDifferenceWaveform)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_VEC3_PARAM(color, 0);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawEnergyDifferenceWaveform(color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcBeatDFT)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(channel, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcBeatDFT(channel));
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

    GM_MEMFUNC_DECL(CalcAverageEnergies)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcAverageEnergies());
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMAudioStream)
GM_REG_MEMFUNC( GMAudioStream, SetActive )
GM_REG_MEMFUNC( GMAudioStream, Update )
GM_REG_MEMFUNC( GMAudioStream, ClearSineWave )
GM_REG_MEMFUNC( GMAudioStream, AddSineWave )
GM_REG_MEMFUNC( GMAudioStream, CalcBeatDFT )
GM_REG_MEMFUNC( GMAudioStream, CalcBeatFFT )
GM_REG_MEMFUNC( GMAudioStream, CalcAverageEnergies )
GM_REG_MEMFUNC( GMAudioStream, DrawWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawBeatWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawEnergyDifferenceWaveform )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMAudioStream);
