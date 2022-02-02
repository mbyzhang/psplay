#ifndef __FRAMER_H__
#define __FRAMER_H__

#include <stdint.h>
#include <stddef.h>
#include "bitstream.h"

#define FRAME_LENGTH_BITS(payload_len_bytes) (((payload_len_bytes) + 3) * 10)

int framer_frame(uint8_t* in, size_t in_len, bitstream_t* s);

#endif
