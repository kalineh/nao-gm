#include "gmDebugLib.h"

#include "gmThread.h"
#include "gmMachine.h"

#include <gm/gmBind.h>

#include <vm/VirtualMachine.h>

namespace funk
{
struct gmfDebugLib
{
	static int GM_CDECL gmfBreak(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);

		 gmDebugSession * session = (gmDebugSession *)VirtualMachine::Get()->GetVM().m_debugUser;

		 if ( session )
		 {
			 session->m_owner->BreakIntoRunningThread();
		 }

		return GM_OK;
	}
};

static gmFunctionEntry s_gmDebugLib[] = 
{
	GM_LIBFUNC_ENTRY(Break, Debug)
};

void gmBindFunkDebugLib( gmMachine * a_machine )
{
	a_machine->RegisterLibrary(s_gmDebugLib, sizeof(s_gmDebugLib) / sizeof(s_gmDebugLib[0]), "Debug" );
}

}