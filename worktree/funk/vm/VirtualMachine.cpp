#include "VirtualMachine.h"

#include <common/Util.h>
#include <common/Input.h>
#include <imgui/ImguiManager.h>

#include <gm/gmMachine.h>
#include <gm/gmThread.h>
#include <gm/gmDebuggerFunk.h>
#include <gm/gmUtilEx.h>
#include <imgui/Imgui.h>
#include <common/IniReader.h>
#include <common/Timer.h>
#include <common/Window.h>

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

	InitGuiSettings();
	InitGuiThreadAllocations();
}

VirtualMachine::~VirtualMachine()
{
	m_console.Log("Destructing Virtual Machine");
	if ( m_vm->GetDebugMode() ) m_debugger.Close();
	delete m_vm;
	m_console.Log("Virtual Machine destructed!");
}

void VirtualMachine::Update()
{
#ifndef FNK_FINAL
	if( Input::Get()->DidKeyJustGoDown("F5") ) ResetVM();
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
	m_console.ClearText();
	m_console.Log("Restarting Virtual Machine..." );

	if ( m_vm->GetDebugMode() ) m_debugger.Close();

	ImguiManager::Get()->ClearAllWindows();
	m_vm->ResetAndFreeMemory();
	m_vm->Init();

	if ( m_vm->GetDebugMode() ) m_debugger.Open(m_vm);

	RunMain();

	m_console.Log("Restarting Virtual Machine complete!");
}

void VirtualMachine::Render()
{
	if ( !m_drawManager.IsNull() )
	{
		// draw only if not debugging
		if ( !m_vm->GetDebugMode() || !m_debugger.IsDebugging() )
		{
			m_vm->GetGlobals()->Set( m_vm, "g_rendering", gmVariable(1) );
			m_vm->ExecuteFunction(m_drawFunc, 0, true, &m_drawManager);
			m_vm->GetGlobals()->Set( m_vm, "g_rendering", gmVariable(0) );		
		}
		else
		{
			m_vm->ExecuteFunction(m_clearFunc, 0, true, &m_drawManager);
		}
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
	m_console.Log(buffer);
	sprintf_s(buffer, "GC Works Per Increment: %d, GC Destructs Per Increment: %d", gcWorkPerIncrement, gcDestructsPerIncrement );
	m_console.Log(buffer);
	sprintf_s(buffer, "Mem Usage Soft: %d bytes, Mem Usage Hard: %d bytes", memUsageSoft, memUsageHard );
	m_console.Log(buffer);

	// attach debugger
	if ( m_vm->GetDebugMode() ) m_debugger.Open(m_vm);

	RegisterLibs(m_vm);

	// set dt
	m_vm->GetGlobals()->Set( m_vm, "g_dt", gmVariable(m_dt) );
	m_vm->GetGlobals()->Set( m_vm, "g_rendering", gmVariable(0) );

	m_threadId = gmCompileStr(m_vm, kEntryFile);	
	printf("'%s' Main thread: %d\n", kEntryFile, m_threadId );

	// get draw manager
	gmVariable drawManagerKey = gmVariable(m_vm->AllocStringObject("g_drawManager"));
	gmTableObject* drawManager = m_vm->GetGlobals()->Get(drawManagerKey).GetTableObjectSafe();
	m_drawManager.Nullify();

	if ( drawManager )
	{
		m_drawManager = gmVariable(gmVariable(drawManager));		

		// grab draw functions
		gmVariable drawKey = gmVariable(m_vm->AllocStringObject("Draw"));
		m_drawFunc = drawManager->Get(drawKey).GetFunctionObjectSafe();
		gmVariable clearKey = gmVariable(m_vm->AllocStringObject("Clear"));
		m_clearFunc = drawManager->Get(clearKey).GetFunctionObjectSafe();
	}
}

void VirtualMachine::HandleErrors()
{
	bool firstErr = true;

	gmLog & compileLog = m_vm->GetLog();
	const char *msg = compileLog.GetEntry(firstErr);
	
	if ( msg ) 
	{
		const char * textHeader = "#############################\n[GameMonkey Run-time Error]:";
		m_console.Log(textHeader);
	}

	while(msg)	
	{
		m_console.Log(msg, false);
		msg = compileLog.GetEntry(firstErr);
	}
	compileLog.Reset();
}

void VirtualMachine::GuiStats()
{
	Imgui::Header("GameMonkey");
	Imgui::FillBarFloat("Update", m_updateMs, 0.0f, 16.0 );
	Imgui::FillBarInt("Mem Usage (Bytes)", m_vm->GetCurrentMemoryUsage(), 0, m_vm->GetDesiredByteMemoryUsageHard() );
	Imgui::FillBarInt("Num Threads", m_numThreads, 0, 500 );
	Imgui::CheckBox("Show Settings", m_showSettingsGui );
	Imgui::CheckBox("Show Allocations", m_showThreadAllocationsGui );
}

void VirtualMachine::Gui()
{
	if ( m_vm->GetDebugMode() ) m_debugger.Gui();
	if ( m_showSettingsGui ) GuiSettings();
	if ( m_showThreadAllocationsGui ) GuiThreadAllocations();
	m_console.Gui();
}

void VirtualMachine::InitGuiSettings()
{
	m_showSettingsGui = false;

	const float minVal = 0.0f;
	const float maxVal = 16.0f;
	const int width = 200;
	const int height = 100;
	const int numVals = 128;

	m_lineGraphUpdate = new LineGraph( minVal, maxVal, v2i(width, height), numVals );
	m_lineGraphMemory = new LineGraph( minVal, maxVal, v2i(width, height), numVals );
}

void VirtualMachine::GuiSettings()
{
	m_lineGraphUpdate->PushVal( m_updateMs );
	m_lineGraphUpdate->SetMaxVal(m_dt*1000.0f);
	m_lineGraphMemory->SetMaxVal( (float)m_vm->GetDesiredByteMemoryUsageHard() );
	m_lineGraphMemory->PushVal( (float)m_vm->GetCurrentMemoryUsage() );

	int workPerIncrement = m_vm->GetGC()->GetWorkPerIncrement();
	int destructPerIncrement = m_vm->GetGC()->GetDestructPerIncrement();
	int memUsageSoft = m_vm->GetDesiredByteMemoryUsageSoft();
	int memUsageHard = m_vm->GetDesiredByteMemoryUsageHard();

	const v2i pos = v2i(300, Window::Get()->Sizei().y - 20 );

	Imgui::Begin("GameMonkey Settings", pos);
	Imgui::Print("Update");
	Imgui::LineGraph( m_lineGraphUpdate );
	Imgui::Print("Memory");
	Imgui::LineGraph( m_lineGraphMemory );
	Imgui::FillBarInt("Mem Usage (Bytes)", m_vm->GetCurrentMemoryUsage(), 0, m_vm->GetDesiredByteMemoryUsageHard() );
	Imgui::Header("Garbage Collector");
	Imgui::SliderInt( "Work Per Increment", workPerIncrement, 1, 600 );
	Imgui::SliderInt( "Destructs Per Increment", destructPerIncrement, 1, 600 );
	Imgui::SliderInt( "Mem Usage Soft", memUsageSoft, 200000, memUsageHard );
	Imgui::SliderInt( "Mem Usage Hard", memUsageHard, memUsageSoft+500, memUsageSoft+200000 );
	Imgui::Separator();
	Imgui::FillBarInt("GC Warnings", m_vm->GetStatsGCNumWarnings(), 0, 200 );
	Imgui::FillBarInt("GC Full Collects", m_vm->GetStatsGCNumFullCollects(), 0, 200 );
	Imgui::FillBarInt("GC Inc Collects", m_vm->GetStatsGCNumIncCollects(), 0, 200 );
	Imgui::End();

	m_vm->SetDesiredByteMemoryUsageSoft(memUsageSoft);
	m_vm->SetDesiredByteMemoryUsageHard(memUsageHard);
	m_vm->GetGC()->SetWorkPerIncrement(workPerIncrement);
	m_vm->GetGC()->SetDestructPerIncrement(destructPerIncrement);
}

void VirtualMachine::InitGuiThreadAllocations()
{
	m_showThreadAllocationsGui = false;
	m_freezeThreadAllocationsGui = false;
}

void VirtualMachine::GuiThreadAllocations()
{
	const v2i pos = v2i(300, Window::Get()->Sizei().y - 20 );

	Imgui::Begin("Thread Allocations (Live)", pos);
	m_freezeThreadAllocationsGui = Imgui::CheckBox("Freeze", m_freezeThreadAllocationsGui);

	if (!m_freezeThreadAllocationsGui)
	{
		std::map<const gmFunctionObject*, ThreadAllocationItem>::iterator i = m_threadAllocationsHistory.begin();
		while (i != m_threadAllocationsHistory.end())
		{
			i->second.eraseCountdown -= 1;

			if (i->second.eraseCountdown <= 0)
			{
				i = m_threadAllocationsHistory.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	if (!m_freezeThreadAllocationsGui)
	{
		std::map<const gmFunctionObject*, int>::const_iterator a = m_vm->GetAllocCountIteratorBegin();
		std::map<const gmFunctionObject*, int>::const_iterator b = m_vm->GetAllocCountIteratorEnd();
		for (; a != b; ++a)
		{
			ThreadAllocationItem& item = m_threadAllocationsHistory[a->first];
			item.allocations = a->second;
			item.eraseCountdown = 300;
		}
	}

	{
		std::map<const gmFunctionObject*, ThreadAllocationItem>::const_iterator a = m_threadAllocationsHistory.begin();
		std::map<const gmFunctionObject*, ThreadAllocationItem>::const_iterator b = m_threadAllocationsHistory.end();
		for (; a != b; ++a)
		{
			const gmFunctionObject* func = a->first;
			const gmuint32 sourceid = func->GetSourceId();
			const char* sourcecode = NULL;
			const char* filename = NULL;
			m_vm->GetSourceCode(sourceid, sourcecode, filename);

			int allocations = a->second.allocations;
			if (allocations <= 0)
				continue;

			char buffer[128] = { '.' };
			const char* fname = func->GetDebugName();
			const char* status = a->second.eraseCountdown == 300 ? "new" : "old";
			sprintf(buffer, "%s:%s [%s]", filename, fname, status);
			int len = strlen(buffer);
			memset( &buffer[len], '.', sizeof(buffer)-len-1);
			itoa(allocations, buffer+50, 10);

			Imgui::Print(buffer);
		}
	}

	Imgui::End();
}

}