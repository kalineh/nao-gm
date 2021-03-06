//
// beat_detection.cpp
//

#include "beat_detection.h"
#include "synthesizer.h"
#include "fft.h"

#include <gfx/DrawPrimitives.h>
#include <sound/SoundMngr.h>

// dsp guide
// http://www.dspguide.com/ch8/2.htm

// beat detection notes
// http://www.flipcode.com/misc/BeatDetectionAlgorithms.pdf

#define PI 3.14159265358979f
#define E 2.71828182846f

// good for verifying DFT results
// http://home.fuse.net/clymer/graphs/fourier.html

void Hanning(int count, float* input, float* output)
{
    // TODO: magical optimized version
    for (int i = 0; i < count; ++i)
    {
        output[i] = input[i] * 0.5f * (1.0f - std::cosf(2.0f * PI * float(i) / float(count)));
    }
}

void sincos_x86(float angle, float* sinout, float* cosout)
{
   _asm
   {
      fld DWORD PTR [angle]
      fsincos
      mov ebx,[cosout]
      fstp DWORD PTR [ebx]
      mov ebx,[sinout]
      fstp DWORD PTR [ebx]
   }
}

int roundup_pot(int value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}

// http://www.librow.com/articles/article-10
void FFT1(int count, float* input, float* out, float magnify_scale, float magnify_power)
{
    static std::vector<librow::complex> buffer;

    const int count_pot = roundup_pot(count);

    buffer.resize(count_pot);
    
    int i = 0;
    for (; i < count; ++i)
        buffer[i] = librow::complex(input[i], 0.0f);
    for (; i < count_pot; ++i)
        buffer[i] = librow::complex(0.0f, 0.0f);

    bool success = librow::CFFT::Inverse(&buffer[0], count_pot, false);
    ASSERT(success);

    memset(out, 0, sizeof(out[0]) * count);
    const int nyquist = count / 2;

    for (int bin = 0; bin < nyquist; ++bin)
    {
        const librow::complex c = buffer[bin];

        const float f0 = std::sqrtf(c.norm());
        const float f1 = f0 * magnify_scale;
        const float f2 = std::powf(f1, magnify_power);
        const float f3 = std::logf(f2);
        const float f4 = std::max<float>(0.000000001f, f3);

        out[bin] = f4;
    }
}

void DFT3(int count, float* input, float* out, float magnify_scale, float magnify_power)
{
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

        float cos = 0.0f;
        float sin = 0.0f;

        for (int t = 0; t < count; ++t)
        {
            const float f = input[t];

            //real += f * std::cosf(a * t);
            //imag += f * std::sinf(a * t);

            sincos_x86(a * t, &sin, &cos);
            real += f * cos;
            imag += f * sin;
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
    , _input_data_added_this_frame(false)
    , _average_index(0)
    , _frequency(44100 / 1)
    , _framerate(60)
    , _frame(0)
    , _microphone_samples_read(0)
    , _clock_start(0)
    , _fft_magnify_scale(60.0f)
    , _fft_magnify_power(1.0f)
    , _synthesizer(nullptr)
    , _mirror_notes_index(0)
{
    _clock_timer.Start();
    _notebrain = StrongHandle<NoteBrain>(new NoteBrain());
    _mirror_notes.resize(_framerate * 30, -1);
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

    const int ideal = _frequency / _framerate;
    const int padded = roundup_pot(ideal);
    
    return padded;
}

int GMAudioStream::GetFrameRate()
{
    return _framerate;
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

    if (_input_data_added_this_frame)
    {
        _average_index += 1;
        _average_index %= _framerate;

        _notebrain->Update();
    }
    else
    {
        // must keep this in valid range whether updating or not, framerate may change
        _average_index %= _framerate;
    }

    // -- synth test -- 

    if (!_synthesizer)
        _synthesizer = new Synthesizer(_frequency, _frequency * 2);

    // the rate at which fmod pulls samples is higher than our update rate
    // since we are running slightly slower because of dropped frames
    // how do we rectify this mismatch?
    //const int samples = _frequency / _framerate;

    // samples required is the minimum to keep up with fmod
    // but we would like to process roughly a frame worth of samples each frame
    // to keep the playback notes being started at a good time
    // since otherwise the synthesizer cursor will only step large amounts as fmod updates
    const int samples_desired = _synthesizer->CalculateStreamDesiredSamplesPerFrame(_frequency, _framerate);
    const int samples_required = _synthesizer->CalculateStreamRequiredSamples();
    const int samples = std::max<int>(samples_desired, samples_required);

    _synthesizer->Update(samples);
    _synthesizer->Play(samples);

    CaptureMirrorNotes();

    //printf("Time: %.2f, %.2f, %.2f\n", GetSecondsByFrame(), GetSecondsByMicrophone(), GetSecondsBySystemClock());

    _input_data_added_this_frame = false;
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

    FFT1(W, &c.raw[0], &c.fft[0], _fft_magnify_scale, _fft_magnify_power);
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
   return 48.0f + 12.0f * (std::logf(hz / 440.0f) / std::logf(2.0f));
}

float PitchToFrequency(float pitch)
{
  return 440.0f * std::powf(2.0f, (pitch - 48.0f) / 12.0f);
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

const int ScaleTypes = 2;
const int ScaleNotes = 7;
const int OctaveNotes = 12;

static const int NoteScaleOffsets[ScaleTypes][ScaleNotes] = {
    { 0, 2, 4, 5, 7, 9, 11 }, // major
    { 0, 2, 3, 5, 7, 8, 10 }, // minor
    // others
};

float CubicMaximize(float y0, float y1, float y2, float y3, float *maxyVal)
{
   // Find coefficients of cubic
   float a, b, c, d;

   a = y0 / -6.0f + y1 / 2.0f - y2 / 2.0f + y3 / 6.0f;
   b = y0 - 5.0f * y1 / 2.0f + 2.0f * y2 - y3 / 2.0f;
   c = -11.0f * y0 / 6.0f + 3.0f * y1 - 3.0f * y2 / 2.0f + y3 / 3.0f;
   d = y0;

   // Take derivative
   float da, db, dc;

   da = 3 * a;
   db = 2 * b;
   dc = c;

   // Find zeroes of derivative using quadratic equation
   float discriminant = db * db - 4 * da * dc;
   if (discriminant < 0.0f)
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

NoteBrain::NoteBrain()
    : _forget_rate(0.0001f)
{
    _notes.resize(84);
    _notes_sorted.resize(_notes.size());

    for (int i = 0; i < (int)_notes.size(); ++i)
    {
        _notes[i].note = i;
        _notes[i].confidence = 0.0f;
    }

    _scales.resize(ScaleTypes * OctaveNotes);
    _scales_sorted.resize(_scales.size());

    for (int s = 0; s < ScaleTypes; ++s)
    {
        for (int n = 0; n < OctaveNotes; ++n)
        {
            const int index = s * OctaveNotes + n;

            _scales[index].scale = s;
            _scales[index].fundamental = n;
            _scales[index].confidence = 0.0f;
        }
    }
}

void NoteBrain::SetForgetRate(float forget)
{
    _forget_rate = forget;
}

float NoteBrain::GetNoteConfidence(int octave, int note)
{
    const int index = octave * OctaveNotes + note;
    return _notes[index].confidence;
}

float NoteBrain::GetScaleConfidence(int scale, int note)
{
    const int index = scale * OctaveNotes + note;
    return _scales[index].confidence;
}

int NoteBrain::GetBestNoteNote(int rank)
{
    return _notes_sorted[rank].note;
}

float NoteBrain::GetBestNoteConfidence(int rank)
{
    return _notes_sorted[rank].confidence;
}

int NoteBrain::GetBestScaleScale(int rank)
{
    return _scales_sorted[rank].scale;
}

int NoteBrain::GetBestScaleFundamental(int rank)
{
    return _scales_sorted[rank].fundamental;
}

float NoteBrain::GetBestScaleConfidence(int rank)
{
    return _scales_sorted[rank].confidence;
}

void NoteBrain::Update()
{
    {
        for (int i = 0; i < (int)_notes.size(); ++i)
        {
            NoteConfidence& note = _notes[i];
            note.confidence = std::max<float>(note.confidence - note.confidence * _forget_rate, 0.0f);
        }

        struct {
            bool operator() (const NoteConfidence& lhs, const NoteConfidence& rhs)
            {
                return lhs.confidence > rhs.confidence;
            }
        } sorter;

        _notes_sorted.resize(_notes.size());
        std::copy(_notes.begin(), _notes.end(), _notes_sorted.begin());
        std::sort(_notes_sorted.begin(), _notes_sorted.end(), sorter);
    }

    CalculateScaleEstimates();

    {
        struct {
            bool operator() (const ScaleConfidence& lhs, const ScaleConfidence& rhs)
            {
                return lhs.confidence > rhs.confidence;
            }
        } sorter;

        _scales_sorted.resize(_scales.size());
        std::copy(_scales.begin(), _scales.end(), _scales_sorted.begin());
        std::sort(_scales_sorted.begin(), _scales_sorted.end(), sorter);
    }
}

void NoteBrain::AddNote(int note, float confidence)
{
    if (note < 0)
        return;

    if (note >= (int)_notes.size())
        return;

    if (confidence <= 0.0f)
        return;
    
    _notes[note].confidence += confidence;
    _notes[note].frame += confidence;
}

void NoteBrain::CalculateScaleEstimates()
{
    // evaluate all notes for all known scales

    for (int s = 0; s < ScaleTypes; ++s)
    {
        for (int n = 0; n < OctaveNotes; ++n)
        {
            const float confidence = CalculateScaleEstimate(s, n);
            const int index = OctaveNotes * s + n;

            _scales[index].scale = s;
            _scales[index].fundamental = n;
            _scales[index].confidence = confidence;
        }
    }
}

float NoteBrain::CalculateScaleEstimate(int scale, int fundamental)
{
    // NOTE: we scan 1 less octave than we store, because the start_note will walk up to one octave extra
    const int Octaves = 6;
    const int OctaveNotes = 12;
    const int ScaleNotes = 7;

    // A, A#, B, C, C#, D, D#, E, F, F#, G, G#
    float result = 0.0f;

    for (int o = 0; o < Octaves; ++o)
    {
        for (int n = 0; n < ScaleNotes; ++n)
        {
            const int index = fundamental + o * OctaveNotes + NoteScaleOffsets[scale][n];
            const float value = _notes[index].confidence;

            result += value;
        }
    }

    return result;
}

GM_REG_NAMESPACE(NoteBrain)
{
	GM_MEMFUNC_DECL(CreateNoteBrain)
	{
		GM_CHECK_NUM_PARAMS(0);
		GM_AL_EXCEPTION_WRAPPER(GM_PUSH_USER_HANDLED( NoteBrain, new NoteBrain() ));
		return GM_OK;
	}
    GM_GEN_MEMFUNC_VOID_FLOAT( NoteBrain, SetForgetRate )
    GM_GEN_MEMFUNC_FLOAT_INT_INT( NoteBrain, GetNoteConfidence )
    GM_GEN_MEMFUNC_FLOAT_INT_INT( NoteBrain, GetScaleConfidence )
    GM_GEN_MEMFUNC_INT_INT( NoteBrain, GetBestNoteNote )
    GM_GEN_MEMFUNC_FLOAT_INT( NoteBrain, GetBestNoteConfidence )
    GM_GEN_MEMFUNC_INT_INT( NoteBrain, GetBestScaleScale )
    GM_GEN_MEMFUNC_INT_INT( NoteBrain, GetBestScaleFundamental )
    GM_GEN_MEMFUNC_FLOAT_INT( NoteBrain, GetBestScaleConfidence )
}

GM_REG_MEM_BEGIN(NoteBrain)
GM_REG_MEMFUNC( NoteBrain, SetForgetRate )
GM_REG_MEMFUNC( NoteBrain, GetNoteConfidence )
GM_REG_MEMFUNC( NoteBrain, GetScaleConfidence )
GM_REG_MEMFUNC( NoteBrain, GetBestNoteNote )
GM_REG_MEMFUNC( NoteBrain, GetBestNoteConfidence )
GM_REG_MEMFUNC( NoteBrain, GetBestScaleScale )
GM_REG_MEMFUNC( NoteBrain, GetBestScaleFundamental )
GM_REG_MEMFUNC( NoteBrain, GetBestScaleConfidence )
GM_REG_MEM_END()

GM_BIND_DEFINE(NoteBrain);

float GaussianSample2(float x, float sigma)
{
    return std::expf(-0.5f * std::powf(x / sigma, 2.0f)) / (2.0f * PI * sigma * sigma);
}

void GMAudioStream::CalcFramePitches(float threshold)
{
    const int W = GetFFTWindowSize();

    // 5 octaves
    _pitch.clear();
    _pitch.resize(87, 0.0f);

    // either can use instantaneous or averages
    std::vector<float>& src = _channels[0].fft;

    // TODO: the first two bins have high values for unknown reasons
    src[0] = 0.0f;
    src[1] = 0.0f;

    // iterate each note
    // determine which bins to read
    // sum weighted values
    // add that for note pitch value

    // get adjacent N bins
    // get gaussian weight for that bin, centered around the note hz x
    // remember that bins are /linear/, so gaussian weighting is ok
    // just need to gaussian weight adjacent bins

    // we cant use constant gaussian convolution filter because the peak is not
    // centered around x=0

    // so, for each bin value, what is it's x value in a gaussian curve centered on note at x?
    // sum and threshold

    // consider variable window size for lower bins

    // offset is distance from center bin to actual hz
    // then it is x = (binhz * step + offset) / binhz

    // so while this is correctly giving gaussian peaks for notes of a given pitch,
    // it gives too much weight to nearby notes making it quite fuzzy
    // we may want to detect the peaks in this data, or just pow() it even further

    float debug_frequencies_view[87] = { 0.0f };
    for (int i = 0; i < 87; ++i)
        debug_frequencies_view[i] = PitchToFrequency(float(i));

    // skip first octaves because too low
    for (int i = 12*3; i < (int)_pitch.size(); ++i)
    {
        const float note_hz = PitchToFrequency(float(i));
        const float bin_step = float(_frequency) / float(W);

        const int center = int(note_hz / bin_step + 0.5f);
        const float bin_hz = float(center) * bin_step;
        const float offset = (note_hz - bin_hz) / bin_step;

        // lower is higher peak and faster falloff
        const float sigma = 0.3f;

        float sum = 0.0f;
        for (int w = -2; w < 3; ++w)
        {
            const int bin = std::max<int>(0, std::min<int>(src.size() - 1, center + w));
            const float weight = GaussianSample2(float(w) + offset, sigma);
            const float value = src[bin] * weight;

            sum += value;
        }

        if (sum < threshold)
            continue;

        _pitch[i] += sum;
    }


    for (int i = 0; i < (int)_pitch.size(); ++i)
    {
        _notebrain->AddNote(i, _pitch[i]);
    }
}

/*
void CalcFramePitchesOld(float threshold)
{
    // we can simply scan for each frequency we want to test
    // if we take a gaussian interpolate of the bins we can estimate the note

    const int W = GetFFTWindowSize();

    // 5 octaves
    //_pitch.clear();
    _pitch.resize(87);

    for (int i = 0; i < (int)_pitch.size(); ++i)
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

            // TODO: offset is fixed, we should need this anymore, but need to fix the 1-off if using this code again
            //const int pitch = int(FrequencyToPitch(hz) + 0.1f) - 20; // TODO: why is it 20 off? (and maybe -1 for 0-based index of notes?)

            // TODO: we can get negative pitch somehow here, figure out why
            //       likely some artifact of the cubic maximize
            if (pitch < 0)
                continue;

            // ignoring out-of-range notes
            if (pitch >= 87)
                continue;

            //write += sprintf(write, "%s%d ", GetNoteName(pitch), GetNoteOctave(pitch));
            //write += sprintf(write, "%.2fhz (%.2f)", pitch, y);
            //TimePrint("%d ", pitch);

            _pitch[pitch] += 1;

            _notebrain->AddNote(pitch, y);
        }

        was_rising = now_rising;
    }

    // TODO
    //_notebrain->EstimateScale();
}
*/

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
    DrawWaveform(_channels[channel].raw, v2(1.0f, 1.0f), color, alpha, false);
}

void GMAudioStream::DrawFrameFFTWaveform(int channel, v3 color, float alpha)
{
    DrawWaveform(_channels[channel].fft, v2(2.0f, 1.0f), color, alpha, true);
}

void GMAudioStream::DrawFrameAverageWaveform(v3 color, float alpha)
{
    DrawWaveform(_average_fft, v2(2.0f, 1.0f), color, alpha, true);
}

void GMAudioStream::DrawFrameDifferenceWaveform(v3 color, float alpha)
{
    DrawWaveform(_difference_fft, v2(2.0f, 1.0f), color, alpha, true);
}

void GMAudioStream::DrawFrameRawBars(int channel, v3 color, float alpha)
{
    DrawBars(_channels[channel].raw, v2(1.0f, 1.0f), color, alpha, false);
}

void GMAudioStream::DrawFrameFFTBars(int channel, v3 color, float alpha)
{
    DrawBars(_channels[channel].fft, v2(2.0f, 1.0f), color, alpha, true);
}

void GMAudioStream::DrawFrameAverageBars(v3 color, float alpha)
{
    DrawBars(_average_fft, v2(2.0f, 1.0f), color, alpha, true);
}

void GMAudioStream::DrawFrameDifferenceBars(v3 color, float alpha)
{
    DrawBars(_difference_fft, v2(2.0f, 1.0f), color, alpha, true);
}

void GMAudioStream::DrawWaveform(const std::vector<float>& data, v2 scale, v3 color, float alpha, bool logarithmic)
{
    const int count = data.size();
    const float exponent = logarithmic ? (1.0f / E) : 1.0f;

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = v2(1024.0f, 768.0f);

    const float step = (tr.x - bl.x) / float(count) * scale.x;
    const float range = (tr.y - bl.y) / scale.y;

	glColor4f( color.x, color.y, color.z, alpha );

    for (int i = 1; i < count; ++i)
    {
        const float value0 = data[i - 1];
        const float value1 = data[i - 0];
        const float ta = std::powf( float(i - 1) / float(count), exponent );
        const float tb = std::powf( float(i - 0) / float(count), exponent );
        const v2 a = bl + v2( (tr.x - bl.x) * ta, range * value0 );
        const v2 b = bl + v2( (tr.x - bl.x) * tb, range * value1 );

        DrawLine(a, b);
    }
}

void GMAudioStream::DrawBars(const std::vector<float>& data, v2 scale, v3 color, float alpha, bool logarithmic)
{
    const int count = data.size();
    const float exponent = logarithmic ? (1.0f / E) : 1.0f;

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = v2(1024.0f, 768.0f);

    const float step = (tr.x - bl.x) / float(count) * scale.x;
    const float range = (tr.y - bl.y) / scale.y;

	glColor4f( color.x, color.y, color.z, alpha );

    for (int i = 2; i < count; ++i)
    {
        const float value0 = data[i - 1];
        const float value1 = data[i - 0];
        const float ta = std::powf( float(i - 1) / float(count), exponent );
        const float tb = std::powf( float(i - 0) / float(count), exponent );
        const v2 a = bl + v2( (tr.x - bl.x) * ta, 0.0f );
        const v2 b = bl + v2( (tr.x - bl.x) * tb, range * value1 );

        v3 colortweak = color * v3((i % 2) == 0 ? 1.0f : 0.9f);

    	glColor4f( colortweak.x, colortweak.y, colortweak.z, alpha );

        DrawRect( a, b - a );
    }
}

void GMAudioStream::DrawSynthesizer(v2 scale, v3 color, float alpha)
{
    //_synthesizer->DrawBuffer(scale, color, alpha);
    _synthesizer->DrawCursor(scale, color * 0.5f, alpha);
    _synthesizer->DrawTracker(scale, color, alpha);
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

    // collect the last W of data

    if ((int)_microphone_buffer.size() >= W)
    {
        _channels[0].raw.resize(W, 0.0f);

        for (int i = 0; i < W; ++i)
        {
            const int index = _microphone_buffer.size() - i - 1;
            const signed char raw = (signed char)_microphone_buffer[index];
            const float value = float(raw + MicrophoneDataTypeMaxValue / 2) / float(MicrophoneDataTypeMaxValue);

            _channels[0].raw[i] = value;
        }

        // apply windowing function
        Hanning(W, &_channels[0].raw[0], &_channels[0].raw[0]);

        _input_data_added_this_frame = true;
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

    _input_data_added_this_frame = true;
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

void GMAudioStream::TestAddSynthNote(int delay, int samples, float pitch, float amplitude)
{
    _synthesizer->SineWave(delay, samples, pitch, amplitude);
}

StrongHandle<NoteBrain> GMAudioStream::GetNoteBrain()
{
    return _notebrain;
}

void GMAudioStream::CaptureMirrorNotes()
{
    const int best_note_note = _notebrain->GetBestNoteNote(0);
    const float best_note_confidence = _notebrain->GetBestNoteConfidence(0);

    if (best_note_confidence > 5.0f)
    {
        _mirror_notes[_mirror_notes_index] = best_note_note;
    }
    else
    {
        _mirror_notes[_mirror_notes_index] = 0;
    }

    _mirror_notes_index += 1;
    _mirror_notes_index %= _mirror_notes.size();
}

void GMAudioStream::PlaybackMirrorNotes()
{
    const int samples = _synthesizer->CalculateStreamDesiredSamplesPerFrame(_frequency, _framerate);

    int cursor = -samples;

    int i = -1;

    // TEST
    //for (int i = 0; i < (int)_mirror_notes.size(); ++i)
        //_mirror_notes[i] = -1;

    //for (int i = 0; i < 60; ++i)
    //{
        //_mirror_notes[i + 60 * 0] = 50;
        //_mirror_notes[i + 60 * 1] = 54;
        //_mirror_notes[i + 60 * 2] = 50;
        //_mirror_notes[i + 60 * 3] = 54;
    //}

    // skip first empty notes
    while (true)
    {
        i++;

        if (i >= (int)_mirror_notes.size() - 1)
            break;

        if (_mirror_notes[i] != 0)
            break;
    }

    for (; i < (int)_mirror_notes.size() - 1; ++i)
    {
        cursor += samples;

        const int note = _mirror_notes[i];
        if (note == 0)
            continue;

        if (note == -1)
            break;

        int note_length = samples;

        while (i < (int)_mirror_notes.size() - 1 && _mirror_notes[i + 1] == note)
        {
            note_length += samples;
            i += 1;
        }

        const float amplitude = 0.5f;
        const int broken_note_calculation = note + 20;
        const float pitch = Note::NotePitch(broken_note_calculation / 12, broken_note_calculation % 12);

        TestAddSynthNote(cursor, note_length, pitch, amplitude);

        cursor += note_length - samples;
    }
}

void GMAudioStream::ResetMirrorNotes()
{
    for (int i = 0; i < (int)_mirror_notes.size(); ++i)
    {
        _mirror_notes[i] = -1;
    }
    _mirror_notes_index = 0;
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

    GM_MEMFUNC_DECL(GetFrameRate)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(a_thread->PushInt(self->GetFrameRate()));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(SetFrameRate)
    {
        GM_CHECK_NUM_PARAMS(1);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_CHECK_INT_PARAM(framerate, 0);
        GM_AL_EXCEPTION_WRAPPER(self->SetFrameRate(framerate));
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

    GM_MEMFUNC_DECL(DrawSynthesizer)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_VEC2_PARAM(scale, 0);
        GM_CHECK_VEC3_PARAM(color, 1);
        GM_CHECK_FLOAT_PARAM(alpha, 2);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_AL_EXCEPTION_WRAPPER(self->DrawSynthesizer(scale, color, alpha));
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

    GM_MEMFUNC_DECL(TestAddSynthNote)
    {
        GM_CHECK_NUM_PARAMS(5);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_CHECK_INT_PARAM(delay, 0);
        GM_CHECK_INT_PARAM(samples, 1);
        GM_CHECK_INT_PARAM(octave, 2);
        GM_CHECK_INT_PARAM(note, 3);
        GM_CHECK_FLOAT_PARAM(amplitude, 4);
        GM_AL_EXCEPTION_WRAPPER(self->TestAddSynthNote(delay, samples, Note::NotePitch(octave, note), amplitude));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(GetNoteBrain)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMAudioStream, self);
        GM_PUSH_USER_HANDLED(NoteBrain, self->GetNoteBrain().Get());
        return GM_OK;
    }

    GM_GEN_MEMFUNC_VOID_VOID( GMAudioStream, PlaybackMirrorNotes )
    GM_GEN_MEMFUNC_VOID_VOID( GMAudioStream, ResetMirrorNotes )
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
GM_REG_MEMFUNC( GMAudioStream, SetFrameRate )
GM_REG_MEMFUNC( GMAudioStream, GetFrameRate )
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
GM_REG_MEMFUNC( GMAudioStream, DrawSynthesizer )
GM_REG_MEMFUNC( GMAudioStream, NoteTuner )
GM_REG_MEMFUNC( GMAudioStream, CalcFramePitches )
GM_REG_MEMFUNC( GMAudioStream, TestGetPianoNotes )
GM_REG_MEMFUNC( GMAudioStream, ResetTimers )
GM_REG_MEMFUNC( GMAudioStream, TestAddSynthNote )
GM_REG_MEMFUNC( GMAudioStream, GetNoteBrain )
GM_REG_MEMFUNC( GMAudioStream, PlaybackMirrorNotes )
GM_REG_MEMFUNC( GMAudioStream, ResetMirrorNotes )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMAudioStream);
