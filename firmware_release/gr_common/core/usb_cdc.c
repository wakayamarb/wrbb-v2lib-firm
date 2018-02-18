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
* File Name     : usb_cdc.c
* Version       : 1.00
* Device        : R5F563NB
* Tool-Chain    : Renesas RX Standard 1.2.0.0
* H/W Platform  : RSK+RX63N
* Description   : USB CDC Class (Abstract Class Model).
                    Provides a virtual COM port.
***********************************************************************************/
/***********************************************************************************
* History       : 13 Aug. 2012  Ver. 1.00 First Release
***********************************************************************************/

/***********************************************************************************
* System Includes
***********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>

/***********************************************************************************
* User Includes
***********************************************************************************/
#include "usb_common.h"
#include "usb_hal.h"
#include "usb_core.h"
#include "usbdescriptors.h"
#include "usb_cdc.h"

/***********************************************************************************
* Defines
***********************************************************************************/
/* CDC Class Requests IDs*/
#define SET_LINE_CODING                     0x20
#define GET_LINE_CODING                     0x21
#define SET_CONTROL_LINE_STATE              0x22
#define SEND_BREAK                          0x23

/*Size of GET_LINE_CODING response data*/
#define LINE_CODING_DATA_SIZE               7
/*Size of SET_CONTROL_LINE_STATE data*/
#define SET_CONTROL_LINE_STATE_DATA_SIZE    7

/***********************************************************************************
* Local Types
***********************************************************************************/
/*Data structure for SET_CONTROL_LINE_STATE*/
typedef struct SET_CONTROL_LINE_STATE_DATA
{
    uint32_t dwDTERate;
    uint8_t bCharFormat;
    uint8_t bParityType;
    uint8_t bDataBits;
}SET_CONTROL_LINE_STATE_DATA;

/*Structure for BULK OUT*/
typedef struct BULK_OUT
{
    volatile bool m_Busy;    /*Busy Flag*/
    uint32_t m_BytesRead;     /*Number of bytes read*/
    USB_ERR m_Err;            /*Error Status*/
    CB_DONE_OUT m_CBDone;    /*Callback done*/
}BULK_OUT;

/*Structure for BULK IN*/
typedef struct BULK_IN
{
    volatile bool m_Busy;    /*Busy Flag*/
    USB_ERR m_Err;            /*Error Status*/
    CB_DONE m_CBDone;        /*Callback done*/
}BULK_IN;

/***********************************************************************************
* Variables
***********************************************************************************/
/*Data Bufer for SET_CONTROL_LINE_STATE data*/
static uint8_t g_SET_CONTROL_LINE_STATE_DATA_Buffer[SET_CONTROL_LINE_STATE_DATA_SIZE];

/*GET_LINE_CODING data response*/
/*Note: These values are not relevant for a virtual COM port.*/
static const uint8_t g_LineCoding[LINE_CODING_DATA_SIZE] =
{
    0x00, 0xc2, 0x01, 0x00,        /* 115200 baud = h'0001c200 - send little endian */
    0x00,                        /* 1 stop bit */
    0x00,                        /* Parity = None */
    0x08                        /* 8 Data Bits */
};

/*Data structure for SET_CONTROL_LINE_STATE*/
static SET_CONTROL_LINE_STATE_DATA g_SetControlLineData;

/*Connected flag*/
static volatile bool g_bConnected = false;

/*Break flag*/
static bool g_bBreakState = false;

/*Bulk Out specific*/
static BULK_OUT g_BulkOut = {
    false,
    0,
    USB_ERR_OK,
    NULL
};

/*Bulk In specific*/
static BULK_IN g_BulkIn = {
    false,
    USB_ERR_OK,
    NULL
};

/***********************************************************************************
* Private Function Prototypes
***********************************************************************************/
static void InitialiseData(void);
static USB_ERR ProcessClassSetupPacket(SetupPacket* _pSetupPacket,
                                        uint16_t* _pNumBytes,
                                        uint8_t** _ppBuffer);

/*Callbacks required by USB CORE*/
static USB_ERR CBUnhandledSetupPacket(SetupPacket* _pSetupPacket,
                                        uint16_t* _pNumBytes,
                                        uint8_t** _ppBuffer);
static void CBDoneControlOut(USB_ERR _err, uint32_t _NumBytes);
static void CBDoneBulkOut(USB_ERR _err, uint32_t _NumBytes);
static void CBDoneBulkIn(USB_ERR _err);
static void CBCable(bool _bConnected);
static void CBError(USB_ERR _err);

static void ReleaseFlags(USB_ERR _err);

/**********************************************************************
* Outline       : USBCDC_Init
* Description   : Initialize this module.
*                  This must be called once before using any of the
*                  other API functions.
*                  Initializes the USB Core layer.
* Argument      : none
* Return value  : Error code.
**********************************************************************/
USB_ERR USBCDC_Init(void)
{
    USB_ERR err;

    /*Initialize this modules data*/
    InitialiseData();

    /*Initialize the USB core*/
    err = USBCORE_Init(gStringDescriptorManufacturer.pucData,
                       gStringDescriptorManufacturer.length,
                       gStringDescriptorProduct.pucData,
                       gStringDescriptorProduct.length,
                       gStringDescriptorSerialNum.pucData,
                       gStringDescriptorSerialNum.length,
                       gDeviceDescriptor.pucData,
                       gDeviceDescriptor.length,
                       gConfigurationDescriptor.pucData,
                       gConfigurationDescriptor.length,
                       (CB_SETUP_PACKET)CBUnhandledSetupPacket,
                       (CB_DONE_OUT)CBDoneControlOut,
                       (CB_CABLE)CBCable,
                       (CB_ERROR)CBError);
    return err;
}
/***********************************************************************************
* End of function USBCDC_Init
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_IsConnected
* Description   : Get the USB cable connected state.
* Argument      : none
* Return value  : true = Connected, false = Disconnected.
**********************************************************************/
bool USBCDC_IsConnected(void)
{
    return (bool)g_bConnected;
}
/***********************************************************************************
* End of function USBCDC_IsConnected
***********************************************************************************/

/**********************************************************************
* Outline         : USBCDC_WriteString
* Description   : Writes string. (BULK IN)
*                  Blocks untill string has been written or error.
*                  Note: As this is a blocking function it must not be
*                  called from an interrupt with higher priority than
*                  the USB interrupts.
* Argument      : -sz: String to write (NULL Terminated)
* Return value  : Error code.
**********************************************************************/
USB_ERR USBCDC_WriteString(const char* const _sz)
{
    USB_ERR err;

    uint32_t NumBytes = (uint32_t)strlen(_sz);

    err = USBCDC_Write(NumBytes, (const uint8_t*)_sz);

    return err;
}
/**********************************************************************************
* End of function USBCDC_WriteString
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_GetChar
* Description   : Read 1 character (BULK OUT).
*                  Note: As this is a blocking function it must not be
*                  called from an interrupt with higher priority than
*                  the USB interrupts.
* Argument      : _pChar: Pointer to Byte to read into.
* Return value  : Error code.
**********************************************************************/
USB_ERR USBCDC_GetChar(uint8_t* _pChar)
{
    USB_ERR err;
    uint32_t dummy;

    err = USBCDC_Read(1, _pChar, &dummy);

    return err;
}
/**********************************************************************************
* End of function USBCDC_GetChar
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_PutChar
* Description   : Write 1 character (BULK IN).
*                  Note: As this is a blocking function it must not be
*                  called from an interrupt with higher priority than
*                  the USB interrupts.
* Argument      : _Char: Byte to write.
* Return value  : Error code.
**********************************************************************/
USB_ERR USBCDC_PutChar(uint8_t _Char)
{
    return USBCDC_Write(1, &_Char);
}
/**********************************************************************************
* End of function USBCDC_PutChar
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_Write
* Description   : Perform a blocking write (BULK IN)
*                  Note: As this is a blocking function it must not be
*                  called from an interrupt with higher priority than
*                  the USB interrupts.
* Argument      : _NumBytes:     Number of bytes to write.
*                  _Buffer:    Data Buffer.
* Return value  : Error code.
**********************************************************************/
USB_ERR USBCDC_Write(uint32_t _NumBytes, const uint8_t* _Buffer)
{
    USB_ERR err;

    /*This can not complete untill connected*/
    if(true == g_bConnected)
    {
        /*Check not already busy*/
        if(false == g_BulkIn.m_Busy)
        {
            /*Set Flag to wait on*/
            g_BulkIn.m_Busy = true;
            g_BulkIn.m_CBDone = NULL; /*No call back this is a blocking function */
            err = USBHAL_Bulk_IN(_NumBytes, _Buffer, (CB_DONE)CBDoneBulkIn);
            if(USB_ERR_OK == err)
            {
                /*Wait for flag that will be cleared when the operation has completed*/
                while(true == g_BulkIn.m_Busy){};
                err = g_BulkIn.m_Err;
            }
            else
            {
                g_BulkIn.m_Busy = false;
            }
        }
        else
        {
            err = USB_ERR_BUSY;
        }
    }
    else
    {
        err = USB_ERR_NOT_CONNECTED;
    }

    return err;
}
/***********************************************************************************
* End of function USBCDC_Write
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_Write_Async
* Description   : Start an asynchronous write. (BULK IN)
* Argument      : _NumBytes:     Number of bytes to write.
*                  _Buffer:    Data Buffer.
*                  _cb:         Callback when done.
* Return value  : Error Code.
**********************************************************************/
USB_ERR USBCDC_Write_Async(uint32_t _NumBytes, const uint8_t* _Buffer, CB_DONE _cb)
{
    USB_ERR err;

    /*This can not complete untill connected*/
    if(true == g_bConnected)
    {
        /*Check not already busy*/
        if(false == g_BulkIn.m_Busy)
        {
            /*Set Flag to wait on*/
            g_BulkIn.m_Busy = true;
            g_BulkIn.m_CBDone = _cb;
            err = USBHAL_Bulk_IN(_NumBytes, _Buffer, (CB_DONE)CBDoneBulkIn);
            if(USB_ERR_OK != err)
            {
                g_BulkIn.m_Busy = false;
            }
        }
        else
        {
            err = USB_ERR_BUSY;
        }
    }
    else
    {
        err = USB_ERR_NOT_CONNECTED;
    }

    return err;
}
/***********************************************************************************
* End of function USBCDC_Write_Async
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_Read
* Description   : Perform a blocking read. (BULK OUT)
*                  Note: As this is a blocking function it must not be
*                  called from an interrupt with higher priority than
*                  the USB interrupts.
* Argument      : _BufferSize:     Buffer Size
*                  _Buffer:        Buffer to read data into.
*                  _pNumBytesRead: (OUT)Number of bytes read.
* Return value  : Error Code.
**********************************************************************/
USB_ERR USBCDC_Read(uint32_t _BufferSize, uint8_t* _Buffer, uint32_t* _pNumBytesRead)
{
    USB_ERR err;

    /*This can not complete until connected*/
    if(true == g_bConnected)
    {
        /*Check not already busy*/
        if(false == g_BulkOut.m_Busy)
        {
            /*Set Flag to wait on*/
            g_BulkOut.m_Busy = true;
            g_BulkOut.m_CBDone = NULL; /*No call back this is a blocking function */
            err = USBHAL_Bulk_OUT(_BufferSize, _Buffer, (CB_DONE_OUT)CBDoneBulkOut);
            if(USB_ERR_OK == err)
            {
                /*Wait for flag that will be cleared when the operation has completed*/
                while(true == g_BulkOut.m_Busy){};

                *_pNumBytesRead = g_BulkOut.m_BytesRead;
                err = g_BulkOut.m_Err;
            }
            else
            {
                g_BulkOut.m_Busy = false;
            }
        }
        else
        {
            err = USB_ERR_BUSY;
        }
    }
    else
    {
        err = USB_ERR_NOT_CONNECTED;
    }

    return err;
}
/**********************************************************************************
* End of function USBCDC_Read
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_Read_Async
* Description   : Start an asynchronous read. (BULK OUT)
* Argument      : _BufferSize:     Buffer Size
*                  _Buffer:        Buffer to read data into.
*                  _cb: Callback when done.
* Return value  : Error Code.
**********************************************************************/
USB_ERR USBCDC_Read_Async(uint32_t _BufferSize, uint8_t* _Buffer, CB_DONE_OUT _cb)
{
    USB_ERR err;

    /*This can not complete until connected*/
    if(true == g_bConnected)
    {
        /*Check not already busy*/
        if(false == g_BulkOut.m_Busy)
        {
            /*Set Flag to wait on*/
            g_BulkOut.m_Busy = true;
            g_BulkOut.m_CBDone = _cb;
            err = USBHAL_Bulk_OUT(_BufferSize, _Buffer,
                                 (CB_DONE_OUT)CBDoneBulkOut);

            if(USB_ERR_OK != err)
            {
                g_BulkOut.m_Busy = false;
            }
        }
        else
        {
            err = USB_ERR_BUSY;
        }
    }
    else
    {
        err = USB_ERR_NOT_CONNECTED;
    }

    return err;
}
/**********************************************************************************
* End of function USBCDC_Read_Async
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_Cancel
* Description   : Cancel waiting on any blocking functions.
* Argument      : none
* Return value  : Error code.
**********************************************************************/
USB_ERR USBCDC_Cancel(void)
{
    USB_ERR err;

    ReleaseFlags(USB_ERR_CANCEL);

    /*Reset HAL*/
    err = USBHAL_Reset();

    return err;
}
/**********************************************************************************
* End of function USBCDC_Cancel
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_IsBreakState
* Description   : Read break (SEND_BREAK) state
* Argument      : none
* Return value  : current break state (true if CDC has received SEND_BREAK request)
**********************************************************************/
bool USBCDC_IsBreakState(void)
{
    return g_bBreakState;
}
/***********************************************************************************
* End of function USBCDC_IsBreakState
***********************************************************************************/

/**********************************************************************
* Outline       : USBCDC_Clear_BreakState
* Description   : Clear break (SEND_BREAK) state
* Argument      : none
* Return value  : previous break state
**********************************************************************/
bool USBCDC_Clear_BreakState(void)
{
    bool bPreviousResult = g_bBreakState;
    g_bBreakState = false;
    return bPreviousResult;
}
/***********************************************************************************
* End of function USBCDC_Clear_BreakState
***********************************************************************************/

/**********************************************************************
* Outline       : CBCable
* Description   : Callback when the USB cable is connected or disconnected.
* Argument      : _bConnected: true = Connected, false = Disconnected.
* Return value  : none
**********************************************************************/
static void CBCable(bool _bConnected)
{
    if(true == _bConnected)
    {
        DEBUG_MSG_LOW( ("USBCDC: Cable Connected\r\n"));

        /* Initialise data - as this is like re-starting */
        InitialiseData();

        g_bConnected = true;
    }
    else
    {
        DEBUG_MSG_LOW( ("USBCDC: Cable Disconnected\r\n"));
        g_bConnected = false;

        /*In case we are waiting on any flags set them here*/
        ReleaseFlags(USB_ERR_NOT_CONNECTED);
    }
}
/**********************************************************************************
* End of function CBCable
***********************************************************************************/

/**********************************************************************
* Outline       : CBError
* Description   : Callback saying that an error has occoured in a
*                   lower layer.
* Argument      : _err - error code.
* Return value  : none
**********************************************************************/
static void CBError(USB_ERR _err)
{
    DEBUG_MSG_LOW(("USBCDC: ***CBError***\r\n"));

    /*If the terminal sends data to us when we're not expecting any
    then would get this and it's not an error.
    However, could be that data is arriving too quick for us to read
    and echo out.*/
    if(USB_ERR_BULK_OUT_NO_BUFFER == _err)
    {
        DEBUG_MSG_LOW(("USBCDC: No Bulk Out Buffer.\r\n"));
    }
    else
    {
        /*Try resetting HAL*/
        USBHAL_Reset();

        /*Release flags in case we are waiting on any*/
        ReleaseFlags(_err);
    }
}
/**********************************************************************************
* End of function CBError
***********************************************************************************/

/**********************************************************************
* Outline       : CBUnhandledSetupPacket
* Description   : Called from the USB Core when it can't deal with
*                  a setup packet.
*                  Provides a buffer if there is a data stage.
*                  Expect the core to deal with Standard requests so
*                  this should be Class specific.
*                  This is a function of type CB_SETUP_PACKET.
* Argument      : _pSetupPacket - Setup packet.
*                  _pNumBytes - (OUT)Buffer size.
*                  _ppBuffer - (OUT)Buffer.
* Return value  : Error code
**********************************************************************/
static USB_ERR CBUnhandledSetupPacket(SetupPacket* _pSetupPacket,
                                    uint16_t* _pNumBytes,
                                    uint8_t** _ppBuffer)
{
    USB_ERR err;

    switch(EXTRACT_bmRequest_TYPE(_pSetupPacket->bmRequest))
    {
        case REQUEST_CLASS:
        {
            err = ProcessClassSetupPacket(_pSetupPacket, _pNumBytes, _ppBuffer);
            break;
        }
        case REQUEST_VENDOR:
        case REQUEST_STANDARD:
        default:
        {
            DEBUG_MSG_LOW(("USBCDC: Unsupported Request type\r\n"));
            err = USB_ERR_UNKNOWN_REQUEST;
        }
    }

    return err;
}
/**********************************************************************************
* End of function CBUnhandledSetupPacket
***********************************************************************************/

/**********************************************************************
* Outline       : CBDoneBulkOut
* Description   : Callback when a USBHAL_Bulk_OUT request has completed.
*                  Clear busy flag.
*                  Call registered callback.
* Argument      : _err - Error code.
*                  _NumBytes - The number of bytes received from the host.
* Return value  : none
**********************************************************************/
static void CBDoneBulkOut(USB_ERR err, uint32_t NumBytes)
{
    g_BulkOut.m_BytesRead = NumBytes;
    g_BulkOut.m_Err = err;
    g_BulkOut.m_Busy = false;

    /*If a call back was registered then call it.*/
    if(g_BulkOut.m_CBDone != NULL)
    {
        g_BulkOut.m_CBDone(err, NumBytes);
    }
}
/**********************************************************************************
* End of function CBDoneBulkOut
***********************************************************************************/

/**********************************************************************
* Outline       : CBDoneBulkIn
* Description   : A Bulk IN request has completed.
*                  Clear busy flag.
*                  Call registered callback.
* Argument      : err - error code.
* Return value  : none
**********************************************************************/
static void CBDoneBulkIn(USB_ERR _err)
{
    g_BulkIn.m_Busy = false;

    /*If a call back was registered then call it.*/
    if(g_BulkIn.m_CBDone != NULL)
    {
        g_BulkIn.m_CBDone(_err);
    }
}
/**********************************************************************************
* End of function CBDoneBulkIn
***********************************************************************************/

/**********************************************************************
* Outline       : CBDoneControlOut
* Description   : A Control Out has completed in response to a
*                  setup packet handled in CBUnhandledSetupPacket.
* Argument      : _err - Error code.
*                  _NumBytes - The number of bytes received from the host.
* Return value  : none
**********************************************************************/
static void CBDoneControlOut(USB_ERR _err, uint32_t _NumBytes)
{
    assert(USB_ERR_OK == _err);
    /*Assume this is SET_LINE_CODING data as it is the
    only control out this deals with.*/
    assert(SET_CONTROL_LINE_STATE_DATA_SIZE == _NumBytes);

    /*Construct dwDTERate to avoid endian issues.
    NOTE: Buffer always arrives in little endian*/
    g_SetControlLineData.dwDTERate = ((uint32_t)(g_SET_CONTROL_LINE_STATE_DATA_Buffer[0])
                                        << 0) &  0x000000FF;
    g_SetControlLineData.dwDTERate |= ((uint32_t)(g_SET_CONTROL_LINE_STATE_DATA_Buffer[1])
                                        << 8) &  0x0000FF00;
    g_SetControlLineData.dwDTERate |= ((uint32_t)(g_SET_CONTROL_LINE_STATE_DATA_Buffer[2])
                                        << 16) & 0x00FF00FF;
    g_SetControlLineData.dwDTERate |= ((uint32_t)(g_SET_CONTROL_LINE_STATE_DATA_Buffer[3])
                                        << 24) & 0xFF000000;


    g_SetControlLineData.bCharFormat = g_SET_CONTROL_LINE_STATE_DATA_Buffer[4];
    g_SetControlLineData.bParityType = g_SET_CONTROL_LINE_STATE_DATA_Buffer[5];
    g_SetControlLineData.bDataBits = g_SET_CONTROL_LINE_STATE_DATA_Buffer[6];


    DEBUG_MSG_MID(("USBCDC: SET_LINE_CODING data received.\r\n"));
    DEBUG_MSG_MID(("USBCDC: dwDTERate = %lu\r\n", g_SetControlLineData.dwDTERate));
    DEBUG_MSG_MID(("USBCDC: bCharFormat = %d\r\n", g_SetControlLineData.bCharFormat));
    DEBUG_MSG_MID(("USBCDC: bParityType = %d\r\n", g_SetControlLineData.bParityType));
    DEBUG_MSG_MID(("USBCDC: bDataBits = %d\r\n", g_SetControlLineData.bDataBits));

    /*Don't need to do anything with this data*/
}
/**********************************************************************************
* End of function CBDoneControlOut
***********************************************************************************/

/**********************************************************************
* Outline       : ProcessClassSetupPacket
* Description   : Processes a CDC class setup packet.
*                  Provides a buffer if there is a data stage.
* Argument      : _pSetupPacket - Setup packet.
*                  _pNumBytes - (OUT)Buffer size.
*                  _ppBuffer - (OUT)Buffer.
* Return value  : Error code
**********************************************************************/
static USB_ERR ProcessClassSetupPacket(SetupPacket* _pSetupPacket,
                                        uint16_t* _pNumBytes,
                                        uint8_t** _ppBuffer)
{
    USB_ERR err = USB_ERR_OK;

    switch(_pSetupPacket->bRequest)
    {
        case GET_LINE_CODING:
        {
            DEBUG_MSG_LOW(("USBCDC: GET_LINE_CODING\r\n"));

            /*Data IN response */
            *_pNumBytes = LINE_CODING_DATA_SIZE;
            *_ppBuffer = (uint8_t*)g_LineCoding;
            break;
        }
        case SET_LINE_CODING: /*(Required for hyperterminal)*/
        {
            DEBUG_MSG_LOW(("USBCDC: SET_LINE_CODING\r\n"));
            /*No action required for this request.*/
            /*Data OUT*/
            *_pNumBytes = SET_CONTROL_LINE_STATE_DATA_SIZE;
            *_ppBuffer = g_SET_CONTROL_LINE_STATE_DATA_Buffer;
            break;
        }
        case SET_CONTROL_LINE_STATE:
        {
            DEBUG_MSG_LOW(("USBCDC: SET_CONTROL_LINE_STATE\r\n"));
            /*No action required for this request.*/
            /*No data response */
            *_pNumBytes = 0;
            break;
        }
        case SEND_BREAK:
        {
            DEBUG_MSG_LOW(("USBCDC: SEND_BREAK\r\n"));
            g_bBreakState = true;
            /*No data response */
            *_pNumBytes = 0;
            break;
        }
        default:
        {
            DEBUG_MSG_LOW(("USBCDC: Unsupported Class request %d\r\n",
                         _pSetupPacket->bRequest));
            err = USB_ERR_UNKNOWN_REQUEST;
        }
    }

    return err;
}
/**********************************************************************************
* End of function ProcessClassSetupPacket
***********************************************************************************/

/**********************************************************************
* Outline       : ReleaseFlags
* Description   : Set Error codes and release and clear busy flags.
*                  To be used when an error has been detected and need
*                  to ensure this module isn't waiting on a callback
*                  that will not come.
* Argument      : err - Error Code
* Return value  : none
**********************************************************************/
static void ReleaseFlags(USB_ERR err)
{
    g_BulkIn.m_Err = err;
    g_BulkOut.m_Err = err;
    g_BulkIn.m_Busy = false;
    g_BulkOut.m_Busy = false;
}
/**********************************************************************************
* End of function ReleaseFlags
***********************************************************************************/

/**********************************************************************
* Outline       : InitialiseData
* Description   : Initilise this modules data.
*                  Put into a function so that it can be done each time
*                  the USB cable is connected.
* Argument      : none
* Return value  : none
**********************************************************************/
static void InitialiseData(void)
{
    /*Bulk Out*/
    g_BulkOut.m_Busy = false;        /*Busy Flag*/
    g_BulkOut.m_BytesRead = 0;     /*Number of bytes read*/
    g_BulkOut.m_Err = USB_ERR_OK;    /*Error Status*/
    g_BulkOut.m_CBDone = NULL;    /*Callback done*/

    /*Bulk In*/
    g_BulkIn.m_Busy = false;        /*Busy Flag*/
    g_BulkIn.m_Err = USB_ERR_OK;    /*Error Status*/
    g_BulkIn.m_CBDone = NULL;    /*Callback done*/
}
/**********************************************************************************
* End of function InitialiseData
***********************************************************************************/
