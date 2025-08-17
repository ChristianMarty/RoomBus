//**********************************************************************************************************************
// FileName : dataSystem.c
// FilePath : kernel/
// Author   : Christian Marty
// Date		: 30.07.2020
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
	
#include "dataSystem.h"

#include "protocol/kernel/messageLogProtocol.h"
#include "utility/string.h"

#include "littleFS/lfs_util.h"
#include "driver/SAMx5x/kernel/qspi.h"

int ds_mem_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int ds_mem_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int ds_mem_erase(const struct lfs_config *c, lfs_block_t block);
int ds_mem_sync(const struct lfs_config *c);

#define FLASH_DEVICE_ID 0x1840

const struct lfs_config cfg = {
	// block device operations
	.read  = ds_mem_read,
	.prog  = ds_mem_prog,
	.erase = ds_mem_erase,
	.sync  = ds_mem_sync,

	// block device configuration
	.read_size = 64,
	.prog_size = 64,
	.block_size = QSPI_BLOCK_SIZE,
	.block_count = 0x0FFF,
	.cache_size = 64,
	.lookahead_size = 64,
	.block_cycles = 500,
};

const struct lfs_config* dataSystem_getConfig(void)
{
	return &cfg;
}

void reportBootCounter(uint32_t bootCount)
{
	char bootCountStr[] = "Boot Counter: _____";
	string_fromInt32(bootCount, 0, &bootCountStr[14]);
	mlp_sysMessage(bootCountStr);
}

void dataSystem_initialize(lfs_t *lfs)
{
	qspi_init();
	
	qspi_resetDevice();
	
	uint16_t deviceId = qspi_readJedecId();
	
	if(deviceId != FLASH_DEVICE_ID){
		char deviceIdString[] = "QSPI device-id not supported: 0x____";
		string_uInt16ToHex(deviceId, &deviceIdString[32]);
		mlp_sysMessage(deviceIdString);
		
		return;
	}

	qspi_writeStatus2(0x02); // Enable QSPI Mode in Flash
	
	uint16_t i = 0;
	while(qspi_busy()){
		if(i>0x0FFF){
			mlp_sysError("QSPI flash timeout. File System not available!");
			return; 	
		}
		i++;
	}
	
	qspi_readStatus1();
	qspi_readStatus2();
	
	//qspi_sectorErase((uint8_t*)QSPI_START_ADDRESS);
	
	// mount the filesystem
	int err = lfs_mount(lfs, &cfg);

	// reformat if we can't mount the filesystem
	// this should only happen on the first boot
	if (err != LFS_ERR_OK) {
		mlp_sysWarning("Reformat file system. This should only happen on first boot!");
	//	qspi_chipErase();
	//	while(qspi_busy());
		lfs_format(lfs, &cfg);
		lfs_mount(lfs, &cfg);
	}
	
	
	// read current count
	lfs_file_t file;
    uint32_t boot_count = 0;
    err = lfs_file_open(lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
	
	if(err != LFS_ERR_OK)
	{
		mlp_sysError("Mounting error. File System not available!");
		 return;
	}
	
    lfs_file_read(lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(lfs, &file);
    lfs_file_write(lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(lfs, &file);
	
	//lfs_unmount(lfs);
	
	reportBootCounter(boot_count);
}

// Read a region in a block. Negative error codes are propagated
// to the user.
int ds_mem_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	uint32_t dataPtr = QSPI_START_ADDRESS+(QSPI_BLOCK_SIZE* block)+off;
	qspi_read((uint8_t*)dataPtr, (uint8_t*) buffer, size);
	return LFS_ERR_OK;
}

// Program a region in a block. The block must have previously
// been erased. Negative error codes are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int ds_mem_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	uint32_t dataPtr = QSPI_START_ADDRESS+(QSPI_BLOCK_SIZE* block)+off;
	qspi_write((uint8_t*)dataPtr, (uint8_t*) buffer, size);
	
	while(qspi_busy());
	return LFS_ERR_OK;
}

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int ds_mem_erase(const struct lfs_config *c, lfs_block_t block)
{
	uint32_t dataPtr = QSPI_START_ADDRESS+(QSPI_BLOCK_SIZE* block);
	qspi_sectorErase((uint8_t*)dataPtr);
	
	while(qspi_busy());
	return LFS_ERR_OK;
}

// Sync the state of the underlying block device. Negative error codes
// are propagated to the user.
int ds_mem_sync(const struct lfs_config *c)
{
	return LFS_ERR_OK;
}

#ifdef __cplusplus
}
#endif