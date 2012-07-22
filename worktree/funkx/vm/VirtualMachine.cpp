#include "VirtualMachine.h"

#include <common/Util.h>

#include <gm/gmMachine.h>
#include <gm/gmThread.h>
#include <gm/gmDebuggerFunk.h>
#include <gm/gmUtilEx.h>
#include <common/IniReader.h>
#include <common/Timer.h>

#include <map>

#include "VirtualMachineLibs.h"

namespace funk
{
const char * kEntryFile = "common/gm/Core.gm";

VirtualMachine::VirtualMachine()
{
	m_vm = new gmMachine();
	m_vm->SetAutoMemoryUsage(false);

	m_dt = 0.0f;
	m_updateMs = 0.0f;
	m_bUseGmByteCode = false;
	m_numThreads = 0;
	m_threadId = 0;
}

VirtualMachine::~VirtualMachine()
{
	printf("Destructing Virtual Machine");
	if ( m_vm->GetDebugMode() ) m_debugger.Close();
	delete m_vm;
	printf("Virtual Machine destructed!");
}

void VirtualMachine::Update()
{
#ifndef FNK_FINAL
	//if( Input::Get()->DidKeyJustGoDown("F5") ) ResetVM();
#endif
	
	HandleErrors();

	// run main game only if not debugging
	if ( !(m_vm->GetDebugMode() && m_debugger.IsDebugging()) )
	{
		Timer gmTimer;
		gmuint32 delta = (gmuint32)(m_dt*1000.0f);
		m_numThreads = m_vm->Execute( delta );
		m_updateMs = gmTimer.GetTimeMs();
	}

	// update debugger
	if ( m_vm->GetDebugMode() ) 
	{
		m_debugger.Update();
	}
}

void VirtualMachine::ResetVM()
{
	printf("Restarting Virtual Machine..." );

	if ( m_vm->GetDebugMode() ) m_debugger.Close();

	m_vm->ResetAndFreeMemory();
	m_vm->Init();

	if ( m_vm->GetDebugMode() ) m_debugger.Open(m_vm);

	RunMain();

	printf("Restarting Virtual Machine complete!");
}

void VirtualMachine::Tick()
{
	// tick only if not debugging
	if ( !m_vm->GetDebugMode() || !m_debugger.IsDebugging() )
	{
		m_vm->ExecuteFunction(m_tickFunc, 0, true, NULL);
	}
}

void VirtualMachine::RunMain()
{
	IniReader ini("common/ini/main.ini");

	int debugMode = 1;
	int runGmLibs = 0;
#ifdef FNK_FINAL
	debugMode = 0;
	runGmLibs = 1;
#endif

	int fps = ini.GetInt("Window", "FPS");
	int gcWorkPerIncrement = ini.GetInt("VirtualMachine", "GC_WorkPerIncrement");
	int gcDestructsPerIncrement = ini.GetInt("VirtualMachine", "GC_DestructPerIncrement");
	int memUsageSoft = ini.GetInt("VirtualMachine", "MemUsageSoft");
	int memUsageHard = ini.GetInt("VirtualMachine", "MemUsageHard");

	m_vm->GetGC()->SetWorkPerIncrement(gcWorkPerIncrement);
	m_vm->GetGC()->SetDestructPerIncrement(gcDestructsPerIncrement);
	m_vm->SetDesiredByteMemoryUsageSoft(memUsageSoft);
	m_vm->SetDesiredByteMemoryUsageHard(memUsageHard);

	m_vm->SetDebugMode(debugMode == 1);
	m_bUseGmByteCode = runGmLibs == 1;
	m_dt = 1.0f/fps;

	// print current setting
	char buffer[128];
	sprintf_s(buffer, "Running at %d hz, VM Debug Mode: %d, VM Run Byte-Code: %d", fps, debugMode, runGmLibs );
	printf(buffer);
	sprintf_s(buffer, "GC Works Per Increment: %d, GC Destructs Per Increment: %d", gcWorkPerIncrement, gcDestructsPerIncrement );
	printf(buffer);
	sprintf_s(buffer, "Mem Usage Soft: %d bytes, Mem Usage Hard: %d bytes", memUsageSoft, memUsageHard );
	printf(buffer);

	// attach debugger
	if ( m_vm->GetDebugMode() ) m_debugger.Open(m_vm);

	RegisterLibs(m_vm);

	// set dt
	m_vm->GetGlobals()->Set( m_vm, "g_dt", gmVariable(m_dt) );
	m_tickFunc = m_vm->GetGlobals()->Get(m_vm, "TickFunction").GetFunctionObjectSafe();

	m_threadId = gmCompileStr(m_vm, kEntryFile);	
	printf("'%s' Main thread: %d\n", kEntryFile, m_threadId );
}

void VirtualMachine::HandleErrors()
{
	bool firstErr = true;

	gmLog & compileLog = m_vm->GetLog();
	const char *msg = compileLog.GetEntry(firstErr);
	
	if ( msg ) 
	{
		const char * textHeader = "#############################\n[GameMonkey Run-time Error]:";
		printf(textHeader);
	}

	while(msg)	
	{
		printf(msg, false);
		msg = compileLog.GetEntry(firstErr);
	}
	compileLog.Reset();
}

}