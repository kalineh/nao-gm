//
// beat_detection.h
//

#include "main.h"

#include <complex>
#include <sound/MicrophoneRecorder.h>

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
    void Update();

public:
    void ClearInputData();
    
    void AddInputDataSineWave(int frequency, float amplitude);
    void AddInputDataMicrophone();
    void AddInputDataRemoteNao();

    void SetFFTWindowSize(int samples);

    void CalcDFT(int channel);
    void CalcFFT(int channel);
    void CalcAverageAndDifference(int channel);

    void DrawRawWaveform(int channel, v3 color, float alpha);
    void DrawFFTWaveform(int channel, v3 color, float alpha);
    void DrawAverageWaveform(v3 color, float alpha);
    void DrawDifferenceWaveform(v3 color, float alpha);

    // TODO: estimated musical notes + beat confidence
    //std::vector<float> EstimateNotes(float fft_threshold);
    //float EstimateBeat(float fft_threshold);

private:
    void DrawWaveform(const std::vector<float>& channel, float scale, v3 color, float alpha);
    void DrawWaveform(const std::vector<std::complex<float> >& channel, float scale, v3 color, float alpha);

    void Subscribe();

    bool _active;
    boost::shared_ptr<ALSoundProcessing> _sound_processing;
    std::string _name;
    std::string _subscriber_id;
    std::string _ip;
    int _port;

    int _fft_window_size;

    struct Channel
    {
        std::vector<float> raw;
        std::vector<std::complex<float> > fft;
    };

    std::vector<Channel> _channels;

    int _average_index;
    std::vector<float> _average_fft;
    std::vector<float> _difference_fft;
    std::vector<float> _history_fft[60];

    StrongHandle<MicrophoneRecorder> _recorder;
    std::vector<float> _microphone_buffer;
};

GM_BIND_DECL(GMAudioStream);

