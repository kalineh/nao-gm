#ifndef _INCLUDE_VIRTUAL_MACHINE_H
#define _INCLUDE_VIRTUAL_MACHINE_H

#include <common/Singleton.h>
#include <common/StrongHandle.h>

#include <gm/gmVariable.h>
#include <gm/gmDebuggerFunk.h>
#include <gfx/LineGraph.h>

#include <map>

#include "VirtualConsole.h"

class gmMachine;

namespace funk
{
	class VirtualMachine : public Singleton<VirtualMachine>
	{
	public:
		void Update();
		void Render();
		void RunMain();
		void GuiStats();
		void Gui();

		gmMachine & GetVM () { return *m_vm; }
		VirtualConsole &GetConsole() { return m_console; }

		bool IsUsingByteCode() const { return m_bUseGmByteCode; }

	private:
		float	m_updateMs;
		float	m_dt;
		bool	m_bUseGmByteCode; // uses bytecode version
		int		m_numThreads;

		gmMachine *m_vm;
		int m_threadId;

		// draw manager
		gmFunctionObject* m_drawFunc;
		gmFunctionObject* m_clearFunc;
		gmVariable m_drawManager;
		gmDebuggerFunk m_debugger;

		// settings gui
		bool m_showSettingsGui;
		StrongHandle<LineGraph> m_lineGraphUpdate;
		StrongHandle<LineGraph> m_lineGraphMemory;
		void InitGuiSettings();
		void GuiSettings();

		// thread allocations gui
		bool m_showThreadAllocationsGui;
		void InitGuiThreadAllocations();
		void GuiThreadAllocations();

		struct ThreadAllocationItem
		{
			int allocations;
			int eraseCountdown;
		};
		std::map<const gmFunctionObject*, ThreadAllocationItem> m_threadAllocationsHistory;
		bool m_freezeThreadAllocationsGui;

		VirtualConsole m_console;

		void HandleErrors();
		void ResetVM();

		friend Singleton<VirtualMachine>;
		VirtualMachine();
		~VirtualMachine();
	};
}

#endif