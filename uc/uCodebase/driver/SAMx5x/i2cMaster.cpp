

#include "i2cMaster.h"

#include "sercom.h"


void i2cMaster_c::init(Sercom *sercom_p, gclk_generator_t clkGenerator, uint8_t clockDevisor, bool fourWire)
{
	sercom_initClock(clkGenerator,sercom_p);

	_sercom_p = sercom_p;
	_clkGenerator = clkGenerator;
	
	_sercom_p->I2CM.CTRLA.bit.SWRST = true; // Reset
	
	_status = i2cMaster_status_t::idel;
	_clockDevisor = clockDevisor;
	
	while(_sercom_p->I2CM.SYNCBUSY.bit.SWRST);
	
	_sercom_p->I2CM.CTRLA.bit.MODE = 0x05; // SERCOM as I2C Master
	_sercom_p->I2CM.CTRLA.bit.PINOUT = fourWire;
	
	_sercom_p->I2CM.CTRLB.bit.SMEN = true;
	_sercom_p->I2CM.BAUD.bit.BAUD = clockDevisor;
	_sercom_p->I2CM.INTENSET.reg = 0x83;
	_sercom_p->I2CM.CTRLA.bit.ENABLE = true;
	_sercom_p->I2CM.STATUS.bit.BUSSTATE = 0x01; // Set state to idel
	
	
	/*kernel->nvic.assignInterruptHandler(SERCOM1_0_IRQn,(interruptHandler_t)&i2cMaster_c::MbInterrupt);
	kernel->nvic.assignInterruptHandler(SERCOM1_1_IRQn,(interruptHandler_t)&i2cMaster_c::SbInterrupt);
	kernel->nvic.assignInterruptHandler(SERCOM1_3_IRQn,(interruptHandler_t)&i2cMaster_c::ErrorInterrupt);//*/
	
	kernel.tickTimer.reset(&_timeoutTimer);
}

void i2cMaster_c::handler(void)
{
	if(kernel.tickTimer.delay1ms(&_timeoutTimer,I2C_MASTER_TIMEOUT))
	{
		if(_status != i2cMaster_status_t::idel) _status = i2cMaster_status_t::error;
	}
}

bool i2cMaster_c::busy(void)
{
	return (_status != i2cMaster_status_t::idel);
}

i2cMaster_c::i2c_transactionNumber_t i2cMaster_c::getCurrentTransactionNumber(void)
{
	return _transactionNumber;
}

i2cMaster_c::i2c_transactionNumber_t i2cMaster_c::getNextTransactionNumber(void)
{
	_lastTransactionNumber ++;
	if(_lastTransactionNumber == 0) _lastTransactionNumber ++;;
	
	return _lastTransactionNumber;
}

void i2cMaster_c::reset(void)
{
	init(_sercom_p,_clkGenerator,_clockDevisor,false);
}

void i2cMaster_c::resetError(void)
{
	if(_status == i2cMaster_status_t::error)_status = i2cMaster_status_t::idel;
}

bool i2cMaster_c::isUnreachable(void)
{
	if(_status == i2cMaster_status_t::unreachable) return true;
	else return false;
}

bool i2cMaster_c::hasError(void)
{
	if(_status == i2cMaster_status_t::error) return true;
	else return false;
}

void i2cMaster_c::closeTransaction(i2c_transactionNumber_t transactionNumber)
{
	if(_transactionNumber != transactionNumber) return;
	
	_transactionNumber = 0;
	_status = i2cMaster_status_t::idel;	
}

i2cMaster_c::i2c_transactionNumber_t i2cMaster_c::transaction(uint8_t address, uint8_t command, uint8_t* txData, uint8_t txSize, uint8_t* rxData, uint8_t rxSize, i2c_transactionNumber_t transactionNumber)
{
	if(_sercom_p->I2CM.STATUS.bit.BUSSTATE == 0x01)_status = i2cMaster_status_t::idel;
	
	if(_status == i2cMaster_status_t::unreachable) _status = i2cMaster_status_t::idel;
	if(_status != i2cMaster_status_t::idel) return 0;
	
	_command = command;
	_txData = txData;
	_txDataSize = txSize;
	_rxData = rxData;
	_rxDataSize = rxSize;
	_address = address;
	
	if(transactionNumber != 0) _transactionNumber = transactionNumber;
	else _transactionNumber = getNextTransactionNumber();
	
	_sercom_p->I2CM.CTRLB.bit.ACKACT = true; // No Acknolegment
	_sercom_p->I2CM.ADDR.bit.ADDR = (_address<<1);
	_status = i2cMaster_status_t::txCommand;
	
	return _transactionNumber;
}

void i2cMaster_c::MbInterrupt(void)
{
	kernel.tickTimer.reset(&_timeoutTimer);
	
	if(_sercom_p->I2CM.STATUS.bit.RXNACK)
	{
		if(_status == i2cMaster_status_t::txCommand) _status = i2cMaster_status_t::unreachable;
		else _status = i2cMaster_status_t::error;
		
		_sercom_p->I2CM.INTFLAG.bit.MB = true;
		_sercom_p->I2CM.CTRLB.bit.CMD = 0x03; // Send Stop
	}
	else if(_status == i2cMaster_status_t::txCommand)
	{
		_sercom_p->I2CM.DATA.reg = _command;
		_bufferPos = 0;
		_status = i2cMaster_status_t::txData;
	}
	else if(_status == i2cMaster_status_t::txData)
	{
		if(_bufferPos < _txDataSize)
		{
			_sercom_p->I2CM.DATA.reg = _txData[_bufferPos];
			_bufferPos ++;
		}
		else
		{
			if(_rxDataSize != 0)
			{
				_sercom_p->I2CM.CTRLB.bit.CMD = 0x01; // Send repeated start
				_sercom_p->I2CM.ADDR.bit.ADDR = (_address<<1)|0x01;
				
				//_sercom_p->I2CM.CTRLB.bit.CMD = 0x02;
				_status = i2cMaster_status_t::rxData;
				//_rxData[_bufferPos] = _sercom_p->I2CM.DATA.reg;
				_bufferPos = 0;
				
			}
			else
			{
				_status = i2cMaster_status_t::idel;
				_sercom_p->I2CM.CTRLB.bit.CMD = 0x03; // Send Stop
				
			}
		}
	}
	else if(_status == i2cMaster_status_t::idel)
	{
		_sercom_p->I2CM.INTFLAG.bit.MB = 1;
	}
	
}

void i2cMaster_c::SbInterrupt(void)
{
	kernel.tickTimer.reset(&_timeoutTimer);
	
	if(_status == i2cMaster_status_t::rxData)
	{
		if(_bufferPos < (_rxDataSize-1))
		{
			_rxData[_bufferPos] = _sercom_p->I2CM.DATA.reg;
			_bufferPos++;
			_sercom_p->I2CM.CTRLB.bit.ACKACT = false;  // Ack
			_sercom_p->I2CM.CTRLB.bit.CMD = 0x02;
		}
		else
		{
			//_sercom_p->I2CM.CTRLB.bit.ACKACT = false;  // Ack
			_status = i2cMaster_status_t::idel;
		}
		
	}
	else if(_status == i2cMaster_status_t::idel)
	{
		_sercom_p->I2CM.CTRLB.bit.ACKACT = true; // NAck
		_sercom_p->I2CM.CTRLB.bit.CMD = 0x03; // Send Stop
		_rxData[_bufferPos] =_sercom_p->I2CM.DATA.reg;
	}
}

void i2cMaster_c::ErrorInterrupt(void)
{
	init(_sercom_p,_clkGenerator,_clockDevisor,false);
}
