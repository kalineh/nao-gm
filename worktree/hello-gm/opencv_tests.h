//
// opencv_tests.h
//

#include "main.h"

#include <opencv/cv.h>

using namespace funk;

class GMOpenCVMat
    : public HandledObj<GMOpenCVMat>
{
public:
    GM_BIND_TYPEID(GMOpenCV);

    explicit GMOpenCVMat(v2 dimen);
    
    void ReadFromTexture(StrongHandle<Texture> src);
    void WriteToTexture(StrongHandle<Texture> dst);

    void GaussianBlur(int kernel_size, float sigma1, float sigma2);
    void BilateralFilter(int iterations, int diameter, float sigma_color, float sigma_space);
    void SobelFilter(int kernel_size, float scale, float delta);
    void CannyThreshold(int kernel_size, float threshold_low, float threshold_high);

private:
    void ShowFlipped(const char* title, cv::Mat* mat);

    cv::Mat _data;
};

GM_BIND_DECL(GMOpenCVMat);
