#include "gmDebuggerFunk.h"

#include <cstring>
#include <assert.h>

#include <gm/gmMachine.h>
#include <gm/gmThread.h>
#include <gm/gmConfig.h>
#include <vm/VirtualMachine.h>

// disabled for nao
//#define USE_VISUAL_DEBUGGER

using namespace funk;

const int DEBUG_WINDOW_PADDING = 20;
const float DEBUG_WINDOW_WIDTH = 0.47f;
const float DEBUG_WINDOW_HEIGHT = 0.6f;
const float DEBUG_THREADVIEWER_WIDTH = 0.25f;
const float DEBUG_THREADVIEWER_HEIGHT = 0.3f;

const char * DEBUG_KEY = "PAUSE";

void gmDebugSendMsg(gmDebugSession * a_session, const void * a_command, int a_len)
{
	a_session->m_owner->ReceiveMsg(a_command, a_len);
}

const void * gmDebugPumpMsg(gmDebugSession * a_session, int &a_len)
{
	if( a_session->m_owner->HasPacket() )
	{
		const gmDebuggerFunk::DataPacket & packet = a_session->m_owner->GetPacket();
		a_len = packet.len;
		return packet.data;
	}
	
	return NULL;	
}

const char * ConvertStateToStr( gmThread::State state )
{
	switch (state)
	{
	case gmThread::RUNNING:			return "RUNNING";
	case gmThread::SLEEPING:		return "SLEEPING";
	case gmThread::BLOCKED:			return "BLOCKED";
	case gmThread::KILLED:			return "KILLED";
	case gmThread::EXCEPTION:		return "EXCEPTION";
	case gmThread::SYS_PENDING:		return "SYS_PENDING";
	case gmThread::SYS_YIELD:		return "SYS_YIELD";
	case gmThread::SYS_EXCEPTION:	return "SYS_EXCEPTION";
	}

	return "";
}

bool ThreadImgui(gmThread * a_thread, void * a_context)
{
#ifdef USE_VISUAL_DEBUGGER
	if ( a_thread->GetState() == gmThread::EXCEPTION ) 
	{
		return true;
	}

	char buffer[128];
	sprintf_s( buffer, "Thread %d (%d) - %s (%.2f secs, %d bytes)", 
		a_thread->GetId(), 
		a_thread->GetGroup(), 
		ConvertStateToStr(a_thread->GetState()),
		a_thread->GetThreadTime()/1000.0f,
		a_thread->GetSystemMemUsed()
		);

	if ( a_thread->GetState() == gmThread::RUNNING )
	{
		if ( Imgui::Button(buffer) )
		{
			(*(gmThread**)a_context) = a_thread;
		}
	}
	else
	{
		Imgui::Print(buffer);
	}

#endif
	
	return true;
}

bool ThreadFindRunning(gmThread * a_thread, void * a_context)
{
	if ( a_thread->GetState() == gmThread::RUNNING ) 
	{
		(*(gmThread**)a_context) = a_thread;
		return false;
	}

	return true;
}

void gmDebuggerFunk::Open( gmMachine * machine )
{
	m_debugSesh.Open(machine);
	m_debugSesh.m_owner = this;
	m_debugSesh.m_sendMessage = gmDebugSendMsg;
	m_debugSesh.m_pumpMessage = gmDebugPumpMsg;

	memset( &m_debugState, 0, sizeof(m_debugState) );
	m_debugState.threadId = GM_INVALID_THREAD;
	m_showFunctions = false;

	// table traverse
	m_debugState.tableTraverse.Construct( machine, 16 );
	m_debugState.ResetTableSelector();

	// debug rendering
#ifdef USE_VISUAL_DEBUGGER
	const v2i dimen = Window::Get()->Sizei();
	m_texBG = new Texture( dimen.x, dimen.y );
	m_cam2d.InitScreenSpace();
#endif
}

void gmDebuggerFunk::BreakIntoRunningThread()
{
	if ( IsDebugging() ) 
	{
		printf("Cannot break while debugger is running");
		return;
	}

	gmThread * thr = NULL;
	m_debugSesh.GetMachine()->ForEachThread( ThreadFindRunning, &thr );

	// clicked on a thread
	if ( thr && thr->GetState() == gmThread::RUNNING )
	{
		QueuePacket("mbrk", thr->GetId() );
	}
	else
	{
		printf("Couldn't break. No RUNNING threads!\n");
	}	
}

void gmDebuggerFunk::Update()
{
	gmMachine * machine = m_debugSesh.GetMachine();

#if USE_VISUAL_DEBUGGER
	if ( Input::Get()->DidKeyJustGoDown(DEBUG_KEY) )
	{
		BreakIntoRunningThread();
	}
#endif

	// run debug thread
	if( IsDebugging() )
	{
		gmThread * thr = machine->GetThread( m_debugState.threadId );
		if ( thr && thr->m_debugFlags != 0 ) thr->Sys_Execute();
		machine->CollectGarbage(true);
	}

	m_debugSesh.Update();
}

void gmDebuggerFunk::Close()
{
	m_debugState.tableTraverse.Destruct(m_debugSesh.GetMachine());
	m_debugSesh.Close();
}

void gmDebuggerFunk::BeginSession()
{
	gmMachine* machine = m_debugSesh.GetMachine();
	machine->GetGlobals()->Set(machine, "g_debuggerOn", gmVariable(1));

	m_debugState.jumpToLineNumber = true;

#ifdef VISUAL_DEBUGGER
	const v2i dimen = Window::Get()->Sizei();
	int numBytes = sizeof(unsigned int)*dimen.x*dimen.y;
	unsigned char * buffer = new unsigned char[numBytes];
	memset( buffer, 0, numBytes );

	// grab front buffer data
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, dimen.x, dimen.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	m_texBG->Bind();
	m_texBG->SubData( buffer, dimen.x, dimen.y );
	m_texBG->Unbind();

	delete [] buffer;
#endif
}

void gmDebuggerFunk::EndSession()
{
	gmMachine* machine = m_debugSesh.GetMachine();
	machine->GetGlobals()->Set(machine, "g_debuggerOn", gmVariable(0));

	m_debugState.threadId = GM_INVALID_THREAD;
	m_debugState.sourceId = 0;
}

void gmDebuggerFunk::HandleRenderException()
{
#ifdef USE_VISUAL_DEBUGGER
	Renderer::Get()->HandleErrorDraw();

	// handle Imgui error
	if ( ImguiManager::Get()->state.workingWindow )
	{
		char buffer[128];
		sprintf_s( buffer, "Exception occurred between Imgui::Begin('%s')", ImguiManager::Get()->state.workingWindowTitle.c_str() );
		printf(buffer);

		Imgui::End();
	}
#endif
}

void gmDebuggerFunk::ReceiveMsg( const void * a_command, int &a_len )
{
#if USE_VISUAL_DEBUGGER
	if ( !a_command ) return;

	gmMachine* machine = m_debugSesh.GetMachine();
	const int command = *(int*)a_command;

	switch(command)
	{
		case GM_MAKE_ID32('d','b','r','k'):
		{
			int threadId = *((int*)(a_command)+1);
			int lineNumber = *((int*)(a_command)+3);
			gmuint32 sourceId = *((gmuint32*)(a_command)+2);

			assert( threadId != GM_INVALID_THREAD );

			if ( !IsDebugging() ) BeginSession();

			m_debugState.jumpToLineNumber = (m_debugState.threadId != threadId) || (m_debugState.sourceId != sourceId);
			m_debugState.threadId = threadId;
			m_debugState.sourceId = sourceId;
			m_debugState.lineNumber = lineNumber;
			m_debugState.callStackLevel = 0;
			m_debugState.ResetTableSelector();

			break;
		}
		case GM_MAKE_ID32('d','e','x','c'):
		{
			const int threadId = *((int*)(a_command)+1);
			assert( threadId != GM_INVALID_THREAD );
			
			// ignore if console command failed
			if ( threadId == VirtualMachine::Get()->GetConsole().GetLastCmdThread() )
			{
				break;
			}

			// handle rendering exception
			gmVariable varRendering = machine->GetGlobals()->Get(machine, "g_rendering");
			if ( varRendering.IsInt() && varRendering.GetInt() == 1 ) 
			{
				HandleRenderException();
			}

			if ( !IsDebugging() ) BeginSession();			

			// get thread
			m_debugState.threadId = threadId;
			gmThread* thr = machine->GetThread(m_debugState.threadId);

			const gmStackFrame * frame = thr->GetFrame();
			int base = thr->GetIntBase();
			const gmuint8 * ip = thr->GetInstruction();

			// get the function object
			gmVariable * fnVar = &thr->GetBottom()[base - 1];
			assert(fnVar->m_type == GM_FUNCTION);
			gmFunctionObject * fn = (gmFunctionObject *) GM_MOBJECT(machine, fnVar->m_value.m_ref);

			m_debugState.lineNumber = fn->GetLine(ip);
			m_debugState.sourceId = fn->GetSourceId();
			m_debugState.callStackLevel = 0;
			m_debugState.ResetTableSelector();

			char buffer[64];
			sprintf_s( buffer, "Exception in Thread %d", threadId );
			printf(buffer);

			break;
		}
		case GM_MAKE_ID32('d','s','t','p'):
		{
			// Nothing.
		}
	}
#endif
}

void gmDebuggerFunk::DrawBG()
{
#ifdef USE_VISUAL_DEBUGGER
	const v2 dimen = Window::Get()->Sizef();

	m_cam2d.Begin();
	Renderer::Get()->BeginDefaultShader();
	
	// texture
	m_texBG->Bind(0);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin( GL_QUADS );

		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( dimen.x, 0.0f );

		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( 0.0f, 0.0f );

		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( 0.0f, dimen.y  );

		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( dimen.x, dimen.y );

	glEnd();
	m_texBG->Unbind();

	// color overlay
	gmThread * thr = m_debugSesh.GetMachine()->GetThread( m_debugState.threadId );
	v3 color = v3(0.0f, 0.0f, 1.0f);
	if ( thr && thr->GetState() == gmThread::EXCEPTION ) color = v3(1.0f,0.0f,0.0f);
	glColor4f( color.x, color.y, color.z, 0.15f );

	glBegin( GL_QUADS );

		glVertex2f( dimen.x, 0.0f );
		glVertex2f( 0.0f, 0.0f );
		glVertex2f( 0.0f, dimen.y  );
		glVertex2f( dimen.x, dimen.y );

	glEnd();

	m_cam2d.End();
	Renderer::Get()->EndDefaultShader();
#endif
}

void gmDebuggerFunk::Gui()
{
#ifdef USE_VISUAL_DEBUGGER
	if ( IsDebugging() )
	{
		DrawBG();
		GuiSource();
		GuiThread();
		GuiThreadsViewer();
	}
#endif
}

void gmDebuggerFunk::GuiSource()
{
#ifdef USE_VISUAL_DEBUGGER
	if ( !IsDebugging() ) return;

	const char *source;
	const char *filename;
	m_debugSesh.GetMachine()->GetSourceCode(m_debugState.sourceId, source, filename);

	// Window title
	{
		char titleBuffer[128];
		sprintf(titleBuffer, "Source Code: %s", filename );
		const v2i windowDimen = Window::Get()->Sizei();
		const int width = (int)(windowDimen.x*DEBUG_WINDOW_WIDTH);
		const int height = (int)(windowDimen.y*DEBUG_WINDOW_HEIGHT);

		Imgui::Begin(titleBuffer, 
			v2i(DEBUG_WINDOW_PADDING, height + DEBUG_WINDOW_PADDING),
			v2i(width, height) );
	}

	int srcLen = strlen(source);
	int offset = 0;
	int lineNumber = 1;
	const char * textPos = source;
	
	// traverse each line
	while( textPos-source < srcLen )
	{
		const char * endPos = strchr( textPos, '\n' );

		if ( endPos == NULL ) endPos = source+srcLen;

		// copy line
		char buffer[256];
		int len = min( (int)(endPos-textPos), (int)sizeof(buffer)-1);
		memcpy(buffer, textPos, len);
		buffer[len] = 0;
		textPos = endPos+1;

		char lineCountBuffer[8];
		sprintf(lineCountBuffer, "%4d:", lineNumber);

		if ( lineNumber == m_debugState.lineNumber ) Imgui::Separator();
		Imgui::Print(lineCountBuffer);
		Imgui::SameLine();
		Imgui::Print(buffer);
		if ( lineNumber == m_debugState.lineNumber ) Imgui::Separator();

		++lineNumber;
	}

	if ( m_debugState.jumpToLineNumber )
	{
		m_debugState.jumpToLineNumber = false;
		Imgui::SetScrollY( (float)m_debugState.lineNumber / max(1,(lineNumber-2)) );
	}

	Imgui::End();
#endif
}

bool gmDebuggerFunk::IsDebugging() const
{ 
	return m_debugState.threadId != GM_INVALID_THREAD; 
}

void gmDebuggerFunk::GuiThread()
{
#ifdef USE_VISUAL_DEBUGGER
	gmThread * thr = m_debugSesh.GetMachine()->GetThread( m_debugState.threadId );
	
	// occurs when you continue from breakpoint
	if ( thr == NULL || (thr->GetState() != gmThread::EXCEPTION && thr->m_debugFlags == 0) )
	{
		EndSession();		
		return;
	}

	assert( m_debugState.sourceId != 0 );
	gmMachine* machine = m_debugSesh.GetMachine();
	
	// create variables table
	gmTableObject * table = machine->AllocTableObject();
	table->Set(machine, "[globals]", gmVariable( machine->GetGlobals() ) );
	table->Set(machine, "[this]", *thr->GetThis() );
	
	// gather stack-frame local variables
	{
		const gmStackFrame * frame = thr->GetFrame();
		gmVariable * base = thr->GetBase();
		int level = 0;

		while(frame)
		{
			gmVariable * fnVar = base - 1;
			gmFunctionObject * fn = (gmFunctionObject *) GM_MOBJECT(machine, fnVar->m_value.m_ref);
			
			if( level == m_debugState.callStackLevel )
			{
				for(int i = 0; i < fn->GetNumParamsLocals(); ++i )
				{
					table->Set( machine, fn->GetSymbol(i), base[i] );
				}

				break;
			}

			base = thr->GetBottom() + frame->m_returnBase;
			frame = frame->m_prev;
			++level;
		}
	}

	// begin window
	char buffer[256];
	sprintf( buffer, "Thread %d (%s)", m_debugState.threadId, ConvertStateToStr(thr->GetState()) );
	const v2i windowDimen = Window::Get()->Sizei();
	const int width = (int)(windowDimen.x*DEBUG_WINDOW_WIDTH);
	const int height = (int)(windowDimen.y*DEBUG_WINDOW_HEIGHT);
	Imgui::Begin(buffer, 
		v2i(windowDimen.x-DEBUG_WINDOW_PADDING-width, height + DEBUG_WINDOW_PADDING),
		v2i(width, height) );
	
	// debug code stepping functions
	if ( thr->GetState() != gmThread::EXCEPTION )
	{
		if ( Imgui::Button("Continue") || Input::Get()->DidKeyJustGoDown(DEBUG_KEY) )  QueuePacket( "mrun", m_debugState.threadId );
		Imgui::SameLine();
		if ( Imgui::Button("Step Into") ) QueuePacket( "msin", m_debugState.threadId );
		Imgui::SameLine();
		if ( Imgui::Button("Step Over") ) QueuePacket( "msov", m_debugState.threadId );
		Imgui::SameLine();
		if ( Imgui::Button("Step Out") )  QueuePacket( "msou", m_debugState.threadId );
		Imgui::SameLine();
		Imgui::Print(" _ ");
		Imgui::SameLine();
	}

	// can kill thread
	if ( Imgui::Button("Kill Thread") )
	{
		machine->Sys_SwitchState(thr, gmThread::KILLED);
		EndSession();
	}
	
	// call-stack
	Imgui::Header("Call Stack");
	{
		const gmStackFrame * frame = thr->GetFrame();
		int base = thr->GetIntBase();
		const gmuint8 * ip = thr->GetInstruction();
		int level = 0;

		while(frame)
		{
			// get the function object
			gmVariable * fnVar = &thr->GetBottom()[base - 1];
			if( fnVar->m_type == GM_FUNCTION )
			{
				gmFunctionObject * fn = (gmFunctionObject *) GM_MOBJECT(machine, fnVar->m_value.m_ref);

				int lineNumber = fn->GetLine(ip);
				gmuint32 sourceId = fn->GetSourceId();
				const char *source;
				const char *filename;
				m_debugSesh.GetMachine()->GetSourceCode(sourceId, source, filename);

				char * arrowbuffer = level == m_debugState.callStackLevel ? ">> " : "";
				sprintf(buffer, "%s[%d] %s() - %s (%d)", arrowbuffer, level, fn->GetDebugName(), filename, lineNumber );

				if ( Imgui::Button(buffer) )
				{
					m_debugState.ResetTableSelector();
					m_debugState.lineNumber = lineNumber;
					m_debugState.sourceId = sourceId;
					m_debugState.callStackLevel = level;
					m_debugState.jumpToLineNumber = true;
				}
			}

			base = frame->m_returnBase;
			ip = frame->m_returnAddress;
			frame = frame->m_prev;

			++level;
		}
	}

	Imgui::Header("Variables");
	Imgui::CheckBox("Show Functions", m_showFunctions);
	Imgui::Separator();
	funk::ImguiOutputTable( table, &m_debugState.tableTraverse, m_showFunctions );

	// thread info
	sprintf(buffer, "Group Id: %d\nStart Time: %.2f secs\nRunning Time: %.2f secs\nMachine Time: %.2f secs\nSystem Memory Used: %d bytes", 
		thr->GetGroup(), 
		thr->GetStartTime()/1000.0f, 
		thr->GetThreadTime()/1000.0f, 
		machine->GetTime()/1000.0f, 
		thr->GetSystemMemUsed() );

	Imgui::Header("Thread Info");
	Imgui::PrintParagraph(buffer);

	Imgui::End();
#endif
}

void gmDebuggerFunk::GuiThreadsViewer()
{
#ifdef USE_VISUAL_DEBUGGER
	gmMachine* machine = m_debugSesh.GetMachine();

	const v2i windowDimen = Window::Get()->Sizei();
	const int width = (int)(windowDimen.x*DEBUG_THREADVIEWER_WIDTH);
	const int height = (int)(windowDimen.y*DEBUG_THREADVIEWER_HEIGHT);
	Imgui::Begin("Threads Viewer", 
		v2i(windowDimen.x - width - DEBUG_WINDOW_PADDING, windowDimen.y - DEBUG_WINDOW_PADDING), 
		v2i(width, height) );

	gmThread * returnThread = NULL;
	machine->ForEachThread( ThreadImgui, &returnThread );

	// clicked on a thread
	if ( returnThread && returnThread->GetId() != m_debugState.threadId )
	{
		gmThread * thr = machine->GetThread(  m_debugState.threadId );
		if ( thr->GetState() == gmThread::EXCEPTION )
		{
			machine->Sys_SwitchState(thr, gmThread::KILLED);
		}
		else
		{
			QueuePacket( "mrun", m_debugState.threadId );
		}
				
		QueuePacket( "mbrk", returnThread->GetId() );
		m_debugState.threadId = returnThread->GetId();
	}

	Imgui::End();
#endif
}

const gmDebuggerFunk::DataPacket & gmDebuggerFunk::GetPacket()
{
	assert( !m_packets.empty() );

	DataPacket &packet = m_packets.front();
	memcpy( &m_currPacket, &packet, sizeof(DataPacket) );
	m_packets.pop();

	return m_currPacket;
}

void gmDebuggerFunk::QueuePacket( char * msg )
{
	DataPacket &packet = CreatePacket();
	packet.len = 4;

	memcpy( packet.data, msg, 4 );
}

void gmDebuggerFunk::QueuePacket( char * msg, int id )
{
	DataPacket &packet = CreatePacket();
	packet.len = 8;

	memcpy( packet.data, msg, 4 );
	memcpy( packet.data+4, &id, 4 );
}

void gmDebuggerFunk::QueuePacket( char * msg, int id, int data )
{
	DataPacket &packet = CreatePacket();
	packet.len = 12;

	memcpy( packet.data, msg, 4 );
	memcpy( packet.data+4, &id, 4 );
	memcpy( packet.data+8, &data, 4 );
}

gmDebuggerFunk::DataPacket& gmDebuggerFunk::CreatePacket()
{
	m_packets.push( DataPacket() );
	DataPacket &packet = m_packets.back();
	return packet;
}

void gmDebuggerFunk::DebugState::ResetTableSelector()
{
	for( int i = 0; i < tableTraverse.Size(); ++i )
	{
		tableTraverse.SetAt( i, gmVariable(-1) );
	}
}