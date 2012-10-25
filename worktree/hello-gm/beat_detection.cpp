//
// beat_detection.cpp
//

#include "beat_detection.h"

#include <gfx/DrawPrimitives.h>

// dsp guide
// http://www.dspguide.com/ch8/2.htm

// beat detection notes
// http://www.flipcode.com/misc/BeatDetectionAlgorithms.pdf

#define PI 3.14159265f

#include <complex>
#include <vector>
#include <time.h>

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
void DFT4_old(int count, float* inreal, float* inimag, float* outreal, float* outimag)
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

            real += f * std::cosf(i * j * 2 * PI / n);
            imag += f * std::sinf(i * j * 2 * PI / n);
        }

        outreal[i] = real;
        outimag[i] = imag;
    }
}

void Hanning(int count, float* input, float* output)
{
    // TODO: magical optimized version
    for (int i = 0; i < count; ++i)
    {
        output[i] = input[i] * 0.5f * (1.0f - std::cosf(2.0f * PI * float(i) / float(count)));
    }
}

void DFT3(int count, float* input, float* out, float magnify_scale, float magnify_power)
{
    // using a hann window cleans up our result a lot, much less random faff
    Hanning(count, input, input);

    // discrete fourier transform, but we are not normalizing yet
    // F[bin] = 1/N * (f[n] * e ^ i * 2pi * bin * n/N), for each n
    // this code is:
    // F[bin] = (f[n] * e ^ i * 2pi * bin * n/N), for each n

    static std::vector<std::complex<float> > cfft;

    cfft.clear();
    cfft.resize(count);

    memset(out, 0, sizeof(out[0]) * count);

    const int nyquist = count / 2;

    for (int bin = 0; bin < nyquist; ++bin)
    {
        const float a = 2.0f * PI * float(bin) / float(count);
        
        float real = 0.0f;
        float imag = 0.0f;

        for (int t = 0; t < count; ++t)
        {
            const float f = input[t];

            real += f * std::cosf(a * t);
            imag += f * std::sinf(a * t);
        }

        cfft[bin] = std::complex<float>(real, imag);
    }

    for (int bin = 0; bin < nyquist; ++bin)
    {
        const std::complex<float> c = cfft[bin];

        const float f0 = std::sqrtf(c.real() * c.real() + c.imag() * c.imag());
        const float f1 = f0 * magnify_scale;
        const float f2 = std::powf(f1, magnify_power);
        const float f3 = std::logf(f2);
        const float f4 = std::max<float>(0.000000001f, f3);

        out[bin] = f4;
    }
}

/*
void DFT3_test(int count, float* input, float* out)
{
    const float _fft_magnify_scale = 1.0f;
    const float _fft_magnify_power = 1.0f;

    // using a hann window cleans up our result a lot, much less random faff
    Hanning(count, input, input);

    // discrete fourier transform, but we are not normalizing yet
    // F[bin] = 1/N * (f[n] * e ^ i * 2pi * bin * n/N), for each n
    // this code is:
    // F[bin] = (f[n] * e ^ i * 2pi * bin * n/N), for each n

    static std::vector<float> in;
    static std::vector<std::complex<float> > cfft;
    static std::vector<float> rfft;

    in.clear();
    in.resize(count);

    cfft.clear();
    cfft.resize(count);

    rfft.clear();
    rfft.resize(count);

    memset(out, 0, sizeof(out[0]) * count);

    const int nyquist = count / 2;

    for (int i = 0; i < count; ++i)
    {
        in[i] = input[i];
    }

    for (int i = 0; i < count; ++i)
    {
        const float a = 2.0f * PI * float(i) / float(count);
        
        float real = 0.0f;
        float imag = 0.0f;

        for (int t = 0; t < count; ++t)
        {
            const float f = in[t];

            real += f * std::cosf(a * t);
            imag += f * std::sinf(a * t);
        }

        cfft[i] = std::complex<float>(real, imag);
    }

    // compute power

    for (int i = 0; i < count; ++i)
    {
        const std::complex<float> c = cfft[i];
        in[i] = c.real() * c.real() + c.imag() * c.imag();
    }

    // take cube root

    for (int i = 0; i < count; ++i)
    {
        in[i] = std::powf(in[i], 1.0f / 3.0f);
    }

    // fft again

    for (int i = 0; i < count; ++i)
    {
        const float a = 2.0f * PI * float(i) / float(count);
        
        float real = 0.0f;
        float imag = 0.0f;

        for (int t = 0; t < count; ++t)
        {
            const float f = in[t];

            real += f * std::cosf(a * t);
            imag += f * std::sinf(a * t);
        }

        cfft[i] = std::complex<float>(real, imag);
    }

    // take real part

    for (int i = 0; i < nyquist; ++i)
    {
        rfft[i] = cfft[i].real();
    }

    // clip at zero

    for (int i = 0; i < nyquist; ++i)
    {
        rfft[i] = std::max<float>(rfft[i], 0.0f);
        out[i] = rfft[i];
    }

    // subtract time-doubled signal

    for (int bin = 0; bin < nyquist; ++bin)
    {
        if (bin % 2 == 0)
            rfft[bin] -= out[bin / 2];
        else
            rfft[bin] -= ((out[bin / 2] + out[bin / 2 + 1]) / 2);
    }

    // clip at zero again

    for (int bin = 0; bin < nyquist; ++bin)
    {
        rfft[bin] = std::max<float>(rfft[bin], 0.0f);
        out[bin] = rfft[bin];
    }

}
*/

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
    , _frequency(44100 / 1)
    , _framerate(60)
    , _frame(0)
    , _microphone_samples_read(0)
    , _clock_start(0)
    , _fft_magnify_scale(60.0f)
    , _fft_magnify_power(1.0f)
{
    _clock_timer.Start();
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

void GMAudioStream::SetFrequency(int frequency)
{
    _frequency = frequency;
}

void GMAudioStream::SetFrameRate(int framerate)
{
    _framerate = framerate;
}

int GMAudioStream::GetFFTWindowSize()
{
    // ensure frequency divides evenly into framerate
    //CHECK(((float(_frequency) / float(_framerate)) - float(_frequency / _framerate)) == 0.0f);

    return _frequency / _framerate;
}

void GMAudioStream::Update()
{
    const int MaxChannels = 4;
    const int W = GetFFTWindowSize();

    // intended to be a noop when already sufficiently sized

    _channels.resize(MaxChannels);

    for (int i = 0; i < (int)_channels.size(); ++i)
    {
        _channels[i].raw.resize(W);
        _channels[i].fft.resize(W);
    }

    _history_fft.resize(_framerate);
    _history_difference_fft.resize(_framerate);

    _average_index += 1;
    _average_index %= _framerate;

    //printf("Time: %.2f, %.2f, %.2f\n", GetSecondsByFrame(), GetSecondsByMicrophone(), GetSecondsBySystemClock());

    _synthesizer.resize(_frequency);
}

void GMAudioStream::CalcFrameDFT(int channel)
{
    const int W = GetFFTWindowSize();

    Channel& c = _channels[channel];

    CHECK((int)c.raw.size() >= W);
    CHECK((int)c.fft.size() >= W);

    DFT3(W, &c.raw[0], &c.fft[0], _fft_magnify_scale, _fft_magnify_power);
}

void GMAudioStream::CalcFrameFFT(int channel)
{
    const int W = GetFFTWindowSize();

    Channel& c = _channels[channel];

    CHECK((int)c.raw.size() >= W);
    CHECK((int)c.fft.size() >= W);

    //FFT fft(W, true);
    //std::vector<std::complex<float> > results = fft.transform(c.fft);
    //for (int i = 0; i < W; ++i)
        //c.fft[i] = results[i];
}

void GMAudioStream::CalcFrameAverageAndDifference(int channel)
{
    const int W = GetFFTWindowSize();

    // TODO: split history pushback to seperate function?

    Channel& c = _channels[channel];

    // store history frame

    std::vector<float>& history = _history_fft[_average_index];

    history.resize(W);

    for (int i = 0; i < (int)c.fft.size(); ++i)
    {
        history[i] = c.fft[i];
    }

    // accumulate history

    _average_fft.resize(W);

    for (int i = 0; i < _framerate; ++i)
    {
        const int count = std::min<int>(_history_fft[i].size(), W);
        for (int j = 0; j < count; ++j)
        {
            _average_fft[j] += _history_fft[i][j];
        }
    }

    // calculate average over history

    for (int i = 0; i < W; ++i)
    {
        _average_fft[i] /= float(_framerate);
    }

    // calc instant energy difference

    _difference_fft.resize(W);

    for (int i = 0; i < W; ++i)
    {
        const float e = _history_fft[_average_index][i];
        const float a = _average_fft[i];

        _difference_fft[i] = std::max<float>(e - a, 0.0f);
    }

    // keep history of difference

    _history_difference_fft[_average_index].resize(W);

    for (int i = 0; i < W; ++i)
    {
        _history_difference_fft[_average_index][i] = _difference_fft[i];
    }
}

int GMAudioStream::CalcEstimatedBeatsPerSecond(int channel, int bin, float threshold)
{
    return CalcEstimatedBeatsPerSecondAverage(channel, bin, threshold);
}

int GMAudioStream::CalcEstimatedBeatsPerSecondDiscrete(int channel, int bin, float threshold)
{
    // wrong because adjacent difference values are detected as beats

    std::vector<int> beat_indices;

    for (int i = 0; i < _framerate; ++i)
    {
        const int average_index = (i + _average_index) % _framerate;
        std::vector<float>& fft = _history_difference_fft[average_index];

        // no history data yet
        if (bin > (int)fft.size())
            continue;

        const float power = fft[bin];
        if (power < threshold)
            continue;

        beat_indices.push_back(i);
    }

    // need at least 3 beats
    if (beat_indices.size() < 3)
        return 0;

    int difference = beat_indices[0] - beat_indices[1];

    int beats = 2;

    for (int i = 2; i < (int)beat_indices.size(); ++i)
    {
        const int prev_beat_index = beat_indices[i];
        const int next_beat_index = beat_indices[i];
        const int next_difference = next_beat_index - prev_beat_index;

        const int variance = std::abs(next_difference - difference);
        if (variance < 3)
            beats++;
    }

    return beats;
}

int GMAudioStream::CalcEstimatedBeatsPerSecondAverage(int channel, int bin, float threshold)
{
    // for each item in the list we calc the beats
    // beats are any frame which threshold is over N

    const int W = GetFFTWindowSize();

    int beats = 0;

    for (int i = 1; i < _framerate; ++i)
    {
        const int average_index = (i + _average_index) % _framerate;
        std::vector<float>& fft = _history_difference_fft[average_index];

        // no history data yet
        if (bin > (int)fft.size())
            continue;

        const int previous_index = (i + _average_index - 1) % _framerate;
        std::vector<float>& prevfft = _history_difference_fft[previous_index];

        if (bin > (int)prevfft.size())
            continue;

        const float power = fft[bin];
        if (power < threshold)
            continue;

        // already held down
        if (prevfft[bin] > threshold)
            continue;

        beats += 1;
    }

    //printf("beats: %d: %d\n", bin, beats);
    return beats;
}

float FrequencyToPitch(float hz)
{
   return 69.0f + 12.0f * (std::logf(hz / 440.0f) / std::logf(2.0f));
}

float PitchToFrequency(float pitch)
{
  return 440.0f * std::powf(2.0f, (pitch - 69.0f) / 12.0f);
}

char gPitchName[10];

const char* GetNoteName(int pitch)
{
    const char* notes[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return notes[pitch % 12];
}

int GetNoteOctave(int pitch)
{
    return (pitch + 3) / 12 - 2;
}

float CubicMaximize(float y0, float y1, float y2, float y3, float *maxyVal)
{
   // Find coefficients of cubic
   float a, b, c, d;

   a = y0 / -6.0 + y1 / 2.0 - y2 / 2.0 + y3 / 6.0;
   b = y0 - 5.0 * y1 / 2.0 + 2.0 * y2 - y3 / 2.0;
   c = -11.0 * y0 / 6.0 + 3.0 * y1 - 3.0 * y2 / 2.0 + y3 / 3.0;
   d = y0;

   // Take derivative
   float da, db, dc;

   da = 3 * a;
   db = 2 * b;
   dc = c;

   // Find zeroes of derivative using quadratic equation
   float discriminant = db * db - 4 * da * dc;
   if (discriminant < 0.0)
      return -1.0;              // error

   float x1 = (-db + sqrt(discriminant)) / (2 * da);
   float x2 = (-db - sqrt(discriminant)) / (2 * da);

   // The one which corresponds to a local _maximum_ in the
   // cubic is the one we want - the one with a negative
   // second derivative
   float dda = 2 * da;
   float ddb = db;

#define CUBIC(x,a,b,c,d)  (a*x*x*x + b*x*x + c*x + d)

   if (dda * x1 + ddb < 0) {
     *maxyVal = CUBIC(x1,a,b,c,d);
     return x1;
   }
   else {
     *maxyVal = CUBIC(x2,a,b,c,d);
     return x2;
   }

#undef CUBIC
}

class NoteBrain
{
public:
    NoteBrain();

    void Update(float forget);
    void AddNote(int note, float confidence);

    void EstimateScale();
    void GenerateMelody();

    void DrawNoteConfidences(v3 color, float alpha);

private:
    float EstimateScaleConfidence(int start_note, const int* offsets);

    // 0-83
    // A0 to G#6
    std::vector<float> _notes;
};

NoteBrain::NoteBrain()
{
    _notes.resize(84);
}

void NoteBrain::Update(float forget)
{
    for (int i = 0; i < _notes.size(); ++i)
    {
        _notes[i] = std::max<float>(_notes[i] - forget, 0.0f);
    }
}

void NoteBrain::AddNote(int note, float confidence)
{
    if (note < 0)
        return;

    if (note >= _notes.size())
        return;

    _notes[note] += confidence;
}

void NoteBrain::EstimateScale()
{
    const int ScaleTypes = 2;
    const int ScaleNotes = 7;
    const int OctaveNotes = 12;

    static const int scale_offsets[ScaleTypes][ScaleNotes] = {
        { 0, 2, 4, 5, 7, 9, 11 }, // major
        { 0, 2, 3, 5, 7, 8, 10 }, // minor
        // others
    };

    // A, A#, B, C, C#, D, D#, E, F, F#, G, G#
    float scale_confidence[ScaleTypes][OctaveNotes] = { 0.0f };

    // evaluate all notes for all known scales

    for (int s = 0; s < ScaleTypes; ++s)
    {
        for (int n = 0; n < OctaveNotes; ++n)
        {
            const int* scale = scale_offsets[s];
            const float confidence = EstimateScaleConfidence(n, scale);
            scale_confidence[s][n] = confidence;
        }
    }

    // find best match

    float best_confidence = 0.0f;
    int best_scale = -1;
    int best_note = -1;

    for (int s = 0; s < ScaleTypes; ++s)
    {
        for (int n = 0; n < OctaveNotes; ++n)
        {
            const float confidence = scale_confidence[s][n];

            if (confidence > best_confidence)
            {
                best_confidence = confidence;
                best_scale = s;
                best_note = n;
            }
        }
    }

    if (best_note >= 0)
    {
        const char* scale_names[] = { "major", "minor" };
        const char* note_names[] = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", };

        printf("scale: %s %s\n", note_names[best_note], scale_names[best_scale]);
    }
}

float NoteBrain::EstimateScaleConfidence(int start_note, const int* offsets)
{
    // NOTE: we scan 1 less octave than we store, because the start_note will walk up to one octave extra
    const int Octaves = 6;
    const int OctaveNotes = 12;
    const int ScaleNotes = 7;

    // A, A#, B, C, C#, D, D#, E, F, F#, G, G#
    float result = 0.0f;

    for (int i = 0; i < Octaves; ++i)
    {
        for (int n = 0; n < ScaleNotes; ++n)
        {
            const int index = start_note + i * OctaveNotes + offsets[n];
            const float value = _notes[index];

            result += value;
        }
    }

    return result;
}

void GMAudioStream::CalcFramePitches(float threshold)
{
    // we can simply scan for each frequency we want to test
    // if we take a gaussian interpolate of the bins we can estimate the note

    const int W = GetFFTWindowSize();

    static NoteBrain nb;

    // 5 octaves
    //_pitch.clear();
    _pitch.resize(87);

    for (int i = 0; i < _pitch.size(); ++i)
    {
        _pitch[i] = std::max<float>(_pitch[i] - 0.15f, 0.0f);
    }

    // either can use instantaneous or averages
    std::vector<float>& src = _channels[0].fft;
    //std::vector<float>& src = _average_fft;

    bool was_rising = src[1] > src[0];
    for (int bin = 2; bin < W; ++bin)
    {
        bool now_rising = src[bin] > src[bin - 1];
        if (now_rising && !was_rising)
        {
            float y = 0.0f;

            const int left = bin - 2;
            const float cubic = CubicMaximize(src[left], src[left+1], src[left+2], src[left+3], &y);
            const float maximum = float(left) + cubic;

            if (y != y)
                continue;

            if (y < threshold)
                continue;

            if (maximum < 0.0f)
                continue;

            const float hz = maximum * _frequency / float(W);

            const int pitch = int(FrequencyToPitch(hz) + 0.1f) - 20 - 1; // TODO: why is it 20 off? (-1 for 0-based index of notes)

            // TODO: we can get negative pitch somehow here, figure out why
            //       likely some artifact of the cubic maximize
            if (pitch < 0)
                continue;

            // ignoring out-of-range notes
            if (pitch >= 88)
                continue;

            //write += sprintf(write, "%s%d ", GetNoteName(pitch), GetNoteOctave(pitch));
            //write += sprintf(write, "%.2fhz (%.2f)", pitch, y);
            //TimePrint("%d ", pitch);

            _pitch[pitch] += 1;

            nb.AddNote(pitch, y);
        }

        was_rising = now_rising;
    }

    nb.EstimateScale();
}

void GMAudioStream::NoteTuner(float threshold)
{
    const int W = GetFFTWindowSize();

    // hz of bin B is (Frequency / Bins) * B
    // ie. bin 7 is (44100 / (44100 / 60)) * 7 = 44100 / 735 * 7 = 420hz
    // ie. bin 8 is (44100 / (44100 / 60)) * 8 = 44100 / 735 * 8 = 480hz
    // a piano A4 is 440hz, so will have data in both these bins

    // sub-bin accuracy is needed for determining clean note tones accurately
    // currently uses cubic maximum to estimate
    // can probably test weighted averaging based on bin magnitude

    // either can use instantaneous or averages
    std::vector<float>& src = _channels[0].fft;
    //std::vector<float>& src = _average_fft;

    // 49 - 60 inclusive
    const char* piano_notes[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    char buffer[1024] = { 0 };
    char* write = buffer;

    bool was_rising = src[1] > src[0];

    for (int bin = 2; bin < W; ++bin)
    {
        bool now_rising = src[bin] > src[bin - 1];

        if (now_rising && !was_rising)
        {
            float y = 0.0f;

            const int left = bin - 2;
            const float cubic = CubicMaximize(src[left], src[left+1], src[left+2], src[left+3], &y);
            const float maximum = float(left) + cubic;

            if (y != y)
                continue;

            if (y < threshold)
                continue;

            if (maximum < 0.0f)
                continue;

            const float hz = maximum * _frequency / float(W);

            const int pitch = int(FrequencyToPitch(hz) + 0.5f);

            // TODO: we can get negative pitch somehow here, figure out why
            //       likely some artifact of the cubic maximize
            if (pitch < 0)
                continue;

            const char* note = piano_notes[pitch % 12];
            write += sprintf(write, "%s%d ", GetNoteName(pitch), GetNoteOctave(pitch));
            //write += sprintf(write, "%.2fhz (%.2f)", pitch, y);
        }

        was_rising = now_rising;
    }

    if (write != buffer)
        TimePrint("best note: %s", buffer);
}

void GMAudioStream::TimePrint(const char* format, ...)
{
    char buffer[4096] = { 0 };
	va_list args;
	va_start(args, format);
	vsnprintf_s(buffer, 4096, 4096, format, args);
	va_end (args);

    const int W = GetFFTWindowSize();
    const float seconds_per_frame = float(W) / float(_frequency);
    const float time = float(_frame) * seconds_per_frame;

    printf("%.2f: %s\n", time, buffer);
}

int GMAudioStream::TestGetPianoNotes(float threshold, std::vector<int>& test_notes)
{
    const int W = GetFFTWindowSize();
        
    std::vector<float>& src = _difference_fft;

    struct Note
    {
        float volume;
        int index;

        bool operator< (const Note& rhs)
        {
            return volume < rhs.volume;
        }
    };

    std::vector<Note> notes;

    for (int i = 0; i < (int)src.size(); ++i)
    {
        const float f = src[i];
        if (f < threshold)
            continue;

        Note note = { f, i };
        notes.push_back(note);
    }

    std::sort(notes.begin(), notes.end());

    // 49 - 60 inclusive
    const char* piano_notes[] = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", };

    for (int i = 0; i < (int)notes.size(); ++i)
    {
        const int frequency_bin = notes[i].index;
        const float wave_frequency_over_window = float(frequency_bin);
        const float window_to_seconds = ((44100.0f / 4.0f) / 2.0f) / W;
        const float est_frequency = wave_frequency_over_window * window_to_seconds;

        const int note_index = int(12.0f * std::logf(est_frequency / 440.0f)) + 49;

        const int piano_octave = note_index / 12;
        const int piano_key = (note_index - 1) % 12;

        test_notes.push_back(piano_key);
    }

    return 0;
}

void GMAudioStream::DrawFrameRawWaveform(int channel, v3 color, float alpha)
{
    DrawWaveform(_channels[channel].raw, v2(1.0f, 1.0f), color, alpha);
}

void GMAudioStream::DrawFrameFFTWaveform(int channel, v3 color, float alpha)
{
    DrawWaveform(_channels[channel].fft, v2(2.0f, 1.0f), color, alpha);
}

void GMAudioStream::DrawFrameAverageWaveform(v3 color, float alpha)
{
    DrawWaveform(_average_fft, v2(2.0f, 1.0f / 60.0f), color, alpha);
}

void GMAudioStream::DrawFrameDifferenceWaveform(v3 color, float alpha)
{
    DrawWaveform(_difference_fft, v2(2.0f, 1.0f / 60.0f), color, alpha);
}

void GMAudioStream::DrawFrameRawBars(int channel, v3 color, float alpha)
{
    DrawBars(_channels[channel].raw, v2(1.0f, 1.0f), color, alpha);
}

void GMAudioStream::DrawFrameFFTBars(int channel, v3 color, float alpha)
{
    const int W = GetFFTWindowSize();
    const float scale = 1.0f / float(W);
    DrawBars(_channels[channel].fft, v2(2.0f, 1.0f / 60.0f), color, alpha);
}

void GMAudioStream::DrawFrameAverageBars(v3 color, float alpha)
{
    DrawBars(_average_fft, v2(2.0f, 1.0f / 60.0f), color, alpha);
}

void GMAudioStream::DrawFrameDifferenceBars(v3 color, float alpha)
{
    DrawBars(_difference_fft, v2(2.0f, 1.0f / 60.0f), color, alpha);
}

void GMAudioStream::DrawWaveform(const std::vector<float>& data, v2 scale, v3 color, float alpha)
{
    const int count = data.size();

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = v2(1024.0f, 768.0f);

    const float step = (tr.x - bl.x) / float(data.size()) * scale.x;
    const float range = (tr.y - bl.y) / scale.y;

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

void GMAudioStream::DrawBars(const std::vector<float>& data, v2 scale, v3 color, float alpha)
{
    const int count = data.size();

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = v2(1024.0f, 768.0f);

    const float step = (tr.x - bl.x) / float(data.size()) * scale.x;
    const float range = (tr.y - bl.y) / scale.y;

	glColor4f( color.x, color.y, color.z, alpha );

    for (int i = 1; i < count; ++i)
    {
        const float value0 = data[i - 1];
        const float value1 = data[i - 0];
        const v2 a = bl + v2( step * float(i - 1), range * value0 );
        const v2 b = bl + v2( step * float(i - 0), range * value1 );

        DrawLine(v2(a.x, a.y), v2(a.x, b.y));
        DrawLine(v2(a.x, b.y), v2(b.x, b.y));
        DrawLine(v2(b.x, b.y), v2(b.x, b.y));
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

void GMAudioStream::ClearInputDataFrame()
{
    for (int i = 0; i < (int)_channels.size(); ++i)
    {
        _channels[i].raw.clear();
        _channels[i].fft.clear();
    }

    _frame++;
}

void GMAudioStream::SetFFTMagnifyScale(float scale)
{
    _fft_magnify_scale = scale;
}

void GMAudioStream::SetFFTMagnifyPower(float power)
{
    _fft_magnify_power = power;
}

void GMAudioStream::AddInputDataFrameRemoteNao()
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

void GMAudioStream::AddInputDataFrameMicrophone()
{
    const int W = GetFFTWindowSize();
    const int MicrophoneDataTypeMaxValue = 255;

    CHECK(_recorder);

    // currently we are taking the top N samples
    // but rather we need to take one frame of samples
    // until there is at least one frame, we ignore

    // we are almost in sync, drifting about 1s every 3 min
    // the mic buffer is filling up faster than we can process it
    // we can probably assume the mic buffer is well-timed and 
    // use it's status to run extra updates to sync

    // microphone is drifting behind game time and clock time slowly
    // we will just deal with microphone data as though it is available
    // at our expected clock time, and ignore drift!

    // init empty values

    _channels[0].raw.clear();
    _channels[0].raw.resize(W, 0.0f);

    // collect the last W of data

    if ((int)_microphone_buffer.size() >= W)
    {
        for (int i = 0; i < W; ++i)
        {
            const int index = _microphone_buffer.size() - i - 1;
            const signed char raw = (signed char)_microphone_buffer[index];
            const float value = float(raw + MicrophoneDataTypeMaxValue / 2) / float(MicrophoneDataTypeMaxValue);

            _channels[0].raw[i] = value;
        }
    }

    // remove the frame of buffer data

    const int removal = std::min<int>(W, _microphone_buffer.size());

    //if (removal < W)
        //TimePrint("mic: missing %d frames of data", W - removal);

    const int shift = _microphone_buffer.size() - removal;
    if (shift > 0)
    {
        memcpy(&_microphone_buffer[0], &_microphone_buffer[0] + shift, removal);
        _microphone_buffer.resize(_microphone_buffer.size() - removal);
    }

    _microphone_samples_read += shift;

    //TimePrint("mic: buffer: %d samples", _microphone_buffer.size());
}

void GMAudioStream::AddInputDataFrameSineWave(int frequency, float amplitude)
{
    const int channels = 4;
    const int W = GetFFTWindowSize();

    _channels.resize(channels);

    for (int i = 0; i < channels; ++i)
    {
        _channels[i].raw.resize(W);

        for (int j = 0; j < W; ++j)
        {
            const float d = float(j) / float(W) * PI * 2.0f;
            const float t = float(d * frequency);
            const float s = std::sinf(t) * 0.5f * amplitude;
            _channels[i].raw[j] += s;
        }
    }
}

void GMAudioStream::UpdateMicrophoneBuffer()
{
    typedef signed char MicrophoneDataType;

    const int W = GetFFTWindowSize();
    const FMOD_SOUND_FORMAT format = FMOD_SOUND_FORMAT_PCM8;
    const int bytesize = sizeof(MicrophoneDataType);

    if (!_recorder)
    {
        _recorder = new MicrophoneRecorder(_frequency, 1, format);
        _recorder->RecordStart();
    }

    _recorder->Update();

    // currently we are taking the top N samples
    // but rather we need to take one frame of samples
    // until there is at least one frame, we ignore

    // we are almost in sync, drifting about 1s every 3 min
    // the mic buffer is filling up faster than we can process it
    // we can probably assume the mic buffer is well-timed and 
    // use it's status to run extra updates to sync

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
            //TimePrint("mic: dropping %d samples", shift);
            memcpy(&_microphone_buffer[0], &_microphone_buffer[0] + shift, one_second_data);
        }

        _microphone_buffer.resize(std::min<int>(one_second_data, _microphone_buffer.size()));
    }
}

bool GMAudioStream::IsMicrophoneFrameBuffered()
{
    const int W = GetFFTWindowSize();
    const bool pending = (int)_microphone_buffer.size() > W;
    return pending;
}

float GMAudioStream::GetSecondsByFrame()
{
    return float(_frame) / float(_framerate);
}

float GMAudioStream::GetSecondsByMicrophone()
{
    return float(_microphone_samples_read / _frequency) / 60.0f;
}

float GMAudioStream::GetSecondsBySystemClock()
{
    // NOTE: only accurate to while seconds
    //const int relative = ::clock() - _clock_start;
    //return float(relative / CLOCKS_PER_SEC);
    return _clock_timer.GetTimeSecs();
}

void GMAudioStream::ResetTimers()
{
    _frame = 0;
    _microphone_samples_read = 0;
    _clock_start = ::clock();
    _clock_timer.Start();
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

    GM_MEMFUNC_DECL(ClearInputDataFrame)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->ClearInputDataFrame());
        return GM_OK;
    }

    GM_MEMFUNC_DECL(AddInputDataFrameSineWave)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_INT_PARAM(frequency, 0);
        GM_CHECK_FLOAT_PARAM(amplitude, 1);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->AddInputDataFrameSineWave(frequency, amplitude));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(AddInputDataFrameMicrophone)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->AddInputDataFrameMicrophone());
        return GM_OK;
    }

    GM_MEMFUNC_DECL(AddInputDataFrameRemoteNao)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->AddInputDataFrameRemoteNao());
        return GM_OK;
    }

    GM_MEMFUNC_DECL(UpdateMicrophoneBuffer)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->UpdateMicrophoneBuffer());
        return GM_OK;
    }

    GM_MEMFUNC_DECL(IsMicrophoneFrameBuffered)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(a_thread->PushInt(self->IsMicrophoneFrameBuffered() ? 1 : 0));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(SetFFTMagnifyScale)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_FLOAT_PARAM(scale, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->SetFFTMagnifyScale(scale));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(SetFFTMagnifyPower)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_FLOAT_PARAM(power, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->SetFFTMagnifyPower(power));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(GetFFTWindowSize)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(a_thread->PushInt(self->GetFFTWindowSize()));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawFrameRawWaveform)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(channel, 0);
        GM_CHECK_VEC3_PARAM(color, 1);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawFrameRawWaveform(channel, color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawFrameFFTWaveform)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(channel, 0);
        GM_CHECK_VEC3_PARAM(color, 1);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawFrameFFTWaveform(channel, color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawFrameAverageWaveform)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_VEC3_PARAM(color, 0);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawFrameAverageWaveform(color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawFrameDifferenceWaveform)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_VEC3_PARAM(color, 0);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawFrameDifferenceWaveform(color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawFrameRawBars)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(channel, 0);
        GM_CHECK_VEC3_PARAM(color, 1);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawFrameRawBars(channel, color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawFrameFFTBars)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(channel, 0);
        GM_CHECK_VEC3_PARAM(color, 1);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawFrameFFTBars(channel, color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawFrameAverageBars)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_VEC3_PARAM(color, 0);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawFrameAverageBars(color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(DrawFrameDifferenceBars)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_VEC3_PARAM(color, 0);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawFrameDifferenceBars(color, alpha));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcFrameDFT)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(channel, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcFrameDFT(channel));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcFrameFFT)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(channel, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcFrameFFT(channel));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcFrameAverageAndDifference)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(channel, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcFrameAverageAndDifference(channel));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcEstimatedBeatsPerSecond)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(channel, 0);
        GM_CHECK_INT_PARAM(bin, 1);
        GM_CHECK_FLOAT_PARAM(threshold, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(a_thread->PushInt(self->CalcEstimatedBeatsPerSecond(channel, bin, threshold)));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(NoteTuner)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_FLOAT_PARAM(threshold, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->NoteTuner(threshold));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CalcFramePitches)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_FLOAT_PARAM(threshold, 0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->CalcFramePitches(threshold));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(TestGetPianoNotes)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_FLOAT_PARAM(threshold, 0);
        GM_CHECK_TABLE_PARAM(test_notes, 1);
		GM_GET_THIS_PTR(GMAudioStream, self);
        std::vector<int> test_notes_vec;
        GM_AL_EXCEPTION_WRAPPER(a_thread->PushInt(self->TestGetPianoNotes(threshold, test_notes_vec)));

        for (int i = 0; i < (int)test_notes_vec.size(); ++i)
        {
            test_notes->Set(a_thread->GetMachine(), i, gmVariable(test_notes_vec[i]));
        }

        return GM_OK;
    }

    GM_MEMFUNC_DECL(ResetTimers)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->ResetTimers());
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMAudioStream)
GM_REG_MEMFUNC( GMAudioStream, SetActive )
GM_REG_MEMFUNC( GMAudioStream, Update )
GM_REG_MEMFUNC( GMAudioStream, ClearInputDataFrame )
GM_REG_MEMFUNC( GMAudioStream, AddInputDataFrameSineWave )
GM_REG_MEMFUNC( GMAudioStream, AddInputDataFrameMicrophone )
GM_REG_MEMFUNC( GMAudioStream, AddInputDataFrameRemoteNao )
GM_REG_MEMFUNC( GMAudioStream, UpdateMicrophoneBuffer )
GM_REG_MEMFUNC( GMAudioStream, IsMicrophoneFrameBuffered )
GM_REG_MEMFUNC( GMAudioStream, SetFFTMagnifyScale )
GM_REG_MEMFUNC( GMAudioStream, SetFFTMagnifyPower )
GM_REG_MEMFUNC( GMAudioStream, GetFFTWindowSize )
GM_REG_MEMFUNC( GMAudioStream, CalcFrameDFT )
GM_REG_MEMFUNC( GMAudioStream, CalcFrameFFT )
GM_REG_MEMFUNC( GMAudioStream, CalcFrameAverageAndDifference )
GM_REG_MEMFUNC( GMAudioStream, CalcEstimatedBeatsPerSecond )
GM_REG_MEMFUNC( GMAudioStream, DrawFrameRawWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawFrameFFTWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawFrameAverageWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawFrameDifferenceWaveform )
GM_REG_MEMFUNC( GMAudioStream, DrawFrameRawBars )
GM_REG_MEMFUNC( GMAudioStream, DrawFrameFFTBars )
GM_REG_MEMFUNC( GMAudioStream, DrawFrameAverageBars )
GM_REG_MEMFUNC( GMAudioStream, DrawFrameDifferenceBars )
GM_REG_MEMFUNC( GMAudioStream, NoteTuner )
GM_REG_MEMFUNC( GMAudioStream, CalcFramePitches )
GM_REG_MEMFUNC( GMAudioStream, TestGetPianoNotes )
GM_REG_MEMFUNC( GMAudioStream, ResetTimers )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMAudioStream);
