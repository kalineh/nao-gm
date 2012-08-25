//
// filters.h
//

#include "main.h"

using namespace funk;

class Filters
{
public:
    static void SobelARGBNaive(StrongHandle<Texture> out, StrongHandle<Texture> in, int threshold);
    static void SobelARGB(StrongHandle<Texture> out, StrongHandle<Texture> in, int threshold);
    static void BilateralARGBNaive(StrongHandle<Texture> out, StrongHandle<Texture> in, float spatial_sigma, float edge_sigma);
    static void BilateralARGB(StrongHandle<Texture> out, StrongHandle<Texture> in, float spatial_sigma, float edge_sigma);
    static void BoxBlurARGB(StrongHandle<Texture> out, StrongHandle<Texture> in);
    static void GaussianBlurARGB(StrongHandle<Texture> out, StrongHandle<Texture> in, float sigma);

    static void HoughTransformARGB(StrongHandle<Texture> out, StrongHandle<Texture> in, int theta_steps, int rho_bins, int rho_threshold);
    static void HoughLinesARGB(StrongHandle<Texture> out, StrongHandle<Texture> in, float peak_threshold);
    static void HoughLineSegmentsARGB(StrongHandle<Texture> out, StrongHandle<Texture> in, float peak_threshold);
};

void RegisterGmFiltersLib(gmMachine* a_vm);

class GMVideoDisplay
    : public HandledObj<GMVideoDisplay>
{
public:
	GM_BIND_TYPEID(GMVideoDisplay);

    GMVideoDisplay(const char* name, const char* ip, int port);

    void SetActive(bool active);
    void SetCamera(int which);

    // resolution – Resolution requested. { 0 = kQQVGA, 1 = kQVGA, 2 = kVGA, 3 = k4VGA }
    // colorSpace – Colorspace requested. { 0 = kYuv, 9 = kYUV422, 10 = kYUV, 11 = kRGB, 12 = kHSY, 13 = kBGR }

    void SetResolution(const char* resolution);
    void SetColorspace(const char* colorspace);

    StrongHandle<Texture> GetTexture();

    void Update();

private:
    void Subscribe(int resolution, int colorspace);

    void GetRemoteImage();

    bool _active;
    AL::ALVideoDeviceProxy _proxy;
    std::string _name;
    std::string _subscriber_id;
    int _resolution;
    int _colorspace;
    StrongHandle<Texture> _texture;
};

GM_BIND_DECL(GMVideoDisplay);

class GMSonar
    : public HandledObj<GMSonar>
{
public:
	GM_BIND_TYPEID(GMSonar);

    GMSonar(const char* name, const char* ip, int port);

    void SetActive(bool active);

    void Update();

    float GetValueByIndex(int index) const;

private:
    void Subscribe();

    void UpdateMemoryValues();

    bool _active;
    AL::ALSonarProxy _proxy;
    AL::ALMemoryProxy _memory_proxy;
    std::string _subscriber_id;
    std::vector<std::string> _output_names;
    AL::ALValue _output_names_value;
};

GM_BIND_DECL(GMSonar);
