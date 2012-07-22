/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <cstdlib>

#include <string>

#include <alcommon/almodule.h>
#include <alcommon/alproxy.h>
#include <alcommon/albroker.h>
#include <alcommon/albrokermanager.h>

#include <alvalue/alvalue.h>
#include <althread/almutex.h>
#include <althread/alcriticalsection.h>

#include <alproxies/almotionproxy.h>
#include <alproxies/altexttospeechproxy.h>
#include <alproxies/alspeechrecognitionproxy.h>
#include <alproxies/almemoryproxy.h>

#include <qi/log.hpp>
#include <qi/os.hpp>

#include "gm/gmMachine.h"


// no exception mode
//#define MODULE_ERROR(description) do { printf(description); qi::os::exit(1); } while (0)

#define MODULE_ERROR(description) do { throw AL::ALError(getName(), __FUNCTION__, description, __FILE__, __LINE__); } while (0)

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))


void RegisterProjectLibs(gmMachine* vm)
{
}

void ListenForSound()
{
}

class HandVoiceControl
    : public AL::ALModule
{
public:
    HandVoiceControl(AL::ALBroker::Ptr broker, const std::string& name)
        : AL::ALModule(broker, std::string("HandVoiceControl"))
        , mutex(AL::ALMutex::createALMutex())
    {
        setModuleDescription("Control hand open/close with voice commands.");

        functionName("onWordRecognized", getName(), "Handle voice command notifications.");
        BIND_METHOD(HandVoiceControl::onWordRecognized);
    }

    virtual ~HandVoiceControl()
    {
    }

    void startRecognition()
    {
        try
        {
            memory = AL::ALMemoryProxy(getParentBroker());
            speech = AL::ALTextToSpeechProxy(getParentBroker());
            recognition = AL::ALSpeechRecognitionProxy(getParentBroker());

#define IGNORE_EXCEPTION(code) try { code ; } catch (...) { }
               
            IGNORE_EXCEPTION((memory.unsubscribeToEvent("SpeechDetected", getName())));
            IGNORE_EXCEPTION((memory.unsubscribeToEvent("WordRecognized", getName())));

#undef IGNORE_EXCEPTION

            // force an un-subscribe in case we are already on (this stops the ASR)
            //memory.unsubscribeToEvent("WordRecognized", getName());
            //recognition.unsubscribe(getName());

            recognition.setAudioExpression(true);
            recognition.setVisualExpression(true);
            //recognition.setParameter("EarSpeed", 0.0f);
            //recognition.setParameter("EarUseSpeechDetector", 2.0f);
            //recognition.setParameter("EarUseFilter", 1.0f);

            std::vector<std::string> vocabulary;

            vocabulary.push_back("open");
            vocabulary.push_back("close");

            recognition.setVocabulary(vocabulary, true);
            recognition.setLanguage("English");

            memory.subscribeToEvent("WordRecognized", getName(), "onWordRecognized");
            //memory.subscribeToEvent("SpeechDetected", getName(), "onWordRecognized");

            speech.say("started listening");
        }
        catch (const AL::ALError& e)
        {
            qiLogError("HandVoiceControl") << e.what() << std::endl;
        }
    }

    void stopRecognition()
    {
        speech.say("stopped listening");
        memory.unsubscribeToEvent("onWordRecognized", "HandleVoiceControl");
    }

    virtual void init()
    {
    }

    struct WordConfidencePair
    {
        std::string word;
        float confidence;
    };

    struct WordConfidencePairComparison
    {
        bool operator()(const WordConfidencePair& lhs, const WordConfidencePair& rhs)
        {
            return lhs.confidence < rhs.confidence;
        }
    };

    void ParseWordRecognizedArray(std::vector<WordConfidencePair>& results, const AL::ALValue& value, float recognitionThreshold)
    {
        // unexpected data
        if (value.getType() != AL::ALValue::TypeArray)
            MODULE_ERROR("invalid array type");

        // unexpected data
        if (value.getSize() % 2 != 0)
            MODULE_ERROR("invalid array size");

        for (int i = 0; i < (int)value.getSize(); i += 2)
        {
            AL::ALValue wordValue = value[i];
            AL::ALValue confidenceValue = value[i + 1];

            float confidence = confidenceValue.operator float &();
            if (confidence >= recognitionThreshold)
            {
                WordConfidencePair pair = { wordValue.toString(), confidence };
                results.push_back(pair);
            }
        }

        std::sort(results.begin(), results.end(), WordConfidencePairComparison());
    }

    void onWordRecognized(const std::string& name, const AL::ALValue& value, const std::string& myname)
    {
        try
        {
            AL::ALCriticalSection section(mutex);
            AL::ALValue v = memory.getData("WordRecognized");

            std::vector<WordConfidencePair> words;
            ParseWordRecognizedArray(words, value, 0.5f);

            if (words.size() > 0)
            {
                speech.say(words[0].word);
            }

            //speech.say("word");
            //speech.say(v.toString());
        }
        catch (const AL::ALError& e)
        {
            qiLogError("onWordRecognized") << e.what() << std::endl;
        }

        // this gets called a lot, try and suppress excessive calls
        qi::os::msleep(2000);
    }

private:
    AL::ALMemoryProxy memory;
    AL::ALTextToSpeechProxy speech;
    AL::ALSpeechRecognitionProxy recognition;

    boost::shared_ptr<AL::ALMutex> mutex;
};

class Diagnostics
    : public AL::ALModule
{
public:
    Diagnostics(AL::ALBroker::Ptr broker, const std::string& name)
        : AL::ALModule(broker, std::string("Diagnostics"))
        , mutex(AL::ALMutex::createALMutex())
    {
        setModuleDescription("Report various diagnostics.");
    }

    struct BodyPartDescriptor
    {
        const char* name;
        const char* desc;
    };

    static const BodyPartDescriptor BodyParts[26];

    int getBodyPartCount()
    {
        return sizeof(BodyParts) / sizeof(BodyParts[0]);
    }

    void reportAllBodyParts()
    {
        for (int i = 0; i < getBodyPartCount(); ++i)
        {
            reportBodyPartName(i);
        }
    }

    void reportStatus()
    {
        // TODO: is there overhead in creating one of these every time?
        AL::ALMotionProxy motion(getParentBroker());
        AL::ALTextToSpeechProxy speech(getParentBroker());
        speech.setLanguage("English");
        std::string summary = motion.getSummary();
        //speech.say(motion.getSummary());
    }

    void reportBodyPartName(int index)
    {
        AL::ALCriticalSection section(mutex);

        BodyPartDescriptor part = Diagnostics::BodyParts[index];

        // TODO: is there overhead in creating one of these every time?
        AL::ALTextToSpeechProxy speech(getParentBroker());
        speech.setLanguage("English");
        speech.say(std::string(part.desc));
    }

    void reportTemperatures(float thresholdDegrees = 40.0f)
    {
        for (int i = 0; i < getBodyPartCount(); ++i)
        {
        }
    }

    virtual void init()
    {
    }

    boost::shared_ptr<AL::ALMutex> mutex;
};

const Diagnostics::BodyPartDescriptor Diagnostics::BodyParts[26] = { 
    { "HeadYaw", "Head Yaw", },
    { "LShoulderPitch", "Left Shoulder Pitch", },
    { "LHipYawPitch", "Left Hip Yaw Pitch", },
    { "RHipYawPitch", "Right Hip Yaw Pitch", },
    { "RShoulderPitch", "Right Shoulder Pitch", },
    { "HeadPitch", "Head Pitch", },
    { "LShoulderRoll", "Left Shoulder Roll", },
    { "LHipRoll", "Left Hip Roll", },
    { "RHipRoll", "Right Hip Roll", },
    { "RShoulderRoll", "Right Shoulder Roll", },
    { "LElbowYaw", "Left Elbow Yaw", },
    { "LHipPitch", "Left Hip Pitch", },
    { "RHipPitch", "Right Hip Pitch", },
    { "RElbowYaw", "Right Elbow Yaw", },
    { "LElbowRoll", "Left Elbow Roll", },
    { "LKneePitch", "Left Knee Pitch", },
    { "RKneePitch", "Right Knee Pitch", },
    { "RElbowRoll", "Right Elbow Roll", },
    { "LWristYaw", "Left Wrist Yaw", },
    { "LAnklePitch", "Left Ankle Pitch", },
    { "RAnklePitch", "Right Ankle Pitch", },
    { "RWristYaw", "Right Wrist Yaw", },
    { "LHand", "Left Hand", },
    { "RAnkleRoll", "Right Ankle Roll", },
    { "LAnkleRoll", "Left Angle Roll", },
    { "RHand", "Rigth Hand", },
};

void randomtest()
{
    std::string ip = "192.168.11.9";
    std::string port = "9559";

    int portn = std::atoi(port.c_str());

    AL::ALProxy tts("ALTextToSpeech", ip, portn);
    tts.callVoid("say", std::string("hello"));

    AL::ALSpeechRecognitionProxy sr(ip, portn);

    sr.setAudioExpression(true);
    sr.setVisualExpression(true);
    sr.setLanguage("English");

    AL::ALMotionProxy motion(ip, portn);
    int a = motion.post.openHand("RHand");
    tts.callVoid("say", std::string("opening"));
    int b = motion.post.closeHand("RHand");
    tts.callVoid("say", std::string("closing"));

    while (true)
    {
    }
}

class Bumper : public AL::ALModule
{
  public:
    Bumper(boost::shared_ptr<AL::ALBroker> broker, const std::string& name);
    virtual ~Bumper();
    virtual void init();
    void onRightBumperPressed();

  private:
    AL::ALMemoryProxy fMemoryProxy;
    AL::ALTextToSpeechProxy fTtsProxy;

    boost::shared_ptr<AL::ALMutex> fCallbackMutex;
    float fState;
};

Bumper::Bumper(
  boost::shared_ptr<AL::ALBroker> broker,
  const std::string& name): AL::ALModule(broker, name),
    fCallbackMutex(AL::ALMutex::createALMutex())
{
  setModuleDescription("This module presents how to subscribe to a simple event (here RightBumperPressed) and use a callback method.");

  functionName("onRightBumperPressed", getName(), "Method called when the right bumper is pressed. Makes a LED animation.");
  BIND_METHOD(Bumper::onRightBumperPressed)
}

Bumper::~Bumper() {
  fMemoryProxy.unsubscribeToEvent("onRightBumperPressed", "Bumper");
}

void Bumper::init() {
  try {
    /** Create a proxy to ALMemory.
    */
    fMemoryProxy = AL::ALMemoryProxy(getParentBroker());

    fState = fMemoryProxy.getData("RightBumperPressed");
    /** Subscribe to event LeftBumperPressed
    * Arguments:
    * - name of the event
    * - name of the module to be called for the callback
    * - name of the bound method to be called on event
    */
    fMemoryProxy.subscribeToEvent("RightBumperPressed", "Bumper",
                                  "onRightBumperPressed");
  }
  catch (const AL::ALError& e) {
    qiLogError("module.example") << e.what() << std::endl;
  }
}

void Bumper::onRightBumperPressed() {
  qiLogInfo("module.example") << "Executing callback method on right bumper event" << std::endl;
  /**
  * As long as this is defined, the code is thread-safe.
  */
  AL::ALCriticalSection section(fCallbackMutex);

  /**
  * Check that the bumper is pressed.
  */
  fState =  fMemoryProxy.getData("RightBumperPressed");
  if (fState  > 0.5f) {
    return;
  }
  try {
    fTtsProxy = AL::ALTextToSpeechProxy(getParentBroker());
    fTtsProxy.say("Right bumper pressed");
  }
  catch (const AL::ALError& e) {
    qiLogError("module.example") << e.what() << std::endl;
  }
}

int main(int argc, char** argv)
{
    std::string ip = "192.168.11.9";
    std::string port = "9559";

    if (argc > 1) ip = argv[1];
    if (argc > 2) port = argv[2];

    int portn = std::atoi(port.c_str());

    AL::ALBrokerManager::getInstance()->killAllBroker();
    AL::ALBroker::Ptr broker = AL::ALBroker::createBroker("main", "0.0.0.0", 54000, ip, portn);

    try
    {
        //AL::ALModule::createModule<Bumper>(broker, "Bumper");
        auto hvc = AL::ALModule::createModule<HandVoiceControl>(broker, "HandVoiceControl");

        //HandVoiceControl hvc(broker, "HandVoiceControl");
        hvc->startRecognition();

        AL::ALMotionProxy motion(broker);

        auto d = AL::ALModule::createModule<Diagnostics>(broker, "Diagnostics");
        d->reportStatus();

        static bool finish = false;
        while (!finish)
        {
            motion.openHand("RHand");
            motion.setStiffnesses("HeadYaw", 0.1f);
            motion.angleInterpolation("HeadYaw", 0.2f, 1.0f, true);
            qi::os::msleep(1000);
            motion.closeHand("RHand");
            motion.angleInterpolation("HeadYaw", 0.0f, 1.0f, true);
            qi::os::msleep(1000);
        }

        hvc->stopRecognition();
    }
    catch (const AL::ALError& e)
    {
        qiLogError("error") << e.what() << std::endl;
    }

    //AL::ALMotionProxy motion(ip, portn);
    //int a = motion.post.openHand("RHand");
    //tts.callVoid("say", std::string("opening"));
    //int b = motion.post.closeHand("RHand");
    //tts.callVoid("say", std::string("closing"));

    return 0;
}
