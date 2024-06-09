/*
 * systemControl.h
 *
 * Created: 14.07.2019 12:11:06
 *  Author: Christian
 */ 

#ifndef DATASYSTEM_H_
#define DATASYSTEM_H_

#include "sam.h"
#include <stdbool.h>

#include "littleFS/lfs.h"
#include "littleFS/lfs_util.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "drv/SAMx5x/qspi.h"

uint32_t dataSystem_init(lfs_t *lfs);

const struct lfs_config* dataSystem_getConfig(void);

#ifdef __cplusplus
}
#endif


#endif /* DATASYSTEM_H_ */