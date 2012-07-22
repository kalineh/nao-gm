/*
    _____               __  ___          __            ____        _      __
   / ___/__ ___ _  ___ /  |/  /__  ___  / /_____ __ __/ __/_______(_)__  / /_
  / (_ / _ `/  ' \/ -_) /|_/ / _ \/ _ \/  '_/ -_) // /\ \/ __/ __/ / _ \/ __/
  \___/\_,_/_/_/_/\__/_/  /_/\___/_//_/_/\_\\__/\_, /___/\__/_/ /_/ .__/\__/
                                               /___/             /_/
                                             
  See Copyright Notice in gmMachine.h

*/

#include "gmConfig.h"
#include "gmMathLib.h"
#include "gmThread.h"
#include "gmMachine.h"
#include "gmHelpers.h"
#include "gmUtil.h"
#include <math.h>

#include <math/v2.h>
#include <math/v3.h>
#include <math/ColorUtil.h>


//
// Conversion
//

using namespace funk;

int GM_CDECL gmfToString(gmThread * a_thread)
{
  const gmVariable * var = a_thread->GetThis();

  if(GM_INT == var->m_type)
  {
    char numberAsStringBuffer[64];
    sprintf(numberAsStringBuffer, "%d", var->m_value.m_int); // this won't be > 64 chars
    a_thread->PushNewString(numberAsStringBuffer);
  }
  else if (GM_FLOAT == var->m_type)
  {
    char numberAsStringBuffer[64];

    sprintf(numberAsStringBuffer, "%f", var->m_value.m_float); // this won't be > 64 chars

    a_thread->PushNewString(numberAsStringBuffer);
  }
  else if (GM_VEC2 == var->m_type)
  {
	  char numberAsStringBuffer[64];
	  sprintf(numberAsStringBuffer, "v2(%f, %f)", var->m_value.m_v2.x, var->m_value.m_v2.y ); // this won't be > 64 chars
	  a_thread->PushNewString(numberAsStringBuffer);
  }

  else if (GM_VEC3 == var->m_type)
  {
	  char numberAsStringBuffer[64];
	  sprintf(numberAsStringBuffer, "v3(%f, %f, %f)", var->m_value.m_v3.x, var->m_value.m_v3.y, var->m_value.m_v3.z ); // this won't be > 64 chars
	  a_thread->PushNewString(numberAsStringBuffer);
  }

  else if (GM_STRING == var->m_type)
  {
    a_thread->PushString( (gmStringObject *) GM_OBJECT(var->m_value.m_ref) );
  }
  else
  {
    return GM_EXCEPTION;
  }

  
  return GM_OK;
}



int GM_CDECL gmfToFloat(gmThread * a_thread)
{
  const gmVariable * var = a_thread->GetThis();

  if(GM_INT == var->m_type)
  {
    a_thread->PushFloat((float)var->m_value.m_int);
  }
  else if (GM_FLOAT == var->m_type)
  {
    a_thread->PushFloat(var->m_value.m_float);
  }
  else if (GM_STRING == var->m_type)
  {
    gmStringObject * strObj = (gmStringObject *) GM_OBJECT(var->m_value.m_ref);
    const char * cstr = * strObj;

    a_thread->PushFloat( (float)atof(cstr) );
  }
  else
  {
    //a_thread->PushFloat( 0.0f );
    return GM_EXCEPTION;
  }
  
  return GM_OK;
}



int GM_CDECL gmfToInt(gmThread * a_thread)
{
  const gmVariable * var = a_thread->GetThis();

  if(GM_INT == var->m_type)
  {
    a_thread->PushInt(var->m_value.m_int);
  }
  else if (GM_FLOAT == var->m_type)
  {
    a_thread->PushInt( (int)var->m_value.m_float);
  }
  else if (GM_STRING == var->m_type)
  {
    gmStringObject * strObj = (gmStringObject *) GM_OBJECT(var->m_value.m_ref);
    const char * cstr = * strObj;

    a_thread->PushInt( atoi(cstr) );
  }
  else
  {
    //a_thread->PushInt( 0 );
    return GM_EXCEPTION;
  }
  
  return GM_OK;
}



//
// Math
//



// int randint(int a_min, int a_max);
// returned number is >= a_min and <= a_max (inclusive of max)
static int GM_CDECL gmfRandInt(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(2);
  GM_CHECK_INT_PARAM(min, 0);
  GM_CHECK_INT_PARAM(max, 1);
  
  a_thread->PushInt( gmRandomInt(min, max) );

  return GM_OK;
}



// float randfloat(float a_min, float a_max);
// returned number is >= a_min and < a_max (exclusive of max)
static int GM_CDECL gmfRandFloat(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(2);
  GM_CHECK_FLOAT_PARAM(min, 0);
  GM_CHECK_FLOAT_PARAM(max, 1);

  a_thread->PushFloat( gmRandomFloat(min, max) );

  return GM_OK;
}



// void randseed(int a_seed);
static int GM_CDECL gmfRandSeed(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);
  GM_CHECK_INT_PARAM(seed, 0);
  
  srand(seed);
  
  return GM_OK;
}



static int GM_CDECL gmfAbs(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  if(a_thread->ParamType(0) == GM_INT)
  {
    int intValue = a_thread->Param(0).m_value.m_int;
    a_thread->PushInt(abs(intValue));

    return GM_OK;
  }
  else if(a_thread->ParamType(0) == GM_FLOAT)
  {
    float floatValue = a_thread->Param(0).m_value.m_float;
    a_thread->PushFloat((float)fabsf(floatValue));

    return GM_OK;
  }
  else if (GM_IS_PARAM_VEC2(0))
  {
	  GM_VEC2_PARAM(val,0);
	  a_thread->PushVec2( abs(val) );
  }
  else if (GM_IS_PARAM_VEC3(0))
  {
	  GM_VEC3_PARAM(val,0);
	  a_thread->PushVec3( abs(val) );
  }

  return GM_EXCEPTION;
}



static int GM_CDECL gmfSqrt(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  if(a_thread->ParamType(0) == GM_INT)
  {
    int intValue = a_thread->Param(0).m_value.m_int;
    a_thread->PushInt((int)sqrtf((float)intValue));

    return GM_OK;
  }
  else if(a_thread->ParamType(0) == GM_FLOAT)
  {
    float floatValue = a_thread->Param(0).m_value.m_float;
    a_thread->PushFloat(sqrtf(floatValue));

    return GM_OK;
  }

  return GM_EXCEPTION;
}



static int GM_CDECL gmfPower(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(2);

  int minType = gmMin<int>(a_thread->ParamType(0), a_thread->ParamType(1));
  if(minType < GM_INT)
  {
    return GM_EXCEPTION;
  }
  int maxType = gmMax<int>(a_thread->ParamType(0), a_thread->ParamType(1));

  if(maxType == GM_INT)
  {
    int valX = a_thread->Param(0).m_value.m_int;
    int valY = a_thread->Param(1).m_value.m_int;
    a_thread->PushInt((int)pow((float)valX, (float)valY));

    return GM_OK;
  }
  else if(maxType == GM_FLOAT)
  {
    float valX = gmGetFloatOrIntParamAsFloat(a_thread, 0);
    float valY = gmGetFloatOrIntParamAsFloat(a_thread, 1);
    a_thread->PushFloat((float)pow(valX, valY));

    return GM_OK;
  }
  else
  {
    return GM_EXCEPTION;
  }
}



static int GM_CDECL gmfFloor(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  if(a_thread->ParamType(0) == GM_INT) //Do nothing if Int
  {
    int intValue = a_thread->Param(0).m_value.m_int;
    a_thread->PushInt(intValue);

    return GM_OK;
  }
  else if(a_thread->ParamType(0) == GM_FLOAT)
  {
    float floatValue = a_thread->Param(0).m_value.m_float;
    a_thread->PushFloat(floorf(floatValue));

    return GM_OK;
  }
  else if(a_thread->ParamType(0) == GM_VEC2)
  {
	  v2 vec2Val = a_thread->Param(0).m_value.m_v2;
	  a_thread->PushVec2(v2(floorf(vec2Val.x), floorf(vec2Val.y)));

	  return GM_OK;
  }
  else if(a_thread->ParamType(0) == GM_VEC3)
  {
	  v3 vec2Val = a_thread->Param(0).m_value.m_v3;
	  a_thread->PushVec3(v3(floorf(vec2Val.x), floorf(vec2Val.y), floorf(vec2Val.z)));

	  return GM_OK;
  }

  return GM_EXCEPTION;
}



static int GM_CDECL gmfCeil(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  if(a_thread->ParamType(0) == GM_INT) //Do nothing if Int
  {
    int intValue = a_thread->Param(0).m_value.m_int;
    a_thread->PushInt(intValue);

    return GM_OK;
  }
  else if(a_thread->ParamType(0) == GM_FLOAT)
  {
    float floatValue = a_thread->Param(0).m_value.m_float;
    a_thread->PushFloat(ceilf(floatValue));

    return GM_OK;
  }

  return GM_EXCEPTION;
}



static int GM_CDECL gmfRound(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  if(a_thread->ParamType(0) == GM_INT) //Do nothing if Int
  {
    int intValue = a_thread->Param(0).m_value.m_int;
    a_thread->PushInt(intValue);

    return GM_OK;
  }
  else if(a_thread->ParamType(0) == GM_FLOAT)
  {
    float floatValue = a_thread->Param(0).m_value.m_float;
    a_thread->PushFloat(floorf(floatValue + 0.5f));

    return GM_OK;
  }

  return GM_EXCEPTION;
}



static int GM_CDECL gmfDegToRad(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  float floatValue;

  if(a_thread->ParamType(0) == GM_INT) { floatValue = (float) a_thread->Param(0).m_value.m_int; }
  else if(a_thread->ParamType(0) == GM_FLOAT) { floatValue = a_thread->Param(0).m_value.m_float; }
  else { return GM_EXCEPTION; }

  a_thread->PushFloat( floatValue * (GM_PI_VALUE / 180.0f) );

  return GM_OK;
}



static int GM_CDECL gmfRadToDeg(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  float floatValue;

  if(a_thread->ParamType(0) == GM_INT) { floatValue = (float) a_thread->Param(0).m_value.m_int; }
  else if(a_thread->ParamType(0) == GM_FLOAT) { floatValue = a_thread->Param(0).m_value.m_float; }
  else { return GM_EXCEPTION; }

  a_thread->PushFloat( floatValue * (180.0f / GM_PI_VALUE) );

  return GM_OK;
}



static int GM_CDECL gmfSin(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  float floatValue;

  if(a_thread->ParamType(0) == GM_INT) { floatValue = (float) a_thread->Param(0).m_value.m_int; }
  else if(a_thread->ParamType(0) == GM_FLOAT) { floatValue = a_thread->Param(0).m_value.m_float; }
  else { return GM_EXCEPTION; }

  a_thread->PushFloat(sinf(floatValue));

  return GM_OK;
}



static int GM_CDECL gmfASin(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  float floatValue;

  if(a_thread->ParamType(0) == GM_INT) { floatValue = (float) a_thread->Param(0).m_value.m_int; }
  else if(a_thread->ParamType(0) == GM_FLOAT) { floatValue = a_thread->Param(0).m_value.m_float; }
  else { return GM_EXCEPTION; }

  a_thread->PushFloat(asinf(floatValue));

  return GM_OK;
}



static int GM_CDECL gmfCos(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  float floatValue;

  if(a_thread->ParamType(0) == GM_INT) { floatValue = (float) a_thread->Param(0).m_value.m_int; }
  else if(a_thread->ParamType(0) == GM_FLOAT) { floatValue = a_thread->Param(0).m_value.m_float; }
  else { return GM_EXCEPTION; }

  a_thread->PushFloat(cosf(floatValue));

  return GM_OK;
}



static int GM_CDECL gmfACos(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  float floatValue;

  if(a_thread->ParamType(0) == GM_INT) { floatValue = (float) a_thread->Param(0).m_value.m_int; }
  else if(a_thread->ParamType(0) == GM_FLOAT) { floatValue = a_thread->Param(0).m_value.m_float; }
  else { return GM_EXCEPTION; }

  a_thread->PushFloat(acosf(floatValue));

  return GM_OK;
}



static int GM_CDECL gmfTan(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  float floatValue;

  if(a_thread->ParamType(0) == GM_INT) { floatValue = (float) a_thread->Param(0).m_value.m_int; }
  else if(a_thread->ParamType(0) == GM_FLOAT) { floatValue = a_thread->Param(0).m_value.m_float; }
  else { return GM_EXCEPTION; }

  a_thread->PushFloat(tanf(floatValue));

  return GM_OK;
}



static int GM_CDECL gmfATan(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(1);

  float floatValue;

  if(a_thread->ParamType(0) == GM_INT) { floatValue = (float) a_thread->Param(0).m_value.m_int; }
  else if(a_thread->ParamType(0) == GM_FLOAT) { floatValue = a_thread->Param(0).m_value.m_float; }
  else { return GM_EXCEPTION; }

  a_thread->PushFloat(atanf(floatValue));

  return GM_OK;
}



//returns the arctangent of y/x
static int GM_CDECL gmfATan2(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(2);

  float floatValueY;
  float floatValueX;

  if(a_thread->ParamType(0) == GM_INT) {floatValueY = (float) a_thread->Param(0).m_value.m_int;}
  else if(a_thread->ParamType(0) == GM_FLOAT) {floatValueY = a_thread->Param(0).m_value.m_float;}
  else {return GM_EXCEPTION;}

  if(a_thread->ParamType(1) == GM_INT) {floatValueX = (float) a_thread->Param(1).m_value.m_int;}
  else if(a_thread->ParamType(1) == GM_FLOAT) {floatValueX = a_thread->Param(1).m_value.m_float;}
  else {return GM_EXCEPTION;}

  a_thread->PushFloat(atan2f(floatValueY, floatValueX));

  return GM_OK;
}



static int GM_CDECL gmfLog(gmThread * a_thread)
{
  int numParams = GM_THREAD_ARG->GetNumParams();

  if(numParams == 1) //Natural log
  {
    if(a_thread->ParamType(0) == GM_INT) 
    {
      float floatValue = (float) a_thread->Param(0).m_value.m_int;
      a_thread->PushInt( (int) log(floatValue) );
      return GM_OK;
    }
    else if(a_thread->ParamType(0) == GM_FLOAT) 
    {
      float floatValue = (float) a_thread->Param(0).m_value.m_float;

      a_thread->PushFloat( logf(floatValue) );
      return GM_OK;
    }
    else {return GM_EXCEPTION;}
  }
  else if(numParams == 2) //Log to base params: base, value
  {
    int minType = gmMin<int>(a_thread->ParamType(0), a_thread->ParamType(1));
    if(minType < GM_INT)
    {
      return GM_EXCEPTION;
    }
    int maxType = gmMax<int>(a_thread->ParamType(0), a_thread->ParamType(1));

    if(maxType == GM_INT)
    {
      int base = a_thread->Param(0).m_value.m_int;
      int value = a_thread->Param(1).m_value.m_int;
      
      a_thread->PushInt( (int)( log10f((float)value) / log10f((float)base) ) );

      return GM_OK;
    }
    else if(maxType == GM_FLOAT)
    {
      float base = gmGetFloatOrIntParamAsFloat(a_thread, 0);
      float value = gmGetFloatOrIntParamAsFloat(a_thread, 1);
      
      a_thread->PushFloat( (float)( log10(value) / log10(base) ) );

      return GM_OK;
    }
    else
    {
      return GM_EXCEPTION;
    }
  }
  else
  {
    return GM_EXCEPTION;
  }
}



static int GM_CDECL gmfMin(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(2);

  int minType = gmMin<int>(a_thread->ParamType(0), a_thread->ParamType(1));
  if(minType < GM_INT)
  {
    return GM_EXCEPTION;
  }

  int maxType = gmMax<int>(a_thread->ParamType(0), a_thread->ParamType(1));

  if(maxType == GM_INT)
  {
    int valX = a_thread->Param(0).m_value.m_int;
    int valY = a_thread->Param(1).m_value.m_int;
    a_thread->PushInt( gmMin(valX, valY) );

    return GM_OK;
  }
  else if(maxType == GM_FLOAT)
  {
    float valX = gmGetFloatOrIntParamAsFloat(a_thread, 0);
    float valY = gmGetFloatOrIntParamAsFloat(a_thread, 1);
    a_thread->PushFloat( gmMin(valX, valY) );

    return GM_OK;
  }

  else if (GM_IS_PARAM_VEC2(0))
  {
	  GM_VEC2_PARAM(val0,0);
	  GM_CHECK_VEC2_PARAM(val1,1);
	  a_thread->PushVec2( min( val0, val1 ) );

	  return GM_OK;
  }
  else if (GM_IS_PARAM_VEC3(0))
  {
	  GM_VEC3_PARAM(val0,0);
	  GM_CHECK_VEC3_PARAM(val1,1);
	  a_thread->PushVec3( min( val0, val1 ) );

	  return GM_OK;
  }

  else
  {
    return GM_EXCEPTION;
  }
}



static int GM_CDECL gmfMax(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(2);

  int minType = gmMin<int>(a_thread->ParamType(0), a_thread->ParamType(1));
  if(minType < GM_INT)
  {
    return GM_EXCEPTION;
  }

  int maxType = gmMax<int>(a_thread->ParamType(0), a_thread->ParamType(1));

  if(maxType == GM_INT)
  {
    int valX = a_thread->Param(0).m_value.m_int;
    int valY = a_thread->Param(1).m_value.m_int;
    a_thread->PushInt( gmMax(valX, valY) );

    return GM_OK;
  }
  else if(maxType == GM_FLOAT)
  {
    float valX = gmGetFloatOrIntParamAsFloat(a_thread, 0);
    float valY = gmGetFloatOrIntParamAsFloat(a_thread, 1);
    a_thread->PushFloat( gmMax(valX, valY) );

    return GM_OK;
  }

  else if (GM_IS_PARAM_VEC2(0))
  {
	  GM_VEC2_PARAM(val0,0);
	  GM_CHECK_VEC2_PARAM(val1,1);
	  a_thread->PushVec2( max( val0, val1 ) );

	  return GM_OK;
  }
  else if (GM_IS_PARAM_VEC3(0))
  {
	  GM_VEC3_PARAM(val0,0);
	  GM_CHECK_VEC3_PARAM(val1,1);
	  a_thread->PushVec3( max( val0, val1 ) );
	
	  return GM_OK;
  }

  else
  {
    return GM_EXCEPTION;
  }
}



static int GM_CDECL gmfClamp(gmThread * a_thread)
{
  GM_CHECK_NUM_PARAMS(3);

  //params: min, value, max

  int minType = gmMin3(a_thread->ParamType(0), a_thread->ParamType(1), a_thread->ParamType(2));
  if(minType < GM_INT)
  {
    return GM_EXCEPTION;
  }

  int maxType = gmMax3(a_thread->ParamType(0), a_thread->ParamType(1), a_thread->ParamType(2));

  if(maxType == GM_INT)
  {
    int value = a_thread->Param(0).m_value.m_int;
    int limitMin = a_thread->Param(1).m_value.m_int;
    int limitMax = a_thread->Param(2).m_value.m_int;

    a_thread->PushInt( gmClamp(limitMin, value, limitMax) );

    return GM_OK;
  }
  else if(maxType == GM_FLOAT)
  {
    float value = gmGetFloatOrIntParamAsFloat(a_thread, 0);
    float limitMin = gmGetFloatOrIntParamAsFloat(a_thread, 1);
    float limitMax = gmGetFloatOrIntParamAsFloat(a_thread, 2);
	
    
    a_thread->PushFloat( gmClamp(limitMin, value, limitMax) );

    return GM_OK;
  }

  else if (GM_IS_PARAM_VEC2(0))
  {
	  GM_VEC2_PARAM(val0,0);

	  if ( a_thread->ParamType(0) == GM_INT || a_thread->ParamType(0) == GM_FLOAT )
	  {
			 GM_FLOAT_OR_INT_PARAM( min, 0, 0.0f);
			 GM_FLOAT_OR_INT_PARAM( max, 1, 0.0f);
			  a_thread->PushVec2( clamp( val0, v2(min), v2(max)) );
	  }
	  else
	  {
		  GM_CHECK_VEC2_PARAM(val1,1);
		  GM_CHECK_VEC2_PARAM(val2,2);
		  a_thread->PushVec2( clamp( val0, val1, val2 ) );
	  }

	  return GM_OK;
  }
  else if (GM_IS_PARAM_VEC3(0))
  {
	  GM_VEC3_PARAM(val0,0);

	  if ( a_thread->ParamType(0) == GM_INT || a_thread->ParamType(0) == GM_FLOAT )
	  {
		  GM_FLOAT_OR_INT_PARAM( min, 0, 0.0f);
		  GM_FLOAT_OR_INT_PARAM( max, 1, 0.0f);
		  a_thread->PushVec3( clamp( val0, v3(min), v3(max)) );
	  }
	  else
	  {
		  GM_CHECK_VEC3_PARAM(val1,1);
		  GM_CHECK_VEC3_PARAM(val2,2);
		  a_thread->PushVec3( clamp( val0, val1, val2 ) );
	  }

	  return GM_OK;
  }

  else
  {
    return GM_EXCEPTION;
  }
}

static int GM_CDECL gmfCubicPulse(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(3);

	GM_CHECK_FLOAT_OR_INT_PARAM( center,0 );
	GM_CHECK_FLOAT_OR_INT_PARAM( width, 1 );
	GM_CHECK_FLOAT_OR_INT_PARAM( x,		2 );

	x = fabs( x - center );
	float val = 0.0f;

	if ( x <= width )
	{
		x /= width;
		val = 1.0f - x*x*(3.0f-2.0f*x);
	}

	a_thread->PushFloat(val);
	return GM_OK;
}

static int GM_CDECL gmfSaturate(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(1);

	if ( GM_IS_PARAM_VEC2(0) )
	{
		GM_VEC2_PARAM(val,0);
		a_thread->PushVec2( saturate(val) );
		return GM_OK;
	}

	else if ( GM_IS_PARAM_VEC3(0) )
	{
		GM_VEC3_PARAM(val,0);
		a_thread->PushVec3( saturate(val) );
		return GM_OK;
	}
	else if ( GM_IS_PARAM_FLOAT(0) )
	{
		GM_FLOAT_PARAM(val,0, 0.0f);
		a_thread->PushFloat( saturate(val) );
		return GM_OK;
	}

	return GM_EXCEPTION;
}

static int GM_CDECL gmfNormalize(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(1);

	if ( GM_IS_PARAM_VEC2(0) )
	{
		GM_VEC2_PARAM(val,0);
		a_thread->PushVec2( normalize(val) );
		return GM_OK;
	}

	else if ( GM_IS_PARAM_VEC3(0) )
	{
		GM_VEC3_PARAM(val,0);
		a_thread->PushVec3( normalize(val) );
		return GM_OK;
	}

	return GM_EXCEPTION;
}

static int GM_CDECL gmfLength(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(1);

	if ( GM_IS_PARAM_VEC2(0) )
	{
		GM_VEC2_PARAM(val,0);
		a_thread->PushFloat( length(val) );
		return GM_OK;
	}

	else if ( GM_IS_PARAM_VEC3(0) )
	{
		GM_VEC3_PARAM(val,0);
		a_thread->PushFloat( length(val) );
		return GM_OK;
	}

	return GM_EXCEPTION;
}

static int GM_CDECL gmfLengthSqr(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(1);

	if ( GM_IS_PARAM_VEC2(0) )
	{
		GM_VEC2_PARAM(val,0);
		a_thread->PushFloat( lengthSqr(val) );
		return GM_OK;
	}

	else if ( GM_IS_PARAM_VEC3(0) )
	{
		GM_VEC3_PARAM(val,0);
		a_thread->PushFloat( lengthSqr(val) );
		return GM_OK;
	}

	return GM_EXCEPTION;
}


static int GM_CDECL gmfFmod(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(2);
	GM_CHECK_FLOAT_OR_INT_PARAM(x,0);
	GM_CHECK_FLOAT_OR_INT_PARAM(y,1);

	a_thread->PushFloat(fmod(x,y));
	return GM_OK;
}

static int GM_CDECL gmfLerp(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(3);

	if ( GM_IS_PARAM_VEC2(0) )
	{
		GM_CHECK_VEC2_PARAM( v0, 0 );
		GM_CHECK_VEC2_PARAM( v1, 1 );
		GM_CHECK_FLOAT_OR_INT_PARAM( alpha, 2 );
		a_thread->PushVec2( lerp(v0,v1,alpha) );
	}

	else if ( GM_IS_PARAM_VEC3(0) )
	{
		GM_CHECK_VEC3_PARAM( v0, 0 );
		GM_CHECK_VEC3_PARAM( v1, 1 );
		GM_CHECK_FLOAT_OR_INT_PARAM( alpha, 2 );
		a_thread->PushVec3( lerp(v0,v1,alpha) );
	}
	else
	{
		GM_CHECK_FLOAT_OR_INT_PARAM( x0,	0 );
		GM_CHECK_FLOAT_OR_INT_PARAM( x1,	1 );
		GM_CHECK_FLOAT_OR_INT_PARAM( alpha, 2 );

		a_thread->PushFloat( x0 + (x1-x0)*alpha  );
	}

	return GM_OK;
}

static int GM_CDECL gmfDot(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(2);

	if (GM_IS_PARAM_VEC2(0))
	{
		GM_VEC2_PARAM(val0,0);
		GM_CHECK_VEC2_PARAM(val1,1);
		a_thread->PushFloat( dot(val0, val1));

		return GM_OK;
	}
	else if (GM_IS_PARAM_VEC3(0))
	{
		GM_VEC3_PARAM(val0,0);
		GM_CHECK_VEC3_PARAM(val1,1);
		a_thread->PushFloat( dot(val0, val1));

		return GM_OK;
	}
	else
	{
		return GM_EXCEPTION;
	}
}

static int GM_CDECL gmfSmoothStep(gmThread* a_thread)
{
	GM_CHECK_NUM_PARAMS(3);

	GM_CHECK_FLOAT_OR_INT_PARAM( x0,	0 );
	GM_CHECK_FLOAT_OR_INT_PARAM( x1,	1 );
	GM_CHECK_FLOAT_OR_INT_PARAM( alpha, 2 );

	a_thread->PushFloat( smoothstep(x0,x1,alpha)  );

	return GM_OK;
}

static int GM_CDECL gmfStep(gmThread* a_thread)
{
	GM_CHECK_NUM_PARAMS(2);

	GM_CHECK_FLOAT_OR_INT_PARAM( edge,	0 );
	GM_CHECK_FLOAT_OR_INT_PARAM( val,	1 );

	float result = val<edge ? 0.0f : 1.0f;
	a_thread->PushFloat( result );

	return GM_OK;
}

static int GM_CDECL gmfWrap(gmThread* a_thread)
{
	GM_CHECK_NUM_PARAMS(3);

	GM_CHECK_FLOAT_OR_INT_PARAM( val,	0 );
	GM_CHECK_FLOAT_OR_INT_PARAM( minVal,1 );
	GM_CHECK_FLOAT_OR_INT_PARAM( maxVal, 2 );

	a_thread->PushFloat( wrapf(val, minVal, maxVal)  );

	return GM_OK;
}

static int GM_CDECL gmfDistance(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(2);

	if (GM_IS_PARAM_VEC2(0))
	{
		GM_VEC2_PARAM(val0,0);
		GM_CHECK_VEC2_PARAM(val1,1);
		a_thread->PushVec2( distance(val0, val1));

		return GM_OK;
	}
	else if (GM_IS_PARAM_VEC3(0))
	{
		GM_VEC3_PARAM(val0,0);
		GM_CHECK_VEC3_PARAM(val1,1);
		a_thread->PushVec3( distance(val0, val1));

		return GM_OK;
	}
	else
	{
		return GM_EXCEPTION;
	}
}

static int GM_CDECL gmfCross(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(2);

	GM_CHECK_VEC3_PARAM(val0,0);
	GM_CHECK_VEC3_PARAM(val1,1);
	a_thread->PushVec3( cross(val0, val1));

	return GM_OK;
}

static int GM_CDECL gmfHermite(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(5);

	if ( GM_IS_PARAM_VEC3(0) )
	{
		GM_CHECK_VEC3_PARAM(v0,		0);
		GM_CHECK_VEC3_PARAM(tan0,	1);
		GM_CHECK_VEC3_PARAM(v1,		2);
		GM_CHECK_VEC3_PARAM(tan1,	3);
		GM_CHECK_FLOAT_PARAM(t,		4);

		a_thread->PushVec3( hermite(v0, tan0, v1, tan1, t) );
		return GM_OK;
	}
	
	else if ( GM_IS_PARAM_VEC2(0) )
	{
		GM_CHECK_VEC2_PARAM(v0,		0);
		GM_CHECK_VEC2_PARAM(tan0,	1);
		GM_CHECK_VEC2_PARAM(v1,		2);
		GM_CHECK_VEC2_PARAM(tan1,	3);
		GM_CHECK_FLOAT_PARAM(t,		4);

		a_thread->PushVec2( hermite(v0, tan0, v1, tan1, t) );
		return GM_OK;
	}

	return GM_EXCEPTION;
}

static int GM_CDECL gmfUnitCircle(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_FLOAT_PARAM(angleRad, 0);

	a_thread->PushVec2( unitCircle(angleRad) );
	return GM_OK;
}

static int GM_CDECL gmfGaussian(gmThread * a_thread)
{
	GM_CHECK_FLOAT_PARAM(x, 0);
	GM_FLOAT_OR_INT_PARAM(mean, 1, 0.0f);
	GM_FLOAT_OR_INT_PARAM(stdDev, 2, 1.0f);

	assert(stdDev != 0.0f );

	const float coeff = 1.0f / sqrtf(2.0f*3.14159f);
	const float diff = x - mean;
	float result = (coeff/stdDev)*expf( -diff*diff/(2.0f*stdDev*stdDev) );

	a_thread->PushFloat(result);

	return GM_OK;
}

static int GM_CDECL gmfSign(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(1);

	if ( GM_IS_PARAM_FLOAT(0) )
	{
		GM_CHECK_FLOAT_PARAM(val, 0);
		a_thread->PushFloat( sign<float>(val) );
	}
	else if ( GM_IS_PARAM_INT(0) ) 
	{
		GM_CHECK_INT_PARAM(val, 0);
		a_thread->PushInt( sign<int>(val) );
	}
	else
	{
		return GM_EXCEPTION;
	}

	return GM_OK;
}

static int GM_CDECL gmfHSVtoRGB(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_VEC3_PARAM( hsv, 0 ); // radians
	
	a_thread->PushVec3( HSVtoRGB(hsv) );
	return GM_OK;
}

static int GM_CDECL gmfRGBtoHSV(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_VEC3_PARAM( rgb, 0 );

	a_thread->PushVec3( RGBtoHSV(rgb) ); // radians
	return GM_OK;
}


static int GM_CDECL gmfSelect(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(3);

	GM_CHECK_INT_PARAM( condition, 0 );

	gmVariable var0 = a_thread->Param(1);
	gmVariable var1 = a_thread->Param(2);
	gmVariable result = condition != 0 ? var0 : var1;

	a_thread->Push(result);

	return GM_OK;
}

static int GM_CDECL gmfCreateVec2(gmThread * a_thread)
{
	int numParams = a_thread->GetNumParams();

	if ( numParams == 0 )
	{
		a_thread->PushVec2( v2() );
	}

	else if ( numParams == 1 )
	{
		if ( GM_IS_PARAM_VEC2(0) )
		{
			GM_VEC2_PARAM(vec,0);
			a_thread->PushVec2( v2(vec) );
		}
		else
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( val, 0 );
			a_thread->PushVec2( v2(val) );
		}
	}

	else if ( numParams == 2 )
	{
		GM_CHECK_FLOAT_OR_INT_PARAM( val0, 0 );
		GM_CHECK_FLOAT_OR_INT_PARAM( val1, 1 );
		a_thread->PushVec2( v2(val0,val1) );
	}

	else return GM_EXCEPTION;

	return GM_OK;
}


static int GM_CDECL gmfCreateVec3(gmThread * a_thread)
{
	int numParams = a_thread->GetNumParams();

	if ( numParams == 0 )
	{
		a_thread->PushVec3( v3() );
	}

	else if ( numParams == 1 )
	{
		if ( GM_IS_PARAM_VEC3(0) )
		{
			GM_VEC3_PARAM(vec,0);
			a_thread->PushVec3( v3(vec) );
		}
		else if ( GM_IS_PARAM_VEC2(0))
		{
			GM_VEC2_PARAM(vec,0);
			a_thread->PushVec3( v3(vec.x, vec.y, 0.0f) );
		}
		else
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( val, 0 );
			a_thread->PushVec3( v3(val) );
		}
	}

	else if ( numParams == 2 )
	{
		GM_CHECK_FLOAT_OR_INT_PARAM( val0, 0 );
		GM_CHECK_FLOAT_OR_INT_PARAM( val1, 1 );
		a_thread->PushVec3( v3(val0, val1, 0.0f) );
	}

	else if ( numParams == 3 )
	{
		GM_CHECK_FLOAT_OR_INT_PARAM( val0, 0 );
		GM_CHECK_FLOAT_OR_INT_PARAM( val1, 1 );
		GM_CHECK_FLOAT_OR_INT_PARAM( val2, 2 );
		a_thread->PushVec3( v3(val0, val1, val2) );			
	}

	else return GM_EXCEPTION;

	return GM_OK;
}

//
// Libs and bindings
//


static gmFunctionEntry s_mathLib[] = 
{ 
  /*gm
    \lib math
  */
  /*gm
    \function abs
    \brief abs will return the absolute value of the passed int \ float
    \param int\float
    \return int\float abs(param)
  */
  {"abs", gmfAbs},
  /*gm
    \function sqrt
    \brief sqrt will return the square root of the passed int \ float
    \param int\float
    \return int\float sqrt(param)
  */
  {"sqrt", gmfSqrt},
  /*gm
    \function power
    \brief take the power of a A to B
    \param int\float A
    \param int\float B
    \return float A to the power of B
  */
  {"pow", gmfPower},
  /*gm
    \function floor
    \brief floor
    \param float A
    \return float floor(A)
  */
  {"floor", gmfFloor},
  /*gm
    \function ceil
    \brief ceil
    \param float A
    \return float ceil(A)
  */
  {"ceil", gmfCeil},
  /*gm
    \function round
    \brief round
    \param float A
    \return float round(A)
  */
  {"round", gmfRound},
  /*gm
    \function degtorad
    \brief degtorad will convert degrees to radians
    \param float\int deg
    \return float
  */
  {"degtorad", gmfDegToRad},
  /*gm
    \function radtodeg
    \brief radtodeg will convert radians to degrees
    \param float\int rad
    \return float
  */
  {"radtodeg", gmfRadToDeg},
  /*gm
    \function cos
    \brief cos will return the radian cosine
    \param float
    \return float
  */
  {"cos", gmfCos},
  /*gm
    \function sin
    \brief sin will return the radian sine
    \param float
    \return float
  */
  {"sin", gmfSin},
  /*gm
    \function tan
    \brief tan will return the radian tan (sin/cos)
    \param float
    \return float
  */
  {"tan", gmfTan},
  /*gm
    \function acos
    \brief acos will return the radian arc cosine
    \param float
    \return float
  */
  {"acos", gmfACos},
  /*gm
    \function asin
    \brief asin will return the radian arc sine
    \param float
    \return float
  */
  {"asin", gmfASin},
  /*gm
    \function atan
    \brief atan will return the radian arc tangent
    \param float
    \return float
  */
  {"atan", gmfATan},
  /*gm
    \function atan
    \brief atan will return the radian arc tangent of x / y
    \param float x
    \param float y
    \return float
  */
  {"atan2", gmfATan2},
  /*gm
    \function log
    \brief log will return the natural logarithm of 1 parameter, or (base, value) the logarithm to base
    \param float natural \ base
    \param float value (optional)
    \return float
  */
  {"log", gmfLog},
  /*gm
    \function min
    \brief min will return the min of the 2 passed values
    \param float\int A
    \param float\int B
    \return float \ int min(A, B)
  */
  {"min", gmfMin},
  /*gm
    \function max
    \brief max will return the max of the 2 passed values
    \param float\int A
    \param float\int B
    \return float \ int max(A, B)
  */
  {"max", gmfMax},
  /*gm
  \function fmod
  \brief fmod(x,y)
  \param float\int x
  \param float\int y
  \return float fmod of x/y
  */
  {"fmod", gmfFmod},
  /*gm
  \function lerp
  \brief lerp will return the lerped value. lerp(x0, x1, alpha)
  \param float\int X0
  \param float\int X1
  \param float	   ALPHA
  \return float\int value lerped from X0->X1
  */
  {"lerp", gmfLerp},
  /*gm
    \function clamp
    \brief clamp will return the clamed value. clamp(min, val, max)
	\param float\int VALUE
    \param float\int MIN
    \param float\int MAX
    \return float\int value clamped to min, max
  */
   {"clamp", gmfClamp},
  /*gm
  \function cubicPulse
  \brief cubicPulse  return cubic pulse [0.0, 1.0] cubicPulse(center, width, x)
  \param float\int center
  \param float\int width
  \param float\int x
  \return float cubic pulse
  */
 {"cubicPulse", gmfCubicPulse},
  /*gm
    \function randint
    \brief randint will return a random int from lower inclusive to upper.
    \param int lower inclusive
    \param int upper
    \return int 
  */
  {"randint", gmfRandInt},
  /*gm
    \function randfloat
    \brief randfloat will return a random float from lower inclusive to upper.
    \param float lower inclusive
    \param float upper
    \return float 
  */
  {"randfloat", gmfRandFloat},
  /*gm
    \function randseed
    \brief randseed will seed the random number generator
    \param int seed
  */
  {"randseed", gmfRandSeed},
  /*gm
    \function select
    \brief selects between two variables
    \param int/bool condition
	\param var
	\param var
  */
  {"select", gmfSelect},

  // funk! 
  {"wrap", gmfWrap},
  {"step", gmfStep},
  {"smoothstep", gmfSmoothStep},
  {"dot", gmfDot},
  {"distance", gmfDistance},
  {"length", gmfLength},
  {"lengthSqr", gmfLengthSqr},
  {"normalize", gmfNormalize},
  {"saturate", gmfSaturate},
  {"cross", gmfCross},
  {"hermite", gmfHermite},
  {"unitCircle", gmfUnitCircle},
  {"gaussian", gmfGaussian},
  {"sign", gmfSign },  
  {"HSVtoRGB", gmfHSVtoRGB }, 
  {"RGBtoHSV", gmfRGBtoHSV }, 

  {"v2", gmfCreateVec2},
  {"v3", gmfCreateVec3},
};


static gmFunctionEntry s_intLib[] = 
{ 
  {"String", gmfToString},
  {"Float", gmfToFloat},
  {"Int", gmfToInt},
};



static gmFunctionEntry s_floatLib[] = 
{ 
  {"String", gmfToString},
  {"Float", gmfToFloat},
  {"Int", gmfToInt},
};


void gmBindMathLib(gmMachine * a_machine)
{
  a_machine->RegisterLibrary(s_mathLib, sizeof(s_mathLib) / sizeof(s_mathLib[0]));
  a_machine->RegisterTypeLibrary(GM_INT, s_intLib, sizeof(s_intLib) / sizeof(s_intLib[0]));
  a_machine->RegisterTypeLibrary(GM_FLOAT, s_floatLib, sizeof(s_floatLib) / sizeof(s_floatLib[0]));
}