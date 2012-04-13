//
//  Burrows–Wheeler Transform
//  Copyright (c) 2012 Saumitro Dasgupta. All rights reserved.
//  Released under the MIT license.
//

#pragma once

// Allocates a reusable buffer used by the forward/reverse bwt functions
void* allocate_bwt_work_buffer(int n);

void free_bwt_work_buffer(void* wb);

//
//  Compute the Burrows-Wheeler Transform
//
//  data    : The input data.
//  out     : The transformed output.
//  n       : The size of the input data in bytes
//  wb      : A valid work buffer.
//
//  Returns : The null index if succesful, -1 otherwise.
//
int bwt(const char* data, char* out, int n, void* wb);

//
//  Inverse Burrows-Wheeler Transform
//
//  data    : Th bwt-transformed input data.
//  out     : The output of the inversion. Can be the same as data.
//  n       : The size of the input data in bytes.
//  zeroIdx : The null index returned by the original bwt transform.
//  wb      : A valid work buffer.
//
void inverse_bwt(const char* data, char* out, int n, int zeroIdx, void* wb);
