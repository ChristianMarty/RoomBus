#include "utility/string.h"
#include "utility/memory.h"

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

TEST_CASE( "Test string_removeFront", "[string_removeFront]" ) {

    SECTION("Basic Functionality") {
        char string[] = "Test12345";
        string_removeFront(string,3,sizeof(string));
        REQUIRE(match_string( string, "t12345", sizeof("t12345") ));
    }

    SECTION("Remove > string") {
        char string[] = "Test12345";
        string_removeFront(string,12,sizeof(string));
        REQUIRE(string[0] == 0);
    }
}

TEST_CASE( "Test string_readLine", "[string_readLine]" ) {

    SECTION("Test windows") {
        char string1[] = "Test\r\n";
        REQUIRE(string_readLine(string1, sizeof(string1)) == 4);
    }

    SECTION("Test mac") {
        char string1[] = "Test\r";
        REQUIRE(string_readLine(string1, sizeof(string1)) == 4);
    }

    SECTION("Test unix") {
        char string1[] = "Test\n";
        REQUIRE(string_readLine(string1, sizeof(string1)) == 4);
    }

    SECTION("Test other") {
        char string1[] = "Test\n\r";
        REQUIRE(string_readLine(string1, sizeof(string1)) == 4);
    }

    SECTION("Test none") {
        char string1[] = "Test";
        REQUIRE(string_readLine(string1, sizeof(string1)) == 0);
    }


    SECTION("Read lines") {
        const char fullString[] = "Test 1\r\nAbcd 2\r\nQWER 3\r\n4567 4\r\nBLAS 5\r\nasld586\r\n";
        int dataSize = sizeof(fullString);
        #define bufferSize  9

        int dataPosition = 0;
        int lineLength = 0;
        int bufferPosition = 0;
        char buffer[bufferSize];
        memory_copy((uint8_t *)buffer, (uint8_t *)fullString, bufferSize);
        dataPosition+=bufferSize;

        while(dataPosition < dataSize){

            bufferPosition+=lineLength;
            if(bufferSize > bufferPosition && (buffer[bufferPosition] == '\n' || buffer[bufferPosition] == '\r')){
                lineLength = 1;
                continue;
            }else{
                int remainingLength = bufferSize-bufferPosition;
                if(remainingLength!=0) {
                    // move old remaining data to start of buffer
                    memory_copy((uint8_t *) buffer, (uint8_t *) &buffer[bufferPosition], remainingLength);
                }
                int refillLength = bufferSize-remainingLength;
                memory_copy((uint8_t*)&buffer[remainingLength], (uint8_t*)&fullString[dataPosition], refillLength);
                dataPosition += refillLength;
                bufferPosition = 0;
            }

            lineLength = string_readLine(&buffer[bufferPosition], bufferSize-bufferPosition);
            if(lineLength != 0){
                char line[64];
                memory_copy((uint8_t*)line, (uint8_t*)&buffer[bufferPosition], lineLength);
                line[lineLength] = 0;
                printf(line);
            }

        }
    }
}