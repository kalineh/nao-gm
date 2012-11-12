//
// synthesizer.cpp
//

#include "synthesizer.h"

// for helper functions only, remove
#include "beat_detection.h"

#include <gfx/DrawPrimitives.h>
#include <sound/SoundMngr.h>

#include <complex>
#include <vector>
#include <time.h>

#define PI 3.14159265358979f

static int fmod_pcm_length;
static float* fmod_pcm_buffer;
static unsigned int fmod_pcm_read_cursor;
static unsigned int fmod_pcm_write_cursor;

FMOD_RESULT F_CALLBACK pcmreadcallback_singlenote(FMOD_SOUND *sound, void *data, unsigned int datalen)
{
    static float seconds = 0.0f;

    const int samples = datalen / sizeof(float);

    const float note_hz = 440.0f;
    const float volume = 1.0f;

    // elapsed buffer by samples
    const float elapsed = samples / 44100.0f;
    const float step = elapsed / float(samples);

    float* p = (float*)data;

    for (int i = 0; i < samples; ++i)
    {
        const float t = seconds + step * i;
        const float s = sin(t * PI * 2.0f * note_hz) * 0.5f + 0.5f;
        p[i] = s * volume;
    }

    seconds += elapsed;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK pcmreadcallback(FMOD_SOUND *sound, void *data, unsigned int datalen)
{
    float* buffer = (float*)data;
    const int samples = datalen / sizeof(float);

    // fallback to last update data when no samples are ready
    if (fmod_pcm_read_cursor + samples >= fmod_pcm_write_cursor)
    {
        //printf("fallback %d samples\n", samples);
        return FMOD_ERR_NOTREADY;
    }

    for (int i = 0; i < samples; ++i)
    {
        const int index = (fmod_pcm_read_cursor + i) % fmod_pcm_length;
        buffer[i] = fmod_pcm_buffer[index];
    }

    fmod_pcm_read_cursor += samples;

    //printf("fmod read %d samples: %d\n", samples, fmod_pcm_read_cursor);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK pcmsetposcallback(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    // ignore
    return FMOD_OK;
}


FMODStream::FMODStream()
    : _sound(NULL)
    , _channel(NULL)
    , _frequency(0)
    , _buffer(0)
    , _buffer_length(0)
{
}

void FMODStream::Init(int frequency, float* buffer, int buffer_length)
{
    _frequency = frequency;
    _buffer = buffer;
    _buffer_length = buffer_length;

    // always gives FMOD_ERR_FORMAT :(
    //CreateStream(frequency);

    CreatePcmStream(frequency);
}

void FMODStream::CreatePcmStream(int frequency)
{
    const float seconds = float(_buffer_length) / _frequency;

    FMOD_RESULT result = FMOD_OK;
    FMOD_MODE mode = FMOD_2D | FMOD_LOOP_NORMAL | FMOD_SOFTWARE | FMOD_OPENUSER | FMOD_CREATESTREAM;
    FMOD_CREATESOUNDEXINFO settings; 

    memset(&settings, 0, sizeof(settings));

    settings.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    settings.length = _buffer_length * sizeof(float);
    settings.numchannels = 1;
    settings.defaultfrequency = frequency;
    settings.format = FMOD_SOUND_FORMAT_PCMFLOAT;

    settings.pcmreadcallback = pcmreadcallback;
    settings.pcmsetposcallback = pcmsetposcallback;

    fmod_pcm_read_cursor = 0U;
    fmod_pcm_write_cursor = 0U;
    fmod_pcm_length = _buffer_length;
    fmod_pcm_buffer = _buffer;

	FMOD::System* fmodSys = SoundMngr::Get()->GetSys();
    result = fmodSys->createSound(NULL, mode, &settings, &_sound);
    CHECK(result == FMOD_OK);
}

void FMODStream::CreateStream(int frequency)
{
    FMOD_RESULT result;
    FMOD_CREATESOUNDEXINFO settings;

    memset(&result, 0, sizeof(result));
    memset(&settings, 0, sizeof(settings));

    settings.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    settings.length = int( float(frequency) * 1.0f ) * sizeof(float) * 2;
    settings.decodebuffersize = frequency;
    settings.numchannels = 2;
    settings.defaultfrequency = frequency;
    settings.format = FMOD_SOUND_FORMAT_PCMFLOAT;

	FMOD::System* fmodSys = SoundMngr::Get()->GetSys();

    const char* buffer = (const char*)_buffer;
    result = fmodSys->createSound(buffer, FMOD_2D | FMOD_LOOP_NORMAL | FMOD_SOFTWARE | FMOD_OPENMEMORY_POINT | FMOD_CREATESTREAM, &settings, &_sound);
    CHECK(result == FMOD_OK);
}

void FMODStream::Play(int samples)
{
    fmod_pcm_write_cursor += samples;

    if (samples > 0)
    {
        //printf("stream write %d samples: %d\n", samples, fmod_pcm_write_cursor);
    }

    if (_channel != NULL)
        return;

	FMOD::System* fmodSys = SoundMngr::Get()->GetSys();
    FMOD_RESULT result = fmodSys->playSound(FMOD_CHANNEL_FREE, _sound, false, &_channel);
    CHECK(result == FMOD_OK);
}

/*

time needs to be synced to fmod
but it needs to be sample count because float lossiness will desync sounds
we need to track how many samples to create to be 0.5s ahead of fmod
making a synth sound needs to be at a given sample T

updating synth at 60hz~, each update we check for notes due to play
notes due to play must have their sample T, so we start mid-frame
playing at frame start means we would have up to 1/hz latency

B = buffer size
HB = B/2



synth update:
the key idea is that time is irrevelant
but only how many samples need generating
which in this case is determined by how much fmod is past the half-buffer mark
we could theoretically run at any framerate if there is enough time to generate HB samples

- how many samples behind HB are we
  - that is our count of samples to create (N)
- what notes occur in our sample generate time N
  - add note player for each
    - each note has a context: pitch, waveform, current t, max t
  - update each note by N samples
    - remove any completed notes

how do we determine how far behind we are?

fmod pulls N frames of data
we need to maintain HB minimum (advance)
when fmod updates, we subtract from advance
samples to update is HB - advance

*/

float Note::NotePitch(int octave, int note)
{
    // TODO: fix the offset issues

    const int n = octave * 12 + note;
    const float frequency = PitchToFrequency(float(n));

    return frequency;
}

Note Note::Noise(int sample_start, int sample_end, float amplitude)
{
    Note note;
    
    note._type = 1;
    note._sample_start = sample_start;
    note._sample_end = sample_end;
    note._pitch = 0.0f;
    note._amplitude = amplitude;

    return note;
}

Note Note::SineWave(int sample_start, int sample_end, float pitch, float amplitude)
{
    Note note;

    note._type = 2;
    note._sample_start = sample_start;
    note._sample_end = sample_end;
    note._pitch = pitch;
    note._amplitude = amplitude;

    return note;
}

void Note::Update(int frequency, int a, int b, float* out)
{
    // TODO: wrapping
    const int playback_samples = b - a;
    const int actual_samples = _sample_end - _sample_start;
    const int samples = std::min<int>(playback_samples, actual_samples);

    for (int i = 0; i < samples; ++i)
    {
        const int s = (a - _sample_start) + i;

        switch (_type)
        {
        case 0: break;
        case 1: out[i] = GenerateNoise(frequency, s); break;
        case 2: out[i] = GenerateSine(frequency, s); break;
        default:
            ASSERT(false);
            break;
        }
    }
}

float Note::GenerateNoise(int frequency, int s)
{
    const int i = s ^ s * s;

    return float(i % 65535) / 65535.0f;
}

float Note::GenerateSine(int frequency, int s)
{
    // r is sample range b - a
    // s is sample, from 0 to r
    // rt is time of this note range in seconds (r / f)
    // st is time of this sample, from 0.0f at a, to 1.0f at b

    const int range = _sample_end - _sample_start;
    const float range_time = float(range) / float(frequency);
    const float sample_time = float(s) / float(range);

    const float t = sample_time * _pitch * PI * 2.0f;
    const float f = std::sinf(t) * _amplitude;

    return f;
}

void Tracker::Reset()
{
    _pending.clear();
    _live.clear();
}

void Tracker::Queue(const Note& note)
{
    _pending.insert(_pending.begin(), note);
}

void Tracker::Update(int sample_a, int sample_b, int frequency, float* buffer)
{
    // ensure buffer is valid for writing sample_b - sample_a elements

    {
        auto a = _pending.begin();
        auto b = _pending.end();

        for (; a != b; )
        {
            const Note& note = *a;

            if (note._sample_start < sample_b)
            {
                _live.push_back(note);
                a = _pending.erase(a);
                b = _pending.end();
            }
            else
            {
                ++a;
            }
        }
    }

    for (int i = 0; i < (int)_live.size(); ++i)
    {
        _live[i].Update(frequency, sample_a, sample_b, buffer);
    }

    {
        auto a = _live.begin();
        auto b = _live.end();

        for (; a != b; )
        {
            const Note& note = *a;

            if (sample_b >= note._sample_end)
            {
                a = _live.erase(a);
                b = _live.end();
            }
            else
            {
                ++a;
            }
        }
    }
}

Synthesizer::Synthesizer(int frequency, int buffer_samples)
    : _stream()
    , _frequency(frequency)
    , _cursor(0U)
{
    _buffer.resize(buffer_samples);

    _stream.Init(frequency, &_buffer[0], _buffer.size());
}

int Synthesizer::CalculateStreamRequiredSamples() const
{
    const int desired_lead = _buffer.size() / 2;
    const int current_lead = _cursor - fmod_pcm_read_cursor;
    const int samples_needed = desired_lead - current_lead;

    const int clamped_samples_needed = std::max<int>(samples_needed, 0);

    ASSERT(current_lead >= 0);

    return clamped_samples_needed;
}

int Synthesizer::CalculateStreamDesiredSamplesPerFrame(int frequency, int framerate) const
{
    const int samples_per_second = frequency;
    const int samples_per_frame = samples_per_second / framerate;
    
    return samples_per_frame;
}

float Synthesizer::CalculateAbsoluteTime(int frequency) const
{
    const float sample_time = 1.0f / float(frequency);
    return sample_time * float(_cursor);
}

void Synthesizer::Update(int samples)
{
    if (samples <= 0)
        return;

    _scratch.resize(samples);

    float* scratch = &_scratch[0];
    memset(scratch, 0, samples * sizeof(float));

    _tracker.Update(_cursor, _cursor + samples, _frequency, scratch);
    
    for (int i = 0; i < (int)_scratch.size(); ++i)
    {
        const int index = (_cursor + i) % _buffer.size();
        const float value = scratch[i];

        _buffer[index] = value;
    }

    _cursor += samples;
}

void Synthesizer::Play(int samples)
{
    _stream.Play(samples);
}

void Synthesizer::Noise(int samples, float amplitude)
{
    _tracker.Queue(Note::Noise(_cursor, _cursor + samples, amplitude));
}

void Synthesizer::SineWave(int samples, float pitch, float amplitude)
{
    _tracker.Queue(Note::SineWave(_cursor, _cursor + samples, pitch, amplitude));
}

void Synthesizer::DrawBuffer(v2 scale, v3 color, float alpha)
{
    const int count = _buffer.size();

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = scale;

    const float step = (tr.x - bl.x) / float(count);
    const float range = (tr.y - bl.y);

	glColor4f( color.x, color.y, color.z, alpha );

    for (int i = 1; i < count; ++i)
    {
        const float value0 = _buffer[i - 1];
        const float value1 = _buffer[i - 0];
        const v2 a = bl + v2( step * float(i - 1), range * value0 );
        const v2 b = bl + v2( step * float(i - 0), range * value1 );

        DrawLine(a, b);
    }
}

void Synthesizer::DrawCursor(v2 scale, v3 color, float alpha)
{
    const int count = _buffer.size();

    const v2 bl = v2(0.0f, 0.0f);
    const v2 tr = scale;

    const float step = (tr.x - bl.x) / float(count);
    const float range = (tr.y - bl.y);

	glColor4f( color.x, color.y, color.z, alpha );

    int cursor_read = fmod_pcm_read_cursor % count;
    int cursor_write = fmod_pcm_write_cursor % count;

    {
        const float value0 = 0.0f;
        const float value1 = 1.0f;

        const v2 a = bl + v2( step * float(cursor_read - 1), range * value0 );
        const v2 b = bl + v2( step * float(cursor_read - 0), range * value1 );

        DrawLine(v2(a.x, a.y), v2(a.x, b.y));
        DrawLine(v2(a.x, b.y), v2(b.x, b.y));
        DrawLine(v2(b.x, b.y), v2(b.x, b.y));
    }

    {
        const float value0 = 0.0f;
        const float value1 = 1.0f;

        const v2 a = bl + v2( step * float(cursor_write - 1), range * value0 );
        const v2 b = bl + v2( step * float(cursor_write - 0), range * value1 );

        DrawLine(v2(a.x, a.y), v2(a.x, b.y));
        DrawLine(v2(a.x, b.y), v2(b.x, b.y));
        DrawLine(v2(b.x, b.y), v2(b.x, b.y));
    }
}
