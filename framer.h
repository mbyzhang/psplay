#ifndef __FRAMER_H__
#define __FRAMER_H__

#include <stdint.h>
#include <stddef.h>

int framer_frame(uint8_t* in, size_t in_len, uint8_t* out, size_t out_len);
int framer_frame_10b(uint8_t* in, size_t in_len, uint16_t* out, size_t out_len);

#endif
