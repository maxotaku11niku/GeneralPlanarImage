/* gpitool - Converts images into .GPI format
 * Copyright (c) 2024 Maxim Hoxha
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Main image handler
 */

extern "C"
{
    #include <png.h>
    #include <jpeglib.h>
}
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include "imagehandler.h"

//Controls the amount to expand our rectangle of interest in each direction in order to give some "burn in" to error diffusion
#define EDD_EXPAND_X        19
#define EDD_EXPAND_Y_TOP    19
#define EDD_EXPAND_Y_BOTTOM 3

const ColourRGBA8 defaultPalette[16] =
{
    { 0x11, 0x11, 0x11, 0xFF },
    { 0x77, 0x77, 0x77, 0xFF },
    { 0xBB, 0x33, 0xBB, 0xFF },
    { 0xFF, 0x77, 0xFF, 0xFF },
    { 0x77, 0x11, 0x11, 0xFF },
    { 0xDD, 0x44, 0x44, 0xFF },
    { 0xFF, 0xBB, 0x77, 0xFF },
    { 0xCC, 0xBB, 0x33, 0xFF },
    { 0x22, 0x77, 0x33, 0xFF },
    { 0x55, 0xDD, 0x55, 0xFF },
    { 0x88, 0xFF, 0x55, 0xFF },
    { 0xFF, 0xFF, 0x66, 0xFF },
    { 0x33, 0x33, 0xBB, 0xFF },
    { 0x33, 0xAA, 0xFF, 0xFF },
    { 0x99, 0xFF, 0xFF, 0xFF },
    { 0xFF, 0xFF, 0xFF, 0xFF }
};

const float bayer2x2[4] = { -0.5f,   0.0f,
                             0.25f, -0.25f };

const float bayer4x4[16] = { -0.5f,     0.0f,    -0.375f,   0.125f,
                              0.25f,   -0.25f,    0.375f,  -0.125f,
                             -0.3125f,  0.1875f, -0.4375f,  0.0625f,
                              0.4375f, -0.0625f,  0.3125f, -0.1875f };

const float bayer8x8[64] = { -0.5f,       0.0f,      -0.375f,     0.125f,    -0.46875f,   0.03125f,  -0.34375f,   0.15625f,
                              0.25f,     -0.25f,      0.375f,    -0.125f,     0.28125f,  -0.21875f,   0.40625f,  -0.09375f,
                             -0.3125f,    0.1875f,   -0.4375f,    0.0625f,   -0.28125f,   0.21875f,  -0.40625f,   0.09375f,
                              0.4375f,   -0.0625f,    0.3125f,   -0.1875f,    0.46875f,  -0.03125f,   0.34375f,  -0.15625f,
                             -0.453125f,  0.046875f, -0.328125f,  0.171875f, -0.484375f,  0.015625f, -0.359375f,  0.140625f,
                              0.296875f, -0.203125f,  0.421875f, -0.078125f,  0.265625f, -0.234375f,  0.390625f, -0.109375f,
                             -0.265625f,  0.234375f, -0.390625f,  0.109375f, -0.296875f,  0.203125f, -0.421875f,  0.078125f,
                              0.484375f, -0.015625f,  0.359375f, -0.140625f,  0.453125f, -0.046875f,  0.328125f, -0.171875f };

const float bayer16x16[256] = { -0.5f,         0.0f,        -0.375f,       0.125f,      -0.46875f,     0.03125f,    -0.34375f,     0.15625f,    -0.4921875f,   0.0078125f,  -0.3671875f,   0.1328125f,  -0.4609375f,   0.0390625f,  -0.3359375f,   0.1640625f,
                                 0.25f,       -0.25f,        0.375f,      -0.125f,       0.28125f,    -0.21875f,     0.40625f,    -0.09375f,     0.2578125f,  -0.2421875f,   0.3828125f,  -0.1171875f,   0.2890625f,  -0.2109375f,   0.4140625f,  -0.0859375f,
                                -0.3125f,      0.1875f,     -0.4375f,      0.0625f,     -0.28125f,     0.21875f,    -0.40625f,     0.09375f,    -0.3046875f,   0.1953125f,  -0.4296875f,   0.0703125f,  -0.2734375f,   0.2265625f,  -0.3984375f,   0.1015625f,
                                 0.4375f,     -0.0625f,      0.3125f,     -0.1875f,      0.46875f,    -0.03125f,     0.34375f,    -0.15625f,     0.4453125f,  -0.0546875f,   0.3203125f,  -0.1796875f,   0.4765625f,  -0.0234375f,   0.3515625f,  -0.1484375f,
                                -0.453125f,    0.046875f,   -0.328125f,    0.171875f,   -0.484375f,    0.015625f,   -0.359375f,    0.140625f,   -0.4453125f,   0.0546875f,  -0.3203125f,   0.1796875f,  -0.4765625f,   0.0234375f,  -0.3515625f,   0.1484375f,
                                 0.296875f,   -0.203125f,    0.421875f,   -0.078125f,    0.265625f,   -0.234375f,    0.390625f,   -0.109375f,    0.3046875f,  -0.1953125f,   0.4296875f,  -0.0703125f,   0.2734375f,  -0.2265625f,   0.3984375f,  -0.1015625f,
                                -0.265625f,    0.234375f,   -0.390625f,    0.109375f,   -0.296875f,    0.203125f,   -0.421875f,    0.078125f,   -0.2578125f,   0.2421875f,  -0.3828125f,   0.1171875f,  -0.2890625f,   0.2109375f,  -0.4140625f,   0.0859375f,
                                 0.484375f,   -0.015625f,    0.359375f,   -0.140625f,    0.453125f,   -0.046875f,    0.328125f,   -0.171875f,    0.4921875f,  -0.0078125f,   0.3671875f,  -0.1328125f,   0.4609375f,  -0.0390625f,   0.3359375f,  -0.1640625f,
                                -0.48828125f,  0.01171875f, -0.36328125f,  0.13671875f, -0.45703125f,  0.04296875f, -0.33203125f,  0.16796875f, -0.49609375f,  0.00390625f, -0.37109375f,  0.12890625f, -0.46484375f,  0.03515625f, -0.33984375f,  0.16015625f,
                                 0.26171875f, -0.23828125f,  0.38671875f, -0.11328125f,  0.29296875f, -0.20703125f,  0.41796875f, -0.08203125f,  0.25390625f, -0.24609375f,  0.37890625f, -0.12109375f,  0.28515625f, -0.21484375f,  0.41015625f, -0.08984375f,
                                -0.30078125f,  0.19921875f, -0.42578125f,  0.07421875f, -0.26953125f,  0.23046875f, -0.39453125f,  0.10546875f, -0.30859375f,  0.19140625f, -0.43359375f,  0.06640625f, -0.27734375f,  0.22265625f, -0.40234375f,  0.09765625f,
                                 0.44921875f, -0.05078125f,  0.32421875f, -0.17578125f,  0.48046875f, -0.01953125f,  0.35546875f, -0.14453125f,  0.44140625f, -0.05859375f,  0.31640625f, -0.18359375f,  0.47265625f, -0.02734375f,  0.34765625f, -0.15234375f,
                                -0.44140625f,  0.05859375f, -0.31640625f,  0.18359375f, -0.47265625f,  0.02734375f, -0.34765625f,  0.15234375f, -0.44921875f,  0.05078125f, -0.32421875f,  0.17578125f, -0.48046875f,  0.01953125f, -0.35546875f,  0.14453125f,
                                 0.30859375f, -0.19140625f,  0.43359375f, -0.06640625f,  0.27734375f, -0.22265625f,  0.40234375f, -0.09765625f,  0.30078125f, -0.19921875f,  0.42578125f, -0.07421875f,  0.26953125f, -0.23046875f,  0.39453125f, -0.10546875f,
                                -0.25390625f,  0.24609375f, -0.37890625f,  0.12109375f, -0.28515625f,  0.21484375f, -0.41015625f,  0.08984375f, -0.26171875f,  0.23828125f, -0.38671875f,  0.11328125f, -0.29296875f,  0.20703125f, -0.41796875f,  0.08203125f,
                                 0.49609375f, -0.00390625f,  0.37109375f, -0.12890625f,  0.46484375f, -0.03515625f,  0.33984375f, -0.16015625f,  0.48828125f, -0.01171875f,  0.36328125f, -0.13671875f,  0.45703125f, -0.04296875f,  0.33203125f, -0.16796875f };

const float void16x16_1[256] = { -0.35546875f, -0.05859375f,  0.4765625f,   0.1796875f,  -0.1875f,     -0.046875f,   -0.421875f,    0.1171875f,  -0.16015625f,  0.46484375f, -0.09765625f, -0.40234375f,  0.12890625f,  0.28515625f, -0.27734375f, -0.4375f,
                                  0.0546875f,  -0.20703125f, -0.3046875f,   0.296875f,    0.08203125f, -0.2734375f,   0.390625f,    0.30859375f, -0.23046875f,  0.03515625f,  0.18359375f,  0.3515625f,  -0.00390625f, -0.484375f,   -0.08984375f,  0.37109375f,
                                  0.26171875f,  0.41796875f,  0.00390625f, -0.40625f,     0.2421875f,  -0.34765625f, -0.1328125f,   0.171875f,   -0.44140625f,  0.26953125f, -0.3671875f,  -0.1796875f,   0.07421875f,  0.484375f,   -0.15625f,     0.2109375f,
                                 -0.2421875f,  -0.125f,      -0.46484375f,  0.14453125f,  0.43359375f, -0.0859375f,   0.06640625f,  0.4921875f,  -0.3125f,     -0.0390625f,   0.4296875f,  -0.2578125f,   0.25f,       -0.3203125f,  -0.4140625f,   0.11328125f,
                                 -0.37890625f,  0.32421875f, -0.03515625f, -0.171875f,    0.34375f,    -0.24609375f, -0.4921875f,  -0.0078125f,   0.328125f,    0.09765625f, -0.109375f,   -0.46875f,     0.1640625f,   0.33203125f, -0.07421875f,  0.0234375f,
                                 -0.29296875f,  0.19140625f,  0.48046875f, -0.36328125f,  0.03125f,     0.203125f,    0.2578125f,  -0.21484375f, -0.38671875f, -0.1640625f,   0.2265625f,   0.39453125f, -0.0234375f,  -0.203125f,    0.4375f,      0.2734375f,
                                  0.3828125f,   0.05859375f, -0.2265625f,   0.109375f,   -0.30078125f, -0.42578125f,  0.40625f,    -0.0703125f,   0.125f,       0.45703125f, -0.28515625f, -0.4296875f,   0.046875f,   -0.34375f,     0.13671875f, -0.49609375f,
                                 -0.15234375f, -0.41796875f, -0.0546875f,   0.29296875f,  0.44140625f, -0.1484375f,   0.16015625f,  0.35546875f, -0.33984375f,  0.2890625f,   0.01171875f,  0.1875f,     -0.13671875f,  0.359375f,   -0.25390625f, -0.09375f,
                                  0.4609375f,   0.15234375f,  0.23046875f, -0.47265625f, -0.10546875f, -0.015625f,   -0.26953125f,  0.05078125f, -0.4609375f,  -0.1015625f,  -0.22265625f, -0.39453125f,  0.47265625f,  0.23828125f,  0.09375f,     0.0f,
                                 -0.375f,      -0.19921875f,  0.34765625f, -0.328125f,    0.0859375f,   0.3125f,     -0.3984375f,  -0.18359375f,  0.38671875f,  0.21484375f,  0.078125f,    0.31640625f, -0.0625f,     -0.44921875f, -0.32421875f,  0.30078125f,
                                  0.20703125f, -0.28125f,     0.02734375f,  0.40234375f, -0.234375f,    0.24609375f,  0.48828125f,  0.17578125f, -0.04296875f, -0.30859375f,  0.42578125f,  0.1328125f,  -0.265625f,   -0.17578125f,  0.4140625f,  -0.03125f,
                                  0.375f,      -0.12890625f,  0.12109375f, -0.4453125f,  -0.06640625f, -0.359375f,    0.01953125f, -0.5f,         0.27734375f, -0.14453125f, -0.43359375f, -0.3515625f,   0.0390625f,   0.265625f,    0.15625f,    -0.48046875f,
                                  0.0703125f,  -0.41015625f,  0.46875f,    -0.16796875f,  0.19921875f,  0.421875f,   -0.12109375f,  0.1015625f,  -0.26171875f,  0.453125f,   -0.01171875f,  0.36328125f, -0.1171875f,   0.49609375f, -0.23828125f, -0.078125f,
                                 -0.31640625f,  0.28125f,    -0.01953125f,  0.3203125f,   0.140625f,   -0.296875f,   -0.2109375f,   0.3359375f,  -0.3828125f,   0.1484375f,  -0.1953125f,   0.1953125f,  -0.45703125f,  0.08984375f, -0.390625f,    0.33984375f,
                                 -0.19140625f,  0.22265625f, -0.25f,       -0.37109375f,  0.04296875f, -0.453125f,    0.25390625f, -0.08203125f,  0.37890625f,  0.0625f,     -0.05078125f,  0.3046875f,  -0.3359375f,   0.234375f,   -0.140625f,    0.015625f,
                                  0.3984375f,   0.10546875f, -0.48828125f, -0.11328125f,  0.3671875f,   0.44921875f,  0.0078125f,  -0.33203125f,  0.21875f,    -0.4765625f,  -0.2890625f,   0.41015625f, -0.21875f,    -0.02734375f,  0.4453125f,   0.16796875f };

const float void16x16_2[256] = { -0.0546875f,  -0.17578125f,  0.1328125f,   0.25f,       -0.453125f,    0.41015625f,  0.21484375f, -0.42578125f,  0.28515625f, -0.22265625f, -0.44140625f,  0.01953125f, -0.1875f,      0.38671875f, -0.35546875f,  0.08984375f,
                                 -0.41015625f, -0.25390625f,  0.375f,      -0.3359375f,  -0.109375f,    0.11328125f, -0.0625f,     -0.31640625f,  0.04296875f, -0.1171875f,   0.3671875f,   0.125f,      -0.25f,        0.203125f,   -0.00390625f,  0.46484375f,
                                  0.29296875f,  0.03515625f, -0.48046875f, -0.01171875f,  0.4609375f,  -0.2421875f,   0.33984375f,  0.1484375f,   0.453125f,   -0.37109375f,  0.2265625f,   0.4921875f,  -0.328125f,   -0.12890625f, -0.44921875f,  0.15234375f,
                                 -0.10546875f,  0.40234375f,  0.234375f,   -0.14453125f,  0.30078125f, -0.39453125f, -0.19140625f, -0.48828125f, -0.0234375f,  -0.1640625f,   0.08203125f, -0.47265625f, -0.03515625f,  0.265625f,    0.34375f,    -0.2890625f,
                                  0.09375f,    -0.21484375f,  0.1640625f,  -0.30859375f,  0.0703125f,   0.1875f,      0.015625f,    0.390625f,    0.26953125f, -0.28125f,     0.3203125f,  -0.078125f,    0.41796875f,  0.06640625f, -0.171875f,   -0.390625f,
                                 -0.04296875f, -0.36328125f,  0.48828125f, -0.43359375f, -0.0703125f,   0.43359375f, -0.34765625f, -0.09765625f,  0.20703125f, -0.23046875f, -0.421875f,    0.17578125f, -0.3515625f,  -0.26171875f,  0.22265625f,  0.4453125f,
                                  0.12890625f,  0.0078125f,   0.28125f,    -0.15625f,     0.359375f,   -0.2734375f,   0.13671875f, -0.45703125f,  0.05078125f,  0.46875f,     0.109375f,    0.37890625f, -0.203125f,    0.02734375f,  0.35546875f, -0.46484375f,
                                 -0.33203125f,  0.328125f,   -0.234375f,   -0.49609375f,  0.24609375f, -0.02734375f, -0.20703125f,  0.33203125f, -0.140625f,   -0.3203125f,  -0.0078125f,  -0.12109375f,  0.25390625f, -0.41796875f,  0.15625f,    -0.08203125f,
                                  0.421875f,   -0.125f,       0.19921875f,  0.09765625f,  0.03125f,     0.47265625f, -0.40234375f,  0.23828125f,  0.4140625f,  -0.3828125f,   0.3046875f,  -0.4921875f,  -0.046875f,    0.484375f,   -0.15234375f, -0.27734375f,
                                  0.0546875f,  -0.40625f,     0.37109375f, -0.30078125f, -0.3671875f,  -0.11328125f,  0.16796875f,  0.078125f,   -0.05859375f, -0.1796875f,   0.140625f,    0.44140625f, -0.3046875f,   0.0859375f,   0.3125f,      0.18359375f,
                                  0.2734375f,  -0.01953125f, -0.1953125f,  -0.05078125f,  0.4375f,      0.296875f,   -0.24609375f, -0.4453125f,  -0.29296875f,  0.27734375f,  0.19140625f, -0.23828125f,  0.01171875f, -0.375f,      -0.2109375f,  -0.4765625f,
                                 -0.33984375f,  0.45703125f,  0.14453125f,  0.21875f,    -0.46875f,    -0.16015625f,  0.34765625f,  0.0f,         0.3984375f,   0.0390625f,  -0.4375f,     -0.1015625f,   0.3359375f,   0.2109375f,   0.40625f,    -0.07421875f,
                                  0.10546875f, -0.2578125f,  -0.4296875f,   0.39453125f,  0.05859375f, -0.32421875f,  0.12109375f, -0.0859375f,   0.4765625f,  -0.19921875f, -0.34375f,     0.3828125f,   0.1171875f,  -0.4140625f,  -0.16796875f,  0.00390625f,
                                  0.3515625f,  -0.13671875f,  0.2890625f,  -0.09375f,    -0.2265625f,   0.1796875f,   0.2578125f,  -0.359375f,   -0.484375f,    0.1015625f,   0.23046875f, -0.1484375f,  -0.0390625f,  -0.26953125f,  0.48046875f,  0.2421875f,
                                 -0.4609375f,   0.046875f,   -0.37890625f, -0.03125f,     0.49609375f, -0.3984375f,   0.36328125f, -0.1328125f,  -0.015625f,    0.31640625f, -0.296875f,    0.44921875f,  0.0625f,     -0.5f,         0.16015625f, -0.3125f,
                                  0.42578125f,  0.1953125f,   0.32421875f, -0.28515625f,  0.0234375f,  -0.18359375f,  0.07421875f, -0.265625f,    0.4296875f,   0.171875f,   -0.06640625f, -0.38671875f,  0.26171875f,  0.30859375f, -0.08984375f, -0.21875f };

const float void16x16_3[256] = {  0.48828125f,  0.37890625f,  0.015625f,   -0.3125f,      0.35546875f, -0.48828125f,  0.078125f,   -0.23046875f, -0.30859375f,  0.41015625f, -0.4375f,      0.0f,        -0.1328125f,   0.43359375f,  0.33984375f, -0.41015625f,
                                 -0.2734375f,  -0.15234375f, -0.0703125f,  -0.21484375f,  0.2109375f,  -0.3515625f,   0.30859375f,  0.14453125f, -0.39453125f, -0.1015625f,   0.1640625f,   0.36328125f, -0.29296875f,  0.2734375f,  -0.19140625f,  0.0859375f,
                                  0.2265625f,  -0.37109375f,  0.2890625f,  -0.43359375f,  0.4140625f,  -0.0078125f,  -0.16796875f,  0.44140625f,  0.03125f,     0.2578125f,   0.0703125f,  -0.375f,      -0.0625f,      0.12109375f, -0.328125f,   -0.015625f,
                                  0.33203125f,  0.05859375f,  0.45703125f,  0.16015625f,  0.09375f,    -0.11328125f, -0.27734375f, -0.45703125f,  0.3828125f,  -0.203125f,   -0.26171875f,  0.4765625f,  -0.48046875f,  0.20703125f,  0.39453125f, -0.44921875f,
                                  0.1328125f,  -0.12890625f, -0.25390625f, -0.046875f,   -0.38671875f,  0.23828125f,  0.3359375f,  -0.07421875f,  0.1953125f,  -0.33984375f, -0.0234375f,   0.3203125f,  -0.16015625f,  0.02734375f, -0.2265625f,  -0.08984375f,
                                 -0.3046875f,  -0.5f,         0.37109375f, -0.1875f,     -0.32421875f,  0.4921875f,   0.125f,       0.01171875f, -0.41796875f,  0.29296875f, -0.1171875f,   0.10546875f,  0.41796875f, -0.421875f,    0.27734375f,  0.46484375f,
                                  0.0078125f,   0.25390625f,  0.19140625f,  0.0390625f,   0.30078125f, -0.46875f,    -0.2109375f,  -0.30078125f,  0.4609375f,   0.15234375f, -0.4609375f,   0.234375f,   -0.2890625f,  -0.05078125f,  0.171875f,   -0.35546875f,
                                 -0.20703125f,  0.40625f,    -0.4140625f,  -0.09375f,     0.421875f,   -0.03125f,     0.08203125f, -0.14453125f,  0.3984375f,   0.046875f,   -0.2421875f,   0.3671875f,  -0.3671875f,  -0.18359375f,  0.07421875f,  0.34375f,
                                 -0.02734375f,  0.1015625f,  -0.15625f,    -0.28125f,     0.13671875f,  0.21875f,    -0.40625f,     0.26953125f, -0.359375f,   -0.17578125f, -0.08203125f, -0.00390625f,  0.203125f,    0.49609375f, -0.125f,      -0.3984375f,
                                  0.4453125f,   0.16796875f, -0.4765625f,   0.32421875f, -0.34765625f, -0.23828125f,  0.359375f,   -0.05859375f,  0.18359375f,  0.328125f,   -0.44140625f,  0.4296875f,   0.11328125f, -0.484375f,    0.3046875f,  -0.265625f,
                                 -0.33203125f,  0.265625f,   -0.0546875f,   0.48046875f,  0.01953125f, -0.12109375f,  0.4375f,     -0.49609375f, -0.26953125f,  0.08984375f,  0.25f,       -0.3203125f,  -0.234375f,    0.04296875f,  0.22265625f, -0.0859375f,
                                  0.375f,      -0.21875f,     0.0546875f,  -0.3828125f,   0.23046875f, -0.1796875f,   0.0625f,      0.15625f,    -0.01171875f,  0.47265625f, -0.390625f,   -0.13671875f, -0.03515625f,  0.390625f,   -0.4296875f,   0.12890625f,
                                 -0.1484375f,  -0.453125f,    0.19921875f, -0.28515625f,  0.40234375f, -0.4453125f,   0.28515625f, -0.31640625f, -0.19921875f, -0.09765625f,  0.28125f,     0.17578125f,  0.3515625f,  -0.171875f,   -0.296875f,    0.00390625f,
                                  0.42578125f,  0.09765625f,  0.31640625f, -0.10546875f, -0.01953125f,  0.1171875f,   0.34765625f, -0.36328125f,  0.38671875f,  0.0234375f,  -0.2578125f,  -0.42578125f,  0.06640625f,  0.46875f,    -0.37890625f,  0.296875f,
                                 -0.04296875f, -0.34375f,    -0.1953125f,   0.453125f,   -0.40234375f, -0.25f,       -0.078125f,    0.1875f,     -0.46484375f,  0.109375f,    0.44921875f, -0.3359375f,  -0.06640625f,  0.1484375f,   0.2421875f,  -0.24609375f,
                                  0.1796875f,  -0.47265625f,  0.140625f,    0.26171875f,  0.05078125f, -0.140625f,    0.484375f,    0.24609375f, -0.0390625f,  -0.1640625f,   0.3125f,      0.21484375f, -0.22265625f, -0.4921875f,   0.03515625f, -0.109375f };

ColourOkLabA SRGBToOkLab(ColourRGBA c)
{
    float l = SRGBtoLMS[0] * c.R + SRGBtoLMS[1] * c.G + SRGBtoLMS[2] * c.B;
    float m = SRGBtoLMS[3] * c.R + SRGBtoLMS[4] * c.G + SRGBtoLMS[5] * c.B;
    float s = SRGBtoLMS[6] * c.R + SRGBtoLMS[7] * c.G + SRGBtoLMS[8] * c.B;
    l = cbrtf(l); m = cbrtf(m); s = cbrtf(s);
    float L = CRLMStoOKLab[0] * l + CRLMStoOKLab[1] * m + CRLMStoOKLab[2] * s;
    float a = CRLMStoOKLab[3] * l + CRLMStoOKLab[4] * m + CRLMStoOKLab[5] * s;
    float b = CRLMStoOKLab[6] * l + CRLMStoOKLab[7] * m + CRLMStoOKLab[8] * s;
    L = (OkLabK3 * L - OkLabK1 + sqrtf((OkLabK3 * L - OkLabK1) * (OkLabK3 * L - OkLabK1) + 4.0f * OkLabK2 * OkLabK3 * L)) * 0.5f;
    ColourOkLabA outcol = { L, a, b, c.A };
    return outcol;
}

ColourRGBA OkLabToSRGB(ColourOkLabA c)
{
    c.L = (c.L * (c.L + OkLabK1))/(OkLabK3 * (c.L + OkLabK2));
    float l = OKLabtoCRLMS[0] * c.L + OKLabtoCRLMS[1] * c.a + OKLabtoCRLMS[2] * c.b;
    float m = OKLabtoCRLMS[3] * c.L + OKLabtoCRLMS[4] * c.a + OKLabtoCRLMS[5] * c.b;
    float s = OKLabtoCRLMS[6] * c.L + OKLabtoCRLMS[7] * c.a + OKLabtoCRLMS[8] * c.b;
    l = l*l*l; m = m*m*m; s = s*s*s;
    float R = LMStoSRGB[0] * l + LMStoSRGB[1] * m + LMStoSRGB[2] * s;
    float G = LMStoSRGB[3] * l + LMStoSRGB[4] * m + LMStoSRGB[5] * s;
    float B = LMStoSRGB[6] * l + LMStoSRGB[7] * m + LMStoSRGB[8] * s;
    ColourRGBA outcol = { R, G, B, c.A };
    return outcol;
}

ColourRGBA8 BlendSRGB8(ColourRGBA8 l, ColourRGBA8 r, float amt)
{
    ColourRGBA8 outcol;
    //Should be vectorised
    float lf[4] = { ((float)l.R)/255.0f, ((float)l.G)/255.0f, ((float)l.B)/255.0f, ((float)l.A)/255.0f };
    float rf[4] = { ((float)r.R)/255.0f, ((float)r.G)/255.0f, ((float)r.B)/255.0f, ((float)r.A)/255.0f };
    float outf[4];
    float namt = (1.0f - amt);
    for (int i = 0; i < 4; i++)
    {
        outf[i] = (namt * lf[i]) + (amt * rf[i]);
    }

    int outR = (int)(outf[0] * 255.0f + 0.5f);
    if (outR > 0xFF) outR = 0xFF;
    else if (outR < 0) outR = 0;
    int outG = (int)(outf[1] * 255.0f + 0.5f);
    if (outG > 0xFF) outG = 0xFF;
    else if (outG < 0) outG = 0;
    int outB = (int)(outf[2] * 255.0f + 0.5f);
    if (outB > 0xFF) outB = 0xFF;
    else if (outB < 0) outB = 0;
    int outA = (int)(outf[3] * 255.0f + 0.5f);
    if (outA > 0xFF) outA = 0xFF;
    else if (outA < 0) outA = 0;

    outcol.R = (unsigned char)outR;
    outcol.G = (unsigned char)outG;
    outcol.B = (unsigned char)outB;
    outcol.A = (unsigned char)outA;
    return outcol;
}

ColourOkLabA RoundTripLabA4bpc(ColourOkLabA c)
{
    ColourRGBA inRGBA = OkLabToSRGB(c);
    ColourRGBA8 inRGBA4 = LinearFloatToSRGB4(inRGBA);
    ColourRGBA outRGBA = SRGB8ToLinearFloat(inRGBA4);
    return SRGBToOkLab(outRGBA);
}

ColourOkLabA RoundTripLabA8bpc(ColourOkLabA c)
{
    ColourRGBA inRGBA = OkLabToSRGB(c);
    ColourRGBA8 inRGBA8 = LinearFloatToSRGB8(inRGBA);
    ColourRGBA outRGBA = SRGB8ToLinearFloat(inRGBA8);
    return SRGBToOkLab(outRGBA);
}

ImageHandler::ImageHandler()
{
    numColours = 16;
    numColourPlanes = 4;
    planeMask = 0x00F;
    is8BitColour = false;
    memset(palette, 0, sizeof(palette));

    ditherMethod = BAYER4X4;
    luminosityDither = 0.4;
    saturationDither = 0.1;
    hueDither = 0.8;
    luminosityDiffusion = 1.0;
    chromaDiffusion = 1.0;
    luminosityRandomisation = 0.0;
    chromaRandomisation = 0.0;
    chromaBias = 1.0;
    preBrightness = 0.0;
    preContrast = 0.0;
    postBrightness = 0.0;
    postContrast = 0.0;
    boustrophedon = false;
}

ImageHandler::~ImageHandler()
{

}

#define FORMAT_PNG           0
#define FORMAT_JPEG          1
#define FORMAT_UNRECOGNISED -1

int JpegSignatureCheck(unsigned char* inBuf)
{
    if (inBuf[0] != 0xFF) return 1;
    else if (inBuf[1] != 0xD8) return 1;
    else if (inBuf[2] != 0xFF) return 1;
    else return 0;
}

int ImageHandler::OpenImageFile(char* inFileName)
{
    FILE* file = fopen(inFileName, "rb");
    if (file == nullptr)
    {
        puts("Couldn't open file!");
        return 1;
    }

    int formatType = FORMAT_UNRECOGNISED;
    unsigned char magicCheck[8];
    fread(magicCheck, 1, 8, file);
    //Time to check exactly what kind of image file this is
    if (!png_sig_cmp(magicCheck, 0, 8))
    {
        formatType = FORMAT_PNG;
    }
    else if (!JpegSignatureCheck(magicCheck))
    {
        formatType = FORMAT_JPEG;
    }

    int w;
    int h;
    switch (formatType)
    {
        case FORMAT_PNG:
        {
            //PNG reading
            png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            if (pngPtr == nullptr)
            {
                return 1;
            }
            png_infop pngInfoPtr = png_create_info_struct(pngPtr);
            if (pngInfoPtr == nullptr)
            {
                png_destroy_read_struct(&pngPtr, nullptr, nullptr);
                return 1;
            }

            png_init_io(pngPtr, file);
            png_set_sig_bytes(pngPtr, 8);
            png_read_info(pngPtr, pngInfoPtr);
            w = png_get_image_width(pngPtr, pngInfoPtr);
            h = png_get_image_height(pngPtr, pngInfoPtr);
            unsigned char bitdepth = png_get_bit_depth(pngPtr, pngInfoPtr);
            unsigned char colourtype = png_get_color_type(pngPtr, pngInfoPtr);
            unsigned char channels = png_get_channels(pngPtr, pngInfoPtr);
            srcImage.width = w;
            srcImage.height = h;
            //Transform to RGBA
            if (colourtype == PNG_COLOR_TYPE_PALETTE)
            {
                png_set_palette_to_rgb(pngPtr);
            }
            if (colourtype == PNG_COLOR_TYPE_GRAY && bitdepth < 8)
            {
                png_set_expand_gray_1_2_4_to_8(pngPtr);
            }
            if (png_get_valid(pngPtr, pngInfoPtr, PNG_INFO_tRNS))
            {
                png_set_tRNS_to_alpha(pngPtr);
            }
            if (bitdepth == 16)
            {
                png_set_strip_16(pngPtr);
            }
            if (colourtype == PNG_COLOR_TYPE_RGB || colourtype == PNG_COLOR_TYPE_GRAY || colourtype == PNG_COLOR_TYPE_PALETTE)
            {
                png_set_add_alpha(pngPtr, 0xFF, PNG_FILLER_AFTER);
            }
            if (colourtype == PNG_COLOR_TYPE_GRAY || colourtype == PNG_COLOR_TYPE_GRAY_ALPHA)
            {
                png_set_gray_to_rgb(pngPtr);
            }
            double gamma;
            if (png_get_gAMA(pngPtr, pngInfoPtr, &gamma))
            {
                png_set_gamma(pngPtr, 2.2, gamma);
            }
            else
            {
                png_set_gamma(pngPtr, 2.2, 0.45455);
            }
            png_read_update_info(pngPtr, pngInfoPtr);
            //Allocate and read in PNG
            srcImage.data = new ColourRGBA8[w * h];
            unsigned char** rowPtrs = new unsigned char*[h];
            for (int i = 0; i < h; i++)
            {
                rowPtrs[i] = (unsigned char*)(srcImage.data + i * w);
            }
            png_read_image(pngPtr, rowPtrs);

            //We're done
            png_read_end(pngPtr, pngInfoPtr);
            png_destroy_read_struct(&pngPtr, &pngInfoPtr, nullptr);

            fclose(file);
            delete[] rowPtrs;
        }
            break;
        case FORMAT_JPEG:
        {
            //JPEG reading
            fseek(file, 0, SEEK_SET);
            struct jpeg_decompress_struct dinfo;
            struct jpeg_error_mgr jerr;
            dinfo.err = jpeg_std_error(&jerr);
            jpeg_create_decompress(&dinfo);
            jpeg_stdio_src(&dinfo, file);
            jpeg_read_header(&dinfo, TRUE);
            jpeg_start_decompress(&dinfo);

            w = dinfo.output_width;
            h = dinfo.output_height;
            srcImage.width = w;
            srcImage.height = h;
            srcImage.data = new ColourRGBA8[w * h];
            JSAMPROW rowBuf = new JSAMPLE[3 * w];
            while (dinfo.output_scanline < h)
            {
                ColourRGBA8* rowPtr = srcImage.data + w * dinfo.output_scanline;
                jpeg_read_scanlines(&dinfo, &rowBuf, 1);
                for (int i = 0; i < w; i++)
                {
                    ColourRGBA8 outcol;
                    outcol.R = rowBuf[3 * i];
                    outcol.G = rowBuf[3 * i + 1];
                    outcol.B = rowBuf[3 * i + 2];
                    outcol.A = 0xFF;
                    rowPtr[i] = outcol;
                }
            }

            jpeg_finish_decompress(&dinfo);
            jpeg_destroy_decompress(&dinfo);
            fclose(file);
        }
            break;
        default:
            puts("Unsupported file format!");
            fclose(file);
            return 2;
    }

    encImage.width = w;
    encImage.height = h;
    encImage.data = new ColourRGBA8[w * h];
    memcpy(encImage.data, srcImage.data, w * h * sizeof(ColourRGBA8));

    numColours = 16;
    numColourPlanes = 4;
    planeMask = 0x00F;
    is8BitColour = false;
    transparencyThreshold = 0x80;
    memcpy(palette, defaultPalette, sizeof(defaultPalette));
    GetLabPaletteFromRGBA8Palette();

    return 0;
}

void ImageHandler::CloseImageFile()
{
    delete[] srcImage.data;
    delete[] encImage.data;
}

bool ImageHandler::IsPalettePerfect()
{
    int w = srcImage.width;
    int h = srcImage.height;
    ColourRGBA8* pixels = srcImage.data;
    long long numPixels = w * h;
    int tThres = transparencyThreshold;
    if (tThres < 0) tThres = 0;
    else if (tThres > 0xFF) tThres = 0xFF;
    unsigned int transT = (unsigned int)tThres;
    transT <<= 24;

    //pointer reinterpretations
    unsigned int* clistui = (unsigned int*)palette;
    unsigned int* pixui = (unsigned int*)pixels;
    for (int i = 0; i < numPixels; i++)
    {
        unsigned int inCol = pixui[i];
        bool foundCol = false;
        if (inCol < transT) continue; //Ignore 'transparent' colours
        inCol |= 0xFF000000; //Force full opacity for comparison
        for (int j = 0; j < numColours; j++)
        {
            if (clistui[j] == inCol)
            {
                foundCol = true;
                break;
            }
        }
        if (!foundCol)
        {
            return false;
        }
    }
    return true;
}


typedef struct
{
    ColourOkLabA mean;
    ColourOkLabA lastmean;
    long long numInCluster;
    double sumL;
    double suma;
    double sumb;
} KMean;

bool ImageHandler::GetBestPalette(float uvbias, float bright, float contrast)
{
    int w = srcImage.width;
    int h = srcImage.height;
    ColourRGBA8* pixels = srcImage.data;
    long long numPixels = w * h;
    int tThres = transparencyThreshold;
    if (tThres < 0) tThres = 0;
    else if (tThres > 0xFF) tThres = 0xFF;
    unsigned int transT = (unsigned int)tThres;
    transT <<= 24;
    float ftThres = ((float)tThres)/255.0f;

    //Determine if there are too many unique colours
    ColourRGBA8 colourList[256];
    int totalColours = 0;
    bool tooManyColours = false;
    //pointer reinterpretations
    unsigned int* clistui = (unsigned int*)colourList;
    unsigned int* pixui = (unsigned int*)pixels;
    for (int i = 0; i < numPixels; i++)
    {
        unsigned int inCol = pixui[i];
        bool newCol = true;
        if (inCol < transT) continue; //Ignore 'transparent' colours
        inCol |= 0xFF000000; //Force full opacity for comparison
        for (int j = 0; j < totalColours; j++)
        {
            if (inCol == clistui[j])
            {
                newCol = false;
                break;
            }
        }
        if (newCol)
        {
            totalColours++;
            if (totalColours > numColours)
            {
                tooManyColours = true;
                break;
            }
            else
            {
                clistui[totalColours-1] = inCol;
            }
        }
    }
    if (!tooManyColours)
    {
        numColours = totalColours;
        memcpy(palette, colourList, numColours * sizeof(ColourRGBA8));
        GetLabPaletteFromRGBA8Palette();
        return true;
    }

    //Do the k-means algorithm if there are more unique colours in the image than the number of colours in the palette
    ColourOkLabA* colours = new ColourOkLabA[numPixels];
    KMean means[256];
    //Initialise means
    for (int i = 0; i < numColours; i++)
    {
        KMean* m = &means[i];
        m->numInCluster = 0;
        m->sumL = 0.0; m->suma = 0.0; m->sumb = 0.0;
    }
    //Initialise colour array
    for (long long i = 0; i < numPixels; i++)
    {
        //Without this clamping the k-means clustering would never converge
        ColourRGBA8 pixcol = pixels[i];
        float alpha = ((float)pixcol.A)/255.0f; //Avoid loss of precision in alpha value
        ColourRGBA incol = OkLabToSRGB(ColourAdjust(SRGBToOkLab(SRGB8ToLinearFloat(pixcol)), bright, contrast));
        if (incol.R > 1.0f) incol.R = 1.0f; else if (incol.R < 0.0f) incol.R = 0.0f;
        if (incol.G > 1.0f) incol.G = 1.0f; else if (incol.G < 0.0f) incol.G = 0.0f;
        if (incol.B > 1.0f) incol.B = 1.0f; else if (incol.B < 0.0f) incol.B = 0.0f;
        ColourOkLabA outcol = SRGBToOkLab(incol);
        outcol.A = alpha;
        colours[i] = outcol;
    }
    //Pick some means (k-means||)
    ColourOkLabA* csamples = new ColourOkLabA[17 * numColours];
    double* probs = new double[numPixels];
    unsigned long long rnum = RNGUpdate() % numPixels;
    csamples[0] = colours[rnum];
    int totalSamples = 1;
    int sampPerIter = 2 * numColours;
    //Pick some initial points
    for (int i = 0; i < 8; i++)
    {
        //Cost calculation
        double cost = 0.0;
        #pragma omp parallel for
        for (long long j = 0; j < numPixels; j++)
        {
            ColourOkLabA col = colours[j];
            if (col.A < ftThres) //Ignore 'transparent' colours
            {
                probs[j] = 0.0;
                continue;
            }
            float lowestDistance = 999999999999999999999999.9;
            for (int k = 0; k < totalSamples; k++)
            {
                const ColourOkLabA incol = csamples[k];
                const float dL = (col.L - incol.L) * uvbias;
                const float da = col.a - incol.a;
                const float db = col.b - incol.b;
                const float dist = (dL * dL) + (da * da) + (db * db);
                if (dist < lowestDistance)
                {
                    lowestDistance = dist;
                }
            }
            #pragma omp atomic update
            cost += (double)lowestDistance;
            probs[j] = (double)lowestDistance;
        }
        double probmod = ((double)sampPerIter)/cost;
        double cumProb = 0.0; //hee hee
        for (long long j = 0; j < numPixels; j++)
        {
            cumProb += probmod * probs[j];
            probs[j] = cumProb;
        }
        //Pick samples
        #pragma omp parallel for
        for (int j = 0; j < sampPerIter; j++)
        {
            double p = ((RNGUpdateDouble() * 0.5) + 0.5) * cumProb;
            long long ind = numPixels/2;
            long long lBound = 0;
            long long uBound = numPixels - 1;
            while (ind != lBound || ind != uBound)
            {
                if (p < probs[ind])
                {
                    uBound = ind;
                }
                else
                {
                    lBound = ind + 1;
                }
                ind = lBound + ((uBound - lBound)/2);
            }
            csamples[totalSamples] = colours[ind];
            totalSamples++;
        }
    }
    //Weight the points
    long long* weights = (long long*)calloc(totalSamples, sizeof(long long));
    for (long long i = 0; i < numPixels; i++)
    {
        ColourOkLabA col = colours[i];
        if (col.A < ftThres) continue; //Ignore 'transparent' colours
        float lowestDistance = 999999999999999999999999.9;
        int chosenColour = 0;
        for (int j = 0; j < totalSamples; j++)
        {
            const ColourOkLabA incol = csamples[j];
            const float dL = (col.L - incol.L) * uvbias;
            const float da = col.a - incol.a;
            const float db = col.b - incol.b;
            const float dist = (dL * dL) + (da * da) + (db * db);
            if (dist < lowestDistance)
            {
                lowestDistance = dist;
                chosenColour = j;
            }
        }
        weights[chosenColour]++;
    }
    //Recluster according to k-means++
    KMean* initM = &means[0];
    rnum = RNGUpdate() % totalSamples;
    initM->mean = csamples[rnum];
    for (int i = 1; i < numColours; i++)
    {
        //Cost calculation
        double cost = 0.0;
        for (int j = 0; j < totalSamples; j++)
        {
            ColourOkLabA col = csamples[j];
            if (col.A < ftThres) //Ignore 'transparent' colours
            {
                probs[j] = 0.0;
                continue;
            }
            float lowestDistance = 999999999999999999999999.9;
            for (int k = 0; k < i; k++)
            {
                const ColourOkLabA incol = means[k].mean;
                const float dL = (col.L - incol.L) * uvbias;
                const float da = col.a - incol.a;
                const float db = col.b - incol.b;
                const float dist = (dL * dL) + (da * da) + (db * db);
                if (dist < lowestDistance)
                {
                    lowestDistance = dist;
                }
            }
            cost += (double)lowestDistance;
            probs[j] = (double)lowestDistance;
        }
        double probmod = ((double)sampPerIter)/cost;
        double cumProb = 0.0; //hee hee
        for (long long j = 0; j < totalSamples; j++)
        {
            cumProb += probmod * probs[j] * ((double)weights[j]);
            probs[j] = cumProb;
        }
        double p = ((RNGUpdateDouble() * 0.5) + 0.5) * cumProb;
        int ind = totalSamples/2;
        int lBound = 0;
        int uBound = totalSamples - 1;
        while (ind != lBound || ind != uBound)
        {
            if (p < probs[ind])
            {
                uBound = ind;
            }
            else
            {
                lBound = ind + 1;
            }
            ind = lBound + ((uBound - lBound)/2);
        }
        means[i].mean = csamples[ind];
    }

    delete[] probs;
    free(weights);
    delete[] csamples;

    //Iterate the means
    int iterationsLeft = 100 + numColours;
    while (iterationsLeft > 0)
    {
        //Zero out sums and counts
        for (int i = 0; i < numColours; i++)
        {
            KMean* m = &means[i];
            m->lastmean = m->mean;
            m->numInCluster = 0;
            m->sumL = 0.0; m->suma = 0.0; m->sumb = 0.0;
        }

        //Associate each colour with the closest mean
        #pragma omp parallel for
        for (long long i = 0; i < numPixels; i++)
        {
            ColourOkLabA col = colours[i];
            if (col.A < ftThres) continue; //Ignore 'transparent' colours
            float lowestDistance = 999999999999999999999999.9;
            int chosenColour = 0;
            for (int j = 0; j < numColours; j++)
            {
                const ColourOkLabA incol = means[j].mean;
                const float dL = (col.L - incol.L) * uvbias;
                const float da = col.a - incol.a;
                const float db = col.b - incol.b;
                const float dist = (dL * dL) + (da * da) + (db * db);
                if (dist < lowestDistance)
                {
                    lowestDistance = dist;
                    chosenColour = j;
                }
            }
            KMean* m = &means[chosenColour];
            #pragma omp atomic update
            m->sumL += col.L;
            #pragma omp atomic update
            m->suma += col.a;
            #pragma omp atomic update
            m->sumb += col.b;
            #pragma omp atomic update
            m->numInCluster++;
        }

        //Calculate means
        double meandiff = 0.0;
        for (int i = 0; i < numColours; i++)
        {
            KMean* m = &means[i];
            ColourOkLabA meancol;
            if (m->numInCluster <= 0) //Fallback because of suspected division by zero errors;
            {
                meancol.L = (RNGUpdateFloat() * 0.5f) + 0.5f;
                meancol.a = RNGUpdateFloat() * 0.5f;
                meancol.b = RNGUpdateFloat() * 0.5f;
            }
            else
            {
                meancol.L = m->sumL/((double)m->numInCluster);
                meancol.a = m->suma/((double)m->numInCluster);
                meancol.b = m->sumb/((double)m->numInCluster);
            }
            meancol.A = 1.0f;
            if (is8BitColour) meancol = RoundTripLabA8bpc(meancol);
            else meancol = RoundTripLabA4bpc(meancol);
            m->mean = meancol;
            const double dL = (((double)m->mean.L) - ((double)m->lastmean.L)) * ((double)uvbias);
            const double da = ((double)m->mean.a) - ((double)m->lastmean.a);
            const double db = ((double)m->mean.b) - ((double)m->lastmean.b);
            const double dist = (dL * dL) + (da * da) + (db * db);
            meandiff += sqrt(dist);
        }
        if (meandiff == 0.0) iterationsLeft = 0; //Break out if convergence has been reached
        iterationsLeft--;
    }

    //Confirm colours
    if (is8BitColour)
    {
        for (int i = 0; i < numColours; i++)
        {
            ColourOkLabA incol = means[i].mean;
            ColourRGBA midcol = OkLabToSRGB(incol);
            palette[i] = LinearFloatToSRGB8(midcol);
        }
    }
    else
    {
        for (int i = 0; i < numColours; i++)
        {
            ColourOkLabA incol = means[i].mean;
            ColourRGBA midcol = OkLabToSRGB(incol);
            palette[i] = LinearFloatToSRGB4(midcol);
        }
    }

    GetLabPaletteFromRGBA8Palette();

    delete[] colours;

    return false;
}

void ImageHandler::ShufflePaletteBasedOnOccurrence()
{
    int w = encImage.width;
    int h = encImage.height;
    long long imgsize = ((long long)w) * ((long long)h);
    uint32_t* pixels = (uint32_t*)encImage.data;
    uint32_t* pal = (uint32_t*)palette;
    int occurrence[256];
    memset(occurrence, 0, sizeof(occurrence));
    for (long long i = 0; i < imgsize; i++)
    {
        uint32_t pix = pixels[i];
        for (int j = 0; j < numColours; j++)
        {
            if (pix == pal[j])
            {
                occurrence[j]++;
            }
        }
    }
    uint32_t tempPal[256];
    memcpy(tempPal, pal, sizeof(tempPal));
    for (int i = 0; i < numColours-1; i++)
    {
        int highestOcc = occurrence[i];
        int chosenColour = i;
        for (int j = i + 1; j < numColours; j++)
        {
            int occ = occurrence[j];
            if (occ > highestOcc)
            {
                highestOcc = occ;
                chosenColour = j;
            }
        }
        uint32_t currentColour = tempPal[i];
        uint32_t swapColour = tempPal[chosenColour];
        tempPal[chosenColour] = currentColour;
        tempPal[i] = swapColour;
    }
    memcpy(pal, tempPal, sizeof(tempPal));
    GetLabPaletteFromRGBA8Palette();
}

void ImageHandler::DitherImage()
{
    DitherImage(ditherMethod, luminosityDither, saturationDither, hueDither, luminosityDiffusion, chromaDiffusion, luminosityRandomisation, chromaRandomisation, chromaBias, preBrightness, preContrast, postBrightness, postContrast, boustrophedon);
}

void ImageHandler::DitherImage(int ditherMethod, double ditAmtL, double ditAmtS, double ditAmtH, double ditAmtEL, double ditAmtEC, double rngAmtL, double rngAmtC, double cbias, double preB, double preC, double postB, double postC, bool globBoustro)
{
    ColourRGBA8 (ImageHandler::*odfunc)(ColourOkLabA, int, int, float, float, float, float, float, float);
    ColourRGBA8 (ImageHandler::*eddfunc)(ColourOkLabA, int, int, int, float, float, float, float, float, ColourOkLabA*, int, float, float);
    int eddMarginX, eddMarginY;
    int w = srcImage.width;
    int h = srcImage.height;
    ColourRGBA8* pixels = srcImage.data;
    ColourRGBA8* outpix = encImage.data;
    int tThres = transparencyThreshold;
    if (tThres < 0) tThres = 0;
    else if (tThres > 0xFF) tThres = 0xFF;
    unsigned int transT = (unsigned int)tThres;
    ColourRGBA8 zeroCol;
    if ((planeMask & 0x100) == 0x100) //Yes mask plane -> fill 'transparent' colours with #00000000
    {
        zeroCol.R = 0; zeroCol.G = 0; zeroCol.B = 0; zeroCol.A = 0;
    }
    else zeroCol = palette[0]; //No mask plane -> fill 'transparent' colours with colour 0

    //Select function and set parameters
    switch (ditherMethod)
    {
        case NODITHER:
            for (long long i = 0; i < h; i++)
            {
                for (long long j = 0; j < w; j++)
                {
                    const long long index = i * w + j;
                    ColourRGBA8 pixcol = pixels[index];
                    if (pixcol.A < transT)
                    {
                        outpix[index] = zeroCol;
                        continue;
                    }
                    ColourOkLabA incol = SRGBToOkLab(SRGB8ToLinearFloat(pixcol));
                    incol = ColourAdjust(incol, preB, preC);
                    outpix[index] = GetClosestColourOkLab(incol, postB, postC, cbias);
                }
            }
            return;
        case BAYER2X2:
            odfunc = &ImageHandler::OrderedDitherBayer2x2; break;
        case BAYER4X4:
            odfunc = &ImageHandler::OrderedDitherBayer4x4; break;
        case BAYER8X8:
            odfunc = &ImageHandler::OrderedDitherBayer8x8; break;
        case BAYER16X16:
            odfunc = &ImageHandler::OrderedDitherBayer16x16; break;
        case VOID16X16:
            odfunc = &ImageHandler::OrderedDitherVoid16x16; break;
        case FLOYD_STEINBERG:
            eddfunc = &ImageHandler::DitherFloydSteinberg;
            eddMarginX = 1; eddMarginY = 1;
            break;
        case FLOYD_FALSE:
            eddfunc = &ImageHandler::DitherFloydFalse;
            eddMarginX = 1; eddMarginY = 1;
            break;
        case JJN:
            eddfunc = &ImageHandler::DitherJJN;
            eddMarginX = 2; eddMarginY = 2;
            break;
        case STUCKI:
            eddfunc = &ImageHandler::DitherStucki;
            eddMarginX = 2; eddMarginY = 2;
            break;
        case BURKES:
            eddfunc = &ImageHandler::DitherBurkes;
            eddMarginX = 2; eddMarginY = 1;
            break;
        case SIERRA:
            eddfunc = &ImageHandler::DitherSierra;
            eddMarginX = 2; eddMarginY = 2;
            break;
        case SIERRA2ROW:
            eddfunc = &ImageHandler::DitherSierra2Row;
            eddMarginX = 2; eddMarginY = 1;
            break;
        case FILTERLITE:
            eddfunc = &ImageHandler::DitherFilterLite;
            eddMarginX = 1; eddMarginY = 1;
            break;
        case ATKINSON:
            eddfunc = &ImageHandler::DitherAtkinson;
            eddMarginX = 2; eddMarginY = 2;
            break;
    }

    //Carry out operation
    if (ditherMethod < FLOYD_STEINBERG) //Ordered dithering
    {
        #pragma omp parallel for
        for (long long i = 0; i < h; i++)
        {
            for (long long j = 0; j < w; j++)
            {
                const long long index = i * w + j;
                ColourRGBA8 pixcol = pixels[index];
                if (pixcol.A < transT)
                {
                    outpix[index] = zeroCol;
                    continue;
                }
                ColourOkLabA incol = SRGBToOkLab(SRGB8ToLinearFloat(pixcol));
                incol = ColourAdjust(incol, preB, preC);
                outpix[index] = (this->*odfunc)(incol, j, i, ditAmtL, ditAmtS, ditAmtH, postB, postC, cbias);
            }
        }
    }
    else //Error diffusion
    {
        int ew = w + 2*EDD_EXPAND_X;
        int eh = h + EDD_EXPAND_Y_TOP + EDD_EXPAND_Y_BOTTOM; //Expand image to ease in error diffusion
        ColourRGBA8* expandedInput = new ColourRGBA8[ew * eh];
        ColourOkLabA* diffusedError = (ColourOkLabA*)calloc(ew * eh, sizeof(ColourOkLabA));

        long long firstLine;
        long long lastLine;
        for (long long i = 0; i < h; i++) //Find first line with non-transparent colours
        {
            bool foundLine = false;
            for (long long j = 0; j < w; j++)
            {
                const long long index = i * w + j;
                if (pixels[index].A < transT)
                {
                    continue;
                }
                else
                {
                    foundLine = true;
                    break;
                }
            }
            if (foundLine)
            {
                firstLine = i;
                break;
            }
        }
        lastLine = firstLine;
        for (long long i = firstLine + 1; i < h; i++) //Find last line with non-transparent colours
        {
            bool foundLine = false;
            for (long long j = 0; j < w; j++)
            {
                const long long index = i * w + j;
                if (pixels[index].A < transT)
                {
                    continue;
                }
                else
                {
                    foundLine = true;
                    break;
                }
            }
            if (foundLine)
            {
                lastLine = i;
            }
        }

        long long topBlendLine = firstLine;
        long long bottomBlendLine = firstLine;
        for (long long i = firstLine; i <= lastLine; i++) //Iterate clamping algorithm over all lines with non-transparent colours
        {
            int lineState = 0;
            long long outLine = i + EDD_EXPAND_Y_TOP;
            long long leftPix = 0;
            for (long long j = 0; j < w; j++)
            {
                const long long index = i * w + j;
                ColourRGBA8 pixcol = pixels[index];
                if (pixcol.A < transT)
                {
                    switch (lineState)
                    {
                        case 0: //In left edge, searching for right pixel
                            break;
                        case 1: //In non-transparent, searching for left pixel
                            leftPix = j - 1;
                            lineState = 2;
                            break;
                        case 2: //In transparent, searching for right pixel
                            break;
                    }
                }
                else
                {
                    switch (lineState)
                    {
                        case 0: //In left edge, searching for right pixel
                            pixcol.A = 0x00; //Force full transparency
                            for (long long k = 0; k < j + EDD_EXPAND_X; k++) //Clamp to left edge
                            {
                                expandedInput[outLine * ew + k] = pixcol;
                            }
                            lineState = 1;
                        case 1: //In non-transparent, searching for left pixel
                            pixcol.A = 0xFF; //Force full opacity
                            expandedInput[outLine * ew + j + EDD_EXPAND_X] = pixcol;
                            break;
                        case 2: //In transparent, searching for right pixel
                        {
                            ColourRGBA8 leftCol = pixels[i * w + leftPix];
                            pixcol.A = 0x00; //Force full transparency
                            leftCol.A = 0x00; //Force full transparency
                            for (long long k = leftPix + 1; k < j; k++) //Blend between sides
                            {
                                expandedInput[outLine * ew + k + EDD_EXPAND_X] = BlendSRGB8(leftCol, pixcol, (((float)k) - ((float)leftPix))/(((float)j) - ((float)leftPix)));
                            }
                        }
                            pixcol.A = 0xFF; //Force full opacity
                            expandedInput[outLine * ew + j + EDD_EXPAND_X] = pixcol;
                            lineState = 1;
                            break;
                    }
                }
            }
            switch (lineState)
            {
                case 0: //In left edge, searching for right pixel -> all transparent line
                    continue;
                case 1: //In non-transparent, searching for left pixel -> clamp to right edge
                    leftPix = w - 1;
                case 2: //In transparent, searching for right pixel -> clamp to right edge
                {
                    ColourRGBA8 leftCol = pixels[i * w + leftPix];
                    leftCol.A = 0x00; //Force full transparency
                    for (long long k = leftPix + 1; k < w; k++)
                    {
                        expandedInput[outLine * ew + k + EDD_EXPAND_X] = leftCol;
                    }
                }
                    bottomBlendLine = i;
                    break;
            }
            if (bottomBlendLine - 1 > topBlendLine) //Blend between non-transparent lines
            {
                ColourRGBA8* tPtr = &expandedInput[(topBlendLine + EDD_EXPAND_Y_TOP) * ew];
                ColourRGBA8* bPtr = &expandedInput[(bottomBlendLine + EDD_EXPAND_Y_TOP) * ew];
                for (long long j = topBlendLine + 1; j < bottomBlendLine; j++)
                {
                    float blendFac = (((float)j) - ((float)topBlendLine))/(((float)bottomBlendLine) - ((float)topBlendLine));
                    for (long long k = 0; k < ew; k++)
                    {
                        ColourRGBA8 tCol = tPtr[k];
                        ColourRGBA8 bCol = bPtr[k];
                        tCol.A = 0x00; //Force full transparency
                        bCol.A = 0x00; //Force full transparency
                        expandedInput[(j + EDD_EXPAND_Y_TOP) * ew + k] = BlendSRGB8(tCol, bCol, blendFac);
                    }
                }
            }
            topBlendLine = i;
        }

        //Vertical clamping
        ColourRGBA8* rowptr = &expandedInput[(firstLine + EDD_EXPAND_Y_TOP) * ew];
        for (long long i = 0; i < firstLine + EDD_EXPAND_Y_TOP; i++)
        {
            for (long long j = 0; j < ew; j++)
            {
                ColourRGBA8 rCol = rowptr[j];
                rCol.A = 0x00; //Force full transparency
                expandedInput[i * ew + j] = rCol;
            }
        }
        rowptr = &expandedInput[(lastLine + EDD_EXPAND_Y_TOP) * ew];
        for (long long i = lastLine + EDD_EXPAND_Y_TOP; i < eh; i++)
        {
            for (long long j = 0; j < ew; j++)
            {
                ColourRGBA8 rCol = rowptr[j];
                rCol.A = 0x00; //Force full transparency
                expandedInput[i * ew + j] = rCol;
            }
        }

        for (long long i = 0; i < eh-eddMarginY; i++)
        {
            const int boustro = i % 2;
            if (globBoustro && boustro)
            {
                for (long long j = ew-1-eddMarginX; j >= eddMarginX; j--)
                {
                    const long long index = i * ew + j;
                    ColourRGBA8 pixcol = expandedInput[index];
                    ColourOkLabA incol = SRGBToOkLab(SRGB8ToLinearFloat(pixcol));
                    incol = ColourAdjust(incol, preB, preC);
                    ColourRGBA8 outcol = (this->*eddfunc)(incol, j, i, ew, ditAmtEL, ditAmtEC, postB, postC, cbias, diffusedError, -1, rngAmtL, rngAmtC);
                    if (pixcol.A < 0.5f)
                    {
                        expandedInput[index] = zeroCol;
                        continue;
                    }
                    else expandedInput[index] = outcol;
                }
            }
            else
            {
                for (long long j = eddMarginX; j < ew-eddMarginX; j++)
                {
                    const long long index = i * ew + j;
                    ColourRGBA8 pixcol = expandedInput[index];
                    ColourOkLabA incol = SRGBToOkLab(SRGB8ToLinearFloat(pixcol));
                    incol = ColourAdjust(incol, preB, preC);
                    ColourRGBA8 outcol = (this->*eddfunc)(incol, j, i, ew, ditAmtEL, ditAmtEC, postB, postC, cbias, diffusedError, 1, rngAmtL, rngAmtC);
                    if (pixcol.A < 0.5f)
                    {
                        expandedInput[index] = zeroCol;
                        continue;
                    }
                    else expandedInput[index] = outcol;
                }
            }
        }
        //Copy back
        for (int i = 0; i < h; i++)
        {
            memcpy(&outpix[i * w], &expandedInput[(i + EDD_EXPAND_Y_TOP) * ew + EDD_EXPAND_X], w * sizeof(ColourRGBA8));
        }
        free(diffusedError);
        delete[] expandedInput;
    }
}

PlanarInfo ImageHandler::GeneratePlanarData()
{
    PlanarInfo outinf;

    if (numColours > 256)
    {
        puts("Too many colours!!");
        outinf.planeData = nullptr;
        return outinf;
    }

    bool transparency = (planeMask & 0x0100) > 0;
    outinf.is8BitColour = is8BitColour;
    outinf.numColours = 1 << numColourPlanes;
    outinf.planeMask = planeMask;
    outinf.numPlanes = numColourPlanes;
    if (transparency) outinf.numPlanes++;

    //Convert to indexed colour representation
    int w = encImage.width;
    int h = encImage.height;
    long long imgsize = ((long long)w) * ((long long)h);
    int pwidth = (w + 0x7)/0x8;
    int psize = pwidth * h;
    outinf.planew = pwidth;
    outinf.planeh = h;
    outinf.planeSize = psize;
    short* indices = new short[imgsize];
    uint32_t* img = (uint32_t*)encImage.data;
    uint32_t* pal = (uint32_t*)palette;
    int nc = outinf.numColours;
    for (int i = 0; i < imgsize; i++)
    {
        uint32_t pix = img[i];
        bool foundcol = false;
        for (int j = 0; j < nc; j++)
        {
            if (pix == pal[j])
            {
                indices[i] = j;
                foundcol = true;
                break;
            }
        }
        if (!foundcol)
        {
            if (transparency) indices[i] = -1;
            else indices[i] = 0;
        }
    }
    unsigned char** pData = new unsigned char*[outinf.numPlanes];
    outinf.planeData = pData;
    int splane = 0;
    for (int i = 0; i < outinf.numPlanes; i++)
    {
        pData[i] = (unsigned char*)calloc(psize, 1);
    }
    if (transparency) //Generate mask plane
    {
        splane++;
        unsigned char* curPlane = pData[0];
        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                short ind = indices[i * w + j];
                if (ind >= 0) curPlane[i * pwidth + (j >> 3)] |= (0x01 << (7 - (j & 0x7)));
            }
        }
    }

    for (int i = splane; i < outinf.numPlanes; i++)
    {
        unsigned char* curPlane = pData[i];
        short curMask = 0x0001 << (i - splane);
        for (int j = 0; j < h; j++)
        {
            for (int k = 0; k < w; k++)
            {
                short ind = indices[j * w + k];
                if ((ind >= 0) && (ind & curMask))
                {
                    curPlane[j * pwidth + (k >> 3)] |= (0x01 << (7 - (k & 0x7)));
                }
            }
        }
    }

    return outinf;
}

void ImageHandler::FreePlanarData(PlanarInfo* pinfo)
{
    for (int i = 0; i < pinfo->numPlanes; i++)
    {
        free(pinfo->planeData[i]);
    }
    delete[] pinfo->planeData;
}


void ImageHandler::GetLabPaletteFromRGBA8Palette()
{
    minL = 1.0f; maxL = 0.0f;
    maxC = 0.0f;
    for (int i = 0; i < numColours; i++)
    {
        ColourOkLabA labcol = SRGBToOkLab(SRGB8ToLinearFloat(palette[i]));
        labPalette[i] = labcol;
        if (labcol.L < minL) minL = labcol.L; if (labcol.L > maxL) maxL = labcol.L;
        float sat = hypotf(labcol.a, labcol.b);
        if (sat > maxC) maxC = sat;
    }
}

ColourRGBA8 ImageHandler::GetClosestColourOkLab(ColourOkLabA col, float bright, float contrast, float uvbias)
{
    float lowestDistance = 999999999999999999999999.9;
    int chosenColour = 0;
    col = ColourAdjust(col, bright, contrast);
    for (int i = 0; i < numColours; i++)
    {
        const ColourOkLabA incol = labPalette[i];
        const float dL = (col.L - incol.L) * uvbias;
        const float da = col.a - incol.a;
        const float db = col.b - incol.b;
        const float dist = (dL * dL) + (da * da) + (db * db);
        if (dist < lowestDistance)
        {
            lowestDistance = dist;
            chosenColour = i;
        }
    }
    return palette[chosenColour];
}

ColourOkLabA ImageHandler::GetClosestColourOkLabWithError(ColourOkLabA col, ColourOkLabA* error, float bright, float contrast, float uvbias, float rngAmtL, float rngAmtC)
{
    float lowestDistance = 999999999999999999999999.9;
    int chosenColour = 0;
    ColourOkLabA postcol = ColourAdjust(col, bright, contrast);
    for (int i = 0; i < numColours; i++)
    {
        const ColourOkLabA incol = labPalette[i];
        const float dL = (postcol.L - incol.L) * uvbias;
        const float da = postcol.a - incol.a;
        const float db = postcol.b - incol.b;
        const float dist = (dL * dL) + (da * da) + (db * db);
        if (dist < lowestDistance)
        {
            lowestDistance = dist;
            chosenColour = i;
        }
    }
    ColourOkLabA outcol = labPalette[chosenColour];
    error->L = col.L - outcol.L + RNGUpdateFloat()*rngAmtL;
    error->a = col.a - outcol.a + RNGUpdateFloat()*rngAmtC;
    error->b = col.b - outcol.b + RNGUpdateFloat()*rngAmtC;
    error->A = 0.0f;
    return outcol;
}

ColourOkLabA ImageHandler::ClampColourOkLab(ColourOkLabA col)
{
    if (col.L < minL) col.L = minL; else if (col.L > maxL) col.L = maxL;
    float sat = hypotf(col.a, col.b);
    if (sat > maxC)
    {
        col.a *= maxC/sat;
        col.b *= maxC/sat;
    }
    return col;
}


ColourRGBA8 ImageHandler::OrderedDitherBayer2x2(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias)
{
    col.L += bayer2x2[(y % 2) * 2 + (x % 2)] * amtL;
    float sat = hypotf(col.a, col.b);
    float hue = atan2f(col.b, col.a);
    const float midsat = -amtS * bayer2x2[(y % 2) * 2 + ((x + 1) % 2)];
    sat *= 1.0f + midsat;
    sat += midsat * 0.5f;
    hue += amtH * bayer4x4[((y + 1) % 2) * 2 + (x % 2)];
    col.a = sat * cosf(hue);
    col.b = sat * sinf(hue);
    return GetClosestColourOkLab(col, bright, contrast, uvbias);
}

ColourRGBA8 ImageHandler::OrderedDitherBayer4x4(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias)
{
    col.L += bayer4x4[(y % 4) * 4 + (x % 4)] * amtL;
    float sat = hypotf(col.a, col.b);
    float hue = atan2f(col.b, col.a);
    const float midsat = -amtS * bayer4x4[((y + 3) % 4) * 4 + ((x + 1) % 4)];
    sat *= 1.0f + midsat;
    sat += midsat * 0.5f;
    hue += amtH * bayer4x4[((y + 1) % 4) * 4 + ((x + 2) % 4)];
    col.a = sat * cosf(hue);
    col.b = sat * sinf(hue);
    return GetClosestColourOkLab(col, bright, contrast, uvbias);
}

ColourRGBA8 ImageHandler::OrderedDitherBayer8x8(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias)
{
    col.L += bayer8x8[(y % 8) * 8 + (x % 8)] * amtL;
    float sat = hypotf(col.a, col.b);
    float hue = atan2f(col.b, col.a);
    const float midsat = -amtS * bayer8x8[((y + 6) % 8) * 8 + ((x + 1) % 8)];
    sat *= 1.0f + midsat;
    sat += midsat * 0.5f;
    hue += amtH * bayer8x8[((y + 3) % 8) * 8 + ((x + 4) % 8)];
    col.a = sat * cosf(hue);
    col.b = sat * sinf(hue);
    return GetClosestColourOkLab(col, bright, contrast, uvbias);
}

ColourRGBA8 ImageHandler::OrderedDitherBayer16x16(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias)
{
    col.L += bayer16x16[(y % 16) * 16 + (x % 16)] * amtL;
    float sat = hypotf(col.a, col.b);
    float hue = atan2f(col.b, col.a);
    const float midsat = -amtS * bayer16x16[((y + 7) % 16) * 16 + ((x + 4) % 16)];
    sat *= 1.0f + midsat;
    sat += midsat * 0.5f;
    hue += amtH * bayer16x16[((y + 10) % 16) * 16 + ((x + 1) % 16)];
    col.a = sat * cosf(hue);
    col.b = sat * sinf(hue);
    return GetClosestColourOkLab(col, bright, contrast, uvbias);
}

ColourRGBA8 ImageHandler::OrderedDitherVoid16x16(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias)
{
    int matind = (y % 16) * 16 + (x % 16);
    col.L += void16x16_1[matind] * amtL;
    float sat = hypotf(col.a, col.b);
    float hue = atan2f(col.b, col.a);
    const float midsat = -amtS * void16x16_2[matind];
    sat *= 1.0f + midsat;
    sat += midsat * 0.5f;
    hue += amtH * void16x16_3[matind];
    col.a = sat * cosf(hue);
    col.b = sat * sinf(hue);
    return GetClosestColourOkLab(col, bright, contrast, uvbias);
}

ColourRGBA8 ImageHandler::DitherFloydSteinberg(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC)
{
    ColourOkLabA outerr;
    ColourOkLabA* diffCol = &diffErr[x +  y * w];
    float inalpha = col.A;
    col = ColourOkLabAAddAccumulate(col, *diffCol);
    col = ClampColourOkLab(col);
    ColourOkLabA outcol = GetClosestColourOkLabWithError(col, &outerr, bright, contrast, uvbias, rngAmtL, rngAmtC);

    diffCol = &diffErr[(x + boustro) +  y * w];
    const ColourOkLabA coeff1 = { 0.4375f * amtL,  0.4375f * amtC, 0.4375f * amtC, 1.0f };
    *diffCol = ColourOkLabAFMAAccumulate(*diffCol, outerr, coeff1);
    diffCol = &diffErr[(x - boustro) + (y + 1) * w];
    const ColourOkLabA coeff2 = { 0.1875f * amtL,  0.1875f * amtC, 0.1875f * amtC, 1.0f };
    *diffCol = ColourOkLabAFMAAccumulate(*diffCol, outerr, coeff2);
    diffCol = &diffErr[ x            + (y + 1) * w];
    const ColourOkLabA coeff3 = { 0.3125f * amtL,  0.3125f * amtC, 0.3125f * amtC, 1.0f };
    *diffCol = ColourOkLabAFMAAccumulate(*diffCol, outerr, coeff3);
    diffCol = &diffErr[(x + boustro) + (y + 1) * w];
    const ColourOkLabA coeff4 = { 0.0625f * amtL,  0.0625f * amtC, 0.0625f * amtC, 1.0f };
    *diffCol = ColourOkLabAFMAAccumulate(*diffCol, outerr, coeff4);

    outcol.A = inalpha;
    return LinearFloatToSRGB8(OkLabToSRGB(outcol));
}

ColourRGBA8 ImageHandler::DitherFloydFalse(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC)
{
    ColourOkLabA outerr;
    ColourOkLabA* diffCol = &diffErr[x +  y * w];
    float inalpha = col.A;
    col = ColourOkLabAAddAccumulate(col, *diffCol);
    col = ClampColourOkLab(col);
    ColourOkLabA outcol = GetClosestColourOkLabWithError(col, &outerr, bright, contrast, uvbias, rngAmtL, rngAmtC);

    const ColourOkLabA coeff1 = { 0.375f * amtL, 0.375f * amtC, 0.375f * amtC, 1.0f };
    const ColourOkLabA errc = ColourOkLabAMultiply(outerr, coeff1);
    diffCol = &diffErr[(x + boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x            + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    const ColourOkLabA coeff2 = { 0.25f * amtL, 0.25f * amtC, 0.25f * amtC, 1.0f };
    diffCol = &diffErr[(x + boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAFMAAccumulate(*diffCol, outerr, coeff2);

    outcol.A = inalpha;
    return LinearFloatToSRGB8(OkLabToSRGB(outcol));
}

ColourRGBA8 ImageHandler::DitherJJN(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC)
{
    ColourOkLabA outerr;
    ColourOkLabA* diffCol = &diffErr[x +  y * w];
    float inalpha = col.A;
    col = ColourOkLabAAddAccumulate(col, *diffCol);
    col = ClampColourOkLab(col);
    ColourOkLabA outcol = GetClosestColourOkLabWithError(col, &outerr, bright, contrast, uvbias, rngAmtL, rngAmtC);

    const ColourOkLabA coeff1 = { (7.0f/48.0f) * amtL, (7.0f/48.0f) * amtC, (7.0f/48.0f) * amtC, 1.0f };
    ColourOkLabA errc = ColourOkLabAMultiply(outerr, coeff1);
    diffCol = &diffErr[(x + boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x            + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff2 = { (5.0f/48.0f) * amtL, (5.0f/48.0f) * amtC, (5.0f/48.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff2);
    diffCol = &diffErr[(x + 2 * boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x + boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x                + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff3 = { (3.0f/48.0f) * amtL, (3.0f/48.0f) * amtC, (3.0f/48.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff3);
    diffCol = &diffErr[(x + 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x + boustro)     + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro)     + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff4 = { (1.0f/48.0f) * amtL, (1.0f/48.0f) * amtC, (1.0f/48.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff4);
    diffCol = &diffErr[(x + 2 * boustro) + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - 2 * boustro) + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    outcol.A = inalpha;
    return LinearFloatToSRGB8(OkLabToSRGB(outcol));
}

ColourRGBA8 ImageHandler::DitherStucki(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC)
{
    ColourOkLabA outerr;
    ColourOkLabA* diffCol = &diffErr[x +  y * w];
    float inalpha = col.A;
    col = ColourOkLabAAddAccumulate(col, *diffCol);
    col = ClampColourOkLab(col);
    ColourOkLabA outcol = GetClosestColourOkLabWithError(col, &outerr, bright, contrast, uvbias, rngAmtL, rngAmtC);

    const ColourOkLabA coeff1 = { (8.0f/42.0f) * amtL, (8.0f/42.0f) * amtC, (8.0f/42.0f) * amtC, 1.0f };
    ColourOkLabA errc = ColourOkLabAMultiply(outerr, coeff1);
    diffCol = &diffErr[(x + boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x            + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff2 = { (4.0f/42.0f) * amtL, (4.0f/42.0f) * amtC, (4.0f/42.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff2);
    diffCol = &diffErr[(x + 2 * boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x + boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x                + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff3 = { (2.0f/42.0f) * amtL, (2.0f/42.0f) * amtC, (2.0f/42.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff3);
    diffCol = &diffErr[(x + 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x + boustro)     + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro)     + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff4 = { (1.0f/42.0f) * amtL, (1.0f/42.0f) * amtC, (1.0f/42.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff4);
    diffCol = &diffErr[(x + 2 * boustro) + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - 2 * boustro) + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    outcol.A = inalpha;
    return LinearFloatToSRGB8(OkLabToSRGB(outcol));
}

ColourRGBA8 ImageHandler::DitherBurkes(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC)
{
    ColourOkLabA outerr;
    ColourOkLabA* diffCol = &diffErr[x +  y * w];
    float inalpha = col.A;
    col = ColourOkLabAAddAccumulate(col, *diffCol);
    col = ClampColourOkLab(col);
    ColourOkLabA outcol = GetClosestColourOkLabWithError(col, &outerr, bright, contrast, uvbias, rngAmtL, rngAmtC);

    const ColourOkLabA coeff1 = { (8.0f/32.0f) * amtL, (8.0f/32.0f) * amtC, (8.0f/32.0f) * amtC, 1.0f };
    ColourOkLabA errc = ColourOkLabAMultiply(outerr, coeff1);
    diffCol = &diffErr[(x + boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x            + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff2 = { (4.0f/32.0f) * amtL, (4.0f/32.0f) * amtC, (4.0f/32.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff2);
    diffCol = &diffErr[(x + 2 * boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x + boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff3 = { (2.0f/32.0f) * amtL, (2.0f/32.0f) * amtC, (2.0f/32.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff3);
    diffCol = &diffErr[(x + 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    outcol.A = inalpha;
    return LinearFloatToSRGB8(OkLabToSRGB(outcol));
}

ColourRGBA8 ImageHandler::DitherSierra(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC)
{
    ColourOkLabA outerr;
    ColourOkLabA* diffCol = &diffErr[x +  y * w];
    float inalpha = col.A;
    col = ColourOkLabAAddAccumulate(col, *diffCol);
    col = ClampColourOkLab(col);
    ColourOkLabA outcol = GetClosestColourOkLabWithError(col, &outerr, bright, contrast, uvbias, rngAmtL, rngAmtC);

    const ColourOkLabA coeff1 = { (5.0f/32.0f) * amtL, (5.0f/32.0f) * amtC, (5.0f/32.0f) * amtC, 1.0f };
    ColourOkLabA errc = ColourOkLabAMultiply(outerr, coeff1);
    diffCol = &diffErr[(x + boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x            + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff2 = { (4.0f/32.0f) * amtL, (4.0f/32.0f) * amtC, (4.0f/32.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff2);
    diffCol = &diffErr[(x + boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff3 = { (3.0f/32.0f) * amtL, (3.0f/32.0f) * amtC, (3.0f/32.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff3);
    diffCol = &diffErr[(x + 2 * boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x                + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff4 = { (2.0f/32.0f) * amtL, (2.0f/32.0f) * amtC, (2.0f/32.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff4);
    diffCol = &diffErr[(x + 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x + boustro)     + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro)     + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    outcol.A = inalpha;
    return LinearFloatToSRGB8(OkLabToSRGB(outcol));
}

ColourRGBA8 ImageHandler::DitherSierra2Row(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC)
{
    ColourOkLabA outerr;
    ColourOkLabA* diffCol = &diffErr[x +  y * w];
    float inalpha = col.A;
    col = ColourOkLabAAddAccumulate(col, *diffCol);
    col = ClampColourOkLab(col);
    ColourOkLabA outcol = GetClosestColourOkLabWithError(col, &outerr, bright, contrast, uvbias, rngAmtL, rngAmtC);

    const ColourOkLabA coeff1 = { (4.0f/16.0f) * amtL, (4.0f/16.0f) * amtC, (4.0f/16.0f) * amtC, 1.0f };
    diffCol = &diffErr[(x + boustro) +  y * w];
    *diffCol = ColourOkLabAFMAAccumulate(*diffCol, outerr, coeff1);

    const ColourOkLabA coeff2 = { (3.0f/16.0f) * amtL, (3.0f/16.0f) * amtC, (3.0f/16.0f) * amtC, 1.0f };
    ColourOkLabA errc = ColourOkLabAMultiply(outerr, coeff2);
    diffCol = &diffErr[(x + 2 * boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x                + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff3 = { (2.0f/16.0f) * amtL, (2.0f/16.0f) * amtC, (2.0f/16.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff3);
    diffCol = &diffErr[(x + boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    const ColourOkLabA coeff4 = { (1.0f/16.0f) * amtL, (1.0f/16.0f) * amtC, (1.0f/16.0f) * amtC, 1.0f };
    errc = ColourOkLabAMultiply(outerr, coeff4);
    diffCol = &diffErr[(x + 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - 2 * boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    outcol.A = inalpha;
    return LinearFloatToSRGB8(OkLabToSRGB(outcol));
}

ColourRGBA8 ImageHandler::DitherFilterLite(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC)
{
    ColourOkLabA outerr;
    ColourOkLabA* diffCol = &diffErr[x +  y * w];
    float inalpha = col.A;
    col = ColourOkLabAAddAccumulate(col, *diffCol);
    col = ClampColourOkLab(col);
    ColourOkLabA outcol = GetClosestColourOkLabWithError(col, &outerr, bright, contrast, uvbias, rngAmtL, rngAmtC);

    const ColourOkLabA coeff1 = { 0.5f * amtL, 0.5f * amtC, 0.5f * amtC, 1.0f };
    diffCol = &diffErr[(x + boustro) +  y * w];
    *diffCol = ColourOkLabAFMAAccumulate(*diffCol, outerr, coeff1);
    const ColourOkLabA coeff2 = { 0.25f * amtL, 0.25f * amtC, 0.25f * amtC, 1.0f };
    const ColourOkLabA errc = ColourOkLabAMultiply(outerr, coeff2);
    diffCol = &diffErr[ x            + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro) + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    outcol.A = inalpha;
    return LinearFloatToSRGB8(OkLabToSRGB(outcol));
}

ColourRGBA8 ImageHandler::DitherAtkinson(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC)
{
    ColourOkLabA outerr;
    ColourOkLabA* diffCol = &diffErr[x +  y * w];
    float inalpha = col.A;
    col = ColourOkLabAAddAccumulate(col, *diffCol);
    col = ClampColourOkLab(col);
    ColourOkLabA outcol = GetClosestColourOkLabWithError(col, &outerr, bright, contrast, uvbias, rngAmtL, rngAmtC);

    const ColourOkLabA coeff = { amtL/6.0f, amtC/6.0f, amtC/6.0f, 1.0f }; //Note: the canonical Atkinson dither only diffuses 3/4 of the error, but we'll normalise this one anyway
    const ColourOkLabA errc = ColourOkLabAMultiply(outerr, coeff);
    diffCol = &diffErr[(x + boustro)     +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x + 2 * boustro) +  y * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x - boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x                + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[(x + boustro)     + (y + 1) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);
    diffCol = &diffErr[ x                + (y + 2) * w];
    *diffCol = ColourOkLabAAddAccumulate(*diffCol, errc);

    outcol.A = inalpha;
    return LinearFloatToSRGB8(OkLabToSRGB(outcol));
}
