#ifdef TEST_RUN

#include "utility/bytearray.h"

TEST_CASE( "Test Bytearray", "[bytearray]" ) {

    SECTION("Bytearray Push Byte") {
        uint8_t data[2] = {0x00, 0x00};
        uint8_t output_data[] = {0xAA, 0xBB};
        bytearray_t bytearray = {
                .data = &data[0],
                .size = 0,
                .capacity = sizeof(data)
        };

        REQUIRE(bytearray_getSize(&bytearray) == 0);
        REQUIRE(bytearray_getSpace(&bytearray) == sizeof(data));
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(bytearray_pushByte(&bytearray, 0xAA) == true);
        REQUIRE(bytearray_getSize(&bytearray) == 1);
        REQUIRE(bytearray_getSpace(&bytearray) == 1);
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(bytearray_pushByte(&bytearray, 0xBB) == true);
        REQUIRE(bytearray_getSize(&bytearray) == 2);
        REQUIRE(bytearray_getSpace(&bytearray) == 0);
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(bytearray_pushByte(&bytearray, 0xCC) == false);
        REQUIRE(bytearray_getSize(&bytearray) == 2);
        REQUIRE(bytearray_getSpace(&bytearray) == 0);
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(match_array(data, output_data, sizeof(data))) ;

        bytearray_flush(&bytearray);
        REQUIRE(bytearray_getSize(&bytearray) == 0);
        REQUIRE(bytearray_getSpace(&bytearray) == sizeof(data));
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

    }

    SECTION("Bytearray Push Array") {
        uint8_t input_data[] = {0xAA, 0xBB, 0xCC, 0xDD};
        uint8_t output_data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xAA, 0xBB, 0xCC, 0xDD, 0x00, 0x00};
        uint8_t data[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0x00, 0x00};
        bytearray_t bytearray = {
                .data = &data[0],
                .size = 0,
                .capacity = sizeof(data)
        };

        REQUIRE(bytearray_getSize(&bytearray) == 0);
        REQUIRE(bytearray_getSpace(&bytearray) == sizeof(data));
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(bytearray_pushArray(&bytearray, &input_data[0], sizeof(input_data)) == true);
        REQUIRE(bytearray_getSize(&bytearray) == 4);
        REQUIRE(bytearray_getSpace(&bytearray) == 6);
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(bytearray_pushArray(&bytearray, &input_data[0], sizeof(input_data)) == true);
        REQUIRE(bytearray_getSize(&bytearray) == 8);
        REQUIRE(bytearray_getSpace(&bytearray) == 2);
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(bytearray_pushArray(&bytearray, &input_data[0], sizeof(input_data)) == false);
        REQUIRE(bytearray_getSize(&bytearray) == 8);
        REQUIRE(bytearray_getSpace(&bytearray) == 2);
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(match_array(data, output_data, sizeof(data))) ;

        bytearray_flush(&bytearray);
        REQUIRE(bytearray_getSize(&bytearray) == 0);
        REQUIRE(bytearray_getSpace(&bytearray) == sizeof(data));
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));
    }

    SECTION("Bytearray Push U16") {
        uint8_t output_data[] = {0xAA, 0xBB, 0x00};
        uint8_t data[] = {0x00, 0x00, 0x00 };
        bytearray_t bytearray = {
                .data = &data[0],
                .size = 0,
                .capacity = sizeof(data)
        };

        REQUIRE(bytearray_getSize(&bytearray) == 0);
        REQUIRE(bytearray_getSpace(&bytearray) == sizeof(data));
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(bytearray_pushU16(&bytearray, 0xAABB) == true);
        REQUIRE(bytearray_getSize(&bytearray) == 2);
        REQUIRE(bytearray_getSpace(&bytearray) == 1);
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(bytearray_pushU16(&bytearray, 0xAABB) == false);
        REQUIRE(bytearray_getSize(&bytearray) == 2);
        REQUIRE(bytearray_getSpace(&bytearray) == 1);
        REQUIRE(bytearray_getCapacity(&bytearray) == sizeof(data));

        REQUIRE(match_array(data, output_data, sizeof(data))) ;
    }
}
#endif