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
 * Image compressor
 */

extern "C"
{
    #include <zlib.h>
}

#include <string.h>
#include <vector>
#include <algorithm>
#include "imagecompressor.h"

ImageCompressor::ImageCompressor()
{

}

ImageCompressor::~ImageCompressor()
{

}

typedef struct
{
    int copyFrom;
    int copyTo;
    int copyDistance;
    int copyLength;
    int tempCopyLength;
    //bits 0-1 -> escape type (0 -> none, 1 -> byte 1, 2 -> byte 2, 3 -> rejected at the first stage), bit 2 -> still expandable
    int typeBits;
} LZMatch;

int ImageCompressor::CompressAndSaveImageDeflate(char* outFileName)
{
    //Header
    unsigned char header[268];
    int headerSize = 0x0C;
    header[0x0] = 'G'; header[0x1] = 'P'; header[0x2] = 'I'; //Magic number
    header[0x3] = 0x00; //Flags
    ImageInfo* iinfo = ihand->GetEncodedImage();
    PlanarInfo pinfo = ihand->GeneratePlanarData();
    ColourRGBA8* pal = ihand->GetCurrentPalette();
    *((uint16_t*)(&header[0x4])) = (uint16_t)(iinfo->width - 1);
    *((uint16_t*)(&header[0x6])) = (uint16_t)(iinfo->height - 1);
    *((uint16_t*)(&header[0x8])) = 0x0000; //Number of tiles (fixed to 1 for now)
    *((uint16_t*)(&header[0xA])) = pinfo.planeMask;
    if (pinfo.is8BitColour)
    {
        header[0x3] |= 0x08;
        for (int i = 0; i < pinfo.numColours; i++)
        {
            header[i * 3] = pal[i].R;
            header[i * 3 + 1] = pal[i].G;
            header[i * 3 + 2] = pal[i].B;
            headerSize += 3;
        }
    }
    else
    {
        for (int i = 0; i < pinfo.numColours; i++)
        {
            unsigned char compByte;
            short inCol = (short)(pal[i].R);
            inCol += 0x08; inCol /= 0x11;
            if (inCol > 0xF) inCol = 0xF; else if (inCol < 0x0) inCol = 0x0;
            compByte = inCol & 0xF;
            inCol = (short)(pal[i].G);
            inCol += 0x08; inCol /= 0x11;
            if (inCol > 0xF) inCol = 0xF; else if (inCol < 0x0) inCol = 0x0;
            compByte |= (inCol & 0xF) << 4;
            header[headerSize] = compByte;
            headerSize++;
            inCol = (short)(pal[i].B);
            inCol += 0x08; inCol /= 0x11;
            if (inCol > 0xF) inCol = 0xF; else if (inCol < 0x0) inCol = 0x0;
            compByte = inCol & 0xF;
            i++;
            inCol = (short)(pal[i].R);
            inCol += 0x08; inCol /= 0x11;
            if (inCol > 0xF) inCol = 0xF; else if (inCol < 0x0) inCol = 0x0;
            compByte |= (inCol & 0xF) << 4;
            header[headerSize] = compByte;
            headerSize++;
            inCol = (short)(pal[i].G);
            inCol += 0x08; inCol /= 0x11;
            if (inCol > 0xF) inCol = 0xF; else if (inCol < 0x0) inCol = 0x0;
            compByte = inCol & 0xF;
            inCol = (short)(pal[i].B);
            inCol += 0x08; inCol /= 0x11;
            if (inCol > 0xF) inCol = 0xF; else if (inCol < 0x0) inCol = 0x0;
            compByte |= (inCol & 0xF) << 4;
            header[headerSize] = compByte;
            headerSize++;
        }
    }

    //Compress planes
    unsigned char* filterTable = new unsigned char[(pinfo.planeh+1)/2];
    unsigned char* fPlane = new unsigned char[pinfo.planeSize];
    unsigned char* fRows[16];
    for (int i = 0; i < 16; i++)
    {
        fRows[i] = new unsigned char[pinfo.planew];
    }
    unsigned char* finalPlaneData[9];
    bool isFiltered = false;
    for (int i = 0; i < pinfo.numPlanes; i++)
    {
        unsigned char* curPlane = pinfo.planeData[i];
        uint32_t compressedSize = 0;

        //Find the best filters for each line heuristically (minimal entropy)
        unsigned char* compressedData = new unsigned char[pinfo.planeSize * 2]; //overallocate just in case
        finalPlaneData[i] = compressedData;
        int pw = pinfo.planew;
        int ph = pinfo.planeh;
        int totalOccurrence[256];
        int tempOccurrence[256*16];
        double entropy[16];
        memset(totalOccurrence, 0, sizeof(totalOccurrence));
        for (int j = 0; j < ph; j++)
        {
            //Try all valid filters
            memset(tempOccurrence, 0, sizeof(tempOccurrence));
            for (int k = 0; k < 16; k++)
            {
                if (j == 0 && k & 0x8) //Reject NOT filters on the first line, we don't want to bias towards a needlessly complicated filter
                {
                    entropy[k] = 9999999999999999999999999999.9;
                    continue;
                }
                int filtType = k & 0x7;
                switch (filtType) //Initial validation
                {
                    case 0: //filt = S[x,y], unconditional
                    case 1: //filt = S[x,y] XOR S[x-1,y], unconditional
                        break;
                    case 2: //filt = S[x,y] XOR S[x,y-1], current y must not be 0
                        if (j < 1)
                        {
                            entropy[k] = 9999999999999999999999999999.9;
                            continue;
                        }
                        break;
                    case 3: //filt = S[x,y] XOR S[x,y-height] (one tile before), current tile must not be 0 (TODO)
                        entropy[k] = 9999999999999999999999999999.9;
                        continue;
                    case 4: //filt = S[x,y] XOR S[x,y] one plane before, current plane must be 1 or higher
                        if (i < 1)
                        {
                            entropy[k] = 9999999999999999999999999999.9;
                            continue;
                        }
                        break;
                    case 5: //filt = S[x,y] XOR S[x,y] two planes before, current plane must be 2 or higher
                        if (i < 2)
                        {
                            entropy[k] = 9999999999999999999999999999.9;
                            continue;
                        }
                        break;
                    case 6: //filt = S[x,y] XOR S[x,y] three planes before, current plane must be 3 or higher
                        if (i < 3)
                        {
                            entropy[k] = 9999999999999999999999999999.9;
                            continue;
                        }
                        break;
                    case 7: //filt = S[x,y] XOR S[x,y] four planes before, current plane must be 4 or higher
                        if (i < 4)
                        {
                            entropy[k] = 9999999999999999999999999999.9;
                            continue;
                        }
                        break;
                }

                //Filter line
                switch (k)
                {
                    case 0: //filt = S[x,y]
                        memcpy(fRows[k], &curPlane[j * pw], pw);
                        break;
                    case 1: //filt = S[x,y] XOR S[x-1,y]
                    {
                        unsigned char carry = 0;
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in = curPlane[j * pw + n];
                            unsigned char out = in ^ ((in >> 1) | carry);
                            if (in & 0x01) carry = 0x80;
                            else carry = 0x00;
                            fRows[k][n] = out;
                        }
                    }
                        break;
                    case 2: //filt = S[x,y] XOR S[x,y-1]
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = curPlane[(j - 1) * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = out;
                        }
                        break;
                    case 3: //filt = S[x,y] XOR S[x,y-height] (one tile before)
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = curPlane[(j - ph) * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = out;
                        }
                        break;
                    case 4: //filt = S[x,y] XOR S[x,y] one plane before
                    {
                        unsigned char* tPlane = pinfo.planeData[i-1];
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = tPlane[j * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = out;
                        }
                    }
                        break;
                    case 5: //filt = S[x,y] XOR S[x,y] two planes before
                    {
                        unsigned char* tPlane = pinfo.planeData[i-2];
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = tPlane[j * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = out;
                        }
                    }
                        break;
                    case 6: //filt = S[x,y] XOR S[x,y] three planes before
                    {
                        unsigned char* tPlane = pinfo.planeData[i-3];
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = tPlane[j * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = out;
                        }
                    }
                        break;
                    case 7: //filt = S[x,y] XOR S[x,y] four planes before
                    {
                        unsigned char* tPlane = pinfo.planeData[i-4];
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = tPlane[j * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = out;
                        }
                    }
                        break;
                    case 8: //filt = NOT(S[x,y])
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in = curPlane[j * pw + n];
                            fRows[k][n] = ~in;
                        }
                        break;
                    case 9: //filt = NOT(S[x,y] XOR S[x-1,y])
                    {
                        unsigned char carry = 0;
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in = curPlane[j * pw + n];
                            unsigned char out = in ^ ((in >> 1) | carry);
                            if (in & 0x01) carry = 0x80;
                            else carry = 0x00;
                            fRows[k][n] = ~out;
                        }
                    }
                        break;
                    case 10: //filt = NOT(S[x,y] XOR S[x,y-1])
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = curPlane[(j - 1) * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = ~out;
                        }
                        break;
                    case 11: //filt = NOT(S[x,y] XOR S[x,y-height])
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = curPlane[(j - ph) * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = ~out;
                        }
                        break;
                    case 12: //filt = NOT(S[x,y] XOR S[x,y] one plane before)
                    {
                        unsigned char* tPlane = pinfo.planeData[i-1];
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = tPlane[j * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = ~out;
                        }
                    }
                        break;
                    case 13: //filt = NOT(S[x,y] XOR S[x,y] two planes before)
                    {
                        unsigned char* tPlane = pinfo.planeData[i-2];
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = tPlane[j * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = ~out;
                        }
                    }
                        break;
                    case 14: //filt = NOT(S[x,y] XOR S[x,y] three planes before)
                    {
                        unsigned char* tPlane = pinfo.planeData[i-3];
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = tPlane[j * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = ~out;
                        }
                    }
                        break;
                    case 15: //filt = NOT(S[x,y] XOR S[x,y] four planes before)
                    {
                        unsigned char* tPlane = pinfo.planeData[i-4];
                        for (int n = 0; n < pw; n++)
                        {
                            unsigned char in1 = curPlane[j * pw + n];
                            unsigned char in2 = tPlane[j * pw + n];
                            unsigned char out = in1 ^ in2;
                            fRows[k][n] = ~out;
                        }
                    }
                        break;
                }

                //Count the occurrence of each byte
                for (int n = 0; n < pw; n++)
                {
                    tempOccurrence[(k * 256) + fRows[k][n]]++;
                }

                //Determine the total entropy of all lines determined so far and this one
                double tempEntropy = 0.0;
                for (int n = 0; n < 256; n++)
                {
                    int occ = totalOccurrence[n] + tempOccurrence[(k * 256) + n];
                    if (occ <= 0) continue;
                    double prob = ((double)occ)/((double)((j+1) * pw));
                    tempEntropy -= prob * log2(prob);
                }
                entropy[k] = tempEntropy;
            }

            //Select locally best filter
            int bestFilter = 0;
            double bestEntropy = 999999999999999999999999999999.9;
            for (int k = 0; k < 16; k++)
            {
                double nextEntropy = entropy[k];
                if (nextEntropy <= bestEntropy)
                {
                    if (nextEntropy == bestEntropy) //Tiebreak
                    {
                        if (!(bestFilter & 0x8) && k & 0x8) //Prefer filters that don't need a NOT over ones that do
                        {
                            continue;
                        }
                        switch (bestFilter & 0x7) //Tiebreaking uses nontrivial rules: we prefer 'simpler' filters over complicated ones
                        {
                            case 0: //filt = S[x,y], top priority (memcpy)
                                break;
                            case 1: //filt = S[x,y] XOR S[x-1,y], lowest priority (complicated af)
                                bestFilter = k;
                                break;
                            case 2: //filt = S[x,y] XOR S[x,y-1], medium priority (near pointer on DOS)
                            case 3: //filt = S[x,y] XOR S[x,y-height] (one tile before), medium priority (near pointer on DOS)
                                break;
                            case 4: //filt = S[x,y] XOR S[x,y] one plane before, low priority (far pointer on DOS)
                            case 5: //filt = S[x,y] XOR S[x,y] two planes before, low priority (far pointer on DOS)
                            case 6: //filt = S[x,y] XOR S[x,y] three planes before, low priority (far pointer on DOS)
                            case 7: //filt = S[x,y] XOR S[x,y] four planes before, low priority (far pointer on DOS)
                                break;
                        }
                    }
                    else //No need to tie break, so select straightforwardly
                    {
                        bestEntropy = nextEntropy;
                        bestFilter = k;
                    }
                }
            }
            if (bestFilter != 0)
            {
                isFiltered = true;
            }
            if (j & 0x1) filterTable[j >> 1] |= (unsigned char)(bestFilter << 4);
            else filterTable[j >> 1] = (unsigned char)bestFilter;
            memcpy(&fPlane[j * pw], fRows[bestFilter], pw);
            for (int k = 0; k < 256; k++)
            {
                totalOccurrence[k] += tempOccurrence[(bestFilter * 256) + k];
            }

            printf("Plane %i/%i, row %i/%i, best filter %1x, bits per 8 pixels %.6f\n", i+1, pinfo.numPlanes, j+1, ph, bestFilter, entropy[bestFilter]);
        }

        //Copy filter table into the compressed data section
        memcpy(compressedData + 4, filterTable, (pinfo.planeh+1)/2);
        unsigned char* cptr = compressedData + 4 + ((pinfo.planeh+1)/2);

        //Compress filtered data using zlib's deflate implementation
        z_stream zStream;
        zStream.zalloc = Z_NULL;
        zStream.zfree = Z_NULL;
        zStream.opaque = Z_NULL;
        deflateInit2(&zStream, 9, Z_DEFLATED, 15, 8, Z_FILTERED);
        zStream.next_in = fPlane;
        zStream.avail_in = pinfo.planeSize;
        zStream.next_out = cptr;
        zStream.avail_out = pinfo.planeSize * 2 - (4 + ((pinfo.planeh+1)/2));
        zStream.data_type = Z_BINARY;
        deflate(&zStream, Z_FINISH);
        compressedSize = zStream.total_out;
        deflateEnd(&zStream);

        *((uint32_t*)(&compressedData[0])) = compressedSize;

        printf("Plane %i done, size %i\n", i, compressedSize);
    }
    for (int i = 0; i < 16; i++)
    {
        delete[] fRows[i];
    }

    //Save to file
    FILE* ofile = fopen(outFileName, "wb");
    fwrite(header, 1, headerSize, ofile);
    for (int i = 0; i < pinfo.numPlanes; i++)
    {
        unsigned char* curPlane = finalPlaneData[i];
        uint32_t size = *((uint32_t*)(&curPlane[0]));
        size += 4 + ((pinfo.planeh+1)/2);
        fwrite(curPlane, 1, size, ofile);
        delete[] finalPlaneData[i];
    }
    fclose(ofile);
    delete[] fPlane;
    delete[] filterTable;
    ImageHandler::FreePlanarData(&pinfo);
    return 0;
}
