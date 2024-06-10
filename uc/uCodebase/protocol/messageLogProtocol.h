//**********************************************************************************************************************
// FileName : messageLogProtocol.h
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 09.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef MESSAGE_LOG_PROTOCOL_H_
#define MESSAGE_LOG_PROTOCOL_H_

#include "kernel/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

void mlp_sysMessage(const char *message);
void mlp_sysWarning(const char *message);
void mlp_sysError(const char *message);
void mlp_appMessage(const char *message);
void mlp_appWarning(const char *message);
void mlp_appError(const char *message);

#ifdef __cplusplus
}
#endif

#endif /* MESSAGE_LOG_PROTOCOL_H_ */