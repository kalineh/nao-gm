//
// beat_detection.h
//

#include "main.h"

#include <complex>

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

    void SetActive(bool active);
    void Update();

public:
    void GetRemoteAudioData();

    void ClearSineWave();
    void AddSineWave(int frequency);

    void CalcBeatDFT(int channel);
    void CalcBeatFFT(int channel);

    void CalcAverageEnergies();

    void DrawWaveform(int channel, v3 color, float aplha);
    void DrawBeatWaveform(int channel, v3 color, float aplha);
    void DrawEnergyDifferenceWaveform(v3 color, float aplha);

private:
    typedef std::vector<float> Channel;
    typedef std::vector<Channel> Data;
    typedef std::vector<std::complex<float> > ComplexChannel;

    void DrawWaveformImpl(const Channel& channel, float scale, v3 color, float alpha);
    void Subscribe();

    bool _active;
    boost::shared_ptr<ALSoundProcessing> _sound_processing;
    std::string _name;
    std::string _subscriber_id;
    std::string _ip;
    int _port;

    float _rolling_ft_min;
    float _rolling_ft_max;
    
    Data _data;
    ComplexChannel _transformed;

    // TODO: cleaner
    float _last_phase_min;
    float _last_phase_max;

    Data _energy_history;
    std::vector<float> _average_energy;
    int _average_energy_index;
    std::vector<float> _energy_differences;
};

GM_BIND_DECL(GMAudioStream);

