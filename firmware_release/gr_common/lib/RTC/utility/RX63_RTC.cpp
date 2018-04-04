/***************************************************************************
 *
 * PURPOSE
 *   RTC(Real Time Clock) function module file.
 *
 * TARGET DEVICE
 *   RX63N
 *
 * AUTHOR
 *   Renesas Electronics Corp.
 *
 *
 ***************************************************************************
 * Copyright (C) 2014 Renesas Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See file LICENSE.txt for further informations on licensing terms.
 ***************************************************************************/
/**
 * @file  RTC0.cpp
 * @brief RX63Nマイコン内蔵の時計機能（RTC：リアル・タイム・クロック）を使うためのライブラリです。
 *
 * RTCクラスはこのライブラリをC++でカプセル化して使いやすくしたものです。
 *
 * Modified 27th May 2014 by Yuuki Okamiya from RL78duino.cpp
 */

/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include <utility/RX63_RTC.h>
#include "rx63n/iodefine.h"
#include "rx63n/interrupt_handlers.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define RTC_WAIT_10USEC 320 //!< 10 us待ちカウント値


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/
static inline uint8_t HEX2BCD(int s16HEX);
static inline int BCD2HEX(uint8_t u8BCD);


/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/



/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/
static fInterruptFunc_t g_fRTCInterruptFunc = NULL;


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/
/** ************************************************************************
 * @addtogroup group101
 *
 * @{
 ***************************************************************************/
/** ************************************************************************
 * @defgroup group12 時計機能
 *
 * @{
 ***************************************************************************/
/**
 * RTCのハードウェアを初期化します。
 *
 * @retval 0：RTCの起動に失敗しました。
 * @retval 1：RTCの起動に成功しました。
 * @retval 2：RTCは既に起動しています。
 *
 * @attention なし
 ***************************************************************************/
int rtc_init()
{
#ifndef __RX600__
    /* Protection off */
    SYSTEM.PRCR.WORD = 0xA503;
#endif

    /* Check if the MCU has come from a cold start (power on reset) */
    if(0 == SYSTEM.RSTSR1.BIT.CWSF)
    {
        /* Set the warm start flag */
        SYSTEM.RSTSR1.BIT.CWSF = 1;

        /* Disable the sub-clock oscillator */
        SYSTEM.SOSCCR.BIT.SOSTP = 1;

        /* Wait for register modification to complete */
        while(1 != SYSTEM.SOSCCR.BIT.SOSTP);

        /* Disable the input from the sub-clock */
        RTC0.RCR3.BYTE = 0x0C;

        /* Wait for the register modification to complete */
        while(0 != RTC0.RCR3.BIT.RTCEN);

        /* Wait for at least 5 cycles of sub-clock */
        uint32_t wait = 0x6000;
        while(wait--);

        /* Start sub-clock */
        SYSTEM.SOSCCR.BIT.SOSTP = 0;

        /* Perform 8 delay iterations */
        for(uint8_t i = 0; i < 8; i++)
        {
            /* Wait in while loop for ~0.5 seconds */
            wait = 0x2FFFFE;
            while(wait--);
        }
    }
    else
    {
        /* Start sub-clock */
        SYSTEM.SOSCCR.BIT.SOSTP = 0;

        /* Wait for the register modification to complete */
        while(0 != SYSTEM.SOSCCR.BIT.SOSTP);
    }

    /* Set RTC clock input from sub-clock, and supply to RTC module */
    RTC0.RCR4.BIT.RCKSEL = 0;
    RTC0.RCR3.BIT.RTCEN = 1;

    /* Wait for at least 5 cycles of sub-clock */
    uint32_t wait = 0x6000;
    while(wait--);

    /* It is now safe to set the RTC registers */

    /* Stop the clock */
    RTC0.RCR2.BIT.START = 0x0;

    /* Wait for start bit to clear */
    while(0 != RTC0.RCR2.BIT.START);

    /* Reset the RTC unit */
    RTC0.RCR2.BIT.RESET = 1;

    /* Wait until reset is complete */
    while(RTC0.RCR2.BIT.RESET);

    // コールバックハンドラの初期化
    g_fRTCInterruptFunc = NULL;

    return 1;
}


/**
 * RTCを停止します。
 *
 * @retval 0：RTCの停止に失敗しました。
 * @retval 1：RTCの停止に成功しました。
 *
 * @attention なし
 ***************************************************************************/
int rtc_deinit()
{
    RTC0.RCR3.BIT.RTCEN = 0;
    RTC0.RCR4.BIT.RCKSEL = 1;

    // コールバックハンドラの初期化
    g_fRTCInterruptFunc = NULL;

    return 1;
}


/**
 * RTCの時間を設定します。
 *
 * @param[in] time 設定する時刻が格納された構造体を指定します。
 *
 * @retval 0：時刻の設定に失敗しました。
 * @retval 1：時刻の設定に成功しました。
 *
 * @attention 時刻の値はBCDではありません。また、rtc_init()関数を自動的に呼び出し、RTCを初期化します。
 ***************************************************************************/
int rtc_set_time(RTC_TIMETYPE* time)
{

    if(RTC0.RCR3.BIT.RTCEN == 0){
        rtc_init();
    }

    /* Write 0 to RTC start bit */
    RTC0.RCR2.BIT.START = 0x0;
    /* Wait for start bit to clear */
    while(0 != RTC0.RCR2.BIT.START);
    /* Alarm enable bits are undefined after a reset,
       disable non-required alarm features */

    RTC0.RWKAR.BIT.ENB = 0;
    RTC0.RDAYAR.BIT.ENB = 0;
    RTC0.RMONAR.BIT.ENB = 0;
    RTC0.RYRAREN.BIT.ENB = 0;

    /* Operate RTC in 24-hr mode */
    RTC0.RCR2.BIT.HR24 = 0x1;

    RTC0.RYRCNT.WORD  = HEX2BCD(time->year % 100);
    RTC0.RMONCNT.BYTE = HEX2BCD(time->mon);
    RTC0.RDAYCNT.BYTE = HEX2BCD(time->day);
    RTC0.RHRCNT.BYTE  = HEX2BCD(time->hour);
    RTC0.RMINCNT.BYTE = HEX2BCD(time->min);
    RTC0.RSECCNT.BYTE = HEX2BCD(time->second);
    RTC0.RWKCNT.BYTE  = HEX2BCD(time->weekday);

    /* Start the clock */
    RTC0.RCR2.BIT.START = 0x1;
    /* Wait until the start bit is set to 1 */
    while(1 != RTC0.RCR2.BIT.START);

    return 1;
}


/**
 * RTCの時間を取得します。
 *
 * @param[out] time 時刻を格納する構造体を指定します。
 *
 * @retval 0：時刻の取得に失敗しました。
 * @retval 1：時刻の取得に成功しました。
 *
 * @attention 時刻の値はBCDではありません。
 ***************************************************************************/
int rtc_get_time(RTC_TIMETYPE* time)
{
    IEN(RTC, CUP) = 0;
    RTC0.RCR1.BIT.CIE = 1;
    do {
        IR(RTC, CUP) = 0;
        time->year    = BCD2HEX(RTC0.RYRCNT.WORD) + 2000;
        time->mon     = BCD2HEX(RTC0.RMONCNT.BYTE);
        time->day     = BCD2HEX(RTC0.RDAYCNT.BYTE);
        time->hour    = BCD2HEX(0x3f & RTC0.RHRCNT.BYTE);
        time->min     = BCD2HEX(RTC0.RMINCNT.BYTE);
        time->second  = BCD2HEX(RTC0.RSECCNT.BYTE);
        time->weekday = BCD2HEX(RTC0.RWKCNT.BYTE);
    } while (IR(RTC, CUP));
    RTC0.RCR1.BIT.CIE = 0;
    return 1;
}

unsigned short rtc_get_year()
{
    return BCD2HEX(RTC0.RYRCNT.WORD) + 2000;
}

unsigned char rtc_get_mon()
{
    return BCD2HEX(RTC0.RMONCNT.BYTE);
}

unsigned char rtc_get_day()
{
    return BCD2HEX(RTC0.RDAYCNT.BYTE);
}

unsigned char rtc_get_hour()
{
    return BCD2HEX(0x3f & RTC0.RHRCNT.BYTE);
}

unsigned char rtc_get_min()
{
    return BCD2HEX(RTC0.RMINCNT.BYTE);
}

unsigned char rtc_get_second()
{
    return BCD2HEX(RTC0.RSECCNT.BYTE);
}

unsigned char rtc_get_weekday()
{
    return BCD2HEX(RTC0.RWKCNT.BYTE);
}


/**
 * アラーム時に実行するハンドラを登録します。
 *
 * @param[in] fFunction アラーム時に実行するハンドラを指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_attach_alarm_handler(void (*fFunction)(void))
{
    g_fRTCInterruptFunc = fFunction;
}


/**
 * アラーム時間を設定します。
 *
 * @param[in] hour      時を指定します。
 * @param[in] min       分を指定します。
 * @param[in] week_flag 曜日を指定します。複数の曜日を指定する場合は論理和で接続します。
 *
 * @retval 0：アラームの設定に失敗しました。
 * @retval 1：アラームの設定に成功しました。
 *
 * @attention 時刻の値はBCDではありません。
 ***************************************************************************/
int rtc_set_alarm_time(int hour, int min, int week_flag)
{
    /* Configure the alarm as follows -
        Alarm time - 12:00:00
        Enable the hour, minutes and seconds alarm      */
    RTC0.RMINAR.BYTE = HEX2BCD(min);
    RTC0.RHRAR.BYTE  = HEX2BCD(hour);
    if(week_flag <= 0x06){
        RTC0.RWKAR.BYTE  = week_flag;
    }

    RTC0.RMINAR.BIT.ENB = 1;
    RTC0.RHRAR.BIT.ENB = 1;
    if(week_flag <= 0x06){
        RTC0.RWKAR.BIT.ENB = 1;
    } else {
        RTC0.RWKAR.BIT.ENB = 0;
    }

    /* Enable alarm and interrupts*/
    RTC0.RCR1.BIT.AIE = 1;
    while(!RTC0.RCR1.BIT.AIE);

    /* Enable RTC Alarm interrupts */
    IPR(RTC, ALM) = 3u;
    IEN(RTC, ALM) = 1u;
    IR(RTC, ALM)  = 0u;

    return 1;
}

/**
 * アラームをONにします。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_alarm_on()
{
    /* Enable alarm and periodic interrupts*/
    RTC0.RCR1.BIT.AIE = 1;
    while(!RTC0.RCR1.BIT.AIE);
}

/**
 * アラームをOFFにします。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_alarm_off()
{
    /* Disable alarm and periodic interrupts*/
    RTC0.RCR1.BIT.AIE = 0;
    while(RTC0.RCR1.BIT.AIE);
}

/**
 * 時刻補正を設定します
 *
 * @parm [in] adj    補正するカウント値を指定します。(+進み、-遅れ、最大6bit) 補正を停止する場合は0を指定します。補正は10秒毎にadj/32768秒の増減を行います。
 * @parm [in] aadjp  補正のタイミングを指定します。0: 1分毎に補正, 1: 10秒毎に補正を行います。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_correct(int adj, int aadjp)
{
	int tmp_int;
	if (adj == 0) {
		RTC0.RADJ.BYTE = 0x00;						// 補正を停止 PMADJ[1:0] = 00b
		while (RTC0.RADJ.BYTE != 0x00);				// 設定が反映されるまで待機
	}
	else if (adj > 0) {								// adj > 0 の場合
		RTC0.RADJ.BYTE = 0x00;						// 補正を停止 PMADJ[1:0] = 00b
		while (RTC0.RADJ.BYTE != 0x00);				// 設定が反映されるまで待機
		RTC0.RCR2.BIT.AADJE = 1;					// 0: 自動補正禁止, 1: 自動補正許可
		while (RTC0.RCR2.BIT.AADJE != 1);			// AADJE ビットが変更されるまで待つ
		RTC0.RCR2.BIT.AADJP = aadjp == 0 ? 0 : 1;	// 0: 1分毎に補正, 1: 10秒毎に補正
		while (RTC0.RCR2.BIT.AADJP != 1);			// AADJP ビットが変更されるまで待つ
		tmp_int = 0x40 | (0x3F & adj);				// RADJ.PMADJ[1:0] = 01b (進ませる)
		RTC0.RADJ.BYTE = tmp_int;					// RADJ.ADJ[5:0] に補正値の絶対値を設定
		while (RTC0.RADJ.BYTE != tmp_int);			// 設定が反映されるまで待機

	}
	else {											// adj < 0 の場合
		RTC0.RADJ.BYTE = 0x00;						// 補正を停止 PMADJ[1:0] = 00b
		while (RTC0.RADJ.BYTE != 0x00);				// 設定が反映されるまで待機
		RTC0.RCR2.BIT.AADJE = 1;					// 0: 自動補正禁止, 1: 自動補正許可
		while (RTC0.RCR2.BIT.AADJE != 1);			// AADJE ビットが変更されるまで待つ
		RTC0.RCR2.BIT.AADJP = aadjp == 0 ? 0 : 1;   // 0: 1分毎に補正, 1: 10秒毎に補正
		while (RTC0.RCR2.BIT.AADJP != 1);			// AADJP ビットが変更されるまで待つ
		tmp_int = 0x80 | (0x3F & abs(adj));			// RADJ.PMADJ[1:0] = 10b (遅らせる)
		RTC0.RADJ.BYTE = tmp_int;					// RADJ.ADJ[5:0] に補正値の絶対値を設定
		while (RTC0.RADJ.BYTE != tmp_int);			// 設定が反映されるまで待機
	}
}

/// @cond
/**
 * アラームの割り込みハンドラです。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
extern "C" {
void INT_Excep_RTC_ALM(void)
{
     if (g_fRTCInterruptFunc != NULL) {
         (*g_fRTCInterruptFunc)();
     }
    /* Clear the interrupt flag */
    IR(RTC, ALM) = 0;

}
} //extern C
/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/
static inline uint8_t HEX2BCD(int s16HEX)
{
    return ((s16HEX / 10) << 4) | (s16HEX % 10);
}

static inline int BCD2HEX(uint8_t u8BCD)
{
    return ((u8BCD >> 4) * 10) + (u8BCD & 0x0F);
}


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
