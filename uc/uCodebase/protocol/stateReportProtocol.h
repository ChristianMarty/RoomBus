
#ifndef STATE_REPORT_PROTOCOL_H_
#define STATE_REPORT_PROTOCOL_H_

#include "sam.h"
#include "kernel/kernel.h"

typedef enum {
	srp_cmd_group0Report  = 0,
	srp_cmd_group1Report  = 1,
	srp_cmd_individualStateReport = 4,
	srp_cmd_stateReportRequest = 5,
	srp_cmd_stateReportChannelNameReporting = 6,
	srp_cmd_stateReportChannelNameRequest= 7
}srp_commands_t;

typedef enum {
	srp_state_off = 0,
	srp_state_on = 1,
	srp_state_transitioning = 2,
	srp_state_undefined = 3
}srp_state_t;

typedef struct{
	const uint8_t channel;
	const char *description;
	srp_state_t * const state;
}srp_stateReportChannel_t;

typedef struct{
	const uint8_t sourceAddress;
	const uint8_t channel;
	srp_state_t * const state;
}srp_stateReport_t;

typedef struct{
	const srp_stateReport_t* states;
	const uint8_t stateSize;
	
	const srp_stateReportChannel_t* channels;
	const uint8_t channelSize;
}stateReportProtocol_t;

void srp_stateReportRequestAll(const kernel_t *kernel, const stateReportProtocol_t *srp);

void srp_parseGroup0Report(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp);
void srp_parseGroup1Report(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp);
void srp_parseIndividualStateReport(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp);

void srp_receiveHandler(const kernel_t *kernel, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp);

void srp_stateReportRequestHandler(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp);
void srp_stateReportChannelNameRequestHandler(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp);

void srp_sendGroupReport(const kernel_t *kernel, const stateReportProtocol_t *srp);

void tsp_onStateChange_callback(const kernel_t *kernel);

#endif /* STATE_REPORT_PROTOCOL_H_ */