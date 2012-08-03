/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <cstdlib>
#include <string>

#include "gm/gmMachine.h"
#include "gm/gmBindHeader.h"

#include "Core.h"
#include <common/HandledObj.h>

#include <alproxies/alvideodeviceproxy.h>

using namespace funk;

class Filters
{
public:
    static void SobelARGB(StrongHandle<Texture> in, StrongHandle<Texture> out, int threshold);
    static void BilateralARGB(StrongHandle<Texture> in, StrongHandle<Texture> out, float spatial_sigma, float edge_sigma);
    static void BoxBlurARGB(StrongHandle<Texture> in, StrongHandle<Texture> out);
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

    //void FilterSobel(...);
    //void FilterBlobs(...);

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

