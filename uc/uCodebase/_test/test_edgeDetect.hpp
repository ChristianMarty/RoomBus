#ifdef TEST_RUN

#include "../utility/edgeDetect.h"

TEST_CASE( "Test edgeDetect", "[ed_onChange]" ) {

    edgeDetect_t edgeDetect_change;
    edgeDetect_t edgeDetect_falling;
    edgeDetect_t edgeDetect_rising;
    bool signal[] =         { false, false, true,true,false, true, false};
    bool result_change[] =  { false, false, true,false,true,true,true};
    bool result_falling[] = { false, false, false,false,true,false,true};
    bool result_rising[] =  { false, false, true,false,false,true,false};

    for(uint8_t i = 0; i<sizeof(signal); i++)
    {
        REQUIRE(ed_onChange(&edgeDetect_change, signal[i]) ==  result_change[i]);
        REQUIRE(ed_onFalling(&edgeDetect_falling, signal[i]) == result_falling[i]);
        REQUIRE(ed_onRising(&edgeDetect_rising, signal[i]) == result_rising[i]);
    }
}

#endif