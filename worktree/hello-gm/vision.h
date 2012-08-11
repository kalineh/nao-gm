/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <cstdlib>
#include <string>
#include <vector>

#include "gm/gmMachine.h"
#include "gm/gmBindHeader.h"

#include "Core.h"
#include <common/HandledObj.h>
#include <common/StrongHandle.h>

#include <alproxies/alvideodeviceproxy.h>
#include <alproxies/alsonarproxy.h>
#include <alproxies/almemoryproxy.h>

#define GLM_MESSAGES
#define GLM_FORCE_INLINE
#define GLM_FORCE_SSE2
#include <glm/glm.hpp>
#include <glm/gtx/simd_vec4.hpp>
#include <glm/gtc/swizzle.hpp>

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

    //static void HoughTransformARGB(std::vector<glm::vec2> pairs, StrongHandle<Texture> in);
    static void HoughTransformARGB(StrongHandle<Texture> out, StrongHandle<Texture> in);
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
