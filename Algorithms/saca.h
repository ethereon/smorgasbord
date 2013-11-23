//
//  Linear Suffix Array Consturction Algorithm
//  Copyright (c) 2012 Saumitro Dasgupta. All rights reserved.
//  Released under the MIT license.
//

#pragma once

#include <stdint.h>

//
//  Construct the Suffix Array
//  
//  s   :   The input string.
//  SA  :   The suffix array. Must be a valid array of n elements.
//  n   :   The number of bytes in s.
//  k   :   The number of alphabets comprising the input string.
//  cs  :   The word size for the input string. Use 1 for a byte-stream.
//
void construct_suffix_array(const char* s, int32_t* SA, int n, int k, int cs);
