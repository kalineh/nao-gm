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

class Synthesizer
{
public:
    explicit Synthesizer(int frequency, int buffer_samples);

    void Update(int samples);
    void Play(int samples);

    void ReadBuffer(float* output, int samples);

    void SinWave(float frequency, float amplitude, int samples);
    void SquareWave(float frequency, float amplitude, int samples);
    void NoteWave(int note, int octave, float amplitude, int samples);

    float NoteHz(int note, int octave);

private:
    FMODStream _stream;

    int _frequency;
    unsigned int _cursor;
    std::vector<float> _buffer;
};

