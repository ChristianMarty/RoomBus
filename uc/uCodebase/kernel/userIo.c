//**********************************************************************************************************************
// FileName : userIo.c
// FilePath : kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
	
#include "userIo.h"
#include "main.h"

#include "kernel/systemControl.h"
#include "driver/SAMx5x/kernel/tickTimer.h"

extern systemControl_t sysControlHandler;

union {
	struct {
		uint8_t red : 1;
		uint8_t green : 1;
		uint8_t blue : 1;
		uint8_t blinking : 1;
	} bit;
	uint8_t byte;
}ledState;

tickTimer_t taskTimer;
tickTimer_t redLedTimer;
tickTimer_t greenLedTimer;

uint8_t administratorAccessButtonCounter = 0;

void userIo_initialize(void)
{
	LedRedDisable();
	LedGreenDisable();
	LedGreenDisable();
	
	ledState.byte = 0;

	LedInitialization(); // Set pins for LED as output
	AabInitialization(); // enable input for Button pin
}

void userIo_handler(void)
{
	if(!AabRead()){
		administratorAccessButtonCounter = 0;
	}
	
	if(tickTimer_delay1ms(&taskTimer, 500)){
		
		if(AabRead()){
			administratorAccessButtonCounter++;
		}
		
		if(administratorAccessButtonCounter > 6) {
			sysControlHandler.sysStatus.bit.administratorAccess = true;
		}
		
		if(sysControlHandler.sysStatus.bit.identify){
			ledState.bit.blinking = !ledState.bit.blinking;
			if(ledState.bit.blinking){
				LedRedEnable();
				LedGreenEnable();
				LedBlueEnable();
			}else{
				LedRedDisable();
				LedGreenDisable();
				LedBlueDisable();
			}
		}
	}
	
	if(sysControlHandler.sysStatus.bit.identify) return; // <- return --------------------------------------------
		
	if(!sysControlHandler.sysControl.bit.userLedEnabled){
		LedRedDisable();
		LedGreenDisable();
		if(sysControlHandler.sysStatus.bit.administratorAccess) LedBlueEnable();
		else LedBlueDisable();
		return;
	}
	
	if(tickTimer_delay1ms(&redLedTimer,10)){
		ledState.bit.red = false;
	}
			
	if(tickTimer_delay1ms(&greenLedTimer,10)){
		ledState.bit.green = false;
	}
	
	ledState.bit.blue = sysControlHandler.sysStatus.bit.administratorAccess;
	
	if(ledState.bit.red){
		LedRedEnable();
		ledState.bit.blue = false;
	}else{
		LedRedDisable();
	}
	
	if(ledState.bit.green){
		LedGreenEnable();
		ledState.bit.blue = false;
	}else{
		LedGreenDisable();
	}

	if(ledState.bit.blue){
		LedBlueEnable();
	}else{
		LedBlueDisable();
	}
}

void userIo_txLed(void)
{
	tickTimer_reset(&redLedTimer);
	ledState.bit.red = true;
}

void userIo_rxLed(void)
{
	tickTimer_reset(&greenLedTimer);
	ledState.bit.green = true;
}

#ifdef __cplusplus
}
#endif