/******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized.
* This software is owned by Renesas Electronics Corporation and is  protected
* under all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR  A
* PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE  EXPRESSLY
* DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE  LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this
* software and to discontinue the availability of this software.
* By using this software, you agree to the additional terms and
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
*******************************************************************************
* Copyright (C) 2010(2011) Renesas Electronics Corpration
* and Renesas Solutions Corp. All rights reserved.
*******************************************************************************
* File Name	   : r_flash_api_rx600.c
* Version	   : 1.00
* Device 	   : RX63N
* Tool-Chain   : RX Family C Compiler
* H/W Platform : RX63N
* Description  : Flash programming for the RX63N Group
*******************************************************************************
* History : DD.MM.YYYY Version Description
*         : 22.05.2012 1.00    First Release
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#ifndef GRSAKURA
/* Intrinsic functions of MCU */
#include <machine.h>

/* Type define */
#include "r_usbc_cTypedef.h"
#else
/* Intrinsic functions of MCU */
/* Type define */
#include "r_flash_api_rx600.h"
#endif

/* Get board and MCU definitions. */
#include "iodefine.h"

/******************************************************************************
Macro definitions
******************************************************************************/
/* System clock speed in Hz. */
#define ICLK_HZ					(96000000)
/* Peripheral clock speed in Hz. */
#define PCLK_HZ					(48000000)
/* External bus clock speed in Hz. */
#define BCLK_HZ					(12000000)
/* FlashIF clock speed in Hz. */
#define FCLK_HZ					(48000000)

/*  Max Programming Time for 128 bytes (ROM) is 12ms. */
#define WAIT_MAX_ROM_WRITE      ((int32_t)(  12000 * (50.0 / (FCLK_HZ / 1000000))) * (ICLK_HZ / 1000000))
/*  Max Programming Time for 2 bytes (ROM) is 2ms. */
#define WAIT_MAX_DF_WRITE      ((int32_t)(  2000 * (50.0 / (FCLK_HZ / 1000000))) * (ICLK_HZ / 1000000))
/*  Max timeout value when using the peripheral clock notification command is 60us. */
#define	WAIT_MAX_FCU_CLOCK		((int32_t)(     60 * (50.0 / (FCLK_HZ / 1000000))) * (ICLK_HZ / 1000000))
/*  Max Erasure Time for a 64kB block is around 1152ms. */
#define WAIT_MAX_ERASE			((int32_t)(1152000 * (50.0 / (FCLK_HZ / 1000000))) * (ICLK_HZ / 1000000))
/*  The number of ICLK ticks needed for 35us delay are calculated below */
#define	WAIT_TRESW				(35 * (ICLK_HZ / 1000000))
/*  The number of ICLK ticks needed for 10us delay are calculated below */
#define	WAIT_T10USEC			(10 * (ICLK_HZ / 1000000))
/* The number of loops to wait for FENTRYR timeout. */
#define FLASH_FENTRYR_TIMEOUT   (4)


/* Memory specifics for the RX610 group */
/* Defines the start program/erase address for the different flash areas */
#define	ROM_AREA_OVER			(0x01000000)
#define ROM_AREA_0				(0x00F80000)
#define ROM_AREA_1				(0x00F00000)
#define ROM_AREA_2				(0x00E80000)
#define ROM_AREA_3				(0x00E00000)

/* Programming size for ROM in bytes */
#define ROM_PROGRAM_SIZE		128


/******************************************************************************
Typedef definitions
******************************************************************************/
/* These typedefs are used for guaranteeing correct accesses to memory. When 
   working with the FCU sometimes byte or word accesses are required. */
typedef volatile uint8_t  * FCU_BYTE_PTR;
typedef volatile uint16_t * FCU_WORD_PTR;
typedef volatile uint32_t * FCU_LONG_PTR;


/******************************************************************************
Exported global variables 
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/

/* Stores whether the peripheral clock notification command has been executed */
static uint8_t		g_fcu_pclk_command = 0;

/* Enter PE mode function prototype */
static uint8_t		flash_enter_pe_mode( const FCU_BYTE_PTR fcubpOpe );
static uint8_t      dflash_enter_pe_mode( const FCU_BYTE_PTR fcubpOpe );
/* Exit PE mode function prototype */
static void			flash_exit_pe_mode( const FCU_BYTE_PTR fcubpOpe );

/* Notify peripheral clock function prototype */
static uint8_t		flash_notify_peripheral_clock(FCU_BYTE_PTR flash_addr);



/******************************************************************************
* Function Name	:	flash_Initialize
* Description	:	Initializes the FCU peripheral block.
*					NOTE:This function does not have to execute from in RAM.
* Arguments		:	none
* Return Value	:	FLASH_SUCCESS	= Operation Successful
*					FLASH_FAILURE	= Operation Failed
******************************************************************************/
uint8_t flash_Initialize( void )
{
	/* Declare source and destination pointers */
	uint32_t * src, * dst;

	/* Declare iteration counter variable */
	uint16_t i;

    /* Disable FCU interrupts in FCU block */
	FLASH.FAEINT.BIT.ROMAEIE = 0;
	FLASH.FAEINT.BIT.CMDLKIE = 0;
	FLASH.FAEINT.BIT.DFLAEIE = 0;
	FLASH.FAEINT.BIT.DFLRPEIE = 0;
	FLASH.FAEINT.BIT.DFLWPEIE = 0;

	/* Disable FCU interrupts in ICU */

	/* Disable flash interface error (FIFERR) */
	IPR(FCU, FIFERR) = 0;
	IEN(FCU, FIFERR) = 0;

	/* Disable flash ready interrupt (FRDYI) */
	IPR(FCU, FRDYI) = 0;
	IEN(FCU, FRDYI) = 0;

	/* Before writing data to the FCU RAM, clear FENTRYR to stop the FCU. */
	if( FLASH.FENTRYR.WORD != 0x0000 )
	{
		/* Disable the FCU from accepting commands - Clear both the
		   FENTRY0(ROM) and FENTRYD(Data Flash) bits to 0 */
		FLASH.FENTRYR.WORD = 0xAA00;

		/* Read FENTRYR to ensure it has been set to 0. Note that the top byte
		   of the FENTRYR register is not retained and is read as 0x00. */
		while( 0x0000 != FLASH.FENTRYR.WORD )
		{
			/* Wait until FENTRYR is 0. */
		}
	}

	/* Enable the FCU RAM */
	FLASH.FCURAME.WORD = 0xC401;

	/* Copies the FCU firmware to the FCU RAM.
	   Source: H'FEFFE000 to H'FF00000 (FCU firmware area)
	   Destination: H'007F8000 to H'007FA000 (FCU RAM area) */
	/* Set source pointer */
	src = (uint32_t *)0xFEFFE000;

	/* Set destination pointer */
	dst = (uint32_t *)0x007F8000;

	/* Iterate for loop to copy the FCU firmware */
	for( i = 0; i < (0x2000 / 4); i++ )
	{
		/* Copy data from the source to the destination pointer */
		*dst++ = *src++;
	}

	/* Disable the FRDYI interrupt */
	FLASH.FRDYIE.BIT.FRDYIE = 0;

	/* Return no errors */
	return FLASH_SUCCESS;
}
/******************************************************************************
End of function  flash_Initialize
******************************************************************************/

#ifndef GRSAKURA
#pragma section FRAM
#else
#endif //GRSAKURA

/******************************************************************************
* Function Name	:	flash_coderom_EraseBlock
* Description	:	Erases an entire flash block.
* Arguments		:	fcubpOpe		= Operation address to erase
* Return Value	:	FLASH_SUCCESS	= Operation Successful
*					FLASH_FAILURE	= Operation Failed
******************************************************************************/
uint8_t flash_coderom_EraseBlock( const uint32_t addr )
{
	FCU_BYTE_PTR	fcubpOpe = (FCU_BYTE_PTR)(addr & 0x00FFFFFF);
	uint8_t			result;

#ifndef GRSAKURA
	const uint32_t	pswSaved = get_psw();
	set_psw( pswSaved & ~0x10000 );	// Disable interrupt
#else
    bool di = isNoInterrupts();
    noInterrupts();
#endif //GRSAKURA

	/* Enter ROM PE mode, check if operation successful */
	result = flash_enter_pe_mode( fcubpOpe );
	if( result == FLASH_SUCCESS )
	{
		/* Cancel the ROM Protect feature */
		FLASH.FPROTR.WORD = 0x5501;

		/* Send the FCU Command (ROM block erase) */
		*fcubpOpe = 0x20;
		*fcubpOpe = 0xD0;

		/* Wait while FCU operation is in progress */
		while( FLASH.FSTATR0.BIT.FRDY == 0 )
		{
		}
		
		/* Check if erase operation was successful by checking 
		   bit 'ERSERR' (bit5) and 'ILGLERR' (bit 6) of register 'FSTATR0' */
		if( (FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR  == 1) )
		{
			result = FLASH_FAILURE;
		}
	}

	/* Leave Program/Erase Mode */
	flash_exit_pe_mode( fcubpOpe );

#ifndef GRSAKURA
	set_psw( pswSaved );
#else
    if (!di) {
      interrupts();
    }
#endif
	return result;
}
/******************************************************************************
End of function  flash_coderom_EraseBlock
******************************************************************************/
/******************************************************************************
* Function Name :   flash_datarom_EraseBlock
* Description   :   Erases an entire flash block.
* Arguments     :   fcubpOpe        = Operation address to erase
* Return Value  :   FLASH_SUCCESS   = Operation Successful
*                   FLASH_FAILURE   = Operation Failed
******************************************************************************/
uint8_t flash_datarom_EraseBlock( const uint32_t addr )
{
    FCU_BYTE_PTR    fcubpOpe = (FCU_BYTE_PTR)(addr & 0x00FFFFFF);
    uint8_t         result;

#ifndef GRSAKURA
    const uint32_t  pswSaved = get_psw();
    set_psw( pswSaved & ~0x10000 ); // Disable interrupt
#else
    bool di = isNoInterrupts();
    noInterrupts();
#endif //GRSAKURA

    /* Enter ROM PE mode, check if operation successful */
    result = dflash_enter_pe_mode( fcubpOpe );
    if( result == FLASH_SUCCESS )
    {
        /* Cancel the ROM Protect feature */
        FLASH.DFLWE0.WORD = 0x1eff;
        FLASH.DFLWE1.WORD = 0xe1ff;

        /* Send the FCU Command (ROM block erase) */
        *fcubpOpe = 0x20;
        *fcubpOpe = 0xD0;

        /* Wait while FCU operation is in progress */
        while( FLASH.FSTATR0.BIT.FRDY == 0 )
        {
        }

        /* Check if erase operation was successful by checking
           bit 'ERSERR' (bit5) and 'ILGLERR' (bit 6) of register 'FSTATR0' */
        if( (FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR  == 1) )
        {
            result = FLASH_FAILURE;
        }
    }

    /* Leave Program/Erase Mode */
    flash_exit_pe_mode( fcubpOpe );

#ifndef GRSAKURA
    set_psw( pswSaved );
#else
    if (!di) {
      interrupts();
    }
#endif
    return result;
}
/******************************************************************************
End of function  flash_datarom_EraseBlock
******************************************************************************/

/******************************************************************************
* Function Name	:	flash_coderom_WriteData
* Description	:	Program data into code flash.
* Arguments		:	fcubpOpe		= Operation address to programing.
*					pData			= Data buffer to write.
*					nSize			= Data size to write.
* Return Value	:	FLASH_SUCCESS	= Operation Successful
*					FLASH_FAILURE	= Operation Failed
******************************************************************************/
uint8_t flash_coderom_WriteData( const uint32_t addr, void* pData, const uint16_t nDataSize )
{
	FCU_BYTE_PTR	fcubpOpe = (FCU_BYTE_PTR)(addr & 0x00FFFFFF);
	uint16_t		nLeftSize = nDataSize, i;
	uint16_t*		pwData = (uint16_t*)pData;
	FCU_WORD_PTR	fcuwpWrite = (FCU_WORD_PTR)fcubpOpe;
	uint8_t			result;

#ifndef GRSAKURA
    const uint32_t  pswSaved = get_psw();
    set_psw( pswSaved & ~0x10000 ); // Disable interrupt
#else
    bool di = isNoInterrupts();
    noInterrupts();
#endif //GRSAKURA

	/* Enter PE mode, check if operation is successful */
	result = flash_enter_pe_mode( fcubpOpe );
	if( result == FLASH_SUCCESS )
	{
		/* Cancel the ROM Protect feature */
		FLASH.FPROTR.WORD = 0x5501;

		/* Iterate loop while there are still bytes remaining to write */
		while( nLeftSize )
		{
			if( nLeftSize < 128 )
			{
				result = FLASH_FAILURE;
				break;
			}
		
			/* Write the FCU Program command */
			*fcubpOpe = 0xE8;
			*fcubpOpe = 64;		// 128/2=64word
		
			/* Write 64 words data */
			for( i = 0; i < 64; i++ )
			{
				*fcuwpWrite = *pwData;
			
				fcuwpWrite ++;
				pwData ++;
			}

			/* Write the final FCU command for programming */
			*fcubpOpe = 0xD0;

			/* Wait until FCU operation finishes, or a timeout occurs */
			while( FLASH.FSTATR0.BIT.FRDY == 0 )
			{
			}

			/* Check for illegal command or programming errors */
			if( (FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.PRGERR  == 1) )
			{
				/* Return FLASH_FAILURE, operation failure */
				result = FLASH_FAILURE;
				break;
			}

			/* Increment the flash address and the buffer address by the size of the transfer thats just completed */
			nLeftSize -= 128;
		}
	}

	/* Leave Program/Erase Mode */
	flash_exit_pe_mode( fcubpOpe );

#ifndef GRSAKURA
    set_psw( pswSaved );
#else
    if (!di) {
      interrupts();
    }
#endif

	return result;
}
/******************************************************************************
End of function  flash_coderom_WriteData
******************************************************************************/

/******************************************************************************
* Function Name :   flash_datarom_WriteData
* Description   :   Program data into data flash.
* Arguments     :   fcubpOpe        = Operation address to programming.(4bytes align)
*                   data           = Data buffer to write.
*                   size           = Data size to write.
* Return Value  :   FLASH_SUCCESS   = Operation Successful
*                   FLASH_FAILURE   = Operation Failed
******************************************************************************/
uint8_t flash_datarom_WriteData( const uint32_t addr, void* pData, const uint16_t nDataSize )
{
    FCU_BYTE_PTR    fcubpOpe = (FCU_BYTE_PTR)(addr & 0x00FFFFFF);
    uint16_t        nLeftSize = nDataSize, i;
    uint16_t*       pwData = (uint16_t*)pData;
    FCU_WORD_PTR    fcuwpWrite = (FCU_WORD_PTR)fcubpOpe;
    uint8_t         result;

#ifndef GRSAKURA
    const uint32_t  pswSaved = get_psw();
    set_psw( pswSaved & ~0x10000 ); // Disable interrupt
#else
    bool di = isNoInterrupts();
    noInterrupts();
#endif //GRSAKURA

    /* Enter PE mode, check if operation is successful */
    result = dflash_enter_pe_mode( fcubpOpe );
    if( result == FLASH_SUCCESS )
    {
        /* Cancel the ROM Protect feature */
        FLASH.DFLWE0.WORD = 0x1eff;
        FLASH.DFLWE1.WORD = 0xe1ff;

        /* Iterate loop while there are still bytes remaining to write */
        while( nLeftSize )
        {
            if( nLeftSize < 2 )
            {
                result = FLASH_FAILURE;
                break;
            }

            /* Write the FCU Program command */
            *fcubpOpe = 0xE8;
            *fcubpOpe = 1;     // 1 word

            /* Write 1 word data */
            for( i = 0; i < 1; i++ )
            {
                *fcuwpWrite = *pwData;

                fcuwpWrite ++;
                pwData ++;
            }

            /* Write the final FCU command for programming */
            *fcubpOpe = 0xD0;

            /* Wait until FCU operation finishes, or a timeout occurs */
            while( FLASH.FSTATR0.BIT.FRDY == 0 )
            {
            }

            /* Check for illegal command or programming errors */
            if( (FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.PRGERR  == 1) )
            {
                /* Return FLASH_FAILURE, operation failure */
                result = FLASH_FAILURE;
                break;
            }

            /* Increment the flash address and the buffer address by the size of the transfer thats just completed */
            nLeftSize -= 2;
        }
    }

    /* Leave Program/Erase Mode */
    flash_exit_pe_mode( fcubpOpe );

#ifndef GRSAKURA
    set_psw( pswSaved );
#else
    if (!di) {
      interrupts();
    }
#endif

    /* Return FLASH_SUCCESS, operation success */
    return result;
}
/******************************************************************************
End of function  flash_datarom_WriteData
******************************************************************************/
/******************************************************************************
* Function Name :   flash_datarom_blankcheck
* Description   :   Blank check data flash.
* Arguments     :   addr        = Operation address to check.
* Return Value  :   0   = blank
*                   1   = not blank
******************************************************************************/
bool flash_datarom_blankcheck( const uint32_t addr )
{
    FCU_BYTE_PTR    fcubpOpe = (FCU_BYTE_PTR)(addr & 0x00FFFFFF);

    uint8_t         result;

#ifndef GRSAKURA
    const uint32_t  pswSaved = get_psw();
    set_psw( pswSaved & ~0x10000 ); // Disable interrupt
#else
    bool di = isNoInterrupts();
    noInterrupts();
#endif //GRSAKURA
    FLASH.FMODR.BIT.FRDMD = 1;

    /* Enter PE mode, check if operation is successful */
    result = dflash_enter_pe_mode( fcubpOpe );
    if( result == FLASH_SUCCESS )
    {
        FLASH.DFLBCCNT.WORD = (uint16_t)((addr) & 0x7fe);

        /* Write the FCU Program command */
        *fcubpOpe = 0x71;
        *fcubpOpe = 0xd0;

        /* Wait until FCU operation finishes, or a timeout occurs */
        while( FLASH.FSTATR0.BIT.FRDY == 0 ){}

        /* Check for illegal command or programming errors */
        if( (FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.PRGERR  == 1) )
        {
            /* Return FLASH_FAILURE, operation failure */
            result = FLASH_FAILURE;
        }

    }
    result = FLASH.DFLBCSTAT.WORD;

    /* Leave Program/Erase Mode */
    flash_exit_pe_mode( fcubpOpe );

#ifndef GRSAKURA
    set_psw( pswSaved );
#else
    if (!di) {
      interrupts();
    }
#endif

    /* Return FLASH_SUCCESS, operation success */
    return (bool)result;
}
/******************************************************************************
End of function  flash_datarom_WriteData
******************************************************************************/

/******************************************************************************
* Function Name	:	flash_enter_pe_mode
* Description	:	Puts the FCU into program/erase mode.
* Arguments		:	fcubpOpe		= Operation address to program/erase
* Return Value	:	FLASH_SUCCESS	= Operation Successful
*					FLASH_FAILURE	= Operation Failed
******************************************************************************/
static uint8_t flash_enter_pe_mode( FCU_BYTE_PTR fcubpOpe )
{
	/* FENTRYR must be 0x0000 before bit FENTRY0 or FENTRYD can be set to 1 */
	FLASH.FENTRYR.WORD = 0xAA00;

	/* Read FENTRYR to ensure it has been set to 0. Note that the top byte
	   of the FENTRYR register is not retained and is read as 0x00. */
	while( 0x0000 != FLASH.FENTRYR.WORD )
	{
		/* Wait until FENTRYR is 0. */
	}

	/* Enter ROM PE mode */
	{
		const uint32_t	addrOpe = (uint32_t)fcubpOpe;
		if( (addrOpe >= ROM_AREA_3) && (addrOpe < ROM_AREA_2) )
		{
			FLASH.FENTRYR.WORD = 0xAA08;
		}
		else 
		if( (addrOpe >= ROM_AREA_2) && (addrOpe < ROM_AREA_1) )
		{
			FLASH.FENTRYR.WORD = 0xAA04;
		}
		else 
		if( (addrOpe >= ROM_AREA_1) && (addrOpe < ROM_AREA_0) )
		{
			FLASH.FENTRYR.WORD = 0xAA02;
		}
		else 
		if( (addrOpe >= ROM_AREA_0) && (addrOpe < ROM_AREA_OVER) )
		{
			FLASH.FENTRYR.WORD = 0xAA01;
		}
		else
		{
			return FLASH_FAILURE;
		}
	}
	
	/* Enable Write/Erase of ROM/Data Flash */
	FLASH.FWEPROR.BYTE = 0x01;

	/* Check for FCU error */
	if( (FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR == 1)
	 || (FLASH.FSTATR0.BIT.PRGERR  == 1) || (FLASH.FSTATR1.BIT.FCUERR == 1) )
	{
		/* Return FLASH_FAILURE, operation failure */
		return FLASH_FAILURE;
	}

	/* Check to see if peripheral clock notification command is needed */
	if( g_fcu_pclk_command == 0 )
	{
		/* Inform FCU of flash clock speed, check if operation is succesful */
		if( flash_notify_peripheral_clock( fcubpOpe ) != 0 )
		{
			/* Return FLASH_FAILURE, operation failure */
			return FLASH_FAILURE;
		}

		/* No need to notify FCU of clock supplied to flash again */
		g_fcu_pclk_command = 1;
    }

	/* Return FLASH_SUCCESS, operation successful */
	return FLASH_SUCCESS;
}
/******************************************************************************
End of function flash_enter_pe_mode
******************************************************************************/
/******************************************************************************
* Function Name: data_flash_status_clear
* Description  : Clear the status of the Data Flash operation.
*                NOTE: This function does not have to execute from in RAM.
* Arguments    : none
* Return Value : none
******************************************************************************/
static void data_flash_status_clear (void)
{
    /* Declare temporary pointer */
    FCU_BYTE_PTR ptrb;

    /* Set pointer to Data Flash to issue a FCU command if needed */
    ptrb = (FCU_BYTE_PTR)(100000);

    /* Check to see if an error has occurred with the FCU.  If set, then
       issue a status clear command to bring the FCU out of the
       command-locked state */
    if(FLASH.FSTATR0.BIT.ILGLERR == 1)
    {
        /* FASTAT must be set to 0x10 before the status clear command
           can be successfully issued  */
        if(FLASH.FASTAT.BYTE != 0x10)
        {
            /* Set the FASTAT register to 0x10 so that a status clear
               command can be issued */
            FLASH.FASTAT.BYTE = 0x10;
        }
    }

    /* Issue a status clear command to the FCU */
    *ptrb = 0x50;
}
/******************************************************************************
End of function  data_flash_status_clear
******************************************************************************/

/******************************************************************************
* Function Name :   dflash_enter_pe_mode
* Description   :   Puts the FCU into program/erase mode.
* Arguments     :   fcubpOpe        = Operation address to program/erase
* Return Value  :   FLASH_SUCCESS   = Operation Successful
*                   FLASH_FAILURE   = Operation Failed
******************************************************************************/
static uint8_t dflash_enter_pe_mode( const FCU_BYTE_PTR fcubpOpe )
{
    /* Disable the FRDYI interrupt */
    FLASH.FRDYIE.BIT.FRDYIE = 0;

    /* Set FENTRYD bit(Bit 7) and FKEY (B8-15 = 0xAA) */
    FLASH.FENTRYR.WORD = 0xAA80;

    data_flash_status_clear();

    /* Enable Write/Erase of ROM/Data Flash */
    FLASH.FWEPROR.BYTE = 0x01;

    /* Check for FCU error */
    if(     (FLASH.FSTATR0.BIT.ILGLERR == 1)
        ||  (FLASH.FSTATR0.BIT.ERSERR  == 1)
        ||  (FLASH.FSTATR0.BIT.PRGERR  == 1)
        ||  (FLASH.FSTATR1.BIT.FCUERR  == 1))
    {
        /* Return FLASH_FAILURE, operation failure */
        return FLASH_FAILURE;
    }

    /* Check to see if peripheral clock notification command is needed */
    if( g_fcu_pclk_command == 0 )
    {
        /* Disable FCU interrupts, so interrupt will not trigger after
           peripheral clock notification command */
        FLASH.FRDYIE.BIT.FRDYIE = 0;

        /* Inform FCU of flash clock speed, check if operation is succesful */
        if(flash_notify_peripheral_clock((FCU_BYTE_PTR)fcubpOpe)!=0)
        {
            /* Return FLASH_FAILURE, operation failure */
            return FLASH_FAILURE;
        }
        /* No need to notify FCU of clock supplied to flash again */
        g_fcu_pclk_command = 1;
    }

    /* Return FLASH_SUCCESS, operation successful */
    return FLASH_SUCCESS;
}
/******************************************************************************
End of function flash_enter_pe_mode
******************************************************************************/

/******************************************************************************
* Function Name	:	flash_exit_pe_mode
* Description	:	Takes the FCU out of program/erase mode.
* Arguments		:	fcubpOpe		= Operation address to program/erase
* Return Value	:	none
******************************************************************************/
static void flash_exit_pe_mode( const FCU_BYTE_PTR fcubpOpe )
{
	/* Iterate while loop whilst FCU operation is in progress */
	while( FLASH.FSTATR0.BIT.FRDY == 0 )
	{
	}

	/* Check FSTATR0 and execute a status register clear command if needed */
	if( (FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR == 1) || (FLASH.FSTATR0.BIT.PRGERR == 1) )
	{
		/* Clear ILGLERR */
		if( FLASH.FSTATR0.BIT.ILGLERR == 1 )
		{
			/* FASTAT must be set to 0x10 before the status clear command
			   can be successfully issued  */
			if( FLASH.FASTAT.BYTE != 0x10 )
			{
				/* Set the FASTAT register to 0x10 so that a status clear
				    command can be issued */
				FLASH.FASTAT.BYTE = 0x10;
			}
		}

		/* Send status clear command to FCU */
		*fcubpOpe = 0x50;
	}

	/* Enter ROM Read mode */
	FLASH.FENTRYR.WORD = 0xAA00;

	/* Read FENTRYR to ensure it has been set to 0. Note that the top byte
	   of the FENTRYR register is not retained and is read as 0x00. */
	while( 0x0000 != FLASH.FENTRYR.WORD )
	{
		/* Wait until FENTRYR is 0. */
	}

	/* Flash write/erase disabled */
	FLASH.FWEPROR.BYTE = 0x02;
    FLASH.DFLRE0.WORD = 0x2dff;
    FLASH.DFLRE1.WORD = 0xd2ff;

}
/******************************************************************************
End of function  flash_exit_pe_mode
******************************************************************************/

/******************************************************************************
* Function Name: flash_notify_peripheral_clock
* Description  : Notifies FCU or clock supplied to flash unit
* Arguments    : flash_addr - 
*                    Flash address you will be erasing or writing to
* Return Value : FLASH_SUCCESS - 
*                    Operation Successful
*                FLASH_FAILURE -
*                    Operation Failed
******************************************************************************/
static uint8_t flash_notify_peripheral_clock(FCU_BYTE_PTR flash_addr)
{
	/* Notify Peripheral Clock(PCK) */
	/* Set frequency of PCK in MHz */
	FLASH.PCKAR.WORD = (FCLK_HZ / 1000000);

	/* Execute Peripheral Clock Notification Commands */
	*flash_addr = 0xE9;
	*flash_addr = 0x03;
	*(FCU_WORD_PTR)flash_addr = 0x0F0F;
	*(FCU_WORD_PTR)flash_addr = 0x0F0F;
	*(FCU_WORD_PTR)flash_addr = 0x0F0F;
	*flash_addr = 0xD0;

	/* Check FRDY */
	while( FLASH.FSTATR0.BIT.FRDY == 0 )
	{
	}

	/* Check ILGLERR */
	if( FLASH.FSTATR0.BIT.ILGLERR == 1 )
	{
		/* Return FLASH_FAILURE, operation failure*/
		return FLASH_FAILURE;
	}

	/* Return FLASH_SUCCESS, operation success */
	return FLASH_SUCCESS;

}

/******************************************************************************
End of function flash_notify_peripheral_clock
******************************************************************************/

