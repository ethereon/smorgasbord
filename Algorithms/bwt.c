//
//  Burrows–Wheeler Transform
//  Copyright (c) 2012 Saumitro Dasgupta. All rights reserved.
//  Released under the MIT license.
//

#include "bwt.h"
#include "saca.h"
#include <stdlib.h>
#include <stdint.h>

void* allocate_bwt_work_buffer(int n)
{
    return malloc(sizeof(int)*n);
}

void free_bwt_work_buffer(void* wb)
{
    free(wb);
}

int bwt(const char* data, char* out, int n, void* wb)
{
    //Construct the suffix array for the data block
    int32_t* SA = (int32_t*)wb;
    construct_suffix_array(data, SA, n, 256, 1);
    
    //Compute the forward BWT
    int zeroIdx = -1;
    out[0] = data[n-1];
    for(int i=0, j=1; i<n; ++i)
    {
        if(SA[i])
        {
            out[j++] = data[SA[i]-1];
        }
        else
        {
            zeroIdx = i+1;
        }
    }
    
    return zeroIdx;
}

void inverse_bwt(const char* data, char* out, int n, int zeroIdx, void* wb)
{
    
    uint32_t pos[256] = {0};
    uint32_t* lut = (uint32_t*)wb;
    
    uint32_t i, t, offset;
    
    //Get character counts
    for(i=0; i<n; ++i)
    {
        ++pos[data[i]];
    }
    
    //Get the head index for each character's bucket
    for(i=0, offset=0; i<256; ++i)
    {
        t = pos[i];
        pos[i] = offset;
        offset += t;
    }
    
    //Create an index LUT for the data
    for(i=0; i<zeroIdx; ++i)
    {
        lut[pos[data[i]]++] = i;
    }        
    while(i<n)
    {
        lut[pos[data[i]]++] = i+1;
        ++i;
    }
    
    //Generate the original string
    for(i=0, t=zeroIdx; i<n; ++i)
    {
        //Binary search to locate the smallest k such that pos[k]>=t.
        int imax = 255, imin = 0;
        while(imax>=imin)
        {
            int imid = (imax+imin)>>1;
            
            if(pos[imid]<t)
            {
                imin = imid+1;
            }
            else
            {
                imax = imid-1;
            }
        }
        
        out[i] = (char)(imax+1);
        t = lut[t-1];
    }
}
