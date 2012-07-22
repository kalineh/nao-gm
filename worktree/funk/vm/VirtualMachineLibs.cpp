#include "VirtualMachineLibs.h"

#include <gm/gmMachine.h>
#include <gm/gmBind.h>

#include <gm/gmSystemLib.h>
#include <gm/gmMathLib.h>
#include <gm/gmVector3Lib.h>
#include <gm/gmArrayLib.h>
#include <gm/gmStringLib.h>
#include <gm/gmDebug.h>
#include <gm/gmDebugLib.h>

#include <math/v2.h>
#include <math/v3.h>
#include <math/Perlin2d.h>
#include <math/CubicSpline2d.h>

extern void RegisterProjectLibs(gmMachine* vm);

namespace funk
{

void RegisterLibs( gmMachine * vm )
{
	// Init libraries
	gmBindDebugLib(vm);
	gmBindFunkDebugLib(vm);
	gmBindSystemLib(vm);
	gmBindMathLib(vm);
	gmBindArrayLib(vm);
	gmBindStringLib(vm);

	// Init common custom user types
	GM_BIND_INIT( Perlin2d, vm );

	// Init game-specific types
	RegisterProjectLibs(vm);
}

}