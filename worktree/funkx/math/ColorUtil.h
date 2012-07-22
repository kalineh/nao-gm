#ifndef _INCLUDE_COLOR_UTIL_H
#define _INCLUDE_COLOR_UTIL_H

#include "v3.h"

namespace funk
{
	v3 HSVtoRGB( v3 HSV ); // Hue in radians
	v3 RGBtoHSV( v3 RGB ); // Hue in radians
}
#endif