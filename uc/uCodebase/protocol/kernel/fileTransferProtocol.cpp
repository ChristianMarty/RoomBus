//**********************************************************************************************************************
// FileName : fileTransferProtocol.cpp
// FilePath : protocol/kernel/
// Author   : Christian Marty
// Date		: 09.06.2024
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#include "protocol/kernel/fileTransferProtocol.h"
#include "protocol/kernel/messageLogProtocol.h"

#include "kernel/bus.h"

#include "utility/pack.h"
#include "utility/string.h"

#include "kernel/littleFS/lfs_util.h"
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


void ftp_initialize(void)
{
	
}

void ftp_handler(void)
{
	
}

void ftp_sendFileInfo(uint8_t address, lfs_info *info)
{
	bus_message_t msg;
	
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,address, busProtocol_fileTransferProtocol, ftp_cmd_response , busPriority_low);
	bus_pushByte(&msg,ftp_rsp_fileInfo);
	bus_pushByte(&msg,ftp_rsps_ok);
	
	switch (info->type) {
		case LFS_TYPE_REG: bus_pushByte(&msg,0x00); break;
		case LFS_TYPE_DIR: bus_pushByte(&msg,0x01); break;
		default:           bus_pushByte(&msg,0xFF); break;
	}
	
	bus_pushWord32(&msg, info->size);
	bus_pushString(&msg, info->name);
	bus_send(&msg);
}

void ftp_makeFile(uint8_t sourceAddress, lfs_t *lfs, const char *path)
{
	lfs_file_t file;
	int err = lfs_file_open(lfs, &file, path, LFS_O_EXCL | LFS_O_CREAT);
	lfs_file_close(lfs, &file);
	if(err) mlp_sysError("File error");
}

void ftp_deleteFile(uint8_t sourceAddress, lfs_t *lfs, const char *path)
{
	int err = lfs_remove(lfs, path);
	if(err) mlp_sysError("File error");
}

int ftp_list(uint8_t sourceAddress, lfs_t *lfs, const char *path)
{
    lfs_dir_t dir;
    int err = lfs_dir_open(lfs, &dir, path);
    if (err) {
        return err;
    }

    struct lfs_info info;
    while (true) {
        int res = lfs_dir_read(lfs, &dir, &info);
        if (res < 0) {
            return res;
        }

        if (res == 0) {
            break;
        }
		
		ftp_sendFileInfo(sourceAddress, &info);
    }

    err = lfs_dir_close(lfs, &dir);
    if (err) {
        return err;
    }

    return 0;
}


uint32_t ftp_fileCrc(lfs_t *lfs, const char *path)
{
	lfs_file_t file;
	
	lfs_file_open(lfs, &file, path, LFS_O_RDONLY);
	lfs_file_rewind(lfs, &file);
	
	uint32_t filesize = lfs_file_size(lfs, &file);
	
	uint32_t crc = 0xFFFFFFFF;
	uint8_t buffer[64];
	for(uint32_t i = 0; i < filesize; i+= 64)
	{
		uint8_t size = lfs_file_read(lfs, &file, buffer, 64);
		crc = lfs_crc(crc, buffer, size);
	}
	
	lfs_file_close(lfs, &file);
	
	return crc;
}


/*****************************************************************************
 * File Read
 * ***************************************************************************/
ftp_transfer_t readTransfer;

void ftp_startSendFile(uint8_t sourceAddress, lfs_t *lfs, const char *path)
{
	readTransfer.pending = true;
	
	bus_message_t msg;
	
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,sourceAddress, busProtocol_fileTransferProtocol, ftp_cmd_response, busPriority_low);
	bus_pushByte(&msg,ftp_rsp_startFileRead);
	
	readTransfer.crc = ftp_fileCrc(lfs,path);

    lfs_file_open(lfs, &readTransfer.file, path, LFS_O_RDONLY);
	lfs_file_rewind(lfs, &readTransfer.file);
	
	readTransfer.size = lfs_file_size(lfs, &readTransfer.file);
	readTransfer.offset = 0;
	
	bus_pushWord32(&msg, readTransfer.crc);
	bus_pushWord32(&msg, readTransfer.size);

	bus_send(&msg);
}

void ftp_fileTransmitHandler(uint8_t sourceAddress, lfs_t *lfs, uint8_t *data, uint8_t size)
{
	if(readTransfer.offset == readTransfer.size)
	{
		bus_message_t msg;
		
		bus_getMessageSlot(&msg);
		bus_writeHeader(&msg,sourceAddress, busProtocol_fileTransferProtocol, ftp_cmd_request, busPriority_low);
		bus_pushByte(&msg,ftp_req_endFileRead);
		bus_send(&msg);
		
		lfs_file_close(lfs, &readTransfer.file);
	}
	else
	{
		uint8_t buffer[56];
		int32_t txSize = lfs_file_read(lfs, &readTransfer.file, buffer, 56);
	
		// TODO: Error handling
	
		bus_message_t msg;
	
		bus_getMessageSlot(&msg);
		bus_writeHeader(&msg,sourceAddress, busProtocol_fileTransferProtocol, ftp_cmd_read, busPriority_low);
	
		bus_pushWord32(&msg, readTransfer.offset);
		bus_pushArray(&msg, buffer, txSize);
	
		bus_send(&msg);
	
		readTransfer.offset += txSize;
	}
	
	
}

void ftp_endTransmitFile(uint8_t sourceAddress, lfs_t *lfs)
{
	
}


/*****************************************************************************
 * File Write
 * ***************************************************************************/
ftp_transfer_t writeTransfer;

void ftp_startReceiveFile(uint8_t sourceAddress, lfs_t *lfs, uint8_t *data, uint8_t size)
{
	writeTransfer.pending = true;
	
	uint32_t fileCRC = unpack_uint32(&data[0]);
	uint32_t fileSize = unpack_uint32(&data[4]);
	
	uint8_t i;
	for(i= 0; i < size-8; i++)
	{
		writeTransfer.path[i] = data[i+8];
	}
	writeTransfer.path[i] = 0;
	
	lfs_file_open(lfs, &writeTransfer.file, writeTransfer.path, LFS_O_WRONLY | LFS_O_CREAT);
	
	writeTransfer.offset = 0;
	writeTransfer.size = fileSize;
	writeTransfer.crc = fileCRC;
	
	bus_message_t msg;
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,sourceAddress, busProtocol_fileTransferProtocol, ftp_cmd_response, busPriority_low);
	bus_pushByte(&msg,ftp_rsp_startFileWrite);
	bus_send(&msg);	
	
}

void ftp_fileReceiveHandler(uint8_t sourceAddress, lfs_t *lfs, uint8_t *data, uint8_t size)
{
	bus_message_t msg;
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,sourceAddress, busProtocol_fileTransferProtocol, ftp_cmd_write_ack, busPriority_low);
	
	uint32_t offset = unpack_uint32(&data[0]);
	
	if(writeTransfer.offset == offset)
	{
		bus_pushByte(&msg,ftp_rsps_ok);
		lfs_file_write(lfs, &writeTransfer.file, &data[4], size-4);
		
		writeTransfer.offset += size-4;
	}
	else
	{
		bus_pushByte(&msg,ftp_rsps_writeSequencingError);
	}
	
	bus_send(&msg);
}

void ftp_endReceiveFile(uint8_t sourceAddress, lfs_t *lfs)
{
	if(writeTransfer.pending != true) return; // TODO: Flag some error?
	
	lfs_file_close(lfs, &writeTransfer.file);
	
	uint32_t crc = ftp_fileCrc(lfs,writeTransfer.path);
	
	bus_message_t msg;
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,sourceAddress, busProtocol_fileTransferProtocol, ftp_cmd_response, busPriority_low);
	bus_pushByte(&msg,ftp_rsp_completeWrite);
	
	if(crc ==  writeTransfer.crc) bus_pushByte(&msg,ftp_rsps_writeSuccessful);
	else bus_pushByte(&msg,ftp_rsps_writeCrcError);
	
	bus_pushWord32(&msg, crc);
	bus_pushWord32(&msg, writeTransfer.size);
	
	bus_send(&msg);	
	
	writeTransfer.size = 0;
	writeTransfer.pending = false;
	
}


/*****************************************************************************
 * 
 * ***************************************************************************/

bool ftp_receiveHandler(lfs_t *lfs, bus_rxMessage_t *message)
{
	if(message->command == ftp_cmd_request) 
	{
		uint8_t request = message->data[0];
		char path[50];
		uint8_t i;
		for(i= 0; i < message->dataLength-1; i++)
		{
			path[i] = message->data[i+1];
		}
		path[i] = 0;
		
		switch(request)
		{
			case ftp_req_list: 		ftp_list(message->sourceAddress, lfs, &path[0]);
									return true; 
								
			case ftp_req_makeFile:	ftp_makeFile(message->sourceAddress, lfs, &path[0]);
									return true;
			
			case ftp_req_deleteFile: ftp_deleteFile(message->sourceAddress, lfs, &path[0]);
									 return true;
									 
			case ftp_req_startFileRead: ftp_startSendFile(message->sourceAddress, lfs, &path[0]);
										return true;
			
			
			// File Write 
			case ftp_req_startFileWrite: ftp_startReceiveFile(message->sourceAddress, lfs, &message->data[1], message->dataLength-1);
										 return true;
			
			case ftp_req_endFileWrite:	 ftp_endReceiveFile(message->sourceAddress,lfs);
										 return true;
										
		}
		
	}
	else if(message->command == ftp_cmd_write)
	{
		ftp_fileReceiveHandler(message->sourceAddress, lfs, message->data, message->dataLength);
		return true;
	}
	else if(message->command == ftp_cmd_read_ack)
	{
		ftp_fileTransmitHandler(message->sourceAddress, lfs, message->data, message->dataLength);
		return true;
	}
	
	return false;
}

#ifdef __cplusplus
}
#endif
