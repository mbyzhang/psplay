#include <gtest/gtest.h>
#include <stdint.h>
#include <thread>
#include "../ping_pong_buf.h"

#define BUF_SIZE 64

TEST(PingPongBufTest, BasicReadWrite) {
    ppbuf_t ppbuf;
    ASSERT_EQ(ppbuf_init(&ppbuf, BUF_SIZE), 0);

    ASSERT_TRUE(ppbuf_write_available(&ppbuf));
    ASSERT_FALSE(ppbuf_read_available(&ppbuf));

    void* buf1 = ppbuf_buf_write_begin(&ppbuf);

    ASSERT_TRUE(ppbuf_write_available(&ppbuf));
    ASSERT_FALSE(ppbuf_read_available(&ppbuf));

    ppbuf_buf_write_done(&ppbuf);

    ASSERT_TRUE(ppbuf_write_available(&ppbuf));
    ASSERT_TRUE(ppbuf_read_available(&ppbuf));

    void* buf2 = ppbuf_buf_read_begin(&ppbuf);

    ASSERT_TRUE(ppbuf_write_available(&ppbuf));
    ASSERT_TRUE(ppbuf_read_available(&ppbuf));

    ppbuf_buf_read_done(&ppbuf);

    ASSERT_TRUE(ppbuf_write_available(&ppbuf));
    ASSERT_FALSE(ppbuf_read_available(&ppbuf));

    ASSERT_EQ(buf1, buf2);

    ppbuf_destroy(&ppbuf);
}

TEST(PingPongBufTest, WriteTwiceReadTwice) {
    ppbuf_t ppbuf;
    ASSERT_EQ(ppbuf_init(&ppbuf, BUF_SIZE), 0);

    void* buf1 = ppbuf_buf_write_begin(&ppbuf);
    ppbuf_buf_write_done(&ppbuf);

    void* buf2 = ppbuf_buf_write_begin(&ppbuf);
    ppbuf_buf_write_done(&ppbuf);

    ASSERT_FALSE(ppbuf_write_available(&ppbuf));
    ASSERT_TRUE(ppbuf_read_available(&ppbuf));

    void* buf3 = ppbuf_buf_read_begin(&ppbuf);
    ASSERT_FALSE(ppbuf_write_available(&ppbuf));
    ppbuf_buf_read_done(&ppbuf);
    ASSERT_TRUE(ppbuf_write_available(&ppbuf));

    void* buf4 = ppbuf_buf_read_begin(&ppbuf);
    ppbuf_buf_read_done(&ppbuf);

    ASSERT_TRUE(ppbuf_write_available(&ppbuf));
    ASSERT_FALSE(ppbuf_read_available(&ppbuf));

    ASSERT_EQ(buf1, buf3);
    ASSERT_EQ(buf2, buf4);

    ppbuf_destroy(&ppbuf);
}

TEST(PingPongBufTest, InterleavedReadWrite) {
    ppbuf_t ppbuf;
    ASSERT_EQ(ppbuf_init(&ppbuf, BUF_SIZE), 0);

    void* buf1 = ppbuf_buf_write_begin(&ppbuf);
    ppbuf_buf_write_done(&ppbuf);

    void* buf2 = ppbuf_buf_write_begin(&ppbuf);
    void* buf3 = ppbuf_buf_read_begin(&ppbuf);
    
    ASSERT_TRUE(ppbuf_read_available(&ppbuf));
    ASSERT_TRUE(ppbuf_write_available(&ppbuf));

    ASSERT_EQ(buf3, buf1);
    ASSERT_NE(buf3, buf2);

    ppbuf_buf_write_done(&ppbuf);
    void* buf4 = ppbuf_buf_read_begin(&ppbuf);
    ASSERT_EQ(buf4, buf3);
    ppbuf_buf_read_done(&ppbuf);

    void* buf5 = ppbuf_buf_read_begin(&ppbuf);
    ASSERT_EQ(buf5, buf2);

    ppbuf_destroy(&ppbuf);
}

TEST(PingPongBufTest, ConcurrentReadWrite) {
    ppbuf_t ppbuf;
    ASSERT_EQ(ppbuf_init(&ppbuf, BUF_SIZE), 0);
    const int loop_size = 100000;
    std::thread writer_thread([&]() {
        for (int i = 0; i < loop_size; ++i) {
            int* buf = (int*)ppbuf_buf_write_begin(&ppbuf);
            *buf = i;
            ppbuf_buf_write_done(&ppbuf);
        }
    });

    std::thread reader_thread([&]() {
        for (int i = 0; i < loop_size; ++i) {
            int* buf = (int*)ppbuf_buf_read_begin(&ppbuf);
            ASSERT_EQ(*buf, i);
            ppbuf_buf_read_done(&ppbuf);
        }
    });

    writer_thread.join();
    reader_thread.join();

    ppbuf_destroy(&ppbuf);
}
