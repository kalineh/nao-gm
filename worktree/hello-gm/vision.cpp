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

#include <math.h>

#define GLM_MESSAGES
#define GLM_FORCE_INLINE
#define GLM_FORCE_SSE2
#include <glm/glm.hpp>
#include <glm/gtx/simd_vec4.hpp>
#include <glm/gtc/swizzle.hpp>

// Performance notes (release build):

// sobel using naive integer inline convolution: 2ms (suprising)
// bilateral using naive integer inline convolution (3x3 window): 90ms
// boxblur using vec4 3x3 kernel: 8ms
// gaussianblur using vec4 5x5 kernel: 8ms
// gaussianblur using vec4 2x 3x1 kernel: 7ms
// gaussianblur using vec4 2x 3x1 kernel with imagecache: 6ms
// gaussianblur using simdVec4 2x 5x1 kernel with imagecache: 7ms
// gaussianblur using simdVec4 2x 5x1 kernel with imagecache: 5.5ms
// gaussianblur using simdVec4 2x 5x1 kernel with imagecache with simd vectorize: 3.5ms

using namespace funk;

const glm::vec4 LuminanceCoefficientARGB = glm::vec4(1.0f, 0.2126f, 0.7152f, 0.0722f);
const glm::vec4 LuminancePerceivedCoefficientARGB = glm::vec4(1.0f, 0.299f, 0.587f, 0.114f);
const glm::simdVec4 simdLuminanceCoefficientARGB = glm::simdVec4(1.0f, 0.2126f, 0.7152f, 0.0722f);
const glm::simdVec4 simdLuminancePerceivedCoefficientARGB = glm::simdVec4(1.0f, 0.299f, 0.587f, 0.114f);

class ImageCache
{
public:
    struct Image
    {
        bool used;
        int width;
        int height;
        int pixelsize;
        void* data;
    };

private:

    std::vector<Image> _cache;

public:

    template <class PixelType>
    PixelType* Pop(int width, int height)
    {
        const int pixelsize = sizeof(PixelType);

        for (int i = 0; i < (int)_cache.size(); ++i)
        {
            Image& image = _cache[i];

            if (image.used)
                continue;
            if (image.width != width)
                continue;
            if (image.height != height)
                continue;
            if (image.pixelsize != pixelsize)
                continue;

            image.used = true;
            return (PixelType*)image.data;
        }

        // require aligned allocation for simd
        void* allocation = _aligned_malloc(width * height * pixelsize, 16);

        Image newimage = {
            true,
            width,
            height,
            pixelsize,
            allocation,
        };
        
        _cache.push_back(newimage);
        return (PixelType*)newimage.data;
    }

    void Push(void* data)
    {
        for (int i = 0; i < (int)_cache.size(); ++i)
        {
            Image& image = _cache[i];
            if (image.data == data)
            {
                image.used = false; 
                break;
            }
        }
    }
};

ImageCache g_imagecache;

struct ImageView
{
    int width;
    int height;

    explicit ImageView(int w, int h)
        : width(w)
        , height(h)
    {
    }

    int indexof(int x, int y) const
    {
        x = clamp(x, 0, width - 1);
        y = clamp(y, 0, height - 1);

        return x + y * width;
    }
};

// fast integer clamping
//x -= a;
//x &= (~x) >> 31;
//x += a;
//x -= b;
//x &= x >> 31;
//x += b;

void VectorizeImageARGBALLL(glm::simdVec4* out, uint32_t* in, int w, int h)
{
    const glm::simdVec4 d = glm::simdVec4(1.0f / 255.0f);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const int i = x + y * w;
            const uint32_t pixel = in[i];
            const uint8_t a = (pixel & 0xFF000000) >> 24;
            const uint8_t r = (pixel & 0x00FF0000) >> 16;
            const uint8_t g = (pixel & 0x0000FF00) >> 8;
            const uint8_t b = (pixel & 0x000000FF) >> 0;
            const glm::simdVec4 v = glm::simdVec4(a, r, g, b) * d * simdLuminanceCoefficientARGB;

            // TODO: replace with horizontal-add
            const glm::simdVec4 o = 
                v.swizzle<glm::X, glm::Y, glm::Z, glm::W>() +
                v.swizzle<glm::X, glm::Z, glm::W, glm::Y>() +
                v.swizzle<glm::X, glm::W, glm::Y, glm::Z>();

            // scale alpha back down
            const glm::simdVec4 oo = o * glm::simdVec4(1.0f / 3.0f, 1.0f, 1.0f, 1.0f);
            
            out[i] = oo;
        }
    }
}

void VectorizeImageARGBARGB(glm::vec4* out, uint32_t* in, int w, int h)
{
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const int i = x + y * w;

            const uint32_t pixel = in[i];
            const uint8_t a = (pixel & 0xFF000000) >> 24;
            const uint8_t r = (pixel & 0x00FF0000) >> 16;
            const uint8_t g = (pixel & 0x0000FF00) >> 8;
            const uint8_t b = (pixel & 0x000000FF) >> 0;
            const glm::vec4 v = glm::vec4(a, r, g, b) / 255.0f;

            out[i] = v;
        }
    }
}

void VectorizeImageARGBARGB(glm::simdVec4* out, uint32_t* in, int w, int h)
{
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const int i = x + y * w;

            const uint32_t pixel = in[i];
            const uint8_t a = (pixel & 0xFF000000) >> 24;
            const uint8_t r = (pixel & 0x00FF0000) >> 16;
            const uint8_t g = (pixel & 0x0000FF00) >> 8;
            const uint8_t b = (pixel & 0x000000FF) >> 0;
            const glm::simdVec4 v = glm::simdVec4(a, r, g, b) / 255.0f;

            out[i] = v;
        }
    }
}

void RestoreAlphaARGB(glm::simdVec4* out, glm::simdVec4* in, int w, int h)
{
    const glm::simdVec4 m = glm::simdVec4(0.0f, 1.0f, 1.0f, 1.0f);
    const glm::simdVec4 a = glm::simdVec4(1.0f, 0.0f, 0.0f, 0.0f);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const int i = x + y * w;
            out[i] = in[i] * m + a;
        }
    }
}

void UnvectorizeImageARGBARGB(uint32_t* out, glm::vec4* in, int w, int h)
{
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const int i = x + y * w;

            const glm::vec4 pixel = in[i] * 255.0f;
            const uint8_t a = uint8_t(pixel.x);
            const uint8_t r = uint8_t(pixel.y);
            const uint8_t g = uint8_t(pixel.z);
            const uint8_t b = uint8_t(pixel.w);

            const uint32_t v = 
                (a << 24) |
                (r << 16) |
                (g <<  8) |
                (b <<  0);

            out[i] = v;
        }
    }
}

void UnvectorizeImageARGBARGB(uint32_t* out, glm::simdVec4* in, int w, int h)
{
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const int i = x + y * w;

            const glm::vec4 pixel = glm::vec4_cast(in[i] * 255.0f);
            const uint8_t a = uint8_t(pixel.x);
            const uint8_t r = uint8_t(pixel.y);
            const uint8_t g = uint8_t(pixel.z);
            const uint8_t b = uint8_t(pixel.w);

            const uint32_t v = 
                (a << 24) |
                (r << 16) |
                (g <<  8) |
                (b <<  0);

            out[i] = v;
        }
    }
}

void Convolve3x3(glm::vec4* out, glm::vec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a b c
            // d e f
            // g h i

            const glm::vec4 a = in[view.indexof(x - 1, y - 1)];
            const glm::vec4 b = in[view.indexof(x + 0, y - 1)];
            const glm::vec4 c = in[view.indexof(x + 1, y - 1)];
            const glm::vec4 d = in[view.indexof(x - 1, y + 0)];
            const glm::vec4 e = in[view.indexof(x + 0, y + 0)];
            const glm::vec4 f = in[view.indexof(x + 1, y + 0)];
            const glm::vec4 g = in[view.indexof(x - 1, y + 1)];
            const glm::vec4 h = in[view.indexof(x + 0, y + 1)];
            const glm::vec4 i = in[view.indexof(x + 1, y + 1)];

            out[view.indexof(x, y)] = 
                a * kernel[0] + b * kernel[1] + c * kernel[2] +
                d * kernel[3] + e * kernel[4] + f * kernel[5] +
                g * kernel[6] + h * kernel[7] + i * kernel[8];
        }
    }
}

void Convolve3x3(glm::simdVec4* out, glm::simdVec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a b c
            // d e f
            // g h i

            const glm::simdVec4 a = in[view.indexof(x - 1, y - 1)];
            const glm::simdVec4 b = in[view.indexof(x + 0, y - 1)];
            const glm::simdVec4 c = in[view.indexof(x + 1, y - 1)];
            const glm::simdVec4 d = in[view.indexof(x - 1, y + 0)];
            const glm::simdVec4 e = in[view.indexof(x + 0, y + 0)];
            const glm::simdVec4 f = in[view.indexof(x + 1, y + 0)];
            const glm::simdVec4 g = in[view.indexof(x - 1, y + 1)];
            const glm::simdVec4 h = in[view.indexof(x + 0, y + 1)];
            const glm::simdVec4 i = in[view.indexof(x + 1, y + 1)];

            out[view.indexof(x, y)] = 
                a * kernel[0] + b * kernel[1] + c * kernel[2] +
                d * kernel[3] + e * kernel[4] + f * kernel[5] +
                g * kernel[6] + h * kernel[7] + i * kernel[8];
        }
    }
}

void Convolve3x1(glm::vec4* out, glm::vec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a b c

            const glm::vec4 a = in[view.indexof(x - 1, y)];
            const glm::vec4 b = in[view.indexof(x + 0, y)];
            const glm::vec4 c = in[view.indexof(x + 1, y)];

            out[view.indexof(x, y)] = 
                a * kernel[0] + b * kernel[1] + c * kernel[2];
        }
    }
}

void Convolve1x3(glm::vec4* out, glm::vec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a
            // b
            // c

            const glm::vec4 a = in[view.indexof(x, y - 1)];
            const glm::vec4 b = in[view.indexof(x, y + 0)];
            const glm::vec4 c = in[view.indexof(x, y + 1)];

            out[view.indexof(x, y)] = 
                a * kernel[0] + b * kernel[1] + c * kernel[2];
        }
    }
}

void Convolve3x1(glm::simdVec4* out, glm::simdVec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a b c

            const glm::simdVec4 a = in[view.indexof(x - 1, y)];
            const glm::simdVec4 b = in[view.indexof(x + 0, y)];
            const glm::simdVec4 c = in[view.indexof(x + 1, y)];

            out[view.indexof(x, y)] = 
                a * kernel[0] + b * kernel[1] + c * kernel[2];
        }
    }
}

void Convolve1x3(glm::simdVec4* out, glm::simdVec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a
            // b
            // c

            const glm::simdVec4 a = in[view.indexof(x, y - 1)];
            const glm::simdVec4 b = in[view.indexof(x, y + 0)];
            const glm::simdVec4 c = in[view.indexof(x, y + 1)];

            out[view.indexof(x, y)] = 
                a * kernel[0] + b * kernel[1] + c * kernel[2];
        }
    }
}

void Convolve5x1(glm::vec4* out, glm::vec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a b c d e

            const glm::simdVec4 a = glm::simdVec4(in[view.indexof(x - 2, y)]);
            const glm::simdVec4 b = glm::simdVec4(in[view.indexof(x - 1, y)]);
            const glm::simdVec4 c = glm::simdVec4(in[view.indexof(x + 0, y)]);
            const glm::simdVec4 d = glm::simdVec4(in[view.indexof(x + 1, y)]);
            const glm::simdVec4 e = glm::simdVec4(in[view.indexof(x + 2, y)]);

            out[view.indexof(x, y)] = 
                glm::vec4_cast(a * kernel[0] + b * kernel[1] + c * kernel[2] + d * kernel[3] + e * kernel[4]);
        }
    }
}

void Convolve1x5(glm::vec4* out, glm::vec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a
            // b
            // c
            // d
            // e

            const glm::simdVec4 a = glm::simdVec4(in[view.indexof(x, y - 2)]);
            const glm::simdVec4 b = glm::simdVec4(in[view.indexof(x, y - 1)]);
            const glm::simdVec4 c = glm::simdVec4(in[view.indexof(x, y + 0)]);
            const glm::simdVec4 d = glm::simdVec4(in[view.indexof(x, y + 1)]);
            const glm::simdVec4 e = glm::simdVec4(in[view.indexof(x, y + 2)]);

            out[view.indexof(x, y)] = 
                glm::vec4_cast(a * kernel[0] + b * kernel[1] + c * kernel[2] + d * kernel[3] + e * kernel[4]);
        }
    }
}

void Convolve5x1(glm::simdVec4* out, glm::simdVec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a b c d e

            const glm::simdVec4 a = in[view.indexof(x - 2, y)];
            const glm::simdVec4 b = in[view.indexof(x - 1, y)];
            const glm::simdVec4 c = in[view.indexof(x + 0, y)];
            const glm::simdVec4 d = in[view.indexof(x + 1, y)];
            const glm::simdVec4 e = in[view.indexof(x + 2, y)];

            out[view.indexof(x, y)] = 
                a * kernel[0] + b * kernel[1] + c * kernel[2] + d * kernel[3] + e * kernel[4];
        }
    }
}

void Convolve1x5(glm::simdVec4* out, glm::simdVec4* in, const ImageView& view, const float kernel[9])
{
    for (int y = 0; y < view.height; ++y)
    {
        for (int x = 0; x < view.width; ++x)
        {
            // a
            // b
            // c
            // d
            // e

            const glm::simdVec4 a = in[view.indexof(x, y - 2)];
            const glm::simdVec4 b = in[view.indexof(x, y - 1)];
            const glm::simdVec4 c = in[view.indexof(x, y + 0)];
            const glm::simdVec4 d = in[view.indexof(x, y + 1)];
            const glm::simdVec4 e = in[view.indexof(x, y + 2)];

            out[view.indexof(x, y)] = 
                a * kernel[0] + b * kernel[1] + c * kernel[2] + d * kernel[3] + e * kernel[4];
        }
    }
}

void Filters::SobelARGBNaive(StrongHandle<Texture> out, StrongHandle<Texture> in, int threshold)
{
    CHECK(in->Sizei() == out->Sizei());

    // ignore edges because simplicity

    int border = 1;

    const int w = in->Sizei().x;
    const int h = in->Sizei().y;
    const int pixelsize = 4;

    uint32_t* buffer_in = new uint32_t[w * h];
    uint32_t* buffer_out = new uint32_t[w * h];
    //glm::vec4* buffer_work = new glm::vec4[w * h]

    in->Bind(0);
    in->GetTexImage(buffer_in);
    in->Unbind();

    glFinish();

    // test copy
    //memcpy(buffer_out, buffer_in, w * h * pixelsize);

    // a, b, c
    // d, e, f
    // g, h, i

    const int kernel_x[] = {
        -1, +0, +1,
        -2, +0, +2,
        -1, +0, +1,
    };

    struct
    {
        int compute(const int kernel[], int a, int d, int g, int c, int f, int i)
        {
            return 
                a * kernel[0] +
                d * kernel[3] +
                g * kernel[6] +
                c * kernel[2] +
                f * kernel[5] +
                i * kernel[8];
        }
    } GX;

    const int kernel_y[] = {
        -1, -2, -1,
        +0, +0, +0,
        -1, +2, +1,
    };

    struct
    {
        int compute(const int kernel[], int a, int b, int c, int g, int h, int i)
        {
            return 
                a * kernel[0] +
                b * kernel[1] +
                c * kernel[2] +
                g * kernel[6] +
                h * kernel[7] +
                i * kernel[8];
        }
    } GY;

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
            const uint32_t d = buffer_in[((x - 1) + (y + 0) * w)];
            const uint32_t f = buffer_in[((x + 1) + (y + 0) * w)];
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

            const uint8_t dr = (d & 0x00FF0000) >> 16;
            const uint8_t dg = (d & 0x0000FF00) >> 8;
            const uint8_t db = (d & 0x000000FF) >> 0;

            const uint8_t fr = (f & 0x00FF0000) >> 16;
            const uint8_t fg = (f & 0x0000FF00) >> 8;
            const uint8_t fb = (f & 0x000000FF) >> 0;

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
            // -2,  0, +2
            // -1, -2, +1

            int sum_rx = GX.compute(kernel_x, ar, dr, gr, cr, fr, ir);
            int sum_gx = GX.compute(kernel_x, ag, dg, gg, cg, fg, ig);
            int sum_bx = GX.compute(kernel_x, ab, db, gb, cb, fb, ib);

            if (sum_rx < 0) sum_rx = 0;
            if (sum_gx < 0) sum_gx = 0;
            if (sum_bx < 0) sum_bx = 0;

            if (sum_rx > 255) sum_rx = 255;
            if (sum_gx > 255) sum_gx = 255;
            if (sum_bx > 255) sum_bx = 255;

            int sum_ry = GY.compute(kernel_y, ar, br, cr, gr, hr, ir);
            int sum_gy = GY.compute(kernel_y, ag, bg, cg, gg, hg, ig);
            int sum_by = GY.compute(kernel_y, ab, bb, cb, gb, hb, ib);

            if (sum_ry < 0) sum_ry = 0;
            if (sum_gy < 0) sum_gy = 0;
            if (sum_by < 0) sum_by = 0;

            if (sum_ry > 255) sum_ry = 255;
            if (sum_gy > 255) sum_gy = 255;
            if (sum_by > 255) sum_by = 255;

            // sum to white
            int sum = sum_rx + sum_gx + sum_bx + sum_ry + sum_gy + sum_by;
            if (sum < 0) sum = 0;
            if (sum > 255) sum = 255;
            if (sum < threshold) sum = 0;

            //sum = 255 - sum;

            // ABGR
            const uint32_t pixel =
                (buffer_in[pixel_index] & 0xFF000000) |
                (sum << 16) |
                (sum << 8) |
                (sum << 0);

            buffer_out[pixel_index] = pixel;
        }
    }

    out->Bind();
    out->SubData(buffer_out, w, h, 0, 0);
    out->Unbind();

    delete buffer_in;
    delete buffer_out;
}

void Filters::SobelARGB(StrongHandle<Texture> out, StrongHandle<Texture> in, int threshold)
{
    CHECK(in->Sizei() == out->Sizei());

    const int w = in->Sizei().x;
    const int h = in->Sizei().y;
    const int pixelsize = 4;

    uint32_t* buffer_in = g_imagecache.Pop<uint32_t>(w, h);
    uint32_t* buffer_out = g_imagecache.Pop<uint32_t>(w, h);
    glm::simdVec4* buffer_vin = g_imagecache.Pop<glm::simdVec4>(w, h);
    glm::simdVec4* buffer_vout = g_imagecache.Pop<glm::simdVec4>(w, h);

    in->Bind(0);
    in->GetTexImage(buffer_in);
    in->Unbind();

    glFinish();

    VectorizeImageARGBALLL(buffer_vin, buffer_in, w, h);

    ImageView view(w, h);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const glm::simdVec4 p00 = buffer_vin[view.indexof(x - 1, y - 1)];
            const glm::simdVec4 p10 = buffer_vin[view.indexof(x + 0, y - 1)];
            const glm::simdVec4 p20 = buffer_vin[view.indexof(x + 1, y - 1)];

            const glm::simdVec4 p01 = buffer_vin[view.indexof(x - 1, y + 0)];
            const glm::simdVec4 p11 = buffer_vin[view.indexof(x + 0, y + 0)];
            const glm::simdVec4 p21 = buffer_vin[view.indexof(x + 1, y + 0)];

            const glm::simdVec4 p02 = buffer_vin[view.indexof(x - 1, y + 1)];
            const glm::simdVec4 p12 = buffer_vin[view.indexof(x + 0, y + 1)];
            const glm::simdVec4 p22 = buffer_vin[view.indexof(x + 1, y + 1)];

            const glm::simdVec4 sx =
                p00 * -1.0f + p20 * +1.0f +
                p01 * -2.0f + p21 * +2.0f +
                p02 * -1.0f + p22 * +1.0f;

            const glm::simdVec4 sy =
                p00 * -1.0f + p10 * -2.0f + p20 * -1.0f +
                p02 * +1.0f + p12 * +2.0f + p22 * +1.0f;

            // abs not working?
            //const glm::simdVec4 sqr = glm::abs(sx) + glm::abs(sy);
            const glm::simdVec4 sqr = sx * sx + sy * sy;
            const glm::simdVec4 p = glm::clamp(sqr, glm::simdVec4(0.0f), glm::simdVec4(1.0f));
            const float thresholdf = float(threshold) / 255.0f;
            const glm::simdVec4 edge = glm::simdVec4(1.0f, thresholdf, thresholdf, thresholdf);

            // mask workaround
            const float maskf = glm::vec4_cast(p).w < thresholdf ? 0.0f : 1.0f;
            const glm::simdVec4 mask = glm::simdVec4(1.0f, maskf, maskf, maskf);
            //const glm::simdVec4 mask = glm::step(thresholdf, p);

            const glm::simdVec4 o = p * mask;

            // step is broken? this gives 1,1,1,1 for both
            //const glm::simdVec4 maska = glm::step(p * 1.1f, p);
            //const glm::simdVec4 maskb = glm::step(p * 0.9f, p);

            buffer_vout[view.indexof(x, y)] = o;
        }
    }

    RestoreAlphaARGB(buffer_vout, buffer_vout, w, h);
    UnvectorizeImageARGBARGB(buffer_out, buffer_vout, w, h);

    out->Bind();
    out->SubData(buffer_out, w, h, 0, 0);
    out->Unbind();

    g_imagecache.Push(buffer_in);
    g_imagecache.Push(buffer_out);
    g_imagecache.Push(buffer_vin);
    g_imagecache.Push(buffer_vout);
}

void Filters::BilateralARGBNaive(StrongHandle<Texture> out, StrongHandle<Texture> in, float spatial_sigma, float edge_sigma)
{
    CHECK(in->Sizei() == out->Sizei());

    // ignore edges because simplicity

    const int w = in->Sizei().x;
    const int h = in->Sizei().y;
    const int pixelsize = 4;

    uint32_t* buffer_in = new uint32_t[w * h];
    uint32_t* buffer_out = new uint32_t[w * h];
    uint8_t* buffer_work = new uint8_t[w * h];

    in->Bind(0);
    in->GetTexImage(buffer_in);
    in->Unbind();

    glFinish();

    // filter

    const int window_x = 3;
    const int window_y = 3;

    struct {
        inline uint32_t get(uint32_t value)
        {
            // roughly approximate luminance (divisor low to enhance brightness a little)
            return (
                ((value & 0x00FF0000) >> 16) * 2 +
                ((value & 0x0000FF00) >> 8) * 5 +
                ((value & 0x000000FF) >> 0) * 1) / 8;
        }
    } sum_rgb;

    // horizontal pass

    for (int y = window_y; y < h - window_y; ++y)
    {
        for (int x = window_x; x < w - window_x; ++x)
        {
            float num = 0.0f;
            float den = 0.0f;

            for (int i = x - window_x; i < x + window_x; ++i)
            {
                const uint32_t src_pixel = buffer_in[x + y * w];
                const uint32_t src_sum = sum_rgb.get(src_pixel);

                const uint32_t window_pixel = buffer_in[i + y * w];
                const uint32_t window_sum = sum_rgb.get(window_pixel);

                // spatial weight
                const float sw1 = float(abs(i - x)) / spatial_sigma;
                const float sw2 = pow(sw1, 2.0f);
                const float sw = exp(-0.5f * sw2);

                // edge weight
                const float ew1 = float(window_sum - src_sum);
                const float ew2 = abs(ew1 / edge_sigma);
                const float ew3 = pow(ew2, 2.0f);
                const float ew = exp(-0.5f * ew3);

                num += window_sum * sw * ew;
                den += sw * ew;
            }

            buffer_work[x + y * w] = uint8_t(num / den);
        }
    }

    // vertical pass

    for (int y = window_y; y < h - window_y; ++y)
    {
        for (int x = window_x; x < w - window_x; ++x)
        {
            float num = 0.0f;
            float den = 0.0f;

            for (int i = y - window_y; i < y + window_y; ++i)
            {
                const uint32_t src_pixel = buffer_in[x + y * w];
                const uint32_t src_sum = sum_rgb.get(src_pixel);

                const uint32_t window_pixel = buffer_in[x + i * w];
                const uint32_t window_sum = sum_rgb.get(window_pixel);

                // spatial weight
                const float sw1 = float(abs(i - y)) / spatial_sigma;
                const float sw2 = pow(sw1, 2.0f);
                const float sw = exp(-0.5f * sw2);

                // edge weight
                const float ew1 = float(window_sum - src_sum);
                const float ew2 = abs(ew1 / edge_sigma);
                const float ew3 = pow(ew2, 2.0f);
                const float ew = exp(-0.5f * ew3);

                const uint8_t work_pixel = buffer_work[x + i * w];

                num += float(work_pixel) * sw * ew;
                den += sw * ew;
            }

            const uint32_t normalized = uint32_t((num / den) / 3.0f);
            const uint32_t clamped = std::min<uint32_t>(normalized, 255);
            const uint8_t result = uint8_t(clamped);
            buffer_out[x + y * w] = 
                (0xFF000000) |
                (result << 16) |
                (result << 8) |
                (result << 0);
        }
    }

    out->Bind();
    out->SubData(buffer_out, w, h, 0, 0);
    out->Unbind();

    delete buffer_in;
    delete buffer_out;
    delete buffer_work;
}

float GaussianSample(float x, float sigma)
{
    const float PI = 3.1415926535f;

    return exp(-0.5f * pow(x / sigma, 2.0f)) / (2.0f * PI * sigma * sigma);
}

void MakeGaussianKernel1D(float* out, int w, float sigma)
{
    float sum = 0.0f;

    // floor() by integer division
    float pivot_x = float(w / 2);

    for (int x = 0; x < w; ++x)
    {
        const float g = GaussianSample(float(x) - pivot_x, sigma);
        out[x] = g;
        sum += g;
    }

    for (int x = 0; x < w; ++x)
    {
        out[x] /= sum;
    }
}

void Filters::BilateralARGB(StrongHandle<Texture> out, StrongHandle<Texture> in, float spatial_sigma, float edge_sigma)
{
    CHECK(in->Sizei() == out->Sizei());

    const int w = in->Sizei().x;
    const int h = in->Sizei().y;
    const int pixelsize = 4;

    uint32_t* buffer_in = g_imagecache.Pop<uint32_t>(w, h);
    uint32_t* buffer_out = g_imagecache.Pop<uint32_t>(w, h);
    glm::vec4* buffer_vin = g_imagecache.Pop<glm::vec4>(w, h);
    glm::vec4* buffer_vout = g_imagecache.Pop<glm::vec4>(w, h);

    in->Bind(0);
    in->GetTexImage(buffer_in);
    in->Unbind();

    glFinish();

    VectorizeImageARGBARGB(buffer_vin, buffer_in, w, h);

    float kernel[5] = { 0.0f };
    MakeGaussianKernel1D(kernel, 5, 1.0f);

    ImageView view(w, h);

    // how do we convolve bilateral?

    // for pixel p in window pixels q
    //   where t = location
    //   where i = intensity
    //   p = G(pt - qt) * G(pi - qi)

    // can we generate a convolution filter?
    //  > no, because intensity is required
    //  > we could cache G(a - b) for all a and b
    //    > a and b for intensity is bounded 0..255 (per channel) (256*256*sizeof(float), 256kb)
    //    > a and b for position is bounded around -window to window (window*window*sizeof(float), 16kb for window=4)
    // 

    Convolve5x1(buffer_vout, buffer_vin, view, kernel);
    Convolve1x5(buffer_vout, buffer_vout, view, kernel);

    UnvectorizeImageARGBARGB(buffer_out, buffer_vout, w, h);

    out->Bind();
    out->SubData(buffer_out, w, h, 0, 0);
    out->Unbind();

    g_imagecache.Push(buffer_in);
    g_imagecache.Push(buffer_out);
    g_imagecache.Push(buffer_vin);
    g_imagecache.Push(buffer_vout);
}

void Filters::GaussianBlurARGB(StrongHandle<Texture> out, StrongHandle<Texture> in, float sigma)
{
    CHECK(in->Sizei() == out->Sizei());

    const int w = in->Sizei().x;
    const int h = in->Sizei().y;
    const int pixelsize = 4;

    uint32_t* buffer_in = g_imagecache.Pop<uint32_t>(w, h);
    uint32_t* buffer_out = g_imagecache.Pop<uint32_t>(w, h);
    glm::simdVec4* buffer_vin = g_imagecache.Pop<glm::simdVec4>(w, h);
    glm::simdVec4* buffer_vout = g_imagecache.Pop<glm::simdVec4>(w, h);

    in->Bind(0);
    in->GetTexImage(buffer_in);
    in->Unbind();

    glFinish();

    VectorizeImageARGBARGB(buffer_vin, buffer_in, w, h);

    float kernel[5] = { 0.0f };
    MakeGaussianKernel1D(kernel, 5, sigma);

    ImageView view(w, h);

    Convolve5x1(buffer_vout, buffer_vin, view, kernel);
    Convolve1x5(buffer_vout, buffer_vout, view, kernel);

    UnvectorizeImageARGBARGB(buffer_out, buffer_vout, w, h);

    out->Bind();
    out->SubData(buffer_out, w, h, 0, 0);
    out->Unbind();

    g_imagecache.Push(buffer_in);
    g_imagecache.Push(buffer_out);
    g_imagecache.Push(buffer_vin);
    g_imagecache.Push(buffer_vout);
}

void Filters::BoxBlurARGB(StrongHandle<Texture> out, StrongHandle<Texture> in)
{
    CHECK(in->Sizei() == out->Sizei());

    // ignore edges because simplicity

    const int w = in->Sizei().x;
    const int h = in->Sizei().y;
    const int pixelsize = 4;

    uint32_t* buffer_in = new uint32_t[w * h];
    uint32_t* buffer_out = new uint32_t[w * h];
    glm::vec4* buffer_vin = new glm::vec4[w * h];
    glm::vec4* buffer_vout = new glm::vec4[w * h];

    in->Bind(0);
    in->GetTexImage(buffer_in);
    in->Unbind();

    glFinish();

    VectorizeImageARGBARGB(buffer_vin, buffer_in, w, h);

    const float kernel[9] = {
        1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
        1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
        1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
    };

    ImageView view(w, h);
    Convolve3x3(buffer_vout, buffer_vin, view, kernel);
    UnvectorizeImageARGBARGB(buffer_out, buffer_vout, w, h);

    out->Bind();
    out->SubData(buffer_out, w, h, 0, 0);
    out->Unbind();

    delete buffer_in;
    delete buffer_out;
    delete buffer_vin;
    delete buffer_vout;
}

static int GM_CDECL gmfFilterSobelARGB(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(3);

	GM_CHECK_USER_PARAM_PTR( Texture, out, 0 );
	GM_CHECK_USER_PARAM_PTR( Texture, in, 1 );
	GM_CHECK_INT_PARAM(threshold, 2);

    Filters::SobelARGB(out, in, threshold);

	return GM_OK;
}

static int GM_CDECL gmfFilterBilateralARGB(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(4);

	GM_CHECK_USER_PARAM_PTR( Texture, out, 0 );
	GM_CHECK_USER_PARAM_PTR( Texture, in, 1 );
    GM_CHECK_FLOAT_PARAM( spatial_sigma, 2 );
    GM_CHECK_FLOAT_PARAM( edge_sigma, 2 );

    Filters::BilateralARGB(out, in, spatial_sigma, edge_sigma);

	return GM_OK;
}

static int GM_CDECL gmfFilterBoxBlurARGB(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(2);

	GM_CHECK_USER_PARAM_PTR( Texture, out, 0 );
	GM_CHECK_USER_PARAM_PTR( Texture, in, 1 );

    Filters::BoxBlurARGB(out, in);

	return GM_OK;
}

static int GM_CDECL gmfFilterGaussianBlurARGB(gmThread * a_thread)
{
	GM_CHECK_NUM_PARAMS(3);

	GM_CHECK_USER_PARAM_PTR( Texture, out, 0 );
	GM_CHECK_USER_PARAM_PTR( Texture, in, 1 );
    GM_CHECK_FLOAT_PARAM( sigma, 2 );

    Filters::GaussianBlurARGB(out, in, sigma);

	return GM_OK;
}

static gmFunctionEntry s_FiltersLib[] = 
{ 
	{ "SobelARGB", gmfFilterSobelARGB },
	{ "BilateralARGB", gmfFilterBilateralARGB },
	{ "BoxBlurARGB", gmfFilterBoxBlurARGB },
	{ "GaussianBlurARGB", gmfFilterGaussianBlurARGB },
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
