#include "utility/string.h"

TEST_CASE( "Test string_getLength", "[string_getLength]" ) {

    SECTION("Normal String") {
        const char str[] = "Test String 1234";
        REQUIRE(string_getLength(str) == 16);
    }

    SECTION("Empty String") {
        const char str[] = "";
        REQUIRE(string_getLength(str) == 0);
    }

    SECTION("String with 0xFF termination") {
        const char str[] = "Test String\xff 1234";
        REQUIRE(string_getLength(str) == 11);
    }
}

TEST_CASE( "Test string_fromInt32", "[string_fromInt32]" ) {

    SECTION("Test 1") {
        char str[] = "       ";
        string_fromInt32(445,0,str);
        REQUIRE(match_string(str,"445",10));
    }

    SECTION("Test 2") {
        char str[] = "                    ";
        string_fromInt32(0xFFFF,0,str);
        REQUIRE(match_string(str,"65535",20));
    }

    SECTION("Test 3") {
        char str[] = "                    ";
        string_fromInt32(0xFFFF,4,str);
        REQUIRE(match_string(str,"6.5535",20));
    }

    SECTION("Test 4") {
        char str[] = "                    ";
        string_fromInt32(0x7FFFFFFF,4,str);
        REQUIRE(match_string(str,"214748.3647",20));
    }
}

TEST_CASE( "Test string_int8ToHex", "[string_int8ToHex]" ) {

    SECTION("Test 1") {
        char str[] = "          ";
        string_uInt8ToHex(0x32,str);
        REQUIRE(match_string(str,"32",10));
    }

    SECTION("Test 2") {
        char str[] = "          ";
        string_uInt8ToHex(0x00,str);
        REQUIRE(match_string(str,"00",10));
    }

    SECTION("Test 3") {
        char str[] = "          ";
        string_uInt8ToHex(0xFF,str);
        REQUIRE(match_string(str,"FF",10));
    }

    SECTION("Test 4") {
        char str[] = "          ";
        string_uInt8ToHex(0xA9,str);
        REQUIRE(match_string(str,"A9",10));
    }
}

TEST_CASE( "Test string_append", "[string_append]" ) {

    SECTION("Test 1") {
        char str1[20] = "Test";
        const char str2[] = "222";
        char test[] = "Test222";
        string_append(str1, str2);
        REQUIRE(match_string(str1, test, sizeof(test)));
    }

}