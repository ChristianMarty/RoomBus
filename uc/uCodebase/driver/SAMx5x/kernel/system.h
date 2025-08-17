//**********************************************************************************************************************
// FileName : system.h
// FilePath : uCodebase/driver/SAMx5x/kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef SYSTEM_H_
#define SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"

void system_initialize(void);
void system_reboot(void);

static inline uint32_t system_deviceId(void)
{
	return DSU->DID.reg;
}

static inline uint32_t system_serialNumberWord0(void)
{
	return *(uint32_t*)0x008061FC;
}

static inline uint32_t system_serialNumberWord1(void)
{
	return *(uint32_t*)0x00806010; 
}

static inline uint32_t system_serialNumberWord2(void)
{
	return *(uint32_t*)0x00806014;
}

static inline uint32_t system_serialNumberWord3(void)
{
	return *(uint32_t*)0x00806018;
}

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_H_ */