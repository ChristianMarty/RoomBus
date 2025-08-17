//**********************************************************************************************************************
// FileName : userIo.h
// FilePath : kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef USER_IO_H_
#define USER_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"

void userIo_initialize(void);
void userIo_handler(void);

void userIo_txLed(void);
void userIo_rxLed(void);

bool userIo_getButton(void);

#ifdef __cplusplus
}
#endif

#endif /* USER_IO_H_ */