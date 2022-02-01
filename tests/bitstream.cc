#include <gtest/gtest.h>
#include "../bitstream.h"

TEST(BitStreamTest, Lifecycle) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 8), 0);
  ASSERT_EQ(s.cap, 8);
  bitstream_destroy(&s);
}

TEST(BitStreamTest, WriteOnceSimple) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 16), 0);

  uint8_t data[] = { 0b10110111, 0b10101010 };
  ASSERT_EQ(bitstream_write(&s, data, 16), 16);
  
  ASSERT_EQ(s.len, 16);
  ASSERT_EQ(s.data[0], data[0]);
  ASSERT_EQ(s.data[1], data[1]);
  ASSERT_EQ(s.pos_read, 0);
  ASSERT_EQ(s.pos_write, 16);

  bitstream_destroy(&s);
}

TEST(BitStreamTest, WriteTwiceSimple) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 16), 0);

  uint8_t data[] = { 0b10110111, 0b10101010 };
  ASSERT_EQ(bitstream_write(&s, data, 8), 8);
  ASSERT_EQ(bitstream_write(&s, data + 1, 8), 8);

  ASSERT_EQ(s.len, 16);
  ASSERT_EQ(s.data[0], data[0]);
  ASSERT_EQ(s.data[1], data[1]);
  ASSERT_EQ(s.pos_read, 0);
  ASSERT_EQ(s.pos_write, 16);

  bitstream_destroy(&s);
}

TEST(BitStreamTest, WriteTwiceNotAligned) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 16), 0);

  uint8_t data[] = { 0b10110111, 0b10101010 };
  ASSERT_EQ(bitstream_write(&s, data, 3), 3);
  ASSERT_EQ(bitstream_write(&s, data + 1, 8), 8);

  ASSERT_EQ(s.len, 11);
  ASSERT_EQ(s.data[0], 0b01010111);
  ASSERT_EQ(s.data[1], 0b101);

  bitstream_destroy(&s);
}

TEST(BitStreamTest, WriteBeyondCapacity) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 3), 0);

  uint8_t data[] = { 0b10110111 };
  ASSERT_EQ(bitstream_write(&s, data, 8), -E2BIG);

  ASSERT_EQ(s.len, 0);

  bitstream_destroy(&s);
}

TEST(BitStreamTest, WriteString) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 24), 0);
  ASSERT_EQ(bitstream_write_str(&s, "010110111"), 9);
  ASSERT_EQ(s.data[0], 0b11011010);
  ASSERT_EQ(s.data[1], 0b1);
}

TEST(BitStreamTest, ReadOnceSimple) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 16), 0);

  uint8_t data[] = { 0b10110111, 0b10101010 };
  ASSERT_EQ(bitstream_write(&s, data, 16), 16);

  uint8_t buf[2];
  ASSERT_EQ(bitstream_read(&s, buf, 16), 16);
  ASSERT_TRUE(memcmp(data, buf, 2) == 0);

  ASSERT_EQ(s.pos_read, 16);
  bitstream_destroy(&s);
}

TEST(BitStreamTest, ReadTwiceSimple) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 16), 0);

  uint8_t data[] = { 0b10110111, 0b10101010 };
  ASSERT_EQ(bitstream_write(&s, data, 16), 16);

  uint8_t buf[2];
  ASSERT_EQ(bitstream_read(&s, buf, 8), 8);
  ASSERT_EQ(bitstream_read(&s, buf + 1, 8), 8);

  ASSERT_TRUE(memcmp(data, buf, 2) == 0);

  ASSERT_EQ(s.pos_read, 16);
  bitstream_destroy(&s);
}

TEST(BitStreamTest, ReadTwiceNotAligned) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 16), 0);

  uint8_t data[] = { 0b10110111, 0b10101010 };
  ASSERT_EQ(bitstream_write(&s, data, 16), 16);

  uint8_t buf[2];
  ASSERT_EQ(bitstream_read(&s, buf, 3), 3);
  ASSERT_EQ(bitstream_read(&s, buf + 1, 8), 8);

  ASSERT_EQ(buf[0], 0b111);
  ASSERT_EQ(buf[1], 0b01010110);

  ASSERT_EQ(s.pos_read, 11);
  bitstream_destroy(&s);
}

TEST(BitStreamTest, ReadBeyondLength) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 24), 0);

  uint8_t data[] = { 0b10110111, 0b10101010 };
  ASSERT_EQ(bitstream_write(&s, data, 16), 16);

  uint8_t buf[3];
  ASSERT_EQ(bitstream_read(&s, buf, 24), 16);

  ASSERT_TRUE(memcmp(data, buf, 2) == 0);

  ASSERT_EQ(s.pos_read, 16);
  bitstream_destroy(&s);
}

TEST(BitStreamTest, ReadEOF) {
  bitstream_t s;
  ASSERT_EQ(bitstream_init(&s, 24), 0);

  uint8_t data[] = { 0b10110111, 0b10101010 };
  ASSERT_EQ(bitstream_write(&s, data, 16), 16);

  uint8_t buf[3];
  ASSERT_EQ(bitstream_read(&s, buf, 16), 16);
  ASSERT_EQ(bitstream_read(&s, buf + 2, 8), 0);

  ASSERT_EQ(s.pos_read, 16);
  bitstream_destroy(&s);
}
