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

void Filters::SobelRGBA(StrongHandle<Texture> in, StrongHandle<Texture> out)
{
    CHECK(in->Sizei() == out->Sizei());

    // ignore edges because simplicity

    int border = 1;

    const int w = in->Sizei().x;
    const int h = in->Sizei().y;
    const int pixelsize = 4;

    uint32_t* buffer_in = new uint32_t[w * h];
    uint32_t* buffer_out = new uint32_t[w * h];

    in->Bind();
    in->ReadPixels(0, 0, w, h, buffer_in);
    in->Unbind();

    glFlush();
    glFinish();

    // test copy
    //memcpy(buffer_out, buffer_in, w * h * pixelsize);

    uint8_t* bytes_in = (uint8_t*)buffer_in;
    for (int y = border; y < h - border; ++y)
    {
        for (int x = border; x < w - border; ++x)
        {
            const int byte_index = (x + y * w) * pixelsize;
            const int pixel_index = (x + y * w);

            const uint32_t a = buffer_in[((x - 1) + (y - 1) * w)];
            const uint32_t b = buffer_in[((x + 0) + (y - 1) * w)];
            const uint32_t c = buffer_in[((x + 1) + (y - 1) * w)];
            const uint32_t g = buffer_in[((x - 1) + (y + 1) * w)];
            const uint32_t h = buffer_in[((x + 0) + (y + 1) * w)];
            const uint32_t i = buffer_in[((x + 1) + (y + 1) * w)];

            const uint8_t ar = (a & 0x00FF0000) >> 16;
            const uint8_t ag = (a & 0x0000FF00) >> 8;
            const uint8_t ab = (a & 0x000000FF) >> 0;

            const uint8_t br = (b & 0x00FF0000) >> 16;
            const uint8_t bg = (b & 0x0000FF00) >> 8;
            const uint8_t bb = (b & 0x000000FF) >> 0;

            const uint8_t cr = (c & 0x00FF0000) >> 16;
            const uint8_t cg = (c & 0x0000FF00) >> 8;
            const uint8_t cb = (c & 0x000000FF) >> 0;

            const uint8_t gr = (g & 0x00FF0000) >> 16;
            const uint8_t gg = (g & 0x0000FF00) >> 8;
            const uint8_t gb = (g & 0x000000FF) >> 0;

            const uint8_t hr = (h & 0x00FF0000) >> 16;
            const uint8_t hg = (h & 0x0000FF00) >> 8;
            const uint8_t hb = (h & 0x000000FF) >> 0;

            const uint8_t ir = (i & 0x00FF0000) >> 16;
            const uint8_t ig = (i & 0x0000FF00) >> 8;
            const uint8_t ib = (i & 0x000000FF) >> 0;

            // a, b, c
            // d, e, f
            // g, h, i

            // -1, -2, +1
            // -2,  0, +2 // not
            // -1, -2, +1

            int sum_r = ar * -1 + br * -2 + cr * +1 + gr * -1 + hr * -2 + ir * +1;
            int sum_g = ar * -1 + br * -2 + cr * +1 + gr * -1 + hr * -2 + ir * +1;
            int sum_b = ar * -1 + br * -2 + cr * +1 + gr * -1 + hr * -2 + ir * +1;

            if (sum_r < 0) sum_r = 0;
            if (sum_g < 0) sum_g = 0;
            if (sum_b < 0) sum_b = 0;

            if (sum_r > 255) sum_r = 255;
            if (sum_g > 255) sum_g = 255;
            if (sum_b > 255) sum_b = 255;

            const uint32_t pixel = (buffer_in[pixel_index] & 0xFF000000) | sum_r | sum_g | sum_b;

            buffer_out[pixel_index] = pixel;
        }
    }

    out->Bind();
    out->SubData(buffer_out, w, h, 0, 0);
    out->Unbind();
}

static int GM_CDECL gmfFilterSobelRGBA(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(2);

	GM_CHECK_USER_PARAM_PTR( Texture, in, 0 );
	GM_CHECK_USER_PARAM_PTR( Texture, out, 1 );

    Filters::SobelRGBA(in, out);

	return GM_OK;
}

static gmFunctionEntry s_FiltersLib[] = 
{ 
	{ "SobelRGBA", gmfFilterSobelRGBA },
};

void RegisterGmFiltersLib(gmMachine* a_vm)
{
	a_vm->RegisterLibrary(s_FiltersLib, sizeof(s_FiltersLib) / sizeof(s_FiltersLib[0]), "Filter");
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

    const int video_w = results[0];
    const int video_h = results[1];
    const int layers = results[2];
    const int colorspace = results[3];

    //if (layers != 3) { return; }
    //if (colorspace != AL::kRGBColorSpace) { return; }

    //CHECK(layers == 3);
    //CHECK(colorspace == AL::kRGBColorSpace);
    CHECK(_texture->Sizei().x == video_w);
    CHECK(_texture->Sizei().y == video_h);

    // QQVGA = 160x120
    // QVGA = 320x240
    // VGA = 640x480
    // 4VGA = 1280x960

    if (colorspace == AL::kRGBColorSpace)
    {
        CHECK(layers == 3);

        const int w = video_w;
        const int h = video_h;

        uint32_t* buffer = new uint32_t[w * h];
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const int src_index = (x + y * w) * layers;
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

        _texture->Bind();
        _texture->SubData(buffer, w, h);
        _texture->Unbind();

        delete buffer;
    }
    else if (colorspace == AL::kYUVColorSpace)
    {
        CHECK(layers == 3);

        const int w = video_w;
        const int h = video_h;

        uint32_t* buffer = new uint32_t[w * h];
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const int src_index = (x + y * w) * 3;
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

        _texture->Bind();
        _texture->SubData(buffer, w, h);
        _texture->Unbind();

        delete buffer;
    }


    // flip - handled in camera code now
    //for (int y = 0; y < h / 2; ++y)
    //{
        //uint32_t line[w];
        //memcpy(line, buffer + y * w, w * sizeof(uint32_t));
        //memcpy(buffer + y * w, buffer + (h - y - 1) * w, w * sizeof(uint32_t));
        //memcpy(buffer + (h - y - 1) * w, line, w * sizeof(uint32_t));
    //}


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
