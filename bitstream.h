#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct {
    uint8_t* data;
    size_t cap;
    size_t len;
    size_t pos_read;
    size_t pos_write;
} bitstream_t;

int bitstream_init(bitstream_t* stream, size_t cap);
ssize_t bitstream_write(bitstream_t* stream, void* data, size_t len);
ssize_t bitstream_write_str(bitstream_t* stream, const char* str);
ssize_t bitstream_read(bitstream_t* stream, void* buf, size_t count);
size_t bitstream_seek(bitstream_t* stream, size_t pos);
void bitstream_dump(bitstream_t* stream);
void bitstream_destroy(bitstream_t* stream);

#ifdef __cplusplus
}
#endif 

#endif
