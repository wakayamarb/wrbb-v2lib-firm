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
 * @file  RTC.h
 * @brief RTC（リアル・タイム・クロック）関数 ヘッダファイル
 *
 * Modified 27th May 2014 by Yuuki Okamiya from RL78duino.cpp
 */
#ifndef RTC_H
#define RTC_H
/***************************************************************************/
/*    Include MCU depend defines.                                          */
/***************************************************************************/
#include "Arduino.h"


/***************************************************************************/
/*    Interrupt handler                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/


/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define RTC_WEEK_SUNDAY     0x00    //!< 曜日設定（日曜日）
#define RTC_WEEK_MONDAY     0x01    //!< 曜日設定（月曜日）
#define RTC_WEEK_TUESDAY    0x02    //!< 曜日設定（月曜日）
#define RTC_WEEK_WEDNESDAY  0x03    //!< 曜日設定（月曜日）
#define RTC_WEEK_THURSDAY   0x04    //!< 曜日設定（月曜日）
#define RTC_WEEK_FRIDAY     0x05    //!< 曜日設定（月曜日）
#define RTC_WEEK_SATURDAY   0x06    //!< 曜日設定（月曜日）

#define RTC_ALARM_SUNDAY    0x01    //!< アラーム曜日設定（日曜日）
#define RTC_ALARM_MONDAY    0x02    //!< アラーム曜日設定（月曜日）
#define RTC_ALARM_TUESDAY   0x04    //!< アラーム曜日設定（火曜日）
#define RTC_ALARM_WEDNESDAY 0x08    //!< アラーム曜日設定（水曜日）
#define RTC_ALARM_THURSDAY  0x10    //!< アラーム曜日設定（木曜日）
#define RTC_ALARM_FRIDAY    0x20    //!< アラーム曜日設定（金曜日）
#define RTC_ALARM_SATURDAY  0x40    //!< アラーム曜日設定（土曜日）
#define RTC_ALARM_EVERYDAY  (RTC_ALARM_SUNDAY | RTC_ALARM_MONDAY | RTC_ALARM_TUESDAY | RTC_ALARM_WEDNESDAY | RTC_ALARM_THURSDAY | RTC_ALARM_FRIDAY | RTC_ALARM_SATURDAY)        //!< アラーム曜日指定（毎日）


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/*! RTC構造体 */
typedef struct tagRTC_TIMETYPE {
    unsigned short  year;   //!< 年
    unsigned char   mon;    //!< 月
    unsigned char   day;    //!< 日
    unsigned char   weekday;//!< 曜日
    unsigned char   hour;   //!< 時
    unsigned char   min;    //!< 分
    unsigned char   second; //!< 秒
} RTC_TIMETYPE;


/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/
int rtc_init();
int rtc_deinit();
int rtc_set_time(RTC_TIMETYPE* time);
int rtc_get_time(RTC_TIMETYPE* time);
void rtc_attach_alarm_handler(void (*fFunction)(void));
int rtc_set_alarm_time(int hour, int min, int week_flag = RTC_ALARM_EVERYDAY);
void rtc_alarm_on();
void rtc_alarm_off();


/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/


/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
#endif /* RTC_H */
