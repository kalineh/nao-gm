#ifndef _INCLUDE_GM_BIND_FUNC_GEN_H_
#define _INCLUDE_GM_BIND_FUNC_GEN_H_

#include "gmThread.h"

///////////////////////////////////////////////////////////////////////////////
//			LIBRARY FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
#define GM_LIBFUNC_ENTRY(FUNC,CLASS) {#FUNC, gmf##CLASS##Lib::gmf##FUNC},

///////////////////////////////////////////////////////////////////////////////
//			GENERATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
#define _GM_GEN_MEMFUNC_BEGIN( FUNC_NAME ) int GM_CDECL gmf##FUNC_NAME( gmThread * a_thread ) {
#define _GM_GEN_MEMFUNC_END() }

#define GM_GEN_MEMFUNC_VOID_VOID( TYPE, MEMFUNC ) \
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(0);								\
	GM_GET_THIS_PTR(TYPE, ptr);							\
	ptr->MEMFUNC();										\
	return GM_OK;										\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_VOID_FLOAT( TYPE, MEMFUNC )		\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC)						\
	GM_CHECK_NUM_PARAMS(1);								\
	GM_CHECK_FLOAT_PARAM( val,	0 );					\
	GM_GET_THIS_PTR(TYPE, ptr);							\
	ptr->MEMFUNC(val);									\
	return GM_OK;										\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_VOID_INT( TYPE, MEMFUNC )		\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC)						\
	GM_CHECK_NUM_PARAMS(1);								\
	GM_CHECK_INT_PARAM( val,	0 );					\
	GM_GET_THIS_PTR(TYPE, ptr);							\
	ptr->MEMFUNC(val);									\
	return GM_OK;										\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_VOID_INT_V2( TYPE, MEMFUNC )		\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC)						\
	GM_CHECK_NUM_PARAMS(2);								\
	GM_CHECK_INT_PARAM( val0,	0 );					\
	GM_CHECK_VEC2_PARAM( val1,	1 );					\
	GM_GET_THIS_PTR(TYPE, ptr);							\
	ptr->MEMFUNC(val0,val1);							\
	return GM_OK;										\
	_GM_GEN_MEMFUNC_END()


#define GM_GEN_MEMFUNC_VOID_STRING( TYPE, MEMFUNC )				\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC)								\
	GM_CHECK_NUM_PARAMS(1);										\
	GM_CHECK_STRING_PARAM( val,	0 );							\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	ptr->MEMFUNC(val);											\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_VOID_INT_INT( TYPE, MEMFUNC )			\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC)								\
	GM_CHECK_NUM_PARAMS(2);										\
	GM_CHECK_INT_PARAM( v0,	0 );								\
	GM_CHECK_INT_PARAM( v1,	1 );								\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	ptr->MEMFUNC(v0,v1);										\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_VOID_INT_FLOAT( TYPE, MEMFUNC )			\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC)								\
	GM_CHECK_NUM_PARAMS(2);										\
	GM_CHECK_INT_PARAM( v0,	0 );								\
	GM_CHECK_FLOAT_PARAM( v1,	1 );							\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	ptr->MEMFUNC(v0,v1);										\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_VOID_V2( TYPE, MEMFUNC )	\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(1);									\
	GM_CHECK_VEC2_PARAM( val,	0 );						\
	GM_GET_THIS_PTR(TYPE, ptr);								\
	ptr->MEMFUNC(val);										\
	return GM_OK;											\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_VOID_V2_V2( TYPE, MEMFUNC )\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(2);									\
	GM_CHECK_VEC2_PARAM( v0,	0 );						\
	GM_CHECK_VEC2_PARAM( v1,	1 );						\
	GM_GET_THIS_PTR(TYPE, ptr);								\
	ptr->MEMFUNC(v0,v1);									\
	return GM_OK;											\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_VOID_V3( TYPE, MEMFUNC )	\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(1);									\
	GM_CHECK_VEC3_PARAM( val,	0 );						\
	GM_GET_THIS_PTR(TYPE, ptr);								\
	ptr->MEMFUNC(val);										\
	return GM_OK;											\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_VOID_FLOAT_FLOAT( TYPE, MEMFUNC )	\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(2);											\
	GM_CHECK_FLOAT_PARAM( val0,	0 );								\
	GM_CHECK_FLOAT_PARAM( val1,	1 );								\
	GM_GET_THIS_PTR(TYPE, ptr);										\
	ptr->MEMFUNC(val0, val1);										\
	return GM_OK;													\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_INT_VOID( TYPE, MEMFUNC )		\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(0);										\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	a_thread->PushInt( (int)ptr->MEMFUNC());					\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_INT_INT( TYPE, MEMFUNC )	\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(1);										\
	GM_CHECK_INT_PARAM( v0,	0 );								\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	a_thread->PushInt( (int)ptr->MEMFUNC(v0));				    \
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_INT_INT_INT( TYPE, MEMFUNC )	\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(2);										\
	GM_CHECK_INT_PARAM( v0,	0 );								\
	GM_CHECK_INT_PARAM( v1,	1 );								\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	a_thread->PushInt( (int)ptr->MEMFUNC(v0,v1));				\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_FLOAT_INT_INT( TYPE, MEMFUNC )	\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC)						\
	GM_CHECK_NUM_PARAMS(2);								\
	GM_CHECK_INT_PARAM( val0,	0 );					\
	GM_CHECK_INT_PARAM( val1,	1 );					\
	GM_GET_THIS_PTR(TYPE, ptr);							\
	a_thread->PushFloat( (float)ptr->MEMFUNC(val0, val1) );\
	return GM_OK;										\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_V2_VOID( TYPE, MEMFUNC )		\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(0);										\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	a_thread->PushVec2( ptr->MEMFUNC());						\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_V2_INT( TYPE, MEMFUNC )		\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(1);										\
	GM_CHECK_INT_PARAM( v0,	0 );								\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	a_thread->PushVec2( ptr->MEMFUNC(v0));						\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_V2_FLOAT( TYPE, MEMFUNC )				\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC)								\
	GM_CHECK_NUM_PARAMS(1);										\
	GM_CHECK_FLOAT_OR_INT_PARAM( v0,	0 );					\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	a_thread->PushVec2( ptr->MEMFUNC(v0));						\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()


#define GM_GEN_MEMFUNC_FLOAT_VOID( TYPE, MEMFUNC )	\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(0);										\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	a_thread->PushFloat( ptr->MEMFUNC());						\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

#define GM_GEN_MEMFUNC_FLOAT_INT( TYPE, MEMFUNC )	\
	_GM_GEN_MEMFUNC_BEGIN(MEMFUNC) \
	GM_CHECK_NUM_PARAMS(1);										\
	GM_CHECK_INT_PARAM( v0,	0 );								\
	GM_GET_THIS_PTR(TYPE, ptr);									\
	a_thread->PushFloat( ptr->MEMFUNC(v0));						\
	return GM_OK;												\
	_GM_GEN_MEMFUNC_END()

///////////////////////////////////////////////////////////////////////////////
//			GET/SET DOT OPERATORS
///////////////////////////////////////////////////////////////////////////////

// SETDOT params
#define GM_SETDOT_PARAM_USER_PTR( NAME, PARAM ) else if(stricmp(memberStr,NAME)==0){ GM_OP_USER_PTR(result,1); ptr->PARAM=result;}
#define GM_SETDOT_PARAM_STR( NAME, PARAM ) else if(stricmp(memberStr,NAME)==0){ GM_OP_STR_PTR(result,1); ptr->PARAM=result;}
#define GM_SETDOT_PARAM_VEC2( NAME, PARAM ) else if(stricmp(memberStr,NAME)==0){ GM_OP_VEC2(result,1); ptr->PARAM=result;}
#define GM_SETDOT_PARAM_VEC3( NAME, PARAM ) else if(stricmp(memberStr,NAME)==0){ GM_OP_VEC3(result,1); ptr->PARAM=result;}
#define GM_SETDOT_PARAM_FLOAT( NAME, PARAM ) else if(stricmp(memberStr,NAME)==0){ GM_OP_FLOAT(result,1); ptr->PARAM=result;}
#define GM_SETDOT_PARAM_INT( NAME, PARAM ) else if(stricmp(memberStr,NAME)==0){ GM_OP_INT(result,1); ptr->PARAM=result;}
#define GM_SETDOT_PARAM_FLOAT_OR_INT( NAME, PARAM ) else if(stricmp(memberStr,NAME)==0){ GM_OP_FLOAT_OR_INT(result,1); ptr->PARAM=result;}

// GETDOT params
#define GM_GETDOT_PARAM_USER_PTR(NAME,PARAM) else if ( stricmp(memberStr,NAME)==0 ) { a_operands[0].SetUser( ptr->PARAM ); }
#define GM_GETDOT_PARAM_STR(NAME,PARAM) else if ( stricmp(memberStr,NAME)==0 ) { a_operands[0].SetString( ptr->PARAM ); }
#define GM_GETDOT_PARAM_VEC2(NAME,PARAM) else if ( stricmp(memberStr,NAME)==0 ) { a_operands[0].SetVec2( ptr->PARAM ); }
#define GM_GETDOT_PARAM_VEC3(NAME,PARAM) else if ( stricmp(memberStr,NAME)==0 ) { a_operands[0].SetVec3( ptr->PARAM ); }
#define GM_GETDOT_PARAM_FLOAT(NAME,PARAM) else if ( stricmp(memberStr,NAME)==0 ) { a_operands[0].SetFloat( ptr->PARAM ); }
#define GM_GETDOT_PARAM_INT(NAME,PARAM) else if ( stricmp(memberStr,NAME)==0 ) { a_operands[0].SetInt( ptr->PARAM ); }


// SETDOT function gen
#define GM_SETDOT_FUNC_BEGIN(TYPE) \
	void GM_CDECL gmfSetDot( gmThread * a_thread, gmVariable * a_operands ) {	\
	GM_OP_USER_PTR( TYPE, 0, ptr );											\
	GM_OP_STR_PTR(memberStr, 2);											\
	if(false){}
#define GM_SETDOT_FUNC_END() \
	else { a_thread->GetMachine()->GetLog().LogEntry( "Setting invalid property %s", memberStr ); } }

// GETDOT function gen
#define GM_GETDOT_FUNC_BEGIN(TYPE)												\
	void GM_CDECL gmfGetDot( gmThread * a_thread, gmVariable * a_operands )	{	\
	GM_OP_USER_PTR( TYPE, 0, ptr );											\
	GM_OP_STR_PTR(memberStr, 1);											\
	if (false){}
#define GM_GETDOT_FUNC_END() \
	else { a_operands[0].Nullify(); } }

// Register GET/SET dot functions
#define _GM_REG_MEMOP_HELPER( FUNC, OPERATOR ) a_machine->RegisterTypeOperator( gmTypeId, OPERATOR, NULL, FUNC  );
#define GM_REG_SETDOT_FUNC( CLASS ) _GM_REG_MEMOP_HELPER( gmf##CLASS::gmfSetDot, O_SETDOT )
#define GM_REG_GETDOT_FUNC( CLASS ) _GM_REG_MEMOP_HELPER( gmf##CLASS::gmfGetDot, O_GETDOT )
#define GM_REG_SETDOT_GETDOT_FUNC( CLASS ) GM_REG_SETDOT_FUNC(CLASS) GM_REG_GETDOT_FUNC(NAMESPACE)



#endif