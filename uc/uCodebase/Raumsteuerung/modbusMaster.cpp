
#include "modbusMaster.h"
#include "drv/SAMx5x/pin.h"
#include "utility/softCRC.h"

#include "utility/string.h"

void modbusMaster_rxInterrupt(modbusMaster_t *modbusMaster)
{
	rs485_rxInterrupt(&modbusMaster->rs485);
}

void modbusMaster_txInterrupt(modbusMaster_t *modbusMaster)
{
	rs485_txInterrupt(&modbusMaster->rs485);
}

void modbusMaster_init(const kernel_t *kernel, modbusMaster_t *modbusMaster, Sercom *sercom_p)
{
	bytearray_init(&modbusMaster->txBuffer, &(modbusMaster->txData[0]), sizeof(modbusMaster->txData));
	bytearray_init(&modbusMaster->rxBuffer, &(modbusMaster->rxData[0]), sizeof(modbusMaster->rxData));
	
	rs485_init(kernel,&modbusMaster->rs485, kernel->clk_16MHz,sercom_p, 2400, sercom_parity_t::sercom_parity_none);
	
	modbusMaster->rs485.tx = &modbusMaster->txBuffer;
	modbusMaster->rs485.rx = &modbusMaster->rxBuffer;
		
	kernel->tickTimer.reset(&modbusMaster->com_timer);		
	
	modbusMaster->readings = nullptr;
	modbusMaster->readings_size = 0;
}

void modbusMaster_attachReadings(const kernel_t *kernel, modbusMaster_t *modbusMaster, 	modbusMaster_reading_t *readings, uint8_t readings_size)
{
	modbusMaster->readings = readings;
	modbusMaster->readings_size = readings_size;
	modbusMaster->readings_index = 0;
	
	for(uint8_t i = 0; i<readings_size; i++)
	{
		kernel->tickTimer.reset(&readings[i]._timer);	
	}
}

bool modbusMaster_send(const kernel_t *kernel, modbusMaster_t *modbusMaster, uint8_t address, modbusMaster_functionCode_t functionCode, const bytearray_t* data)
{
	bytearray_clear(&modbusMaster->txBuffer);
	bytearray_pushByte(&modbusMaster->txBuffer, address);
	bytearray_pushByte(&modbusMaster->txBuffer, functionCode);
	bytearray_pushBytearray(&modbusMaster->txBuffer, data);
	
	uint16_t crc = crcModbus_bytearray(&modbusMaster->txBuffer);
	bytearray_pushU16_LE(&modbusMaster->txBuffer, crc);
	
	modbusMaster->rs485.rxSize = 9;
	
	rs485_send(kernel, &modbusMaster->rs485);
	kernel->tickTimer.reset(&modbusMaster->com_timer);
}

void modbusMaster_handler(const kernel_t *kernel, modbusMaster_t *modbusMaster)
{
	rs485_handler(kernel, &modbusMaster->rs485);
	static uint32_t lastReadingIndex;
	
	if(kernel->tickTimer.delay1ms(&modbusMaster->com_timer, MODBUS_MASTER_TIMEOUT)) {
		if(rs485_busy(&modbusMaster->rs485)) rs485_reset(&modbusMaster->rs485);
	}
	
	if(modbusMaster->readings_size > 0 && !rs485_busy(&modbusMaster->rs485))
	{
		modbusMaster_reading_t *reading = &modbusMaster->readings[modbusMaster->readings_index];	
		
		if(kernel->tickTimer.delay1ms(&reading->_timer, reading->interval))
		{
			lastReadingIndex = modbusMaster->readings_index;
			
			modbusMaster->readings_index ++;
			if(modbusMaster->readings_size == modbusMaster->readings_index){
				modbusMaster->readings_index = 0;
			}
			
			bytearray_t ba;
			uint8_t data[] = {0x00,reading->registerAddress,0x00,2};
			bytearray_init(&ba,&data[0],sizeof(data));
			ba.size = sizeof(data);
				
			modbusMaster_send(kernel, modbusMaster, reading->deviceAddress, modbusMaster_fc_readInputRegister, &ba);
		}
	}
	
	if(rs485_rxCompleted(&modbusMaster->rs485))
	{
		modbusMaster_reading_t *lastReading = &modbusMaster->readings[lastReadingIndex];	
		
		uint32_t raw = bytearray_decodeU32_BE(modbusMaster->rs485.rx, 3);
		float value = *((float*)&raw);
		*lastReading->reading = value;
		
		if(lastReading->callback != nullptr)lastReading->callback(lastReading);
	}
	
}


