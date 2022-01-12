#include "framer.h"
#include "x8b10b.h"

#include <errno.h>

#define FRAME_SFD 0x3c
#define FRAME_EFD 0x1c

#define BUF_SIZE 1024

int framer_frame_10b(uint8_t* in, size_t in_len, uint16_t* out, size_t out_len) {
    int rd = -1;

    if (out_len < in_len + 3) return -E2BIG;

    // preamble
    out[0] = 0b1010101010;

    // start-of-frame delimiter
    x8b10b_enc(FRAME_SFD, &out[1], 1, &rd);

    for (int i = 0; i < in_len; i++) {
        x8b10b_enc(in[i], out + i + 2, 0, &rd);
    }

    // end-of-frame delimiter
    x8b10b_enc(FRAME_EFD, &out[in_len + 2], 1, &rd);

    return in_len + 3;
}

// Adapted from https://github.com/df9ry/x8b10b/blob/master/src/x8b10b.c
static int framer_pack_10b(uint16_t* in, size_t in_len, uint8_t* out, size_t out_len) {
    union {
		uint16_t reg;
		uint8_t oct[2];
	} u;

    u.reg = 0;

    int n_bits = 0;
    int n_bits_total = 0;

    while (in_len > 0) {
        uint16_t d = (*in);
        ++in;
        --in_len;
        u.reg |= d << (6 - n_bits); n_bits += 10;
        while (n_bits >= 8) {
            if (out_len == 0) return -E2BIG;
            (*out) = u.oct[1];
            u.reg <<= 8;
            n_bits -= 8;
            ++out;
            --out_len;
            n_bits_total += 8;
        }
    }
    if (n_bits > 0) {
        if (out_len == 0) return -E2BIG;
        u.reg = u.reg | 0x55 << (8 - n_bits);
        (*out) = u.oct[1];
        n_bits_total += n_bits;
    }
    return n_bits_total;
}

int framer_frame(uint8_t* in, size_t in_len, uint8_t* out, size_t out_len) {
    int ret;
    uint16_t buf[BUF_SIZE];

    ret = framer_frame_10b(in, in_len, buf, BUF_SIZE);
    if (ret < 0) return ret;

    ret = framer_pack_10b(buf, ret, out, out_len);

    return ret;
}
