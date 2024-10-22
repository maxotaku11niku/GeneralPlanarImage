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

#define USING_COMPRESSION_LZ4
//#define USING_COMPRESSION_DEFLATE

extern "C"
{
#ifdef USING_COMPRESSION_DEFLATE
    #include <zlib.h>
#endif
#ifdef USING_COMPRESSION_LZ4
    #include <lz4hc.h>
#endif
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

int ImageCompressor::CompressAndSaveImage(const char* outFileName)
{
    //Header
    unsigned char header[782];
    int headerSize = 0x0E;
    header[0x0] = 'G'; header[0x1] = 'P'; header[0x2] = 'I'; //Magic number
    ImageInfo* iinfo = ihand->GetEncodedImage();
    PlanarInfo pinfo = ihand->GeneratePlanarData();
    ColourRGBA8* pal = ihand->GetCurrentPalette();
    unsigned char flags = 0x00;
    if (pinfo.is8BitColour) flags |= 0x08;
    header[0x3] = flags; //Flags
    if (ihand->isTiled)
    {
        *((uint16_t*)(&header[0x4])) = (uint16_t)(ihand->tileSizeX - 1);
        *((uint16_t*)(&header[0x6])) = (uint16_t)(ihand->tileSizeY - 1);
    }
    else
    {
        *((uint16_t*)(&header[0x4])) = (uint16_t)(iinfo->width - 1);
        *((uint16_t*)(&header[0x6])) = (uint16_t)(iinfo->height - 1);
    }
    *((uint16_t*)(&header[0x8])) = (uint16_t)(pinfo.numTiles - 1); //Number of tiles
    *((uint16_t*)(&header[0xA])) = pinfo.planeMask;
    *((uint16_t*)(&header[0xC])) = 0; //Fill this in as we figure out if filtering is useful for each of the planes
    if (pinfo.is8BitColour)
    {
        for (int i = 0; i < pinfo.numColours; i++)
        {
            header[headerSize] = pal[i].R;
            header[headerSize + 1] = pal[i].G;
            header[headerSize + 2] = pal[i].B;
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

    //Make plane filter masks
    int planeFilterMasks[9];
    int sPlaneMask = 0;
    int pMaskCheck = 0x01;
    if (pinfo.planeMask & 0x0100)
    {
        planeFilterMasks[0] = 0x0100;
        sPlaneMask = 1;
    }
    for (int i = 0; i < 8; i++)
    {
        if (pinfo.planeMask & pMaskCheck)
        {
            planeFilterMasks[sPlaneMask] = pMaskCheck;
            sPlaneMask++;
        }
        pMaskCheck <<= 1;
    }
    int planeFilterMask = 0;

    //Compress planes
    int totalHeight = pinfo.planeh * pinfo.numTiles;
    unsigned char* filterTable = new unsigned char[(totalHeight+1)/2];
    unsigned char* fPlane = new unsigned char[pinfo.planeSize];
    unsigned int* rowOccurrence = new unsigned int[totalHeight * 256];
    unsigned char* fRows[16];
    for (int i = 0; i < 16; i++)
    {
        fRows[i] = new unsigned char[pinfo.planew];
    }
    unsigned char* finalPlaneData[9];
    for (int i = 0; i < pinfo.numPlanes; i++)
    {
        bool isFiltered = false;
        unsigned char* curPlane = pinfo.planeData[i];
        uint32_t compressedSizeFiltered = 0;
        uint32_t compressedSizeUnfiltered = 0;

        //Find the best filters for each line heuristically (minimal entropy)
        unsigned char* compressedDataFiltered = new unsigned char[pinfo.planeSize * 2]; //overallocate just in case
        unsigned char* compressedDataUnfiltered = new unsigned char[pinfo.planeSize * 2]; //overallocate just in case
        int pw = pinfo.planew;
        int th = pinfo.planeh;
        int ph = totalHeight;
        int totalOccurrence[256];
        int tempOccurrence[256*16];
        double entropy[16];
        memset(totalOccurrence, 0, sizeof(totalOccurrence));
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < ph; k++)
            {
                //Try all valid filters
                memset(tempOccurrence, 0, sizeof(tempOccurrence));

                if (j > 0) //Correct for multipass operation
                {
                    for (int n = 0; n < 256; n++)
                    {
                        totalOccurrence[n] -= rowOccurrence[256 * k + n];
                    }
                }

                for (int n = 0; n < 16; n++)
                {
                    if (k == 0 && n & 0x8) //Reject NOT filters on the first line, we don't want to bias towards a needlessly complicated filter
                    {
                        entropy[n] = 9999999999999999999999999999.9;
                        continue;
                    }
                    int filtType = n & 0x7;
                    switch (filtType) //Initial validation
                    {
                        case 0: //filt = S[x,y], unconditional
                        case 1: //filt = S[x,y] XOR S[x-1,y], unconditional
                            break;
                        case 2: //filt = S[x,y] XOR S[x,y-1], current y must not be 0
                            if (j < 1)
                            {
                                entropy[n] = 9999999999999999999999999999.9;
                                continue;
                            }
                            break;
                        case 3: //filt = S[x,y] XOR S[x,y-height] (one tile before), current tile must not be 0
                            if (j < th)
                            {
                                entropy[n] = 9999999999999999999999999999.9;
                                continue;
                            }
                            break;
                        case 4: //filt = S[x,y] XOR S[x,y] one plane before, current plane must be 1 or higher
                            if (i < 1)
                            {
                                entropy[n] = 9999999999999999999999999999.9;
                                continue;
                            }
                            break;
                        case 5: //filt = S[x,y] XOR S[x,y] two planes before, current plane must be 2 or higher
                            if (i < 2)
                            {
                                entropy[n] = 9999999999999999999999999999.9;
                                continue;
                            }
                            break;
                        case 6: //filt = S[x,y] XOR S[x,y] three planes before, current plane must be 3 or higher
                            if (i < 3)
                            {
                                entropy[n] = 9999999999999999999999999999.9;
                                continue;
                            }
                            break;
                        case 7: //filt = S[x,y] XOR S[x,y] four planes before, current plane must be 4 or higher
                            if (i < 4)
                            {
                                entropy[n] = 9999999999999999999999999999.9;
                                continue;
                            }
                            break;
                    }

                    //Filter line
                    switch (n)
                    {
                        case 0: //filt = S[x,y]
                            memcpy(fRows[n], &curPlane[k * pw], pw);
                            break;
                        case 1: //filt = S[x,y] XOR S[x-1,y]
                        {
                            unsigned char carry = 0;
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in = curPlane[k * pw + m];
                                unsigned char out = in ^ ((in >> 1) | carry);
                                if (in & 0x01) carry = 0x80;
                                else carry = 0x00;
                                fRows[n][m] = out;
                            }
                        }
                            break;
                        case 2: //filt = S[x,y] XOR S[x,y-1]
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = curPlane[(k - 1) * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = out;
                            }
                            break;
                        case 3: //filt = S[x,y] XOR S[x,y-height] (one tile before)
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = curPlane[(k - th) * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = out;
                            }
                            break;
                        case 4: //filt = S[x,y] XOR S[x,y] one plane before
                        {
                            unsigned char* tPlane = pinfo.planeData[i-1];
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = tPlane[k * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = out;
                            }
                        }
                            break;
                        case 5: //filt = S[x,y] XOR S[x,y] two planes before
                        {
                            unsigned char* tPlane = pinfo.planeData[i-2];
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = tPlane[k * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = out;
                            }
                        }
                            break;
                        case 6: //filt = S[x,y] XOR S[x,y] three planes before
                        {
                            unsigned char* tPlane = pinfo.planeData[i-3];
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = tPlane[k * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = out;
                            }
                        }
                            break;
                        case 7: //filt = S[x,y] XOR S[x,y] four planes before
                        {
                            unsigned char* tPlane = pinfo.planeData[i-4];
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = tPlane[k * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = out;
                            }
                        }
                            break;
                        case 8: //filt = NOT(S[x,y])
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in = curPlane[k * pw + m];
                                fRows[n][m] = ~in;
                            }
                            break;
                        case 9: //filt = NOT(S[x,y] XOR S[x-1,y])
                        {
                            unsigned char carry = 0;
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in = curPlane[k * pw + m];
                                unsigned char out = in ^ ((in >> 1) | carry);
                                if (in & 0x01) carry = 0x80;
                                else carry = 0x00;
                                fRows[n][m] = ~out;
                            }
                        }
                            break;
                        case 10: //filt = NOT(S[x,y] XOR S[x,y-1])
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = curPlane[(k - 1) * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = ~out;
                            }
                            break;
                        case 11: //filt = NOT(S[x,y] XOR S[x,y-height])
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = curPlane[(k - th) * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = ~out;
                            }
                            break;
                        case 12: //filt = NOT(S[x,y] XOR S[x,y] one plane before)
                        {
                            unsigned char* tPlane = pinfo.planeData[i-1];
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = tPlane[k * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = ~out;
                            }
                        }
                            break;
                        case 13: //filt = NOT(S[x,y] XOR S[x,y] two planes before)
                        {
                            unsigned char* tPlane = pinfo.planeData[i-2];
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = tPlane[k * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = ~out;
                            }
                        }
                            break;
                        case 14: //filt = NOT(S[x,y] XOR S[x,y] three planes before)
                        {
                            unsigned char* tPlane = pinfo.planeData[i-3];
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = tPlane[k * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = ~out;
                            }
                        }
                            break;
                        case 15: //filt = NOT(S[x,y] XOR S[x,y] four planes before)
                        {
                            unsigned char* tPlane = pinfo.planeData[i-4];
                            for (int m = 0; m < pw; m++)
                            {
                                unsigned char in1 = curPlane[k * pw + m];
                                unsigned char in2 = tPlane[k * pw + m];
                                unsigned char out = in1 ^ in2;
                                fRows[n][m] = ~out;
                            }
                        }
                            break;
                    }

                    //Count the occurrence of each byte
                    for (int m = 0; m < pw; m++)
                    {
                        tempOccurrence[(n * 256) + fRows[n][m]]++;
                    }

                    //Determine the total entropy of all lines determined so far and this one
                    double tempEntropy = 0.0;
                    if (j > 0) //Correct for multipass operation
                    {
                        for (int m = 0; m < 256; m++)
                        {
                            int occ = totalOccurrence[m] + tempOccurrence[(n * 256) + m];
                            if (occ <= 0) continue;
                            double prob = ((double)occ)/((double)((k+1) * pw));
                            tempEntropy -= prob * log2(prob);
                        }
                    }
                    else
                    {
                        for (int m = 0; m < 256; m++)
                        {
                            int occ = totalOccurrence[m] + tempOccurrence[(n * 256) + m];
                            if (occ <= 0) continue;
                            double prob = ((double)occ)/((double)(ph * pw));
                            tempEntropy -= prob * log2(prob);
                        }
                    }
                    entropy[n] = tempEntropy;
                }

                //Select locally best filter
                int bestFilter = 0;
                double bestEntropy = 999999999999999999999999999999.9;
                for (int n = 0; n < 16; n++)
                {
                    double nextEntropy = entropy[n];
                    if (nextEntropy <= bestEntropy)
                    {
                        if (nextEntropy == bestEntropy) //Tiebreak
                        {
                            if (!(bestFilter & 0x8) && n & 0x8) //Prefer filters that don't need a NOT over ones that do
                            {
                                continue;
                            }
                            switch (bestFilter & 0x7) //Tiebreaking uses nontrivial rules: we prefer 'simpler' filters over complicated ones
                            {
                                case 0: //filt = S[x,y], top priority (memcpy)
                                    break;
                                case 1: //filt = S[x,y] XOR S[x-1,y], lowest priority (complicated af)
                                    bestFilter = n;
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
                            bestFilter = n;
                        }
                    }
                }
                if (bestFilter != 0)
                {
                    isFiltered = true;
                }
                if (j > 0) //Correct for multipass operation
                {
                    unsigned char fte = filterTable[k >> 1];
                    if (k & 0x1)
                    {
                        fte &= 0x0F;
                        fte |= (unsigned char)(bestFilter << 4);
                        filterTable[k >> 1] = fte;
                    }
                    else
                    {
                        fte &= 0xF0;
                        fte |= (unsigned char)bestFilter;
                        filterTable[k >> 1] = fte;
                    }
                }
                else
                {
                    if (k & 0x1) filterTable[k >> 1] |= (unsigned char)(bestFilter << 4);
                    else filterTable[k >> 1] = (unsigned char)bestFilter;
                }
                memcpy(&fPlane[k * pw], fRows[bestFilter], pw);
                for (int n = 0; n < 256; n++)
                {
                    rowOccurrence[256 * k + n] = tempOccurrence[(bestFilter * 256) + n];
                    totalOccurrence[n] += tempOccurrence[(bestFilter * 256) + n];
                }
            }
        }

        //Copy filter table into the compressed data section
        memcpy(compressedDataFiltered, filterTable, (totalHeight+1)/2);
        unsigned char* cptrf = compressedDataFiltered + ((totalHeight+1)/2);
        unsigned char* cptru = compressedDataUnfiltered;

#ifdef USING_COMPRESSION_DEFLATE
        //Compress filtered data using zlib's deflate implementation
        z_stream zStreamF;
        zStreamF.zalloc = Z_NULL;
        zStreamF.zfree = Z_NULL;
        zStreamF.opaque = Z_NULL;
        deflateInit2(&zStreamF, 9, Z_DEFLATED, 15, 8, Z_FILTERED);
        zStreamF.next_in = fPlane;
        zStreamF.avail_in = pinfo.planeSize;
        zStreamF.next_out = cptrf + 4;
        zStreamF.avail_out = pinfo.planeSize * 2 - (4 + ((totalHeight+1)/2));
        zStreamF.data_type = Z_BINARY;
        deflate(&zStreamF, Z_FINISH);
        compressedSizeFiltered = zStreamF.total_out;
        deflateEnd(&zStreamF);
        //Compress unfiltered data using zlib's deflate implementation
        z_stream zStreamU;
        zStreamU.zalloc = Z_NULL;
        zStreamU.zfree = Z_NULL;
        zStreamU.opaque = Z_NULL;
        deflateInit2(&zStreamU, 9, Z_DEFLATED, 15, 8, 0);
        zStreamU.next_in = curPlane;
        zStreamU.avail_in = pinfo.planeSize;
        zStreamU.next_out = cptru + 4;
        zStreamU.avail_out = pinfo.planeSize * 2 - 4;
        zStreamU.data_type = Z_BINARY;
        deflate(&zStreamU, Z_FINISH);
        compressedSizeUnfiltered = zStreamU.total_out;
        deflateEnd(&zStreamU);
#endif
#ifdef USING_COMPRESSION_LZ4
        //Compress filtered data using LZ4
        compressedSizeFiltered = LZ4_compress_HC((char*)fPlane, (char*)(cptrf + 4), pinfo.planeSize, pinfo.planeSize * 2 - (4 + ((totalHeight+1)/2)), LZ4HC_CLEVEL_MAX);
        //Compress unfiltered data using LZ4
        compressedSizeUnfiltered = LZ4_compress_HC((char*)curPlane, (char*)(cptru + 4), pinfo.planeSize, pinfo.planeSize * 2 - 4, LZ4HC_CLEVEL_MAX);
#endif
        //Choose filtered alternative only if 1. filtering was effective for at least one line 2. size of compressed filtered data + filter spec table < size of compressed unfiltered data
        if (isFiltered && (compressedSizeFiltered + ((totalHeight+1)/2)) < compressedSizeUnfiltered)
        {
            *((uint32_t*)(&cptrf[0])) = compressedSizeFiltered;
            printf("Plane %i done, size %i\n", i, compressedSizeFiltered);
            finalPlaneData[i] = compressedDataFiltered;
            planeFilterMask |= planeFilterMasks[i];
            delete[] compressedDataUnfiltered;
        }
        else
        {
            *((uint32_t*)(&cptru[0])) = compressedSizeUnfiltered;
            printf("Plane %i done, size %i\n", i, compressedSizeUnfiltered);
            finalPlaneData[i] = compressedDataUnfiltered;
            delete[] compressedDataFiltered;
        }
    }
    for (int i = 0; i < 16; i++)
    {
        delete[] fRows[i];
    }

    //Save to file
    *((uint16_t*)(&header[0xC])) = (uint16_t)planeFilterMask;
    FILE* ofile = fopen(outFileName, "wb");
    fwrite(header, 1, headerSize, ofile);
    for (int i = 0; i < pinfo.numPlanes; i++)
    {
        unsigned char* curPlane = finalPlaneData[i];
        uint32_t size;
        if (planeFilterMask & planeFilterMasks[i])
        {
            size = *((uint32_t*)(&curPlane[((totalHeight+1)/2)]));
            size += 4 + ((totalHeight+1)/2);
        }
        else
        {
            size = *((uint32_t*)(&curPlane[0]));
            size += 4;
        }
        fwrite(curPlane, 1, size, ofile);
        delete[] finalPlaneData[i];
    }
    fclose(ofile);
    delete[] fPlane;
    delete[] filterTable;
    delete[] rowOccurrence;
    ImageHandler::FreePlanarData(&pinfo);
    return 0;
}
