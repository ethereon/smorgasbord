//
//  Fast 32-bit Cyclic Redundancy Check
//  Copyright (c) 2012 Saumitro Dasgupta. All rights reserved.
//

#include "crc32.h"
#include <stdlib.h>

#define CRC32_POLYNOMIAL 0xEDB88320

static uint32_t crc32_lut[256];

void initialize_crc32_lookup_table()
{    
    for(int i=0; i<256; ++i)
    {
        uint32_t v = i;
        for(int j=0; j<8; ++j)
        {
            v = (v>>1) ^ (CRC32_POLYNOMIAL & ~((v&1)-1));
        }
        crc32_lut[i] = v;
    }    
}

//Calculate CRC32 using the Sarwate Table Lookup Algorithm
uint32_t calculate_crc32_for_data(uint8_t* data, uint64_t nBytes)
{
    //As per convention, we use 0xFFFFFFFF as the initial value and
    //the final mask. This makes sure that there is no ambiguity between
    //the checksum of a single byte with value zero and an empty stream.
    uint32_t checksum = 0xFFFFFFFF;
    for(uint64_t i=0; i<nBytes; ++i)
    {
        checksum = crc32_lut[((uint8_t)checksum)^data[i]] ^ (checksum>>8);
    }
    return checksum^0xFFFFFFFF;
}