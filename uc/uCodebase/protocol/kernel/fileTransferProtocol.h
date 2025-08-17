//**********************************************************************************************************************
// FileName : fileTransferProtocol.h
// FilePath : protocol/kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef FILE_TRANSFER_PROTOCOL_H_
#define FILE_TRANSFER_PROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"
#include "kernel/littleFS/lfs.h"

void ftp_initialize(void);
void ftp_handler(void);
bool ftp_receiveHandler(lfs_t *lfs, bus_rxMessage_t *message);

#ifdef __cplusplus
}
#endif

#endif /* FILE_TRANSFER_PROTOCOL_H_ */