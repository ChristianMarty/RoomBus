#include "utility/memory.h"

TEST_CASE( "Test memory_copy", "[memory]" ) {

    SECTION("Normal Test") {
        uint8_t source[] = {1,2,3,4,5,6};
        uint8_t destination[] = {0,0,0,0,0,0};
        uint8_t result[] = {1,2,3,4,5,6};

        memory_copy(destination, source, sizeof(destination));
        REQUIRE(match_array(destination, result, sizeof(destination)));
    }
}


TEST_CASE( "Test memory_removeFront", "[memory]" ) {

    SECTION("Normal Test 1") {
        uint8_t data[] = "Some test data as a string";
        REQUIRE(memory_removeFront(7, data, sizeof(data))==sizeof(data)-7);
        REQUIRE(match_array(&data[0], (uint8_t*)"st data as a string", sizeof(data)-7));
    }

    SECTION("Normal Test 2") {
        uint8_t data[] = {1,2,3,4,5,6};
        uint8_t result[] = {0,0,0,0,0,0};
        REQUIRE(memory_removeFront(6, data, sizeof(data)) == 0);
        REQUIRE(match_array(data, result, sizeof(data)));
    }

    SECTION("Normal Test 3") {
        uint8_t data[] = {1,2,3,4,5,6};
        uint8_t result[] = {4,5,6,0,0,0};
        REQUIRE(memory_removeFront(3, data, sizeof(data)) == 3);
        REQUIRE(match_array(data, result, sizeof(data)));
    }
}
