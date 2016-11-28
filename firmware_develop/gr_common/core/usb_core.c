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
* File Name     : usb_core.c
* Version       : 1.00
* Device        : R5F563NB
* Tool-Chain    : Renesas RX Standard 1.2.0.0
* H/W Platform  : RSK+RX63N
* Description   : The USB core sits above the HAL in the USB stack.
                Initilaises the HAL.
                Handles standard USB setup requests.

                NOTE: This only supports 1 language ID and that
                      is currently English.
*******************************************************************************/
/*******************************************************************************
* History       : 13 Aug. 2012  Ver. 1.00 First Release
*******************************************************************************/

/*******************************************************************************
* System Includes
*******************************************************************************/
/* Following header file defines the C preprocessor macro assert(). */
#include <assert.h>

/*******************************************************************************
* User Includes (Project Level Includes)
*******************************************************************************/
/* Following header file defines common USB definitions. */
#include "usb_common.h"
/* Following header file defines USB HAL definitions. */
#include "usb_hal.h"
/* Following header file defines USB core definitions. */
#include "usb_core.h"

#if 0
/* Declaration of abort function. */
void abort(void)
{
    /* Do nothing */
}
#endif


/*******************************************************************************
* Type Definitions
*******************************************************************************/
/*Descriptor Buffer */
typedef struct Descriptor
{
    const uint8_t* pucBuf;
    uint16_t NumBytes;
}Descriptor;

/*Structure that holds Descriptors*/
typedef struct Descriptors
{
    Descriptor Device;
    Descriptor Config;

    Descriptor StringManufacturer;
    Descriptor StringProduct;
    Descriptor StringSerial;

    Descriptor StringLanguageIDs;
}Descriptors;

/*******************************************************************************
* Defines
*******************************************************************************/
/*ENDPOINT_HALT value*/
#define FEATURE_ENDPOINT_HALT 0
/*Value of Recipient bits in bmRequest:*/
#define bmREQUEST_RECIPIANT_DEVICE      0
#define bmREQUEST_RECIPIANT_INTERFACE   1
#define bmREQUEST_RECIPIANT_ENDPOINT    2

/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/
static void CBSetup(const uint8_t(*_pSetupPacket)[USB_SETUP_PACKET_SIZE]);

static void PopulateSetupPacket(const uint8_t(*_pSetupPacket)[USB_SETUP_PACKET_SIZE]);
static USB_ERR ProcessSetupPacket(uint16_t* _pNumBytes, uint8_t** _ppBuffer);
static USB_ERR ProcessStandardSetupPacket(uint16_t* _pNumBytes, uint8_t** _ppBuffer);
static USB_ERR ProcessGetDescriptor(uint16_t* _pNumBytes, const uint8_t** _ppDescriptor);
static USB_ERR ProcessGetDescriptorString(uint16_t* _pNumBytes, const uint8_t** _ppDescriptor);
static USB_ERR ProcessSetConfiguration(uint16_t* _pNumBytes, const uint8_t**_ppBuffer);
static USB_ERR ProcessGetConfiguration(uint16_t* _pNumBytes, const uint8_t**_ppBuffer);
static USB_ERR ProcessSetFeature(uint16_t* _pNumBytes, const uint8_t**_ppBuffer);
static USB_ERR ProcessClearFeature(uint16_t* _pNumBytes, const uint8_t**_ppBuffer);
static USB_ERR ProcessGetStatus(uint16_t* _pNumBytes, const uint8_t**_ppBuffer);

/*******************************************************************************
* Variables
*******************************************************************************/
/*Language ID - Currently only supports English*/
#define STRING_LANGUAGE_IDS_SIZE 0x04
/*#define STRING_LANGUAGE_ID 0x0809 (English UK Not liked by USBView)*/
#define STRING_LANGUAGE_ID 0x0409 /*English USA*/
const uint8_t gStringDescriptorLanguageIDsData[STRING_LANGUAGE_IDS_SIZE] =
{
    STRING_LANGUAGE_IDS_SIZE,   /*Length of this*/
    0x03,                       /*Descriptor Type = STRING*/
    (STRING_LANGUAGE_ID & 0xFF),
    ((STRING_LANGUAGE_ID >> 8) & 0xFF),
};

/*Registered Descriptors */
static Descriptors g_Descriptors =
{
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {gStringDescriptorLanguageIDsData, STRING_LANGUAGE_IDS_SIZE}
};

/*Setup Packet */
static SetupPacket g_oSetupPacket;

/*Registered CallBacks*/
static CB_SETUP_PACKET g_fpCBSetupPacket = NULL;
static CB_DONE_OUT g_fpCBControlOut = NULL;

/*Configuration Value */
static uint8_t g_ConfigurationValue;

/*******************************************************************************
* Outline       : USBCORE_Init
* Description   : Initialise the USB core.
*                 Descriptors are supplied so that this can handle standard
*                 GET_DESCRIPTOR requests.
*                 Initialises the HAL.
*
* Argument      : _Manufacturer:  Manufacturer String Descriptor
*                 _ManufacturerSize: Manufacturer String Descriptor Size
*                 _Product:  Product String Descriptor
*                 _ProductSize: Product String Descriptor Size
*                 _Serial:  Serial String Descriptor
*                 _SerialSize: Serial String Descriptor Size
*                 _DeviceDescriptor: Device descriptor
*                 _ConfigDescriptor: Configuration Descriptor
*                 _fpCBSetupPacket: Function that will be called if
*                     a setup packet can't be handled by this layer.
*                     (Usually a Vender or Class specific GetDescriptor request)
*                 _fpCBControlOut: Function that will be called when
*                             a control data out occours for a setup packet
*                             not handled by this layer.
*                 _fpCBCable: Function that will be called when USB cable is
*                             connected/disconnected
*                 _fpCBError: Function that will be called when an unhandles
*                             error occours.
* Return value  : Error code.
*******************************************************************************/
USB_ERR USBCORE_Init(const uint8_t* _Manufacturer, uint16_t _ManufacturerSize,
                     const uint8_t* _Product, uint16_t _ProductSize,
                     const uint8_t* _Serial, uint16_t _SerialSize,
                     const uint8_t* _DeviceDescriptor, uint16_t _DeviceDescriptorSize,
                     const uint8_t* _ConfigDescriptor, uint16_t _ConfigDescriptorSize,
                     CB_SETUP_PACKET _fpCBSetupPacket,
                     CB_DONE_OUT _fpCBControlOut,
                     CB_CABLE _fpCBCable,
                     CB_ERROR _fpCBError)
{
    USB_ERR err = USB_ERR_OK;

    /*Store passed in descriptors*/
    g_Descriptors.Device.pucBuf = _DeviceDescriptor;
    g_Descriptors.Device.NumBytes = _DeviceDescriptorSize;

    g_Descriptors.Config.pucBuf = _ConfigDescriptor;
    g_Descriptors.Config.NumBytes = _ConfigDescriptorSize;

    g_Descriptors.StringManufacturer.pucBuf = _Manufacturer;
    g_Descriptors.StringManufacturer.NumBytes = _ManufacturerSize;

    g_Descriptors.StringProduct.pucBuf = _Product;
    g_Descriptors.StringProduct.NumBytes = _ProductSize;

    g_Descriptors.StringSerial.pucBuf = _Serial;
    g_Descriptors.StringSerial.NumBytes = _SerialSize;

    /*Store passed in callback*/
    g_fpCBSetupPacket = _fpCBSetupPacket;
    g_fpCBControlOut = _fpCBControlOut;

    /*Initialise variables*/
    g_ConfigurationValue = 0;
    /*Initialise USB HAL*/
    err = USBHAL_Init(CBSetup, _fpCBCable, _fpCBError);

    return err;
}
/******************************************************************************
* End of function USBCORE_Init
******************************************************************************/

/******************************************************************************
* Outline       : CBSetup
* Description   : Callback from HAL when it has received a setup packet.
                  Responds with either an CONTROL IN, OUT, ACK or STALL.
                  If this layer can not deal with the setup packet
                  on its own it will call an upper layer to see if it
                  knows how to deal with it.
* Argument      : _pSetupPacket - buffer containing the setup packet.
* Return value  : none
******************************************************************************/
static void CBSetup(const uint8_t(*_pSetupPacket)[USB_SETUP_PACKET_SIZE])
{
    USB_ERR err = USB_ERR_OK;
    uint16_t NumBytes;
    uint8_t* pBuffer;

    DEBUG_MSG_HIGH( ("USBCORE: SetupPacket received\r\n"));

    /*Populate the Setup Packet structure g_oSetupPacket */
    PopulateSetupPacket(_pSetupPacket);

    /*Process this setup packet*/
    err = ProcessSetupPacket(&NumBytes, &pBuffer);

    if(USB_ERR_UNKNOWN_REQUEST == err)
    {
        /*Can't handle this setup packet*/
        /*Let upper layer try - call registered callback*/
        err = g_fpCBSetupPacket(&g_oSetupPacket, &NumBytes, &pBuffer);
    }

    if(USB_ERR_OK == err)
    {
        /*Is there a data stage?*/
        if(0 != g_oSetupPacket.wLength)
        {
            /* Is this a Data IN or OUT */
            if(0 != EXTRACT_bmRequest_DIRECTION(g_oSetupPacket.bmRequest))
            {
                DEBUG_MSG_MID(("USBCORE: SetupPacket DATA IN\r\n"));

                /*IN*/
                /*Don't send more data than host has requested*/
                if(NumBytes > g_oSetupPacket.wLength)
                {
                    NumBytes = g_oSetupPacket.wLength;
                }

                /*Send Data*/
                USBHAL_Control_IN(NumBytes, pBuffer);
            }
            else
            {
                /*OUT*/
                DEBUG_MSG_MID(("USBCORE: SetupPacket DATA OUT\r\n"));

                assert(g_oSetupPacket.wLength == NumBytes);

                USBHAL_Control_OUT(NumBytes, pBuffer, g_fpCBControlOut);
            }
        }
        else
        {
            /*No data stage - just need to send ACK*/
            DEBUG_MSG_MID(("USBCORE: SetupPacket - No data stage.\r\n"));

            USBHAL_Control_Status();
        }
    }
    else
    {
        DEBUG_MSG_MID(("USBCORE: SetupPacket - stall.\r\n"));

        /*Something wrong with this control pipe so stall it.*/
        USBHAL_Control_Stall();
    }
}
/******************************************************************************
* End of function CBSetup
******************************************************************************/

/******************************************************************************
* Outline       : ProcessSetupPacket
* Description   : Looks to see if this is a standard setup packet
*                 that this layer can possibly deal with.
*
* Argument      : _pNumBytes: (OUT)If this layer can handle this then
*                           this will be set with the size of the data.
*                           (If there is no data stage then this will be set to zero)
*                 _ppBuffer: (OUT)If this layer can handle this then
*                           this will be set to point to the data(IN) or a buffer(OUT).
*                           (If there is a data stage for this packet).
* Return value  : none
******************************************************************************/
static USB_ERR ProcessSetupPacket(uint16_t* _pNumBytes, uint8_t** _ppBuffer)
{
    USB_ERR err = USB_ERR_OK;

    switch(EXTRACT_bmRequest_TYPE(g_oSetupPacket.bmRequest))
    {
        case REQUEST_STANDARD:
        {
            /* Standard Type */
            err = ProcessStandardSetupPacket(_pNumBytes, _ppBuffer);
            break;
        }
        case REQUEST_CLASS:
        case REQUEST_VENDOR:
        default:
        {
            /*Unsupported request*/
            err = USB_ERR_UNKNOWN_REQUEST;
        }
    }

    return err;
}
/******************************************************************************
* End of function ProcessSetupPacket
******************************************************************************/

/******************************************************************************
* Outline       : ProcessStandardSetupPacket
* Description   : Checks to see if this standard setup packet can be
*                 handled by this layer.
*                 Handles GET_DESCRIPTOR.
* Argument      : _pNumBytes: (OUT)If this layer can handle this then
*                           this will be set with the size of the data.
*                           (If there is no data stage then this will be set to zero)
*                 _ppBuffer: (OUT)If this layer can handle this then
*                           this will be set to point to the data(IN) or a buffer(OUT).
*                           (If there is a data stage for this packet).
* Return value  : Error code. - USB_ERR_OK if supported.
**********************************************************************/

static USB_ERR ProcessStandardSetupPacket(uint16_t* _pNumBytes, uint8_t** _ppBuffer)
{
    USB_ERR err = USB_ERR_OK;

    /*NOTE: SET_ADDRESS is supported by the HW.*/

    switch(g_oSetupPacket.bRequest)
    {
        case GET_DESCRIPTOR:
        {
            err = ProcessGetDescriptor(_pNumBytes, (const uint8_t**)_ppBuffer);
            break;
        }
        case GET_CONFIGURATION:
        {
            err = ProcessGetConfiguration(_pNumBytes, (const uint8_t**)_ppBuffer);
            break;
        }
        case SET_CONFIGURATION:
        {
            err = ProcessSetConfiguration(_pNumBytes, (const uint8_t**)_ppBuffer);
            break;
        }
        case SET_FEATURE:
        {
            err = ProcessSetFeature(_pNumBytes, (const uint8_t**)_ppBuffer);
            break;
        }
        case CLEAR_FEATURE:
        {
            err = ProcessClearFeature(_pNumBytes, (const uint8_t**)_ppBuffer);
            break;
        }
        case GET_STATUS:
        {
            err = ProcessGetStatus(_pNumBytes, (const uint8_t**)_ppBuffer);
            break;
        }
        default:
        {
            /*Unsupported request*/
            err = USB_ERR_UNKNOWN_REQUEST;
        }
    }

    return err;
}
/******************************************************************************
End of function ProcessStandardSetupPacket
******************************************************************************/

/******************************************************************************
* Outline       : ProcessSetFeature
* Description   : Process a Set Feature USB request,
* Argument      : _pNumBytes: (OUT)If this layer can handle this then
*                           this will be set with the size of the data.
*                           (If there is no data stage then this will be
*                           set to zero)
*                 _ppBuffer: (OUT)If this layer can handle this then
*                           this will be set to point to the data(IN)
*                           or a buffer(OUT).
*                           (If there is a data stage for this packet).
* Return value  : Error code. - USB_ERR_OK if supported.
******************************************************************************/
static USB_ERR ProcessSetFeature(uint16_t* _pNumBytes, const uint8_t**_ppBuffer)
{
    USB_ERR err = USB_ERR_INVALID_REQUEST;

    DEBUG_MSG_MID(("USBCORE: SetFeature \r\n"));

    /*There is no data stage for this request*/
    *_pNumBytes = 0;
    *_ppBuffer = NULL;

    /*Support feature ENDPOINT_HALT
    This feature is only valid for an endpoint*/
    if(bmREQUEST_RECIPIANT_ENDPOINT ==
         EXTRACT_bmRequest_RECIPIENT(g_oSetupPacket.bmRequest))
    {
        /*Recipient is endpoint (wIndex is endpoint number)*/
        /*Is this a ENDPOINT_HALT feature? (wValue is Feature)*/
        if(FEATURE_ENDPOINT_HALT == g_oSetupPacket.wValue)
        {
            /*Stall the specified endpoint if BULK IN, BULK OUT or INTERRUPT IN*/
            switch(0x0F & g_oSetupPacket.wIndex)
            {
                case EP_BULK_OUT:
                    USBHAL_Bulk_OUT_Stall();
                    err = USB_ERR_OK;
                    break;
                case EP_BULK_IN:
                    USBHAL_Bulk_IN_Stall();
                    err = USB_ERR_OK;
                    break;
                case EP_INTERRUPT_IN:
                    USBHAL_Interrupt_IN_Stall();
                    err = USB_ERR_OK;
                    break;
                default:
                    DEBUG_MSG_MID(("USBCORE: Invalid endpoint\r\n"));
                    break;
            }
        }
    }

    return err;
}
/******************************************************************************
* End of function ProcessSetFeature
******************************************************************************/

/******************************************************************************
* Outline       : ProcessClearFeature
* Description   : Process a Clear Feature USB request,
* Argument      : _pNumBytes: (OUT)If this layer can handle this then
*                           this will be set with the size of the data.
*                           (If there is no data stage then this will be
*                           set to zero)
*                 _ppBuffer: (OUT)If this layer can handle this then
*                           this will be set to point to the data(IN)
*                           or a buffer(OUT).
*                           (If there is a data stage for this packet).
* Return value  : Error code. - USB_ERR_OK if supported.
******************************************************************************/
static USB_ERR ProcessClearFeature(uint16_t* _pNumBytes, const uint8_t**_ppBuffer)
{
    USB_ERR err = USB_ERR_INVALID_REQUEST;

    DEBUG_MSG_MID(("USBCORE: ClearFeature \r\n"));

    /*There is no data stage for this request*/
    *_pNumBytes = 0;
    *_ppBuffer = NULL;

    /*Support feature ENDPOINT_HALT
    This feature is only valid for an endpoint recipient*/
    if(bmREQUEST_RECIPIANT_ENDPOINT ==
         EXTRACT_bmRequest_RECIPIENT(g_oSetupPacket.bmRequest))
    {
        /*Recipient is endpoint (wIndex is endpoint number)*/
        /*Is this a ENDPOINT_HALT feature? (wValue is Feature)*/
        if(FEATURE_ENDPOINT_HALT == g_oSetupPacket.wValue)
        {
            /*Stall the specified endpoint if BULK IN, BULK OUT or INTERRUPT IN*/
            switch(0x0F & g_oSetupPacket.wIndex)
            {
                case EP_BULK_OUT:
                    /*Only clear a BULK OUT if flag "m_bAllowStallClear" is set.*/
                    if(true == (USBHAL_Config_Get()->m_bAllowStallClear))
                    {
                        USBHAL_Bulk_OUT_Stall_Clear();
                    }
                    else
                    {
                        DEBUG_MSG_MID(("USBCORE: Not clearing BULK OUT stall as m_bAllowStallClear flag not set.\r\n"));
                    }
                    err = USB_ERR_OK;
                    break;
                case EP_BULK_IN:
                    /*Only clear a BULK IN if flag "m_bAllowStallClear" is set.*/
                    if(true == (USBHAL_Config_Get()->m_bAllowStallClear))
                    {
                        USBHAL_Bulk_IN_Stall_Clear();
                    }
                    else
                    {
                        DEBUG_MSG_MID(("USBCORE: Not clearing BULK IN stall as m_bAllowStallClear flag not set.\r\n"));
                    }
                    err = USB_ERR_OK;
                    break;
                case EP_INTERRUPT_IN:
                    USBHAL_Interrupt_IN_Stall_Clear();
                    err = USB_ERR_OK;
                    break;
                default:
                    DEBUG_MSG_MID(("USBCORE: Invalid endpoint\r\n"));
                    break;
            }
        }
    }

    return err;
}
/******************************************************************************
End of function ProcessClearFeature
******************************************************************************/

/******************************************************************************
* Outline       : ProcessSetConfiguration
* Description   : Process a Set Configuration USB request,
* Argument      : _pNumBytes: (OUT)If this layer can handle this then
*                           this will be set with the size of the data.
*                           (If there is no data stage then this will be set to zero)
*                 _ppBuffer: (OUT)If this layer can handle this then
*                           this will be set to point to the data(IN) or a buffer(OUT).
*                           (If there is a data stage for this packet).
* Return value  : Error code. - USB_ERR_OK if supported.
******************************************************************************/
static USB_ERR ProcessSetConfiguration(uint16_t* _pNumBytes, const uint8_t**_ppBuffer)
{
    USB_ERR err = USB_ERR_OK;

    /*NOTE: Currently only 1 configuration is supported.*/

    /*The lower byte of the 'value' field contains the configuration.*/
    g_ConfigurationValue = (uint8_t)g_oSetupPacket.wValue;
    DEBUG_MSG_MID(("USBCORE: SetConfiguration: %d \r\n", g_ConfigurationValue));

    /*There is no data stage for this request*/
    *_pNumBytes = 0;
    *_ppBuffer = NULL;

    /*Reset the data toggles on the endpoints*/
    USBHAL_ResetEndpoints();

    return err;
}
/******************************************************************************
End of function ProcessSetConfiguration
******************************************************************************/

/******************************************************************************
* Outline       : ProcessGetConfiguration
* Description   : Process a Get Configuration USB request,
* Argument      : _pNumBytes: (OUT)If this layer can handle this then
*                           this will be set with the size of the data.
*                           (If there is no data stage then this will be set to zero)
*                 _ppBuffer: (OUT)If this layer can handle this then
*                           this will be set to point to the data(IN) or a buffer(OUT).
*                           (If there is a data stage for this packet).
* Return value  : Error code. - USB_ERR_OK if supported.
******************************************************************************/
static USB_ERR ProcessGetConfiguration(uint16_t* _pNumBytes, const uint8_t**_ppBuffer)
{
    USB_ERR err = USB_ERR_OK;

    /*This assumes the device is in the configured state.*/

    DEBUG_MSG_MID(("USBCORE: GetConfiguration: %d \r\n", g_ConfigurationValue));

    /*There is a 1 byte data stage for this request*/
    *_pNumBytes = 1;
    *_ppBuffer = &g_ConfigurationValue;

    return err;
}
/******************************************************************************
End of function ProcessGetConfiguration
******************************************************************************/

/******************************************************************************
* Outline       : ProcessGetStatus
* Description   : Process a Get Status USB request,
* Argument      : _pNumBytes: (OUT)If this layer can handle this then
*                           this will be set with the size of the data.
*                           (If there is no data stage then this will be set to zero)
*                 _ppBuffer: (OUT)If this layer can handle this then
*                           this will be set to point to the data(IN) or a buffer(OUT).
*                           (If there is a data stage for this packet).
* Return value  : Error code. - USB_ERR_OK if supported.
******************************************************************************/
static USB_ERR ProcessGetStatus(uint16_t* _pNumBytes, const uint8_t**_ppBuffer)
{
    USB_ERR err = USB_ERR_OK;

    /*Status Data Response
    16 bit but use 2 8-bit to avoid endian issues:
    StatusData[0] = bits 0->7, StatusData[1] = bits 8->15*/

    static uint8_t StatusData[2];

    DEBUG_MSG_MID(("USBCORE: GetStatus\r\n"));

    /*The response depends upon the recipient*/
    switch(EXTRACT_bmRequest_RECIPIENT(g_oSetupPacket.bmRequest))
    {
        case bmREQUEST_RECIPIANT_DEVICE:
        {
            DEBUG_MSG_HIGH(("USBCORE: GetStatus DEVICE\r\n"));

            /*Status Data: Bit 0 = Self Powered, Bit 1 = Remote Wakeup.*/
            StatusData[0] = 0;
            StatusData[1] = 0;
            break;
        }
        case bmREQUEST_RECIPIANT_INTERFACE:
        {
            DEBUG_MSG_HIGH(("USBCORE: GetStatus INTERFACE\r\n"));

            /*Status Data: All bits 0*/
            StatusData[0] = 0;
            StatusData[1] = 0;
            break;
        }
        case bmREQUEST_RECIPIANT_ENDPOINT:
        {
            DEBUG_MSG_HIGH(("USBCORE: GetStatus ENDPOINT\r\n"));

            /*Status Data: Bit 0 = HALT*/
            /*Default to not halted*/
            StatusData[0] = 0;
            StatusData[1] = 0;

            /*Which endpoint?*/
            switch(0x0F & g_oSetupPacket.wIndex)
            {
                case EP_BULK_OUT:
                    if(true == USBHAL_Bulk_OUT_Is_Stalled())
                    {
                        /*Set HALT bit (BIT 0)*/
                        StatusData[0] |= 0x01;
                    }
                    break;
                case EP_BULK_IN:
                    if(true == USBHAL_Bulk_IN_Is_Stalled())
                    {
                            /*Set HALT bit (BIT 0)*/
                            StatusData[0] |= 0x01;
                    }
                    break;
                case EP_INTERRUPT_IN:
                    if(true == USBHAL_Interrupt_IN_Is_Stalled())
                    {
                        /*Set HALT bit (BIT 0)*/
                        StatusData[0] |= 0x01;
                    }
                    break;
                default:
                    DEBUG_MSG_MID(("USBCORE: Invalid endpoint\r\n"));
                    err = USB_ERR_INVALID_REQUEST;
            }
            break;
        }
        default:
            err = USB_ERR_INVALID_REQUEST;
    }

    /*There is a 2 byte data stage for this request*/
    *_pNumBytes = 2;
    *_ppBuffer = (uint8_t*)StatusData;

    return err;
}
/******************************************************************************
End of function ProcessSetConfiguration
******************************************************************************/

/******************************************************************************
* Outline       : ProcessGetDescriptor
* Description   : Process a GET_DESCRIPTOR setup packet.
*                 Supports Device, Configuration and String requests.
*
* Argument      : _pNumBytes: (OUT)If this layer can handle this then
*                           this will be set with the size of the descriptor.
*                 _ppDescriptor: (OUT)If this layer can handle this then
*                           this will be set to point to the descriptor.
*
* Return value  : Error code - USB_ERR_OK if supported.
******************************************************************************/
static USB_ERR ProcessGetDescriptor(uint16_t* _pNumBytes, const uint8_t** _ppDescriptor)
{
    USB_ERR err = USB_ERR_OK;

    /*The wValue field of the setup packet,
    specifies the descriptor type in the high byte
    and the descriptor index in the low byte.*/

    /*Get Descriptor Type*/
    uint8_t DescriptorType = (uint8_t)((g_oSetupPacket.wValue >> 8) & 0x00FF);
    switch(DescriptorType)
    {
        case DEVICE:
        {
            DEBUG_MSG_MID(("USBCORE: Device Descriptor\r\n"));

            *_pNumBytes = g_Descriptors.Device.NumBytes;
            *_ppDescriptor = g_Descriptors.Device.pucBuf;
            break;
        }
        case CONFIGURATION:
        {
            DEBUG_MSG_MID(("USBCORE: Config Descriptor\r\n"));

            *_pNumBytes = g_Descriptors.Config.NumBytes;
            *_ppDescriptor = g_Descriptors.Config.pucBuf;
            break;
        }
        case STRING:
        {
            DEBUG_MSG_MID(("USBCORE: String Descriptor\r\n"));
            err = ProcessGetDescriptorString(_pNumBytes, _ppDescriptor);
            break;
        }
        case INTERFACE:
            /*Interface descriptor is avalable through Configuration descriptor only.*/
        case ENDPOINT:
            /*Endpoint descriptor is avalable through Configuration descriptor only.*/
        case DEVICE_QUALIFIER:
            /*Note1: May get a DEVICE_QUALIFIER request - but OK not to handle
            it as we don't support different configurations for high and full speed*/
        default:
        {
            /*Unknown descriptor request*/

            /*Note2: HID uses this standard GET_DESCRIPTOR to get
            class specific descriptors.*/
            DEBUG_MSG_MID(("USBCORE: Unhandled GetDescriptor Type %d\r\n", DescriptorType));

            *_pNumBytes = 0;
            *_ppDescriptor = NULL;
            err = USB_ERR_UNKNOWN_REQUEST;
        }
    }

    return err;
}
/******************************************************************************
End of function ProcessGetDescriptor
******************************************************************************/

/******************************************************************************
* Outline       : ProcessGetDescriptorString()
* Description   : Process a GET_DESCRIPTOR type STRING.
*                 Handles LanguageID, Manufacturer, Product
*                 and Serial Number requests.
*                 Note: Only supports 1 language ID.
*
* Argument      : _pNumBytes: (OUT)If this layer can handle this then
*                           this will be set with the size of the descriptor.
*                 _ppDescriptor: (OUT)If this layer can handle this then
*                           this will be set to point to the descriptor.
*
* Return value  : Error Code - USB_ERR_OK if supported.
******************************************************************************/
static USB_ERR ProcessGetDescriptorString(uint16_t* _pNumBytes,
                                         const uint8_t** _ppDescriptor)
{
    USB_ERR err = USB_ERR_OK;

    /*The wIndex field of the setup packet is the language ID.
    If wIndex is 0 then this is a request to get the language ID(s) supported*/
    switch(g_oSetupPacket.wIndex)
    {
        case 0: /*Get Language ID supported*/
        {
            DEBUG_MSG_MID(("Language ID\r\n"));

            *_pNumBytes = g_Descriptors.StringLanguageIDs.NumBytes;
            *_ppDescriptor = g_Descriptors.StringLanguageIDs.pucBuf;

            break;
        }
        case STRING_LANGUAGE_ID: /*Get 1st language ID strings*/
        {
            /* What string index is being requested */
            /*The wValue field specifies descriptor index in the low byte.*/
            switch((g_oSetupPacket.wValue) & 0x00FF)
            {
                case STRING_iMANUFACTURER:
                {
                    DEBUG_MSG_MID(("String Manufacturer\r\n"));

                    *_pNumBytes = g_Descriptors.StringManufacturer.NumBytes;
                    *_ppDescriptor = g_Descriptors.StringManufacturer.pucBuf;
                    break;
                }
                case STRING_iPRODUCT:
                {
                    DEBUG_MSG_MID(("String Product\r\n"));

                    *_pNumBytes = g_Descriptors.StringProduct.NumBytes;
                    *_ppDescriptor = g_Descriptors.StringProduct.pucBuf;
                    break;
                }
                case STRING_iSERIAL:
                {
                    DEBUG_MSG_MID(("String Serial Number\r\n"));

                    *_pNumBytes = g_Descriptors.StringSerial.NumBytes;
                    *_ppDescriptor = g_Descriptors.StringSerial.pucBuf;
                    break;
                }
                default:
                {
                    /*Unknown descriptor request*/
                    DEBUG_MSG_MID(("Error:- Unknown String\r\n"));

                    *_pNumBytes = 0;
                    *_ppDescriptor = NULL;
                    err = USB_ERR_UNKNOWN_REQUEST;
                }
            }
            break;
        }
        default:
        {
            /*Only support 1 language ID*/
            /*Unknown descriptor request*/
            DEBUG_MSG_MID(("Error:- Unknown Language ID\r\n"));

            *_pNumBytes = 0;
            *_ppDescriptor = NULL;
            err = USB_ERR_UNKNOWN_REQUEST;
        }
    }

    return err;
}
/******************************************************************************
End of function ProcessGetDescriptorString
******************************************************************************/

/******************************************************************************
* Outline       : PopulateSetupPacket
* Description   : Read the 8 BYTE setup packet into the g_oSetupPacket
*                 structure so that it is easier to understand.
* Argument      : _pSetupPacket - Setup packet in little endian.
* Return value  : none
******************************************************************************/
static void PopulateSetupPacket(const uint8_t(*_pSetupPacket)[USB_SETUP_PACKET_SIZE])
{
    g_oSetupPacket.bmRequest = (uint8_t)(*_pSetupPacket)[0];
    g_oSetupPacket.bRequest = (uint8_t)(*_pSetupPacket)[1];

    g_oSetupPacket.wValue = (uint16_t)((uint16_t)(*_pSetupPacket)[2] & 0x00FF);
    g_oSetupPacket.wValue |= (((uint16_t)(*_pSetupPacket)[3]) << 8) & 0xFF00;

    g_oSetupPacket.wIndex = (uint16_t)((uint16_t)(*_pSetupPacket)[4] & 0x00FF);
    g_oSetupPacket.wIndex |= (((uint16_t)(*_pSetupPacket)[5]) << 8) & 0xFF00;

    g_oSetupPacket.wLength = (uint16_t)((uint16_t)(*_pSetupPacket)[6] & 0x00FF);
    g_oSetupPacket.wLength |= (((uint16_t)(*_pSetupPacket)[7]) << 8) & 0xFF00;
}
/******************************************************************************
End of function PopulateSetupPacket
******************************************************************************/
