#ifndef __FRAMER_H__
#define __FRAMER_H__

#include <stdint.h>
#include <stddef.h>
#include <correct.h>
#include "bitstream.h"

#define FRAME_MAX_LENGTH_BITS (256 * 8)

typedef struct {
    correct_reed_solomon* rs_header;
    double payload_parity_len_ratio;
} framer_t;

int framer_init(framer_t* framer, double payload_parity_ratio);
int framer_frame(framer_t* framer, uint8_t* in, size_t in_len, bitstream_t* s);
void framer_destory(framer_t* framer);

#endif
