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
 File        : Utilities.cpp
 Author      : E King
 Start Date  : 03/09/13
 Description : Implementation of the common utility functions.

 ******************************************************************************/
/*
 *  Modified 9 May 2014 by Yuuki Okamiya, for remove warnings, fixed timer frequency.
 *  Modified 12 May 2014 by Yuuki Okamiya, modify for analogWriteFrequency
 *  Modified 15 May 2014 by Yuuki Okamiya, change timer channel for each PWM pin.
 *  Modified 18 Jun 2014 by Nozomu Fujita : コメント修正
 *  Modified 19 Jun 2014 by Nozomu Fujita : 周期起動ハンドラ不具合修正
 *  Modified 19 Jun 2014 by Nozomu Fujita : INT_Excep_TPU2_TGI2A() 不具合修正
 *  Modified 19 Jun 2014 by Nozomu Fujita : timer_regist_userfunc() を追加
 */
#include <stddef.h>

#include "Arduino.h"
#include "Time.h"
#include "Utilities.h"

#include "rx63n/interrupt_handlers.h"
#include "rx63n/iodefine.h"
#include "rx63n/util.h"

// DEFINITIONS ****************************************************************/
/** The counter period required to give ~490Hz PWM frequency for 12MHz PCLK. */
#define COUNTER_PERIOD_FOR_490HZ_PWM    24490U
/** The maximum permitted frequency of a PWM signal, assuming a 12MHz PCLK. */
#define MAX_PWM_FREQ                    65535U
/** The counter value per duty cycle step for PWM of 490Hz from 12MHz PCLK. */
#define COUNTER_PER_DUTY_CYCLE_STEP     96U
/** The maximum frequency of a analogWrite signal, assuming a 12MHz PCLK. */
#define MAX_ANALOGWRITE_FREQ                    46875U
/** The minimum frequency of a analogWrite signal, assuming a 12MHz PCLK. */
#define MIN_ANALOGWRITE_FREQ                    184U

#define MAX_CYCLIC_HANDLER      (8)         //!< Number of maximum cyclic handler

// MACROS *********************************************************************/

#define COUNTER_INTERVAL

// DECLARATIONS ***************************************************************/
/** The current conversion reference. */
static word g_frequency_analogwrite = 490;
static fITInterruptFunc_t   g_fITInterruptFunc = NULL;  //!< ユーザー定義インターバルタイマハンドラ
// 周期起動ハンドラ関数テーブル
static fITInterruptFunc_t   g_afCyclicHandler[MAX_CYCLIC_HANDLER] =
{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static uint32_t g_au32CyclicTime[MAX_CYCLIC_HANDLER] =
{ 0, 0, 0, 0, 0, 0, 0, 0};
static uint32_t g_au32CyclicHandlerLastTime[MAX_CYCLIC_HANDLER] =
{ 0, 0, 0, 0, 0, 0, 0, 0};

/** This module's name. */
//static const char *MODULE = "UTILITIES"; //comment out to remove warning

// IMPLEMENTATIONS ************************************************************/

bool set_frequency_for_analogwrite(word frequency)
{
    if (frequency < MIN_ANALOGWRITE_FREQ || frequency > MAX_ANALOGWRITE_FREQ){
        return false;
    }
    g_frequency_analogwrite = frequency;
    return true;
}


/**
 * Generate a BRK instruction. This differs from the usual built-in variant
 * because we need to manually reset the user stack pointer. This is because
 * we do not transition to user mode when the program starts.
 */
void generate_brk()
{
    asm("brk");
    asm("mvtc #30000h, psw");
}

/****************************************************************************
 * Attach interval timer function
 *
 * The callback function is called every 1ms interval
 * @param[in] fFunction Specify callback function
 *
 * @return none
 *
 ***************************************************************************/
void attachIntervalTimerHandler(void (*fFunction)(unsigned long u32Milles))
{
    g_fITInterruptFunc = fFunction;

    startModule(MstpIdTPU0);

    // Stop the timer.
    TPUA.TSTR.BIT.CST5 = 0U;
    // Set the counter to run at the desired frequency.
    TPU5.TCR.BIT.TPSC = 0b011;
    // Set TGRA compare match to clear TCNT.
    TPU5.TCR.BIT.CCLR = 0b001;
    // Set the count to occur on rising edge of PCLK.
    TPU5.TCR.BIT.CKEG = 0b01;
    // Set Normal.
    TPU5.TMDR.BIT.MD = 0b0000;
    // Set the period.
    TPU5.TGRA = 750 - 1; //1ms setting at PCLK/64(750kHz)

    // Set the count to occur on rising edge of PCLK.
    TPU5.TSR.BIT.TGFA = 0U;

    /* Set TGI6A interrupt priority level to 5*/
    IPR(TPU5,TGI5A) = 0x5;
    /* Enable TGI6A interrupts */
    IEN(TPU5,TGI5A) = 0x1;
    /* Clear TGI6A interrupt flag */
    IR(TPU5,TGI5A) = 0x0;
    // Enable the module interrupt for the ms timer.
    TPU5.TIER.BIT.TGIEA = 1U;

    // Start the timer.
    TPUA.TSTR.BIT.CST5 = 1U;
}


/****************************************************************************
 * Detach interval timer function
 *
 * @param[in] none
 *
 * @return none
 *
 ***************************************************************************/
void detachIntervalTimerHandler()
{
    g_fITInterruptFunc = NULL;
    // Stop the timer.
    TPUA.TSTR.BIT.CST5 = 0U;

}

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
void attachCyclicHandler(uint8_t u8HandlerNumber, void (*fFunction)(unsigned long u32Milles), uint32_t u32CyclicTime)
{

    if (u8HandlerNumber < MAX_CYCLIC_HANDLER) {
        g_afCyclicHandler[u8HandlerNumber]              = fFunction;
        g_au32CyclicTime[u8HandlerNumber]               = u32CyclicTime;
        g_au32CyclicHandlerLastTime[u8HandlerNumber]    = millis();
    }

}

/****************************************************************************
 * Detach cyclic handler
 *
 * @param[in] ID from 0 to 7
 *
 * @return none
 *
 ***************************************************************************/
void detachCyclicHandler(uint8_t u8HandlerNumber)
{

    if (u8HandlerNumber < MAX_CYCLIC_HANDLER) {
        g_afCyclicHandler[u8HandlerNumber]              = NULL;
        g_au32CyclicTime[u8HandlerNumber]               = 0;
        g_au32CyclicHandlerLastTime[u8HandlerNumber]    = 0;
    }
}

/****************************************************************************
 * Execution cyclic handler
 *
 * @param[in] none
 *
 * @return none
 *
 ***************************************************************************/

void execCyclicHandler()
{
    int i;

    for (i = 0; i < MAX_CYCLIC_HANDLER; i++) {
        if (g_afCyclicHandler[i] != NULL) {
            unsigned long currentTime = millis();
            if ((currentTime - g_au32CyclicHandlerLastTime[i]) >= g_au32CyclicTime[i]) {
                g_au32CyclicHandlerLastTime[i] = currentTime;
                (*g_afCyclicHandler[i])(currentTime);
            }
        }
    }
}

/****************************************************************************
 * Attach interval timer function
 *
 * The callback function is called every 1ms interval
 * @param[in] fFunction Specify callback function
 *
 * @return always 1
 *
 ***************************************************************************/
unsigned long timer_regist_userfunc(void (*fFunction)(void))
{
    if (fFunction != NULL) {
        attachIntervalTimerHandler((void (*)(unsigned long))fFunction);
    } else {
        detachIntervalTimerHandler();
    }
    return 1;
}

// INTERRUPT HANDLERS *********************************************************/
// Note that these are declared in interrupts_handlers.h but defined here for
// clarity.

// TPU5 TGI5A
void INT_Excep_TPU5_TGI5A(void){
    if (g_fITInterruptFunc != NULL) {
        (*g_fITInterruptFunc)(millis());
    }
}
