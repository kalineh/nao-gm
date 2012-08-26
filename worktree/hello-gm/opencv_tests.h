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

    void GaussianBlur();

private:
    void ShowFlipped(const char* title, cv::Mat* mat);

    cv::Mat _data;
};

GM_BIND_DECL(GMOpenCVMat);
