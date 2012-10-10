//
// beat_detection.h
//

#include "main.h"

#include <complex>
#include <sound/MicrophoneRecorder.h>
#include <common/Timer.h>

using namespace funk;

void RegisterGmAudioLib(gmMachine* a_vm);

class ALSoundProcessing
    : public AL::ALSoundExtractor
{
public:
    ALSoundProcessing(boost::shared_ptr<AL::ALBroker> broker, std::string name);

    virtual ~ALSoundProcessing();

    // ALSoundExtractor interface
    void init();
    void process(const int& channels, const int& samples, const AL_SOUND_FORMAT* buffer, const AL::ALValue& timestamp);

    void ExtractChannel(int channel, std::vector<float>& results);

private:
    std::vector<float> _buffer;
    boost::shared_ptr<AL::ALMutex> _process_mutex;
};

// TODO: move to seperate file
class GMAudioStream
    : public HandledObj<GMAudioStream>
{
public:
	GM_BIND_TYPEID(GMAudioStream);

    GMAudioStream(const char* name, const char* ip, int port);
    ~GMAudioStream();

    void SetActive(bool active);

public:
    // now we need to change not to have a full buffer of input data
    // but a rolling buffer of source data at sample-frequency
    // 
    // the frequency is 1s of data
    // we can pull mic data out as fast as it comes
    // and buffer up to 1s or more
    
    // our update() needs to process at some framerate, 60hz for now
    // each update we need to pull out freq/60 samples of data
    // and we can FFT it and extract the notes for that frame
    
    // each frame we then have a block of FFT frequency
    // this is our old arbitrary window size but it is now time-based

    // our history will be 1s (that is, a 1x Frequency number of samples)

    // our history is already accurate -- it is window-size samples worth of analysed data

    // the difference will be our input data
    // adding input data will need to add a frame worth of data
    // and we need to remove it when processing is done

    void SetFrequency(int frequency);
    void SetFrameRate(int framerate);

    void SetFFTMagnifyScale(float scale);
    void SetFFTMagnifyPower(float power);

    int GetFFTWindowSize();

    void Update();

    void ClearInputDataFrame();
    
    void AddInputDataFrameSineWave(int frequency, float amplitude);
    void AddInputDataFrameMicrophone();
    void AddInputDataFrameRemoteNao();

    void UpdateMicrophoneBuffer();
    bool IsMicrophoneFrameBuffered();

    void CalcFrameDFT(int channel);
    void CalcFrameFFT(int channel);
    void CalcFrameAverageAndDifference(int channel);

    int CalcEstimatedBeatsPerSecond(int channel, int bin, float threshold);
    int CalcEstimatedBeatsPerSecondDiscrete(int channel, int bin, float threshold);
    int CalcEstimatedBeatsPerSecondAverage(int channel, int bin, float threshold);

    void DrawFrameRawWaveform(int channel, v3 color, float alpha);
    void DrawFrameFFTWaveform(int channel, v3 color, float alpha);
    void DrawFrameAverageWaveform(v3 color, float alpha);
    void DrawFrameDifferenceWaveform(v3 color, float alpha);

    void DrawFrameRawBars(int channel, v3 color, float alpha);
    void DrawFrameFFTBars(int channel, v3 color, float alpha);
    void DrawFrameAverageBars(v3 color, float alpha);
    void DrawFrameDifferenceBars(v3 color, float alpha);

    void NoteTuner(float threshold);

    int TestGetPianoNotes(float threshold, std::vector<int>& test_notes);

    float GetSecondsByFrame();
    float GetSecondsByMicrophone();
    float GetSecondsBySystemClock();

    void ResetTimers();

private:
    void TimePrint(const char* format, ...);

    void DrawWaveform(const std::vector<float>& channel, v2 scale, v3 color, float alpha);
    void DrawWaveform(const std::vector<std::complex<float> >& channel, v2 scale, v3 color, float alpha);

    void DrawBars(const std::vector<float>& channel, v2 scale, v3 color, float alpha);
    void DrawBars(const std::vector<std::complex<float> >& channel, v2 scale, v3 color, float alpha);

    void Subscribe();

    bool _active;
    boost::shared_ptr<ALSoundProcessing> _sound_processing;
    std::string _name;
    std::string _subscriber_id;
    std::string _ip;
    int _port;

    int _frequency;
    int _framerate;
    int _frame;
    int _microphone_samples_read;
    int _clock_start;
    funk::Timer _clock_timer;
    float _fft_magnify_scale;
    float _fft_magnify_power;

    struct Channel
    {
        std::vector<float> raw;
        std::vector<std::complex<float> > fft;
    };

    std::vector<Channel> _channels;

    int _average_index;
    std::vector<float> _average_fft;
    std::vector<float> _difference_fft;
    std::vector< std::vector<float> > _history_fft;
    std::vector< std::vector<float> > _history_difference_fft;

    StrongHandle<MicrophoneRecorder> _recorder;
    std::vector<signed char> _microphone_buffer;
};

GM_BIND_DECL(GMAudioStream);

