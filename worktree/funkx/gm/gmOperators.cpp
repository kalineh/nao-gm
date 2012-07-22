/*
    _____               __  ___          __            ____        _      __
   / ___/__ ___ _  ___ /  |/  /__  ___  / /_____ __ __/ __/_______(_)__  / /_
  / (_ / _ `/  ' \/ -_) /|_/ / _ \/ _ \/  '_/ -_) // /\ \/ __/ __/ / _ \/ __/
  \___/\_,_/_/_/_/\__/_/  /_/\___/_//_/_/\_\\__/\_, /___/\__/_/ /_/ .__/\__/
                                               /___/             /_/
                                             
  See Copyright Notice in gmMachine.h

*/

#include "gmConfig.h"
#include "gmOperators.h"
#include "gmThread.h"
#include "gmStringObject.h"
#include <math/v2.h>
#include <math/v3.h>
//#include <math.h>

using namespace funk;

const char * gmGetOperatorName(gmOperator a_operator)
{
  switch(a_operator)
  {
    case O_GETDOT : return "getdot";
    case O_SETDOT : return "setdot";
    case O_GETIND : return "getind";
    case O_SETIND : return "setind";
    case O_ADD : return "add";
    case O_SUB : return "sub";
    case O_MUL : return "mul";
    case O_DIV : return "div";
    case O_REM : return "mod";
    case O_BIT_OR : return "bitor";
    case O_BIT_XOR : return "bitxor";
    case O_BIT_AND : return "bitand";
    case O_BIT_SHIFTLEFT : return "shiftleft";
    case O_BIT_SHIFTRIGHT : return "shiftright";
    case O_BIT_INV : return "bitinv";
    case O_LT : return "lt";
    case O_GT : return "gt";
    case O_LTE : return "lte";
    case O_GTE : return "gte";
    case O_EQ : return "eq";
    case O_NEQ : return "neq";
    case O_NEG : return "neg";
    case O_POS : return "pos";
    case O_NOT : return "not";
#if GM_BOOL_OP
    case O_BOOL : return "bool";
#endif // GM_BOOL_OP
    default :;
  }
  return "undefined";
}

gmOperator gmGetOperator(const char * a_operatorName)
{
  if(_gmstricmp(a_operatorName, "getdot") == 0) return O_GETDOT;
  if(_gmstricmp(a_operatorName, "setdot") == 0) return O_SETDOT;
  if(_gmstricmp(a_operatorName, "getind") == 0) return O_GETIND;
  if(_gmstricmp(a_operatorName, "setind") == 0) return O_SETIND;
  if(_gmstricmp(a_operatorName, "add") == 0) return O_ADD;
  if(_gmstricmp(a_operatorName, "sub") == 0) return O_SUB;
  if(_gmstricmp(a_operatorName, "mul") == 0) return O_MUL;
  if(_gmstricmp(a_operatorName, "div") == 0) return O_DIV;
  if(_gmstricmp(a_operatorName, "mod") == 0) return O_REM;
  if(_gmstricmp(a_operatorName, "bitor") == 0) return O_BIT_OR;
  if(_gmstricmp(a_operatorName, "bitxor") == 0) return O_BIT_XOR;
  if(_gmstricmp(a_operatorName, "bitand") == 0) return O_BIT_AND;
  if(_gmstricmp(a_operatorName, "shiftleft") == 0) return O_BIT_SHIFTLEFT;
  if(_gmstricmp(a_operatorName, "shiftright") == 0) return O_BIT_SHIFTRIGHT;
  if(_gmstricmp(a_operatorName, "bitinv") == 0) return O_BIT_INV;
  if(_gmstricmp(a_operatorName, "lt") == 0) return O_LT;
  if(_gmstricmp(a_operatorName, "gt") == 0) return O_GT;
  if(_gmstricmp(a_operatorName, "lte") == 0) return O_LTE;
  if(_gmstricmp(a_operatorName, "gte") == 0) return O_GTE;
  if(_gmstricmp(a_operatorName, "eq") == 0) return O_EQ;
  if(_gmstricmp(a_operatorName, "neq") == 0) return O_NEQ;
  if(_gmstricmp(a_operatorName, "neg") == 0) return O_NEG;
  if(_gmstricmp(a_operatorName, "pos") == 0) return O_POS;
  if(_gmstricmp(a_operatorName, "not") == 0) return O_NOT;
#if GM_BOOL_OP
  if(_gmstricmp(a_operatorName, "bool") == 0) return O_BOOL;
#endif // GM_BOOL_OP
  return O_MAXOPERATORS;
}


//
// GM_NULL
//

//
// GM_INT
//

void GM_CDECL gmIntOpAdd(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int += a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpSub(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int -= a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpMul(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int *= a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpDiv(gmThread * a_thread, gmVariable * a_operands)
{
#if GMMACHINE_GMCHECKDIVBYZERO
  if(a_operands[1].m_value.m_int != 0)
  {
    a_operands[0].m_value.m_int /= a_operands[1].m_value.m_int;
  }
  else
  {
    a_thread->GetMachine()->GetLog().LogEntry("Divide by zero.");
    a_operands[0].Nullify();
    // NOTE: No proper way to signal exception from here at present
  }
#else // GMMACHINE_GMCHECKDIVBYZERO
  a_operands[0].m_value.m_int /= a_operands[1].m_value.m_int;
#endif // GMMACHINE_GMCHECKDIVBYZERO
}
void GM_CDECL gmIntOpRem(gmThread * a_thread, gmVariable * a_operands)
{
#if GMMACHINE_GMCHECKDIVBYZERO
  if(a_operands[1].m_value.m_int != 0)
  {
    a_operands[0].m_value.m_int %= a_operands[1].m_value.m_int;
  }
  else
  {
    a_thread->GetMachine()->GetLog().LogEntry("Divide by zero.");
    a_operands[0].Nullify();
    // NOTE: No proper way to signal exception from here at present
  }
#else // GMMACHINE_GMCHECKDIVBYZERO
  a_operands[0].m_value.m_int %= a_operands[1].m_value.m_int;
#endif // GMMACHINE_GMCHECKDIVBYZERO
}
void GM_CDECL gmIntOpBitOr(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int |= a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpBitXor(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int ^= a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpBitAnd(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int &= a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpBitShiftLeft(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int <<= a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpBitShiftRight(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int >>= a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpInv(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int = ~a_operands[0].m_value.m_int;
}
void GM_CDECL gmIntOpLT(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int = a_operands[0].m_value.m_int < a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpGT(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int = a_operands[0].m_value.m_int > a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpLTE(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int = a_operands[0].m_value.m_int <= a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpGTE(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int = a_operands[0].m_value.m_int >= a_operands[1].m_value.m_int;
}
void GM_CDECL gmIntOpEQ(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int = (a_operands[0].m_value.m_int == a_operands[1].m_value.m_int);
}
void GM_CDECL gmIntOpNEQ(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int = (a_operands[0].m_value.m_int != a_operands[1].m_value.m_int);
}
void GM_CDECL gmIntOpNEG(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int = -a_operands[0].m_value.m_int;
}
void GM_CDECL gmIntOpPOS(gmThread * a_thread, gmVariable * a_operands)
{
}
void GM_CDECL gmIntOpNOT(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands[0].m_value.m_int = !a_operands[0].m_value.m_int;
}



//
// GM_FLOAT
//

#define INTTOFLOAT(A) (((A)->m_type == GM_FLOAT) ? (A)->m_value.m_float : (float) (A)->m_value.m_int)

void GM_CDECL gmFloatOpAdd(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_float = INTTOFLOAT(a_operands) + INTTOFLOAT(a_operands + 1);
  a_operands->m_type = GM_FLOAT; 
}
void GM_CDECL gmFloatOpSub(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_float = INTTOFLOAT(a_operands) - INTTOFLOAT(a_operands + 1);
  a_operands->m_type = GM_FLOAT; 
}
void GM_CDECL gmFloatOpMul(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_float = INTTOFLOAT(a_operands) * INTTOFLOAT(a_operands + 1);
  a_operands->m_type = GM_FLOAT; 
}
void GM_CDECL gmFloatOpDiv(gmThread * a_thread, gmVariable * a_operands)
{
#if GMMACHINE_GMCHECKDIVBYZERO
  if(INTTOFLOAT(a_operands + 1) != 0)
  {
    a_operands->m_value.m_float = INTTOFLOAT(a_operands) / INTTOFLOAT(a_operands + 1);
    a_operands->m_type = GM_FLOAT;
  }
  else
  {
    a_thread->GetMachine()->GetLog().LogEntry("Divide by zero.");
    a_operands->Nullify(); // NOTE: Should probably return +/- INF, not null
    // NOTE: No proper way to signal exception from here at present
  }
#else // GMMACHINE_GMCHECKDIVBYZERO
  a_operands->m_value.m_float = INTTOFLOAT(a_operands) / INTTOFLOAT(a_operands + 1);
  a_operands->m_type = GM_FLOAT;
#endif // GMMACHINE_GMCHECKDIVBYZERO
}
void GM_CDECL gmFloatOpRem(gmThread * a_thread, gmVariable * a_operands)
{
#if GMMACHINE_GMCHECKDIVBYZERO
  if(INTTOFLOAT(a_operands + 1) != 0)
  {
    a_operands->m_value.m_float = fmodf(INTTOFLOAT(a_operands), INTTOFLOAT(a_operands + 1));
    a_operands->m_type = GM_FLOAT;
  }
  else
  {
    a_thread->GetMachine()->GetLog().LogEntry("Divide by zero.");
    a_operands->Nullify();
    // NOTE: No proper way to signal exception from here at present
  }
#else // GMMACHINE_GMCHECKDIVBYZERO
  a_operands->m_value.m_float = fmodf(INTTOFLOAT(a_operands), INTTOFLOAT(a_operands + 1));
  a_operands->m_type = GM_FLOAT;
#endif // GMMACHINE_GMCHECKDIVBYZERO
}
void GM_CDECL gmFloatOpInc(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_float = INTTOFLOAT(a_operands) + 1.0f;
  a_operands->m_type = GM_FLOAT; 
}
void GM_CDECL gmFloatOpDec(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_float = INTTOFLOAT(a_operands) - 1.0f;
  a_operands->m_type = GM_FLOAT; 
}
void GM_CDECL gmFloatOpLT(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_int = (INTTOFLOAT(a_operands) < INTTOFLOAT(a_operands + 1));
  a_operands->m_type = GM_INT; 
}
void GM_CDECL gmFloatOpGT(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_int = (INTTOFLOAT(a_operands) > INTTOFLOAT(a_operands + 1));
  a_operands->m_type = GM_INT; 
}
void GM_CDECL gmFloatOpLTE(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_int = (INTTOFLOAT(a_operands) <= INTTOFLOAT(a_operands + 1));
  a_operands->m_type = GM_INT; 
}
void GM_CDECL gmFloatOpGTE(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_int = (INTTOFLOAT(a_operands) >= INTTOFLOAT(a_operands + 1));
  a_operands->m_type = GM_INT; 
}
void GM_CDECL gmFloatOpEQ(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_int = (INTTOFLOAT(a_operands) == INTTOFLOAT(a_operands + 1));
  a_operands->m_type = GM_INT; 
}
void GM_CDECL gmFloatOpNEQ(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_int = (INTTOFLOAT(a_operands) != INTTOFLOAT(a_operands + 1));
  a_operands->m_type = GM_INT; 
}
void GM_CDECL gmFloatOpNEG(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_float = -INTTOFLOAT(a_operands);
  a_operands->m_type = GM_FLOAT; 
}
void GM_CDECL gmFloatOpPOS(gmThread * a_thread, gmVariable * a_operands)
{
}
void GM_CDECL gmFloatOpNOT(gmThread * a_thread, gmVariable * a_operands)
{
  if(a_operands->m_value.m_float == 0.0f)
  {
    a_operands->m_value.m_int = 1; a_operands->m_type = GM_INT;
  }
  else
  {
    a_operands->m_value.m_int = 0; a_operands->m_type = GM_INT;
  }
}


//
// GM_VEC2
//

void GM_CDECL gmVec2OpAdd(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC2(v0,0);
	GM_OP_VEC2(v1,1);
	gmVec2 res = {v0.x+v1.x, v0.y+v1.y};
	a_operands[0].SetVec2(res);
}
void GM_CDECL gmVec2OpSub(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC2(v0,0);
	GM_OP_VEC2(v1,1);
	gmVec2 res = {v0.x-v1.x, v0.y-v1.y};
	a_operands[0].SetVec2(res);
}
void GM_CDECL gmVec2OpMul(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC2(vec,0);
	v2 thisVal = vec;

	gmType type = a_operands[1].m_type;

	if ( type == GM_FLOAT || type == GM_INT )
	{
		float coeff = 1.0f;
		if ( type == GM_INT ) coeff = (float)a_operands[1].GetInt();
		else coeff = (float)a_operands[1].GetFloat();
		a_operands[0].SetVec2( thisVal * coeff );
	}
	else if ( type == GM_VEC2 )
	{
		a_operands[0].SetVec2( thisVal * a_operands[1].GetVec2() );
	}
	else
	{
		a_operands[0].Nullify();
	}
}
void GM_CDECL gmVec2OpDiv(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC2(vec,0);
	v2 thisVal = vec;

	gmType type = a_operands[1].m_type;

	if ( type == GM_FLOAT || type == GM_INT )
	{
		float coeff = 1.0f;
		if ( type == GM_INT ) coeff = (float)a_operands[1].GetInt();
		else coeff = (float)a_operands[1].GetFloat();
		a_operands[0].SetVec2( thisVal / coeff );
	}
	else if ( type == GM_VEC2 )
	{
		a_operands[0].SetVec2( thisVal / a_operands[1].GetVec2() );
	}
	else
	{
		a_operands[0].Nullify();
	}
}
void GM_CDECL gmVec2OpEQ(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC2(v0,0);
	GM_OP_VEC2(v1,1);
	int res = v0.x == v1.x && v0.y == v1.x;
	a_operands[0].SetInt(res);
}
void GM_CDECL gmVec2OpNEQ(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC2(v0,0);
	GM_OP_VEC2(v1,1);
	int res = (v0.x != v1.x) || (v0.y != v1.y);
	a_operands[0].SetInt(res);
}
void GM_CDECL gmVec2OpNEG(gmThread * a_thread, gmVariable * a_operands)
{
	a_operands[0].SetVec2( -v2(a_operands[0].m_value.m_v2 ) );
}

void GM_CDECL gmVec2OpPOS(gmThread * a_thread, gmVariable * a_operands)
{
}
void GM_CDECL gmVec2GetDot(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC2( thisVal, 0 );
	GM_OP_STR_PTR(memberStr, 1);

	if( strlen(memberStr) == 2 )
	{
		v2 result;
		
		if ( stricmp( memberStr, "xx") == 0 ) result =  v2(thisVal.x, thisVal.x);
		else if ( stricmp( memberStr, "xy") == 0 ) result =  v2(thisVal.x, thisVal.y);
		else if ( stricmp( memberStr, "yx") == 0 ) result =  v2(thisVal.y, thisVal.x);
		else if ( stricmp( memberStr, "yy") == 0 ) result =  v2(thisVal.y, thisVal.y);		
		else a_operands[0].Nullify();

		a_operands[0].SetVec2(result);
	}

	else if( strlen(memberStr) == 1 )
	{	
		if ( stricmp( memberStr, "x") == 0 ) a_operands[0].SetFloat(thisVal.x);
		else if ( stricmp( memberStr, "y") == 0 ) a_operands[0].SetFloat(thisVal.y);
		else a_operands[0].Nullify();
	}
	else
	{
		a_operands[0].Nullify();
	}
}
void GM_CDECL gmVec2SetDot(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC2( thisVal, 0 );
	GM_OP_STR_PTR(memberStr, 2);

	// get float val
	float newVal = 0.0f;
	if(a_operands[1].m_type == GM_FLOAT) newVal = a_operands[1].m_value.m_float;
	else if(a_operands[1].m_type == GM_INT) newVal = (float)a_operands[1].m_value.m_int;
	else a_thread->GetMachine()->GetLog().LogEntry( "Setting v2 property '%s' as invalid type", memberStr );

	// set member
	if ( stricmp( memberStr, "x") == 0 ) thisVal.x = newVal;
	else if ( stricmp( memberStr, "y") == 0 ) thisVal.y = newVal;
	else a_thread->GetMachine()->GetLog().LogEntry( "Setting invalid property '%s'", memberStr );
}


//
// GM_VEC3
//

void GM_CDECL gmVec3OpAdd(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC3(v0,0);
	GM_OP_VEC3(v1,1);
	gmVec3 res = {v0.x+v1.x, v0.y+v1.y, v0.z+v1.z};
	a_operands[0].SetVec3(res);
}
void GM_CDECL gmVec3OpSub(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC3(v0,0);
	GM_OP_VEC3(v1,1);
	gmVec3 res = {v0.x-v1.x, v0.y-v1.y, v0.z-v1.z};
	a_operands[0].SetVec3(res);
}
void GM_CDECL gmVec3OpMul(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC3(vec,0);
	v3 thisVal = vec;

	gmType type = a_operands[1].m_type;

	if ( type == GM_FLOAT || type == GM_INT )
	{
		float coeff = 1.0f;
		if ( type == GM_INT ) coeff = (float)a_operands[1].GetInt();
		else coeff = (float)a_operands[1].GetFloat();
		a_operands[0].SetVec3( thisVal * coeff );
	}
	else if ( type == GM_VEC3 )
	{
		a_operands[0].SetVec3( thisVal * a_operands[1].GetVec3() );
	}
	else
	{
		a_operands[0].Nullify();
	}
}
void GM_CDECL gmVec3OpDiv(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC3(vec,0);
	v3 thisVal = vec;

	gmType type = a_operands[1].m_type;

	if ( type == GM_FLOAT || type == GM_INT )
	{
		float coeff = 1.0f;
		if ( type == GM_INT ) coeff = (float)a_operands[1].GetInt();
		else coeff = (float)a_operands[1].GetFloat();
		a_operands[0].SetVec3( thisVal / coeff );
	}
	else if ( type == GM_VEC3 )
	{
		a_operands[0].SetVec3( thisVal / a_operands[1].GetVec3() );
	}
	else
	{
		a_operands[0].Nullify();
	}
}
void GM_CDECL gmVec3OpEQ(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC3(v0,0);
	GM_OP_VEC3(v1,1);
	int res = v0.x == v1.x && v0.y == v1.x && v0.z == v1.z;
	a_operands[0].SetInt(res);
}
void GM_CDECL gmVec3OpNEQ(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC3(v0,0);
	GM_OP_VEC3(v1,1);
	int res = (v0.x != v1.x) || (v0.y != v1.y) || (v0.z != v1.z);
	a_operands[0].SetInt(res);
}
void GM_CDECL gmVec3OpNEG(gmThread * a_thread, gmVariable * a_operands)
{
	a_operands[0].SetVec3( -v3(a_operands[0].m_value.m_v3 ) );
}

void GM_CDECL gmVec3OpPOS(gmThread * a_thread, gmVariable * a_operands)
{
}
void GM_CDECL gmVec3GetDot(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC3( thisVal, 0 );
	GM_OP_STR_PTR(memberStr, 1);

	if( strlen(memberStr) == 3 )
	{
		v3 result;

		if ( stricmp( memberStr, "xxx") == 0 ) result = v3(thisVal.x, thisVal.x, thisVal.x);
		else if ( stricmp( memberStr, "xxy") == 0 ) result = v3(thisVal.x, thisVal.x, thisVal.y);
		else if ( stricmp( memberStr, "xxz") == 0 ) result = v3(thisVal.x, thisVal.x, thisVal.z);
		else if ( stricmp( memberStr, "xyx") == 0 ) result = v3(thisVal.x, thisVal.y, thisVal.x);
		else if ( stricmp( memberStr, "xyy") == 0 ) result = v3(thisVal.x, thisVal.y, thisVal.y);
		else if ( stricmp( memberStr, "xyz") == 0 ) result = v3(thisVal.x, thisVal.y, thisVal.z);
		else if ( stricmp( memberStr, "xzx") == 0 ) result = v3(thisVal.x, thisVal.z, thisVal.x);
		else if ( stricmp( memberStr, "xzy") == 0 ) result = v3(thisVal.x, thisVal.z, thisVal.y);
		else if ( stricmp( memberStr, "xzz") == 0 ) result = v3(thisVal.x, thisVal.z, thisVal.z);
		else if ( stricmp( memberStr, "yxx") == 0 ) result = v3(thisVal.y, thisVal.x, thisVal.x);
		else if ( stricmp( memberStr, "yxy") == 0 ) result = v3(thisVal.y, thisVal.x, thisVal.y);
		else if ( stricmp( memberStr, "yxz") == 0 ) result = v3(thisVal.y, thisVal.x, thisVal.z);
		else if ( stricmp( memberStr, "yyx") == 0 ) result = v3(thisVal.y, thisVal.y, thisVal.x);
		else if ( stricmp( memberStr, "yyy") == 0 ) result = v3(thisVal.y, thisVal.y, thisVal.y);
		else if ( stricmp( memberStr, "yyz") == 0 ) result = v3(thisVal.y, thisVal.y, thisVal.z);
		else if ( stricmp( memberStr, "yzx") == 0 ) result = v3(thisVal.y, thisVal.z, thisVal.x);
		else if ( stricmp( memberStr, "yzy") == 0 ) result = v3(thisVal.y, thisVal.z, thisVal.y);
		else if ( stricmp( memberStr, "yzz") == 0 ) result = v3(thisVal.y, thisVal.z, thisVal.z);
		else if ( stricmp( memberStr, "zxx") == 0 ) result = v3(thisVal.z, thisVal.x, thisVal.x);
		else if ( stricmp( memberStr, "zxy") == 0 ) result = v3(thisVal.z, thisVal.x, thisVal.y);
		else if ( stricmp( memberStr, "zxz") == 0 ) result = v3(thisVal.z, thisVal.x, thisVal.z);
		else if ( stricmp( memberStr, "zyx") == 0 ) result = v3(thisVal.z, thisVal.y, thisVal.x);
		else if ( stricmp( memberStr, "zyy") == 0 ) result = v3(thisVal.z, thisVal.y, thisVal.y);
		else if ( stricmp( memberStr, "zyz") == 0 ) result = v3(thisVal.z, thisVal.y, thisVal.z);
		else if ( stricmp( memberStr, "zzx") == 0 ) result = v3(thisVal.z, thisVal.z, thisVal.x);
		else if ( stricmp( memberStr, "zzy") == 0 ) result = v3(thisVal.z, thisVal.z, thisVal.y);
		else if ( stricmp( memberStr, "zzz") == 0 ) result = v3(thisVal.z, thisVal.z, thisVal.z);
		else a_operands[0].Nullify();

		a_operands[0].SetVec3(result);
	}

	else if( strlen(memberStr) == 2 )
	{
		v2 result;

		 if ( stricmp( memberStr, "xx") == 0 ) result =  v2(thisVal.x, thisVal.x);
		else if ( stricmp( memberStr, "xy") == 0 ) result =  v2(thisVal.x, thisVal.y);
		else if ( stricmp( memberStr, "xz") == 0 ) result =  v2(thisVal.x, thisVal.z);
		else if ( stricmp( memberStr, "yx") == 0 ) result =  v2(thisVal.y, thisVal.x);
		else if ( stricmp( memberStr, "yy") == 0 ) result =  v2(thisVal.y, thisVal.y);
		else if ( stricmp( memberStr, "yz") == 0 ) result =  v2(thisVal.y, thisVal.z);
		else if ( stricmp( memberStr, "zx") == 0 ) result =  v2(thisVal.z, thisVal.x);
		else if ( stricmp( memberStr, "zy") == 0 ) result =  v2(thisVal.z, thisVal.y);
		else if ( stricmp( memberStr, "zz") == 0 ) result =  v2(thisVal.z, thisVal.z);
		else a_operands[0].Nullify();

		a_operands[0].SetVec2(result);
	}

	else if( strlen(memberStr) == 1 )
	{	
		if ( stricmp( memberStr, "x") == 0 ) a_operands[0].SetFloat(thisVal.x);
		else if ( stricmp( memberStr, "y") == 0 ) a_operands[0].SetFloat(thisVal.y);
		else if ( stricmp( memberStr, "z") == 0 ) a_operands[0].SetFloat(thisVal.z);
		else a_operands[0].Nullify();
	}
	else
	{
		a_operands[0].Nullify();
	}
}
void GM_CDECL gmVec3SetDot(gmThread * a_thread, gmVariable * a_operands)
{
	GM_OP_VEC3( thisVal, 0 );
	GM_OP_STR_PTR(memberStr, 2);

	// get float val
	float newVal = 0.0f;
	if(a_operands[1].m_type == GM_FLOAT) newVal = a_operands[1].m_value.m_float;
	else if(a_operands[1].m_type == GM_INT) newVal = (float)a_operands[1].m_value.m_int;
	else a_thread->GetMachine()->GetLog().LogEntry( "Setting v2 property '%s' as invalid type", memberStr );

	// set member
	if ( stricmp( memberStr, "x") == 0 ) thisVal.x = newVal;
	else if ( stricmp( memberStr, "y") == 0 ) thisVal.y = newVal;
	else if ( stricmp( memberStr, "z") == 0 ) thisVal.z = newVal;
	else a_thread->GetMachine()->GetLog().LogEntry( "Setting invalid property %s", memberStr );
}

//
// GM_STRING
//

#define GMSTRING_BUFFERSIZE 64

// we could use gmVariable::AsString here, but this is for types <= string.... is a little more efficient.
// a_buffer must be >= 64
inline const char * gmUnknownToString(gmMachine * a_machine, gmVariable * a_unknown, char * a_buffer, int * a_len = NULL)
{
  if(a_unknown->m_type == GM_STRING)
  {
    gmStringObject * str = (gmStringObject *) GM_MOBJECT(a_machine, a_unknown->m_value.m_ref);
    if(a_len) { *a_len = str->GetLength(); }
    return (const char *) *str;
  }
  if(a_unknown->m_type == GM_INT)
  {
    sprintf(a_buffer, "%d", a_unknown->m_value.m_int); // this won't be > 64 chars
  }
  else if(a_unknown->m_type == GM_FLOAT)
  {
    sprintf(a_buffer, "%f", a_unknown->m_value.m_float); // this won't be > 64 chars
  }
  else if (a_unknown->m_type == GM_VEC2)
  {
	  sprintf(a_buffer, "v2(%f, %f)", a_unknown->m_value.m_v2.x, a_unknown->m_value.m_v2.y); // this won't be > 64 chars
  }
  else if (a_unknown->m_type == GM_VEC3)
  {
	  sprintf(a_buffer, "v3(%f, %f, %f)", a_unknown->m_value.m_v3.x, a_unknown->m_value.m_v3.y, a_unknown->m_value.m_v3.z); // this won't be > 64 chars
  }

  else
  {
    strcpy(a_buffer, "null");
  }
  if(a_len) { *a_len = (int)strlen(a_buffer); }
  return a_buffer;
}
void GM_CDECL gmStringOpAdd(gmThread * a_thread, gmVariable * a_operands)
{
  gmMachine * machine = a_thread->GetMachine();
  char buffer1[GMSTRING_BUFFERSIZE];
  char buffer2[GMSTRING_BUFFERSIZE];
  int len1 = 0, len2 = 0;
  const char * str1 = gmUnknownToString(machine, a_operands, buffer1, &len1);
  const char * str2 = gmUnknownToString(machine, a_operands + 1, buffer2, &len2);
  char * buffer = (char *) alloca(len1 + len2 + 1);
  memcpy(buffer, str1, len1);
  memcpy(buffer + len1, str2, len2 + 1);
  a_thread->SetTop(a_operands); // so the garbage collector works
  a_operands->m_type = GM_STRING;
  a_operands->m_value.m_ref = (gmptr) machine->AllocStringObject(buffer, len1 + len2);
}
void GM_CDECL gmStringOpLT(gmThread * a_thread, gmVariable * a_operands)
{
  gmMachine * machine = a_thread->GetMachine();
  char buffer1[GMSTRING_BUFFERSIZE];
  char buffer2[GMSTRING_BUFFERSIZE];
  const char * str1 = gmUnknownToString(machine, a_operands, buffer1);
  const char * str2 = gmUnknownToString(machine, a_operands + 1, buffer2);
  int res = strcmp(str1, str2);
  a_operands->m_type = GM_INT;
  a_operands->m_value.m_ref = (res == -1) ? 1 : 0;
}
void GM_CDECL gmStringOpGT(gmThread * a_thread, gmVariable * a_operands)
{
  gmMachine * machine = a_thread->GetMachine();
  char buffer1[GMSTRING_BUFFERSIZE];
  char buffer2[GMSTRING_BUFFERSIZE];
  const char * str1 = gmUnknownToString(machine, a_operands, buffer1);
  const char * str2 = gmUnknownToString(machine, a_operands + 1, buffer2);
  int res = strcmp(str1, str2);
  a_operands->m_type = GM_INT;
  a_operands->m_value.m_ref = (res == 1) ? 1 : 0;
}
void GM_CDECL gmStringOpLTE(gmThread * a_thread, gmVariable * a_operands)
{
  gmMachine * machine = a_thread->GetMachine();
  char buffer1[GMSTRING_BUFFERSIZE];
  char buffer2[GMSTRING_BUFFERSIZE];
  const char * str1 = gmUnknownToString(machine, a_operands, buffer1);
  const char * str2 = gmUnknownToString(machine, a_operands + 1, buffer2);
  int res = strcmp(str1, str2);
  a_operands->m_type = GM_INT;
  a_operands->m_value.m_ref = (res == 1) ? 0 : 1;
}
void GM_CDECL gmStringOpGTE(gmThread * a_thread, gmVariable * a_operands)
{
  gmMachine * machine = a_thread->GetMachine();
  char buffer1[GMSTRING_BUFFERSIZE];
  char buffer2[GMSTRING_BUFFERSIZE];
  const char * str1 = gmUnknownToString(machine, a_operands, buffer1);
  const char * str2 = gmUnknownToString(machine, a_operands + 1, buffer2);
  int res = strcmp(str1, str2);
  a_operands->m_type = GM_INT;
  a_operands->m_value.m_ref = (res == -1) ? 0 : 1;
}
void GM_CDECL gmStringOpEQ(gmThread * a_thread, gmVariable * a_operands)
{
  gmMachine * machine = a_thread->GetMachine();
  char buffer1[GMSTRING_BUFFERSIZE];
  char buffer2[GMSTRING_BUFFERSIZE];
  const char * str1 = gmUnknownToString(machine, a_operands, buffer1);
  const char * str2 = gmUnknownToString(machine, a_operands + 1, buffer2);
  int res = strcmp(str1, str2);
  a_operands->m_type = GM_INT;
  a_operands->m_value.m_ref = (res == 0) ? 1 : 0;
}
void GM_CDECL gmStringOpNEQ(gmThread * a_thread, gmVariable * a_operands)
{
  gmMachine * machine = a_thread->GetMachine();
  char buffer1[GMSTRING_BUFFERSIZE];
  char buffer2[GMSTRING_BUFFERSIZE];
  const char * str1 = gmUnknownToString(machine, a_operands, buffer1);
  const char * str2 = gmUnknownToString(machine, a_operands + 1, buffer2);
  int res = strcmp(str1, str2);
  a_operands->m_type = GM_INT;
  a_operands->m_value.m_ref = (res == 0) ? 0 : 1;
}

void GM_CDECL gmStringOpNOT(gmThread * a_thread, gmVariable * a_operands)
{
  a_operands->m_value.m_int = 0; a_operands->m_type = GM_INT;
}

//
// GM_TABLE
//

void GM_CDECL gmTableGetDot(gmThread * a_thread, gmVariable * a_operands)
{
  gmTableObject * table = (gmTableObject *) GM_OBJECT(a_operands->m_value.m_ref);
  *a_operands = table->Get(a_operands[1]);
}
void GM_CDECL gmTableSetDot(gmThread * a_thread, gmVariable * a_operands)
{
  gmMachine * machine = a_thread->GetMachine();
  gmTableObject * table = (gmTableObject *) GM_MOBJECT(machine, a_operands->m_value.m_ref);
  table->Set(machine, a_operands[2], a_operands[1]);
}
void GM_CDECL gmTableGetInd(gmThread * a_thread, gmVariable * a_operands)
{
  gmTableObject * table = (gmTableObject *) GM_OBJECT(a_operands->m_value.m_ref);
  *a_operands = table->Get(a_operands[1]);
}
void GM_CDECL gmTableSetInd(gmThread * a_thread, gmVariable * a_operands)
{
  gmMachine * machine = a_thread->GetMachine();
  gmTableObject * table = (gmTableObject *) GM_MOBJECT(machine, a_operands->m_value.m_ref);
  table->Set(machine, a_operands[1], a_operands[2]);
}

//
// GM_USER
//

//
// MISC.
//

void GM_CDECL gmRefOpEQ(gmThread * a_thread, gmVariable * a_operands)
{
  if(a_operands[0].m_type == a_operands[1].m_type && a_operands[0].m_value.m_ref == a_operands[1].m_value.m_ref)
  {
    a_operands->m_type = GM_INT;
    a_operands->m_value.m_int = 1;
  }
  else
  {
    a_operands->m_type = GM_INT;
    a_operands->m_value.m_int = 0;
  }
}
void GM_CDECL gmRefOpNEQ(gmThread * a_thread, gmVariable * a_operands)
{
  if(a_operands[0].m_type == a_operands[1].m_type && a_operands[0].m_value.m_ref == a_operands[1].m_value.m_ref)
  {
    a_operands->m_type = GM_INT;
    a_operands->m_value.m_int = 0;
  }
  else
  {
    a_operands->m_type = GM_INT;
    a_operands->m_value.m_int = 1;
  }
}
void GM_CDECL gmRefOpNOT(gmThread * a_thread, gmVariable * a_operands)
{
  if(a_operands->m_type == GM_NULL)
  {
    a_operands->m_type = GM_INT;
    a_operands->m_value.m_int = 1;
  }
  else
  {
    a_operands->m_type = GM_INT;
    a_operands->m_value.m_int = 0;
  }
}



void gmInitBasicType(gmType a_type, gmOperatorFunction * a_operators)
{
  memset(a_operators, 0, sizeof(gmOperatorFunction) * O_MAXOPERATORS);

  if(a_type == GM_NULL)
  {
    a_operators[O_EQ]     = gmRefOpEQ;
    a_operators[O_NEQ]    = gmRefOpNEQ;
    a_operators[O_NOT]    = gmRefOpNOT;
  }
  else if(a_type == GM_INT)
  {
    a_operators[O_ADD]            = gmIntOpAdd;
    a_operators[O_SUB]            = gmIntOpSub;
    a_operators[O_MUL]            = gmIntOpMul;
    a_operators[O_DIV]            = gmIntOpDiv;
    a_operators[O_REM]            = gmIntOpRem;
    a_operators[O_BIT_OR]         = gmIntOpBitOr;
    a_operators[O_BIT_XOR]        = gmIntOpBitXor;
    a_operators[O_BIT_AND]        = gmIntOpBitAnd;
    a_operators[O_BIT_SHIFTLEFT]  = gmIntOpBitShiftLeft;
    a_operators[O_BIT_SHIFTRIGHT] = gmIntOpBitShiftRight;
    a_operators[O_BIT_INV]        = gmIntOpInv;
    a_operators[O_LT]             = gmIntOpLT;
    a_operators[O_GT]             = gmIntOpGT;
    a_operators[O_LTE]            = gmIntOpLTE;
    a_operators[O_GTE]            = gmIntOpGTE;
    a_operators[O_EQ]             = gmIntOpEQ;
    a_operators[O_NEQ]            = gmIntOpNEQ;
    a_operators[O_NEG]            = gmIntOpNEG;
    a_operators[O_POS]            = gmIntOpPOS;
    a_operators[O_NOT]            = gmIntOpNOT;
  }
  else if(a_type == GM_FLOAT)
  {
    a_operators[O_ADD]    = gmFloatOpAdd;
    a_operators[O_SUB]    = gmFloatOpSub;
    a_operators[O_MUL]    = gmFloatOpMul;
    a_operators[O_DIV]    = gmFloatOpDiv;
    a_operators[O_REM]    = gmFloatOpRem;
    a_operators[O_LT]     = gmFloatOpLT;
    a_operators[O_GT]     = gmFloatOpGT;
    a_operators[O_LTE]    = gmFloatOpLTE;
    a_operators[O_GTE]    = gmFloatOpGTE;
    a_operators[O_EQ]     = gmFloatOpEQ;
    a_operators[O_NEQ]    = gmFloatOpNEQ;
    a_operators[O_NEG]    = gmFloatOpNEG;
    a_operators[O_POS]    = gmFloatOpPOS;
    a_operators[O_NOT]    = gmFloatOpNOT;
  }
  else if(a_type == GM_VEC2)
  {
	  a_operators[O_ADD]    = gmVec2OpAdd;
	  a_operators[O_SUB]    = gmVec2OpSub;
	  a_operators[O_MUL]    = gmVec2OpMul;
	  a_operators[O_DIV]    = gmVec2OpDiv;
	  a_operators[O_EQ]     = gmVec2OpEQ;
	  a_operators[O_NEQ]    = gmVec2OpNEQ;
	  a_operators[O_NEG]    = gmVec2OpNEG;
	  a_operators[O_POS]    = gmVec2OpPOS;
	  a_operators[O_GETDOT] = gmVec2GetDot;
	  a_operators[O_SETDOT] = gmVec2SetDot;
  }
  else if(a_type == GM_VEC3)
  {
	  a_operators[O_ADD]    = gmVec3OpAdd;
	  a_operators[O_SUB]    = gmVec3OpSub;
	  a_operators[O_MUL]    = gmVec3OpMul;
	  a_operators[O_DIV]    = gmVec3OpDiv;
	  a_operators[O_EQ]     = gmVec3OpEQ;
	  a_operators[O_NEQ]    = gmVec3OpNEQ;
	  a_operators[O_NEG]    = gmVec3OpNEG;
	  a_operators[O_POS]    = gmVec3OpPOS;
	  a_operators[O_GETDOT] = gmVec3GetDot;
	  a_operators[O_SETDOT] = gmVec3SetDot;
  }
  else if(a_type == GM_STRING)
  {
    a_operators[O_ADD]    = gmStringOpAdd;
    a_operators[O_LT]     = gmStringOpLT;
    a_operators[O_GT]     = gmStringOpGT;
    a_operators[O_LTE]    = gmStringOpLTE;
    a_operators[O_GTE]    = gmStringOpGTE;
    a_operators[O_EQ]     = gmStringOpEQ;
    a_operators[O_NEQ]    = gmStringOpNEQ;
    a_operators[O_NOT]    = gmStringOpNOT;    
  }
  else if(a_type == GM_TABLE)
  {
    a_operators[O_GETDOT] = gmTableGetDot;
    a_operators[O_SETDOT] = gmTableSetDot;
    a_operators[O_GETIND] = gmTableGetInd;
    a_operators[O_SETIND] = gmTableSetInd;
    a_operators[O_EQ]     = gmRefOpEQ;
    a_operators[O_NEQ]    = gmRefOpNEQ;
    a_operators[O_NOT]    = gmRefOpNOT;
  }
  else if(a_type == GM_FUNCTION)
  {
    a_operators[O_EQ]     = gmRefOpEQ;
    a_operators[O_NEQ]    = gmRefOpNEQ;
    a_operators[O_NOT]    = gmRefOpNOT;
  }
  else if(a_type >= GM_USER)
  {
    a_operators[O_EQ]     = gmRefOpEQ;
    a_operators[O_NEQ]    = gmRefOpNEQ;
    a_operators[O_NOT]    = gmRefOpNOT;
  }
}

