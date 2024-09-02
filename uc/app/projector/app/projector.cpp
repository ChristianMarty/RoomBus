#include "projector.h"
#include "driver/SAMx5x/uart.h"
#include "driver/SAMx5x/pin.h"

uart_c uartCom;

void uart_onRx(void);
void uart_onTx(void);

uint32_t timeoutTimer;
uint32_t statusUpdateTimer;
uint32_t com_timer;

uint32_t projector_turnOnTimer;
uint32_t projector_turnOffTimer;

#define COM_TIMEOUT 200

#define TURN_OFF_TIME 105000
#define TURN_ON_TIME   30000

uint8_t rxBuffer[10];
uint8_t rxCounter;

uint8_t txBuffer[10];
uint8_t txCounter;

bool dataReady = false;

void projector_parseData(uint8_t *data, uint8_t size);

void uart_onRx(void)
{
	uint8_t data = uartCom.RxInterrupt();
	
	if(data == 0x02)
	{
		dataReady = false;
		rxCounter = 0;
	}
	else if(data == 0x03)
	{
		dataReady = true;
	}
	else
	{
		rxBuffer[rxCounter] = data;
		rxCounter++;
	}
}

void uart_onTx(void)
{
	uartCom.TxInterrupt();
}

void projector_sendCmd(const char *cmd)
{
	txCounter = 0;
	txBuffer[txCounter] = 0x02;
	
	while(cmd[txCounter] != 0)
	{
		txCounter++;
		txBuffer[txCounter] = cmd[(txCounter-1)];
		
		if(txCounter > 250) return;
	}
	
	txCounter++;
	txBuffer[txCounter] = 0x03;
	
	txCounter++;
	
	//while(uartCom.txBusy());
	uartCom.sendData(&txBuffer[0],txCounter);
}

projector_onOff_state_t onOff_state = projector_off;
projector_onOff_state_t onOff_state_old = projector_on;

typedef enum {
	projector_cmd_idel,
	projector_cmd_on,
	projector_cmd_off,
	projector_cmd_getOnOff,
	projector_cmd_blankOn,
	projector_cmd_blankOff,
	projector_cmd_getBlank,
	
	projector_cmd_size
}projector_cmd_t;

projector_cmd_t cmdCurrentState;
projector_cmd_t cmdLastState;

bool projector_powerState_old = true;

bool projector_powerState = false;
bool projector_blankState = false;
bool projector_online = false;

bool projector_statusRequest = false;

void projector_parseData(uint8_t *data, uint8_t size)
{
	switch(cmdLastState)
	{
		case projector_cmd_getOnOff:	if(data[0] == '0' && data[1] == '0' && data[2] == '0') projector_powerState = false;
										if(data[0] == '0' && data[1] == '0' && data[2] == '1') projector_powerState = true;
										break;
		
		case projector_cmd_getBlank:	if(data[0] == '0') projector_blankState = false;
										if(data[0] == '1') projector_blankState = true;
										break;
	}
	
	cmdLastState = cmdCurrentState;
}

void projector_init(void)
{
	uartCom.initUart(kernel.clk_16MHz, SERCOM5, 9600, uart_c::none);
	
	// Uart
	pin_enablePeripheralMux(PIN_PORT_B,16,PIN_FUNCTION_C);
	pin_enablePeripheralMux(PIN_PORT_B,17,PIN_FUNCTION_C);
	kernel.nvic.assignInterruptHandler(SERCOM5_2_IRQn,uart_onRx);
	kernel.nvic.assignInterruptHandler(SERCOM5_1_IRQn,uart_onTx);
	
	pin_enableOutput(IO_D17);
	pin_setOutput(IO_D17,true);
	
	kernel.tickTimer.reset(&timeoutTimer);
	kernel.tickTimer.reset(&statusUpdateTimer);
	kernel.tickTimer.reset(&com_timer);
	
	kernel.tickTimer.reset(&projector_turnOnTimer);
	kernel.tickTimer.reset(&projector_turnOffTimer);
	
	onOff_state = projector_off;
	onOff_state_old = projector_on;
	
	projector_powerState_old = true;

	projector_powerState = false;
	projector_blankState = false;
	projector_online = false;

	projector_statusRequest = false;
}

void projector_handler(void)
{
	if(dataReady) 
	{
		projector_parseData(&rxBuffer[0], rxCounter);
		dataReady = false;
		projector_statusRequest = false;
		projector_online = true;
	}
	
	if(projector_statusRequest && kernel.tickTimer.delay1ms(&timeoutTimer, COM_TIMEOUT))
	{
		 projector_online = false;
		 projector_statusRequest = false;
	}
	
	if(onOff_state == projector_cmd_off) projector_blankState = false;

	if(kernel.tickTimer.delay1ms(&com_timer, 100))
	{
		cmdLastState = cmdCurrentState;
		
		if(cmdCurrentState == projector_cmd_on)
		{
			projector_sendCmd("PON");
			cmdCurrentState = projector_cmd_getOnOff;
		}
		else if(cmdCurrentState == projector_cmd_off)
		{
			projector_sendCmd("POF");
			cmdCurrentState = projector_cmd_getOnOff;
		}
		else if(cmdCurrentState == projector_cmd_blankOn) 
		{
			if(!projector_blankState)projector_sendCmd("OSH");
			cmdCurrentState = projector_cmd_getBlank;
		}
		else if(cmdCurrentState == projector_cmd_blankOff)
		{
			if(projector_blankState)projector_sendCmd("OSH");
			cmdCurrentState = projector_cmd_getBlank;
		}
		else if(cmdCurrentState == projector_cmd_getBlank)
		{
			projector_sendCmd("QSH");
			cmdCurrentState = projector_cmd_idel;
		}
		else if(cmdCurrentState == projector_cmd_getOnOff)
		{
			projector_sendCmd("QPW");
			cmdCurrentState = projector_cmd_idel;
		}
	}
	
	if(cmdCurrentState == projector_cmd_idel)
	{
		if(kernel.tickTimer.delay1ms(&statusUpdateTimer, 5000))
		{
			projector_statusRequest = true;
			kernel.tickTimer.reset(&timeoutTimer);
			
			static uint8_t statusUpdateStep = 0;
			
			if(statusUpdateStep == 0)
			{
				cmdCurrentState = projector_cmd_getBlank;
				statusUpdateStep = 1;
			}
			else
			{
				cmdCurrentState = projector_cmd_getOnOff;
				statusUpdateStep = 0;
			}
		}
	}	
	
	if(projector_online)
	{
		if(cmdCurrentState == projector_cmd_idel)
		{
			if(onOff_state == projector_turnOff)
			{
				onOff_state = projector_turningOff;
				cmdCurrentState = projector_cmd_off;
			}
			else if(onOff_state == projector_turnOn)
			{
				onOff_state = projector_turningOn;
				cmdCurrentState = projector_cmd_on;
			}
		}
	
		if(projector_powerState == false) kernel.tickTimer.reset(&projector_turnOnTimer);
		if(projector_powerState == true)  kernel.tickTimer.reset(&projector_turnOffTimer);
	
		if((projector_powerState == true) && (projector_powerState_old == false))
		{
			if(kernel.tickTimer.delay1ms(&projector_turnOnTimer, TURN_ON_TIME))
			{
				onOff_state = projector_on;
				projector_powerState_old = projector_powerState;
			}
			else
			{
				onOff_state = projector_turningOn;
			}
		}
		else if((projector_powerState == false) && (projector_powerState_old == true))
		{
			if(kernel.tickTimer.delay1ms(&projector_turnOffTimer, TURN_OFF_TIME))
			{
				onOff_state = projector_switchOff;
				projector_powerState_old = projector_powerState;
			}
			else
			{
				onOff_state = projector_turningOff;
			}
		}

	}
	else if (onOff_state == projector_switchOn || onOff_state == projector_turnOn)
	{
		
	}
	else
	{
		onOff_state = projector_off;
		
		projector_powerState = false;
		projector_powerState_old = projector_powerState;
	}	
}

bool projector_getSwitchOn(void)
{
	if(onOff_state == projector_switchOn) 
	{
		onOff_state = projector_turnOn;
		return true;
	}
	
	return false;
}

bool projector_getSwitchOff(void)
{
	if(onOff_state == projector_switchOff)
	{
		onOff_state = projector_off;
		return true;
	}
	
	return false;
}

void projector_onOff(bool on)
{
	if(on) onOff_state = projector_switchOn;
	else onOff_state = projector_turnOff;
}

void projector_blank(bool on)
{
	if(on) cmdCurrentState = projector_cmd_blankOn;
	else cmdCurrentState = projector_cmd_blankOff;
}

projector_onOff_state_t projector_getOnOff(void)
{
	return onOff_state;	
}

bool projector_getBlank(void)
{
	return projector_blankState;	
}