
#ifndef MESSAGE_PROTOCOL_H_
#define MESSAGE_PROTOCOL_H_

#include "sam.h"
#include "kernel.h"

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


#endif /* MESSAGE_PROTOCOL_H_ */