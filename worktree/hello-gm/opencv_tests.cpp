//
// opencv_tests.cpp
//

#include "opencv_tests.h"

#include <opencv/highgui.h>

using namespace funk;

#define GM_OPENCV_EXCEPTION_WRAPPER(code) \
    try { code ; } catch (const cv::Exception& e) { GM_EXCEPTION_MSG(e.err.c_str()); return GM_OK; }

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


/* various test
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
*/

void GMOpenCVMat::GaussianBlur(int kernel_size, float sigma1, float sigma2)
{
    cv::GaussianBlur(_data, _data, cv::Size(3, 3), sigma1, sigma2, IPL_BORDER_REPLICATE);
}

void GMOpenCVMat::BilateralFilter(int diameter, float sigma_color, float sigma_space)
{
    cv::Mat src = cv::Mat(_data.size(), CV_8UC3);
    cv::Mat dst = cv::Mat(_data.size(), CV_8UC3);

    cv::cvtColor(_data, src, CV_RGBA2RGB);
    cv::bilateralFilter(src, dst, diameter, sigma_color, sigma_space, IPL_BORDER_REPLICATE);
    cv::cvtColor(dst, _data, CV_RGB2RGBA);

    cv::bitwise_or(_data, cv::Scalar(cv::Vec4b(0, 0, 0, 255)), _data);
}

void GMOpenCVMat::SobelFilter(int kernel_size, float scale, float delta)
{
    const int kernel_size_odd = kernel_size | 1;

    cv::Mat gray = cv::Mat(_data.size(), CV_32F);
    cv::Mat gradient = cv::Mat(_data.size(), CV_32F);
    cv::Mat gradient_x = cv::Mat(_data.size(), CV_32F);
    cv::Mat gradient_y = cv::Mat(_data.size(), CV_32F);
    cv::Mat abs_gradient_x = cv::Mat(_data.size(), CV_32F);
    cv::Mat abs_gradient_y = cv::Mat(_data.size(), CV_32F);

    cv::cvtColor(_data, gray, CV_RGBA2GRAY);

    cv::Sobel(gray, gradient_x, gray.depth(), 1, 0, kernel_size_odd, scale, delta, IPL_BORDER_REPLICATE);
    cv::convertScaleAbs(gradient_x, abs_gradient_x);

    cv::Sobel(gray, gradient_y, gray.depth(), 0, 1, kernel_size_odd, scale, delta, IPL_BORDER_REPLICATE);
    cv::convertScaleAbs(gradient_y, abs_gradient_y);

    cv::addWeighted(abs_gradient_x, 0.5f, abs_gradient_y, 0.5f, 0.0f, gradient);

    cv::cvtColor(gradient, _data, CV_GRAY2RGBA);
    cv::bitwise_or(_data, cv::Scalar(cv::Vec4b(0, 0, 0, 255)), _data);
}

void GMOpenCVMat::CannyThreshold(int kernel_size, float threshold_low, float threshold_high)
{
    const int kernel_size_odd = kernel_size | 1;

    cv::Mat gray = cv::Mat(_data.size(), CV_32F);
    cv::Mat edges = cv::Mat(_data.size(), CV_32F);

    cv::cvtColor(_data, gray, CV_RGBA2GRAY);

    cv::Canny(gray, edges, threshold_low, threshold_high);

    cv::cvtColor(edges, _data, CV_GRAY2RGBA);
    cv::bitwise_or(_data, cv::Scalar(cv::Vec4b(0, 0, 0, 255)), _data);
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
		GM_OPENCV_EXCEPTION_WRAPPER(GM_PUSH_USER_HANDLED( GMOpenCVMat, new GMOpenCVMat(dimen) ));
		return GM_OK;
	}

    GM_MEMFUNC_DECL(ReadFromTexture)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_USER_PARAM_PTR(Texture, src, 0);
        GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_OPENCV_EXCEPTION_WRAPPER(self->ReadFromTexture(src));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(WriteToTexture)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_USER_PARAM_PTR(Texture, dst, 0);
        GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_OPENCV_EXCEPTION_WRAPPER(self->WriteToTexture(dst));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(GaussianBlur)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(kernel_size, 0);
        GM_CHECK_FLOAT_PARAM(sigma1, 1);
        GM_CHECK_FLOAT_PARAM(sigma2, 2);
		GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_OPENCV_EXCEPTION_WRAPPER(self->GaussianBlur(kernel_size, sigma1, sigma2));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(BilateralFilter)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(diameter, 0);
        GM_CHECK_FLOAT_PARAM(sigma_color, 1);
        GM_CHECK_FLOAT_PARAM(sigma_space, 2);
		GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_OPENCV_EXCEPTION_WRAPPER(self->BilateralFilter(diameter, sigma_color, sigma_space));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(SobelFilter)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(kernel_size, 0);
        GM_CHECK_FLOAT_PARAM(scale, 1);
        GM_CHECK_FLOAT_PARAM(delta, 2);
		GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_OPENCV_EXCEPTION_WRAPPER(self->SobelFilter(kernel_size, scale, delta));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(CannyThreshold)
    {
        GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_INT_PARAM(kernel_size, 0);
        GM_CHECK_FLOAT_PARAM(threshold_low, 1);
        GM_CHECK_FLOAT_PARAM(threshold_high, 2);
		GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_OPENCV_EXCEPTION_WRAPPER(self->CannyThreshold(kernel_size, threshold_low, threshold_high));
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMOpenCVMat)
GM_REG_MEMFUNC( GMOpenCVMat, ReadFromTexture )
GM_REG_MEMFUNC( GMOpenCVMat, WriteToTexture )
GM_REG_MEMFUNC( GMOpenCVMat, GaussianBlur )
GM_REG_MEMFUNC( GMOpenCVMat, BilateralFilter )
GM_REG_MEMFUNC( GMOpenCVMat, SobelFilter )
GM_REG_MEMFUNC( GMOpenCVMat, CannyThreshold )
//GM_REG_MEMFUNC( GMOpenCVMat, Threshold )
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