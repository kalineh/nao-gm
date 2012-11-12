//
// synthesizer.h
//

#include "main.h"

#include <sound/MicrophoneRecorder.h>
#include <common/Timer.h>

using namespace funk;


class FMODStream
{
public:
    explicit FMODStream();

    void Init(int frequency, float* buffer, int buffer_length);
    void CreatePcmStream(int frequency);
    void CreateStream(int frequency);
    void Play(int samples);

private:
	FMOD::Sound* _sound;
	FMOD::Channel* _channel;
    int _frequency;
    float* _buffer;
    int _buffer_length;
};

struct Note
{
    int _type; // 0=null, 1=noise, 2=sine // square, tri, saw, pulse
    int _sample_start;
    int _sample_end;
    float _pitch;
    float _amplitude;
    // attack
    // delay
    // duty
    // etc

    static float NotePitch(int octave, int note);

    static Note Noise(int sample_start, int sample_end, float amplitude);
    static Note SineWave(int sample_start, int sample_end, float pitch, float amplitude);

    void Update(int frequency, int a, int b, float* out);

    float GenerateNoise(int frequency, int s);
    float GenerateSine(int frequency, int s);
};

class Tracker
{
public:
    void Reset();
    void Queue(const Note& note);
    void Update(int sample_a, int sample_b, int frequency, float* buffer);

private:
    std::list<Note> _pending;
    std::vector<Note> _live;
};

class Synthesizer
{
public:
    explicit Synthesizer(int frequency, int buffer_samples);

    int CalculateStreamRequiredSamples() const;
    int CalculateStreamDesiredSamplesPerFrame(int frequency, int framerate) const;

    float CalculateAbsoluteTime(int frequency) const;

    void Update(int samples);
    void Play(int samples);

    void Noise(int samples, float amplitude);
    void SineWave(int samples, float pitch, float amplitude);

    void ReadBuffer(float* output, int samples);

    void DrawBuffer(v2 scale, v3 color, float alpha);
    void DrawCursor(v2 scale, v3 color, float alpha);

private:
    FMODStream _stream;
    Tracker _tracker;

    int _frequency;
    unsigned int _cursor;
    std::vector<float> _buffer;
    std::vector<float> _scratch;
};

