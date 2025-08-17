//**********************************************************************************************************************
// FileName : genericClockController.c
// FilePath : uCodebase/driver/SAMx5x/
// Author   : Christian Marty
// Date		: 03.02.2019
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef GENERIC_CLOCK_CONTROLLER_H_
#define GENERIC_CLOCK_CONTROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"

typedef enum {
	GCLK_SRC_XOSC0,
	GCLK_SRC_XOSC1,
	GCLK_SRC_GCLK_IN,
	GCLK_SRC_GCLK_GEN1,
	GCLK_SRC_OSCULP32K,
	GCLK_SRC_XOSC32K,
	GCLK_SRC_DFLL,
	GCLK_SRC_DPLL0,
	GCLK_SRC_DPLL1,
	
	GCLK_SRC_SIZE
}gclk_source_t;

typedef enum{
	GCLK_OSCCTRL_DFLL48,
	GCLK_OSCCTRL_FDPLL0,
	GCLK_OSCCTRL_FDPLL1,
	GCLK_OSCCTRL_VARIOS,
	GCLK_EIC,
	GCLK_FREQM_MSR,
	GCLK_FREQM_REF,
	GCLK_SERCOM0_CORE,
	GCLK_SERCOM1_CORE,
	GCLK_TC0_TC1,
	GCLK_USB,
	GCLK_EVSYS0,
	GCLK_EVSYS1,
	GCLK_EVSYS2,
	GCLK_EVSYS3,
	GCLK_EVSYS4,
	GCLK_EVSYS5,
	GCLK_EVSYS6,
	GCLK_EVSYS7,
	GCLK_EVSYS8,
	GCLK_EVSYS9,
	GCLK_EVSYS10,
	GCLK_EVSYS11,
	GCLK_SERCOM2_CORE,
	GCLK_SERCOM3_CORE,
	GCLK_TCC0_TCC1,
	GCLK_TC2_TC3,
	GCLK_CAN0,
	GCLK_CAN1,
	GCLK_TCC2_TCC3,
	GCLK_TC4_TC5,
	GCLK_PDEC,
	GCLK_AC,
	GCLK_CCL,
	GCLK_SERCOM4_CORE,
	GCLK_SERCOM5_CORE,
	GCLK_SERCOM6_CORE,
	GCLK_SERCOM7_CORE,
	GCLK_TCC4,
	GCLK_TC6_TC7,
	GCLK_ADC0,
	GCLK_ADC1,
	GCLK_DAC, 
	GCLK_I2S0,
	GCLK_I2S1,
	GCLK_SDHC0,
	GCLK_SDHC1,
	GCLK_CM4_TRACE,
	
	GCLK_PERIPHERAL_SIZE	 
}gclk_peripheral_t;

typedef enum{
	GCLK_GENERATOR_0,
	GCLK_GENERATOR_1,
	GCLK_GENERATOR_2,
	GCLK_GENERATOR_3,
	GCLK_GENERATOR_4,
	GCLK_GENERATOR_5,
	GCLK_GENERATOR_6,
	GCLK_GENERATOR_7,
	GCLK_GENERATOR_8,
	GCLK_GENERATOR_9,
	GCLK_GENERATOR_10,
	GCLK_GENERATOR_11,
	
	GCLK_GENERATOR_SIZE
}gclk_generator_t;

typedef enum{
	GCLK_ERROR_noError,
	GCLK_ERROR_clkSourceOutOfRange,
	GCLK_ERROR_generatorOutOfRange,
	GCLK_ERROR_peripheralChannelOutOfRange,
	GCLK_ERROR_generatorAlreadyUsed,
	GCLK_ERROR_peripheralChannelAlreadyUsed,
	
	GCLK_ERROR_SIZE
}gclk_error_t;

void gclk_reset(void);
gclk_generator_t gclk_getFreeGenerator(void);
gclk_error_t gclk_generatorEnable(gclk_generator_t generator, gclk_source_t clkSource, uint16_t divisonFactor);
gclk_error_t glck_generatorDisable(gclk_generator_t generator);
gclk_error_t gclk_peripheralEnable(gclk_peripheral_t peripheralChannel,gclk_generator_t clkGenerator);
gclk_error_t gclk_peripheralDisable(gclk_peripheral_t peripheralChannel);

#ifdef __cplusplus
}
#endif

#endif /* GENERIC_CLOCK_CONTROLLER_H_ */