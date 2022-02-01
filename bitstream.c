#include "bitstream.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"

static ssize_t bitwise_copy(uint8_t* src, size_t src_offset, size_t src_cap, uint8_t* dst, size_t dst_offset, size_t dst_cap, size_t count) {
    size_t src_remaining = MIN(count, src_cap - src_offset);
    ssize_t total_bits_copied = 0;
    if (dst_cap - dst_offset < count) return -E2BIG;

    union {
        uint16_t reg;
        uint8_t oct[2];
    } u;

    u.reg = 0;
    size_t u_len = 0;

    while (src_remaining > 0) {
        size_t n_bits_produced = MIN(src_remaining, 8 - src_offset % 8);
        uint8_t bits_produced = (src[src_offset / 8] >> (src_offset % 8)) & BIT_ONES(n_bits_produced);
        src_offset += n_bits_produced;
        src_remaining -= n_bits_produced;

        u.reg |= bits_produced << u_len;
        u_len += n_bits_produced;
        
        while (u_len > 0) {
            size_t n_bits_consumed = MIN(u_len, 8U - dst_offset % 8U);
            uint8_t bits_consumed = u.reg & BIT_ONES(n_bits_consumed);

            size_t dst_offset_byte = dst_offset / 8;
            size_t dst_bit_pos = dst_offset % 8U;
            dst[dst_offset_byte] = (bits_consumed << dst_bit_pos) | (dst[dst_offset_byte] & BIT_ONES(dst_bit_pos));

            dst_offset += n_bits_consumed;

            u.reg >>= n_bits_consumed;
            u_len -= n_bits_consumed;
            total_bits_copied += n_bits_consumed;
        }
    }

    return total_bits_copied;
}

int bitstream_init(bitstream_t* stream, size_t cap) {
    stream->cap = cap;
    stream->data = calloc((cap + 7) / 8, 1);
    if (stream->data == NULL) {
        return -errno;
    }
    stream->len = 0;
    stream->pos_read = 0;
    stream->pos_write = 0;
    return 0;
}

void bitstream_destroy(bitstream_t* stream) {
    free(stream->data);
}

ssize_t bitstream_write(bitstream_t* stream, void* data, size_t len) {
    ssize_t bits_written = bitwise_copy(data, 0, len, stream->data, stream->pos_write, stream->cap, len);
    if (bits_written < 0) goto fail;
    if (stream->len + bits_written > stream->cap) return -E2BIG;
    stream->pos_write += bits_written;
    stream->len += bits_written;
fail:
    return bits_written;
}

ssize_t bitstream_write_n(bitstream_t* stream, uint64_t n, size_t len) {
    len = MIN(64, len);
    return bitstream_write(stream, &n, len);
}

ssize_t bitstream_read(bitstream_t* stream, void* buf, size_t count) {
    ssize_t bits_read = bitwise_copy(stream->data, stream->pos_read, stream->len, buf, 0, count, count);
    if (bits_read < 0) goto fail;
    stream->pos_read += bits_read;
fail:
    return bits_read;
}

size_t bitstream_seek(bitstream_t* stream, size_t pos) {
    stream->pos_read = pos;
    if (stream->pos_read > stream->len) {
        stream->pos_read = stream->len;
    }
    return stream->pos_read;
}

void bitstream_dump(bitstream_t* stream) {
    if (stream->len == 0) {
        printf("<empty>");
    }

    for (size_t i = 0; i < stream->len; ++i) {
        printf("%d", 1U & (stream->data[i / 8] >> (i % 8)));
    }

    printf("\n");
}

ssize_t bitstream_write_str(bitstream_t* stream, const char* str) {
    ssize_t total_bits_written = 0;
    for (int i = 0; i < strlen(str); i++) {
        uint8_t bit = (str[i] != '0') & 1;
        ssize_t bits_written = bitstream_write(stream, &bit, 1);
        if (bits_written < 0) return bits_written;
        total_bits_written += bits_written;
    }
    return total_bits_written;
}
