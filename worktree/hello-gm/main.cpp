/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <cstdlib>

#include <alcommon/alproxy.h>

#include "gm/gmMachine.h"

void RegisterProjectLibs(gmMachine* vm)
{
}

int main(int argc, char** argv)
{
    std::string ip = "192.168.11.9";
    std::string port = "9559";

    if (argc > 1) ip = argv[1];
    if (argc > 2) port = argv[2];

    AL::ALProxy* proxy = new AL::ALProxy("ALTextToSpeech", ip, std::atoi(port.c_str()));

    proxy->callVoid("say", std::string("hello"));

    return 0;
}
