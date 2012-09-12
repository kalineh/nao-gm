//
// main.h
//

#pragma once
#ifndef _MAIN_H
#define _MAIN_H

// clib

#include <cstdlib>
#include <string>
#include <vector>
#include <math.h>

// gm

#include "gm/gmThread.h"
#include "gm/gmVariable.h"
#include "gm/gmMachine.h"
#include "gm/gmBindHeader.h"
#include "gm/gmBind.h"

// AL

#include <alcommon/alproxy.h>
#include <alcommon/albroker.h>
#include <alcommon/albrokermanager.h>
#include <althread/almutex.h>
#include <althread/alcriticalsection.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alproxies/alsonarproxy.h>
#include <alproxies/almemoryproxy.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alproxies/alsonarproxy.h>
#include <alvision/alvisiondefinitions.h>
#include <almemoryfastaccess/almemoryfastaccess.h>
#include <alaudio/alsoundextractor.h>

// glm

//#define GLM_MESSAGES
#define GLM_FORCE_INLINE
#define GLM_FORCE_SSE2

#include <glm/glm.hpp>
#include <glm/gtx/simd_vec4.hpp>
#include <glm/gtc/swizzle.hpp>

// funk

#include "Core.h"
#include <common/HandledObj.h>
#include <common/StrongHandle.h>
#include <common/Debug.h>
#include <common/StrongHandle.h>
#include <gfx/Texture.h>
#include <vm/VirtualMachine.h>

// common

#include "gmalproxy.h"

#endif // _MAIN_H