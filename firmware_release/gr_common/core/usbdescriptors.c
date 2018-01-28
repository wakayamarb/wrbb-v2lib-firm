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
* File Name     : usbdescriptors.c
* Version       : 1.00
* Device        : R5F563NB
* Tool-Chain    : Renesas RX Standard 1.2.0.0
* H/W Platform  : RSK+RX63N
* Description   : Descriptors required to enumerate a device as a
*                  Communication Device Class - Abstract Class Model(CDC ACM).
*
*                 NOTE: This will need to be modified for a particular
*                  product as it includes company/product specific data including
*                  string descriptors specifying
*                  Manufacturer, Product and Serial Number.
***********************************************************************************/
/***********************************************************************************
* History       : 13 Aug. 2012  Ver. 1.00 First Release
***********************************************************************************/
/*
 *  Modified 12 May 2014 by Yuuki Okamiya, modify descriptor for GR-SAKURA lib2.xx
 */

/***********************************************************************************
User Includes
***********************************************************************************/
#include "usb_hal.h"
#include "usb_core.h"
#include "USBDescriptors.h"

/***********************************************************************************
Defines
***********************************************************************************/
/*Vendor and Product ID*/
/*NOTE Please use your company Vendor ID when developing a new product.*/
#ifdef GRSAKURA
#define VID 0x045B
#define PID 0x0234
#elif defined(GRCITRUS)
#define VID 0x2A50
#define PID 0x0277
#endif
/***********************************************************************************
Variables
***********************************************************************************/
/*    Device Descriptor    */
#define DEVICE_DESCRIPTOR_SIZE    18
static const uint8_t gDeviceDescriptorData[DEVICE_DESCRIPTOR_SIZE] =
{
    /*Size of this descriptor*/
    DEVICE_DESCRIPTOR_SIZE,
    /*Device Descriptor*/
    0x01,
    /*USB Version 2.0*/
    0x00,0x02,
    /*Class Code - CDC*/
    0x02,
    /*Subclass Code*/
    0x00,
    /*Protocol Code*/
    0x00,
    /*Max Packet Size for endpoint 0*/
    CONTROL_IN_PACKET_SIZE,
    (uint8_t)(VID & 0xFF),    /*Vendor ID LSB*/
    (uint8_t)((VID>>8)& 0xFF),/*Vendor ID MSB*/
    (uint8_t)(PID & 0xFF),    /*Product ID LSB*/
    (uint8_t)((PID>>8)& 0xFF),/*Product ID MSB*/
    /*Device Release Number*/
    0x00,0x01,
    /*Manufacturer String Descriptor*/
    STRING_iMANUFACTURER,
    /*Product String Descriptor*/
    STRING_iPRODUCT,
    /*Serial Number String Descriptor*/
    STRING_iSERIAL,
    /*Number of Configurations supported*/
    0x01
};

const DESCRIPTOR gDeviceDescriptor =
{
    DEVICE_DESCRIPTOR_SIZE, gDeviceDescriptorData
};

/*Configuration Descriptor*/
#define CONFIG_DESCRIPTOR_SIZE 67
static const uint8_t gConfigurationDescriptorData[CONFIG_DESCRIPTOR_SIZE] =
{
    /*Size of this descriptor (Just the configuration part)*/
    0x09,
    /*Configuration Descriptor*/
    0x02,
    /*Combined length of all descriptors (little endian)*/
    CONFIG_DESCRIPTOR_SIZE,0x00,
    /*Number of interfaces*/
    0x02,
    /*This Interface Value*/
    0x01,
    /*No String Descriptor for this configuration*/
    0x00,
    /*bmAttributes - Self Powered(USB bus powered), No Remote Wakeup*/
    0xC0,
    /*bmAttributes - Not USB Bus powered, No Remote Wakeup*/
    /*0x80,*/
    /*bMaxPower (2mA units) 100mA (A unit load is defined as 100mA)*/
    50,

/* Communication Class Interface Descriptor */
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
    0x00,
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints*/
    0x01,
    /*Class code = Communication*/
    0x02,
    /*Subclass = Abstract Control Model*/
    0x02,
    /*No Protocol*/
    0x00,
    /*No String Descriptor for this interface*/
    0x00,

/*Header Functional Descriptor*/
    /*bFunctionalLength*/
    0x05,
    /*bDescriptorType = CS_INTERFACE*/
    0x24,
    /*bDescriptor Subtype = Header*/
    0x00,
    /*bcdCDC 1.1*/
    0x10,0x01,

/* ACM Functional Descriptor */
    /*bFunctionalLength*/
    0x04,
    /*bDescriptorType = CS_INTERFACE*/
    0x24,
    /*bDescriptor Subtype = Abstract Control Management*/
    0x02,
    /*bmCapabilities GET_LINE_CODING etc supported*/
    0x02,

/* Union Functional Descriptor */
    /*bFunctionalLength*/
    0x05,
    /*bDescriptorType = CS_INTERFACE*/
    0x24,
    /*bDescriptor Subtype = Union*/
    0x06,
    /*bMasterInterface = Communication Class Interface*/
    0x00,
    /*bSlaveInterface = Data Class Interface*/
    0x01,

/* Call Management Functional Descriptor */
    /*bFunctionalLength*/
    0x05,
    /*bDescriptorType = CS_INTERFACE*/
    0x24,
    /*bDescriptor Subtype = Call Management*/
    0x01,
    /*bmCapabilities*/
    0x00,
    /*bDataInterface: Data Class Interface = 1*/
    0x01,

/* Interrupt Endpoint */
    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 3*/
    0x83,
    /*Endpoint Type is Interrupt*/
    0x03,
    /*Max Packet Size*/
    0x08,0x00,
    /*Polling Interval in mS*/
    0xFF,

/* DATA Class Interface Descriptor */
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
    0x01,
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints*/
    0x02,
    /*Class code = Data Interface*/
    0x0A,
    /*Subclass = Not Used, set as 0*/
    0x00,
    /*No Protocol*/
    0x00,
    /*No String Descriptor for this interface*/
    0x00,

/*Endpoint Bulk OUT */
    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - OUT endpoint, endpoint number = 1*/
    0x01,
    /*Endpoint Type is BULK*/
    0x02,
    /*Max Packet Size*/
    64,0x00,
    /*Polling Interval in mS - IGNORED FOR BULK*/
    0x00,

/* Endpoint Bulk IN */
    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 2*/
    0x82,
    /*Endpoint Type is BULK*/
    0x02,
    /*Max Packet Size*/
    64,0x00,
    /*Polling Interval in mS - IGNORED FOR BULK*/
    0x00
};

const DESCRIPTOR gConfigurationDescriptor =
{
    CONFIG_DESCRIPTOR_SIZE, gConfigurationDescriptorData
};

/*String Descriptors*/
    /*Note Language ID is in USB Core */

/*Manufacturer string*/
#define STRING_MANUFACTURER_SIZE 16
/* "Renesas" */
static const uint8_t gStringDescriptorManufacturerData[STRING_MANUFACTURER_SIZE] =
{
    /* Length of this descriptor*/
    STRING_MANUFACTURER_SIZE,
    /* Descriptor Type = STRING */
    0x03,
    /* Descriptor Text (unicode) */
    'R', 0x00, 'E', 0x00, 'N', 0x00, 'E', 0x00,
    'S', 0x00, 'A', 0x00, 'S', 0x00
};

const DESCRIPTOR  gStringDescriptorManufacturer =
{
    STRING_MANUFACTURER_SIZE,
    gStringDescriptorManufacturerData
};

/*Product string*/
#define STRING_PRODUCT_SIZE 44
/* "CDC USB Demonstration" */
static const uint8_t gStringDescriptorProductData[STRING_PRODUCT_SIZE] =
{
    /* Length of this descriptor*/
    STRING_PRODUCT_SIZE,
    /* Descriptor Type = STRING */
    0x03,
    /* Descriptor Text (unicode) */
#ifdef GRSAKURA
    'G', 0x00, 'a', 0x00, 'd', 0x00, 'g', 0x00,
    'e', 0x00, 't', 0x00, ' ', 0x00, 'R', 0x00,
    'e', 0x00, 'n', 0x00, 'e', 0x00, 's', 0x00,
    'a', 0x00, 's', 0x00, ' ', 0x00, 'S', 0x00,
    'A', 0x00, 'K', 0x00, 'U', 0x00, 'R', 0x00,
    'A', 0x00
#elif defined GRCITRUS
    'G', 0x00, 'a', 0x00, 'd', 0x00, 'g', 0x00,
    'e', 0x00, 't', 0x00, ' ', 0x00, 'R', 0x00,
    'e', 0x00, 'n', 0x00, 'e', 0x00, 's', 0x00,
    'a', 0x00, 's', 0x00, ' ', 0x00, 'C', 0x00,
    'I', 0x00, 'T', 0x00, 'R', 0x00, 'U', 0x00,
    'S', 0x00
#endif
};

const DESCRIPTOR gStringDescriptorProduct =
{
    STRING_PRODUCT_SIZE,
    gStringDescriptorProductData
};

/*Serial number string "1.1"*/
#define STRING_SERIAL_NUM_SIZE 8
static const uint8_t gStringDescriptorSerialNumData[STRING_SERIAL_NUM_SIZE] =
{
    /* Length of this descriptor*/
    STRING_SERIAL_NUM_SIZE,
    /* Descriptor Type = STRING */
    0x03,
    /* Descriptor Text (unicode) */
    '1', 0x00, '.', 0x00, '1', 0x00
};

const DESCRIPTOR gStringDescriptorSerialNum =
{
    STRING_SERIAL_NUM_SIZE,
    gStringDescriptorSerialNumData
};
