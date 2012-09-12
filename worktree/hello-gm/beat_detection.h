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

private:
    void Subscribe();

    void GetRemoteAudioData();

    void BeatFFT();
    void BeatEnergy();

    bool _active;
    boost::shared_ptr<ALSoundProcessing> _sound_processing;
    std::string _name;
    std::string _subscriber_id;

    typedef std::vector<unsigned char> Channel;
    typedef std::vector<Channel> Data;
    
    Data _data;
};

GM_BIND_DECL(GMAudioStream);

