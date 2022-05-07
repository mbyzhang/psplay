#ifndef __FRAMER_H__
#define __FRAMER_H__

#include <stdint.h>
#include <stddef.h>
#include <correct.h>
#include "bitstream.h"

#define FRAME_MAX_LENGTH_BITS 2620
#define FRAME_MAX_PAYLOAD_SIZE 255

#define FRAME_FLAG_NO_PAYLOAD_LINE_CODING   1
#define FRAME_FLAG_NO_HEADER                2

typedef struct {
    correct_reed_solomon* rs_header;
    double payload_parity_len_ratio;
    int flags;
    size_t preamble_length;
    int m_exp;
} framer_t;

int framer_init(framer_t* framer, double payload_parity_ratio, int flags, size_t preamble_length, int m_exp);
int framer_frame(framer_t* framer, uint8_t* in, size_t in_len, bitstream_t* s);
void framer_destory(framer_t* framer);

#endif
