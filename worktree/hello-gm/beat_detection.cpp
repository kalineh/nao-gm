//
// beat_detection.cpp
//

#include "beat_detection.h"

#include <gfx/DrawPrimitives.h>

// dsp guide
// http://www.dspguide.com/ch8/2.htm

// beat detection notes
// http://www.flipcode.com/misc/BeatDetectionAlgorithms.pdf


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
    , _average_index(0)
    , _fft_window_size(256)
{
}

GMAudioStream::~GMAudioStream()
{
    if (_recorder)
    {
        _recorder->RecordEnd();
    }
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
    const int MaxChannels = 4;

    // intended to be a noop when already sufficiently sized

    _channels.resize(MaxChannels);

    for (int i = 0; i < (int)_channels.size(); ++i)
    {
        _channels[i].raw.resize(_fft_window_size);
        _channels[i].fft.resize(_fft_window_size);
    }
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

void GMAudioStream::CalcDFT(int channel)
{
    Channel& c = _channels[channel];

    CHECK((int)c.raw.size() >= _fft_window_size);
    CHECK((int)c.fft.size() >= _fft_window_size);

    std::vector<float> ir(_fft_window_size);
    std::vector<float> ii(_fft_window_size);
    std::vector<float> or(_fft_window_size);
    std::vector<float> oi(_fft_window_size);

    for (int i = 0; i < _fft_window_size; ++i)
    {
        ir[i] = c.raw[i];
        ii[i] = 0.0f;
    }

    DFT(_fft_window_size, &ir[0], &ii[0], &or[0], &oi[0]);

    for (int i = 0; i < _fft_window_size; ++i)
    {
        c.fft[i] = std::complex<float>(or[i], oi[i]);
    }
}

void GMAudioStream::CalcFFT(int channel)
{
    Channel& c = _channels[channel];

    CHECK((int)c.raw.size() >= _fft_window_size);
    CHECK((int)c.fft.size() >= _fft_window_size);

    for (int i = 0; i < _fft_window_size; ++i)
    {
        c.fft[i] = std::complex<float>(c.raw[i], 0.0f);
    }

    FFT fft(_fft_window_size, true);

    std::vector<std::complex<float> > results = fft.transform(c.fft);

    for (int i = 0; i < _fft_window_size; ++i)
    {
        c.fft[i] = results[i];
    }
}

void GMAudioStream::CalcAverageAndDifference(int channel)
{
    // TODO: split history pushback to seperate function?

    // TODO: improve timing, match mic data
    const int FrameCount = 60;

    Channel& c = _channels[channel];

    // store history frame

    std::vector<float>& history = _history_fft[_average_index];

    history.resize(_fft_window_size);

    for (int i = 0; i < (int)c.fft.size(); ++i)
    {
        const std::complex<float> fft = c.fft[i];
        const float f = fft.real() * fft.real() + fft.imag() * fft.imag();
        history[i] = f;
    }

    // accumulate history

    _average_fft.resize(_fft_window_size);

    for (int i = 0; i < FrameCount; ++i)
    {
        const int count = std::min<int>(_history_fft[i].size(), _fft_window_size);
        for (int j = 0; j < count; ++j)
        {
            _average_fft[j] += _history_fft[i][j];
        }
    }

    // calculate average over history

    for (int i = 0; i < _fft_window_size; ++i)
    {
        _average_fft[i] /= float(FrameCount);
    }

    // calc instant energy difference

    _difference_fft.resize(_fft_window_size);

    for (int i = 0; i < _fft_window_size; ++i)
    {
        const float e = _history_fft[_average_index][i];
        const float a = _average_fft[i];

        _difference_fft[i] = std::powf(std::abs(e - a), 2.0f);
    }

    _average_index += 1;
    _average_index %= FrameCount;
}

void GMAudioStream::DrawRawWaveform(int channel, v3 color, float alpha)
{
    DrawWaveform(_channels[channel].raw, 1.0f, color, alpha);
}

void GMAudioStream::DrawFFTWaveform(int channel, v3 color, float alpha)
{
    const float scale = 1.0f / float(_fft_window_size);

    // TODO: is this the correct max?

    DrawWaveform(_channels[channel].fft, scale, color, alpha);
}

void GMAudioStream::DrawAverageWaveform(v3 color, float alpha)
{
    DrawWaveform(_average_fft, 1.0f, color, alpha);
}

void GMAudioStream::DrawDifferenceWaveform(v3 color, float alpha)
{
    DrawWaveform(_difference_fft, 1.0f, color * 0.3f, alpha);
}

void GMAudioStream::DrawWaveform(const std::vector<float>& data, float scale, v3 color, float alpha)
{
    const int count = data.size();

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = v2(1024.0f, 768.0f);

    const float step = (tr.x - bl.x) / float(data.size());
    const float range = (tr.y - bl.y) / scale;

	glColor4f( color.x, color.y, color.z, alpha );

    for (int i = 1; i < count; ++i)
    {
        const float value0 = data[i - 1];
        const float value1 = data[i - 0];
        const v2 a = bl + v2( step * float(i - 1), range * value0 );
        const v2 b = bl + v2( step * float(i - 0), range * value1 );

        DrawLine(a, b);
    }
}

void GMAudioStream::DrawWaveform(const std::vector<std::complex<float> >& data, float scale, v3 color, float alpha)
{
    static std::vector<float> converted;

    converted.resize(data.size());

    for (int i = 0; i < (int)data.size(); ++i)
    {
        const std::complex<float> c = data[i];
        const float f = c.real() * c.real() + c.imag() * c.imag();
        converted[i] = f;
    }

    DrawWaveform(converted, scale, color, alpha);
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

void GMAudioStream::ClearInputData()
{
    for (int i = 0; i < (int)_channels.size(); ++i)
    {
        _channels[i].raw.clear();
        _channels[i].fft.clear();
    }
}

void GMAudioStream::SetFFTWindowSize(int samples)
{
    _fft_window_size = samples;
}
void GMAudioStream::AddInputDataRemoteNao()
{
    if (!_active)
        return;

    const int NaoChannels = 4;

    CHECK(_channels.size() >= NaoChannels);

    for (int i = 0; i < NaoChannels; ++i)
    {
        _sound_processing->ExtractChannel(i, _channels[i].raw);
    }
}

void GMAudioStream::AddInputDataMicrophone()
{
    typedef signed char MicrophoneDataType;
    const int MicrophoneDataTypeMaxValue = 255;
    const FMOD_SOUND_FORMAT format = FMOD_SOUND_FORMAT_PCM8;
    const int bytesize = sizeof(MicrophoneDataType);
    const int _frequency = 44100;

    if (!_recorder)
    {
        _recorder = new MicrophoneRecorder(_frequency, 1, format);
        _recorder->RecordStart();
    }

    _recorder->Update();

    const int samples_size = _recorder->GetDataSize(0);
    const unsigned char* samples_data = (unsigned char*)_recorder->GetData(0);
    if (samples_size > 0)
    {
        // add new samples
        const int start = _microphone_buffer.size();
        _microphone_buffer.resize(_microphone_buffer.size() + samples_size);
        memcpy(&_microphone_buffer[0] + start, samples_data, samples_size);

        // shift data down to start
        // TODO: replace with wrapping indices
        const int one_second_data = _frequency * bytesize;
        if ((int)_microphone_buffer.size() > one_second_data)
        {
            const int shift = _microphone_buffer.size() - one_second_data;
            memcpy(&_microphone_buffer[0], &_microphone_buffer[0] + shift, one_second_data);
        }

        _microphone_buffer.resize(std::min<int>(one_second_data, _microphone_buffer.size()));
    }

    // init empty values

    _channels[0].raw.clear();
    _channels[0].raw.resize(_fft_window_size, 0.0f);

    // collect the last _fft_window_size of data
    // if the fft window is too large to ever be filled by mic data it will never be processed

    if ((int)_microphone_buffer.size() >= _fft_window_size)
    {
        for (int i = 0; i < _fft_window_size; ++i)
        {
            const int index = _microphone_buffer.size() - i - 1;
            const signed char raw = (signed char)_microphone_buffer[index];
            const float value = float(raw + MicrophoneDataTypeMaxValue / 2) / float(MicrophoneDataTypeMaxValue);

            _channels[0].raw[i] = value;
        }
    }
}

void GMAudioStream::AddInputDataSineWave(int frequency, float amplitude)
{
    const int channels = 4;

    _channels.resize(channels);

    for (int i = 0; i < channels; ++i)
    {
        const int entries = _fft_window_size;

        _channels[i].raw.resize(entries);

        for (int j = 0; j < entries; ++j)
        {
            const float d = float(j) / float(entries) * PI * 2.0f;
            const float t = float(d * frequency);
            const float s = std::sinf(t) * 0.5f * amplitude;
            _channels[i].raw[j] += s;
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

    GM_MEMFUNC_DECL(ClearInputData)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->ClearInputData());
        return GM_OK;
    }

    GM_MEMFUNC_DECL(AddInputDataSineWave)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_INT_PARAM(frequency, 0);
        GM_CHECK_FLOAT_PARAM(amplitude, 1);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->AddInputDataSineWave(frequency, amplitude));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(AddInputDataMicrophone)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->AddInputDataMicrophone());
        return GM_OK;
    }

    GM_MEMFUNC_DECL(AddInputDataRemoteNao)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->AddInputDataRemoteNao());
        return GM_OK;
    }

    GM_MEMFUNC_DECL(SetFFTWindowSize)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(samples, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->SetFFTWindowSize(samples));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawRawWaveform)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(channel, 0);
        GM_CHECK_VEC3_PARAM(color, 1);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawRawWaveform(channel, color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawFFTWaveform)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(channel, 0);
        GM_CHECK_VEC3_PARAM(color, 1);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawFFTWaveform(channel, color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawAverageWaveform)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_VEC3_PARAM(color, 0);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawAverageWaveform(color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawDifferenceWaveform)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_VEC3_PARAM(color, 0);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawDifferenceWaveform(color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcDFT)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(channel, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcDFT(channel));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcFFT)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(channel, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcFFT(channel));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcAverageAndDifference)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(channel, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcAverageAndDifference(channel));
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMAudioStream)
GM_REG_MEMFUNC( GMAudioStream, SetActive )
GM_REG_MEMFUNC( GMAudioStream, Update )
GM_REG_MEMFUNC( GMAudioStream, ClearInputData )
GM_REG_MEMFUNC( GMAudioStream, AddInputDataSineWave )
GM_REG_MEMFUNC( GMAudioStream, AddInputDataMicrophone )
GM_REG_MEMFUNC( GMAudioStream, AddInputDataRemoteNao )
GM_REG_MEMFUNC( GMAudioStream, SetFFTWindowSize )
GM_REG_MEMFUNC( GMAudioStream, CalcDFT )
GM_REG_MEMFUNC( GMAudioStream, CalcFFT )
GM_REG_MEMFUNC( GMAudioStream, CalcAverageAndDifference )
GM_REG_MEMFUNC( GMAudioStream, DrawRawWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawFFTWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawAverageWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawDifferenceWaveform )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMAudioStream);
