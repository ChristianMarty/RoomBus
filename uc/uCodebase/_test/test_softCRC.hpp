#ifdef TEST_RUN

#include "../utility/softCRC.h"
#include "../utility/bytearray.h"

TEST_CASE( "Test crc16", "[crc16]" ) {

    SECTION("Test 1") {
        uint8_t input[] = {0x01, 0x02, 0x03, 0x04};
        REQUIRE(crc16(&input[0],sizeof(input)) ==  0x89C3 );
    }

    SECTION("Test 2") {
        uint8_t input[] = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        REQUIRE(crc16(&input[0],sizeof(input)) ==  0x00 );
    }
}

TEST_CASE( "Test crc16 modbus", "[crc16]" ) {

    SECTION("Test 1") {
        uint8_t input[] =  {0x01,0x04,0x00,0x00,0x00,0x02};
        REQUIRE(crcModbus(&input[0],sizeof(input)) ==  0xCB71 );
    }

    SECTION("Test 2") {
        bytearray_t ba;
        uint8_t input[] =  {0x01,0x04,0x00,0x00,0x00,0x02};
        bytearray_init(&ba,&input[0],sizeof(input));
        ba.size = sizeof(input);
        REQUIRE(crcModbus_bytearray(&ba) ==  0xCB71 );
    }

    SECTION("Test 3") {
        bytearray_t ba;
        uint8_t input[] =  {0x01,0x04,0x00,0x00,0x00,0x02,0,0};
        bytearray_init(&ba,&input[0],sizeof(input));
        ba.size = sizeof(input)-2;
        uint16_t crc = crcModbus_bytearray(&ba);

        bytearray_pushU16_LE(&ba, crc);

        REQUIRE(crcModbus_bytearray(&ba) ==  0x00 );
    }
}

#endif