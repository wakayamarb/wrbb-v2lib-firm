/* Hardware I2cMaster Library
 * Copyright (C) 2014 Renesas Electronics
 *
 * This file is part of the RX63N Hardware I2cMaster Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the RX63N Hardware I2cMaster Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef TWI_RX_H
#define TWI_RX_H
/**
 * file
 * brief Hardware I2C and hardware TWI library
 */
#include <stdbool.h>
#include <stdint.h>

#ifndef TWI_FREQ
#define TWI_FREQ 100000L
#endif

/**
 * Get the data of SCI_I2C registers.
 * @param   a : unit offset.
 */
#define SCI_I2C_ADR_OFFSET 0x20
#define SCI_I2C_IR_OFFSET 3
#define SCIx_I2C_SMR_CKS_UNMASK 0b11111100 //for Configure the parity, number of data bits and stop bits

#define SCIx_I2C_SCR_BYTE(a)   *((volatile uint8_t*)&SCI0.SCR + a * SCI_I2C_ADR_OFFSET    )
#define SCIx_I2C_SCMR_BYTE(a)  *((volatile uint8_t*)&SCI0.SCMR + a * SCI_I2C_ADR_OFFSET   )
#define SCIx_I2C_SMR_BYTE(a)   *((volatile uint8_t*)&SCI0.SMR + a * SCI_I2C_ADR_OFFSET    )
#define SCIx_I2C_BRR_BYTE(a)   *((volatile uint8_t*)&SCI0.BRR + a * SCI_I2C_ADR_OFFSET    )
#define SCIx_I2C_SSR_BYTE(a)   *((volatile uint8_t*)&SCI0.SSR + a * SCI_I2C_ADR_OFFSET    )
#define SCIx_I2C_TDR_BYTE(a)   *((volatile uint8_t*)&SCI0.TDR + a * SCI_I2C_ADR_OFFSET    )
#define SCIx_I2C_RDR_BYTE(a)   *((volatile uint8_t*)&SCI0.RDR + a * SCI_I2C_ADR_OFFSET    )
#define SCIx_I2C_SEMR_BYTE(a)  *((volatile uint8_t*)&SCI0.SEMR + a * SCI_I2C_ADR_OFFSET   )
#define SCIx_I2C_SIMR1_BYTE(a)  *((volatile uint8_t*)&SCI0.SIMR1 + a * SCI_I2C_ADR_OFFSET   )
#define SCIx_I2C_SIMR2_BYTE(a)  *((volatile uint8_t*)&SCI0.SIMR2 + a * SCI_I2C_ADR_OFFSET   )
#define SCIx_I2C_SIMR3_BYTE(a)  *((volatile uint8_t*)&SCI0.SIMR3 + a * SCI_I2C_ADR_OFFSET   )
#define SCIx_I2C_SPMR_BYTE(a)  *((volatile uint8_t*)&SCI0.SPMR + a * SCI_I2C_ADR_OFFSET   )
#define SCIx_I2C_SISR_BYTE(a)  *((volatile uint8_t*)&SCI0.SISR + a * SCI_I2C_ADR_OFFSET   )
#define SCIx_I2C_IR_RIE_BYTE(a)  *((volatile uint8_t*)&ICU.IR[214 + a * SCI_I2C_IR_OFFSET] )
#define SCIx_I2C_IR_TIE_BYTE(a)  *((volatile uint8_t*)&ICU.IR[215 + a * SCI_I2C_IR_OFFSET] )
#define SCIx_I2C_IR_TEIE_BYTE(a)  *((volatile uint8_t*)&ICU.IR[216 + a * SCI_I2C_IR_OFFSET] )

//------------------------------------------------------------------------------
void twi_rx_init(uint8_t channel, int freq);
uint8_t twi_rx_read(uint8_t channel, uint8_t last);
bool twi_rx_restart(uint8_t channel, uint8_t addressRW);
bool twi_rx_start(uint8_t channel, uint8_t addressRW);
void twi_rx_stop(uint8_t channel);
bool twi_rx_write(uint8_t channel, uint8_t b);
void twi_rx_setFrequency(uint8_t channel, int freq);

#endif  // HARDWARE_I2C_MASTER_H
