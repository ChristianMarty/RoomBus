
#include "scioSense_APC1.h"

#define HEADER_0 0x42
#define HEADER_1 0x4D

#include "utility/pack.h"

void scioSense_apc1_init(const kernel_t *kernel, scioSense_apc1_t *apc1)
{
	apc1->uartCom.initUart(kernel->clk_16MHz, apc1->sercom_p, 9600, uart_c::none);
	kernel->tickTimer.reset(&apc1->readTimer);
}

uint8_t  measurementDataRequests[] = {HEADER_0,HEADER_1,0xE2,0x00,0x00,0x01,0x71};
	
void scioSense_apc1_handler(const kernel_t *kernel, scioSense_apc1_t *apc1)
{
	if(!apc1->uartCom.txBusy() && kernel->tickTimer.delay1ms(&apc1->readTimer, 5000))
	{
		apc1->rxIndex = 0;
		apc1->uartCom.sendData(measurementDataRequests, sizeof(measurementDataRequests));
	}
	
	if(apc1->rxIndex >= 64)
	{
		if(scioSense_apc1_decode_measurmentData(apc1->rxData, apc1->measurmentData))
		{
			if(apc1->onMeasurmentDataUpdate != nullptr) apc1->onMeasurmentDataUpdate(apc1->measurmentData);
		}
		apc1->rxIndex = 0;
	}
}


bool scioSense_apc1_decode_measurmentData(uint8_t *rxData, scioSense_apc1_measurmentData_t *measurmentData)
{
	if(rxData[0] != HEADER_0 && rxData[1] != HEADER_1 ) return false; // Header Error
	if(rxData[2] != 0 && rxData[3] != 0x3C ) return false; // Length Error
	
	if(rxData[0x3D] != 0) return false; // Error code 
	
	uint16_t checksum = unpack_uint16(&rxData[0x3E]);
	uint16_t tmp = 0;
	for(uint32_t i = 0; i < 0x3D; i++ )
	{
		tmp += rxData[i];
	}
	if(checksum != tmp) return false; // Checksum error 
	
	
	measurmentData->pm1_0 = (float) unpack_uint16(&rxData[0x04]);
	measurmentData->pm2_5 = (float) unpack_uint16(&rxData[0x06]);
	measurmentData->pm10  = (float) unpack_uint16(&rxData[0x08]);
	
	measurmentData->pm1_0Air = (float) unpack_uint16(&rxData[0x0A]);
	measurmentData->pm2_5Air = (float) unpack_uint16(&rxData[0x0C]);
	measurmentData->pm10Air  = (float) unpack_uint16(&rxData[0x0E]);
	
	measurmentData->count0_3um = (float) unpack_uint16(&rxData[0x10]);
	measurmentData->count0_5um = (float) unpack_uint16(&rxData[0x12]);
	measurmentData->count1_0um = (float) unpack_uint16(&rxData[0x14]);
	measurmentData->count2_5um = (float) unpack_uint16(&rxData[0x16]);
	measurmentData->count5_0um = (float) unpack_uint16(&rxData[0x18]);
	measurmentData->count10um  = (float) unpack_uint16(&rxData[0x1A]);
	
	measurmentData->tvoc  = (float) unpack_uint16(&rxData[0x1C]);
	measurmentData->eco2  = (float) unpack_uint16(&rxData[0x1E]);
	
	measurmentData->t_compensated  = ((float) unpack_uint16(&rxData[0x22])) /10;
	measurmentData->rh_compensated = ((float) unpack_uint16(&rxData[0x24])) /10;
	measurmentData->t_raw  = ((float) unpack_uint16(&rxData[0x26])) /10; 
	measurmentData->rh_raw = ((float) unpack_uint16(&rxData[0x28])) /10;
	
	measurmentData->airQualityIndex = (float) rxData[0x3A];
	
	return true;
}

void scioSense_apc1_onRx(scioSense_apc1_t *apc1)
{
	apc1->rxData[apc1->rxIndex] = apc1->uartCom.RxInterrupt();
	apc1->rxIndex++;	
}

void scioSense_apc1_onTx(scioSense_apc1_t *apc1)
{
	apc1->uartCom.TxInterrupt();
}



