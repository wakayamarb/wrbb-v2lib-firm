/*
  wiring_private.h - Internal header file.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 239 2007-01-12 17:58:39Z mellis $
*/
/*
  Modified 13 July 2014 by Nozomu Fujita for GR-SAKURA
*/

#ifndef WiringPrivate_h
#define WiringPrivate_h

#ifndef GRSAKURA
#include <avr/io.h>
#include <avr/interrupt.h>
#endif/*GRSAKURA*/
#include <stdio.h>
#include <stdarg.h>

#include "Arduino.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifndef GRSAKURA
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define EXTERNAL_INT_0 0
#define EXTERNAL_INT_1 1
#define EXTERNAL_INT_2 2
#define EXTERNAL_INT_3 3
#define EXTERNAL_INT_4 4
#define EXTERNAL_INT_5 5
#define EXTERNAL_INT_6 6
#define EXTERNAL_INT_7 7

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define EXTERNAL_NUM_INTERRUPTS 8
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
#define EXTERNAL_NUM_INTERRUPTS 3
#elif defined(__AVR_ATmega32U4__)
#define EXTERNAL_NUM_INTERRUPTS 5
#else
#define EXTERNAL_NUM_INTERRUPTS 2
#endif
#else
#define EXTERNAL_NUM_INTERRUPTS 60 //Actually 17
/** IRQ numbers. */
#define IRQ0    0U
#define IRQ1    1U
#define IRQ2    2U
#define IRQ3    3U
#define IRQ4    4U
#define IRQ5    5U
#define IRQ6    6U
#define IRQ7    7U
#define IRQ8    8U
#define IRQ9    9U
#define IRQ10   10U
#define IRQ11   11U
#define IRQ12   12U
#define IRQ13   13U
#define IRQ14   14U
#define IRQ15   15U
#define NMI     16U // Special case for IO54.
#define GET_ADR_IR(a)    ((volatile byte*)&ICU.IR   [IR_ICU_IRQ0   + a      ])
#define GET_ADR_IER(a)   ((volatile byte*)&ICU.IER  [IER_ICU_IRQ0  + (a / 8)])
#define GET_ADR_IPR(a)   ((volatile byte*)&ICU.IPR  [IPR_ICU_IRQ0  + a      ])
#define GET_ADR_DTCER(a) ((volatile byte*)&ICU.DTCER[DTCE_ICU_IRQ0 + a      ])
#define GET_ADR_IRQCR(a) ((volatile byte*)&ICU.IRQCR[0             + a      ])
/** The ISE bit of a PFS register. */
#define BIT_PFS_ISE             0x40
/** Default interrupt priority. Must be > 0, otherwise CPU won't accept. */
#define DEFAULT_INT_PRIORITY    0x05

#endif/*GRSAKURA*/

typedef void (*voidFuncPtr)(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
