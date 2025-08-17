#ifdef TEST_RUN

#include "bus.h"
#include "protocol/valueSystemProtocol.h"
#include "common/kernel.h"
#include "driver/SAMx5x/kernel/tickTimer.h"

kernel_t kernel {
    .bus {
            .getMessageSlot =bus_getMessageSlot,
            .writeHeader = bus_writeHeader,
            .pushByte = bus_pushByte,
            .pushWord16 = bus_pushWord16,
            .pushWord24 = bus_pushWord24,
            .pushWord32 = bus_pushWord32,
            .pushArray = bus_pushArray,
            .pushString = bus_pushString,
            .send = bus_send
    },
    .tickTimer {
        .reset = tickTimer_reset,
        .delay1ms = tickTimer_delay1ms
    }
};

TEST_CASE( "Test Value Report Protocol", "[valueReportProtocol]" ) {
    tickTimer_initialize(0);
    bus_init();

    const vsp_valueSignal_t valueReportSignal[] = {
            {0x0100, "Value Signal Number", 10, false, {.Number = 10.0}, {.Number = 20.0}, vsp_uom_number, nullptr},
            {0x0101, "Value Signal Long", 10, false, {.Long = 10}, {.Long = 0xF863434A}, vsp_uom_long, nullptr},
    };
    #define valueReportSignalListSize (sizeof(valueReportSignal)/sizeof(vsp_valueSignal_t))

    const vsp_valueSlot_t valueReportSlots[] = {
            {0x0100, "Value Slot", 10, nullptr}
    };
    #define valueReportSlotListSize (sizeof(valueReportSlots)/sizeof(vsp_valueSlot_t))

    vsp_itemState_t valueReportSignalStatusList[valueReportSignalListSize];
    vsp_itemState_t valueReportSlotStatusList[valueReportSlotListSize];

    const valueSystemProtocol_t valueReportSystem = {
            .signals = valueReportSignal,
            .signalSize = valueReportSignalListSize,
            .slots = valueReportSlots,
            .slotSize = valueReportSlotListSize,
            ._signalState = valueReportSignalStatusList,
            ._slotState = valueReportSlotStatusList
    };
    vsp_initialize(&valueReportSystem);
    vsp_mainHandler(&valueReportSystem);

    bus_rxMessage_t message = {
        .sourceAddress = 0x12,
        .command = vsp_cmd_valueCommand,
        .data = nullptr,
        .dataLength = 0
    };

    SECTION("Set minimum Number") {
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setMinimum};
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 10.0);
    }
    SECTION("Set maximum Number") {
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setMaximum};
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 20.0);
    }
    SECTION("Invert Number") {
        valueReportSignalStatusList[0].value.Number = 15;
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_invert};
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 5.0);
    }

    SECTION("Set Number as Number Reject Max") {
        float number = 21;
        valueReportSignalStatusList[0].value.Number = 15;
        uint32_t number32 = *((uint32_t*)&number);
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setNumberValueReject, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 15);
    }
    SECTION("Set Number as Number Reject Min") {
        float number = 9;
        valueReportSignalStatusList[0].value.Number = 15;
        uint32_t number32 = *((uint32_t*)&number);
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setNumberValueReject, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 15);
    }
    SECTION("Set Number as Number Clamp Max") {
        float number = 21;
        uint32_t number32 = *((uint32_t*)&number);
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setNumberValueClamp, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 20);
    }
    SECTION("Set Number as Number Clamp Min") {
        float number = 9;
        uint32_t number32 = *((uint32_t*)&number);
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setNumberValueClamp, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 10);
    }
    SECTION("Set Number as Long Reject Max") {
        uint32_t number32 = 21;
        valueReportSignalStatusList[0].value.Number = 15;
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setLongValueReject, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 15);
    }
    SECTION("Set Number as Long Reject Min") {
        uint32_t number32 = 9;
        valueReportSignalStatusList[0].value.Number = 15;
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setLongValueReject, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 15);
    }
    SECTION("Set Number as Long Clamp Max") {
        uint32_t number32 = 21;
        valueReportSignalStatusList[0].value.Number = 15;
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setLongValueClamp, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 20);
    }
    SECTION("Set Number as Long Clamp Min") {
        uint32_t number32 = 9;
        valueReportSignalStatusList[0].value.Number = 15;
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_setLongValueClamp, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 10);
    }

    SECTION("Add Number to Number Valid Reject") {
        float number = 2;
        valueReportSignalStatusList[0].value.Number = 15;
        uint32_t number32 = *((uint32_t*)&number);
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_addNumberReject, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 17);
    }
    SECTION("Add Number to Number Invalid Reject") {
        float number = 7;
        valueReportSignalStatusList[0].value.Number = 15;
        uint32_t number32 = *((uint32_t*)&number);
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_addNumberReject, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 15);
    }
    SECTION("Add Number to Number Valid Clamp") {
        float number = 2;
        valueReportSignalStatusList[0].value.Number = 15;
        uint32_t number32 = *((uint32_t*)&number);
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_addNumberClamp, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 17);
    }
    SECTION("Add Number to Number Invalid Clamp") {
        float number = 7;
        valueReportSignalStatusList[0].value.Number = 15;
        uint32_t number32 = *((uint32_t*)&number);
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_addNumberClamp, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 20);
    }
    SECTION("Add Long to Number Valid Reject") {
        uint32_t number32 = 2;
        valueReportSignalStatusList[0].value.Number = 15;
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_addLongReject, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 17);
    }
    SECTION("Add Long to Number Invalid Reject") {
        uint32_t number32 = 7;
        valueReportSignalStatusList[0].value.Number = 15;
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_addLongReject, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 15);
    }
    SECTION("Add Long to Number Valid Clamp") {
        uint32_t number32 = 2;
        valueReportSignalStatusList[0].value.Number = 15;
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_addLongClamp, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 17);
    }
    SECTION("Add Long to Number Invalid Clamp") {
        uint32_t number32 = 7;
        valueReportSignalStatusList[0].value.Number = 15;
        uint8_t data[] = {0x01, 0x00, vsp_vCmd_addLongClamp, (uint8_t)(number32>>24), (uint8_t)(number32>>16), (uint8_t)(number32>>8),(uint8_t)(number32) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[0].value.Number == 20);
    }



    SECTION("Set minimum Long") {
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_setMinimum};
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 10);
    }
    SECTION("Set maximum Long") {
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_setMaximum};
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 0xF863434A);
    }

    SECTION("Set Long as Long Reject Max") {
        uint32_t number = 0xFF1D6293;
        valueReportSignalStatusList[1].value.Long  = 15;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_setLongValueReject, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 15);
    }
    SECTION("Set Long as Long Reject Min") {
        uint32_t number = 9;
        valueReportSignalStatusList[1].value.Long = 15;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_setLongValueReject, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 15);
    }
    SECTION("Set Long as Long Clamp Max") {
        uint32_t number = 0xFF1D6293;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_setLongValueClamp, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 0xF863434A);
    }
    SECTION("Set Long as Long Clamp Min") {
        uint32_t number = 9;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_setLongValueClamp, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 10);
    }

    SECTION("Add Long to Long Valid Reject") {
        uint32_t number = 9;
        valueReportSignalStatusList[1].value.Long = 15;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_addLongReject, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 24);
    }
    SECTION("Add Long to Long Invalid Reject") {
        uint32_t number = 20;
        valueReportSignalStatusList[1].value.Long = 0xF8634340;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_addLongReject, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 0xF8634340);
    }
    SECTION("Add Long to Long Valid Clamp") {
        uint32_t number = 2;
        valueReportSignalStatusList[1].value.Long = 0xF8634340;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_addLongClamp, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 0xF8634342);
    }
    SECTION("Add Long to Long Invalid Clamp") {
        uint32_t number = 20;
        valueReportSignalStatusList[1].value.Long = 0xF8634340;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_addLongClamp, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 0xF863434A);
    }

    SECTION("Subtract Long from Long Valid Reject") {
        uint32_t number = 9;
        valueReportSignalStatusList[1].value.Long = 25;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_subtractLongReject, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 16);
    }
    SECTION("Subtract Long from Long Invalid Reject") {
        uint32_t number = 20;
        valueReportSignalStatusList[1].value.Long = 25;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_subtractLongReject, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 25);
    }
    SECTION("Subtract Long from Long Valid Clamp") {
        uint32_t number = 3;
        valueReportSignalStatusList[1].value.Long = 20;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_subtractLongClamp, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 17);
    }
    SECTION("Subtract Long from Long Invalid Clamp") {
        uint32_t number = 20;
        valueReportSignalStatusList[1].value.Long = 25;
        uint8_t data[] = {0x01, 0x01, vsp_vCmd_subtractLongClamp, (uint8_t)(number>>24), (uint8_t)(number>>16), (uint8_t)(number>>8),(uint8_t)(number) };
        message.data = &data[0];
        message.dataLength= sizeof(data);
        vsp_receiveHandler(&valueReportSystem, &message);
        REQUIRE(valueReportSignalStatusList[1].value.Long == 10);
    }
}

#endif