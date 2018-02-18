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
* File Name     : usb_cdc.h
* Version       : 1.00
* Device        : Renesas Generic MCU.
* Tool-Chain    : HEW
* H/W Platform  : RSK Generic
* Description   : USB CDC Class (Abstract Class Model).
                    Provides a virtual COM port.
******************************************************************************/
/******************************************************************************
* History       : 23.01.2012 Ver. 1.00 First Release
******************************************************************************/

#ifndef USB_CDC_H
#define USB_CDC_H

/******************************************************************************
User Includes
******************************************************************************/
#include "usb_common.h"

/******************************************************************************
Function Prototypes
******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

USB_ERR USBCDC_Init(void);
bool USBCDC_IsConnected(void);
USB_ERR USBCDC_WriteString(const char* const _sz);
USB_ERR USBCDC_PutChar(uint8_t _Char);
USB_ERR USBCDC_GetChar(uint8_t* _pChar);
USB_ERR USBCDC_Write(uint32_t _NumBytes, const uint8_t* _Buffer);
USB_ERR USBCDC_Write_Async(uint32_t _NumBytes, const uint8_t* _Buffer, CB_DONE _cb);
USB_ERR USBCDC_Read(uint32_t _BufferSize, uint8_t* _Buffer, uint32_t* _pNumBytesRead);
USB_ERR USBCDC_Read_Async(uint32_t _BufferSize, uint8_t* _Buffer, CB_DONE_OUT _cb);
USB_ERR USBCDC_Cancel(void);
bool USBCDC_IsBreakState(void);
bool USBCDC_Clear_BreakState(void);

#ifdef __cplusplus
}
#endif
/*USB_HID_H*/
#endif
