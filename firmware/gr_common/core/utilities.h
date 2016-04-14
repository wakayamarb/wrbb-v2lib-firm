/***************************************************************************
PURPOSE
     RX63N Library for Arduino compatible framework

TARGET DEVICE
     RX63N

AUTHOR
     Renesas Solutions Corp.
     AND Technology Research Ltd.

***************************************************************************
Copyright (C) 2014 Renesas Electronics. All rights reserved.

This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 2.1 of the License, or (at your option) any
later version.

See file LICENSE.txt for further informations on licensing terms.

***************************************************************************
 Copyright   : (c) AND Technology Research Ltd, 2013
 Address     : 4 Forest Drive, Theydon Bois, Essex, CM16 7EY
 Tel         : +44 (0) 1992 81 4655
 Fax         : +44 (0) 1992 81 3362
 Email       : ed.king@andtr.com
 Website     : www.andtr.com

 Project     : Arduino
 Module      : Core
 File        : Utilities.h
 Author      : E King
 Start Date  : 03/09/13
 Description : Declarations of common utility functions and private core
               functions that should generally not be called by the user.
               v1.00 - First version.
               v1.01 - Changed to C linkage and added generate_break().
                     - Added set_system_clocks().
 @version    : 1.01
 @since      : 1.00


 ******************************************************************************/
/*
 *  Modified 9 May 2014 by Yuuki Okamiya, for remove warnings
 *  Modified 19 Jun 2014 by Nozomu Fujita : timer_regist_userfunc() を追加
 */

#ifndef UTILITIES_HPP_
#define UTILITIES_HPP_

#include <stdbool.h>
#include <stdint.h>
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

// DEFINITIONS ****************************************************************/

// MACROS *********************************************************************/

/** Clock-cycle to microsecond conversions. */
#define CLOCK_CYCLES_PER_MICROSECOND()  (FREQ_CPU_HZ / 1000000L)
#define CLOCK_CYCLES_TO_MICROSECONDS(a) ((a) / CLOCK_CYCLES_PER_MICROSECOND())
#define MICROSECONDS_TO_CLOCK_CYCLES(a) ((a) * CLOCK_CYCLES_PER_MICROSECOND())

/** Bit manipulations. */
#define htonl(x)    (((x) << 24 & 0xFF000000UL) | \
                     ((x) << 8  & 0x00FF0000UL) | \
                     ((x) >> 8  & 0x0000FF00UL) | \
                     ((x) >> 24 & 0x000000FFUL) )
#define ntohl(x)    htonl(x)

// DECLARATIONS ***************************************************************/
/**
 * Helper function to change the frequency for analogWrite
 * @param   frequency : the PWM frequency (184 to 46875Hz)
 * @return  true if the setting was successful, false if not.
 */

bool set_frequency_for_analogwrite(word frequency);

/**
 * Helper function to write a PWM signal to a pin of a given frequency and
 * duty cycle.
 * @param   pin : the pin to write to.
 * @param   frequency : the PWM frequency (1 to 65535Hz)
 * @param   value : the duty cycle between 0 (always off) and 255 (always on).
 * @param   for_analog_write : if true, the frequency is ignored and we provide
 *                             a 490Hz signal for the given duty cycle. If
 *                             false, we ignore the duty cycle and provide a 50%
 *                             duty cycle signal at the provided frequency.
 * @return  true if the operation was successful, false if not.
 */
bool set_pwm_for_pin(int pin, uint16_t frequency, byte value,
        bool for_analog_write);

/**
 * Generate a BRK instruction. This differs from the usual built-in variant
 * because we need to manually reset the user stack pointer. This is because
 * we do not transition to user mode when the program starts.
 */
void generate_brk();

/**
 * Set the system clocks, with configurable peripheral clocks.
 * @param   pclka : value in Hz.
 * @param   pclkb : value in Hz.
 * @return  True if successful, false otherwise.
 */
bool set_system_clocks(double pclka, double pclkb);

/****************************************************************************
 * Attach interval timer function
 *
 * The callback function is called every 1ms interval
 * @param[in] fFunction Specify callback function
 *
 * @return none
 *
 ***************************************************************************/
void attachIntervalTimerHandler(void (*fFunction)(unsigned long));

/****************************************************************************
 * Detach interval timer function
 *
 * @param[in] none
 *
 * @return none
 *
 ***************************************************************************/
void detachIntervalTimerHandler();

/****************************************************************************
 * Attach cyclic handler
 *
 * Attached handler is called every specified interval u32CyclicTime
 *
 * @param[in] u8HandlerNumber Specify ID from 0 to 7
 * @param[in] fFunction       Specify handler
 * @param[in] u32CyclicTime   Specify interval [ms]
 *
 * @return none
 *
 ***************************************************************************/
void attachCyclicHandler(uint8_t u8HandlerNumber, void (*fFunction)(unsigned long u32Milles), uint32_t u32CyclicTime);

/****************************************************************************
 * Detach cyclic handler
 *
 * @param[in] ID from 0 to 7
 *
 * @return none
 *
 ***************************************************************************/
void detachCyclicHandler(uint8_t u8HandlerNumber);

/****************************************************************************
 * Execution cyclic handler
 *
 * @param[in] none
 *
 * @return none
 *
 ***************************************************************************/
void execCyclicHandler();

/**
 * Initialise the USB stack and start its scheduler.
 * @param   mode : USB_MODE_DEVICE or USB_MODE_HOST.
 *                 Note that if USB host is used, the serial-over-USB (console)
 *                 cannot be since they share the same port.
 */
void usb_stack_init(char mode);

/****************************************************************************
 * Register interval timer function
 *
 * The callback function is called every 1ms interval
 * @param[in] fFunction Specify callback function
 *
 * @return always 1
 *
 ***************************************************************************/
unsigned long timer_regist_userfunc(void (*fFunction)(void));

#ifdef __cplusplus
}
#endif

#endif // UTILITIES_HPP_
