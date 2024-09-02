
#include "ams_TSL2561.h"
#include "utility/pack.h"
#include "utility/string.h"

#define TSL2561_ADDRESS 0x39

void ams_TSL2561_init(const kernel_t *kernel, ams_TSL2561_t *tsl2561)
{
	tsl2561->i2c.init(kernel, SERCOM1, kernel->clk_1MHz,16,false); //tsl2561->sercom_p
	
	kernel->tickTimer.reset(&tsl2561->readTimer);
	
	tsl2561->state = tsl2561_state_t::tsl2561_state_pre_init;
}

void ams_TSL2561_handler(const kernel_t *kernel, ams_TSL2561_t *tsl2561)
{
	
	tsl2561->i2c.handler();
	
	if(tsl2561->i2c.hasError() )
	{
		tsl2561->state = tsl2561_state_t::tsl2561_state_error;
		
		kernel->log.error("I2C bus error");
		tsl2561->i2c.closeTransaction(tsl2561->i2cTransaction);
		tsl2561->i2c.reset();
	}
		
	if(!tsl2561->i2c.busy())
	{
		switch(tsl2561->state)
		{
			case tsl2561_state_t::tsl2561_state_pre_init:
			{
				// Read state
				kernel->log.message("I2C Initialization - Read");
				//tsl2561->i2cTransaction = tsl2561->i2c.transaction(TSL2561_ADDRESS,0xD0,nullptr,0,&tsl2561->rx[0],16,0);
				tsl2561->state = tsl2561_state_t::tsl2561_state_pre_init_pending;
				break;
			}
			case tsl2561_state_t::tsl2561_state_pre_init_pending :
			{
				tsl2561->i2c.closeTransaction(tsl2561->i2cTransaction);
				tsl2561->state = tsl2561_state_t::tsl2561_state_init;
				break;
			}
				
			case tsl2561_state_t::tsl2561_state_init: 
			{
				// Enable
				kernel->log.message("I2C Initialization - Write");
				tsl2561->tx[0]  = 0x03;
				tsl2561->i2cTransaction = tsl2561->i2c.transaction(TSL2561_ADDRESS,0xE0,&tsl2561->tx[0],1,0,0,0);
				tsl2561->state = tsl2561_state_t::tsl2561_state_init_pending;
				break;
			}
			case tsl2561_state_t::tsl2561_state_init_pending :
			{
				tsl2561->i2c.closeTransaction(tsl2561->i2cTransaction);
				tsl2561->state = tsl2561_state_t::tsl2561_state_idle;
				break;
			}
				
			case tsl2561_state_t::tsl2561_state_read:
			{
				//kernel->log.message("I2C read now");
				tsl2561->i2cTransaction = tsl2561->i2c.transaction(TSL2561_ADDRESS,0xD0,nullptr,0,&tsl2561->rx[0],16,0);
				tsl2561->state = tsl2561_state_t::tsl2561_state_read_pending;
				break;
			}	
		}
		
		if(kernel->tickTimer.delay1ms(&tsl2561->readTimer, 5000))
		{
			if (tsl2561->state == tsl2561_state_t::tsl2561_state_error)
			{
				tsl2561->state = tsl2561_state_t::tsl2561_state_pre_init;
			}
			
			if(tsl2561->state == tsl2561_state_t::tsl2561_state_idle)
			{
				tsl2561->state = tsl2561_state_t::tsl2561_state_read;
			}
		}	
	}
	
	if(tsl2561->i2c.getCurrentTransactionNumber() == tsl2561->i2cTransaction)
	{		 
		if(tsl2561->i2c.isUnreachable() )
		{
			tsl2561->state = tsl2561_state_t::tsl2561_state_pre_init;
			tsl2561->i2c.closeTransaction(tsl2561->i2cTransaction);
			
			kernel->log.error("I2C device is unreachable");
		}
		else if(!tsl2561->i2c.busy())
		{
			if(tsl2561->state == tsl2561_state_t::tsl2561_state_read_pending)
			{
				uint16_t ch0 = unpack_uint16_le(&tsl2561->rx[12]); // visible and infrared
				uint16_t ch1 = unpack_uint16_le(&tsl2561->rx[14]); // infrared
					
				uint32_t lux = ams_TSL2561_decode_illuminance(ch0, ch1);
				uint8_t brightness = ams_TSL2561_decode_brightness(ch0, ch1);
				tsl2561->measurmentData->illuminance = (float)lux;
				tsl2561->measurmentData->brightness = (float)brightness;
				
				if(tsl2561->onMeasurmentDataUpdate != nullptr) tsl2561->onMeasurmentDataUpdate(tsl2561->measurmentData);
				tsl2561->state = tsl2561_state_t::tsl2561_state_idle;
			}
		}
	}
}

void tsl2561_onMbInterrupt(ams_TSL2561_t *tsl2561)
{
	tsl2561->i2c.MbInterrupt();
}

void tsl2561_onSbInterrupt(ams_TSL2561_t *tsl2561)
{
	tsl2561->i2c.SbInterrupt();
}

void tsl2561_onErrorInterrupt(ams_TSL2561_t *tsl2561)
{
	tsl2561->i2c.ErrorInterrupt();
}

uint8_t ams_TSL2561_decode_brightness(uint16_t ch0, uint16_t ch1)
{
	uint16_t temp =  ch0-ch1;
	if(temp >100) temp = 100;
	return temp;
}



#define LUX_SCALE 14 // scale by 2^14
#define RATIO_SCALE 9 // scale ratio by 2^9
//???????????????????????????????????????????????????
// Integration time scaling factors
//???????????????????????????????????????????????????
#define CH_SCALE 10 // scale channel values by 2^10
#define CHSCALE_TINT0 0x7517 // 322/11 * 2^CH_SCALE
#define CHSCALE_TINT1 0x0fe7 // 322/81 * 2^CH_SCALE

#define K1T 0x0040 // 0.125 * 2^RATIO_SCALE
#define B1T 0x01f2 // 0.0304 * 2^LUX_SCALE
#define M1T 0x01be // 0.0272 * 2^LUX_SCALE
#define K2T 0x0080 // 0.250 * 2^RATIO_SCALE
#define B2T 0x0214 // 0.0325 * 2^LUX_SCALE
#define M2T 0x02d1 // 0.0440 * 2^LUX_SCALE
#define K3T 0x00c0 // 0.375 * 2^RATIO_SCALE
#define B3T 0x023f // 0.0351 * 2^LUX_SCALE
#define M3T 0x037b // 0.0544 * 2^LUX_SCALE
#define K4T 0x0100 // 0.50 * 2^RATIO_SCALE
#define B4T 0x0270 // 0.0381 * 2^LUX_SCALE
#define M4T 0x03fe // 0.0624 * 2^LUX_SCALE
#define K5T 0x0138 // 0.61 * 2^RATIO_SCALE
#define B5T 0x016f // 0.0224 * 2^LUX_SCALE
#define M5T 0x01fc // 0.0310 * 2^LUX_SCALE
#define K6T 0x019a // 0.80 * 2^RATIO_SCALE
#define B6T 0x00d2 // 0.0128 * 2^LUX_SCALE
#define M6T 0x00fb // 0.0153 * 2^LUX_SCALE
#define K7T 0x029a // 1.3 * 2^RATIO_SCALE
#define B7T 0x0018 // 0.00146 * 2^LUX_SCALE
#define M7T 0x0012 // 0.00112 * 2^LUX_SCALE
#define K8T 0x029a // 1.3 * 2^RATIO_SCALE
#define B8T 0x0000 // 0.000 * 2^LUX_SCALE
#define M8T 0x0000 // 0.000 * 2^LUX_SCALE

uint32_t ams_TSL2561_decode_illuminance(uint16_t ch0, uint16_t ch1)
{
	uint32_t chScale = (1 << CH_SCALE);
	chScale = chScale << 4; // scale 1X to 16X
	uint32_t channel0 = ((uint32_t)ch0 * chScale) >> CH_SCALE;
	uint32_t channel1 = ((uint32_t)ch1 * chScale) >> CH_SCALE;

	// find the ratio of the channel values (Channel1/Channel0)
	// protect against divide by zero
	uint32_t ratio1 = 0;
	if (channel0 != 0) ratio1 = (channel1 << (RATIO_SCALE+1)) / channel0;
	// round the ratio value
	uint32_t ratio = (ratio1 + 1) >> 1;
	
	uint16_t b, m;
	if ((ratio >= 0) && (ratio <= K1T))
	{b=B1T; m=M1T;}
	else if (ratio <= K2T)
	{b=B2T; m=M2T;}
	else if (ratio <= K3T)
	{b=B3T; m=M3T;}
	else if (ratio <= K4T)
	{b=B4T; m=M4T;}
	else if (ratio <= K5T)
	{b=B5T; m=M5T;}
	else if (ratio <= K6T)
	{b=B6T; m=M6T;}
	else if (ratio <= K7T)
	{b=B7T; m=M7T;}
	else if (ratio > K8T)
	{b=B8T; m=M8T;}

	
	uint32_t temp;
	temp = ((channel0 * b) - (channel1 * m));
	// do not allow negative lux value
	if (temp < 0) temp = 0;
	// round lsb (2^(LUX_SCALE?1))
	temp += (1 << (LUX_SCALE-1));
	// strip off fractional portion
	uint32_t lux = temp >> LUX_SCALE;
	return lux;
	
}

