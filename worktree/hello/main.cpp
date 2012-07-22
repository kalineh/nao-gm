/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
#include <iostream>
#include <alcommon/alproxy.h>
#include <alproxies/altexttospeechproxy.h>

int main()
{
  std::cout << "Hello, world" << std::endl;

    AL::ALTextToSpeechProxy tts(std::string("192.168.11.9"), 9559);
    tts.say(std::string("test"));
  return 0;
}
