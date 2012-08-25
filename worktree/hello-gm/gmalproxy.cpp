//
// gmalproxy.cpp
//

#include "gmalproxy.h"

using namespace funk;

GMALProxy::GMALProxy(const char* type, const char* ip, int port)
    : _proxy(std::string(type), std::string(ip), port)
    , _current_call(-1)
{
}

gmVariable GMALProxy::CallReturnVariable(const char* function, gmVariable arg)
{
    const std::string function_string = std::string(function);

    AL::ALValue result = AL::ALValue();

    switch (arg.m_type)
    {
    case GM_INT: result = _proxy.call<AL::ALValue>(function_string, arg.GetInt()); break;
    case GM_FLOAT: result = _proxy.call<AL::ALValue>(function_string, arg.GetFloat()); break;
    case GM_VEC2: result = _proxy.call<AL::ALValue>(function_string, arg.GetVec2().x, arg.GetVec2().y); break;
    case GM_STRING: result = _proxy.call<AL::ALValue>(function_string, std::string(arg.GetCStringSafe())); break;
    default: result = _proxy.call<AL::ALValue>(function_string); break;
    }

    gmVariable result_variable;
    result_variable.Nullify();

    switch (result.getType())
    {
    case AL::ALValue::TypeBool: result_variable = gmVariable(int(result.operator bool &() ? 1 : 0)); break;
    case AL::ALValue::TypeInt: result_variable = gmVariable(int(result.operator int &())); break;
    case AL::ALValue::TypeFloat: result_variable = gmVariable(float(result.operator float &())); break;
    case AL::ALValue::TypeString: result_variable = gmVariable(VirtualMachine::Get()->GetVM().AllocStringObject(result.toString().c_str())); break;
    default:
        break;
    }

    return result_variable;
}

float GMALProxy::CallReturnFloat(const char* function, gmVariable arg)
{
    const std::string function_string = std::string(function);

    switch (arg.m_type)
    {
    case GM_INT: return _proxy.call<float>(function_string, arg.GetInt());
    case GM_FLOAT: return _proxy.call<float>(function_string, arg.GetFloat());
    case GM_VEC2: return _proxy.call<float>(function_string, arg.GetVec2().x, arg.GetVec2().y);
    case GM_STRING: return _proxy.call<float>(function_string, std::string(arg.GetCStringSafe()));
    default: return _proxy.call<float>(function_string);
    }

    return 0.0f;
}

void GMALProxy::CallVoid(const char* function, gmVariable arg)
{
    const std::string function_string = std::string(function);

    switch (arg.m_type)
    {
    case GM_INT: _proxy.callVoid(function_string, arg.GetInt()); break;
    case GM_FLOAT: _proxy.callVoid(function_string, arg.GetFloat()); break;
    case GM_VEC2: _proxy.callVoid(function_string, arg.GetVec2().x, arg.GetVec2().y); break;
    case GM_STRING: _proxy.callVoid(function_string, std::string(arg.GetCStringSafe())); break;
    default: _proxy.callVoid(function_string); break;
    }
}

void GMALProxy::PostCall(const char* function, gmVariable arg)
{
    const std::string function_string = std::string(function);

    int id = -1;

    switch (arg.m_type)
    {
    case GM_INT: id = _proxy.pCall(function_string, arg.GetInt()); break;
    case GM_FLOAT: id = _proxy.pCall(function_string, arg.GetFloat()); break;
    case GM_VEC2: id = _proxy.pCall(function_string, arg.GetVec2().x, arg.GetVec2().y); break;
    case GM_STRING: id = _proxy.pCall(function_string, std::string(arg.GetCStringSafe())); break;
    default: id = _proxy.pCall(function_string); break;
    }

    _current_call = id;
}

bool GMALProxy::IsRunning()
{
    return _proxy.isRunning(_current_call);
}

GM_BIND_DECL(GMALProxy);

GM_REG_NAMESPACE(GMALProxy)
{
	GM_MEMFUNC_DECL(CreateGMALProxy)
	{
		GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_STRING_PARAM(type, 0);
        GM_CHECK_STRING_PARAM(ip, 1);
        GM_CHECK_INT_PARAM(port, 2);
		GM_AL_EXCEPTION_WRAPPER(GM_PUSH_USER_HANDLED( GMALProxy, new GMALProxy(type, ip, port) ));
		return GM_OK;
	}

    GM_MEMFUNC_DECL(CallReturnVariable)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_STRING_PARAM(function, 0);
        //GM_CHECK_STRING_PARAM(str, 1);

		GM_GET_THIS_PTR(GMALProxy, self);
		GM_AL_EXCEPTION_WRAPPER(a_thread->Push(self->CallReturnVariable(function, a_thread->Param(1))));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CallReturnFloat)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_STRING_PARAM(function, 0);
        //GM_CHECK_STRING_PARAM(str, 1);

		GM_GET_THIS_PTR(GMALProxy, self);
		GM_AL_EXCEPTION_WRAPPER(a_thread->PushFloat(self->CallReturnFloat(function, a_thread->Param(1))));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CallVoid)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_STRING_PARAM(function, 0);
        //GM_CHECK_STRING_PARAM(str, 1);

		GM_GET_THIS_PTR(GMALProxy, self);
		GM_AL_EXCEPTION_WRAPPER(self->CallVoid(function, a_thread->Param(1)));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(PostCall)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_STRING_PARAM(function, 0);
        //GM_CHECK_STRING_PARAM(str, 1);

		GM_GET_THIS_PTR(GMALProxy, self);
		GM_AL_EXCEPTION_WRAPPER(self->PostCall(function, a_thread->Param(1)));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(IsRunning)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMALProxy, self);
		GM_AL_EXCEPTION_WRAPPER(a_thread->PushInt(self->IsRunning() ? 1 : 0));
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMALProxy)
GM_REG_MEMFUNC( GMALProxy, CallReturnVariable )
GM_REG_MEMFUNC( GMALProxy, CallReturnFloat )
GM_REG_MEMFUNC( GMALProxy, CallVoid )
GM_REG_MEMFUNC( GMALProxy, PostCall )
GM_REG_MEMFUNC( GMALProxy, IsRunning )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMALProxy);
