#include "framer.h"
#include "x8b10b.h"

#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <correct.h>
#include <math.h>
#include "utils.h"

#define FRAME_SFD 0x3c

#define BUF_SIZE 256

#define FRAME_HEADER_RS_PARITY_LEN 2

#define FRAMER_RS_PRIM correct_rs_primitive_polynomial_8_4_3_2_0
#define FRAMER_RS_FCR  1
#define FRAMER_RS_GRG  1

static inline uint16_t rev10b(uint16_t in) {
    const uint16_t lut[] = {
        0b00000, 0b10000, 0b01000, 0b11000, 0b00100, 0b10100, 0b01100, 0b11100,
        0b00010, 0b10010, 0b01010, 0b11010, 0b00110, 0b10110, 0b01110, 0b11110,
        0b00001, 0b10001, 0b01001, 0b11001, 0b00101, 0b10101, 0b01101, 0b11101,
        0b00011, 0b10011, 0b01011, 0b11011, 0b00111, 0b10111, 0b01111, 0b11111,
    };

    return (lut[in & BIT_ONES(5)] << 5) | (lut[in >> 5]);
}

static inline ssize_t bitstream_write_8b10b(bitstream_t* s, uint8_t data, int* rd, int ctrl) {
    uint16_t t;
    x8b10b_enc(data, &t, ctrl, rd);
    t = rev10b(t);
    return bitstream_write(s, &t, 10);
}

static inline ssize_t bitstream_write_8b10b_chunk(bitstream_t* s, uint8_t* data, size_t len, int* rd) {
    if (bitstream_remaining_cap(s) < len * 10) return -E2BIG;
    ssize_t bits_written = 0;
    for (int i = 0; i < len; ++i) {
        ssize_t ret = bitstream_write_8b10b(s, data[i], rd, false);
        if (ret < 0) return ret;
        bits_written += ret;
    }
    return bits_written;
}

int framer_init(framer_t* framer, double payload_parity_len_ratio, framer_format_t format, size_t preamble_length, int m_exp) {
    framer->payload_parity_len_ratio = payload_parity_len_ratio;
    framer->rs_header = correct_reed_solomon_create(FRAMER_RS_PRIM, FRAMER_RS_FCR, FRAMER_RS_GRG, FRAME_HEADER_RS_PARITY_LEN);
    framer->format = format;
    framer->preamble_length = preamble_length;
    framer->m_exp = m_exp;
    if (framer->rs_header == NULL) return -1;
    return 0;
}

int framer_frame(framer_t* framer, uint8_t* in, size_t in_len, bitstream_t* s) {
    int ret = 0;
    int rd = -1;

    uint8_t header[] = { in_len };
    uint8_t header_encoded[sizeof(header) + FRAME_HEADER_RS_PARITY_LEN];
    uint8_t payload_encoded[BUF_SIZE];

    size_t payload_parity_len = ceil(framer->payload_parity_len_ratio * in_len);
    size_t len = framer->preamble_length * (framer->m_exp * 2) + sizeof(header_encoded) * 10;

    switch (framer->format) {
    case FRAMER_FORMAT_STANDARD:
        len += (in_len + payload_parity_len) * 10;
        break;
    case FRAMER_FORMAT_RAW_PAYLOAD:
        len += in_len * 8;
        break;
    }

    if (in_len > FRAME_MAX_PAYLOAD_SIZE) return -EMSGSIZE;
    if (framer->format == FRAMER_FORMAT_STANDARD && in_len + payload_parity_len > 255) return -EMSGSIZE;
    if (bitstream_remaining_cap(s) < len) return -E2BIG;

    // preamble
    for (int i = 0; i < framer->preamble_length; i++) {
        CHECK_ERROR_LT0(bitstream_write_n(s, 1 << framer->m_exp, framer->m_exp * 2));
    }

    // start-of-frame delimiter
    CHECK_ERROR_LT0(bitstream_write_8b10b(s, FRAME_SFD, &rd, true));

    // header
    CHECK_ERROR_LT0(correct_reed_solomon_encode(framer->rs_header, header, sizeof(header), header_encoded));
    CHECK_ERROR_LT0(bitstream_write_8b10b_chunk(s, header_encoded, sizeof(header_encoded), &rd));
    
    // payload
    correct_reed_solomon* rs_payload = correct_reed_solomon_create(FRAMER_RS_PRIM, FRAMER_RS_FCR, FRAMER_RS_GRG, payload_parity_len);

    switch (framer->format) {
    case FRAMER_FORMAT_STANDARD:
        CHECK_ERROR_LT0(correct_reed_solomon_encode(rs_payload, in, in_len, payload_encoded));
        CHECK_ERROR_LT0(bitstream_write_8b10b_chunk(s, payload_encoded, in_len + payload_parity_len, &rd));
        break;
    case FRAMER_FORMAT_RAW_PAYLOAD:
        CHECK_ERROR_LT0(bitstream_write(s, in, in_len * 8));
        break;
    }

    ret = len;
    correct_reed_solomon_destroy(rs_payload);
    return ret;
}

void framer_destory(framer_t* framer) {
    correct_reed_solomon_destroy(framer->rs_header);
}
