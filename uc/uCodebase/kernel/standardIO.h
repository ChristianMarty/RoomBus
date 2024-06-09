/*
 * standardIO.h
 *
 * Created: 12.01.2019 21:37:40
 *  Author: Christian
 */ 

#ifdef __cplusplus
extern "C" {
#endif

#include "sam.h"
#include <stdbool.h>

void stdIO_init(void);

void stdIO_redLED(bool on);
void stdIO_greenLED(bool on);
void stdIO_blueLED(bool on);
void stdIO_setLedRGB(bool r, bool g, bool b);

bool stdIO_getButton(void);

#ifdef __cplusplus
}
#endif