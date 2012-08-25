//
// opencv_tests.cpp
//

#include "opencv_tests.h"

/*
using namespace funk;

GM_REG_NAMESPACE(GMSonar)
{
	GM_MEMFUNC_DECL(CreateGMSonar)
	{
		GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_STRING_PARAM(name, 0);
        GM_CHECK_STRING_PARAM(ip, 1);
        GM_CHECK_INT_PARAM(port, 2);
		GM_AL_EXCEPTION_WRAPPER(GM_PUSH_USER_HANDLED( GMSonar, new GMSonar(name, ip, port) ));
		return GM_OK;
	}

    GM_MEMFUNC_DECL(SetActive)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(active, 0);

		GM_GET_THIS_PTR(GMSonar, self);
		GM_AL_EXCEPTION_WRAPPER(self->SetActive(active != 0));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(GetValue)
    {
        GM_CHECK_NUM_PARAMS(1);
		GM_GET_THIS_PTR(GMSonar, self);
        GM_CHECK_INT_PARAM(index, 0);
        GM_AL_EXCEPTION_WRAPPER(a_thread->PushFloat(self->GetValueByIndex(index)));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(Update)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMSonar, self);
        GM_AL_EXCEPTION_WRAPPER(self->Update());
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMSonar)
GM_REG_MEMFUNC( GMSonar, SetActive )
GM_REG_MEMFUNC( GMSonar, GetValue )
GM_REG_MEMFUNC( GMSonar, Update )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMSonar);
*/
