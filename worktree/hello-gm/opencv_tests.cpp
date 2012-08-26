//
// opencv_tests.cpp
//

#include "opencv_tests.h"

#include <opencv/highgui.h>

using namespace funk;

GMOpenCVMat::GMOpenCVMat(v2 dimen)
    : _data(int(dimen.y), int(dimen.x), CV_8UC4)
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

    for (int i = 0; i < 160 * 120; i += 4)
    {
        //p[i+3] = 0xff;
    }

    //cv::Vec4b alpha(0, 0, 0, 255);
    //cv::Mat out = _data | alpha;
    //_data.mul

    dst->Bind();
    dst->SubData(p, dst->Sizei().x, dst->Sizei().y, 0, 0);
    dst->Unbind();

    glFinish();
}

void GMOpenCVMat::GaussianBlur()
{
    ASSERT(_data.ptr() != NULL);
    //cv::GaussianBlur(_data, _data, cv::Size(3, 3), 1.0f, 0.0f, IPL_BORDER_REPLICATE);
    //cv::boxFilter(_data, _data, _data.depth(), cv::Size(7, 1));

    //cv::imshow("before", _data);

    cv::Mat gray;
    cv::Mat out;
    cv::cvtColor(_data, gray, CV_RGBA2GRAY);
    cv::Sobel(gray, out, gray.depth(), 1, 0);
    cv::Sobel(gray, out, gray.depth(), 0, 1);
    cv::cvtColor(out, _data, CV_GRAY2RGBA);

    // we lose alpha, so mix it back in
    ASSERT(_data.isContinuous());
    cv::bitwise_or(_data, cv::Scalar(cv::Vec4b(0, 0, 0, 255)), _data);

    //ShowFlipped("gray", &gray);
    //ShowFlipped("out", &out);
    ShowFlipped("after", &_data);
}

void GMOpenCVMat::ShowFlipped(const char* title, cv::Mat* mat)
{
    cv::Mat flipped, resized;
    cv::flip(*mat, flipped, 0);
    cv::resize(flipped, resized, flipped.size() * 2);
    cv::imshow(title, resized);
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
        GM_AL_EXCEPTION_WRAPPER(self->WriteToTexture(dst));
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