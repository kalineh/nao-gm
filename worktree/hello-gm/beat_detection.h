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

    void CalcBeatFFT();
    void CalcBeatEnergy();

    void DrawWaveform(int channel, v3 color, float aplha);

private:
    void Subscribe();

    bool _active;
    boost::shared_ptr<ALSoundProcessing> _sound_processing;
    std::string _name;
    std::string _subscriber_id;

    typedef std::vector<float> Channel;
    typedef std::vector<Channel> Data;
    
    Data _data;
};

GM_BIND_DECL(GMAudioStream);

