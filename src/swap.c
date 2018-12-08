//
//  src/swap.c
//  tbd
//
//  Created by inoahdev on 11/20/18.
//  Copyright © 2018 inoahdev. All rights reserved.
//

#include "swap.h"

uint32_t swap_uint32(uint32_t num) {
    num = ((num >> 8) & 0x00ff00ff)  | ((num << 8) & 0xff00ff00);
    num = ((num >> 16) & 0x0000ffff) | ((num << 16) & 0xffff0000);

    return num;
}

uint64_t swap_uint64(uint64_t num) {
    num =
        (num & 0x00000000ffffffffULL) << 32 |
        (num & 0xffffffff00000000ULL) >> 32;

    num =
        (num & 0x0000ffff0000ffffULL) << 16 |
        (num & 0xffff0000ffff0000ULL) >> 16;

    num =
        (num & 0x00ff00ff00ff00ffULL) << 8 |
        (num & 0xff00ff00ff00ff00ULL) >> 8;

    return num;
}

