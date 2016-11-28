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
* File Name     : USB_HAL.h
* Version       : 1.00
* Device        : R5F563NB
* Tool-Chain    : Renesas RX Standard 1.2.0.0
* H/W Platform  : RSK+RX63N
* Description   : Hardware Abstraction Layer (HAL)
*
*           Provides a hardware independent API to the USB peripheral
*           on the RX63N.
*           Supports:-
*           Control IN, Control OUT, Bulk IN, Bulk OUT and Interrupt IN.
***********************************************************************************/
/***********************************************************************************
* History       : 13 Aug. 2012  Ver. 1.00 First Release
***********************************************************************************/

#ifndef _USB_HAL_H_INCLUDED_
#define _USB_HAL_H_INCLUDED_

/***********************************************************************************
* User Includes
***********************************************************************************/
#include "usb_common.h"

/***********************************************************************************
* Defines
***********************************************************************************/
/*Select the USB module to use by commenting out the unused module*/
#define USB_MODULE_0

/*Set the priority of the USB interrupts according to your syastem.*/
#define USB_HAL_INTERRUPT_PROIRITY 9

/*USB packet sizes*/
#define CONTROL_IN_PACKET_SIZE      64
#define CONTROL_OUT_PACKET_SIZE     64
#define BULK_IN_PACKET_SIZE         64
#define BULK_OUT_PACKET_SIZE        64
#define INTERRUPT_IN_PACKET_SIZE    64


/*This HAL sets up a BULK IN, BULK OUT and Interrupt IN endpoints*/
/*BULK OUT - Use Pipe1, EP 1*/
#define PIPE_BULK_OUT       1
#define EP_BULK_OUT         1

/*BULK IN - Use Pipe2, EP 2*/
#define PIPE_BULK_IN        2
#define EP_BULK_IN          2

/*INTERRUPT IN - Use Pipe6, EP 3*/
#define PIPE_INTERRUPT_IN   6
#define EP_INTERRUPT_IN     3

/***********************************************************************************
* Type Definitions
***********************************************************************************/
/* Callback function type definitions */
typedef void(*CB_SETUP)(const uint8_t(*)[USB_SETUP_PACKET_SIZE]);
typedef void(*CB_CABLE)(bool);
typedef void(*CB_ERROR)(USB_ERR);
typedef void(*CB_DONE)(USB_ERR);
typedef void(*CB_DONE_OUT)(USB_ERR, uint32_t);

typedef struct USBHAL_CONFIG
{
    /*Set TRUE if don't need a BULK IN to end on a short packet.*/
    /*Mass Storage doesn't use short packets to end a BULK IN.
    Default FALSE*/
    bool m_bBULK_IN_No_Short_Packet;
    /*m_bAllowStallClear: Set false to prevent a stall condition
    being cleared by a Clear Feature request.
    Default true*/
    bool m_bAllowStallClear;
}USBHAL_CONFIG;

/***********************************************************************************
* Function Prototypes
***********************************************************************************/
USB_ERR USBHAL_Init(CB_SETUP _fpSetup, CB_CABLE _fpCable,
                     CB_ERROR _fpError);

USB_ERR USBHAL_Disable(void);

/*Configuration*/
const USBHAL_CONFIG* USBHAL_Config_Get(void);
USB_ERR USBHAL_Config_Set(USBHAL_CONFIG* _pConfig);

/*Control*/
USB_ERR USBHAL_Control_Status(void);
USB_ERR USBHAL_Control_IN(uint16_t _NumBytes, const uint8_t* _Buffer);
USB_ERR USBHAL_Control_OUT(uint16_t _NumBytes, uint8_t* _Buffer, CB_DONE_OUT _CBDone);
/*Bulk*/
USB_ERR USBHAL_Bulk_IN(uint32_t _NumBytes, const uint8_t* _Buffer, CB_DONE _CBDone);
USB_ERR USBHAL_Bulk_OUT(uint32_t _NumBytes, uint8_t* _Buffer, CB_DONE_OUT _CBDone);

/*Interrupt*/
USB_ERR USBHAL_Interrupt_IN(uint32_t _NumBytes, const uint8_t* _Buffer, CB_DONE _CBDone);

/*Cancel all pending operations and call callbacks*/
USB_ERR USBHAL_Cancel(USB_ERR _err);

/*Reset module - callbacks not called*/
USB_ERR USBHAL_Reset(void);
USB_ERR USBHAL_ResetEndpoints(void);

/*Stall*/
void USBHAL_Control_Stall(void);
void USBHAL_Bulk_IN_Stall(void);
void USBHAL_Bulk_OUT_Stall(void);
void USBHAL_Interrupt_IN_Stall(void);
/*Stall clear */
void USBHAL_Bulk_IN_Stall_Clear(void);
void USBHAL_Bulk_OUT_Stall_Clear(void);
void USBHAL_Interrupt_IN_Stall_Clear(void);
/*Stall Status*/
bool USBHAL_Bulk_IN_Is_Stalled(void);
bool USBHAL_Bulk_OUT_Is_Stalled(void);
bool USBHAL_Interrupt_IN_Is_Stalled(void);

/*USB Interrupt Handler*/
void Interrupt_USBI0(void);
void USBHALInterruptHandler(void);
void USBHALInterruptHandler2(void);

/*USB_HAL_H*/
#endif
