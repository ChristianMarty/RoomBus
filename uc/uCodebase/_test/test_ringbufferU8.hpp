#include "utility/ringbuffer_u8.h"
TEST_CASE( "ringbuffer_u8 perlin", "[ringbuffer_u8]" ) {

    SECTION( "New Buffer" ) {
        ringbufferU8_t buffer;
        uint8_t data[5];
        ringbufferU8_init(&buffer, &data[0], sizeof(data));

        REQUIRE(ringbufferU8_empty(&buffer) == true);
        REQUIRE(ringbufferU8_full(&buffer) == false);
        REQUIRE(ringbufferU8_count(&buffer) == 0);
        REQUIRE(ringbufferU8_space(&buffer) == 5);

        REQUIRE(ringbufferU8_read(&buffer) == 0);
    }

    SECTION( "Add and remove one byte" ) {
        ringbufferU8_t buffer;
        uint8_t data[5];
        ringbufferU8_init(&buffer, &data[0], sizeof(data));

        ringbufferU8_write(&buffer, 0xF1);

        REQUIRE(ringbufferU8_empty(&buffer) == false);
        REQUIRE(ringbufferU8_full(&buffer) == false);
        REQUIRE(ringbufferU8_count(&buffer) == 1);
        REQUIRE(ringbufferU8_space(&buffer) == 4);

        REQUIRE(ringbufferU8_read(&buffer) == 0xF1);

        REQUIRE(ringbufferU8_empty(&buffer) == true);
        REQUIRE(ringbufferU8_full(&buffer) == false);
        REQUIRE(ringbufferU8_count(&buffer) == 0);
        REQUIRE(ringbufferU8_space(&buffer) == 5);
    }

    SECTION( "Add and remove full data" ) {
        ringbufferU8_t buffer;
        uint8_t data[5];
        ringbufferU8_init(&buffer, &data[0], sizeof(data));
        REQUIRE(ringbufferU8_space(&buffer) == 5);

        ringbufferU8_write(&buffer, 0x0A);
        ringbufferU8_write(&buffer, 0x0B);
        ringbufferU8_write(&buffer, 0x0C);
        ringbufferU8_write(&buffer, 0x0D);
        ringbufferU8_write(&buffer, 0x0E);

        REQUIRE(ringbufferU8_empty(&buffer) == false);
        REQUIRE(ringbufferU8_full(&buffer) == true);
        REQUIRE(ringbufferU8_count(&buffer) == 5);
        REQUIRE(ringbufferU8_space(&buffer) == 0);

        REQUIRE(ringbufferU8_get(&buffer, 0) == 0x0A);
        REQUIRE(ringbufferU8_get(&buffer, 1) == 0x0B);
        REQUIRE(ringbufferU8_get(&buffer, 2) == 0x0C);
        REQUIRE(ringbufferU8_get(&buffer, 3) == 0x0D);
        REQUIRE(ringbufferU8_get(&buffer, 4) == 0x0E);
        // test overflow
        REQUIRE(ringbufferU8_get(&buffer, 5) == 0x0A);
        REQUIRE(ringbufferU8_get(&buffer, 6) == 0x0B);

        REQUIRE(ringbufferU8_read(&buffer) == 0x0A);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0B);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0C);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0D);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0E);

        REQUIRE(ringbufferU8_empty(&buffer) == true);
        REQUIRE(ringbufferU8_full(&buffer) == false);
        REQUIRE(ringbufferU8_count(&buffer) == 0);
        REQUIRE(ringbufferU8_space(&buffer) == 5);
    }

    SECTION( "Add and remove data with overflow" ) {
        ringbufferU8_t buffer;
        uint8_t data[5];
        ringbufferU8_init(&buffer, &data[0], sizeof(data));

        ringbufferU8_write(&buffer, 0x0A);
        ringbufferU8_write(&buffer, 0x0B);
        ringbufferU8_write(&buffer, 0x0C);

        REQUIRE(ringbufferU8_empty(&buffer) == false);
        REQUIRE(ringbufferU8_full(&buffer) == false);
        REQUIRE(ringbufferU8_count(&buffer) == 3);

        REQUIRE(ringbufferU8_read(&buffer) == 0x0A);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0B);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0C);

        REQUIRE(ringbufferU8_empty(&buffer) == true);
        REQUIRE(ringbufferU8_full(&buffer) == false);
        REQUIRE(ringbufferU8_count(&buffer) == 0);

        ringbufferU8_write(&buffer, 0x1A);
        ringbufferU8_write(&buffer, 0x1B);
        ringbufferU8_write(&buffer, 0x1C);
        ringbufferU8_write(&buffer, 0x1D);
        ringbufferU8_write(&buffer, 0x1E);

        REQUIRE(ringbufferU8_empty(&buffer) == false);
        REQUIRE(ringbufferU8_full(&buffer) == true);
        REQUIRE(ringbufferU8_count(&buffer) == 5);

        REQUIRE(ringbufferU8_read(&buffer) == 0x1A);
        REQUIRE(ringbufferU8_read(&buffer) == 0x1B);
        REQUIRE(ringbufferU8_read(&buffer) == 0x1C);
        REQUIRE(ringbufferU8_read(&buffer) == 0x1D);
        REQUIRE(ringbufferU8_read(&buffer) == 0x1E);

        REQUIRE(ringbufferU8_empty(&buffer) == true);
        REQUIRE(ringbufferU8_full(&buffer) == false);
        REQUIRE(ringbufferU8_count(&buffer) == 0);
    }

    SECTION( "Add and remove full with overfill" ) {
        ringbufferU8_t buffer;
        uint8_t data[6];
        ringbufferU8_init(&buffer, &data[0], sizeof(data)-1);

        data[5] = 0xAC;

        ringbufferU8_write(&buffer, 0x0A);
        ringbufferU8_write(&buffer, 0x0B);
        ringbufferU8_write(&buffer, 0x0C);
        ringbufferU8_write(&buffer, 0x0D);
        ringbufferU8_write(&buffer, 0x0E);
        ringbufferU8_write(&buffer, 0x0F);

        REQUIRE(data[5] == 0xAC);

        REQUIRE(ringbufferU8_empty(&buffer) == false);
        REQUIRE(ringbufferU8_full(&buffer) == true);
        REQUIRE(ringbufferU8_count(&buffer) == 5);

        REQUIRE(ringbufferU8_read(&buffer) == 0x0A);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0B);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0C);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0D);
        REQUIRE(ringbufferU8_read(&buffer) == 0x0E);

        REQUIRE(ringbufferU8_empty(&buffer) == true);
        REQUIRE(ringbufferU8_full(&buffer) == false);
        REQUIRE(ringbufferU8_count(&buffer) == 0);
    }
}

