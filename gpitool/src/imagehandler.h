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

#pragma once

#include <math.h>
#include <random>

#define PLANEMASK_MASK 0x100
#define PLANENUM_MASK 8

typedef struct
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;
} ColourRGBA8;

typedef struct
{
    float R;
    float G;
    float B;
    float A;
} ColourRGBA;

typedef struct
{
    float L;
    float a;
    float b;
    float A;
} ColourOkLabA;

typedef struct
{
    int width;
    int height;
    ColourRGBA8* data;
} ImageInfo;

typedef struct
{
    unsigned char** planeData;
    unsigned short planeMask;
    int planew;
    int planeh;
    int numTiles;
    int planeSize;
    int numPlanes;
    int numColours;
    bool is8BitColour;
} PlanarInfo;

const float OkLabK1 = 0.206f;
const float OkLabK2 = 0.03f;
const float OkLabK3 = 1.17087378640776f;

const float SRGBtoLMS[9] = { 0.4122214708f, 0.5363325363f, 0.0514459929f,
                             0.2119034982f, 0.6806995451f, 0.1073969566f,
                             0.0883024619f, 0.2817188376f, 0.6299787005f };

const float CRLMStoOKLab[9] = { 0.2104542553f,  0.7936177850f, -0.0040720468f,
                                1.9779984951f, -2.4285922050f,  0.4505937099f,
                                0.0259040371f,  0.7827717662f, -0.8086757660f };

const float OKLabtoCRLMS[9] = { 1.0f,  0.3963377774f,  0.2158037573f,
                                1.0f, -0.1055613458f, -0.0638541728f,
                                1.0f, -0.0894841775f, -1.2914855480f };

const float LMStoSRGB[9] = { 4.0767416621f, -3.3077115913f,  0.2309699292f,
                            -1.2684380046f,  2.6097574011f, -0.3413193965f,
                            -0.0041960863f, -0.7034186147f,  1.7076147010f };

inline ColourOkLabA ColourOkLabAAdd(ColourOkLabA l, ColourOkLabA r)
{
    ColourOkLabA c;
    float ca[4];
    float la[4] = { l.L, l.a, l.b, l.A };
    float ra[4] = { r.L, r.a, r.b, r.A };
    for (int i = 0; i < 4; i++)
    {
        ca[i] = la[i] + ra[i];
    }
    c.L = ca[0]; c.a = ca[1]; c.b = ca[2]; c.A = ca[3];
    return c;
}

inline ColourOkLabA ColourOkLabAAddAccumulate(ColourOkLabA l, ColourOkLabA r)
{
    float la[4] = { l.L, l.a, l.b, l.A };
    float ra[4] = { r.L, r.a, r.b, r.A };
    for (int i = 0; i < 4; i++)
    {
        la[i] += ra[i];
    }
    l.L = la[0]; l.a = la[1]; l.b = la[2]; l.A = la[3];
    return l;
}

inline ColourOkLabA ColourOkLabAMultiply(ColourOkLabA l, ColourOkLabA r)
{

    ColourOkLabA c;
    float ca[4];
    float la[4] = { l.L, l.a, l.b, l.A };
    float ra[4] = { r.L, r.a, r.b, r.A };
    for (int i = 0; i < 4; i++)
    {
        ca[i] = la[i] * ra[i];
    }
    c.L = ca[0]; c.a = ca[1]; c.b = ca[2]; c.A = ca[3];
    return c;
}

inline ColourOkLabA ColourOkLabAMultiplyAccumulate(ColourOkLabA l, ColourOkLabA r)
{
    float la[4] = { l.L, l.a, l.b, l.A };
    float ra[4] = { r.L, r.a, r.b, r.A };
    for (int i = 0; i < 4; i++)
    {
        la[i] *= ra[i];
    }
    l.L = la[0]; l.a = la[1]; l.b = la[2]; l.A = la[3];
    return l;
}

inline ColourOkLabA ColourOkLabAFMA(ColourOkLabA a, ColourOkLabA ml, ColourOkLabA mr)
{
    ColourOkLabA c;
    float ca[4];
    float aa[4] = { a.L, a.a, a.b, a.A };
    float mla[4] = { ml.L, ml.a, ml.b, ml.A };
    float mra[4] = { mr.L, mr.a, mr.b, mr.A };
    for (int i = 0; i < 4; i++)
    {
        ca[i] = aa[i] + (mla[i] * mra[i]);
    }
    c.L = ca[0]; c.a = ca[1]; c.b = ca[2]; c.A = ca[3];
    return c;
}

inline ColourOkLabA ColourOkLabAFMAAccumulate(ColourOkLabA a, ColourOkLabA ml, ColourOkLabA mr)
{
    float aa[4] = { a.L, a.a, a.b, a.A };
    float mla[4] = { ml.L, ml.a, ml.b, ml.A };
    float mra[4] = { mr.L, mr.a, mr.b, mr.A };
    for (int i = 0; i < 4; i++)
    {
        aa[i] += (mla[i] * mra[i]);
    }
    a.L = aa[0]; a.a = aa[1]; a.b = aa[2]; a.A = aa[3];
    return a;
}

inline ColourRGBA SRGBToLinear(ColourRGBA c)
{
    if (c.R <= 0.04045f) c.R /= 12.92f;
    else c.R = powf((c.R + 0.055f)/1.055f, 2.4f);
    if (c.G <= 0.04045f) c.G /= 12.92f;
    else c.G = powf((c.G + 0.055f)/1.055f, 2.4f);
    if (c.B <= 0.04045f) c.B /= 12.92f;
    else c.B = powf((c.B + 0.055f)/1.055f, 2.4f);
    return c;
}

inline ColourRGBA LinearToSRGB(ColourRGBA c)
{
    if (c.R <= 0.0031308f) c.R *= 12.92f;
    else c.R = 1.055f * powf(c.R, 1.0f/2.4f) - 0.055f;
    if (c.G <= 0.0031308f) c.G *= 12.92f;
    else c.G = 1.055f * powf(c.G, 1.0f/2.4f) - 0.055f;
    if (c.B <= 0.0031308f) c.B *= 12.92f;
    else c.B = 1.055f * powf(c.B, 1.0f/2.4f) - 0.055f;
    return c;
}

inline ColourRGBA SRGB8ToLinearFloat(ColourRGBA8 c)
{
    ColourRGBA fltcol = { ((float)c.R)/255.0f, ((float)c.G)/255.0f, ((float)c.B)/255.0f, ((float)c.A)/255.0f };
    return SRGBToLinear(fltcol);
}

inline ColourRGBA8 LinearFloatToSRGB8(ColourRGBA c)
{
    ColourRGBA fltcol = LinearToSRGB(c);
    int oR = (int)((fltcol.R * 255.0f) + 0.5f);
    int oG = (int)((fltcol.G * 255.0f) + 0.5f);
    int oB = (int)((fltcol.B * 255.0f) + 0.5f);
    int oA = (int)((fltcol.A * 255.0f) + 0.5f);
    if (oR > 0xFF) oR = 0xFF; else if (oR < 0) oR = 0;
    if (oG > 0xFF) oG = 0xFF; else if (oG < 0) oG = 0;
    if (oB > 0xFF) oB = 0xFF; else if (oB < 0) oB = 0;
    if (oA > 0xFF) oA = 0xFF; else if (oA < 0) oA = 0;
    ColourRGBA8 outcol = { (unsigned char)oR, (unsigned char)oG, (unsigned char)oB, (unsigned char)oA };
    return outcol;
}

inline ColourRGBA8 LinearFloatToSRGB4(ColourRGBA c)
{
    ColourRGBA fltcol = LinearToSRGB(c);
    int oR = (int)((fltcol.R * 15.0f) + 0.5f);
    int oG = (int)((fltcol.G * 15.0f) + 0.5f);
    int oB = (int)((fltcol.B * 15.0f) + 0.5f);
    int oA = (int)((fltcol.A * 15.0f) + 0.5f);
    if (oR > 0xF) oR = 0xF; else if (oR < 0) oR = 0;
    if (oG > 0xF) oG = 0xF; else if (oG < 0) oG = 0;
    if (oB > 0xF) oB = 0xF; else if (oB < 0) oB = 0;
    if (oA > 0xF) oA = 0xF; else if (oA < 0) oA = 0;
    oR *= 0x11; oG *= 0x11; oB *= 0x11; oA *= 0x11;
    ColourRGBA8 outcol = { (unsigned char)oR, (unsigned char)oG, (unsigned char)oB, (unsigned char)oA };
    return outcol;
}

inline ColourOkLabA ColourAdjust(ColourOkLabA c, float bright, float contrast)
{
    c.L += bright;

    float contrastfac = (1.05f * (contrast + 1.0f)) / (1.05f - contrast);
    c.L -= 0.5f;
    c.L *= contrastfac; c.a *= contrastfac; c.b *= contrastfac;
    c.L += 0.5f;

    return c;
}

ColourOkLabA SRGBToOkLab(ColourRGBA c);
ColourRGBA OkLabToSRGB(ColourOkLabA c);
ColourRGBA8 BlendSRGB8(ColourRGBA8 l, ColourRGBA8 r, float amt);

enum ditherMethods
{
    NODITHER,
    BAYER2X2,
    BAYER4X4,
    BAYER8X8,
    BAYER16X16,
    VOID16X16,
    FLOYD_STEINBERG,
    FLOYD_FALSE,
    JJN,
    STUCKI,
    BURKES,
    SIERRA,
    SIERRA2ROW,
    FILTERLITE,
    ATKINSON
};

enum tileOrderings
{
    ROWMAJOR,
    COLUMNMAJOR
};

class ImageHandler
{
public:
    ImageHandler();
    ~ImageHandler();

    int OpenImageFile(char* inFileName);
    void CloseImageFile();
    bool IsPalettePerfect();
    bool GetBestPalette(float uvbias, float bright, float contrast);
    bool LoadPaletteFile(char* inFileName);
    bool SavePaletteFile(char* outFileName);
    void ShufflePaletteBasedOnOccurrence();
    void DitherImage();
    void DitherImage(int ditherMethod, double ditAmtL, double ditAmtS, double ditAmtH, double ditAmtEL, double ditAmtEC, double rngAmtL, double rngAmtC, double cbias, double preB, double preC, double postB, double postC, bool globBoustro);
    PlanarInfo GeneratePlanarData();
    static void FreePlanarData(PlanarInfo* pinfo);

    inline ImageInfo* GetEncodedImage() { return &encImage; }
    inline ColourRGBA8* GetCurrentPalette() { return palette; }
    inline int GetPlaneMask() { return planeMask; }
    inline int GetNumColours() { return numColours; }
    inline int GetNumColourPlanes() { return numColourPlanes; }

    inline void SetPaletteColour(int index, ColourRGBA8 col)
    {
        palette[index] = col;
        labPalette[index] = SRGBToOkLab(SRGB8ToLinearFloat(col));
    }

    inline void AddPlane(int planeNum)
    {
        if (planeNum < 0 || planeNum > 8) return;
        int maskBit = 1 << planeNum;
        if (!(planeMask & maskBit))
        {
            planeMask |= maskBit;
            if (planeNum != PLANENUM_MASK)
            {
                numColourPlanes++;
                numColours = 1 << numColourPlanes;
                GetLabPaletteFromRGBA8Palette();
            }
        }
    }

    inline void RemovePlane(int planeNum)
    {
        if (planeNum < 0 || planeNum > 8) return;
        int maskBit = 1 << planeNum;
        if (planeMask & maskBit)
        {
            planeMask &= ~maskBit;
            if (planeNum != PLANENUM_MASK)
            {
                numColourPlanes--;
                numColours = 1 << numColourPlanes;
                GetLabPaletteFromRGBA8Palette();
            }
        }
    }

    int transparencyThreshold;
    bool is8BitColour;

    int ditherMethod;
    double luminosityDither;
    double saturationDither;
    double hueDither;
    double luminosityDiffusion;
    double chromaDiffusion;
    double luminosityRandomisation;
    double chromaRandomisation;
    double chromaBias;
    double preBrightness;
    double preContrast;
    double postBrightness;
    double postContrast;
    bool boustrophedon;

    bool isTiled;
    int tileSizeX;
    int tileSizeY;
    int tileOrdering;

private:
    inline unsigned long long RNGUpdate()
    {
        return rng();
    }

    inline float RNGUpdateFloat()
    {
        unsigned long long r = RNGUpdate();
        unsigned int o = 0x3F800000; //1.0
        o |= r >> 41;
        float f = *((float*)(&o)); //Should be between 1 and ~2
        return 2.0f * (f - 1.5f); //Should be between -1 and ~1
    }

    inline double RNGUpdateDouble()
    {
        unsigned long long r = RNGUpdate();
        unsigned long long o = 0x3FF0000000000000; //1.0
        o |= r >> 12;
        double f = *((double*)(&o)); //Should be between 1 and ~2
        return 2.0 * (f - 1.5); //Should be between -1 and ~1
    }

    void GetLabPaletteFromRGBA8Palette();
    ColourRGBA8 GetClosestColourOkLab(ColourOkLabA col, float bright, float contrast, float uvbias);
    ColourOkLabA GetClosestColourOkLabWithError(ColourOkLabA col, ColourOkLabA* error, float bright, float contrast, float uvbias, float rngAmtL, float rngAmtC);
    ColourOkLabA ClampColourOkLab(ColourOkLabA col);
    ColourRGBA8 OrderedDitherBayer2x2(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias);
    ColourRGBA8 OrderedDitherBayer4x4(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias);
    ColourRGBA8 OrderedDitherBayer8x8(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias);
    ColourRGBA8 OrderedDitherBayer16x16(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias);
    ColourRGBA8 OrderedDitherVoid16x16(ColourOkLabA col, int x, int y, float amtL, float amtS, float amtH, float bright, float contrast, float uvbias);
    ColourRGBA8 DitherFloydSteinberg(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC);
    ColourRGBA8 DitherFloydFalse(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC);
    ColourRGBA8 DitherJJN(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC);
    ColourRGBA8 DitherStucki(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC);
    ColourRGBA8 DitherBurkes(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC);
    ColourRGBA8 DitherSierra(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC);
    ColourRGBA8 DitherSierra2Row(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC);
    ColourRGBA8 DitherFilterLite(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC);
    ColourRGBA8 DitherAtkinson(ColourOkLabA col, int x, int y, int w, float amtL, float amtC, float bright, float contrast, float uvbias, ColourOkLabA* diffErr, int boustro, float rngAmtL, float rngAmtC);

    std::mt19937_64 rng;
    ImageInfo srcImage;
    ImageInfo encImage;
    ColourRGBA8 palette[256];
    ColourOkLabA labPalette[256];
    int numColours;
    int planeMask;
    int numColourPlanes;
    float maxL;
    float minL;
    float maxC;
};
