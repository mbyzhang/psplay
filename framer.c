#include "framer.h"
#include "x8b10b.h"

#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include "utils.h"

#define FRAME_SFD 0x3c
#define FRAME_EFD 0x1c

static inline uint16_t rev10b(uint16_t in) {
    const uint16_t lut[] = {
        0b00000, 0b10000, 0b01000, 0b11000, 0b00100, 0b10100, 0b01100, 0b11100,
        0b00010, 0b10010, 0b01010, 0b11010, 0b00110, 0b10110, 0b01110, 0b11110,
        0b00001, 0b10001, 0b01001, 0b11001, 0b00101, 0b10101, 0b01101, 0b11101,
        0b00011, 0b10011, 0b01011, 0b11011, 0b00111, 0b10111, 0b01111, 0b11111,
    };

    return (lut[in & BIT_ONES(5)] << 5) | (lut[in >> 5]);
}

#define FRAMER_X8B10B_WRITE(stream, data, rd, ret, ctrl, fail) do { \
    uint16_t t;                                         \
    x8b10b_enc(data, &t, ctrl, &rd);                       \
    t = rev10b(t);                                      \
    ret = bitstream_write(stream, &t, 10);              \
    if (ret < 0) goto fail;                             \
} while (0);

int framer_frame(uint8_t* in, size_t in_len, bitstream_t* s) {
    int ret;
    int rd = -1;

    if (s->cap < (in_len + 3) * 10) return -E2BIG;

    ret = bitstream_write_n(s, 0b1010101010, 10);
    if (ret < 0) goto fail;

    FRAMER_X8B10B_WRITE(s, FRAME_SFD, rd, ret, true, fail);

    for (int i = 0; i < in_len; ++i) {
        FRAMER_X8B10B_WRITE(s, in[i], rd, ret, false, fail);
    }

    FRAMER_X8B10B_WRITE(s, FRAME_EFD, rd, ret, true, fail);
    
fail:
    return ret;
}
