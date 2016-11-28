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
* File Name     : usb_core.h
* Version       : 1.00
* Device        : R5F563NB
* Tool-Chain    : Renesas RX Standard 1.2.0.0
* H/W Platform  : RSK+RX63N
* Description   : The USB core sits above the HAL in the USB stack.
*                 Initilaises the HAL.
*                 Handles standard USB setup requests.
*
*               NOTE: This only supports 1 language ID and that
*                     is currently English.
*******************************************************************************/
/*******************************************************************************
* History       : 13 Aug. 2012  Ver. 1.00 First Release
*******************************************************************************/

/*******************************************************************************
* Macro Definitions
*******************************************************************************/
/* Multiple inclusion prevention macro */
#ifndef USB_CORE_H
#define USB_CORE_H

/*******************************************************************************
* User Includes
*******************************************************************************/
/* Following header file provides common USB definitions. */
#include "usb_common.h"
/*******************************************************************************
* Defines
*******************************************************************************/
/* Standard setup Packet Request Codes */
#define GET_STATUS          0x00
#define CLEAR_FEATURE       0x01
#define SET_FEATURE         0x03
#define SET_ADDRESS         0x05
#define GET_DESCRIPTOR      0x06
#define SET_DESCRIPTOR      0x07
#define GET_CONFIGURATION   0x08
#define SET_CONFIGURATION   0x09
#define GET_INTERFACE       0x0A
#define SET_INTERFACE       0x0B
#define SYNCH_FRAME         0x0C

/* Descriptor Types */
#define DEVICE              1
#define CONFIGURATION       2
#define STRING              3
#define INTERFACE           4
#define ENDPOINT            5
#define DEVICE_QUALIFIER    6

/*Setup Packet bmRequest d6..5 values */
#define REQUEST_STANDARD    0
#define REQUEST_CLASS       1
#define REQUEST_VENDOR      2

/*String Descriptor Types*/
#define STRING_iMANUFACTURER    1
#define STRING_iPRODUCT         2
#define STRING_iSERIAL          3

/*Macros used to extract bits from bmRequest byte in the SetupPacket*/
/*NOTE: Not using bitfields for bmRequest as bit order is nor ANSI defined
and therefore can cause portability issues.*/
/*Recipient*/
#define EXTRACT_bmRequest_RECIPIENT(bmRequest)  ((uint8_t)(bmRequest & 0x1F))
/*Type*/
#define EXTRACT_bmRequest_TYPE(bmRequest)       ((uint8_t)((bmRequest >> 5) & 0x3))
/*Direction*/
#define EXTRACT_bmRequest_DIRECTION(bmRequest)  ((uint8_t)((bmRequest >> 7) & 0x1))

/*******************************************************************************
* Type Definitions
*******************************************************************************/

/*Setup packet */
typedef struct SetupPacket
{
    /* Request feature */
    uint8_t bmRequest;
    /* Request number */
    uint8_t bRequest;
    /* Field that varies according to request */
    uint16_t wValue;
    /* Field that varies according to request;
     Typically used to pass an index or offset*/
    uint16_t wIndex;
    /* Number of bytes to transfer if there is a Data Stage */
    uint16_t wLength;
}SetupPacket;

/*Call back definition for a received setup packet */
typedef USB_ERR(*CB_SETUP_PACKET)(SetupPacket*, uint16_t* _pNumBytes, uint8_t** _ppBuffer);

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
USB_ERR USBCORE_Init(const uint8_t* _Manufacturer, uint16_t _ManufacturerSize,
                     const uint8_t* _Product, uint16_t _ProductSize,
                     const uint8_t* _Serial, uint16_t _SerialSize,
                     const uint8_t* _DeviceDescriptor, uint16_t _DeviceDescriptorSize,
                     const uint8_t* _ConfigDescriptor, uint16_t _ConfigDescriptorSize,
                     CB_SETUP_PACKET _fpCBSetupPacket,
                     CB_DONE_OUT _fpCBControlOut,
                     CB_CABLE _fpCBCable,
                     CB_ERROR _fpCBError
                     );

/*USB_CORE_H*/
#endif
