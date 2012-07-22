#ifndef _INCLUDE_VIRTUAL_MACHINE_H
#define _INCLUDE_VIRTUAL_MACHINE_H

#include <common/Singleton.h>
#include <common/StrongHandle.h>

#include <gm/gmVariable.h>
#include <gm/gmDebuggerFunk.h>

#include <map>

class gmMachine;

namespace funk
{
	class VirtualMachine : public Singleton<VirtualMachine>
	{
	public:
		void Update();
		void RunMain();
		void Tick();

		gmMachine & GetVM () { return *m_vm; }

		bool IsUsingByteCode() const { return m_bUseGmByteCode; }

	private:
		float	m_updateMs;
		float	m_dt;
		bool	m_bUseGmByteCode; // uses bytecode version
		int		m_numThreads;

		gmMachine *m_vm;
		int m_threadId;

		gmFunctionObject* m_tickFunc;
		gmDebuggerFunk m_debugger;

		struct ThreadAllocationItem
		{
			int allocations;
			int eraseCountdown;
		};
		std::map<const gmFunctionObject*, ThreadAllocationItem> m_threadAllocationsHistory;
		bool m_freezeThreadAllocationsGui;

		void HandleErrors();
		void ResetVM();

		friend Singleton<VirtualMachine>;
		VirtualMachine();
		~VirtualMachine();
	};
}

#endif