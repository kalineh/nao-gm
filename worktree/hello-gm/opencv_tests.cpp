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

void GMOpenCVMat::BilateralFilter(int iterations, int diameter, float sigma_color, float sigma_space)
{
    cv::Mat src = cv::Mat(_data.size(), CV_8UC3);
    cv::Mat dst = cv::Mat(_data.size(), CV_8UC3);

    for (int i = 0; i < iterations; ++i)
    {
        cv::cvtColor(_data, src, CV_RGBA2RGB);
        cv::bilateralFilter(src, dst, diameter, sigma_color, sigma_space, IPL_BORDER_REPLICATE);
        cv::cvtColor(dst, _data, CV_RGB2RGBA);
    }

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

    // gives fat lines, maybe is useful, but at low-res it blobs things together too much
    //cv::dilate(edges, edges, cv::Mat());

    cv::cvtColor(edges, _data, CV_GRAY2RGBA);
    cv::bitwise_or(_data, cv::Scalar(cv::Vec4b(0, 0, 0, 255)), _data);
}

void testPoly()
{
    IplImage* src = cvLoadImage("../common/img/videoleft.png", 1);
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contour = 0;

    CvMat* gray = cvCreateMat(src->height, src->width, CV_8U);
    cvCvtColor(src, gray, CV_BGR2GRAY);
    cvCanny(gray, gray, 30.5f, 50.5f);
    cvDilate(gray, gray, 0, 1);

    CvSeq* poly = NULL;

    //find the contours (edges) of the silhouette, in terms of pixels.
    cvFindContours( gray,
                    storage,
                    &contour,
                    sizeof(CvContour),
                    CV_RETR_LIST );

    //convert the pixel contours to line segments in a polygon.

    IplImage* dst = cvCreateImage(cvGetSize(src), 8, 3);
    cvZero(dst);

    while (contour != nullptr)
    {
        // while it gives us some circle, it's not really useful
        //CvPoint2D32f center = { 0 };
        //float radius = 0.0f;
        //const int result = cvMinEnclosingCircle(contour, &center, &radius);
        //cvDrawCircle(dst, cvPoint(center.x, center.y), radius, CV_RGB(255, 1, 0));

        poly = cvApproxPoly(contour, 
                                 sizeof(CvContour), 
                                 storage,
                                 CV_POLY_APPROX_DP,
                                 2,
                                 1);

        const CvScalar color = cvScalar(
            (rand() % 255),
            (rand() % 255),
            (rand() % 255)
        );

        for (int p = 0; p < poly->total; ++p)
        {
            CvPoint* a = (CvPoint*)cvGetSeqElem(poly, (p + 0) % poly->total);
            CvPoint* b = (CvPoint*)cvGetSeqElem(poly, (p + 1) % poly->total);

            cvDrawLine(dst, *a, *b, color);
        }

        contour = contour->h_next;
    }

    CvMat* resized = cvCreateMat(dst->height * 4, dst->width * 4, CV_8UC3);
    //cvZero(resized);
    cvResize(dst, resized);

    cvNamedWindow("cont", 1);
    cvShowImage("cont", resized);
    cvWaitKey(0);
}

void testContours()
{
    IplImage* src = cvLoadImage("../common/img/videoleft.png", 1);
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contour = 0;

    CvMat* gray = cvCreateMat(src->height, src->width, CV_8U);
    cvCvtColor(src, gray, CV_BGR2GRAY);
    cvCanny(gray, gray, 30.5f, 50.5f);
    //cvThreshold(gray, gray, 1.0f, 255.0f, CV_THRESH_BINARY);

    cvNamedWindow("source", 1);
    cvShowImage("source", gray);

    cvFindContours(gray, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    IplImage* dst = cvCreateImage(cvGetSize(src), 8, 3);
    cvZero(dst);

    for (; contour != 0; contour = contour->h_next)
    {
        CvScalar color = CV_RGB(rand(), 0, 0);
        cvDrawContours(dst, contour, color, color, -1, CV_FILLED, 8);
    }

    cvNamedWindow("cont", 1);
    cvShowImage("cont", dst);
    cvWaitKey(0);
}

void GMOpenCVMat::FindContours(int mode, int method)
{
    //ASSERT(_data.type() == CV_32F || _data.type() == CV_8U);

    std::vector<std::vector<cv::Point> > contours;

    // The contour retrieval mode
    // CV_RETR_EXTERNAL retrieves only the extreme outer contours; It will set hierarchy[i][2]=hierarchy[i][3]=-1 for all the contours
    // CV_RETR_LIST retrieves all of the contours without establishing any hierarchical relationships
    // CV_RETR_CCOMP retrieves all of the contours and organizes them into a two-level hierarchy: on the top level are the external boundaries of the components, on the second level are the boundaries of the holes. If inside a hole of a connected component there is another contour, it will still be put on the top level
    // CV_RETR_TREE retrieves all of the contours and reconstructs the full hierarchy of nested contours. 

    // The contour approximation method.
    // CV_CHAIN_APPROX_NONE stores absolutely all the contour points. That is, every 2 points of a contour stored with this method are 8-connected neighbors of each other
    // CV_CHAIN_APPROX_SIMPLE compresses horizontal, vertical, and diagonal segments and leaves only their end points. E.g. an up-right rectangular contour will be encoded with 4 points
    // CV_CHAIN_APPROX_TC89_L1,CV_CHAIN_APPROX_TC89_KCOS applies one of the flavors of the Teh-Chin chain approximation algorithm; see TehChin89

    ASSERT(mode >= CV_RETR_EXTERNAL);
    ASSERT(mode <= CV_RETR_TREE);
    ASSERT(method >= CV_CHAIN_CODE);
    ASSERT(method <= CV_LINK_RUNS);

    cv::Mat gray = cv::Mat(_data.size(), CV_8U);
    cv::cvtColor(_data, gray, CV_RGBA2GRAY);

    cv::findContours(gray, contours, mode, method);

    cv::Scalar color = cv::Scalar(255, 255, 255, 255);
    cv::Mat result = cv::Mat::zeros(_data.size(), _data.type());

    for (int i = 0; i < int(contours.size()); ++i)
    {
        // seems to be a bug
        //cv::drawContours(result, contours, i, cv::Scalar(1.0f));

        const std::vector<cv::Point>& vertices = contours[i];
        const int count = vertices.size();
        for (int j = 0; j < count; ++j)
        {
            cv::line(result, vertices[j], vertices[(j + 1) % count], color);
        }
    }

    _data = result;
}

void GMOpenCVMat::StereoMatch(StrongHandle<Texture> left, StrongHandle<Texture> right)
{
    return testPoly();
    return testContours();

    // TODO: convert to new cv:: api

    IplImage* srcLeft = cvLoadImage("../common/img/videoleft2.png", 1);
    IplImage* srcRight = cvLoadImage("../common/img/videoright2.png", 1);

    IplImage* leftImage = cvCreateImage(cvGetSize(srcLeft), IPL_DEPTH_8U, 1);
    IplImage* rightImage = cvCreateImage(cvGetSize(srcRight), IPL_DEPTH_8U, 1);

    cvCvtColor(srcLeft, leftImage, CV_BGR2GRAY);
    cvCvtColor(srcRight, rightImage, CV_BGR2GRAY);

    CvSize size = cvGetSize(srcLeft);

    CvMat* disparity_left = cvCreateMat( size.height, size.width, CV_16S );
    CvMat* disparity_right = cvCreateMat( size.height, size.width, CV_16S );

    CvStereoGCState* state = cvCreateStereoGCState( 8, 4 );

    cvFindStereoCorrespondenceGC( leftImage, rightImage, disparity_left, disparity_right, state, 0 );

    cvReleaseStereoGCState( &state );

    CvMat* disparity_left_visual = cvCreateMat( size.height, size.width, CV_8U );

    cvConvertScale( disparity_left, disparity_left_visual, -16 );

    cvNamedWindow("win1", 1);
    cvNamedWindow("win2", 1);
    cvNamedWindow("win3", 1);

    cvShowImage("win1", srcLeft);
    cvShowImage("win2", leftImage);
    cvShowImage("win3", disparity_left_visual);

/*
    CvStereoGCState* state = cvCreateStereoGCState(256, 25000);

    cv::Mat mat_left = cv::Mat(_data.size(), CV_8UC4);
    cv::Mat mat_right = cv::Mat(_data.size(), CV_8UC4);
    cv::Mat disparity = cv::Mat(_data.size(), CV_32F);

    ReadIntoMat(&mat_left, left);
    ReadIntoMat(&mat_right, right);

    cv::Mat mat_left_gray = cv::Mat(mat_left.size(), CV_8U);
    cv::Mat mat_right_gray = cv::Mat(mat_right.size(), CV_8U);

    cv::cvtColor(mat_left, mat_left_gray, CV_RGBA2GRAY);
    cv::cvtColor(mat_right, mat_right_gray, CV_RGBA2GRAY);

    cv::StereoBM stereo = cv::StereoBM(CV_STEREO_BM_BASIC);
    cv::Mat disp = cv::Mat(mat_left.size(), CV_32F);
    stereo(mat_left_gray, mat_right_gray, disp);

    cv::Mat dispn = cv::Mat(_data.size(), CV_32F);
    cv::normalize(disp, dispn, 0.0f, 1.0f, CV_L2);

    cv::imshow("disp", disp);
    cv::imshow("dispn", dispn);

    //ShowFlipped("disp", &disparity);
    */
}

void GMOpenCVMat::ShowFlipped(const char* title, cv::Mat* mat)
{
    cv::Mat flipped, resized;
    cv::flip(*mat, flipped, 0);
    cv::resize(flipped, resized, flipped.size() * 2);
    cv::imshow(title, resized);
}

void GMOpenCVMat::ReadIntoMat(cv::Mat* mat, StrongHandle<Texture> tex)
{
    ASSERT(mat->type() == CV_8UC4);

    uint8_t* p = mat->ptr();

    tex->Bind(0);
    tex->GetTexImage(p);
    tex->Unbind();

    glFinish();
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
        GM_CHECK_NUM_PARAMS(4);
        GM_CHECK_INT_PARAM(iterations, 0);
        GM_CHECK_INT_PARAM(diameter, 1);
        GM_CHECK_FLOAT_PARAM(sigma_color, 2);
        GM_CHECK_FLOAT_PARAM(sigma_space, 3);
		GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_OPENCV_EXCEPTION_WRAPPER(self->BilateralFilter(iterations, diameter, sigma_color, sigma_space));
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

    GM_MEMFUNC_DECL(StereoMatch)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_USER_PARAM_PTR(Texture, left, 0);
        GM_CHECK_USER_PARAM_PTR(Texture, right, 1);
		GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_OPENCV_EXCEPTION_WRAPPER(self->StereoMatch(left, right));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(FindContours)
    {
        GM_CHECK_NUM_PARAMS(2);
        GM_CHECK_INT_PARAM(mode, 0);
        GM_CHECK_INT_PARAM(method, 1);
		GM_GET_THIS_PTR(GMOpenCVMat, self);
        GM_OPENCV_EXCEPTION_WRAPPER(self->FindContours(mode, method));
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
GM_REG_MEMFUNC( GMOpenCVMat, StereoMatch )
GM_REG_MEMFUNC( GMOpenCVMat, FindContours )
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