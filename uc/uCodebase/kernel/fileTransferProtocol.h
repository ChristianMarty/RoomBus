
#ifndef FILE_TRANSFER_PROTOCOL_H_
#define FILE_TRANSFER_PROTOCOL_H_

#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/littleFS/lfs.h"


typedef enum {
	ftp_cmd_request  = 0,
	ftp_cmd_response = 1,

	ftp_cmd_read = 4,
    ftp_cmd_read_ack = 5,
	
    ftp_cmd_write = 6,
    ftp_cmd_write_ack = 7
}ftp_commands_t;

typedef enum {
	ftp_req_list = 0,
	ftp_req_makeFile = 8,
	ftp_req_deleteFile = 9,
	
	ftp_req_startFileRead = 0x20,
	ftp_req_endFileRead = 0x21,
	
	ftp_req_startFileWrite = 0x30,
	ftp_req_endFileWrite = 0x31
}ftp_reqest_t;

typedef enum {
	ftp_rsp_fileInfo = 0,
	
	ftp_rsp_startFileRead = 0x20,
	
	ftp_rsp_startFileWrite = 0x30,
	ftp_rsp_completeWrite = 0x31
}ftp_response_t;

typedef enum {
	ftp_rsps_ok = 0,
	ftp_rsps_pathInvalid = 1,
	
	ftp_rsps_writeSuccessful = 0x20,
	ftp_rsps_writeCrcError = 0x21,
	ftp_rsps_writeSequencingError = 0x22,
	
	ftp_rsps_readSuccessful = 0x30,
	ftp_rsps_readCrcError = 0x31,
	ftp_rsps_readSequencingError = 0x32
}ftp_responseStatus_t;


typedef struct{
	lfs_file_t file;
	uint32_t size;
	uint32_t offset;
	uint32_t crc;
	char path[50];
	bool pending;
	
}ftp_transfer_t;


void ftp_receiveHandler(lfs_t *lfs, uint8_t sourceAddress, uint8_t command, uint8_t *data, uint8_t size);

#endif /* FILE_TRANSFER_PROTOCOL_H_ */