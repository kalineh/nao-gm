#include "VirtualMachineLibs.h"

#include <gm/gmMachine.h>
#include <gm/gmBind.h>

#include <gm/gmSystemLib.h>
#include <gm/gmMathLib.h>
#include <gm/gmVector3Lib.h>
#include <gm/gmArrayLib.h>
#include <gm/gmStringLib.h>
#include <gm/gmInputLib.h>
#include <gm/gmGfxLib.h>
#include <gm/gmWindowLib.h>
#include <gm/gmDebug.h>
#include <gm/gmDebugLib.h>

#include <sound/Sound.h>
#include <sound/MicrophoneRecorder.h>
#include <sound/SoundRecorder.h>
#include <math/v2.h>
#include <math/v3.h>
#include <math/Perlin2d.h>
#include <math/CubicSpline2d.h>
#include <gfx/Cam2d.h>
#include <gfx/Cam3d.h>
#include <gfx/Texture.h>
#include <gfx/VertexShader.h>
#include <gfx/FragShader.h>
#include <gfx/ShaderProgram.h>
#include <gfx/Vbo.h>
#include <gfx/Fbo.h>
#include <gfx/Particles2D.h>
#include <gfx/Font.h>
#include <gfx/LineGraph.h>
#include <imgui/ImguiGM.h>

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
	gmBindInputLib(vm);
	gmBindWindowLib(vm);
	gmBindGfxLib(vm);
	gmBindImguiLib(vm);

	// Init common custom user types
	GM_BIND_INIT( Cam2d, vm );
	GM_BIND_INIT( Cam3d, vm );
	GM_BIND_INIT( Perlin2d, vm );
	GM_BIND_INIT( Sound, vm );
	GM_BIND_INIT( SoundRecorder, vm );
	GM_BIND_INIT( MicrophoneRecorder, vm );
	GM_BIND_INIT( Texture, vm );
	GM_BIND_INIT( VertexShader, vm );
	GM_BIND_INIT( FragShader, vm );
	GM_BIND_INIT( Vbo, vm );
	GM_BIND_INIT( Fbo, vm );
	GM_BIND_INIT( ShaderProgram, vm );
	GM_BIND_INIT( LineGraph, vm );
	GM_BIND_INIT( CubicSpline2d, vm );
	GM_BIND_INIT( Particles2d, vm );
	GM_BIND_INIT( Font, vm );

	// Init game-specific types
	RegisterProjectLibs( vm );
}
}