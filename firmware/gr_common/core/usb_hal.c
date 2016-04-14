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
* File Name     : USB_HAL.c
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
*******************************************************************************/
/*******************************************************************************
* History       : 13 Aug. 2012  Ver. 1.00 First Release
*******************************************************************************/

/*******************************************************************************
System Includes
*******************************************************************************/
/* Following header file defines the C preprocessor macro assert(). */
#include <assert.h>

/*******************************************************************************
User Includes (Project Level Includes)
*******************************************************************************/
/* Defines RX63N port registers */
#include "rx63n/iodefine.h"
/* Following header file defines common USB definitions. */
#include "usb_common.h"
/* Following header file defines USB HAL definitions. */
#include "usb_hal.h"

#include "rx63n/util.h"

/**********************************************************************
System Definitions & Global Variables
**********************************************************************/

/*PID Values*/
#define PID_NAK     0
#define PID_BUF     1
#define PID_STALL_1 2
#define PID_STALL_2 3

/***********************************************************************
Type Definitions
***********************************************************************/
/*Call Backs Registered by USB Core*/
typedef struct CBs
{
    CB_SETUP fpSetup;
    CB_CABLE fpCable;
    CB_ERROR fpError;
}CBs;

/*General purpose buffer*/
typedef struct DataBuff
{
    uint8_t* pucBuf;
    uint32_t NumBytes;
}DataBuff;

/*State of control pipe*/
typedef enum StateControl
{
    STATE_READY,
    STATE_DISCONNECTED,
    STATE_CONTROL_SETUP,
    STATE_CONTROL_IN,
    STATE_CONTROL_OUT
}StateControl;

/*Data structure used for OUT*/
typedef struct OUT
{
    uint32_t m_BuffSize;
    DataBuff m_DataBuff;
    CB_DONE_OUT m_fpDone;
}OUT;

/*Data structure used for IN*/
typedef struct IN
{
    DataBuff m_DataBuff;
    CB_DONE m_fpDone;
}IN;

/*Data structure used for Control*/
typedef struct CONTROL
{
    StateControl m_etState;
    IN m_IN;
    OUT m_OUT;
}CONTROL;

/*Data structure used for Bulk*/
typedef struct BULK
{
    /*Busy Flags*/
    bool m_INBusy;
    bool m_OUTBusy;
    IN m_IN;
    OUT m_OUT;
}BULK;

/*Data structure used for Interrupt*/
typedef struct INTERRUPT
{
    bool m_INBusy; /*Busy Flag*/
    IN m_IN;
}INTERRUPT;

/*******************************************************************************
Variables
*******************************************************************************/
/*Call Backs Registered by USB Core*/
static volatile CBs g_CBs =
{
    NULL,
    NULL,
    NULL,
};

/*Control Pipe Data*/
static volatile CONTROL g_Control =
{
    /*StateControl*/
    STATE_DISCONNECTED,
    /*IN*/
    {
        {NULL, 0},  /*DataBuff*/
        NULL        /*CB (NOT USED)*/
    }
    ,
    /*OUT*/
    {
        0,          /*DataBuffSize*/
        {NULL, 0},  /*DataBuff*/
        NULL        /*CB*/
    }
};

/*BULK Pipe Data*/
static volatile BULK g_Bulk =
{
    false, false, /*Busy Flags*/
    /*IN*/
    {
        {NULL, 0},  /*DataBuff*/
        NULL        /*CB*/
    }
    ,
    /*OUT*/
    {
        0,          /*DataBuffSize*/
        {NULL, 0},  /*DataBuff*/
        NULL        /*CB*/
    }
};

/*INTERRUPT Pipe Data*/
static volatile INTERRUPT g_Int =
{
    false, /*Busy Flag*/
    /*IN*/
    {
        {NULL, 0},  /*DataBuff*/
        NULL        /*CB*/
    }
};

static USBHAL_CONFIG g_Config =
{
    false,  /*m_bBULK_IN_No_Short_Packet*/
    true    /*m_bAutoStallClear*/
};

/*******************************************************************************
Private Function Prototypes
*******************************************************************************/
/*Initialisation*/
static void HW_Init(void);
static void ConfigurePipes(void);
static void SetDefaultInterrupts(void);

/*Operations*/
static void WriteControlINPacket(void);
static void WriteBulkINPacket(void);
static void WriteIntINPacket(void);
static void ReadControlOUTPacket(void);
#if 0
static void ReadBulkOUTPacket(void);
#endif
extern void ReadBulkOUTPacket(void);

/*Interrupt Handlers*/
static void HandleVBus(void);
static void HandleDVST(void);
static void HandleCTRT(void);
static void HandleBEMP(void);
static void HandleBRDY(void);

/*Not direct interrupt handlers*/
static void HandleSetupCmd(void);

/*Set the USB module IO access define based on selected USB Module.
All access to USB peripheral registers is made through this.*/
#ifndef USB_MODULE_0
#error "No USB Module defined"
#endif

/*NOTE USB0 is defined in iodefine.h file*/
#define USBIO USB0
/*HW Initalisation for USB 0 function*/
static void HW_Init_Module0(void);


/*******************************************************************************
Function implementation
***********************************************************************/

/*USB0 interrupt handler*/
//#pragma interrupt Interrupt_USBI0(vect=VECT_USB0_USBI0)
void Interrupt_USBI0(void)
{
//    USBHALInterruptHandler();         //REA: ONR:20150204 Commented this out
}

/*******************************************************************************
* Outline       : USBHAL_Init
* Description   : Initilaises this USB HAL layer.
*                 This must be called before using any other function.
*                 Enables the USB perhipheral ready for enumeration.
*
* Argument      : _fpSetup: Callback function, Setup Packet received.
*                 _fpCable: Callback function, Cable Connected/
                            Disconnected.
*                 _fpError: Callback function, Error occoured.
*
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_Init(CB_SETUP _fpSetup, CB_CABLE _fpCable,
                     CB_ERROR _fpError)
{
    USB_ERR err = USB_ERR_OK;

    /*Check parameters are not NULL*/
    if( (NULL == _fpSetup) ||
        (NULL == _fpCable) ||
        (NULL == _fpError) )
    {
        err = USB_ERR_PARAM;
    }
    else
    {
        /*Store CallBack function pointers*/
        g_CBs.fpSetup = _fpSetup;
        g_CBs.fpCable = _fpCable;
        g_CBs.fpError = _fpError;
    }

    if(USB_ERR_OK == err)
    {
        /*Initialise the USB module.
        This includes enabling USB interrupts*/
        HW_Init();
    }

    return err;
}
/******************************************************************************
End of function USBHAL_Init
******************************************************************************/

/******************************************************************************
* Outline       : USBHAL_Disable
* Description   : Disables the USB module.
* Argument      :
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_Disable(void)
{
    DEBUG_MSG_MID(("USBHAL: - Disable\r\n"));

    /*Stop the selected USB module*/
    #ifdef USB_MODULE_1
        stopModule(MstpIdUSB1);
    #else
        stopModule(MstpIdUSB0);
    #endif

    return USB_ERR_OK;
}
/*******************************************************************************
End USBHAL_Disable function
**********************************************************************/

/*******************************************************************************
* Outline       : USBHAL_Control_Status
* Description   : Send status stage of a control transfer.
*                 This can happen following a setup that has
*                 no data stage or following a data stage OUT.
*                 packet.
* Argument      : none
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_Control_Status(void)
{
    USB_ERR err = USB_ERR_OK;

    DEBUG_MSG_HIGH(("USBHAL: - Control Status\r\n"));

    /*Set CCPL to 1 - this will cause module to automatically handle the status stage.
    i.e Sends ACK to host.*/
    /*Set PID to BUF before setting CCPL*/
    USBIO.DCPCTR.BIT.PID = PID_BUF;
    USBIO.DCPCTR.BIT.CCPL = 1;

    if(STATE_CONTROL_SETUP == g_Control.m_etState)
    {
        /*This is the status stage (ACK)
        at the end of a Control transfer with no data stage.*/
        g_Control.m_etState = STATE_READY;
    }

    return err;
}
/*******************************************************************************
End USBHAL_Control_Status function
**********************************************************************/

/*******************************************************************************
* Outline       : USBHAL_Control_IN
* Description   : Sends supplied data to host and then handles
*                 recieving Status response from host.
*                 Note: This must only be called in response to a setup
*                 packet.
*
*                 Note: Often Windows interrupts this ControlIN with
*                 another setup packet - so this may not get all sent.
*
* Argument      : _NumBytes:    Number of bytes to send.
*                 _Buffer:  Data Buffer.
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_Control_IN(uint16_t _NumBytes, const uint8_t* _Buffer)
{
    USB_ERR err = USB_ERR_OK;

    /*Check state*/
    if(g_Control.m_etState != STATE_CONTROL_SETUP)
    {
        /*State error - didn't expect this now*/
        err = USB_ERR_STATE;
        DEBUG_MSG_LOW(("USBHAL: Control IN State Error\r\n"));
    }
    else
    {
        g_Control.m_etState = STATE_CONTROL_IN;

        DEBUG_MSG_MID(("USBHAL: CONTROL IN start, %u bytes.\r\n", _NumBytes));

        /*Setup data buffer*/
        g_Control.m_IN.m_DataBuff.pucBuf = (uint8_t*)_Buffer;
        g_Control.m_IN.m_DataBuff.NumBytes = _NumBytes;

        /*Send Data*/
        WriteControlINPacket();

        /*Expect to get BEMP interrupt*/
    }

    return err;
}
/*******************************************************************************
End USBHAL_Control_IN function
**********************************************************************/

/*******************************************************************************
* Outline       : USBHAL_Control_OUT
* Description   : This sets up the HAL to receive CONTROL OUT
*                 data from the host and to then send a Status ACK.
*
*                 If a short packet is received before _NumBytes
*                 have been read then this will finish. The actual
*                 number of bytes read will be specified in the CallBack.
*
*                 Note: This must only be called in response to a setup
*                 packet.
*
* Argument      : _NumBytes:    Number of bytes to receive.
*                 _Buffer:  Data Buffer.
*                 _CBDone:  Callback called when OUT has completed.
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_Control_OUT(uint16_t _NumBytes, uint8_t* _Buffer,
                         CB_DONE_OUT _CBDone)
{
    USB_ERR err = USB_ERR_OK;

    /*Check state*/
    if(g_Control.m_etState != STATE_CONTROL_SETUP)
    {
        /*State error - didn't expect this now*/
        err = USB_ERR_STATE;
        DEBUG_MSG_LOW(("USBHAL: Control OUT State Error\r\n"));
    }
    else
    {
        g_Control.m_etState = STATE_CONTROL_OUT;

        DEBUG_MSG_MID(("USBHAL: CONTROL OUT start, %u bytes.\r\n", _NumBytes));

        /*Store parameters*/
        g_Control.m_OUT.m_DataBuff.pucBuf = _Buffer;
        /*Number of bytes received into buffer */
        g_Control.m_OUT.m_DataBuff.NumBytes = 0;
        g_Control.m_OUT.m_BuffSize = _NumBytes;
        g_Control.m_OUT.m_fpDone = _CBDone;

        /*Setup CFIFO to receive data, select DCP (Default Control Pipe)*/
        /*Wait for this to happen*/
        do{
            USBIO.CFIFOSEL.BIT.CURPIPE = 0;
        }while(USBIO.CFIFOSEL.BIT.CURPIPE != 0);

        /*Set PID = BUF*/
        USBIO.DCPCTR.BIT.PID = PID_BUF;
        /*Enable BRDY interrupt for Control pipe - will get interrupt
        when data is received.*/
        USBIO.BRDYENB.BIT.PIPE0BRDYE = 1;
    }

    return err;
}
/*******************************************************************************
End USBHAL_Control_OUT function
**********************************************************************/

/*******************************************************************************
* Outline       : USBHAL_Bulk_OUT
* Description   : This sets up the HAL to receives BULK OUT
*                 data from the host.
*
*                 If a short packet is received before _NumBytes
*                 have been read then this will finish. The actual
*                 number of bytes read will be specified in the CB.
*
* Argument      : _NumBytes:    Number of bytes to receive.
*                 _Buffer:  Data Buffer.
*                 _CBDone:  Callback called when OUT has completed.
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_Bulk_OUT(uint32_t _NumBytes, uint8_t* _Buffer,
                        CB_DONE_OUT _CBDone)
{
    USB_ERR err = USB_ERR_OK;

    /*Check cable is connected*/
    if(STATE_DISCONNECTED == g_Control.m_etState)
    {
        err = USB_ERR_NOT_CONNECTED;
        DEBUG_MSG_MID(("USBHAL: BULK OUT - Not Connected\r\n"));
    }
    else
    {
        /*Check BULK IN isn't busy*/
        if(true == g_Bulk.m_OUTBusy)
        {
            /*Error - already busy*/
            err = USB_ERR_BUSY;
            DEBUG_MSG_LOW(("USBHAL: BULK OUT ***BUSY***\r\n"));
        }
        else
        {
            /*Set busy flag*/
            g_Bulk.m_OUTBusy = true;

            DEBUG_MSG_MID(("USBHAL: BULK OUT start, %lu bytes.\r\n", _NumBytes));

            /*Store parameters*/
            g_Bulk.m_OUT.m_DataBuff.pucBuf = _Buffer;
            /*Number of bytes received into buffer */
            g_Bulk.m_OUT.m_DataBuff.NumBytes = 0;
            g_Bulk.m_OUT.m_BuffSize = _NumBytes;
            g_Bulk.m_OUT.m_fpDone = _CBDone;

            /*Expect to get a BRDY interrupt when data is received*/
        }
    }

    return err;
}
/*******************************************************************************
End USBHAL_Bulk_OUT function
**********************************************************************/

/*******************************************************************************
* Outline       : USBHAL_Bulk_IN
* Description   : Sends supplied data to host using BULK IN
*                 and then calls specified callback.
* Argument      : _NumBytes:    Number of bytes to receive.
*                 _Buffer:  Data Buffer.
*                 _CBDone:  Callback called when IN has completed.
*
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_Bulk_IN(uint32_t _NumBytes, const uint8_t* _Buffer, CB_DONE _CBDone)
{
    USB_ERR err = USB_ERR_OK;

    /*Check cable is connected*/
    if(STATE_DISCONNECTED == g_Control.m_etState)
    {
        err = USB_ERR_NOT_CONNECTED;
        DEBUG_MSG_MID(("USBHAL: BULK IN - Not Connected\r\n"));
    }
    else
    {
        /*Check BULK IN isn't busy*/
        if(true == g_Bulk.m_INBusy)
        {
            /*Error - already busy*/
            err = USB_ERR_BUSY;
            DEBUG_MSG_LOW(("USBHAL: BULK IN ***BUSY***\r\n"));
        }
        else
        {
            /*Set busy flag*/
            g_Bulk.m_INBusy = true;

            DEBUG_MSG_MID(("USBHAL: BULK IN start, %lu bytes.\r\n", _NumBytes));

            /*Setup data buffer*/
            g_Bulk.m_IN.m_DataBuff.pucBuf = (uint8_t*)_Buffer;
            g_Bulk.m_IN.m_DataBuff.NumBytes = _NumBytes;
            g_Bulk.m_IN.m_fpDone = _CBDone;

            /*Write packet. Do this be enabling BRDY interrupt for BULK IN pipe.
            The actual packet will be written out when get the BRDY interrupt.*/
            USBIO.BRDYENB.BIT.PIPE2BRDYE = 1;
        }
    }

    return err;
}
/******************************************************************************
End USBHAL_Bulk_IN function
******************************************************************************/

/******************************************************************************
* Outline       : USBHAL_Interrupt_IN
* Description   : Sends supplied data to host using INTERRUPT IN
*                 and then calls specified callback.
* Argument      : _NumBytes:    Number of bytes to receive.
*                 _Buffer:  Data Buffer.
*                 _CBDone:  Callback called when IN has completed.
*
* Return value  : Error code.
******************************************************************************/
USB_ERR USBHAL_Interrupt_IN(uint32_t _NumBytes,
                             const uint8_t* _Buffer, CB_DONE _CBDone)
{
    USB_ERR err = USB_ERR_OK;

    /*Check cable is connected*/
    if(STATE_DISCONNECTED == g_Control.m_etState)
    {
        err = USB_ERR_NOT_CONNECTED;
        DEBUG_MSG_MID(("USBHAL: INT IN - Not Connected\r\n"));
    }
    else
    {
        /*Check Interrupt IN isn't busy*/
        if(true == g_Int.m_INBusy)
        {
            /*Error - already busy*/
            err = USB_ERR_BUSY;
            DEBUG_MSG_LOW(("USBHAL: INTERRUPT IN ***BUSY***\r\n"));
        }
        else
        {
            /*Set busy flag*/
            g_Int.m_INBusy = true;

            DEBUG_MSG_MID(("USBHAL: INT IN start, %lu bytes.\r\n", _NumBytes));

            /*Setup data buffer*/
            g_Int.m_IN.m_DataBuff.pucBuf = (uint8_t*)_Buffer;
            g_Int.m_IN.m_DataBuff.NumBytes = _NumBytes;
            g_Int.m_IN.m_fpDone = _CBDone;

            /*Write packet. Do this be enabling BRDY interrupt for INTERRUPT IN pipe.
            The actual packet will be written out when get the BRDY interrupt.*/
            USBIO.BRDYENB.BIT.PIPE6BRDYE = 1;
        }
    }

    return err;
}
/******************************************************************************
End USBHAL_Interrupt_IN function
******************************************************************************/

/******************************************************************************
* Outline       : USBHAL_Control_Stall
* Description   : Generate a stall on the Control Endpoint and then
*                 automatically clears it.
* Argument      : none
* Return value  : none
**********************************************************************/
void USBHAL_Control_Stall(void)
{
    DEBUG_MSG_MID(("USBHAL: - Control_Stall\r\n"));

    /*There are two ways of creating a stall depending on current PID*/
    if(USBIO.DCPCTR.BIT.PID == PID_NAK)
    {
        USBIO.DCPCTR.BIT.PID = PID_STALL_1;
    }
    else if(USBIO.DCPCTR.BIT.PID == PID_BUF)
    {
        USBIO.DCPCTR.BIT.PID = PID_STALL_2;
    }
}
/*******************************************************************************
End USBHAL_Control_Stall function
*******************************************************************************/

/**********************************************************************
* Outline       : USBHAL_Bulk_IN_Stall
* Description   : Generate a stall on the BULK IN Endpoint.
* Argument      : none
* Return value  : none
**********************************************************************/
void USBHAL_Bulk_IN_Stall(void)
{
    DEBUG_MSG_MID(("USBHAL: - Bulk_IN_Stall\r\n"));

    /*There are two ways of creating a stall depending on current PID*/
    if(USBIO.PIPE2CTR.BIT.PID == PID_BUF)
    {
        USBIO.PIPE2CTR.BIT.PID = PID_STALL_2;
    }
    else
    {
        USBIO.PIPE2CTR.BIT.PID = PID_STALL_1;
    }
}
/**********************************************************************
End USBHAL_Bulk_IN_Stall function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Bulk_OUT_Stall
* Description   : Generate a stall on the BULK OUT Endpoint.
* Argument      : none
* Return value  : none
**********************************************************************/
void USBHAL_Bulk_OUT_Stall(void)
{
    DEBUG_MSG_MID(("USBHAL: - Bulk_OUT_Stall\r\n"));

    /*There are two ways of creating a stall depending on current PID*/
    if(USBIO.PIPE1CTR.BIT.PID == PID_BUF)
    {
        USBIO.PIPE1CTR.BIT.PID = PID_STALL_2;
    }
    else
    {
        USBIO.PIPE1CTR.BIT.PID = PID_STALL_1;
    }
}
/**********************************************************************
End USBHAL_Bulk_OUT_Stall function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Interrupt_IN_Stall
* Description   : Generate a stall on the Interrupt IN Endpoint.
* Argument      : none
* Return value  : none
**********************************************************************/
void USBHAL_Interrupt_IN_Stall(void)
{
    DEBUG_MSG_MID(("USBHAL: - Interrupt_IN_Stall\r\n"));

    /*There are two ways of creating a stall depending on current PID*/
    if(USBIO.PIPE6CTR.BIT.PID == PID_BUF)
    {
        USBIO.PIPE6CTR.BIT.PID = PID_STALL_2;
    }
    else
    {
        USBIO.PIPE6CTR.BIT.PID = PID_STALL_1;
    }
}
/**********************************************************************
End USBHAL_Interrupt_IN_Stall function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Bulk_IN_Stall_Clear
* Description   : Clear a stall on the BULK IN Endpoint.
*                 Also reset data toggle and clear buffer.
* Argument      : none
* Return value  : none
**********************************************************************/
void USBHAL_Bulk_IN_Stall_Clear(void)
{
    DEBUG_MSG_MID(("USBHAL: - Bulk_IN_Stall Clear\r\n"));

    USBIO.PIPE2CTR.BIT.PID = PID_NAK;

    /*Wait for pipe to be not busy*/
    while(USBIO.PIPE2CTR.BIT.PBUSY == 1){;}

    /*Reset Data toggle*/
    USBIO.PIPE2CTR.BIT.SQCLR = 1;
    /*Clear Buffer*/
    USBIO.PIPE2CTR.BIT.ACLRM = 1;
    USBIO.PIPE2CTR.BIT.ACLRM = 0;
}
/**********************************************************************
End USBHAL_Bulk_IN_Stall_Clear function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Bulk_OUT_Stall_Clear
* Description   : Clear a stall on the BULK OUT Endpoint.
*                 Also reset data toggle and clear buffer.
* Argument      : none
* Return value  : none
**********************************************************************/
void USBHAL_Bulk_OUT_Stall_Clear(void)
{
    DEBUG_MSG_MID(("USBHAL: - Bulk_OUT_Stall Clear\r\n"));

    /*Pipe must not be "curpipe" while being configured.*/
    do{
    USBIO.D1FIFOSEL.BIT.CURPIPE = 0;
    }while(USBIO.D1FIFOSEL.BIT.CURPIPE != 0);

    /*PID must be set to NAK before configuring PIPECFG*/
    USBIO.PIPE1CTR.BIT.PID = PID_NAK;

    /*Wait for pipe to be not busy*/
    while(USBIO.PIPE1CTR.BIT.PBUSY == 1){;}

    /*Reset Data toggle*/
    USBIO.PIPE1CTR.BIT.SQCLR = 1;
    /*Clear Buffer*/
    USBIO.PIPE1CTR.BIT.ACLRM = 1;
    USBIO.PIPE1CTR.BIT.ACLRM = 0;

    /*Set PID to BUF for this OUT pipe*/
    USBIO.PIPE1CTR.BIT.PID = PID_BUF;

    /*Re-select this pipe for D1FIFO*/
    USBIO.D1FIFOSEL.BIT.CURPIPE = PIPE_BULK_OUT;
}
/**********************************************************************
End USBHAL_Bulk_OUT_Stall_Clear function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Interrupt_IN_Stall_Clear
* Description   : Clear a stall on the Interrupt IN Endpoint.
*                 Also reset data toggle and clear buffer.
* Argument      : none
* Return value  : none
**********************************************************************/
void USBHAL_Interrupt_IN_Stall_Clear(void)
{
    DEBUG_MSG_MID(("USBHAL: - Interrupt_IN_Stall Clear\r\n"));

    USBIO.PIPE6CTR.BIT.PID = PID_NAK;

    /*Wait for pipe to be not busy*/
    while(USBIO.PIPE6CTR.BIT.PBUSY == 1){;}

    /*Reset Data toggle*/
    USBIO.PIPE6CTR.BIT.SQCLR = 1;
    /*Clear Buffer*/
    USBIO.PIPE6CTR.BIT.ACLRM = 1;
    USBIO.PIPE6CTR.BIT.ACLRM = 0;
}
/**********************************************************************
End USBHAL_Interrupt_IN_Stall_Clear function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Bulk_IN_Is_Stalled
* Description   : Returns true if the endpoint is stalled.
* Argument      : none
* Return value  : true: Endpoint is stalled.
**********************************************************************/
bool USBHAL_Bulk_IN_Is_Stalled(void)
{
    if( (USBIO.PIPE2CTR.BIT.PID == PID_STALL_1) ||
        (USBIO.PIPE2CTR.BIT.PID == PID_STALL_2))
    {
        return true;
    }
    else
    {
        return false;
    }
}
/**********************************************************************
End USBHAL_Bulk_IN_Is_Stalled function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Bulk_OUT_Is_Stalled
* Description   : Returns true if the endpoint is stalled.
* Argument      : none
* Return value  : true: Endpoint is stalled.
**********************************************************************/
bool USBHAL_Bulk_OUT_Is_Stalled(void)
{
    if( (USBIO.PIPE1CTR.BIT.PID == PID_STALL_1) ||
        (USBIO.PIPE1CTR.BIT.PID == PID_STALL_2))
    {
        return true;
    }
    else
    {
        return false;
    }
}
/**********************************************************************
End USBHAL_Bulk_OUT_Is_Stalled function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Interrupt_IN_Is_Stalled
* Description   : Returns true if the endpoint is stalled.
* Argument      : none
* Return value  : true: Endpoint is stalled.
**********************************************************************/
bool USBHAL_Interrupt_IN_Is_Stalled(void)
{
    if( (USBIO.PIPE6CTR.BIT.PID == PID_STALL_1) ||
        (USBIO.PIPE6CTR.BIT.PID == PID_STALL_2))
    {
        return true;
    }
    else
    {
        return false;
    }
}
/**********************************************************************
End USBHAL_Interrupt_IN_Is_Stalled function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Config_Get
* Description   : Gets the current HAL configuration.
* Argument      : none
* Return value  : Configuration structure.
**********************************************************************/
const USBHAL_CONFIG* USBHAL_Config_Get(void)
{
    return (const USBHAL_CONFIG*)&g_Config;
}
/**********************************************************************
End USBHAL_Config_Get function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Config_Set
* Description   : Sets the HAL configuration.
*                 This only need to be used if the default configuration
*                 is not suitable.
* Argument      : _pConfig: New Configuration.
* Return value  : Error code
**********************************************************************/
USB_ERR USBHAL_Config_Set(USBHAL_CONFIG* _pConfig)
{
    g_Config = *_pConfig;

    return USB_ERR_OK;
}
/**********************************************************************
End USBHAL_Config_Set function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Cancel
* Description   : Cancel any current operations.
*                 Perform a HAL reset.
*                 Any pending "Done" callbacks will then be called with
*                 supplied error code.
*
*                 Note: This is automatically called when the cable
*                 is disconnected.
*
* Argument      : _err: Error code to complete callbacks with.
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_Cancel(USB_ERR _err)
{
    USB_ERR err = USB_ERR_OK;

    CB_DONE cbBulkIN;
    CB_DONE cbIntIN;
    CB_DONE_OUT cbBulkOUT;
    uint32_t NumBytesBulkOUT;

    DEBUG_MSG_MID( ("USBHAL: - Cancel called\r\n"));

    /*Copy callbacks as cleared in USBHAL_Reset*/
    cbBulkIN = g_Bulk.m_IN.m_fpDone;
    cbIntIN = g_Int.m_IN.m_fpDone;
    cbBulkOUT = g_Bulk.m_OUT.m_fpDone;
    /*For OUT also record bytes received so far*/
    NumBytesBulkOUT = g_Bulk.m_OUT.m_DataBuff.NumBytes;

    /*Reset HAL */
    err = USBHAL_Reset();

    /*Call any registered callbacks pending*/
    if((true == g_Bulk.m_INBusy) && (NULL != cbBulkIN))
    {
        cbBulkIN(_err);
    }
    if((true == g_Int.m_INBusy) && (NULL != cbIntIN))
    {
        cbIntIN(_err);
    }
    if((true == g_Bulk.m_OUTBusy) && (NULL != cbBulkOUT))
    {
        cbBulkOUT(_err, NumBytesBulkOUT);
    }

    return err;
}
/**********************************************************************
End USBHAL_Cancel function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_Reset
* Description   : Resets the HAL.
*                 Resets the HAL to default state.
*
*                 Can be used after an unexpected error or when wanting to
*                 cancel a pending operation where you don't want the
*                 'done' callbacks called.
*
*                 Note: Used internally to reset HAL after detecting the USB
*                 cable has been disconnected/connected.
*                 1. Reset State and Busy flags and clear "done" CBs.
*                 (CBs are not called!)
*                 2. Clear all buffers
*                 3. Clear stalls.
*                 4. Enable default interrupts.
* Argument      : none
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_Reset(void)
{
    USB_ERR err = USB_ERR_OK;

    DEBUG_MSG_MID( ("USBHAL: - Resetting HAL\r\n"));

    /*If connected then go to ready state*/
    if(STATE_DISCONNECTED != g_Control.m_etState)
    {
        g_Control.m_etState = STATE_READY;
    }

    /*Reset busy flags*/
    g_Bulk.m_INBusy = false;
    g_Bulk.m_OUTBusy = false;
    g_Int.m_INBusy = false;

    /*Clear all buffers*/
    g_Control.m_IN.m_DataBuff.NumBytes = 0;
    g_Control.m_OUT.m_DataBuff.NumBytes = 0;
    g_Control.m_OUT.m_BuffSize = 0;
    g_Bulk.m_IN.m_DataBuff.NumBytes = 0;
    g_Bulk.m_OUT.m_DataBuff.NumBytes = 0;
    g_Bulk.m_OUT.m_BuffSize = 0;
    g_Int.m_IN.m_DataBuff.NumBytes = 0;

    /*Clear all "Done" CBs*/
    g_Control.m_IN.m_fpDone = NULL;
    g_Control.m_OUT.m_fpDone = NULL;
    g_Bulk.m_IN.m_fpDone = NULL;
    g_Bulk.m_OUT.m_fpDone = NULL;
    g_Int.m_IN.m_fpDone = NULL;

    /*Clear stalls (this also resets data toggle and clears buffers*/
    USBHAL_Bulk_OUT_Stall_Clear();
    USBHAL_Bulk_IN_Stall_Clear();
    USBHAL_Interrupt_IN_Stall_Clear();

    /*Return to the default interrupts*/
    SetDefaultInterrupts();

    return err;
}
/**********************************************************************
End USBHAL_Reset function
**********************************************************************/

/**********************************************************************
* Outline       : USBHAL_ResetEndpoints
* Description   : Resets the data toggle bits and clears buffers.
*                 Should be used following USB bus reset.
*                 USB request.
* Argument      : none
* Return value  : Error code.
**********************************************************************/
USB_ERR USBHAL_ResetEndpoints(void)
{
    /*** BULK OUT ***/
    /*Pipe must not be "curpipe" while being configured.*/
    do{
    USBIO.D1FIFOSEL.BIT.CURPIPE = 0;
    }while(USBIO.D1FIFOSEL.BIT.CURPIPE != 0);

    /*PID must be set to NAK before configuring PIPECFG*/
    USBIO.PIPE1CTR.BIT.PID = PID_NAK;

    /*Wait for pipe to be not busy*/
    while(USBIO.PIPE1CTR.BIT.PBUSY == 1){;}

    /*Reset Data toggle*/
    USBIO.PIPE1CTR.BIT.SQCLR = 1;
    /*Clear Buffer*/
    USBIO.PIPE1CTR.BIT.ACLRM = 1;
    USBIO.PIPE1CTR.BIT.ACLRM = 0;

    /*Set PID to BUF for this OUT pipe*/
    USBIO.PIPE1CTR.BIT.PID = PID_BUF;

    /*Re-select this pipe for D1FIFO and 16bit access.*/
    USBIO.D1FIFOSEL.BIT.MBW = 1;
    USBIO.D1FIFOSEL.BIT.CURPIPE = PIPE_BULK_OUT;

    /***BULK IN***/
    /*PID must be set to NAK before configuring PIPECFG*/
    USBIO.PIPE2CTR.BIT.PID = PID_NAK;

    /*Wait for pipe to be not busy*/
    while(USBIO.PIPE2CTR.BIT.PBUSY == 1){;}

    /*Reset Data toggle*/
    USBIO.PIPE2CTR.BIT.SQCLR = 1;
    /*Clear Buffer*/
    USBIO.PIPE2CTR.BIT.ACLRM = 1;
    USBIO.PIPE2CTR.BIT.ACLRM = 0;

    /***Interrupt IN***/
    /*PID must be set to NAK before configuring PIPECFG*/
    USBIO.PIPE6CTR.BIT.PID = PID_NAK;

    /*Wait for pipe to be not busy*/
    while(USBIO.PIPE6CTR.BIT.PBUSY == 1){;}
    /*Reset Data toggle*/
    USBIO.PIPE6CTR.BIT.SQCLR = 1;
    /*Clear Buffer*/
    USBIO.PIPE6CTR.BIT.ACLRM = 1;
    USBIO.PIPE6CTR.BIT.ACLRM = 0;

    return USB_ERR_OK;
}
/**********************************************************************
End USBHAL_ResetEndpoints function
**********************************************************************/

/**********************************************************************
* Outline       : ReadBulkOUTPacket
* Description   : If a buffer has been setup for BULK OUT then
*                 read data into the buffer.
*                 If buffer is full or a short packet
*                 is received then call the done callback.
*                 If data is available but there is no buffer
*                 setup for it then call the error callback.
* Argument      : none
* Return value  : none
**********************************************************************/
#if 0
static void ReadBulkOUTPacket(void)
{
    uint32_t Count = 0;
    uint16_t DataLength;

    DEBUG_MSG_HIGH(("USBHAL: - ReadBulkOUTPacket\r\n"));

    /*Read data using D1FIFO*/
    /*NOTE: This probably will have already been selected if using BRDY interrupt.*/
    do{
    USBIO.D1FIFOSEL.BIT.CURPIPE = PIPE_BULK_OUT;
    }while(USBIO.D1FIFOSEL.BIT.CURPIPE != PIPE_BULK_OUT);

    /*16 bit access*/
//  USBIO.D1FIFOSEL.BIT.MBW = 1;

    /*Set PID to BUF*/
    USBIO.PIPE1CTR.BIT.PID = PID_BUF;

    /*Wait for buffer to be ready*/
    while(USBIO.D1FIFOCTR.BIT.FRDY == 0){;}

    /*Set Read Count Mode - so DTLN count will decrement as data read from buffer*/
    USBIO.D1FIFOSEL.BIT.RCNT = 1;

    /*Read length of data */
    DataLength = USBIO.D1FIFOCTR.BIT.DTLN;

    /*Read data while there is data in the FIFO and
    room in buffer to store it in.*/
    while((DataLength != 0) &&
          (g_Bulk.m_OUT.m_BuffSize > g_Bulk.m_OUT.m_DataBuff.NumBytes))
    {
        /*If there is at least 2 bytes to read and at least 2 bytes storage in the buffer
        then read 2 bytes with single 16bit access */
        if((DataLength >= 2) &&
            (g_Bulk.m_OUT.m_BuffSize >= (g_Bulk.m_OUT.m_DataBuff.NumBytes+2)))
        {
            /*Read from the FIFO*/
            uint16_t Data = USBIO.D1FIFO;

            if(NULL != g_Bulk.m_OUT.m_DataBuff.pucBuf)
            {
                /*Save data to buffer*/
                /*Save first byte*/
                *g_Bulk.m_OUT.m_DataBuff.pucBuf = (uint8_t)Data;
                g_Bulk.m_OUT.m_DataBuff.pucBuf++;
                /*Save second byte*/
                *g_Bulk.m_OUT.m_DataBuff.pucBuf = (uint8_t)(Data>>8);
                g_Bulk.m_OUT.m_DataBuff.pucBuf++;
            }
            else
            {
                /*Bulk out was set up to read data but throw it away*/
                (void)USBIO.D1FIFO;
            }

            /*Update counts*/
            g_Bulk.m_OUT.m_DataBuff.NumBytes+=2;
            DataLength-=2;
            Count+=2;
        }
        else
        {
            /*The last byte.
            This is written to the Least Significant Byte of 16bit FIFO*/
            if(g_Bulk.m_OUT.m_DataBuff.pucBuf != NULL)
            {
                /* Create 8bit pointer to D1FIFO register */
                uint8_t * ptr = (uint8_t *)&USBIO.D1FIFO;

                /* Read from 8bit pointer */
                *g_Bulk.m_OUT.m_DataBuff.pucBuf = *ptr;
                g_Bulk.m_OUT.m_DataBuff.pucBuf++;
            }
            else
            {
                /* Create 8bit pointer to D1FIFO register */
                uint8_t * ptr = (uint8_t *)&USBIO.D1FIFO;

                /*Bulk out was set up to read data but throw it away*/
                (void)*ptr;
            }
            g_Bulk.m_OUT.m_DataBuff.NumBytes++;
            DataLength--;
            Count++;
        }
    }

    /*Have we completed the BULK OUT request?*/
    /*If this wasn't a full packet or we have filled the buffer*/
    /*Check BULK OUT has been setup*/
    if(0 != g_Bulk.m_OUT.m_BuffSize)
    {
        if((g_Bulk.m_OUT.m_BuffSize == g_Bulk.m_OUT.m_DataBuff.NumBytes) ||
            (Count != BULK_OUT_PACKET_SIZE))
        {
            CB_DONE_OUT CBTemp;

            DEBUG_MSG_MID( ("USBHAL: - ReadBulkOUTPacket Done, %lu Bytes\r\n",
                                 g_Bulk.m_OUT.m_DataBuff.NumBytes));

            /*Buffer not valid anymore*/
            g_Bulk.m_OUT.m_BuffSize = 0;

            /*Before clearing busy flag copy CB.
            NOTE Can't call CB before clearing flag as then CB
            wouldn't be able to set up new request.*/
            CBTemp = g_Bulk.m_OUT.m_fpDone;

            /*Have finished this BULK OUT*/
            g_Bulk.m_OUTBusy = false;

            /*Call registered callback*/
            if(CBTemp != NULL)
            {
                CBTemp(USB_ERR_OK, g_Bulk.m_OUT.m_DataBuff.NumBytes);
            }
        }
    }

    /*NOTE: In double buffer mode DTLN wil be updated with second buffers count.
    Therefore only say error if we've not had a buffer available to at least
    read somne data.*/
    /*If data available and we've not got a buffer then call Error CallBack*/
    if((Count == 0) && (0 != USBIO.D1FIFOCTR.BIT.DTLN))
    {
        /*No buffer available for BULK OUT*/
        DEBUG_MSG_LOW(("USBHAL: ERROR, USB_ERR_BULK_OUT_NO_BUFFER\r\n"));

        /*Call error callback*/
        g_CBs.fpError(USB_ERR_BULK_OUT_NO_BUFFER);

        /*Empty buffer*/
        while(0 != USBIO.D1FIFOCTR.BIT.DTLN)
        {
            /* Create 8bit pointer to D1FIFO register */
                uint8_t * ptr = (uint8_t *)&USBIO.D1FIFO;

            (void)*ptr;
        }
    }
}
#endif
/**********************************************************************
End ReadBulkOUTPacket function
**********************************************************************/

/**********************************************************************
* Outline       : WriteIntINPacket
* Description   : If the Interrupt IN buffer contains data then this will
*                 write it to the pipe buffer until either the packet
*                 is full or all the data has been written.
*                 This controls BRDY interrupt for the pipe to ensure all
*                 data is sent and that the transfer completes on a short packet.
* Argument      : none
* Return value  : none
**********************************************************************/
static void WriteIntINPacket(void)
{
    uint32_t Count = 0;

    /*Write data to Interrupt IN pipe using D0FIFO*/
    /*Select Interrupt Pipe and use 8 bit access*/
    USBIO.D0FIFOSEL.BIT.MBW = 0;

    do{
    USBIO.D0FIFOSEL.BIT.CURPIPE = PIPE_INTERRUPT_IN;
    }while(USBIO.D0FIFOSEL.BIT.CURPIPE != PIPE_INTERRUPT_IN);

    /*Wait for buffer to be ready*/
    while(USBIO.D0FIFOCTR.BIT.FRDY == 0){;}

    /* Write data to the IN Fifo untill have written a full packet
     or we have no more data to write */
    while((Count < INTERRUPT_IN_PACKET_SIZE) &&
         (g_Int.m_IN.m_DataBuff.NumBytes != 0))
    {
        /* Create 8bit pointer to D1FIFO register */
        uint8_t * ptr = (uint8_t *)&USBIO.D0FIFO;

        /* For 8 bit access write only to lower 8 bits of D0FIFO.*/
        *ptr = *g_Int.m_IN.m_DataBuff.pucBuf;
        g_Int.m_IN.m_DataBuff.pucBuf++;
        g_Int.m_IN.m_DataBuff.NumBytes--;
        Count++;
    }

    DEBUG_MSG_HIGH(("USBHAL: INT IN - sending %lu, %lu remaining\r\n",
                      Count, g_Int.m_IN.m_DataBuff.NumBytes));

    /*Send the packet */
    /*Set PID to BUF*/
    USBIO.PIPE6CTR.BIT.PID = PID_BUF;
    /*If we have not written a full packets worth to the buffer then need to
    signal that the buffer is now ready to be sent, set the buffer valid flag (BVAL).*/
    if(Count != BULK_IN_PACKET_SIZE)
    {
        USBIO.D0FIFOCTR.BIT.BVAL = 1;
    }

    /*If have written all data*/
    if(0 == g_Int.m_IN.m_DataBuff.NumBytes)
    {
        /*If this was not a full packet then have finished.*/
        if(Count != INTERRUPT_IN_PACKET_SIZE)
        {
            CB_DONE CBTemp;

            /*Disable interrupts*/
            /*BEMP*/
            USBIO.BEMPENB.BIT.PIPE6BEMPE = 0;
            /*BRDY*/
            USBIO.BRDYENB.BIT.PIPE6BRDYE = 0;

            DEBUG_MSG_MID( ("USBHAL: INT IN - done\r\n"));

            /*Before clearing busy flag copy CB.
            NOTE Can't call CB before clearing flag as then CB
            wouldn't be able to set up new request.*/
            CBTemp = g_Int.m_IN.m_fpDone;

            /*Int IN finished*/
            g_Int.m_INBusy = false;

            /*Call Registered Callback to say this is done*/
            if(NULL != CBTemp)
            {
                CBTemp(USB_ERR_OK);
            }
        }
        else
        {
            /*As this filled the packet exactly we need to send a zero length packet
             to end the transfer*/
            /*Ensure BRDY interrupt is enabled so will send zero packet*/
            USBIO.BRDYENB.BIT.PIPE6BRDYE = 1;
        }
    }
    else
    {
        /*Not written all data so ensure BRDY interrupt is enabled so can send another packet
        when this one has been sent.*/
        USBIO.BRDYENB.BIT.PIPE6BRDYE = 1;
    }
}
/**********************************************************************
End WriteIntINPacket function
**********************************************************************/

/**********************************************************************
* Outline       : WriteBulkINPacket
* Description   : If the Bulk IN buffer contains data then this will
*                 write it to the pipe buffer until either the packet is
*                 full or all the data has been written.
*                 This controls BRDY interrupt for this pipe to ensure
*                 all data is sent and that the transfer completes on a short packet.
*                 (Unless configuration m_bBULK_IN_No_Short_Packet is set.)
* Argument      : none
* Return value  : none
**********************************************************************/
static void WriteBulkINPacket(void)
{
    uint32_t Count = 0;

    /*Write data to Bulk IN pipe using D0FIFO*/
    /*Select pipe (Check this happens before continuing)*/
    /*Set 16 bit access*/
    USBIO.D0FIFOSEL.BIT.MBW = 1;
    do{
        USBIO.D0FIFOSEL.BIT.CURPIPE = PIPE_BULK_IN;
    }while(USBIO.D0FIFOSEL.BIT.CURPIPE != PIPE_BULK_IN);


    /*Wait for buffer to be ready*/
    while(USBIO.D0FIFOCTR.BIT.FRDY == 0){;}

    /* Write data to the IN Fifo untill have written a full packet
     or we have no more data to write */
    while((Count < BULK_IN_PACKET_SIZE) &&
            (g_Bulk.m_IN.m_DataBuff.NumBytes != 0))
    {
        if((g_Bulk.m_IN.m_DataBuff.NumBytes != 1) &&
            ((Count+2) <= BULK_IN_PACKET_SIZE))
        {
            /*Enough data to do a 16bit write and enough room in this packet*/
            uint16_t Data = (uint16_t)*g_Bulk.m_IN.m_DataBuff.pucBuf;
            g_Bulk.m_IN.m_DataBuff.pucBuf++;
            Data |= (uint16_t)(((uint16_t)*g_Bulk.m_IN.m_DataBuff.pucBuf) << 8);
            g_Bulk.m_IN.m_DataBuff.pucBuf++;
            /*Write data to Data Port*/
            USBIO.D0FIFO = Data;
            g_Bulk.m_IN.m_DataBuff.NumBytes-=2;
            Count+=2;
        }
        else
        {
            /*The last byte.
            This is written to the Least Significant Byte of 16bit data port*/
            if(g_Bulk.m_IN.m_DataBuff.pucBuf != NULL)
            {
                /* Create 8bit pointer to USB D0FIFO register */
                uint8_t * ptr = (uint8_t *)&USBIO.D0FIFO;

                /* Write last byte to 8bit data pointer */
                *ptr = *g_Bulk.m_IN.m_DataBuff.pucBuf;

                g_Bulk.m_IN.m_DataBuff.pucBuf++;
            }
            else
            {
                /* Create 8bit pointer to USB D0FIFO register */
                uint8_t * ptr = (uint8_t *)&USBIO.D0FIFO;

                /*This BULK IN was setup to just send dummy data*/
                *ptr = 0xAA;
            }
            g_Bulk.m_IN.m_DataBuff.NumBytes--;
            Count++;
        }
    }

    /*Often commented as it slows it down coniderably.*/
    DEBUG_MSG_HIGH( ("USBHAL: BULK IN - sending %lu, %lu remaining\r\n",
                      Count, g_Bulk.m_IN.m_DataBuff.NumBytes));

    /*Send the packet */
    /*Set PID to BUF*/
    USBIO.PIPE2CTR.BIT.PID = PID_BUF;

    /*If we have not written a full packets worth to the buffer then need to
    signal that the buffer is now ready to be sent, set the buffer valid flag (BVAL).*/
    if(Count != BULK_IN_PACKET_SIZE)
    {
        USBIO.D0FIFOCTR.BIT.BVAL = 1;
    }

    /*If have written all data*/
    if(0 == g_Bulk.m_IN.m_DataBuff.NumBytes)
    {
        /*If this was a full packet then will need to send a zero packet
        to end the transfer (unless m_bBULK_IN_No_Short_Packet) */
        if((Count != BULK_IN_PACKET_SIZE) ||
            (true == g_Config.m_bBULK_IN_No_Short_Packet))
        {
            CB_DONE CBTemp;

            /*Disable this BRDY interrupt*/
            USBIO.BRDYENB.BIT.PIPE2BRDYE = 0;

            DEBUG_MSG_HIGH( ("USBHAL: BULK IN - done.\r\n"));

            /*Before clearing busy flag copy CB.
            NOTE Can't call CB before clearing flag as then CB
            wouldn't be able to set up new request.*/
            CBTemp = g_Bulk.m_IN.m_fpDone;

            /*BULK IN finished*/
            g_Bulk.m_INBusy = false;

            /*Call Registered Callback to say this BULK IN is done*/
            if(NULL != CBTemp)
            {
                CBTemp(USB_ERR_OK);
            }
        }
        else
        {
            /*Enable Interrupt so will send zero packet*/
            USBIO.BRDYENB.BIT.PIPE2BRDYE = 1;
        }
    }
    else
    {
        /*Enable Interrupt so can write rest of data out.*/
        USBIO.BRDYENB.BIT.PIPE2BRDYE = 1;
    }
}
/**********************************************************************
End WriteBulkINPacket function
**********************************************************************/

/**********************************************************************
* Outline       : WriteControlINPacket
* Description   : If the Control IN buffer contains data then this will
*                 write it to the pipe buffer until either the packet
*                 is full or all the data has been written.
*                 If there is no data in the buffer this will send
*                 a zero length packet.
*
* Argument      : none
* Return value  : none
**********************************************************************/
static void WriteControlINPacket(void)
{
    uint16_t Count = 0;

    /*Write data to DCP using CFIFO*/
    /*Select DCP (Check this happens before continuing)*/
    do{
    USBIO.CFIFOSEL.BIT.CURPIPE = 0;
    }while(USBIO.CFIFOSEL.BIT.CURPIPE != 0);

    /*Set 8 bit access*/
//  USBIO.CFIFOSEL.BIT.MBW = 0;

    /*Enable buffer for writing*/
    /*HW Manual says to check the setting has worked*/
    do
    {
        USBIO.CFIFOSEL.BIT.ISEL = 1;
    }while(0 == USBIO.CFIFOSEL.BIT.ISEL);

    /*Wait for buffer to be ready*/
    while(USBIO.CFIFOCTR.BIT.FRDY == 0){;}

    /* Write data to the IN Fifo untill have written a full packet
     or we have no more data to write */
    while((Count < CONTROL_IN_PACKET_SIZE) &&
     (g_Control.m_IN.m_DataBuff.NumBytes != 0))
    {
        /*For 8 bit access write only to low 8 bits of CFIFO, create 8bit
          pointer to CFIFO register.*/
        uint8_t * ptr = (uint8_t *)&USBIO.CFIFO;

        /* Write to register */
        *ptr = (uint8_t) *g_Control.m_IN.m_DataBuff.pucBuf;

        g_Control.m_IN.m_DataBuff.pucBuf++;
        g_Control.m_IN.m_DataBuff.NumBytes--;
        Count++;
    }

    /*Set PID to BUF*/
    USBIO.DCPCTR.BIT.PID = PID_BUF;

    /*If we have not written a full packets worth to the buffer then need to
    signal that the buffer is now ready to be sent, set the buffer valid flag (BVAL).*/
    if(Count != BULK_IN_PACKET_SIZE)
    {
        USBIO.CFIFOCTR.BIT.BVAL = 1;
    }

    DEBUG_MSG_HIGH(("USBHAL: CONTROL IN - sending %u, %lu remaining\r\n",
                      Count, g_Control.m_IN.m_DataBuff.NumBytes));

    /*Expect a BEMP interrupt when this has been sent.
    Can then send more data if required or status stage.*/
}
/**********************************************************************
End WriteControlINPacket function
**********************************************************************/

/**********************************************************************
* Outline       : ReadControlOUTPacket
* Description   : If a buffer has been setup for Control OUT then
*                 read data into the buffer.
*                 If buffer is full or a short packet
*                 is received then call the done callback.
*                 If data is available but there is no buffer
*                 setup for it then call the error callback.

* Argument      : none
* Return value  : none
**********************************************************************/
static void ReadControlOUTPacket(void)
{
    uint32_t Count = 0;

    /*Read data from DCP using CFIFO*/
    /*Select DCP (Check this happens before continuing)*/
    do{
        USBIO.CFIFOSEL.BIT.CURPIPE = 0;
    }while(USBIO.CFIFOSEL.BIT.CURPIPE != 0);

    /*Set 8 bit access*/
//  USBIO.CFIFOSEL.BIT.MBW = 0;

    /*Set PID to BUF*/
    USBIO.DCPCTR.BIT.PID = PID_BUF;

    /*Enable buffer for reading*/
    /*HW Manual says to check the setting has worked*/
    do
    {
        USBIO.CFIFOSEL.BIT.ISEL = 0;
    }while(1 == USBIO.CFIFOSEL.BIT.ISEL);

    /*Set Read Count Mode - so DTLN count will decrement as data read from buffer*/
    USBIO.CFIFOSEL.BIT.RCNT = 1;

    /*Wait for buffer to be ready*/
    while(USBIO.CFIFOCTR.BIT.FRDY == 0){;}

    /*Read data while there is data in the FIFO and
    room in buffer to store it in.*/
    while((g_Control.m_OUT.m_BuffSize > g_Control.m_OUT.m_DataBuff.NumBytes) &&
            (0 != USBIO.CFIFOCTR.BIT.DTLN))
    {
        if(g_Control.m_OUT.m_DataBuff.pucBuf != NULL)
        {
            /* Create 8bit pointer to USB CFIFO register */
            uint8_t * ptr = (uint8_t *)&USBIO.CFIFO;

            *g_Control.m_OUT.m_DataBuff.pucBuf = *ptr;
            g_Control.m_OUT.m_DataBuff.pucBuf++;
            Count++;
        }
        else
        {
            /* Create 8bit pointer to USB CFIFO register */
            uint8_t * ptr = (uint8_t *)&USBIO.CFIFO;

            (void)*ptr;
        }
        g_Control.m_OUT.m_DataBuff.NumBytes++;
    }

    /*Have we completed the CONTROL OUT request?*/
    /*If this wasn't a full packet or we have filled the buffer*/
    if(((0 != g_Control.m_OUT.m_BuffSize) && /*Check CONTROL OUT has been setup*/
    (g_Control.m_OUT.m_BuffSize == g_Control.m_OUT.m_DataBuff.NumBytes)) ||
    (Count != CONTROL_OUT_PACKET_SIZE))
    {
        /*Buffer not valid anymore*/
        g_Control.m_OUT.m_BuffSize = 0;

        /*Disable this interrupt*/
        USBIO.BRDYENB.BIT.PIPE2BRDYE = 0;

        /*End of data transfer - now onto status stage*/
        /*Set CCPL to 1 - this will cause module to automatically handle the status stage.
        i.e Read a zero length packet from host and send ACK response*/
        /*Set PID to BUF before setting CCPL*/
        USBIO.DCPCTR.BIT.PID = PID_BUF;
        USBIO.DCPCTR.BIT.CCPL = 1;

        /*Call registered callback*/
        if(g_Control.m_OUT.m_fpDone != NULL)
        {
            g_Control.m_OUT.m_fpDone(USB_ERR_OK,
                  g_Control.m_OUT.m_DataBuff.NumBytes);
        }
    }

    /*If at this point more data is available then call Error CallBack*/
    if(0 != USBIO.CFIFOCTR.BIT.DTLN)
    {
        /*No buffer available for CONTROL OUT*/
        g_CBs.fpError(USB_ERR_CONTROL_OUT);
    }
}
/**********************************************************************
End ReadControlOUTPacket function
**********************************************************************/

/**********************************************************************
* Outline       : HandleVBus
* Description   : VBUS interrupt has occoured.
*                 USB cable has been connected or disconnected
*                 (or chatter has been detected.)
*                 If Disconnected then use registered callback to inform user.
*                 Note: User is not informed of connection event until SetAddress
*                 USB request is received.
* Argument      : none
* Return value  : none
**********************************************************************/
static void HandleVBus(void)
{
    /*Check VBSTS to see the VBUS value.*/
    uint8_t vbsts_value;
    uint8_t count;
    #define CHATTER_COUNT 5

    /*Keep reading VBSTS until it is stable*/
    do
    {
        count = CHATTER_COUNT;
        /*Read VBSTS then confirm it's value does not change for CHATTER_COUNT times */
        vbsts_value = (uint8_t)USBIO.INTSTS0.BIT.VBSTS;
        while(count != 0)
        {
            if((uint8_t)USBIO.INTSTS0.BIT.VBSTS != vbsts_value)
            {
                /*Chatter - signal is not stable*/
                break;
            }
            else
            {
                count--;
            }
        }
    }while(count != 0);

    if(1 == vbsts_value)
    {
        /* USB Bus Connected */
        g_Control.m_etState = STATE_READY;

        /*Reset HAL*/
        USBHAL_Reset();

        /*Call Registered Callback to tell that cable is connected*/
        /*NOTE: Moved until Set Address has been recieved as more important for
        user to know when enumerated that just vbus change.*/
        /*g_CBs.fpCable(true);*/
    }
    else
    {
        /* USB Bus Disconnected */
        g_Control.m_etState = STATE_DISCONNECTED;

        /*Cancel any pending operations -
        including calling any 'done' callbacks */
        USBHAL_Cancel(USB_ERR_NOT_CONNECTED);

        /*Call Registered Callback*/
        g_CBs.fpCable(false);
    }
}
/**********************************************************************
End HandleVBus function
**********************************************************************/

/**********************************************************************
* Outline       : HandleDVST
* Description   : Device State Transition Interrupt.
*                 Used to detect a USB bus reset.
* Argument      : none
* Return value  : none
**********************************************************************/
static void HandleDVST(void)
{
    /*Read new device state in DVSQ bits*/
    switch(USBIO.INTSTS0.BIT.DVSQ)
    {
        case 0:
            DEBUG_MSG_LOW(("USBHAL: Entered Powered State\r\n"));
        break;
        case 1:
            DEBUG_MSG_LOW(("USBHAL: Entered Default State\r\n"));

            /*Has a bus reset just occured*/
            if(USBIO.DVSTCTR0.BIT.RHST == 4)
            {
                DEBUG_MSG_LOW(("USBHAL: USB BUS RESET\r\n"));

                /*Reset endpoints - this also ensures BULK out is still able
                to receive data.*/
                USBHAL_ResetEndpoints();
            }
        break;
        case 2:
            DEBUG_MSG_LOW(("USBHAL: Entered Address State\r\n"));
            /*This is a conveniant point to tell user that USB is connected. */
            g_CBs.fpCable(true);
        break;
        case 3:
            DEBUG_MSG_LOW(("USBHAL: Entered Configured State\r\n"));
            /*Nothing todo*/
        break;
        case 4:
        default:
            DEBUG_MSG_LOW(("USBHAL: Entered Suspended State\r\n"));
            break;
            /*Nothing todo*/
    }
}
/**********************************************************************
End HandleDVST function
**********************************************************************/

/**********************************************************************
* Outline       : HandleCTRT
* Description   : Control Transfer Stage Transition Interrupt
*                 Used to detect the receipt of a Setup command.
* Argument      : none
* Return value  : none
**********************************************************************/
static void HandleCTRT(void)
{
    switch(USBIO.INTSTS0.BIT.CTSQ)
    {
        case 0:
            DEBUG_MSG_MID(("USBHAL: CTSQ: IDLE or Setup\r\n"));
        break;
        case 1:
            DEBUG_MSG_MID(("USBHAL: CTSQ: Control Read Data\r\n"));
            /*Setup packet has been received.*/
            /*This is a Request that has a data write stage*/
            HandleSetupCmd();
        break;
        case 2:
            DEBUG_MSG_MID(("USBHAL: CTSQ: Control Read Status\r\n"));
        break;
        case 3:
            DEBUG_MSG_MID(("USBHAL: CTSQ: Control Write Data\r\n"));
            /*Setup packet has been received.*/
            /*This is a Request that has a data write stage*/
            HandleSetupCmd();
        break;
        case 4:
            DEBUG_MSG_MID(("USBHAL: CTSQ: Control Write Status\r\n"));
        break;
        case 5:
            DEBUG_MSG_MID(("USBHAL: CTSQ: Control Write(no data) Status\r\n"));
            /*Setup packet has been received.*/
            /*This is a Request that doesn't have a data stage*/
            HandleSetupCmd();
        break;
        case 6:
            DEBUG_MSG_MID(("USBHAL: CTSQ: Control Transfer Error\r\n"));
        break;
        default:
            DEBUG_MSG_LOW(("USBHAL: CTSQ: INVALID\r\n"));
        break;
    };
}
/**********************************************************************
End HandleCTRT function
**********************************************************************/

/**********************************************************************
* Outline       : HandleBRDY
* Description   : Buffer Ready Interrupt.
*                 Used to start/continue:
*                   1. Interrupt IN packets.
*                   2. Bulk IN packets.
*                   3. Bulk IN packets.
*                   4. Bulk OUT packets.
*                   5. Control Out packets.
* Argument      : none
* Return value  : none
**********************************************************************/
static void HandleBRDY(void)
{
    DEBUG_MSG_HIGH(("USBHAL: BRDY\r\n"));

    /*Has Interrupt pipe caused this interrupt (pipe6)*/
    if((1 == USBIO.BRDYSTS.BIT.PIPE6BRDY) && (1 == USBIO.BRDYENB.BIT.PIPE6BRDYE))
    {
        DEBUG_MSG_HIGH(("USBHAL: BRDY PIPE Interrupt\r\n"));

        /*Clear this bit (write 1 to all other bits)*/
        USBIO.BRDYSTS.WORD = ~0x0040;

        /*Send another packet (possibly zero legth)*/
        WriteIntINPacket();
    }

    /*Has BULK IN pipe caused this interrupt (pipe2)*/
    if((1 == USBIO.BRDYSTS.BIT.PIPE2BRDY) && (1== USBIO.BRDYENB.BIT.PIPE2BRDYE))
    {
        DEBUG_MSG_HIGH(("USBHAL: BRDY PIPE Bulk IN\r\n"));

        /*Clear this bit (write 1 to all other bits)*/
        USBIO.BRDYSTS.WORD = ~0x0004;

        /*Send another packet (possibly zero legth)*/
        WriteBulkINPacket();
    }

    /*Has BULK OUT pipe caused this interrupt (pipe1)*/
    if((1 == USBIO.BRDYSTS.BIT.PIPE1BRDY) && (1== USBIO.BRDYENB.BIT.PIPE1BRDYE))
    {
        DEBUG_MSG_HIGH(("USBHAL: BRDY PIPE Bulk OUT\r\n"));

        /*Clear this bit (write 1 to all other bits)*/
        USBIO.BRDYSTS.WORD = ~0x0002;

        /*Read received packet*/
        ReadBulkOUTPacket();
    }

    /*Has Control OUT pipe caused this interrupt (pipe1)*/
    if((1 == USBIO.BRDYSTS.BIT.PIPE0BRDY) && (1== USBIO.BRDYENB.BIT.PIPE0BRDYE))
    {
        DEBUG_MSG_HIGH(("USBHAL: BRDY PIPE Control OUT\r\n"));

        /*Clear this bit (write 1 to all other bits)*/
        USBIO.BRDYSTS.WORD = ~0x0001;

        /*Read received packet*/
        ReadControlOUTPacket();
    }
}
/**********************************************************************
End HandleBRDY function
**********************************************************************/

/**********************************************************************
* Outline       : HandleBEMP
* Description   : Buffer Empty Interrupt
*                 Used for Control IN to write another packet or
*                   move to status stage.
* Argument      : none
* Return value  : none
**********************************************************************/
static void HandleBEMP(void)
{
    DEBUG_MSG_HIGH(("USBHAL: BEMP\r\n"));

    /*Has control pipe caused this interrupt*/
    if(1 == USBIO.BEMPSTS.BIT.PIPE0BEMP)
    {
        DEBUG_MSG_HIGH(("USBHAL: BEMP PIPE0\r\n"));

        /*Clear this bit (write 1 to all other bits)*/
        USBIO.BEMPSTS.WORD = ~0x0001;

        /*If we are performing a CONTROL IN DATA*/
        if(STATE_CONTROL_IN == g_Control.m_etState)
        {
            /*If there is more data to send*/
            if(g_Control.m_IN.m_DataBuff.NumBytes != 0)
            {
                WriteControlINPacket();
            }
            else
            {
                /*End of data transfer*/

                /*Clear CFIFO (seemed necessary to prevent duplicate transfers)*/
                USBIO.CFIFOCTR.BIT.BCLR = 1;

                /*Status Stage*/
                /*Set CCPL to 1 - this will cause module to automatically handle the status stage.
                i.e Transmit zero length packet and recieve ACK from host.*/
                /*Ensure PID is set to BUF before setting CCPL*/
                USBIO.DCPCTR.BIT.PID = PID_BUF;
                USBIO.DCPCTR.BIT.CCPL = 1;
            }
        }
    }

    /*NOTE: for other pipes we are not using BEMP, only BRDY*/
}
/**********************************************************************
End HandleBEMP function
**********************************************************************/

/**********************************************************************
* Outline       : HandleSetupCmd
* Description   : Have received a Control Setup packet.
*                 Use registered callback to inform user.
* Argument      : none
* Return value  : none
**********************************************************************/
static void HandleSetupCmd(void)
{
    uint8_t SetupCmdBuffer[USB_SETUP_PACKET_SIZE];
    uint16_t* p_16;

    DEBUG_MSG_MID(("USBHAL: - Setup Received\r\n"));

    /*Don't check state as host may have aborted other states -
    so this just sets the state to STATE_CONTROL_SETUP*/

    /*Clear VALID flag*/
    USBIO.INTSTS0.BIT.VALID = 0;

    /*Read the 8 bytes setup command into temporary buffer*/
    p_16 = (uint16_t*)SetupCmdBuffer;
    *p_16++ = USBIO.USBREQ.WORD;
    *p_16++ = USBIO.USBVAL;
    *p_16++ = USBIO.USBINDX;
    *p_16 = USBIO.USBLENG;

    g_Control.m_etState = STATE_CONTROL_SETUP;
    /*NOTE: Expect to continue with a Data Stage
     - (either Control IN or Control OUT) or straight to a Status Stage ,
    Users of this HAL must set this up when handling the Setup callback.*/

    /*Call Registered Callback*/
    g_CBs.fpSetup((const uint8_t(*)[USB_SETUP_PACKET_SIZE])SetupCmdBuffer);
}
/**********************************************************************
End HandleSetupCmd function
**********************************************************************/

/**********************************************************************
* Outline       : HW_Init
* Description   : Enable and Initiailise the USB perhipheral.
*                 Note: USB requires EXTAL = 12MHz.
*                 UCLK = EXTAL*4 = 48MHz.
* Argument      : none
* Return value  : none
**********************************************************************/
static void HW_Init(void)
{
    /*Module Initialisation*/
    HW_Init_Module0();

    /*Start USB clock*/
    USBIO.SYSCFG.BIT.SCKE = 1;

    /*Enable USB operation*/
    USBIO.SYSCFG.BIT.USBE = 1;

    /*Select USB function mode*/
    USBIO.SYSCFG.BIT.DCFM = 0;
    /*Enable D+ pull up*/
    USBIO.SYSCFG.BIT.DPRPU = 1;

    /*Configire Pipes/Endpoints */
    ConfigurePipes();

    /* Enable specific USB interrupts */
    SetDefaultInterrupts();
}
/**********************************************************************
End HW_Init function
**********************************************************************/

/**********************************************************************
* Outline       : HW_Init_Module0
* Description   : HW Initialisation specific for module 0
* Argument      : -
* Return value  : -
**********************************************************************/
static void HW_Init_Module0(void)
{
#if 0
    /* Enable write to PFSWE bit */
    MPC.PWPR.BIT.B0WI = 0;
    /* Disable write protection to PFS registers */
    MPC.PWPR.BIT.PFSWE = 1;
#endif

    /* Enable port1 pins 4 an 6 for USB peripheral function */
    assignPinFunction(PIN_IO32, 0x11, 0, 0);
    assignPinFunction(PIN_IO34, 0x11, 0, 0);

    /* Use the USB0_DPUPE pin for peripheral functions */
    PORT1.PMR.BIT.B4 = 1;
    /*Set Port P14(USB0DPUPE) as output direction*/
    PORT1.PDR.BIT.B4 = 0;

    /* Use the USB0_VBUS pin for peripheral functions */
    PORT1.PMR.BIT.B6 = 1;
    /*Set Port P16(USB0_VBUS) as input direction*/
    PORT1.PDR.BIT.B6 = 0;

    /*Enable USB interrupts at high level*/
    ICU.IER[IER_USB0_USBI0].BIT.IEN3 = 1;
    /*Set interrupt priority*/
    ICU.IPR[IPR_USB0_USBI0].BIT.IPR = USB_HAL_INTERRUPT_PROIRITY;
    /*Clear USB0 interrupt flag*/
    ICU.IR[IR_USB0_USBI0].BIT.IR = 0;

    #ifdef USB_MODULE_1
        startModule(MstpIdUSB1);
    #else
        startModule(MstpIdUSB0);
    #endif
}
/**********************************************************************
End HW_Init_Module0 function
**********************************************************************/

/**********************************************************************
* Outline       : ConfigurePipes
* Description   : Configures BULK IN, BULK OUT and Interrupt IN pipes.
*                 Pipe 1 = BULK OUT
*                 Pipe 2 = BULK IN
*                 Pipe 6 = Interrupt IN
* Argument      : -
* Return value  : -
**********************************************************************/
static void ConfigurePipes(void)
{
    /*** BULK OUT ***/
    {
        /*This is written for a particular pipe*/
        #if(1 != PIPE_BULK_OUT)
            #error
        #endif

        /*Select Pipe*/
        USBIO.PIPESEL.BIT.PIPESEL = PIPE_BULK_OUT;

        /*Pipe Control*/
        /*Note: PID must be set to NAK before configuring PIPECFG*/
        USBIO.PIPE1CTR.WORD = 0x000;

        /*Wait for pipe to be not busy*/
        while(USBIO.PIPE1CTR.BIT.PBUSY == 1){;}

        /*Pipe Configure, Direction, Type, Double Buffer, BRDY Interrupt operation*/
        /*Default */
        USBIO.PIPECFG.WORD = 0;
        /*Endpoint*/
        USBIO.PIPECFG.BIT.EPNUM = EP_BULK_OUT;
        /*Direction*/
        USBIO.PIPECFG.BIT.DIR = 0;
        /*Double Buffer */
        USBIO.PIPECFG.BIT.DBLB = 1;
        /*BRDY Interrupt Operation*/
        USBIO.PIPECFG.BIT.BFRE = 0;
        /*Transfer Type */
        USBIO.PIPECFG.BIT.TYPE = 1;

        /*Pipe Packet Size*/
        USBIO.PIPEMAXP.BIT.MXPS = BULK_OUT_PACKET_SIZE;

        /*Reset Data toggle*/
        USBIO.PIPE1CTR.BIT.SQCLR = 1;

        /*Clear Buffer*/
        USBIO.PIPE1CTR.BIT.ACLRM = 1;
        USBIO.PIPE1CTR.BIT.ACLRM = 0;

        /*Set PID to BUF so it can receive data*/
        USBIO.PIPE1CTR.BIT.PID = PID_BUF;

        /*Exclusively use D1FIFO for BULK OUT pipe and 16bit access*/
        USBIO.D1FIFOSEL.BIT.MBW = 1;
        USBIO.D1FIFOSEL.BIT.CURPIPE = PIPE_BULK_OUT;
    }

    /*** BULK IN ***/
    {
        /*This is written for a particular pipe*/
        #if(2 != PIPE_BULK_IN)
            #error
        #endif

        /*Select Pipe*/
        USBIO.PIPESEL.BIT.PIPESEL = PIPE_BULK_IN;

        /*Pipe Control*/
        /*Note: PID must be set to NAK before configuring PIPECFG*/
        USBIO.PIPE2CTR.WORD = 0x000;
        /*Wait for pipe to be not busy*/
        while(USBIO.PIPE2CTR.BIT.PBUSY == 1){;}

        /*Pipe Configure, Direction, Type, Double Buffer, BRDY Interrupt operation*/
        /*Default */
        USBIO.PIPECFG.WORD = 0;
        /*Endpoint*/
        USBIO.PIPECFG.BIT.EPNUM = EP_BULK_IN;
        /*Direction*/
        USBIO.PIPECFG.BIT.DIR = 1;
        /*Double Buffer */
        USBIO.PIPECFG.BIT.DBLB = 1;
        /*BRDY Interrupt Operation*/
        USBIO.PIPECFG.BIT.BFRE = 0;
        /*Transfer Type */
        USBIO.PIPECFG.BIT.TYPE = 1;

        /*Pipe Packet Size*/
        USBIO.PIPEMAXP.BIT.MXPS = BULK_IN_PACKET_SIZE;
    }

    /*** INTERRUPT IN ***/
    {
        /*This is written for a particular pipe*/
        #if(6 != PIPE_INTERRUPT_IN)
            #error
        #endif

        /*Select Pipe*/
        USBIO.PIPESEL.BIT.PIPESEL = PIPE_INTERRUPT_IN;

        /*Pipe Control*/
        /*Note: PID must be set to NAK before configuring PIPECFG*/
        USBIO.PIPE6CTR.WORD = 0x000;

        /*Pipe Configure, Direction, Type, Double Buffer, BRDY Interrupt operation*/
        /*Default */
        USBIO.PIPECFG.WORD = 0;
        /*Endpoint*/
        USBIO.PIPECFG.BIT.EPNUM = EP_INTERRUPT_IN;
        /*Direction*/
        USBIO.PIPECFG.BIT.DIR = 1;
        /*Double Buffer */
        USBIO.PIPECFG.BIT.DBLB = 1;
        /*BRDY Interrupt Operation*/
        USBIO.PIPECFG.BIT.BFRE = 0;
        /*Transfer Type */
        USBIO.PIPECFG.BIT.TYPE = 2;

        /*Pipe Packet Size*/
        USBIO.PIPEMAXP.BIT.MXPS = INTERRUPT_IN_PACKET_SIZE;
    }

    /*Un-select any pipe*/
    USBIO.PIPESEL.BIT.PIPESEL = 0;
}
/**********************************************************************
End ConfigurePipes function
**********************************************************************/

/**********************************************************************
* Outline       : SetDefaultInterrupts
* Description   : Enable the default set of interrupts.
*                 Some interrupts will be enabled later as they are required.
* Argument      : none
* Return value  : none
**********************************************************************/
static void SetDefaultInterrupts(void)
{


    /*Enable VBSE (VBUS) Interrupt*/
    USBIO.INTENB0.BIT.VBSE = 1;
    /*Enable DVST (Device State Transition) Interrupt*/
    USBIO.INTENB0.BIT.DVSE = 1;
    /*Enable CTRT (Control Transfer Stage Transition) Interrupt*/
    USBIO.INTENB0.BIT.CTRE = 1;
    /*Enable BEMP - see particular pipe BEMP enable aswell*/
    USBIO.INTENB0.BIT.BEMPE = 1;
    /*Enable BRDY - see particular pipe BRDY enable aswell*/
    USBIO.INTENB0.BIT.BRDYE = 1;

    /***Pipe specific***/

    /*Contol pipe*/
    /*BEMP: used to continue sending data (or moving on to status stage)
    during CONTROL IN DATA*/
    USBIO.BEMPENB.BIT.PIPE0BEMPE = 1;
    /*BRDY used for Control OUT = enabled when required*/
    USBIO.BRDYENB.BIT.PIPE0BRDYE = 0;

    /*Interrupt IN pipe (pipe6)*/
    /*BEMP - enable only as required*/
    USBIO.BEMPENB.BIT.PIPE6BEMPE = 0;
    /*BRDY - enable only as required*/
    USBIO.BRDYENB.BIT.PIPE6BRDYE = 0;

    /*Bulk IN pipe (pipe2)*/
    /*BEMP - enable only as required*/
    USBIO.BEMPENB.BIT.PIPE2BEMPE = 0;
    /*BRDY - enable only as required*/
    USBIO.BRDYENB.BIT.PIPE2BRDYE = 0;

    /*Bulk OUT pipe (pipe1)*/
    /*BEMP - enable only as required*/
    USBIO.BEMPENB.BIT.PIPE1BEMPE = 0;
    /*BRDY - enable so host can always send us data*/
    USBIO.BRDYENB.BIT.PIPE1BRDYE = 1;
}
/**********************************************************************
End SetDefaultInterrupts function
**********************************************************************/

/**********************************************************************
* Outline       : USBHALInterruptHandler
* Description   : USB interrupt handler.
*                 User must ensure that this gets called when
*                 the USB interrupt occours.
*
* Argument      : none
* Return value  : none
**********************************************************************/
void USBHALInterruptHandler(void)
{
    /*The status flag tells us what caused the interrupt. */

    /*Is this a VBUS Interrupt?*/
    if(1 == USBIO.INTSTS0.BIT.VBINT)
    {
        /*Clear interrupt flag*/
        USBIO.INTSTS0.BIT.VBINT = 0;

        /* VBUS State change i.e USB cable connected/disconnected */
        HandleVBus();
    }

    /*Is this a DVST Interrupt?*/
    if(1 == USBIO.INTSTS0.BIT.DVST)
    {
        /*Clear interrupt flag*/
        USBIO.INTSTS0.BIT.DVST = 0;

        /* Device State Transition */
        HandleDVST();
    }

    /*Is this a CTRT interrupt?*/
    if(1 == USBIO.INTSTS0.BIT.CTRT)
    {
        /* Control Transfer Stage Transition */
        HandleCTRT();

        /*Clear interrupt flag*/
        /*Note: Had to do this after call to HandleCTRT to avoid getting
        two interrupts for same setup packet. */
        USBIO.INTSTS0.BIT.CTRT = 0;
    }

    /*Is this a BEMP interrupt?*/
    if(1 == USBIO.INTSTS0.BIT.BEMP)
    {
        /*Note: Can't clear this flag by SW.*/

        /*Buffer Empty Interrupt*/
        HandleBEMP();
    }

    /*Is this a BRDY interrupt?*/
    if(1 == USBIO.INTSTS0.BIT.BRDY)
    {
        /*Note: Can't clear this flag by SW.*/

        /*Buffer Ready Interrupt*/
        HandleBRDY();
    }
}
/**********************************************************************
End USBHALInterruptHandler function
**********************************************************************/
