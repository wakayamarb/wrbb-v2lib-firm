/*
  EEPROM.cpp - EEPROM library
  Copyright (c) 2006 David A. Mellis.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/* Modified 20 April 2015 by Yuuki Okamiya for GR-SAKURA */

/******************************************************************************
 * Includes
 ******************************************************************************/

#ifndef __RX600__
#include <avr/eeprom.h>
#else
#include "utility/r_flash_api_rx600.h"
#endif
#include "Arduino.h"
#include "EEPROM.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/

EEPROMClass::EEPROMClass(){
    flash_Initialize();
}
uint8_t EEPROMClass::read(int address)
{
#ifndef __RX600__
	return eeprom_read_byte((unsigned char *) address);
#else
	if(flash_datarom_blankcheck(DF_ADDRESS + (address & 0xfffffffe)) == 0)
	    //blank
	    return 0xff;
	return *(volatile unsigned char *)(address + DF_ADDRESS);
#endif //__RX600__
}

uint8_t EEPROMClass::write(int address, uint8_t value)
{
uint8_t result = FLASH_SUCCESS;
#ifndef __RX600__
	eeprom_write_byte((unsigned char *) address, value);
#else
#if 0
	uint8_t wbuf[DF_ALIGN];
	uint8_t rbuf[DF_ERASE_BLOCK_SIZE]; // for block write
	uint8_t bbuf[DF_ERASE_BLOCK_SIZE]; // for blank check
	uint32_t w_addr = address & 0xfffffffe; // needed for blank check to target address
	uint32_t b_addr = address & 0xffffffe0; // block address

	for (int i = 0; i < DF_ALIGN; i++){
	    if((address & 0x00000001) == i){
	        wbuf[i] = value;
	    } else {
            wbuf[i] = this->read(w_addr + i);
	    }
	}

	if(flash_datarom_blankcheck(DF_ADDRESS + w_addr)){
	    // not blank
	    for(uint8_t i = 0; i < DF_ERASE_BLOCK_SIZE; i+=DF_ALIGN){
	        if(flash_datarom_blankcheck(DF_ADDRESS + b_addr + i)){
                bbuf[i]   = 0;
	            // not blank
                if (i == (w_addr & 0x1f)){
                    for(int j = 0 ; j < DF_ALIGN; j++){
                        rbuf[i + j]   = wbuf[j];
                    }
                } else {
                    for(int j = 0 ; j < DF_ALIGN; j++){
                        rbuf[i + j]   = this->read(b_addr + i + j);
                    }
                }
	        } else {
                bbuf[i] = 1;
	        }
	    }
	    result = flash_datarom_EraseBlock(DF_ADDRESS + (address & DF_BLOCK_MASK));

	    for(int i = 0; i < DF_ERASE_BLOCK_SIZE; i+=DF_ALIGN){
            if(bbuf[i]){ // do nothing because of blank

            } else {
                result = flash_datarom_WriteData(DF_ADDRESS + b_addr + i, &rbuf[i], DF_ALIGN);
            }
        }

	} else { // blank
	    result = flash_datarom_WriteData(DF_ADDRESS + w_addr, wbuf, DF_ALIGN);
	}

#else
	result = write(address, &value, 1);
#endif
#endif //__RX600__
	return result;
}

#ifdef __RX600__
// Return true if there are any differences between the specified data and the flash rom.
bool EEPROMClass::diff_flash(int address, void *data, int len)
{
	for (int i = 0; i < len; i++) {
		if (read(address + i) != ((uint8_t *) data)[i]) {
			return true;
		}
	}
	return false;
}

// Write partial data (smaller than a block) to the specified address.
uint8_t EEPROMClass::write_partial(int address, void *data, int len)
{
	uint8_t result = FLASH_SUCCESS;
	int block_addr = address & DF_BLOCK_MASK;
	int aligned_addr = address & ~(DF_ALIGN - 1);
	int aligned_end_addr = (address + len + 1) & ~(DF_ALIGN - 1);
	uint8_t buf[DF_ERASE_BLOCK_SIZE]; // for block write

	bool need_erase = false;

	// Copy data to internal buffer.
	// Read flash if start or end address is not aligned.
	for (int i = aligned_addr; i < aligned_end_addr; i++) {
		if ((i < address) || (address + len <= i)) {
			buf[i - block_addr] = read(i);
		} else {
			buf[i - block_addr] = ((uint8_t *)data)[i - address];
		}
	}

	// Check if we need to erase the block.
	for (int i = aligned_addr; i < aligned_end_addr; i += DF_ALIGN) {
		if (diff_flash(i, buf + i - block_addr, DF_ALIGN)) {
			if (flash_datarom_blankcheck(DF_ADDRESS + i)) {
				// not blank
				need_erase = true;
			}
		}
	}
	if (need_erase) {
		for (int i = block_addr; i < aligned_addr; i++) {
			buf[i - block_addr] = read(i);
		}
		for (int i = aligned_end_addr; i < block_addr + DF_ERASE_BLOCK_SIZE; i++) {
			buf[i - block_addr] = read(i);
		}
		result = flash_datarom_EraseBlock(DF_ADDRESS + block_addr);
	    result = flash_datarom_WriteData(
				DF_ADDRESS + block_addr, buf, DF_ERASE_BLOCK_SIZE);
	} else {
	    result = flash_datarom_WriteData(
				DF_ADDRESS + aligned_addr,
				buf + aligned_addr - block_addr,
				aligned_end_addr - aligned_addr);
	}
	return result;
}

// Write data to the specified block.
// The address and the length must be block-aligned.
uint8_t EEPROMClass::write_block(int address, void *data, int len)
{
	uint8_t result = FLASH_SUCCESS;

	if ((address & (DF_ERASE_BLOCK_SIZE - 1)) != 0) {
		// The address is not block-aligned.
		return FLASH_FAILURE;
	}
	if ((len & (DF_ERASE_BLOCK_SIZE - 1)) != 0) {
		// The length is not block-aligned.
		return FLASH_FAILURE;
	}

	for (int j = 0; j < len / DF_ERASE_BLOCK_SIZE; j++) {
		bool need_erase = false;
		int block_ofs = j * DF_ERASE_BLOCK_SIZE;
		for (int i = 0; i < DF_ERASE_BLOCK_SIZE; i += DF_ALIGN) {
			if (diff_flash(address + block_ofs + i,
						(uint8_t *)data + block_ofs + i, DF_ALIGN)) {
				if (flash_datarom_blankcheck(DF_ADDRESS + address + block_ofs + i)) {
					// not blank
					need_erase = true;
				}
			}
		}
		if (need_erase) {
			result = flash_datarom_EraseBlock(DF_ADDRESS + address + block_ofs);
		}
		result = flash_datarom_WriteData(
				DF_ADDRESS + address + block_ofs,
				(uint8_t *)data + block_ofs,
				DF_ERASE_BLOCK_SIZE);
	}
	return result;
}

// Write data to flash rom.
// This accepts non-block-aligned address and length.
uint8_t EEPROMClass::write(int address, void *data, int len)
{
	uint8_t result = FLASH_SUCCESS;
	int block_addr = address & DF_BLOCK_MASK;
	int len2;

	// Write first part before 32-byte aligned block.
	if (block_addr != address) {
		int off = address - block_addr;
		len2 = (len > DF_ERASE_BLOCK_SIZE - off) ? DF_ERASE_BLOCK_SIZE - off : len;
		result = write_partial(address, data, len2);
		len -= len2;
		address += len2;
		data = (void *)((uint8_t *)data + len2);
	}

	// Write 32-byte aligned blocks.
	len2 = len & DF_BLOCK_MASK;
	result = write_block(address, data, len2);
	len -= len2;
	address += len2;
	data = (void *)((uint8_t *)data + len2);

	// Write remaining part.
	result = write_partial(address, data, len);

	return result;
}
#endif //__RX600__

EEPROMClass EEPROM;
