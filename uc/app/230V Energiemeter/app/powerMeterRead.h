/*
 * powerMeterRead.h
 *
 * Created: 02.01.2024 21:41:35
 *  Author: Christian
 */ 
#include "protocol/valueSystemProtocol.h"
#include "interface/modbusMaster.h"

#ifndef POWERMETERREAD_H_
#define POWERMETERREAD_H_

void modbusMaster_onReadingCompleted(const modbusMaster_reading_t *reading);

modbusMaster_t modbusMaster;

typedef struct {
	float volatge;
	float current;
	float activePower;
	float apparentPower;
	float reactivePower;
	float frequency;
	float importEnergy;
} energyMeter_t;

energyMeter_t meter[3];

#define READ_INTERVAL (30*1000) // read every 30 seconds
#define SEND_INTERVAL READ_INTERVAL

// ------------------------------------------------------------------------------------------------
// M1 -> Modbus Address 3
// M2 -> Modbus Address 2
// M3 -> Modbus Address 1
// ------------------------------------------------------------------------------------------------
modbusMaster_reading_t modbusMaster_reading [] = {
	{3, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister,  0, &meter[0].volatge,		 READ_INTERVAL, nullptr},
	{2, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister,  0, &meter[1].volatge,		 READ_INTERVAL, nullptr},
	{1, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister,  0, &meter[2].volatge,		 READ_INTERVAL, nullptr},
	
	{3, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister,  6, &meter[0].current,		 READ_INTERVAL, nullptr},
	{2, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister,  6, &meter[1].current,		 READ_INTERVAL, nullptr},
	{1, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister,  6, &meter[2].current,		 READ_INTERVAL, nullptr},
	
	{3, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 12, &meter[0].activePower,	 READ_INTERVAL, nullptr},
	{2, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 12, &meter[1].activePower,	 READ_INTERVAL, nullptr},
	{1, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 12, &meter[2].activePower,	 READ_INTERVAL, nullptr},
	
	{3, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 18, &meter[0].apparentPower, READ_INTERVAL, nullptr},
	{2, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 18, &meter[1].apparentPower, READ_INTERVAL, nullptr},
	{1, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 18, &meter[2].apparentPower, READ_INTERVAL, nullptr},
	
	{3, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 24, &meter[0].reactivePower, READ_INTERVAL, nullptr},
	{2, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 24, &meter[1].reactivePower, READ_INTERVAL, nullptr},
	{1, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 24, &meter[2].reactivePower, READ_INTERVAL, nullptr},
	
	{3, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 70, &meter[0].frequency,	 READ_INTERVAL, nullptr},
	{2, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 70, &meter[1].frequency,	 READ_INTERVAL, nullptr},
	{1, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 70, &meter[2].frequency,	 READ_INTERVAL, nullptr},
	
	{3, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 76, &meter[0].importEnergy,	 READ_INTERVAL, nullptr},
	{2, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 76, &meter[1].importEnergy,	 READ_INTERVAL, nullptr},
	{1, modbusMaster_functionCode_t::modbusMaster_fc_readInputRegister, 76, &meter[2].importEnergy,	 READ_INTERVAL, modbusMaster_onReadingCompleted}
};
#define modbusMaster_reading_size ( sizeof(modbusMaster_reading) / sizeof(modbusMaster_reading_t) )

vsp_valueSignal_t valueSignals[]={
	{ 0x01, "M1 - Voltage",			SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 300.0f},	 vsp_uom_number, nullptr},
	{ 0x02, "M1 - Current",			SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 10.0f},	 vsp_uom_number, nullptr},
	{ 0x03, "M1 - Active Power",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 1000.0f},  vsp_uom_number, nullptr},
	{ 0x04, "M1 - Apparent Power",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 1000.0f},  vsp_uom_number, nullptr},
	{ 0x05, "M1 - Reactive Power",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 1000.0f},  vsp_uom_number, nullptr},
	{ 0x06, "M1 - Frequency",		SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 100.0f},   vsp_uom_number, nullptr},
	{ 0x07, "M1 - Import Energy",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 65000.0f}, vsp_uom_number, nullptr},
	
	{ 0x08, "M2 - Voltage",			SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 300.0f},   vsp_uom_number, nullptr},
	{ 0x09, "M2 - Current",			SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 10.0f},    vsp_uom_number, nullptr},
	{ 0x0A, "M2 - Active Power",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 1000.0f},  vsp_uom_number, nullptr},
	{ 0x0B, "M2 - Apparent Power",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 1000.0f},  vsp_uom_number, nullptr},
	{ 0x0C, "M2 - Reactive Power",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 1000.0f},  vsp_uom_number, nullptr},
	{ 0x0D, "M2 - Frequency",		SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 100.0f},   vsp_uom_number, nullptr},
	{ 0x0E, "M2 - Import Energy",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 65000.0f}, vsp_uom_number, nullptr},
	
	{ 0x0F, "M3 - Voltage",			SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 300.0f},   vsp_uom_number, nullptr},
	{ 0x10, "M3 - Current",			SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 10.0f},    vsp_uom_number, nullptr},
	{ 0x11, "M3 - Active Power",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 1000.0f},  vsp_uom_number, nullptr},
	{ 0x12, "M3 - Apparent Power",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 1000.0f},  vsp_uom_number, nullptr},
	{ 0x13, "M3 - Reactive Power",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 1000.0f},  vsp_uom_number, nullptr},
	{ 0x14, "M3 - Frequency",		SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 100.0f},   vsp_uom_number, nullptr},
	{ 0x15, "M3 - Import Energy",	SEND_INTERVAL, true, {.Number = 0.0f}, {.Number = 65000.0f}, vsp_uom_number, nullptr}
};
#define valueSignalListSize ARRAY_LENGTH(valueSignals)
vsp_itemState_t valueSignalStateList[valueSignalListSize];


valueSystemProtocol_t valueSystem =
{
	.signals = valueSignals,
	.signalSize = valueSignalListSize,
	.slots = nullptr,
	.slotSize = 0,
	._signalState = valueSignalStateList,
	._slotState = nullptr
};

#endif /* POWERMETERREAD_H_ */