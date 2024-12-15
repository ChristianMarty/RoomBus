#ifdef TEST_RUN

#include "utility/perlin.h"

TEST_CASE( "Test perlin", "[perlin]" ) {

    SECTION( "Test 1" ) {

        #define PERLIN_SIZE 16
        uint8_t seed[PERLIN_SIZE] = {8,13,2,15,4,3,5,9,15,3,8,3,11,7,5,2};
        uint8_t output[PERLIN_SIZE];

        perlin(&seed[0], &output[0], PERLIN_SIZE);

        uint8_t solution[PERLIN_SIZE] = {8,7,6,6,6,7,8,10,13,11,10,10,10,9,7,7};
        REQUIRE(match_array(output, solution, PERLIN_SIZE)) ;
    }
}
#endif