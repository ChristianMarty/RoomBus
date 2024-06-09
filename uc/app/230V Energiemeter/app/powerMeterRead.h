/*
 * powerMeterRead.h
 *
 * Created: 02.01.2024 21:41:35
 *  Author: Christian
 */ 
#include "Raumsteuerung/valueReportProtocol.h"
#include "Raumsteuerung/modbusMaster.h"

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

vrp_valueItem_t valueItemList[]={
	{ 0x01, true, ((float)(0.0f)), ((float)(300.0f)),   &meter[0].volatge,       UOM_RAW_FLOAT, "M1 - Voltage",        nullptr},
	{ 0x02, true, ((float)(0.0f)), ((float)(10.0f)),    &meter[0].current,       UOM_RAW_FLOAT, "M1 - Current",        nullptr},
	{ 0x03, true, ((float)(0.0f)), ((float)(1000.0f)),  &meter[0].activePower,   UOM_RAW_FLOAT, "M1 - Active Power",   nullptr},
	{ 0x04, true, ((float)(0.0f)), ((float)(1000.0f)),  &meter[0].apparentPower, UOM_RAW_FLOAT, "M1 - Apparent Power", nullptr},
	{ 0x05, true, ((float)(0.0f)), ((float)(1000.0f)),  &meter[0].reactivePower, UOM_RAW_FLOAT, "M1 - Reactive Power", nullptr},
	{ 0x06, true, ((float)(0.0f)), ((float)(100.0f)),   &meter[0].frequency,     UOM_RAW_FLOAT, "M1 - Frequency",      nullptr},
	{ 0x07, true, ((float)(0.0f)), ((float)(65000.0f)), &meter[0].importEnergy,  UOM_RAW_FLOAT, "M1 - Import Energy",  nullptr},
	
	{ 0x08, true, ((float)(0.0f)), ((float)(300.0f)),   &meter[1].volatge,       UOM_RAW_FLOAT, "M2 - Voltage",        nullptr},
	{ 0x09, true, ((float)(0.0f)), ((float)(10.0f)),    &meter[1].current,       UOM_RAW_FLOAT, "M2 - Current",        nullptr},
	{ 0x0A, true, ((float)(0.0f)), ((float)(1000.0f)),  &meter[1].activePower,   UOM_RAW_FLOAT, "M2 - Active Power",   nullptr},
	{ 0x0B, true, ((float)(0.0f)), ((float)(1000.0f)),  &meter[1].apparentPower, UOM_RAW_FLOAT, "M2 - Apparent Power", nullptr},
	{ 0x0C, true, ((float)(0.0f)), ((float)(1000.0f)),  &meter[1].reactivePower, UOM_RAW_FLOAT, "M2 - Reactive Power", nullptr},
	{ 0x0D, true, ((float)(0.0f)), ((float)(100.0f)),   &meter[1].frequency,     UOM_RAW_FLOAT, "M2 - Frequency",      nullptr},
	{ 0x0E, true, ((float)(0.0f)), ((float)(65000.0f)), &meter[1].importEnergy,  UOM_RAW_FLOAT, "M2 - Import Energy",  nullptr},
	
	{ 0x0F, true, ((float)(0.0f)), ((float)(300.0f)),   &meter[2].volatge,       UOM_RAW_FLOAT, "M3 - Voltage",        nullptr},
	{ 0x10, true, ((float)(0.0f)), ((float)(10.0f)),    &meter[2].current,       UOM_RAW_FLOAT, "M3 - Current",        nullptr},
	{ 0x11, true, ((float)(0.0f)), ((float)(1000.0f)),  &meter[2].activePower,   UOM_RAW_FLOAT, "M3 - Active Power",   nullptr},
	{ 0x12, true, ((float)(0.0f)), ((float)(1000.0f)),  &meter[2].apparentPower, UOM_RAW_FLOAT, "M3 - Apparent Power", nullptr},
	{ 0x13, true, ((float)(0.0f)), ((float)(1000.0f)),  &meter[2].reactivePower, UOM_RAW_FLOAT, "M3 - Reactive Power", nullptr},
	{ 0x14, true, ((float)(0.0f)), ((float)(100.0f)),   &meter[2].frequency,     UOM_RAW_FLOAT, "M3 - Frequency",      nullptr},
	{ 0x15, true, ((float)(0.0f)), ((float)(65000.0f)), &meter[2].importEnergy,  UOM_RAW_FLOAT, "M3 - Import Energy",  nullptr}
};
#define valueItemListSize (sizeof(valueItemList)/sizeof(vrp_valueItem_t))

valueReportProtocol_t valueReportProtocol =
{
	.value = valueItemList,
	.valueSize = valueItemListSize
};

#endif /* POWERMETERREAD_H_ */