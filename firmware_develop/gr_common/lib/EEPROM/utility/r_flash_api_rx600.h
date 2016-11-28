/*
 * r_flash_api_rx600.h
 *
 *  Created on: Apr 17, 2015
 *      Author: a5034000
 */

#ifndef R_FLASH_API_RX600_H_
#define R_FLASH_API_RX600_H_

#include "Arduino.h"

/**** Function Return Values ****/
/* Operation was successful */
#define FLASH_SUCCESS           (0x00)
/* Operation failed */
#define FLASH_FAILURE           (0x06)
/*  Bottom of DF Area */
#define DF_ADDRESS              0x00100000
/* Used for getting DF block */
#define DF_MASK                 0xFFFFF800
/* Used for getting DF block */
#define DF_BLOCK_MASK           0xFFFFFFE0
/* Used for getting erase boundary in DF block when doing blank checking */
#define DF_ERASE_BLOCK_SIZE     0x00000020
/* Used for programming/blank check DF align */
#define DF_ALIGN                (2)


#ifdef __cplusplus
extern "C" {
#endif

uint8_t flash_Initialize( void );
uint8_t flash_coderom_EraseBlock( const uint32_t addr );
uint8_t flash_datarom_EraseBlock( const uint32_t addr );
uint8_t flash_coderom_WriteData( const uint32_t addr, void* pData, const uint16_t nDataSize );
uint8_t flash_datarom_WriteData( const uint32_t addr, void* pData, const uint16_t nDataSize );
bool flash_datarom_blankcheck( const uint32_t addr );


#ifdef __cplusplus
}
#endif


#endif /* R_FLASH_API_RX600_H_ */
