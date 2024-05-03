//Wrapper for Trixter's 8086 assembly LZ4 decompression routine

#pragma once

//source must point to the 32-bit length of the block followed by the LZ4-compressed data
unsigned int LZ4Decompress(__far unsigned char* dest, __far const unsigned char* source);
