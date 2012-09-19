//
// beat_detection.h
//

#include "main.h"

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
    void GenerateSineWave(int frequency);

    void CalcBeatFFT(int channel);
    void CalcBeatEnergy(int channel);

    void DrawWaveform(int channel, v3 color, float aplha);
    void DrawBeatWaveform(int channel, v3 color, float aplha);

private:
    typedef std::vector<float> Channel;
    typedef std::vector<Channel> Data;

    void DrawWaveformImpl(const Channel& channel, v3 color, float alpha);
    void Subscribe();

    bool _active;
    boost::shared_ptr<ALSoundProcessing> _sound_processing;
    std::string _name;
    std::string _subscriber_id;
    std::string _ip;
    int _port;
    
    Data _data;
    Channel _transformed;
};

GM_BIND_DECL(GMAudioStream);

