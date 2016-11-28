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

#ifndef GRSAKURA
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
#ifndef GRSAKURA
	return eeprom_read_byte((unsigned char *) address);
#else
	if(flash_datarom_blankcheck(DF_ADDRESS + (address & 0xfffffffe)) == 0)
	    //blank
	    return 0xff;
	return *(volatile unsigned char *)(address + DF_ADDRESS);
#endif //GRSAKURA
}

uint8_t EEPROMClass::write(int address, uint8_t value)
{
uint8_t result = FLASH_SUCCESS;
#ifndef GRSAKURA
	eeprom_write_byte((unsigned char *) address, value);
#else
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

#endif //GRSAKURA
	return result;
}

EEPROMClass EEPROM;
