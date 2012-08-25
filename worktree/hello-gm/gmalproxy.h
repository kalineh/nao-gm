//
// gmalproxy.h
//

#pragma once

#include "main.h"

using namespace funk;

class GMALProxy
    : public HandledObj<GMALProxy>
{
public:
	GM_BIND_TYPEID(GMALProxy);

    GMALProxy(const char* type, const char* ip, int port);

    gmVariable CallReturnVariable(const char* function, gmVariable arg);
    float CallReturnFloat(const char* function, gmVariable arg);
    void CallVoid(const char* function, gmVariable arg);
    void PostCall(const char* function, gmVariable arg);

    bool IsRunning();

private:
    AL::ALProxy _proxy;
    int _current_call;
};

GM_BIND_DECL(GMALProxy);
