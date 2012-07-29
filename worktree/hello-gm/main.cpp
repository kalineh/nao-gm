/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <cstdlib>
#include <string>

#include "gm/gmMachine.h"
#include "gm/gmBind.h"

#include "Core.h"

#include "vision.h"
#include "gmalproxy.h"

using namespace funk;

void RegisterProjectLibs(gmMachine* vm)
{
	GM_BIND_INIT( GMALProxy, vm );
	GM_BIND_INIT( GMVideoDisplay, vm );
}

int main(int argc, char** argv)
{
	funk::Core app;

	app.HandleArgs(argc, argv);
	app.Init();
	app.Run();
	app.Deinit();

    return 0;
}
