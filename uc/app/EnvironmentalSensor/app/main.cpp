/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

#include "Raumsteuerung/stateReportProtocol.h"
#include "Raumsteuerung/triggerProtocol.h"
#include "Raumsteuerung/eventProtocol.h"
#include "Raumsteuerung/serialBridgProtocol.h"

#include "utility/string.h"

#include "peripherals/scioSense_APC1.h"
#include "peripherals/ams_TSL2561.h"
#include "Raumsteuerung/valueReportProtocol.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Environmental Sensor",
/*main		 */ main,
/*onRx		 */ onReceive
};

scioSense_apc1_measurmentData_t apc1_measurmentData;
ams_TSL2561_measurmentData_t tsl2561_measurmentData;
float brightness;

void sbp_transmit_port0(const uint8_t *data, uint8_t size);

const sbp_port_t serialPortList[] = {
	{sbp_type_uart, sbp_transmit_port0}
};
#define serialPortListSize (sizeof(serialPortList)/sizeof(sbp_port_t))

void sbp_transmit_port0(const uint8_t *data, uint8_t size)
{
}

const serialBridgeProtocol_t serialBridge = {
	.ports = serialPortList,
	.portSize = serialPortListSize
};

vrp_valueItem_t valueItemList[]={
	{ 0x01, true, ((float)(0.0f)),   ((float)(50.0f)),    &apc1_measurmentData.t_compensated, UOM_RAW_FLOAT, "Temperature",nullptr},
	{ 0x02, true, ((float)(0.0f)),   ((float)(100.0f)),   &apc1_measurmentData.rh_compensated, UOM_RAW_FLOAT, "Humidity",nullptr},
	{ 0x03, true, ((float)(0.0f)),   ((float)(500.0f)),   &apc1_measurmentData.pm1_0Air, UOM_RAW_FLOAT, "PM 1.0",nullptr},
	{ 0x04, true, ((float)(0.0f)),   ((float)(1000.0f)),  &apc1_measurmentData.pm2_5Air, UOM_RAW_FLOAT, "PM 2.5",nullptr},
	{ 0x05, true, ((float)(0.0f)),   ((float)(1500.0f)),  &apc1_measurmentData.pm10Air, UOM_RAW_FLOAT, "PM 10",nullptr},
	{ 0x06, true, ((float)(400.0f)), ((float)(65000.0f)), &apc1_measurmentData.eco2, UOM_RAW_FLOAT, "eCO2",nullptr},
	{ 0x07, true, ((float)(0.0f)),   ((float)(65000.0f)), &apc1_measurmentData.tvoc, UOM_RAW_FLOAT, "TVOC",nullptr},
	{ 0x08, true, ((float)(1.0f)),   ((float)(5.0f)),     &apc1_measurmentData.airQualityIndex, UOM_RAW_FLOAT, "Air Quality Index",nullptr},
	{ 0x09, true, ((float)(0.0f)),   ((float)(65000.0f)), &tsl2561_measurmentData.illuminance, UOM_RAW_FLOAT, "Illuminance",nullptr},
	{ 0x0A, true, ((float)(0.0f)),   ((float)(100.0f)),   &tsl2561_measurmentData.brightness, UOM_RAW_FLOAT, "Brightness",nullptr}
}; 
#define valueItemListSize (sizeof(valueItemList)/sizeof(vrp_valueItem_t))

valueReportProtocol_t valueReportProtocol=
{
	.value = valueItemList,
	.valueSize = valueItemListSize
};

void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{	
	switch(protocol)
	{
		/*case busProtocol_triggerProtocol:		tsp_receiveHandler(kernel, sourceAddress, command,data, size, &triggerSystem); break;
		case busProtocol_stateReportProtocol:	srp_receiveHandler(kernel,sourceAddress,command,data,size, &stateReport); break;
		case busProtocol_eventProtocol:			esp_receiveHandler(kernel, sourceAddress, command, data, size, &eventSystem); break;
		case busProtocol_serialBridgeProtocol:	sbp_receiveHandler(kernel, sourceAddress, command, data, size, &serialBridge); break;*/
		case busProtocol_valueReportProtocol:	vrp_receiveHandler(kernel,sourceAddress,command,data,size,&valueReportProtocol); break;
	}
}

void onMeasurmentDataUpdate(scioSense_apc1_measurmentData_t *measurmentData)
{
	vrp_sendValues(_kernel, &valueReportProtocol);
	
	/*_kernel->log.message("------------------------");
	char tmp0[40];
	
	{
		char tmp[20] = "T : ";
		_kernel->log.message(string_append(tmp, string_uInt16ToHex(measurmentData->t_compensated, tmp0)));
	}

	{
		char tmp[20] = "RH: ";
		_kernel->log.message(string_append(tmp, string_uInt16ToHex(measurmentData->rh_compensated, tmp0)));
	}
	
	{
		char tmp[20] = "PM 1.0: ";
		_kernel->log.message(string_append(tmp, string_uInt16ToHex(measurmentData->pm1_0, tmp0)));
	}
	
	{
		char tmp[20] = "PM 2.5: ";
		_kernel->log.message(string_append(tmp, string_uInt16ToHex(measurmentData->pm2_5, tmp0)));
	}
	
	{
		char tmp[20] = "PM 10: ";
		_kernel->log.message(string_append(tmp, string_uInt16ToHex(measurmentData->pm10, tmp0)));
	}
	
	//_kernel->log.message(string_append("RH:", string_uInt16ToHex(measurmentData->rh_compensated, tmp)));*/
}


ams_TSL2561_t tsl2561 = {
	.sercom_p = SERCOM1,
	.measurmentData = &tsl2561_measurmentData,
};

void i2c_onMbInterrupt(void)
{
	tsl2561_onMbInterrupt(&tsl2561);
}

void i2c_onSbInterrupt(void)
{
	tsl2561_onSbInterrupt(&tsl2561);
}

void i2c_onErrorInterrupt(void)
{
	tsl2561_onErrorInterrupt(&tsl2561);
}

scioSense_apc1_t apc1 = {
	.sercom_p = SERCOM5,
	.measurmentData = &apc1_measurmentData,
	.onMeasurmentDataUpdate = onMeasurmentDataUpdate
};

void uart_onRx(void)
{
	scioSense_apc1_onRx(&apc1);
}

void uart_onTx(void)
{
	scioSense_apc1_onTx(&apc1);
}

#define APC1_RESET IO_D14
#define APC1_SET IO_D15

int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		pin_enableOutput(APC1_SET);   // SET pin
		pin_enableOutput(APC1_RESET); // RESET pin
		
		pin_setOutput(APC1_RESET, false);
		pin_setOutput(APC1_SET,   true);
		pin_setOutput(APC1_RESET, true);
		
		scioSense_apc1_init(kernel, &apc1);
		kernel->nvic.assignInterruptHandler(SERCOM5_2_IRQn,uart_onRx);
		kernel->nvic.assignInterruptHandler(SERCOM5_1_IRQn,uart_onTx);
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
		
		ams_TSL2561_init(kernel, &tsl2561);
		pin_enablePeripheralMux(IO_D08, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D09, PIN_FUNCTION_C);
		kernel->nvic.assignInterruptHandler(SERCOM1_0_IRQn,i2c_onMbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM1_1_IRQn,i2c_onSbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM1_3_IRQn,i2c_onErrorInterrupt);
		
		
		//kernel->eemem_read(&ememData[0]);

		kernel->appSignals->appReady = true;
		
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		
		scioSense_apc1_handler(kernel, &apc1);
		ams_TSL2561_handler(kernel, &tsl2561);
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}