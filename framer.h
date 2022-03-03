#ifndef __FRAMER_H__
#define __FRAMER_H__

#include <stdint.h>
#include <stddef.h>
#include <correct.h>
#include "bitstream.h"

#define FRAME_MAX_LENGTH_BITS (256 * 8)

typedef enum {
    FRAMER_FORMAT_STANDARD = 0,
    FRAMER_FORMAT_RAW_PAYLOAD = 1, // raw non-line-coded and non-ECC-protected payload
} framer_format_t;

typedef struct {
    correct_reed_solomon* rs_header;
    double payload_parity_len_ratio;
    framer_format_t format;
} framer_t;

int framer_init(framer_t* framer, double payload_parity_ratio, framer_format_t format);
int framer_frame(framer_t* framer, uint8_t* in, size_t in_len, bitstream_t* s);
void framer_destory(framer_t* framer);

#endif
