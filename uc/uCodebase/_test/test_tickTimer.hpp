#include "../driver/SAMx5x/kernel/tickTimer.h"

TEST_CASE( "Test tickTimer", "[tickTimer]" ) {

    SECTION("Test basic functionality") {
        tickTimer_initialize(0);

        tickTimer_t timer;
        tickTimer_reset(&timer);
        REQUIRE(tickTimer_delay1ms(&timer,1) == false);
        tickTimer_interruptHandler();
        REQUIRE(tickTimer_delay1ms(&timer,1) == true);
    }

    SECTION("Test more functionality") {
        tickTimer_initialize(0);

        tickTimer_t timer;
        tickTimer_reset(&timer);
        REQUIRE(tickTimer_delay1ms(&timer,1) == false);
        tickTimer_interruptHandler();
        REQUIRE(tickTimer_delay1ms(&timer,1) == true);
        REQUIRE(tickTimer_delay1ms(&timer,1) == false);
        REQUIRE(tickTimer_delay1ms(&timer,2) == false);
        tickTimer_interruptHandler();
        REQUIRE(tickTimer_delay1ms(&timer,2) == false);
        tickTimer_interruptHandler();
        REQUIRE(tickTimer_delay1ms(&timer,1) == true);
    }

    SECTION("Test timer reset") {
        tickTimer_initialize(0);

        tickTimer_t timer;
        tickTimer_reset(&timer);
        REQUIRE(tickTimer_delay1ms(&timer,1) == false);
        tickTimer_interruptHandler();
        tickTimer_reset(&timer);
        REQUIRE(tickTimer_delay1ms(&timer,1) == false);
        tickTimer_interruptHandler();
        REQUIRE(tickTimer_delay1ms(&timer,1) == true);
    }

    SECTION("Test overflow") {
        tickTimer_initialize(0);

        for(uint32_t i= 0; i<0xFFFE;i++)
        {
            tickTimer_interruptHandler();
        }

        tickTimer_t timer;
        tickTimer_reset(&timer);
        REQUIRE(tickTimer_delay1ms(&timer,0x02) == false);
        tickTimer_interruptHandler();
        tickTimer_interruptHandler();
        REQUIRE(tickTimer_delay1ms(&timer,0x02) == true);
    }
}