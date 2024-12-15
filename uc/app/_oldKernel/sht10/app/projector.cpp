#include "projector.h"
#include "uart.h"
#include "pin.h"

uart_c uartCom;

void uart_onRx(void);
void uart_onTx(void);

uint32_t timeoutTimer;

void projector_init(kernel_t *kernel)
{
	gclk_peripheralEnable(GCLK_SERCOM5_CORE,kernel->clk_16MHz);
	MCLK->APBDMASK.bit.SERCOM5_ = true;
		
	uartCom.initUart(SERCOM5, 9600, uart_c::none);
		
	// Uart
	pin_enablePeripheralMux(PIN_PORT_B,16,PIN_FUNCTION_C);
	pin_enablePeripheralMux(PIN_PORT_B,17,PIN_FUNCTION_C);
	kernel->assignInterruptHandler(SERCOM5_2_IRQn,uart_onRx);
	kernel->assignInterruptHandler(SERCOM5_1_IRQn,uart_onTx);	
	
	pin_enableOutput(IO_D17);
	pin_setOutput(IO_D17,true);
	
	kernel->tickTimer_reset(&timeoutTimer);
}

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

projector_onOff_state_t onOff_state;
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

bool onOffState;
bool blankState;

void projector_parseData(uint8_t *data, uint8_t size)
{
	switch(cmdLastState)
	{
		case projector_cmd_getOnOff:	if(data[0] == '0' && data[1] == '0' && data[2] == '0') onOffState = 0;
										if(data[0] == '0' && data[1] == '0' && data[2] == '1') onOffState = 1;
										break;
		
		case projector_cmd_getBlank:	if(data[0] == '0') blankState = 0;
										if(data[0] == '1') blankState = 1;
										break;
	}
	
	cmdLastState = cmdCurrentState;
}

void projector_handler(kernel_t *kernel)
{
	if(dataReady) 
	{
		projector_parseData(&rxBuffer[0], rxCounter);
		dataReady = false;
	}
	
	if(kernel->tickTimer_delay1ms(&timeoutTimer, 100))
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
			if(!blankState)projector_sendCmd("OSH");
			cmdCurrentState = projector_cmd_getBlank;
			
		}
		else if(cmdCurrentState == projector_cmd_blankOff)
		{
			if(blankState)projector_sendCmd("OSH");
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
}

void projector_onOff(bool on)
{
	if(on) cmdCurrentState = projector_cmd_on;
	else cmdCurrentState = projector_cmd_off;
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