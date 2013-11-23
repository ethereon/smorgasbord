//
//  Linear Suffix Array Consturction Algorithm
//  Copyright (c) 2012 Saumitro Dasgupta. All rights reserved.
//  Released under the MIT License.
//
//  Implementation Notes
//  ---------------------
//
//  This implementation is based on the induced sort algorithm, as published in
//  the 2008 paper "Two Efficient Algorithms for Linear Suffix Array Construction" by
//  Nong, Zhang and Chan. The occasional reference to a definition or lemma in the
//  code below refer to this paper.
//
//  This implementation, unlike the reference, does not require the string to be
//  terminated by a sentinel (a unique lexicographically smallest character).
//  It can correctly handle any arbitrary binary string, including null characters.
//

#include "saca.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

unsigned char mask[]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
#define tget(i) ( (t[(i)/8]&mask[(i)%8]) ? 1 : 0 )
#define tset(i, b) t[(i)/8]=(b) ? (mask[(i)%8]|t[(i)/8]) : ((~mask[(i)%8])&t[(i)/8])
#define chr(i) (cs==sizeof(int)?((int*)s)[i]:((unsigned char *)s)[i])

//Determine if the character at i is a leftmost S-Type (Def 2.1 in the paper)
#define isLMS(i) (i>0 && tget(i) && !tget(i-1))

void get_buckets(const char* s, int* bucket, int n, int k, int cs, bool tail)
{
    int sum=0, i;

    //Clear all buckets
    memset(bucket, 0, sizeof(int)*k);

    //Count the occurrences of each character
    for(i=0; i<n; ++i) ++bucket[chr(i)];

    if(tail)
    {
        //Set the tail index of each bucket
        for(i=0; i<k; ++i)
        {
            sum+=bucket[i];
            bucket[i]=sum-1;
        }
    }
    else
    {
        //Set the head index of each bucket
        for(i=0; i<k; ++i)
        {
            sum+=bucket[i];
            bucket[i]=sum-bucket[i];
      }
    }
}

void induce_sa(const char* t, int *SA, const char* s, int* bucket, int n, int k, int cs)
{
    int i, j;

    //Scan left to right in the suffix array, and fill in the L-type prefixes
    get_buckets(s, bucket, n, k, cs, false);

    //If our buffer had a sentinel character at (n-1), SA[0] would be the (n-1)th index, and
    //as a result (n-2) would be picked up as an L-type prefix. Since we don't have
    //a sentinel, compensate for it here.
    SA[bucket[chr(n-1)]] = n-1; ++bucket[chr(n-1)];
    for(i=0; i<n; ++i)
    {
        j = SA[i]-1;
        if(j>=0 && !tget(j))
        {
            //Fill in the value and shift the bucket head right.
            SA[bucket[chr(j)]++]=j;
        }
    }

    //Scan right to left in the suffix array, and fill in the S-type characters
    get_buckets(s, bucket, n, k, cs, true);
    for(i=n-1; i>=0; --i)
    {
        j = SA[i]-1;
        if(j>=0 && tget(j))
        {
            //Fill in the value and shift the bucket tail left.
            SA[bucket[chr(j)]--]=j;
        }
    }
}

void construct_suffix_array(const char* s, int32_t* SA, int n, int k, int cs)
{
    int i, j;

    //First, we create the bitset t which classifies each character
    //as either L or S type.
    char *t = malloc(n/8+1);

    //Classify the last character as L type, since the last character
    //is an implicit sentinel which is lexicographically the smallest character.
    tset(n-1, 0);

    for(i=n-2; i>=0; --i)
    {
        tset(i, (chr(i)<chr(i+1) || (chr(i)==chr(i+1) && tget(i+1)==1))?1:0);
    }

    //Stage 1: Reduce by problem by at least 1/2 (divide-and-conquer)

    //Allocate the buckets for k symbols
    int* bucket = malloc(sizeof(int)*k);

    get_buckets(s, bucket, n, k, cs, true);

    //Clear the suffix array
    for(i=0; i<n; ++i) SA[i]=-1;

    //Find all LMS substrings (i=0 cannot be an LMS)
    for(i=1; i<n;++i)
    {
        if(isLMS(i)) SA[bucket[chr(i)]--]=i;
    }

    //Induce-sort the LMS substrings
    induce_sa(t, SA, s, bucket, n, k, cs);
    free(bucket);

    //Isolate the sorted LMS substrings (def 2.2) into the upper part of SA.
    //Lemma 2.1 guarantees that n1 < floor(n/2).
    int n1=0;
    for(i=0; i<n; ++i)
        if(isLMS(SA[i]))
            SA[n1++]=SA[i];

    //Initialize SA[n1..n) for naming.
    for(i=n1; i<n; ++i) SA[i]=-1;

    //Find the lexicographic names for all substrings.
    int name=0, prev=-1;
    for(i=0; i<n1; ++i)
    {
        int idx = SA[i];
        bool diff = false;

        //Compare the substrings, as described in (def 2.3).
        for(j=0; j<n; ++j)
        {
            if(prev==-1 ||
               idx+j==n ||
               prev+j==n ||
               chr(idx+j)!=chr(prev+j) ||
               tget(idx+j)!=tget(prev+j))
            {
                //The substring starting at this LMS (idx) is distinct
                //from the previous one.
                diff = true;
                break;
            }
            else if(j>0 &&
                ( isLMS(idx+j) ||
                    isLMS(prev+j) ||
                    (idx+j)==(n-1) ||
                    (prev+j)==(n-1) ))
            {
                //The substring starting at this LMS (idx) is identical
                //to the previous one.
                break;
            }
        }

        if(diff)
        {
            ++name;
            prev=idx;
        }

        //Insert the names in correct relative-order.
        SA[n1+(idx/2)] = name-1;
    }

    //Compact the relatively ordered names at the end of the array.
    //This lower portion of the array will become s1.
    for(i=n-1, j=n-1; i>=n1; --i)
        if(SA[i]>=0)
            SA[j--]=SA[i];

    //We have now generated s1 (step 4 in figure 1)
    int* s1 = SA+n-n1;

    //Stage 2: Solve the reduced problem
    if(name<n1)
    {
        //Recursively solve
        construct_suffix_array((char*)s1, SA, n1, name, sizeof(int));
    }
    else
    {
        for(i=0; i<n1; ++i) SA[s1[i]]=i;
    }

    //Stage 3: Induce the final SA
    bucket = malloc(sizeof(int)*k);
    get_buckets(s, bucket, n, k, cs, true);
    for(i=1, j=0; i<n; ++i)
    {
        if(isLMS(i)) s1[j++]=i;
    }

    for(i=0; i<n1; ++i) SA[i]=s1[SA[i]];
    for(i=n1; i<n; ++i) SA[i]=-1;
    for(i=n1-1; i>=0; --i)
    {
        j=SA[i];
        SA[i]=-1;
        SA[bucket[chr(j)]--] = j;
    }

    induce_sa(t, SA, s, bucket, n, k, cs);

    free(bucket);
    free(t);
}
