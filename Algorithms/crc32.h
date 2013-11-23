//
//  Fast 32-bit Cyclic Redundancy Check
//  Copyright (c) 2012 Saumitro Dasgupta. All rights reserved.
//

#pragma once

#include <stdint.h>

void initialize_crc32_lookup_table(void);

uint32_t calculate_crc32_for_data(uint8_t* data, uint64_t nBytes);

