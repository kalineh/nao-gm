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
    , _resolution(AL::kQQVGA)
    , _colorspace(AL::kRGBColorSpace)
{
}

void GMVideoDisplay::SetActive(bool active)
{
    if (active && !_active)
    {
        Subscribe(_resolution, _colorspace);
        _active = active;
        return;
    }

    if (!active && _active)
    {
        _proxy.unsubscribe(_subscriber_id);        
        _active = active;
        _subscriber_id = _name;
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

    if (strcmp(resolution, "QQVGA") == 0) which = AL::kQQVGA;
    else if (strcmp(resolution, "QVGA") == 0) which = AL::kQVGA;
    else if (strcmp(resolution, "VGA") == 0) which = AL::kVGA;
    else if (strcmp(resolution, "4VGA") == 0) which = AL::k4VGA;
    else
    {
        CHECK(false);
        return;
    }

    if (which == _resolution)
        return;

    switch (which)
    {
    case AL::kQQVGA: _texture = new Texture(SizeQQVGA.x, SizeQQVGA.y); break;
    case AL::kQVGA: _texture = new Texture(SizeQVGA.x, SizeQVGA.y); break;
    case AL::kVGA: _texture = new Texture(SizeVGA.x, SizeVGA.y); break;
    case AL::k4VGA: _texture = new Texture(Size4VGA.x, Size4VGA.y); break;
    }

    _resolution = which;

    if (_active)
        Subscribe(_resolution, _colorspace);
}

void GMVideoDisplay::SetColorspace(const char* colorspace)
{
    int which = -1;

    // native - use if possible
    if (strcmp(colorspace, "YUV422") == 0) which = AL::kYUV422ColorSpace;
    else if (strcmp(colorspace, "Yuv") == 0) which = AL::kYuvColorSpace;
    else if (strcmp(colorspace, "yUv") == 0) which = AL::kyUvColorSpace;
    else if (strcmp(colorspace, "yuV") == 0) which = AL::kyuVColorSpace;
    else if (strcmp(colorspace, "Rgb") == 0) which = AL::kRgbColorSpace;
    else if (strcmp(colorspace, "rGb") == 0) which = AL::krGbColorSpace;
    else if (strcmp(colorspace, "rgB") == 0) which = AL::krgBColorSpace;
    else if (strcmp(colorspace, "Hsy") == 0) which = AL::kHsyColorSpace;
    else if (strcmp(colorspace, "hSy") == 0) which = AL::khSyColorSpace;
    else if (strcmp(colorspace, "hsY") == 0) which = AL::khsYColorSpace;
    else if (strcmp(colorspace, "YUV") == 0) which = AL::kYUVColorSpace;
    else if (strcmp(colorspace, "RGB") == 0) which = AL::kRGBColorSpace;
    else if (strcmp(colorspace, "HSY") == 0) which = AL::kHSYColorSpace;
    else if (strcmp(colorspace, "BGR") == 0) which = AL::kBGRColorSpace;
    else
    {
        CHECK(false);
        return;
    }

    if (which == _colorspace)
        return;

    _colorspace = which;

    if (_active)
        Subscribe(_resolution, _colorspace);
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

void GMVideoDisplay::Subscribe(int resolution, int colorspace)
{
    try
    {
        _proxy.unsubscribe(_subscriber_id);
    }
    catch (const AL::ALError&)
    {
        // ignore, just attempting to avoid hanging subscriptions
    }

    CHECK(resolution >= 0);
    CHECK(colorspace >= 0);

    _subscriber_id = _proxy.subscribe(_name, resolution, colorspace, VideoFPS);

    _proxy.setParam(AL::kCameraVFlipID, 0);

    switch (resolution)
    {
    case AL::kQQVGA: _texture = new Texture(SizeQQVGA.x, SizeQQVGA.y); break;
    case AL::kQVGA: _texture = new Texture(SizeQVGA.x, SizeQVGA.y); break;
    case AL::kVGA: _texture = new Texture(SizeVGA.x, SizeVGA.y); break;
    case AL::k4VGA: _texture = new Texture(Size4VGA.x, Size4VGA.y); break;
    }
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
    
    //if (layers != 3) { return; }
    //if (colorspace != AL::kRGBColorSpace) { return; }

    //CHECK(layers == 3);
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
