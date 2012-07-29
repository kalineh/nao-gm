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

void Filters::Sobel(StrongHandle<Texture> in, StrongHandle<Texture> out)
{
    CHECK(in->Sizei() == out->Sizei());

    // ignore edges because simplicity

    int border = 1;

    const int w = in->Sizei().x;
    const int h = in->Sizei().y;
    const int pixelsize = 4;

    uint32_t* buffer = new uint32_t[w * h];

    in->Bind();
    in->ReadPixels(0, 0, w, h, buffer);
    in->Unbind();

    uint8_t* bytes = (uint8_t*)buffer;
    for (int y = border; y < h - border; ++y)
    {
        for (int x = border; x < w - border; ++x)
        {
            const int src_index = (x + y * w) * pixelsize;
            const int dst_index = (x + y * w);

            const uint8_t r = bytes[src_index + 0];
            const uint8_t g = bytes[src_index + 1];
            const uint8_t b = bytes[src_index + 2];
            const uint8_t a = bytes[src_index + 3];

            buffer[dst_index] = 
                (a << 24) |
                (b << 16) | 255 |
                (g <<  8) |
                (r <<  0);
        }
    }

    out->Bind();
    out->SubData(buffer, w, h, 0, 0);
    out->Unbind();
}


const int VideoFPS = 1;
const v2i SizeQQVGA = v2i(160, 120);
const v2i SizeQVGA = SizeQQVGA * 2;
const v2i SizeVGA = SizeQVGA * 2;
const v2i Size4VGA = SizeVGA * 2;

GMVideoDisplay::GMVideoDisplay(const char* name, const char* ip, int port)
    : _active(false)
    , _proxy(std::string(ip), port)
    , _name(name)
    , _subscriber_id(name)
	, _texture(new Texture(8, 8))
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

        _proxy.setParam(AL::kCameraVFlipID, 0);
		_texture = new Texture(SizeQQVGA.x, SizeQQVGA.y);

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

        switch (which)
        {
        case AL::kQQVGA: _texture = new Texture(SizeQQVGA.x, SizeQQVGA.y); break;
        case AL::kQVGA: _texture = new Texture(SizeQVGA.x, SizeQVGA.y); break;
        case AL::kVGA: _texture = new Texture(SizeVGA.x, SizeVGA.y); break;
        case AL::k4VGA: _texture = new Texture(Size4VGA.x, Size4VGA.y); break;
        }
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
    if (!_active)
        return;

    GetRemoteImage();
}

void GMVideoDisplay::GetRemoteImage()
{
    AL::ALValue results = _proxy.getImageRemote(_subscriber_id);

    uint8_t* bytes = (uint8_t*)(results[6].GetBinary());
    if (bytes == NULL)
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
    CHECK(_texture->Sizei().x == video_w);
    CHECK(_texture->Sizei().y == video_h);

    // QQVGA = 160x120
    // QVGA = 320x240
    // VGA = 640x480
    // 4VGA = 1280x960

    const int w = video_w;
    const int h = video_h;

    uint32_t* buffer = new uint32_t[w * h];
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const int src_index = (x + y * w) * video_pixelsize;
            const int dst_index = (x + y * w);

            const uint8_t r = bytes[src_index + 0];
            const uint8_t g = bytes[src_index + 1];
            const uint8_t b = bytes[src_index + 2];
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
