/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include "vision.h"

#include "gm/gmBind.h"

#include <common/Debug.h>
#include <common/StrongHandle.h>
#include <vm/VirtualMachine.h>
#include <gfx/Texture.h>

#include <alcommon/alproxy.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alvision/alvisiondefinitions.h>

using namespace funk;

const int VideoFPS = 1;

GMVideoDisplay::GMVideoDisplay(const char* name, const char* ip, int port)
    : _active(false)
    , _proxy(std::string(ip), port)
    , _name(name)
    , _subscriber_id(name)
{
}

void GMVideoDisplay::SetActive(bool active)
{
    if (active && !_active)
    {
        try
        {
            _proxy.unsubscribe(_name);
        }
        catch (const AL::ALError&)
        {
            // ignore, just attempting to avoid hanging subscriptions
        }

        _subscriber_id = _proxy.subscribe(_name, AL::kQQVGA, AL::kRGBColorSpace, VideoFPS);
        _active = active;

        _proxy.setParam(AL::kCameraVFlipID, 1);

        return;
    }

    if (!active && _active)
    {
        _proxy.unsubscribe(_subscriber_id);        
        _active = active;
    }
}

void GMVideoDisplay::SetCamera(int which)
{
    // does work when active?
    _proxy.setParam(AL::kCameraSelectID, which);
}

// resolution – Resolution requested. { 0 = kQQVGA, 1 = kQVGA, 2 = kVGA, 3 = k4VGA }
// colorSpace – Colorspace requested. { 0 = kYuv, 9 = kYUV422, 10 = kYUV, 11 = kRGB, 12 = kHSY, 13 = kBGR }

void GMVideoDisplay::SetResolution(const char* resolution)
{
    int which = -1;

    if (strcmp(resolution, "QQVGA")) which = AL::kQQVGA;
    if (strcmp(resolution, "QVGA")) which = AL::kQVGA;
    if (strcmp(resolution, "VGA")) which = AL::kVGA;
    if (strcmp(resolution, "4VGA")) which = AL::k4VGA;

    if (which >= 0)
    {
        _proxy.setParam(AL::kCameraColorSpaceID, which);
    }
}

void GMVideoDisplay::SetColorspace(const char* colorspace)
{
    int which = -1;

    // native - use if possible
    if (strcmp(colorspace, "YUV422")) which = AL::kYUV422ColorSpace;

    if (strcmp(colorspace, "Yuv")) which = AL::kYuvColorSpace;
    if (strcmp(colorspace, "yUv")) which = AL::kyUvColorSpace;
    if (strcmp(colorspace, "yuV")) which = AL::kyuVColorSpace;
    if (strcmp(colorspace, "Rgb")) which = AL::kRgbColorSpace;
    if (strcmp(colorspace, "rGb")) which = AL::krGbColorSpace;
    if (strcmp(colorspace, "rgB")) which = AL::krgBColorSpace;
    if (strcmp(colorspace, "Hsy")) which = AL::kHsyColorSpace;
    if (strcmp(colorspace, "hSy")) which = AL::khSyColorSpace;
    if (strcmp(colorspace, "hsY")) which = AL::khsYColorSpace;
    if (strcmp(colorspace, "YUV")) which = AL::kYUVColorSpace;
    if (strcmp(colorspace, "RGB")) which = AL::kRGBColorSpace;
    if (strcmp(colorspace, "HSY")) which = AL::kHSYColorSpace;
    if (strcmp(colorspace, "BGR")) which = AL::kBGRColorSpace;

    if (which >= 0)
    {
        _proxy.setParam(AL::kCameraColorSpaceID, which);
    }
}

StrongHandle<Texture> GMVideoDisplay::GetTexture()
{
    return _texture;
}

void GMVideoDisplay::Update()
{
    GetRemoteImage();
}

void GMVideoDisplay::GetRemoteImage()
{
    AL::ALValue results = _proxy.getImageRemote(_subscriber_id);

    uint8_t* data = (uint8_t*)(results[6].GetBinary());
    if (data == NULL)
    {
        CHECK(false);
        return;
    }

    // [0] : width;
    // [1] : height;
    // [2] : number of layers;
    // [3] : ColorSpace;
    // [4] : timestamp (seconds);
    // [5] : timestamp (micro-seconds);
    // [6] : array of size height * width * nblayers containing image data;
    // [7] : camera ID (kTop=0, kBottom=1);
    // [8] : left angle (radian);
    // [9] : topAngle (radian);
    // [10]: rightAngle (radian);
    // [11]: bottomAngle (radian);

    const int video_pixelsize = 3;
    const int video_w = results[0];
    const int video_h = results[1];
    const int layers = results[2];
    const int colorspace = results[3];

    const int layer_size = video_w * video_h * video_pixelsize;

    CHECK(layers == 3);
    CHECK(colorspace == AL::kRGBColorSpace);

    // QQVGA = 160x120
    // QVGA = 320x240
    // VGA = 640x480
    // 4VGA = 1280x960

    const int w = std::min<int>(_texture->Sizei().y, video_w);
    const int h = std::min<int>(_texture->Sizei().x, video_h);

    uint32_t* buffer = new uint32_t[w * h];
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const int src_index = (x + y * w) * video_pixelsize;
            const int dst_index = (x + y * w);

            const uint8_t r = data[src_index + 0];
            const uint8_t g = data[src_index + 1];
            const uint8_t b = data[src_index + 2];
            const uint8_t a = 255;

            buffer[dst_index] = 
                (a << 24) |
                (b << 16) |
                (g <<  8) |
                (r <<  0);
        }
    }

    // flip
    //for (int y = 0; y < h / 2; ++y)
    //{
        //uint32_t line[w];
        //memcpy(line, buffer + y * w, w * sizeof(uint32_t));
        //memcpy(buffer + y * w, buffer + (h - y - 1) * w, w * sizeof(uint32_t));
        //memcpy(buffer + (h - y - 1) * w, line, w * sizeof(uint32_t));
    //}

    _texture->Bind();
    _texture->SubData(buffer, w, h);
    _texture->Unbind();

    delete buffer;

    _proxy.releaseImage(_subscriber_id);
}

GM_REG_NAMESPACE(GMVideoDisplay)
{
	GM_MEMFUNC_DECL(CreateGMVideoDisplay)
	{
		GM_CHECK_NUM_PARAMS(3);
        GM_CHECK_STRING_PARAM(name, 0);
        GM_CHECK_STRING_PARAM(ip, 1);
        GM_CHECK_INT_PARAM(port, 2);
		GM_AL_EXCEPTION_WRAPPER(GM_PUSH_USER_HANDLED( GMVideoDisplay, new GMVideoDisplay(name, ip, port) ));
		return GM_OK;
	}

    GM_MEMFUNC_DECL(SetActive)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(active, 0);

		GM_GET_THIS_PTR(GMVideoDisplay, self);
		GM_AL_EXCEPTION_WRAPPER(self->SetActive(active != 0));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(SetCamera)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_INT_PARAM(camera, 0);

		GM_GET_THIS_PTR(GMVideoDisplay, self);
		GM_AL_EXCEPTION_WRAPPER(self->SetCamera(camera));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(SetResolution)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_STRING_PARAM(resolution, 0);

		GM_GET_THIS_PTR(GMVideoDisplay, self);
		GM_AL_EXCEPTION_WRAPPER(self->SetResolution(resolution));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(SetColorspace)
    {
        GM_CHECK_NUM_PARAMS(1);
        GM_CHECK_STRING_PARAM(colorspace, 0);

		GM_GET_THIS_PTR(GMVideoDisplay, self);
		GM_AL_EXCEPTION_WRAPPER(self->SetColorspace(colorspace));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(GetTexture)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMVideoDisplay, self);
        GM_AL_EXCEPTION_WRAPPER(GM_PUSH_USER_HANDLED(Texture, self->GetTexture()));
        return GM_OK;
    }

    GM_MEMFUNC_DECL(Update)
    {
        GM_CHECK_NUM_PARAMS(0);
		GM_GET_THIS_PTR(GMVideoDisplay, self);
        GM_AL_EXCEPTION_WRAPPER(self->Update());
        return GM_OK;
    }
}

GM_REG_MEM_BEGIN(GMVideoDisplay)
GM_REG_MEMFUNC( GMVideoDisplay, SetActive )
GM_REG_MEMFUNC( GMVideoDisplay, SetCamera )
GM_REG_MEMFUNC( GMVideoDisplay, SetResolution )
GM_REG_MEMFUNC( GMVideoDisplay, SetColorspace )
GM_REG_MEMFUNC( GMVideoDisplay, GetTexture )
GM_REG_MEMFUNC( GMVideoDisplay, Update )
GM_REG_MEM_END()

GM_BIND_DEFINE(GMVideoDisplay);
