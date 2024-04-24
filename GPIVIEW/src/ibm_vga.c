//VGA interface
//Maxim Hoxha 2024

#include "ibm_vga.h"

unsigned int displayRegionStartAddress = 0;
unsigned short displayLines = 480;
unsigned short totalLines = 480;
unsigned short displayX = 640;
unsigned short displayPitch = 40;
