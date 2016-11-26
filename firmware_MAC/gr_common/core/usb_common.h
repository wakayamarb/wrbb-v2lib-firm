/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*******************************************************************************/
/* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.   */
/*******************************************************************************
* File Name     : usb_common.h
* Version       : 1.00
* Device        : R5F563NB
* Tool-Chain    : Renesas RX Standard 1.2.0.0
* H/W Platform  : RSK+RX63N
* Description   : Common USB definitions.
*******************************************************************************/
/*******************************************************************************
* History       : 13 Aug. 2012  Ver. 1.00 First Release
*******************************************************************************/

/*******************************************************************************
* Macro Definitions
*******************************************************************************/
/* Multiple inclusion prevention macro */
#ifndef USB_COMMON_H
#define USB_COMMON_H

/*******************************************************************************
* System Includes
*******************************************************************************/
/* Following header file provides standard integer type definitions. */
#include <stdint.h>
/* Following header file provides Boolean definitions. */
#include <stdbool.h>
/* Following header file provides standard library definitions. */
#include <stdlib.h>
#ifndef RELEASE
    #include <stdio.h>
#endif


/*******************************************************************************
* Defines
*******************************************************************************/
/*Debug messages*/
/*The debug level - decides what level of message will be output*/
#define DEBUG_LEVEL_NONE    0 /*No Debug*/
#define DEBUG_LEVEL_LOW     1 /*Low amount of debug info.*/
#define DEBUG_LEVEL_MID     2 /*Mid amount of debug info.*/
#define DEBUG_LEVEL_HIGH    3 /*High amount of debug info.*/

/*Set the current level of debug output using one of the #defines above.*/
/*WARNING NOTE: Too high a level of debug message output may cause problems
with some drivers, where they cause the interrupt handling to take too long.*/
#define DEBUG_LEVEL     DEBUG_LEVEL_NONE

/*MACROS used to ouput DEBUG messages depending upon set DEBUG_LEVEL
1. DEBUG_MSG_HIGH:  Output msg if DEBUG_LEVEL is DEBUG_LEVEL_HIGH
2. DEBUG_MSG_MID:   Output msg if DEBUG_LEVEL is DEBUG_LEVEL_MID or higher.
3. DEBUG_MSG_LOW:   Output msg if DEBUG_LEVEL is DEBUG_LEVEL_LOW or higher.*/

/*Only allow debug messages in a debug build*/
#ifndef RELEASE
    #if DEBUG_LEVEL >= DEBUG_LEVEL_HIGH
        #define DEBUG_MSG_HIGH(x) printf x
    #else
        #define DEBUG_MSG_HIGH(x)
    #endif

    #if DEBUG_LEVEL >= DEBUG_LEVEL_MID
        #define DEBUG_MSG_MID(x) printf x
    #else
        #define DEBUG_MSG_MID(x)
    #endif

    #if DEBUG_LEVEL >= DEBUG_LEVEL_LOW
        #define DEBUG_MSG_LOW(x) printf x
    #else
        #define DEBUG_MSG_LOW(x)
    #endif
#else
    /*RELEASE */
    #define DEBUG_MSG_HIGH(x)
    #define DEBUG_MSG_MID(x)
    #define DEBUG_MSG_LOW(x)
#endif

/*A Setup Packet is always 8 bytes*/
#define USB_SETUP_PACKET_SIZE 8

#ifndef true
#define true      (1)
#define false     (0)
#endif
/*******************************************************************************
* USB Error Values
*******************************************************************************/
#define USB_ERR_OK                  ((USB_ERR)0)
#define USB_ERR_FAIL                ((USB_ERR)-1)
#define USB_ERR_PARAM               ((USB_ERR)-2)
#define USB_ERR_STATE               ((USB_ERR)-3)
#define USB_ERR_BULK_OUT            ((USB_ERR)-4)
#define USB_ERR_BULK_OUT_NO_BUFFER  ((USB_ERR)-5)
#define USB_ERR_CONTROL_OUT         ((USB_ERR)-6)
#define USB_ERR_NOT_CONNECTED       ((USB_ERR)-7)
#define USB_ERR_UNKNOWN_REQUEST     ((USB_ERR)-8)
#define USB_ERR_INVALID_REQUEST     ((USB_ERR)-9)
#define USB_ERR_CANCEL              ((USB_ERR)-10)
#define USB_ERR_BUSY                ((USB_ERR)-11)

/*******************************************************************************
Type Definitions
*******************************************************************************/
typedef int16_t USB_ERR;

/*USB_COMMON_H*/
#endif
