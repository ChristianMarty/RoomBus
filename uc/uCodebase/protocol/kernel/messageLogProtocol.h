//**********************************************************************************************************************
// FileName : messageLogProtocol.h
// FilePath : protocol/kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef MESSAGE_LOG_PROTOCOL_H_
#define MESSAGE_LOG_PROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"

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