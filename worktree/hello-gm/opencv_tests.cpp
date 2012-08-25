//
// opencv_tests.cpp
//

#include "opencv_tests.h"

using namespace funk;

GMOpenCVMat::GMOpenCVMat(v2 dimen)
    : _data(int(dimen.x), int(dimen.y), CV_32F)
{
}

void GMOpenCVMat::ReadFromTexture(StrongHandle<Texture> src)
{
    uint8_t* p = _data.ptr();

    src->Bind(0);
    src->GetTexImage(p);
    src->Unbind();

    glFinish();
}

void GMOpenCVMat::WriteToTexture(StrongHandle<Texture> dst)
{
    uint8_t* p = _data.ptr();

    dst->Bind();
    dst->SubData(p, dst->Sizei().x, dst->Sizei().y, 0, 0);
    dst->Unbind();
}

void GMOpenCVMat::GaussianBlur()
{
    ASSERT(_data.ptr() != NULL);
    //cv::GaussianBlur(_data, _data, cv::Size(3, 3), 1.0f, 0.0f, IPL_BORDER_REPLICATE);
    cv::Sobel(_data, _data, CV_32F, 1, 0);
}

GM_REG_NAMESPACE(GMOpenCVMat)
{
	GM_MEMFUNC_DECL(CreateGMOpenCVMat)
	{
		GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_VEC2_PARAM(dimen, 0);
		GM_AL_EXCEPTION_WRAPPER(GM_PUSH_USER_HANDLED( GMOpenCVMat, new GMOpenCVMat(dimen) ));
		return GM_OK;
	}

    GM_MEMFUNC_DECL(ReadFromTexture)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_USER_PARAM_PTR(Texture, src, 0);
        GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_AL_EXCEPTION_WRAPPER(self->ReadFromTexture(src));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(WriteToTexture)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_USER_PARAM_PTR(Texture, dst, 0);
        GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_AL_EXCEPTION_WRAPPER(self->ReadFromTexture(dst));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(GaussianBlur)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_AL_EXCEPTION_WRAPPER(self->GaussianBlur());
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMOpenCVMat)
GM_REG_MEMFUNC( GMOpenCVMat, ReadFromTexture )
GM_REG_MEMFUNC( GMOpenCVMat, WriteToTexture )
GM_REG_MEMFUNC( GMOpenCVMat, GaussianBlur )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMOpenCVMat);

//static gmFunctionEntry s_OpenCVLib[] = 
//{ 
	//{ "SobelARGB", gmfFilterSobelARGB },
//};

//void RegisterGmOpenCVLib(gmMachine* a_vm)
//{
	//a_vm->RegisterLibrary(s_OpenCVLib, sizeof(s_OpenCVLib) / sizeof(s_OpenCVLib[0]), "OpenCV");
//}