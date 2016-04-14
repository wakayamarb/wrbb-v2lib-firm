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
* File Name     : usbdescriptors.h
* Version       : 1.00
* Device        : Renesas Generic MCU.
* Tool-Chain    : HEW
* H/W Platform  : RSK Generic
* Description   : Descriptors required to enumerate a device as a
*                    Communication Device Class - Abstract Class Model(CDC ACM).
*
* NOTE          : This will need to be modified for a particular
*                 product as it includes company/product specific data including
*                 string descriptors specifying
*                 Manufacturer, Product and Serial Number.
***********************************************************************************/
/***********************************************************************************
* History       : 23.01.2012 Ver. 1.00 First Release
***********************************************************************************/

#ifndef FILENAME_USBDESCRIPTORS_H
#define FILENAME_USBDESCRIPTORS_H

/***********************************************************************************
System Includes
***********************************************************************************/
#include <stdint.h>

/***********************************************************************************
Type Definitions
***********************************************************************************/
/*    Descriptor    */
typedef    struct DESCRIPTOR
{
    const uint16_t length;
    const uint8_t* pucData;
}DESCRIPTOR;

/* Device Descriptor */
extern const DESCRIPTOR gDeviceDescriptor;

/* Configuration, Interface, CDC and Endpoint Descriptor */
extern const DESCRIPTOR gConfigurationDescriptor;

/* String descriptors */
extern const DESCRIPTOR gStringDescriptorLanguageIDs;
extern const DESCRIPTOR gStringDescriptorManufacturer;
extern const DESCRIPTOR gStringDescriptorProduct;
extern const DESCRIPTOR gStringDescriptorSerialNum;

/* FILENAME_USBDESCRIPTORS_H */
#endif
