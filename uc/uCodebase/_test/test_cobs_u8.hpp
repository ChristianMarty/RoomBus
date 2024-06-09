#ifdef TEST_RUN

#include "../utility/cobs_u8.h"

TEST_CASE( "COBS u8 encode", "[cobs_encode]" ) {

    SECTION( "Test 1" ) {
        const uint8_t source_data[] = {0x01,0x00, 0x17, 0x23 };
        const uint8_t test[] = {0x02, 0x01,0x03,0x17,0x23,0x00};
        uint8_t destination_data[sizeof(source_data)+2];
        uint8_t destination_length = cobs_encode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(source_data)+2);
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 2" ) {
        const uint8_t source_data[] = {};
        const uint8_t test[] = {0x01, 0x00};
        uint8_t destination_data[sizeof(source_data) + 2];
        uint8_t destination_length = cobs_encode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(source_data)+2);
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 3" ) {
        const uint8_t source_data[] = {0x00};
        const uint8_t test[] = {0x01,0x01,  0x00};
        uint8_t destination_data[sizeof(source_data) + 2];
        uint8_t destination_length = cobs_encode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(source_data)+2);
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 4" ) {
        const uint8_t source_data[] = {0x77};
        const uint8_t test[] = {0x02,0x77,  0x00};
        uint8_t destination_data[sizeof(source_data) + 2];
        uint8_t destination_length = cobs_encode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(source_data)+2);
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 5" ) {
        const uint8_t source_data[] = {0x77, 0x66, 0x55, 0x44};
        const uint8_t test[] = {0x05,0x77, 0x66, 0x55, 0x44,  0x00};
        uint8_t destination_data[sizeof(source_data) + 2];
        uint8_t destination_length = cobs_encode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(source_data)+2);
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }
}


TEST_CASE( "COBS u8 decode", "[cobs_decode]" ) {

    SECTION("Test 1") {
        const uint8_t test[] = {0x01,0x00, 0x17, 0x23 };
        const uint8_t source_data[] = {0x02, 0x01,0x03,0x17,0x23,0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length));
    }

    SECTION( "Test 2" ) {
        const uint8_t test[] = {};
        const uint8_t source_data[] = {0x01, 0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 3" ) {
        const uint8_t test[] = {0x00};
        const uint8_t source_data[] = {0x01,0x01,  0x00};;
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 4" ) {
        const uint8_t test[] = {0x77};
        const uint8_t source_data[] = {0x02, 0x77, 0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 5" ) {
        const uint8_t test[] = {0x77, 0x66, 0x55, 0x44};
        const uint8_t source_data[] = {0x05,0x77, 0x66, 0x55, 0x44,  0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 6" ) {
        const uint8_t source_data[] = {0x05,0x77, 0x66, 0x55, 0x44,  0x09};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 7" ) {
        const uint8_t source_data[] = {};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 8" ) {
        const uint8_t source_data[] = {0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 9" ) {
        const uint8_t source_data[] = {0x00, 0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 10" ) {
        const uint8_t source_data[] = {0x00, 0x00, 0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 11" ) {
        const uint8_t test[] = {0x77, 0x66};
        const uint8_t source_data[] = {0x03,0x77, 0x66, 0x00, 0x44,  0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 12 - multiple frames" ) {
        const uint8_t test[] = {0x77, 0x66, 0x00, 0x99};
        const uint8_t source_data[] = {0x03,0x77, 0x66, 0x02, 0x99, 0x00, 0x01,  0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 13 - framing error" ) {
        const uint8_t source_data[] = {0x03,0x77, 0x66, 0x02, 0x00, 0x44,  0x00};
        uint8_t destination_data[sizeof(source_data)];
        uint8_t destination_length = cobs_decode(&destination_data[0], &source_data[0], sizeof(source_data));
        REQUIRE(destination_length ==0);
    }
}


TEST_CASE( "COBS u8 Decode Stream", "[cobs_decodeStream]" ) {

    cobs_decodeStream_t cobsStream;

    SECTION("Test 1") {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t test[] = {0x01,0x00, 0x17, 0x23 };
        const uint8_t source_data[] = {0x02, 0x01,0x03,0x17,0x23,0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length));
    }

    SECTION( "Test 2" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t test[] = {};
        const uint8_t source_data[] = {0x01, 0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 3" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t test[] = {0x00};
        const uint8_t source_data[] = {0x01,0x01,  0x00};;
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 4" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t test[] = {0x77};
        const uint8_t source_data[] = {0x02, 0x77, 0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 5" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t test[] = {0x77, 0x66, 0x55, 0x44};
        const uint8_t source_data[] = {0x05,0x77, 0x66, 0x55, 0x44,  0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 6" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t source_data[] = {0x05,0x77, 0x66, 0x55, 0x44,  0x09};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 7" ) {
        cobs_decodeStreamStart(&cobsStream);
        uint8_t destination_data[10];
        uint16_t destination_length = cobs_decodeStream(&cobsStream, 0, &destination_data[0]);
        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 8" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t source_data[] = {0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 9" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t source_data[] = {0x00, 0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 10" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t source_data[] = {0x00, 0x00, 0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == 0);
    }

    SECTION( "Test 11" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t test[] = {0x77, 0x66};
        const uint8_t source_data[] = {0x03,0x77, 0x66, 0x00, 0x44,  0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 12 - multiple frames" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t test[] = {0x77, 0x66, 0x00, 0x99};
        const uint8_t source_data[] = {0x03,0x77, 0x66, 0x02, 0x99, 0x00, 0x01,  0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length == sizeof(test));
        REQUIRE(match_array(destination_data, test, destination_length)) ;
    }

    SECTION( "Test 13 - framing error" ) {
        cobs_decodeStreamStart(&cobsStream);
        const uint8_t source_data[] = {0x03,0x77, 0x66, 0x02, 0x00, 0x44,  0x00};
        uint8_t destination_data[sizeof(source_data)];

        uint16_t destination_length = 0;
        for(uint8_t i = 0; i<sizeof(source_data); i++){
            destination_length = cobs_decodeStream(&cobsStream, source_data[i], &destination_data[0]);
            if(destination_length) break;
        }

        REQUIRE(destination_length ==0);
    }
}

#endif