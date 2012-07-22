/*
    _____               __  ___          __            ____        _      __
   / ___/__ ___ _  ___ /  |/  /__  ___  / /_____ __ __/ __/_______(_)__  / /_
  / (_ / _ `/  ' \/ -_) /|_/ / _ \/ _ \/  '_/ -_) // /\ \/ __/ __/ / _ \/ __/
  \___/\_,_/_/_/_/\__/_/  /_/\___/_//_/_/\_\\__/\_, /___/\__/_/ /_/ .__/\__/
                                               /___/             /_/
                                             
  See Copyright Notice in gmMachine.h

*/

#include <string.h>
#include "gmDebugger.h"

//
// Please note that gmDebugger.c/.h are for implementing
// a debugger application and should not be included
// in an normal GM application build.
//

#ifndef GM_MAKE_ID32
  #define GM_MAKE_ID32( a, b, c, d )  ( ((d)<<24) | ((c)<<16) | ((b)<<8) | (a))
#endif //GM_MAKE_ID32

#define ID_mrun GM_MAKE_ID32('m','r','u','n')
#define ID_msin GM_MAKE_ID32('m','s','i','n')
#define ID_msou GM_MAKE_ID32('m','s','o','u')
#define ID_msov GM_MAKE_ID32('m','s','o','v')
#define ID_mgct GM_MAKE_ID32('m','g','c','t')
#define ID_mgsr GM_MAKE_ID32('m','g','s','r')
#define ID_mgsi GM_MAKE_ID32('m','g','s','i')
#define ID_mgti GM_MAKE_ID32('m','g','t','i')
#define ID_mgvi GM_MAKE_ID32('m','g','v','i')
#define ID_msbp GM_MAKE_ID32('m','s','b','p')
#define ID_mbrk GM_MAKE_ID32('m','b','r','k')
#define ID_mend GM_MAKE_ID32('m','e','n','d')

#define ID_dbrk GM_MAKE_ID32('d','b','r','k')
#define ID_dexc GM_MAKE_ID32('d','e','x','c')
#define ID_drun GM_MAKE_ID32('d','r','u','n')
#define ID_dstp GM_MAKE_ID32('d','s','t','p')
#define ID_dsrc GM_MAKE_ID32('d','s','r','c')
#define ID_dctx GM_MAKE_ID32('d','c','t','x')
#define ID_call GM_MAKE_ID32('c','a','l','l')
#define ID_vari GM_MAKE_ID32('v','a','r','i')
#define ID_done GM_MAKE_ID32('d','o','n','e')
#define ID_dsri GM_MAKE_ID32('d','s','r','i')
#define ID_srci GM_MAKE_ID32('s','r','c','i')
#define ID_done GM_MAKE_ID32('d','o','n','e')
#define ID_dthi GM_MAKE_ID32('d','t','h','i')
#define ID_thri GM_MAKE_ID32('t','h','r','i')
#define ID_done GM_MAKE_ID32('d','o','n','e')
#define ID_derr GM_MAKE_ID32('d','e','r','r')
#define ID_dmsg GM_MAKE_ID32('d','m','s','g')
#define ID_dack GM_MAKE_ID32('d','a','c','k')
#define ID_dend GM_MAKE_ID32('d','e','n','d')

//
// Please note that gmDebugger.c/.h are for implementing
// a debugger application and should not be included
// in an normal GM application build.
//


gmDebuggerSession::gmDebuggerSession()
{
  m_outSize = 256;
  m_out = (void*) new char[m_outSize];
  m_outCursor = 0;
  m_in = NULL;
  m_inCursor = m_inSize = NULL;
}


gmDebuggerSession::~gmDebuggerSession()
{
  if(m_out)
  {
    delete [] (char*)m_out;
  }
}


void gmDebuggerSession::Update()
{
  for(;;)
  {
    m_in = m_pumpMessage(this, m_inSize);
    if(m_in == NULL) break;
    m_inCursor = 0;

    int id, pa, pb, pc;
    const char * sa, * sb, * sc;

    Unpack(id);
    switch(id)
    {
      case ID_dbrk :
        Unpack(id).Unpack(pa).Unpack(pb);
        gmDebuggerBreak(this, id, pa, pb);
        break;
      case ID_drun :
        Unpack(id);
        gmDebuggerRun(this, id);
        break;
      case ID_dstp :
        Unpack(id);
        gmDebuggerStop(this, id);
        break;
      case ID_dsrc :
        Unpack(id).Unpack(sa).Unpack(sb);
        gmDebuggerSource(this, id, sa, sb);
        break;
      case ID_dexc :
        Unpack(id);
        gmDebuggerException(this, id);
        break;
      case ID_dctx :
        Unpack(id).Unpack(pa); // thread id, callframe
        gmDebuggerBeginContext(this, id, pa);
        for(;;)
        {
          Unpack(id);
          if(id == ID_call)
          {
            Unpack(id).Unpack(sa).Unpack(pa).Unpack(pb).Unpack(sb).Unpack(sc).Unpack(pc);
            gmDebuggerContextCallFrame(this, id, sa, pa, pb, sb, sc, pc);
          }
          else if(id == ID_vari)
          {
            Unpack(sa).Unpack(sb).Unpack(pa);
            gmDebuggerContextVariable(this, sa, sb, pa);
          }
          else if(id == ID_done) break;
          else break;
        }
        gmDebuggerEndContext(this);
        break;
      case ID_dsri :
        // todo
        break;
      case ID_dthi :
        gmDebuggerBeginThreadInfo(this);
        for(;;)
        {
          Unpack(id);
          if(id == ID_thri)
          {
            Unpack(pa).Unpack(pb);
            gmDebuggerThreadInfo(this, pa, pb);
          }
          else if(id == ID_done) break;
          else break;
        }
        gmDebuggerEndThreadInfo(this);
        break;
      case ID_derr :
        Unpack(sa);
        gmDebuggerError(this, sa);
        break;
      case ID_dmsg :
        Unpack(sa);
        gmDebuggerMessage(this, sa);
        break;
      case ID_dack :
        Unpack(pa).Unpack(pb);
        gmDebuggerAck(this, pa, pb);
        break;
      case ID_dend :
        gmDebuggerQuit(this);
        break;
      default:;
    }
  }
}


bool gmDebuggerSession::Open()
{
  m_outCursor = 0;
  return true;
}


bool gmDebuggerSession::Close()
{
  return true;
}


gmDebuggerSession &gmDebuggerSession::Pack(int a_val)
{
  Need(4);
  memcpy((char *) m_out + m_outCursor, &a_val, 4);
  m_outCursor += 4;
  return *this;
}


gmDebuggerSession &gmDebuggerSession::Pack(const char * a_val)
{
  if(a_val)
  {
    int len = strlen(a_val) + 1;
    Need(len);
    memcpy((char *) m_out + m_outCursor, a_val, len);
    m_outCursor += len;
  }
  else
  {
    Need(1);
    memcpy((char *) m_out + m_outCursor, "", 1);
    m_outCursor += 1;
  }
  return *this;
}


void gmDebuggerSession::Send()
{
  m_sendMessage(this, m_out, m_outCursor);
  m_outCursor = 0;
}


gmDebuggerSession &gmDebuggerSession::Unpack(int &a_val)
{
  if(m_inCursor + 4 <= m_inSize)
  {
    memcpy(&a_val, (const char *) m_in + m_inCursor, 4);
    m_inCursor += 4;
  }
  else
  {
    a_val = 0;
  }
  return *this;
}


gmDebuggerSession &gmDebuggerSession::Unpack(const char * &a_val)
{
  a_val = (const char *) m_in + m_inCursor;
  m_inCursor += strlen(a_val) + 1;
  return *this;
}


void gmDebuggerSession::Need(int a_bytes)
{
  if((m_outCursor + a_bytes) >= m_outSize)
  {
    int newSize = m_outSize + a_bytes + 256;
    void * buffer = (void*)new char[newSize];
    memcpy(buffer, m_out, m_outCursor);
    delete [] (char*)m_out;
    m_out = buffer;
    m_outSize = newSize;
  }
}


void gmMachineRun(gmDebuggerSession * a_session, int a_threadId)
{
  a_session->Pack(ID_mrun).Pack(a_threadId).Send();
}

void gmMachineStepInto(gmDebuggerSession * a_session, int a_threadId)
{
  a_session->Pack(ID_msin).Pack(a_threadId).Send();
}

void gmMachineStepOver(gmDebuggerSession * a_session, int a_threadId)
{
  a_session->Pack(ID_msov).Pack(a_threadId).Send();
}

void gmMachineStepOut(gmDebuggerSession * a_session, int a_threadId)
{
  a_session->Pack(ID_msou).Pack(a_threadId).Send();
}

void gmMachineGetContext(gmDebuggerSession * a_session, int a_threadId, int a_callframe)
{
  a_session->Pack(ID_mgct).Pack(a_threadId).Pack(a_callframe).Send();
}

void gmMachineGetSource(gmDebuggerSession * a_session, int a_sourceId)
{
  a_session->Pack(ID_mgsr).Pack(a_sourceId).Send();
}

void gmMachineGetSourceInfo(gmDebuggerSession * a_session)
{
  a_session->Pack(ID_mgsi).Send();
}

void gmMachineGetThreadInfo(gmDebuggerSession * a_session)
{
  a_session->Pack(ID_mgti).Send();
}

void gmMachineGetVariableInfo(gmDebuggerSession * a_session, int a_variableId)
{
  a_session->Pack(ID_mgvi).Pack(a_variableId).Send();
}

void gmMachineSetBreakPoint(gmDebuggerSession * a_session, int a_responseId, int a_sourceId, int a_lineNumber, int a_threadId, int a_enabled)
{
  a_session->Pack(ID_msbp).Pack(a_responseId).Pack(a_sourceId).Pack(a_lineNumber).Pack(a_threadId).Pack(a_enabled).Send();
}

void gmMachineBreak(gmDebuggerSession * a_session, int a_threadId)
{
  a_session->Pack(ID_mbrk).Pack(a_threadId).Send();
}

void gmMachineQuit(gmDebuggerSession * a_session)
{
  a_session->Pack(ID_mend).Send();
}

// HACK: need for link symbols

#include "gm/gmDebug.h"

void gmDebuggerBreak(gmDebuggerSession * a_session, int a_threadId, int a_sourceId, int a_lineNumber) {
  a_session->Pack(ID_dbrk).Pack(a_threadId).Pack(a_sourceId).Pack(a_lineNumber).Send();
}
void gmDebuggerException(gmDebuggerSession * a_session, int a_threadId) {
  a_session->Pack(ID_dexc).Pack(a_threadId).Send();
}
void gmDebuggerRun(gmDebuggerSession * a_session, int a_threadId) {
  a_session->Pack(ID_drun).Pack(a_threadId).Send();
}
void gmDebuggerStop(gmDebuggerSession * a_session, int a_threadId) {
  a_session->Pack(ID_dstp).Pack(a_threadId).Send();
}
void gmDebuggerSource(gmDebuggerSession * a_session, int a_sourceId, const char * a_sourceName, const char * a_source) {
  a_session->Pack(ID_dsrc).Pack(a_sourceId).Pack(a_sourceName).Pack(a_source).Send();
}
void gmDebuggerBeginContext(gmDebuggerSession * a_session, int a_threadId, int a_callFrame) {
  a_session->Pack(ID_dctx).Pack(a_threadId).Pack(a_callFrame);
}
void gmDebuggerContextCallFrame(gmDebuggerSession * a_session, int a_callFrame, const char * a_functionName, int a_sourceId, int a_lineNumber, const char * a_thisSymbol, const char * a_thisValue, gmptr a_thisId) {
  a_session->Pack(ID_call).Pack(a_callFrame).Pack(a_functionName).Pack(a_sourceId).Pack(a_lineNumber).Pack(a_thisSymbol).Pack(a_thisValue).Pack(a_thisId);
}
void gmDebuggerContextVariable(gmDebuggerSession * a_session, const char * a_varSymbol, const char * a_varValue, gmptr a_varId) {
  a_session->Pack(ID_vari).Pack(a_varSymbol).Pack(a_varValue).Pack(a_varId);
}
void gmDebuggerEndContext(gmDebuggerSession * a_session) {
  a_session->Pack(ID_done).Send();
}
void gmDebuggerBeginSourceInfo(gmDebuggerSession * a_session) {
  a_session->Pack(ID_dsri);
}
void gmDebuggerSourceInfo(gmDebuggerSession * a_session, int a_sourceId, const char * a_sourceName) {
  a_session->Pack(ID_srci).Pack(a_sourceId).Pack(a_sourceName);
}
void gmDebuggerEndSourceInfo(gmDebuggerSession * a_session) {
  a_session->Pack(ID_done).Send();
}
void gmDebuggerBeginThreadInfo(gmDebuggerSession * a_session) {
  a_session->Pack(ID_dthi);
}
void gmDebuggerThreadInfo(gmDebuggerSession * a_session, int a_threadId, int a_threadState) {
  a_session->Pack(ID_thri).Pack(a_threadId).Pack(a_threadState);
}
void gmDebuggerEndThreadInfo(gmDebuggerSession * a_session) {
  a_session->Pack(ID_done).Send();
}
void gmDebuggerError(gmDebuggerSession * a_session, const char * a_error) {
  a_session->Pack(ID_derr).Pack(a_error).Send();
}
void gmDebuggerMessage(gmDebuggerSession * a_session, const char * a_message) {
  a_session->Pack(ID_dmsg).Pack(a_message).Send();
}
void gmDebuggerAck(gmDebuggerSession * a_session, int a_response, int a_posNeg) {
  a_session->Pack(ID_dack).Pack(a_response).Pack(a_posNeg).Send();
}
void gmDebuggerQuit(gmDebuggerSession * a_session) {
  a_session->Pack(ID_dend).Send();
}
