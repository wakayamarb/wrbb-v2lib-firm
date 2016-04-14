/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

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

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/
/*
  Modified 13 July 2014 by Nozomu Fujita
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#ifndef GRSAKURA
#include <avr/pgmspace.h>
#else /*GRSAKURA*/
#include "rx63n/typedefine.h"
#endif/*GRSAKURA*/

#define NUM_DIGITAL_PINS            66
#ifndef GRSAKURA
#define NUM_ANALOG_INPUTS           8
#else /*GRSAKURA*/
#define NUM_ANALOG_INPUTS           16
#endif/*GRSAKURA*/
/** Invalid pin/port/bitmask. */
#define INVALID_IO (-1)

#define analogInputToDigitalPin(p)  ((p < 6) ? (p) + 14 : -1)

#if defined(GRSAKURA)
#define digitalPinHasPWM(p)         ((p) == 0 || (p) == 1 || (p) == 2 || (p) == 3 || (p) == 4 || \
                                     (p) == 5 || (p) == 6 || (p) == 7 || (p) == 9 || (p) == 10 || (p) == 11 \
                                    )
#elif defined(__AVR_ATmega8__)
#define digitalPinHasPWM(p)         ((p) == 9 || (p) == 10 || (p) == 11)
#else
#define digitalPinHasPWM(p)         ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11)
#endif

static const uint8_t SS   = 10; // 22 is used for SD as SS
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t SDA = 18;
static const uint8_t SCL = 19;
#ifndef GRSAKURA
#define LED_BUILTIN 13
#endif/*GRSAKURA*/

static const uint8_t A0 = 14;
static const uint8_t A1 = 15;
static const uint8_t A2 = 16;
static const uint8_t A3 = 17;
static const uint8_t A4 = 18;
static const uint8_t A5 = 19;
static const uint8_t A6 = 20;
static const uint8_t A7 = 21;

#ifdef GRSAKURA
/** IO pins, header CN8. */
#define PIN_IO0     0
#define PIN_IO1     1
#define PIN_IO2     2
#define PIN_IO3     3
#define PIN_IO4     4
#define PIN_IO5     5
#define PIN_IO6     6
#define PIN_IO7     7

/** IO pins, header CN7. */
#define PIN_IO8     8
#define PIN_IO9     9
#define PIN_IO10    10
#define PIN_IO11    11
#define PIN_IO12    12
#define PIN_IO13    13

/** IO pins, header CN7. */
#define PIN_IO14    14
#define PIN_IO15    15
#define PIN_IO16    16
#define PIN_IO17    17
#define PIN_IO18    18
#define PIN_IO19    19
#define PIN_IO20    20
#define PIN_IO21    21

/** IO pins, header CN9. */
#define PIN_IO22    22
#define PIN_IO23    23
#define PIN_IO24    24
#define PIN_IO25    25
#define PIN_IO26    26
#define PIN_IO27    27
#define PIN_IO28    28
#define PIN_IO29    29

/** IO pins, header CN10. */
#define PIN_IO30    30
#define PIN_IO31    31
#define PIN_IO32    32
#define PIN_IO33    33
#define PIN_IO34    34
#define PIN_IO35    35

/** IO pins, header CN12. */
#define PIN_IO36    36
#define PIN_IO37    37
#define PIN_IO38    38
#define PIN_IO39    39
#define PIN_IO40    40
#define PIN_IO41    41
#define PIN_IO42    42
#define PIN_IO43    43

/** IO pins, header CN11. */
#define PIN_IO44    44
#define PIN_IO45    45
#define PIN_IO46    46
#define PIN_IO47    47
#define PIN_IO48    48
#define PIN_IO49    49
#define PIN_IO50    50
#define PIN_IO51    51

/** IO pins, header CN13. */
#define PIN_IO52    52
#define PIN_IO53    53
#define PIN_IO54    54
#define PIN_IO55    55

/** IO pins, header CN6. */
#define PIN_IO56    56
#define PIN_IO57    57
#define PIN_IO58    58
#define PIN_IO59    59
#define PIN_IO60    60

/** LEDs (D1-D4) and SW. */
#define PIN_LED0    61
#define PIN_LED1    62
#define PIN_LED2    63
#define PIN_LED3    64
#define PIN_SW      65

// ANALOG IO PINS -------------------------------------------------------------/

/** Analog pins, header CN15. */
#define PIN_AN000     14
#define PIN_AN001     15
#define PIN_AN002     16
#define PIN_AN003     17
#define PIN_AN004     18
#define PIN_AN005     19

/** Analog pins, header CN13. */
#define PIN_AN006     20
#define PIN_AN007     21

/** Analog pins, header CN12. */
#define PIN_AN008     22
#define PIN_AN009     23
#define PIN_AN010     24
#define PIN_AN011     25
#define PIN_AN012     26
#define PIN_AN013     27

/** Analog input, RX63N internal */
#define PIN_ANINT     28
#define PIN_ANTMP     29
#endif/*GRSAKURA*/

#ifdef GRSAKURA
#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 21) ? (&PCICR) : ((uint8_t *)0))
#define digitalPinToPCICRbit(p) (((p) <= 7) ? 2 : (((p) <= 13) ? 0 : 1))
#define digitalPinToPCMSK(p)    (((p) <= 7) ? (&PCMSK2) : (((p) <= 13) ? (&PCMSK0) : (((p) <= 21) ? (&PCMSK1) : ((uint8_t *)0))))
#define digitalPinToPCMSKbit(p) (((p) <= 7) ? (p) : (((p) <= 13) ? ((p) - 8) : ((p) - 14)))
#endif/*GRSAKURA*/

#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : ((p) == 3 ? 1 : NOT_AN_INTERRUPT))

#ifdef ARDUINO_MAIN

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATMEGA8 & 168 / ARDUINO
//
//                  +-\/-+
//            PC6  1|    |28  PC5 (AI 5)
//      (D 0) PD0  2|    |27  PC4 (AI 4)
//      (D 1) PD1  3|    |26  PC3 (AI 3)
//      (D 2) PD2  4|    |25  PC2 (AI 2)
// PWM+ (D 3) PD3  5|    |24  PC1 (AI 1)
//      (D 4) PD4  6|    |23  PC0 (AI 0)
//            VCC  7|    |22  GND
//            GND  8|    |21  AREF
//            PB6  9|    |20  AVCC
//            PB7 10|    |19  PB5 (D 13)
// PWM+ (D 5) PD5 11|    |18  PB4 (D 12)
// PWM+ (D 6) PD6 12|    |17  PB3 (D 11) PWM
//      (D 7) PD7 13|    |16  PB2 (D 10) PWM
//      (D 8) PB0 14|    |15  PB1 (D 9) PWM
//                  +----+
//
// (PWM+ indicates the additional PWM pins on the ATmega168.)

// ATMEL ATMEGA1280 / ARDUINO
//
// 0-7 PE0-PE7   works
// 8-13 PB0-PB5  works
// 14-21 PA0-PA7 works 
// 22-29 PH0-PH7 works
// 30-35 PG5-PG0 works
// 36-43 PC7-PC0 works
// 44-51 PJ7-PJ0 works
// 52-59 PL7-PL0 works
// 60-67 PD7-PD0 works
// A0-A7 PF0-PF7
// A8-A15 PK0-PK7


// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
#ifndef GRSAKURA
const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
};
#endif/*GRSAKURA*/

#define _offset(port) ((uint32_t)&port - (uint32_t)&PORT0)
const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	_offset(PORT2), /* PIN_IO0 (P21) */
	_offset(PORT2), /* PIN_IO1 (P20) */
	_offset(PORT2), /* PIN_IO2 (P22) */
	_offset(PORT2), /* PIN_IO3 (P23) */
	_offset(PORT2), /* PIN_IO4 (P24) */
	_offset(PORT2), /* PIN_IO5 (P25) */
	_offset(PORT3), /* PIN_IO6 (P32) */
	_offset(PORT3), /* PIN_IO7 (P33) */
	_offset(PORTC), /* PIN_IO8 (PC2) */
	_offset(PORTC), /* PIN_IO9 (PC3) */
	_offset(PORTC), /* PIN_IO10(PC4) */
	_offset(PORTC), /* PIN_IO11(PC6) */
	_offset(PORTC), /* PIN_IO12(PC7) */
	_offset(PORTC), /* PIN_IO13(PC5) */
	_offset(PORT4), /* PIN_IO14(P40) */
	_offset(PORT4), /* PIN_IO15(P41) */
	_offset(PORT4), /* PIN_IO16(P42) */
	_offset(PORT4), /* PIN_IO17(P43) */
	_offset(PORT4), /* PIN_IO18(P44) */
	_offset(PORT4), /* PIN_IO19(P45) */
	_offset(PORT4), /* PIN_IO20(P46) */
	_offset(PORT4), /* PIN_IO21(P47) */
	_offset(PORTC), /* PIN_IO22(PC0) */
	_offset(PORTC), /* PIN_IO23(PC1) */
	_offset(PORT5), /* PIN_IO24(P50) */
	_offset(PORT5), /* PIN_IO25(P51) */
	_offset(PORT5), /* PIN_IO26(P52) */
	_offset(PORT5), /* PIN_IO27(P53) */
	_offset(PORT5), /* PIN_IO28(P54) */
	_offset(PORT5), /* PIN_IO29(P55) */
	_offset(PORT1), /* PIN_IO30(P12) */
	_offset(PORT1), /* PIN_IO31(P13) */
	_offset(PORT1), /* PIN_IO32(P14) */
	_offset(PORT1), /* PIN_IO33(P15) */
	_offset(PORT1), /* PIN_IO34(P16) */
	_offset(PORT1), /* PIN_IO35(P17) */
	_offset(PORTD), /* PIN_IO36(PD0) */
	_offset(PORTD), /* PIN_IO37(PD1) */
	_offset(PORTD), /* PIN_IO38(PD2) */
	_offset(PORTD), /* PIN_IO39(PD3) */
	_offset(PORTD), /* PIN_IO40(PD4) */
	_offset(PORTD), /* PIN_IO41(PD5) */
	_offset(PORTD), /* PIN_IO42(PD6) */
	_offset(PORTD), /* PIN_IO43(PD7) */
	_offset(PORTE), /* PIN_IO44(PE0) */
	_offset(PORTE), /* PIN_IO45(PE1) */
	_offset(PORTE), /* PIN_IO46(PE2) */
	_offset(PORTE), /* PIN_IO47(PE3) */
	_offset(PORTE), /* PIN_IO48(PE4) */
	_offset(PORTE), /* PIN_IO49(PE5) */
	_offset(PORTE), /* PIN_IO50(PE6) */
	_offset(PORTE), /* PIN_IO51(PE7) */
	_offset(PORT0), /* PIN_IO52(P07) */
	_offset(PORT0), /* PIN_IO53(P05) */
	_offset(PORT3), /* PIN_IO54(P35) */
	_offset(PORTJ), /* PIN_IO55(PJ3) */
	_offset(PORT2), /* PIN_IO56(P27) */
	_offset(PORT3), /* PIN_IO57(P34) */
	_offset(PORT2), /* PIN_IO58(P26) */
	_offset(PORT3), /* PIN_IO59(P31) */
	_offset(PORT3), /* PIN_IO60(P30) */
	_offset(PORTA), /* PIN_LED0(PA0) */
	_offset(PORTA), /* PIN_LED1(PA1) */
	_offset(PORTA), /* PIN_LED2(PA2) */
	_offset(PORTA), /* PIN_LED3(PA6) */
	_offset(PORTA), /* PIN_SW  (PA7) */
};
#undef _offset

const uint8_t PROGMEM digital_pin_to_bit_PGM[] = {
	1, /* PIN_IO0 (P21) */
	0, /* PIN_IO1 (P20) */
	2, /* PIN_IO2 (P22) */
	3, /* PIN_IO3 (P23) */
	4, /* PIN_IO4 (P24) */
	5, /* PIN_IO5 (P25) */
	2, /* PIN_IO6 (P32) */
	3, /* PIN_IO7 (P33) */
	2, /* PIN_IO8 (PC2) */
	3, /* PIN_IO9 (PC3) */
	4, /* PIN_IO10(PC4) */
	6, /* PIN_IO11(PC6) */
	7, /* PIN_IO12(PC7) */
	5, /* PIN_IO13(PC5) */
	0, /* PIN_IO14(P40) */
	1, /* PIN_IO15(P41) */
	2, /* PIN_IO16(P42) */
	3, /* PIN_IO17(P43) */
	4, /* PIN_IO18(P44) */
	5, /* PIN_IO19(P45) */
	6, /* PIN_IO20(P46) */
	7, /* PIN_IO21(P47) */
	0, /* PIN_IO22(PC0) */
	1, /* PIN_IO23(PC1) */
	0, /* PIN_IO24(P50) */
	1, /* PIN_IO25(P51) */
	2, /* PIN_IO26(P52) */
	3, /* PIN_IO27(P53) */
	4, /* PIN_IO28(P54) */
	5, /* PIN_IO29(P55) */
	2, /* PIN_IO30(P12) */
	3, /* PIN_IO31(P13) */
	4, /* PIN_IO32(P14) */
	5, /* PIN_IO33(P15) */
	6, /* PIN_IO34(P16) */
	7, /* PIN_IO35(P17) */
	0, /* PIN_IO36(PD0) */
	1, /* PIN_IO37(PD1) */
	2, /* PIN_IO38(PD2) */
	3, /* PIN_IO39(PD3) */
	4, /* PIN_IO40(PD4) */
	5, /* PIN_IO41(PD5) */
	6, /* PIN_IO42(PD6) */
	7, /* PIN_IO43(PD7) */
	0, /* PIN_IO44(PE0) */
	1, /* PIN_IO45(PE1) */
	2, /* PIN_IO46(PE2) */
	3, /* PIN_IO47(PE3) */
	4, /* PIN_IO48(PE4) */
	5, /* PIN_IO49(PE5) */
	6, /* PIN_IO50(PE6) */
	7, /* PIN_IO51(PE7) */
	7, /* PIN_IO52(P07) */
	5, /* PIN_IO53(P05) */
	5, /* PIN_IO54(P35) */
	3, /* PIN_IO55(PJ3) */
	7, /* PIN_IO56(P27) */
	4, /* PIN_IO57(P34) */
	6, /* PIN_IO58(P26) */
	1, /* PIN_IO59(P31) */
	0, /* PIN_IO60(P30) */
	0, /* PIN_LED0(PA0) */
	1, /* PIN_LED1(PA1) */
	2, /* PIN_LED2(PA2) */
	6, /* PIN_LED3(PA6) */
	7, /* PIN_SW  (PA7) */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(1), /* PIN_IO0 (P21) */
	_BV(0), /* PIN_IO1 (P20) */
	_BV(2), /* PIN_IO2 (P22) */
	_BV(3), /* PIN_IO3 (P23) */
	_BV(4), /* PIN_IO4 (P24) */
	_BV(5), /* PIN_IO5 (P25) */
	_BV(2), /* PIN_IO6 (P32) */
	_BV(3), /* PIN_IO7 (P33) */
	_BV(2), /* PIN_IO8 (PC2) */
	_BV(3), /* PIN_IO9 (PC3) */
	_BV(4), /* PIN_IO10(PC4) */
	_BV(6), /* PIN_IO11(PC6) */
	_BV(7), /* PIN_IO12(PC7) */
	_BV(5), /* PIN_IO13(PC5) */
	_BV(0), /* PIN_IO14(P40) */
	_BV(1), /* PIN_IO15(P41) */
	_BV(2), /* PIN_IO16(P42) */
	_BV(3), /* PIN_IO17(P43) */
	_BV(4), /* PIN_IO18(P44) */
	_BV(5), /* PIN_IO19(P45) */
	_BV(6), /* PIN_IO20(P46) */
	_BV(7), /* PIN_IO21(P47) */
	_BV(0), /* PIN_IO22(PC0) */
	_BV(1), /* PIN_IO23(PC1) */
	_BV(0), /* PIN_IO24(P50) */
	_BV(1), /* PIN_IO25(P51) */
	_BV(2), /* PIN_IO26(P52) */
	_BV(3), /* PIN_IO27(P53) */
	_BV(4), /* PIN_IO28(P54) */
	_BV(5), /* PIN_IO29(P55) */
	_BV(2), /* PIN_IO30(P12) */
	_BV(3), /* PIN_IO31(P13) */
	_BV(4), /* PIN_IO32(P14) */
	_BV(5), /* PIN_IO33(P15) */
	_BV(6), /* PIN_IO34(P16) */
	_BV(7), /* PIN_IO35(P17) */
	_BV(0), /* PIN_IO36(PD0) */
	_BV(1), /* PIN_IO37(PD1) */
	_BV(2), /* PIN_IO38(PD2) */
	_BV(3), /* PIN_IO39(PD3) */
	_BV(4), /* PIN_IO40(PD4) */
	_BV(5), /* PIN_IO41(PD5) */
	_BV(6), /* PIN_IO42(PD6) */
	_BV(7), /* PIN_IO43(PD7) */
	_BV(0), /* PIN_IO44(PE0) */
	_BV(1), /* PIN_IO45(PE1) */
	_BV(2), /* PIN_IO46(PE2) */
	_BV(3), /* PIN_IO47(PE3) */
	_BV(4), /* PIN_IO48(PE4) */
	_BV(5), /* PIN_IO49(PE5) */
	_BV(6), /* PIN_IO50(PE6) */
	_BV(7), /* PIN_IO51(PE7) */
	_BV(7), /* PIN_IO52(P07) */
	_BV(5), /* PIN_IO53(P05) */
	_BV(5), /* PIN_IO54(P35) */
	_BV(3), /* PIN_IO55(PJ3) */
	_BV(7), /* PIN_IO56(P27) */
	_BV(4), /* PIN_IO57(P34) */
	_BV(6), /* PIN_IO58(P26) */
	_BV(1), /* PIN_IO59(P31) */
	_BV(0), /* PIN_IO60(P30) */
	_BV(0), /* PIN_LED0(PA0) */
	_BV(1), /* PIN_LED1(PA1) */
	_BV(2), /* PIN_LED2(PA2) */
	_BV(6), /* PIN_LED3(PA6) */
	_BV(7), /* PIN_SW  (PA7) */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
#ifndef GRSAKURA
	NOT_ON_TIMER, /* 0 - port D */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	// on the ATmega168, digital pin 3 has hardware pwm
#if defined(__AVR_ATmega8__)
	NOT_ON_TIMER,
#else
	TIMER2B,
#endif
	NOT_ON_TIMER,
	// on the ATmega168, digital pins 5 and 6 have hardware pwm
#if defined(__AVR_ATmega8__)
	NOT_ON_TIMER,
	NOT_ON_TIMER,
#else
	TIMER0B,
	TIMER0A,
#endif
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 8 - port B */
	TIMER1A,
	TIMER1B,
#if defined(__AVR_ATmega8__)
	TIMER2,
#else
	TIMER2A,
#endif
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 14 - port C */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
#else /*GRSAKURA*/
#endif/*GRSAKURA*/
};

#endif

// These serial port names are intended to allow libraries and architecture-neutral
// sketches to automatically default to the correct port name for a particular type
// of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
// the first hardware serial port whose RX/TX pins are not dedicated to another use.
//
// SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
//
// SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
//
// SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
//
// SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
//
// SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
//                            pins are NOT connected to anything by default.
#define SERIAL_PORT_MONITOR   Serial
#define SERIAL_PORT_HARDWARE  Serial

#endif
