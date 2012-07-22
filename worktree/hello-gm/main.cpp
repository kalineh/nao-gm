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

            try
            {
                memory.unsubscribeToEvent("SpeechDetected", getName());
            }
            catch (const AL::ALError&)
            {
                // ignore
            }
            try
            {
                memory.unsubscribeToEvent("WordRecognized", getName());
            }
            catch (const AL::ALError&)
            {
                // ignore
            }

            // force an un-subscribe in case we are already on (this stops the ASR)
            //memory.unsubscribeToEvent("WordRecognized", getName());
            //recognition.unsubscribe(getName());

            // set("EarSpeed"): 0..3; where 0 = slow accurate, 3 = fast inaccurate

            recognition.setAudioExpression(true);
            recognition.setVisualExpression(true);

            std::vector<std::string> vocabulary;

            vocabulary.push_back("open");
            vocabulary.push_back("close");

            recognition.setVocabulary(vocabulary, true);
            recognition.setLanguage("English");

            memory.subscribeToEvent("WordRecognized", getName(), "onWordRecognized");
            memory.subscribeToEvent("SpeechDetected", getName(), "onWordRecognized");

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

    void onWordRecognized(const std::string& name, const AL::ALValue& value, const std::string& myname)
    {
        try
        {
            AL::ALCriticalSection section(mutex);
            AL::ALValue v = memory.getData("WordRecognized");

            speech.say("word");
            //speech.say(v.toString());
        }
        catch (const AL::ALError& e)
        {
            qiLogError("onWordRecognized") << e.what() << std::endl;
        }
    }

private:
    AL::ALMemoryProxy memory;
    AL::ALTextToSpeechProxy speech;
    AL::ALSpeechRecognitionProxy recognition;

    boost::shared_ptr<AL::ALMutex> mutex;
};

void test()
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

        static bool finish = false;
        while (!finish)
        {
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
