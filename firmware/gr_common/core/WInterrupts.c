/* -*- mode: jde; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
  Part of the Wiring project - http://wiring.uniandes.edu.co

  Copyright (c) 2004-05 Hernando Barragan

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
  
  Modified 24 November 2006 by David A. Mellis
  Modified 1 August 2010 by Mark Sproul
*/

#include <inttypes.h>
#ifndef GRSAKURA
#include <avr/io.h>
#include <avr/interrupt.h>
#endif /*GRSAKURA*/
#include <avr/pgmspace.h>
#include <stdio.h>

#include "wiring_private.h"
#include "rx63n/interrupt_handlers.h"

static volatile voidFuncPtr intFunc[EXTERNAL_NUM_INTERRUPTS];
// volatile static voidFuncPtr twiIntFunc;

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
  if(interruptNum < EXTERNAL_NUM_INTERRUPTS) {
    intFunc[interruptNum] = userFunc;
    
    // Configure the interrupt mode (trigger on low input, any change, rising
    // edge, or falling edge).  The mode constants were chosen to correspond
    // to the configuration bits in the hardware register, so we simply shift
    // the mode into place.

#ifdef GRSAKURA
    // Enable the interrupt.
    // Set the digital filters to use PCLK/1 and disable them.
    ICU.IRQFLTC0.WORD = 0U;
    ICU.IRQFLTE0.BYTE = 0U;
    ICU.IRQFLTC1.WORD = 0U;
    ICU.IRQFLTE1.BYTE = 0U;

    // Determine the mode mask.
    unsigned short mode_bitmask = 0x0;
    switch (mode)
    {
    // ------------------------------------------------------------------------/
    case LOW:
        mode_bitmask = 0x00;
        break;
    // ------------------------------------------------------------------------/
    case FALLING:
        mode_bitmask = 0x04;
        break;
    // ------------------------------------------------------------------------/
    case RISING:
        mode_bitmask = 0x08;
        break;
    // ------------------------------------------------------------------------/
    case CHANGE:
        mode_bitmask = 0x0C;
        break;
    // ------------------------------------------------------------------------/
    default:
        return;
    }
#endif //GRSAKURA

#ifndef GRSAKURA
    switch (interruptNum) {
#if defined(__AVR_ATmega32U4__)
	// I hate doing this, but the register assignment differs between the 1280/2560
	// and the 32U4.  Since avrlib defines registers PCMSK1 and PCMSK2 that aren't 
	// even present on the 32U4 this is the only way to distinguish between them.
    case 0:
	EICRA = (EICRA & ~((1<<ISC00) | (1<<ISC01))) | (mode << ISC00);
	EIMSK |= (1<<INT0);
	break;
    case 1:
	EICRA = (EICRA & ~((1<<ISC10) | (1<<ISC11))) | (mode << ISC10);
	EIMSK |= (1<<INT1);
	break;	
    case 2:
        EICRA = (EICRA & ~((1<<ISC20) | (1<<ISC21))) | (mode << ISC20);
        EIMSK |= (1<<INT2);
        break;
    case 3:
        EICRA = (EICRA & ~((1<<ISC30) | (1<<ISC31))) | (mode << ISC30);
        EIMSK |= (1<<INT3);
        break;
    case 4:
        EICRB = (EICRB & ~((1<<ISC60) | (1<<ISC61))) | (mode << ISC60);
        EIMSK |= (1<<INT6);
        break;
#elif defined(EICRA) && defined(EICRB) && defined(EIMSK)
    case 2:
      EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
      EIMSK |= (1 << INT0);
      break;
    case 3:
      EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
      EIMSK |= (1 << INT1);
      break;
    case 4:
      EICRA = (EICRA & ~((1 << ISC20) | (1 << ISC21))) | (mode << ISC20);
      EIMSK |= (1 << INT2);
      break;
    case 5:
      EICRA = (EICRA & ~((1 << ISC30) | (1 << ISC31))) | (mode << ISC30);
      EIMSK |= (1 << INT3);
      break;
    case 0:
      EICRB = (EICRB & ~((1 << ISC40) | (1 << ISC41))) | (mode << ISC40);
      EIMSK |= (1 << INT4);
      break;
    case 1:
      EICRB = (EICRB & ~((1 << ISC50) | (1 << ISC51))) | (mode << ISC50);
      EIMSK |= (1 << INT5);
      break;
    case 6:
      EICRB = (EICRB & ~((1 << ISC60) | (1 << ISC61))) | (mode << ISC60);
      EIMSK |= (1 << INT6);
      break;
    case 7:
      EICRB = (EICRB & ~((1 << ISC70) | (1 << ISC71))) | (mode << ISC70);
      EIMSK |= (1 << INT7);
      break;
#else		
    case 0:
    #if defined(EICRA) && defined(ISC00) && defined(EIMSK)
      EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
      EIMSK |= (1 << INT0);
    #elif defined(MCUCR) && defined(ISC00) && defined(GICR)
      MCUCR = (MCUCR & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
      GICR |= (1 << INT0);
    #elif defined(MCUCR) && defined(ISC00) && defined(GIMSK)
      MCUCR = (MCUCR & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
      GIMSK |= (1 << INT0);
    #elif defined(GRSAKURA)
      //GRSAKURA


    #else
      #error attachInterrupt not finished for this CPU (case 0)
    #endif
      break;

    case 1:
    #if defined(EICRA) && defined(ISC10) && defined(ISC11) && defined(EIMSK)
      EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
      EIMSK |= (1 << INT1);
    #elif defined(MCUCR) && defined(ISC10) && defined(ISC11) && defined(GICR)
      MCUCR = (MCUCR & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
      GICR |= (1 << INT1);
    #elif defined(MCUCR) && defined(ISC10) && defined(GIMSK) && defined(GIMSK)
      MCUCR = (MCUCR & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
      GIMSK |= (1 << INT1);
    #elif defined(GRSAKURA)
      //GRSAKURA
    #else
      #warning attachInterrupt may need some more work for this cpu (case 1)
    #endif
      break;
    
    case 2:
    #if defined(EICRA) && defined(ISC20) && defined(ISC21) && defined(EIMSK)
      EICRA = (EICRA & ~((1 << ISC20) | (1 << ISC21))) | (mode << ISC20);
      EIMSK |= (1 << INT2);
    #elif defined(MCUCR) && defined(ISC20) && defined(ISC21) && defined(GICR)
      MCUCR = (MCUCR & ~((1 << ISC20) | (1 << ISC21))) | (mode << ISC20);
      GICR |= (1 << INT2);
    #elif defined(MCUCR) && defined(ISC20) && defined(GIMSK) && defined(GIMSK)
      MCUCR = (MCUCR & ~((1 << ISC20) | (1 << ISC21))) | (mode << ISC20);
      GIMSK |= (1 << INT2);
    #endif
      break;
#endif
    }
  }
}

#else
    // Determine which IRQ is being requested. Each pin is mapped to a specific
    // IRQ (external pin interrupt) and some IRQs are mapped to multiple pins.
    // Some pin choices are unavailable on the Sakura.
    switch (interruptNum)
    {
    // IRQ0 -------------------------------------------------------------------/
    case PIN_IO36:
        // Clear the interrupt enable for this IRQ vector.
        *GET_ADR_IER(IRQ0) &= ~BIT_00;

        // Set the desired pin to an input.
        pinMode(PIN_IO36, INPUT_PULLUP);

        // Enable the IRQ on this pin. Use byte access to ensure all other bits
        // are cleared.
        MPC.PD0PFS.BYTE = BIT_PFS_ISE;

        // Set the method of interruption.
        *GET_ADR_IRQCR(IRQ0) |= mode_bitmask;

        // Clear the interrupt flag.
        *GET_ADR_IR(IRQ0) &= ~BIT_00;

        // Set the interrupt priority so that the CPU accepts it.
        *GET_ADR_IPR(IRQ0) = DEFAULT_INT_PRIORITY;

        // Enable the interrupt.
        *GET_ADR_IER(IRQ0) |= BIT_00;
        break;

    // IRQ1 -------------------------------------------------------------------/
    case PIN_IO37:
        *GET_ADR_IER(IRQ1) &= ~BIT_01;
        pinMode(PIN_IO37, INPUT_PULLUP);
        MPC.PD1PFS.BYTE = BIT_PFS_ISE;
        *GET_ADR_IRQCR(IRQ1) |= mode_bitmask;
        *GET_ADR_IR(IRQ1) &= ~BIT_00;
        *GET_ADR_IPR(IRQ1) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ1) |= BIT_01;
        break;

    // IRQ2 -------------------------------------------------------------------/
    case PIN_IO30:
    case PIN_IO38:
        *GET_ADR_IER(IRQ2) &= ~BIT_02;
        // Disable pins other than the requested one for this IRQ.
        if (interruptNum == PIN_IO30)
        {
            pinMode(PIN_IO30, INPUT_PULLUP);
            MPC.P12PFS.BYTE = BIT_PFS_ISE;
            MPC.PD2PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO38)
        {
            pinMode(PIN_IO38, INPUT_PULLUP);
            MPC.P12PFS.BYTE = 0U;
            MPC.PD2PFS.BYTE = BIT_PFS_ISE;
        }
        *GET_ADR_IRQCR(IRQ2) |= mode_bitmask;
        *GET_ADR_IR(IRQ2) &= ~BIT_00;
        *GET_ADR_IPR(IRQ2) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ2) |= BIT_02;
        break;

    // IRQ3 -------------------------------------------------------------------/
    case PIN_IO31:
    case PIN_IO39:
        *GET_ADR_IER(IRQ3) &= ~BIT_03;
        if (interruptNum == PIN_IO31)
        {
            pinMode(PIN_IO31, INPUT_PULLUP);
            MPC.P13PFS.BYTE = BIT_PFS_ISE;
            MPC.PD3PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO39)
        {
            pinMode(PIN_IO39, INPUT_PULLUP);
            MPC.P13PFS.BYTE = 0U;
            MPC.PD3PFS.BYTE = BIT_PFS_ISE;
        }
        *GET_ADR_IRQCR(IRQ3) |= mode_bitmask;
        *GET_ADR_IR(IRQ3) &= ~BIT_00;
        *GET_ADR_IPR(IRQ3) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ3) |= BIT_03;
        break;

    // IRQ4 -------------------------------------------------------------------/
    case PIN_IO32:
    case PIN_IO40:
        *GET_ADR_IER(IRQ4) &= ~BIT_04;
        if (interruptNum == PIN_IO32)
        {
            pinMode(PIN_IO32, INPUT_PULLUP);
            MPC.P14PFS.BYTE = BIT_PFS_ISE;
            MPC.PD4PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO40)
        {
            pinMode(PIN_IO40, INPUT_PULLUP);
            MPC.P14PFS.BYTE = 0U;
            MPC.PD4PFS.BYTE = BIT_PFS_ISE;
        }
        *GET_ADR_IRQCR(IRQ4) |= mode_bitmask;
        *GET_ADR_IR(IRQ4) &= ~BIT_00;
        *GET_ADR_IPR(IRQ4) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ4) |= BIT_04;
        break;

    // IRQ5 -------------------------------------------------------------------/
    case PIN_IO33:
    case PIN_IO41:
    case PIN_IO49:
        *GET_ADR_IER(IRQ5) &= ~BIT_05;
        if (interruptNum == PIN_IO33)
        {
            pinMode(PIN_IO33, INPUT_PULLUP);
            MPC.P15PFS.BYTE = BIT_PFS_ISE;
            MPC.PD5PFS.BYTE = 0U;
            MPC.PE5PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO41)
        {
            pinMode(PIN_IO41, INPUT_PULLUP);
            MPC.P15PFS.BYTE = 0U;
            MPC.PD5PFS.BYTE = BIT_PFS_ISE;
            MPC.PE5PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO49)
        {
            pinMode(PIN_IO49, INPUT_PULLUP);
            MPC.P15PFS.BYTE = 0U;
            MPC.PD5PFS.BYTE = 0U;
            MPC.PE5PFS.BYTE = BIT_PFS_ISE;
        }
        *GET_ADR_IRQCR(IRQ5) |= mode_bitmask;
        *GET_ADR_IR(IRQ5) &= ~BIT_00;
        *GET_ADR_IPR(IRQ5) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ5) |= BIT_05;
        break;

    // IRQ6 -------------------------------------------------------------------/
    case PIN_IO34:
    case PIN_IO42:
    case PIN_IO50:
        *GET_ADR_IER(IRQ6) &= ~BIT_06;
        if (interruptNum == PIN_IO34)
        {
            pinMode(PIN_IO34, INPUT_PULLUP);
            MPC.P16PFS.BYTE = BIT_PFS_ISE;
            MPC.PD6PFS.BYTE = 0U;
            MPC.PE6PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO42)
        {
            pinMode(PIN_IO42, INPUT_PULLUP);
            MPC.P16PFS.BYTE = 0U;
            MPC.PD6PFS.BYTE = BIT_PFS_ISE;
            MPC.PE6PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO50)
        {
            pinMode(PIN_IO50, INPUT_PULLUP);
            MPC.P16PFS.BYTE = 0U;
            MPC.PD6PFS.BYTE = 0U;
            MPC.PE6PFS.BYTE = BIT_PFS_ISE;
        }
        *GET_ADR_IRQCR(IRQ6) |= mode_bitmask;
        *GET_ADR_IR(IRQ6) &= ~BIT_00;
        *GET_ADR_IPR(IRQ6) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ6) |= BIT_06;
        break;

    // IRQ7 -------------------------------------------------------------------/
    case PIN_IO35:
    case PIN_IO43:
    case PIN_IO51:
        *GET_ADR_IER(IRQ7) &= ~BIT_07;
        if (interruptNum == PIN_IO35)
        {
            pinMode(PIN_IO35, INPUT_PULLUP);
            MPC.P17PFS.BYTE = BIT_PFS_ISE;
            MPC.PD7PFS.BYTE = 0U;
            MPC.PE7PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO43)
        {
            pinMode(PIN_IO43, INPUT_PULLUP);
            MPC.P17PFS.BYTE = 0U;
            MPC.PD7PFS.BYTE = BIT_PFS_ISE;
            MPC.PE7PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO51)
        {
            pinMode(PIN_IO51, INPUT_PULLUP);
            MPC.P17PFS.BYTE = 0U;
            MPC.PD7PFS.BYTE = 0U;
            MPC.PE7PFS.BYTE = BIT_PFS_ISE;
        }
        *GET_ADR_IRQCR(IRQ7) |= mode_bitmask;
        *GET_ADR_IR(IRQ7) &= ~BIT_00;
        *GET_ADR_IPR(IRQ7) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ7) |= BIT_07;
        break;

    // IRQ8 -------------------------------------------------------------------/
    case PIN_IO1:
        *GET_ADR_IER(IRQ8) &= ~BIT_00;
        pinMode(PIN_IO1, INPUT_PULLUP);
        MPC.P20PFS.BYTE = BIT_PFS_ISE;
        *GET_ADR_IRQCR(IRQ8) |= mode_bitmask;
        *GET_ADR_IR(IRQ8) &= ~BIT_00;
        *GET_ADR_IPR(IRQ8) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ8) |= BIT_00;
        break;

    // IRQ9 -------------------------------------------------------------------/
    case PIN_IO0:
        *GET_ADR_IER(IRQ9) &= ~BIT_01;
        pinMode(PIN_IO0, INPUT_PULLUP);
        MPC.P21PFS.BYTE = BIT_PFS_ISE;
        *GET_ADR_IRQCR(IRQ9) |= mode_bitmask;
        *GET_ADR_IR(IRQ9) &= ~BIT_00;
        *GET_ADR_IPR(IRQ9) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ9) |= BIT_01;
        break;

    // IRQ10 ------------------------------------------------------------------/
    case PIN_IO29:
        *GET_ADR_IER(IRQ10) &= ~BIT_02;
        pinMode(PIN_IO29, INPUT_PULLUP);
        MPC.P55PFS.BYTE = BIT_PFS_ISE;
        *GET_ADR_IRQCR(IRQ10) |= mode_bitmask;
        *GET_ADR_IR(IRQ10) &= ~BIT_00;
        *GET_ADR_IPR(IRQ10) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ10) |= BIT_02;
        break;

    // IRQ11 ------------------------------------------------------------------/
    // No mapping available for the Sakura.

    // IRQ12 ------------------------------------------------------------------/
    case PIN_IO23:
        *GET_ADR_IER(IRQ12) &= ~BIT_04;
        pinMode(PIN_IO23, INPUT_PULLUP);
        MPC.PC1PFS.BYTE = BIT_PFS_ISE;
        *GET_ADR_IRQCR(IRQ12) |= mode_bitmask;
        *GET_ADR_IR(IRQ12) &= ~BIT_00;
        *GET_ADR_IPR(IRQ12) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ12) |= BIT_04;
        break;

    // IRQ13 ------------------------------------------------------------------/
    case PIN_IO53:
    case PIN_IO11:
        *GET_ADR_IER(IRQ13) &= ~BIT_05;
        if (interruptNum == PIN_IO53)
        {
            pinMode(PIN_IO53, INPUT_PULLUP);
            MPC.P05PFS.BYTE = BIT_PFS_ISE;
            MPC.PC6PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO11)
        {
            pinMode(PIN_IO11, INPUT_PULLUP);
            MPC.P05PFS.BYTE = 0U;
            MPC.PC6PFS.BYTE = BIT_PFS_ISE;
        }
        *GET_ADR_IRQCR(IRQ13) |= mode_bitmask;
        *GET_ADR_IR(IRQ13) &= ~BIT_00;
        *GET_ADR_IPR(IRQ13) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ13) |= BIT_05;
        break;

    // IRQ14 ------------------------------------------------------------------/
    case PIN_IO22:
    case PIN_IO12:
        *GET_ADR_IER(IRQ14) &= ~BIT_06;
        if (interruptNum == PIN_IO22)
        {
            pinMode(PIN_IO22, INPUT_PULLUP);
            MPC.PC0PFS.BYTE = BIT_PFS_ISE;
            MPC.PC7PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO12)
        {
            pinMode(PIN_IO12, INPUT_PULLUP);
            MPC.PC0PFS.BYTE = 0U;
            MPC.PC7PFS.BYTE = BIT_PFS_ISE;
        }
        *GET_ADR_IRQCR(IRQ14) |= mode_bitmask;
        *GET_ADR_IR(IRQ14) &= ~BIT_00;
        *GET_ADR_IPR(IRQ14) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ14) |= BIT_06;
        break;

    // IRQ15 ------------------------------------------------------------------/
    case PIN_IO52:
        *GET_ADR_IER(IRQ15) &= ~BIT_07;
        pinMode(PIN_IO52, INPUT_PULLUP);
        MPC.PC1PFS.BYTE = BIT_PFS_ISE;
        *GET_ADR_IRQCR(IRQ15) |= mode_bitmask;
        *GET_ADR_IR(IRQ15) &= ~BIT_00;
        *GET_ADR_IPR(IRQ15) = DEFAULT_INT_PRIORITY;
        *GET_ADR_IER(IRQ15) |= BIT_07;
        break;

    // NMI --------------------------------------------------------------------/
    case PIN_IO54:
        // The NMI pin (P35/IO54) is a special case. First, disable the digital
        // filter for the NMI. Note that the NMI cannot be disabled without a
        // reset once it is enabled.
        ICU.NMIFLTE.BIT.NFLTEN = 0U;
        ICU.NMIFLTC.BIT.NFCLKSEL = 0U;

        // Set the method of interruption. Only falling or rising edges can be
        // used for the NMI.
        if (FALLING == mode)
        {
            ICU.NMICR.BIT.NMIMD = 0U;
        }
        else if (RISING == mode)
        {
            ICU.NMICR.BIT.NMIMD = 1U;
        }
        else
        {
            return;
        }

        // Clear the NMI flag by writing to the NMICLR register.
        ICU.NMICLR.BIT.NMICLR = 1U;

        // Enable the digital filter.
        ICU.NMIFLTE.BIT.NFLTEN = 1U;

        // Enable the interrupt.
        ICU.NMIER.BIT.NMIEN = 1U;
        break;

    // ------------------------------------------------------------------------/
    default:
        break;
    } // switch : interruptNum
#endif //GRSAKURA

  }
}
void detachInterrupt(uint8_t interruptNum) {
  if(interruptNum < EXTERNAL_NUM_INTERRUPTS) {
    // Disable the interrupt.  (We can't assume that interruptNum is equal
    // to the number of the EIMSK bit to clear, as this isn't true on the
    // ATmega8.  There, INT0 is 6 and INT1 is 7.)
    intFunc[interruptNum] = NULL;

#ifndef GRSAKURA
    switch (interruptNum) {
#if defined(__AVR_ATmega32U4__)
    case 0:
        EIMSK &= ~(1<<INT0);
        break;
    case 1:
        EIMSK &= ~(1<<INT1);
        break;
    case 2:
        EIMSK &= ~(1<<INT2);
        break;
    case 3:
        EIMSK &= ~(1<<INT3);
        break;
    case 4:
        EIMSK &= ~(1<<INT6);
        break;
#elif defined(EICRA) && defined(EICRB) && defined(EIMSK)
    case 2:
      EIMSK &= ~(1 << INT0);
      break;
    case 3:
      EIMSK &= ~(1 << INT1);
      break;
    case 4:
      EIMSK &= ~(1 << INT2);
      break;
    case 5:
      EIMSK &= ~(1 << INT3);
      break;
    case 0:
      EIMSK &= ~(1 << INT4);
      break;
    case 1:
      EIMSK &= ~(1 << INT5);
      break;
    case 6:
      EIMSK &= ~(1 << INT6);
      break;
    case 7:
      EIMSK &= ~(1 << INT7);
      break;
#else
    case 0:
    #if defined(EIMSK) && defined(INT0)
      EIMSK &= ~(1 << INT0);
    #elif defined(GICR) && defined(ISC00)
      GICR &= ~(1 << INT0); // atmega32
    #elif defined(GIMSK) && defined(INT0)
      GIMSK &= ~(1 << INT0);
    #elif defined(GRSAKURA)
      //GRSAKURA
    #else
      #error detachInterrupt not finished for this cpu
    #endif
      break;

    case 1:
    #if defined(EIMSK) && defined(INT1)
      EIMSK &= ~(1 << INT1);
    #elif defined(GICR) && defined(INT1)
      GICR &= ~(1 << INT1); // atmega32
    #elif defined(GIMSK) && defined(INT1)
      GIMSK &= ~(1 << INT1);
    #elif defined(GRSAKURA)
      //GRSAKURA
    #else
      #warning detachInterrupt may need some more work for this cpu (case 1)
    #endif
      break;
#endif
    }
#endif // GRSAKURA
    // Determine which IRQ is being disabled. Note that the NMI (IO54) cannot
    // be disabled.
    switch (interruptNum)
    {
    // IRQ0 -------------------------------------------------------------------/
    case PIN_IO36:
        // Clear the interrupt enable for this IRQ vector.
        *GET_ADR_IER(IRQ0) &= ~BIT_00;

        // Clear the IRQ on this pin. Use byte access to ensure all other bits
        // are cleared.
        MPC.PD0PFS.BYTE = 0U;

        // Clear the interrupt priority.
        *GET_ADR_IPR(IRQ0) = 0U;

        // Clear the interrupt flag.
        *GET_ADR_IR(IRQ0) &= ~BIT_00;
        break;

    // IRQ1 -------------------------------------------------------------------/
    case PIN_IO37:
        *GET_ADR_IER(IRQ1) &= ~BIT_01;
        MPC.PD1PFS.BYTE = 0U;
        *GET_ADR_IPR(IRQ1) = 0U;
        *GET_ADR_IR(IRQ1) &= ~BIT_00;
        break;

    // IRQ2 -------------------------------------------------------------------/
    case PIN_IO30:
    case PIN_IO38:
        *GET_ADR_IER(IRQ2) &= ~BIT_02;
        // Disable only the requested pin.
        if (interruptNum == PIN_IO30)
        {
            MPC.P12PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO38)
        {
            MPC.PD2PFS.BYTE = 0U;
        }
        *GET_ADR_IPR(IRQ2) = 0U;
        *GET_ADR_IR(IRQ2) &= ~BIT_00;
        break;

    // IRQ3 -------------------------------------------------------------------/
    case PIN_IO31:
    case PIN_IO39:
        *GET_ADR_IER(IRQ3) &= ~BIT_03;
        if (interruptNum == PIN_IO31)
        {
            MPC.P13PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO39)
        {
            MPC.PD3PFS.BYTE = 0U;
        }
        *GET_ADR_IPR(IRQ3) = 0U;
        *GET_ADR_IR(IRQ3) &= ~BIT_00;
        break;

    // IRQ4 -------------------------------------------------------------------/
    case PIN_IO32:
    case PIN_IO40:
        *GET_ADR_IER(IRQ4) &= ~BIT_04;
        if (interruptNum == PIN_IO32)
        {
            MPC.P14PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO40)
        {
            MPC.PD4PFS.BYTE = 0U;
        }
        *GET_ADR_IPR(IRQ4) = 0U;
        *GET_ADR_IR(IRQ4) &= ~BIT_00;
        break;

    // IRQ5 -------------------------------------------------------------------/
    case PIN_IO33:
    case PIN_IO41:
    case PIN_IO49:
        *GET_ADR_IER(IRQ5) &= ~BIT_05;
        if (interruptNum == PIN_IO33)
        {
            MPC.P15PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO41)
        {
            MPC.PD5PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO49)
        {
            MPC.PE5PFS.BYTE = 0U;
        }
        *GET_ADR_IPR(IRQ5) = 0U;
        *GET_ADR_IR(IRQ5) &= ~BIT_00;
        break;

    // IRQ6 -------------------------------------------------------------------/
    case PIN_IO34:
    case PIN_IO42:
    case PIN_IO50:
        *GET_ADR_IER(IRQ6) &= ~BIT_06;
        if (interruptNum == PIN_IO34)
        {
            MPC.P16PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO42)
        {
            MPC.PD6PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO50)
        {
            MPC.PE6PFS.BYTE = 0U;
        }
        *GET_ADR_IPR(IRQ6) = 0U;
        *GET_ADR_IR(IRQ6) &= ~BIT_00;
        break;

    // IRQ7 -------------------------------------------------------------------/
    case PIN_IO35:
    case PIN_IO43:
    case PIN_IO51:
        *GET_ADR_IER(IRQ7) &= ~BIT_07;
        if (interruptNum == PIN_IO35)
        {
            MPC.P17PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO43)
        {
            MPC.PD7PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO51)
        {
            MPC.PE7PFS.BYTE = 0U;
        }
        *GET_ADR_IPR(IRQ7) = 0U;
        *GET_ADR_IR(IRQ7) &= ~BIT_00;
        break;

    // IRQ8 -------------------------------------------------------------------/
    case PIN_IO1:
        *GET_ADR_IER(IRQ8) &= ~BIT_00;
        MPC.P20PFS.BYTE = 0U;
        *GET_ADR_IPR(IRQ8) = 0U;
        *GET_ADR_IR(IRQ8) &= ~BIT_00;
        break;

    // IRQ9 -------------------------------------------------------------------/
    case PIN_IO0:
        *GET_ADR_IER(IRQ9) &= ~BIT_01;
        MPC.P21PFS.BYTE = 0U;
        *GET_ADR_IPR(IRQ9) = 0U;
        *GET_ADR_IR(IRQ9) &= ~BIT_00;
        break;

    // IRQ10 ------------------------------------------------------------------/
    case PIN_IO29:
        *GET_ADR_IER(IRQ10) &= ~BIT_02;
        MPC.P55PFS.BYTE = 0U;
        *GET_ADR_IPR(IRQ10) = 0U;
        *GET_ADR_IR(IRQ10) &= ~BIT_00;
        break;

    // IRQ11 ------------------------------------------------------------------/
    // No mapping available for the Sakura.

    // IRQ12 ------------------------------------------------------------------/
    case PIN_IO23:
        *GET_ADR_IER(IRQ12) &= ~BIT_04;
        MPC.PC1PFS.BYTE = 0U;
        *GET_ADR_IPR(IRQ12) = 0U;
        *GET_ADR_IR(IRQ12) &= ~BIT_00;
        break;

    // IRQ13 ------------------------------------------------------------------/
    case PIN_IO53:
    case PIN_IO11:
        *GET_ADR_IER(IRQ13) &= ~BIT_05;
        if (interruptNum == PIN_IO53)
        {
            MPC.P05PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO11)
        {
            MPC.PC6PFS.BYTE = 0U;
        }
        *GET_ADR_IPR(IRQ13) = 0U;
        *GET_ADR_IR(IRQ13) &= ~BIT_00;
        break;

    // IRQ14 ------------------------------------------------------------------/
    case PIN_IO22:
    case PIN_IO12:
        *GET_ADR_IER(IRQ14) &= ~BIT_06;
        if (interruptNum == PIN_IO22)
        {
            MPC.PC0PFS.BYTE = 0U;
        }
        if (interruptNum == PIN_IO12)
        {
            MPC.PC7PFS.BYTE = 0U;
        }
        *GET_ADR_IPR(IRQ14) = 0U;
        *GET_ADR_IR(IRQ14) &= ~BIT_00;
        break;

    // IRQ15 ------------------------------------------------------------------/
    case PIN_IO52:
        *GET_ADR_IER(IRQ15) &= ~BIT_07;
        MPC.PC1PFS.BYTE = 0U;
        *GET_ADR_IPR(IRQ15) = 0U;
        *GET_ADR_IR(IRQ15) &= ~BIT_00;
        break;

    // NMI --------------------------------------------------------------------/
    case PIN_IO54:
        // Cannot be disabled.
        break;

    // ------------------------------------------------------------------------/
    default:
        break;
    } // switch : pin.

  }
}
/*
void attachInterruptTwi(void (*userFunc)(void) ) {
  twiIntFunc = userFunc;
}
*/

#if defined(__AVR_ATmega32U4__)
ISR(INT0_vect) {
	if(intFunc[EXTERNAL_INT_0])
		intFunc[EXTERNAL_INT_0]();
}

ISR(INT1_vect) {
	if(intFunc[EXTERNAL_INT_1])
		intFunc[EXTERNAL_INT_1]();
}

ISR(INT2_vect) {
    if(intFunc[EXTERNAL_INT_2])
		intFunc[EXTERNAL_INT_2]();
}

ISR(INT3_vect) {
    if(intFunc[EXTERNAL_INT_3])
		intFunc[EXTERNAL_INT_3]();
}

ISR(INT6_vect) {
    if(intFunc[EXTERNAL_INT_4])
		intFunc[EXTERNAL_INT_4]();
}

#elif defined(EICRA) && defined(EICRB)

ISR(INT0_vect) {
  if(intFunc[EXTERNAL_INT_2])
    intFunc[EXTERNAL_INT_2]();
}

ISR(INT1_vect) {
  if(intFunc[EXTERNAL_INT_3])
    intFunc[EXTERNAL_INT_3]();
}

ISR(INT2_vect) {
  if(intFunc[EXTERNAL_INT_4])
    intFunc[EXTERNAL_INT_4]();
}

ISR(INT3_vect) {
  if(intFunc[EXTERNAL_INT_5])
    intFunc[EXTERNAL_INT_5]();
}

ISR(INT4_vect) {
  if(intFunc[EXTERNAL_INT_0])
    intFunc[EXTERNAL_INT_0]();
}

ISR(INT5_vect) {
  if(intFunc[EXTERNAL_INT_1])
    intFunc[EXTERNAL_INT_1]();
}

ISR(INT6_vect) {
  if(intFunc[EXTERNAL_INT_6])
    intFunc[EXTERNAL_INT_6]();
}

ISR(INT7_vect) {
  if(intFunc[EXTERNAL_INT_7])
    intFunc[EXTERNAL_INT_7]();
}

#else

#ifndef GRSAKURA
ISR(INT0_vect) {
  if(intFunc[EXTERNAL_INT_0])
    intFunc[EXTERNAL_INT_0]();
}

ISR(INT1_vect) {
  if(intFunc[EXTERNAL_INT_1])
    intFunc[EXTERNAL_INT_1]();
}
#else

 //GRSAKURA
// INTERRUPT HANDLERS *********************************************************/
// NMI and IRQ interrupt handlers. Note that all of these are declared in
// interrupts_handlers.h but defined here for clarity.

/**
 * NMI interrupt handler.
 */
void NonMaskableInterrupt(void)
{
    if (intFunc[PIN_IO54] != NULL)
    {
        intFunc[PIN_IO54]();
    }
}

/**
 * IRQ0 interrupt handler.
 */
void INT_Excep_ICU_IRQ0(void)
{
    if (intFunc[PIN_IO36] != NULL)
    {
        intFunc[PIN_IO36]();
    }
}

/**
 * IRQ1 interrupt handler.
 */
void INT_Excep_ICU_IRQ1(void)
{
    if (intFunc[PIN_IO37] != NULL)
    {
        intFunc[PIN_IO37]();
    }
}

/**
 * IRQ2 interrupt handler.
 */
void INT_Excep_ICU_IRQ2(void)
{
    if (intFunc[PIN_IO30] != NULL)
    {
        intFunc[PIN_IO30]();
    } else if (intFunc[PIN_IO38] != NULL){
        intFunc[PIN_IO38]();
    }
}

/**
 * IRQ3 interrupt handler.
 */
void INT_Excep_ICU_IRQ3(void)
{
    if (intFunc[PIN_IO31] != NULL)
    {
        intFunc[PIN_IO31]();
    } else if (intFunc[PIN_IO39] != NULL){
        intFunc[PIN_IO39]();
    }
}

/**
 * IRQ4 interrupt handler.
 */
void INT_Excep_ICU_IRQ4(void)
{
    if (intFunc[PIN_IO32] != NULL)
    {
        intFunc[PIN_IO32]();
    } else if (intFunc[PIN_IO40] != NULL){
        intFunc[PIN_IO40]();
    }
}

/**
 * IRQ5 interrupt handler.
 */
void INT_Excep_ICU_IRQ5(void)
{
    if (intFunc[PIN_IO33] != NULL)
    {
        intFunc[PIN_IO33]();
    } else if (intFunc[PIN_IO41] != NULL){
        intFunc[PIN_IO41]();
    } else if (intFunc[PIN_IO49] != NULL){
        intFunc[PIN_IO49]();
    }
}

/**
 * IRQ6 interrupt handler.
 */
void INT_Excep_ICU_IRQ6(void)
{
    if (intFunc[PIN_IO34] != NULL)
    {
        intFunc[PIN_IO34]();
    } else if (intFunc[PIN_IO42] != NULL){
        intFunc[PIN_IO42]();
    } else if (intFunc[PIN_IO50] != NULL){
        intFunc[PIN_IO50]();
    }
}

/**
 * IRQ7 interrupt handler.
 */
void INT_Excep_ICU_IRQ7(void)
{
    if (intFunc[PIN_IO35] != NULL)
    {
        intFunc[PIN_IO35]();
    } else if (intFunc[PIN_IO43] != NULL){
        intFunc[PIN_IO43]();
    } else if (intFunc[PIN_IO51] != NULL){
        intFunc[PIN_IO51]();
    }
}

/**
 * IRQ8 interrupt handler.
 */
void INT_Excep_ICU_IRQ8(void)
{
    if (intFunc[PIN_IO1] != NULL)
    {
        intFunc[PIN_IO1]();
    }
}
/**
 * IRQ9 interrupt handler.
 */
void INT_Excep_ICU_IRQ9(void)
{
    if (intFunc[PIN_IO0] != NULL)
    {
        intFunc[PIN_IO0]();
    }
}

/**
 * IRQ10 interrupt handler.
 */
void INT_Excep_ICU_IRQ10(void)
{
    if (intFunc[PIN_IO29] != NULL)
    {
        intFunc[PIN_IO29]();
    }
}

/**
 * IRQ11 interrupt handler.
 */
void INT_Excep_ICU_IRQ11(void)
{
}

/**
 * IRQ12 interrupt handler.
 */
void INT_Excep_ICU_IRQ12(void)
{
    if (intFunc[PIN_IO23] != NULL)
    {
        intFunc[PIN_IO23]();
    }
}

/**
 * IRQ13 interrupt handler.
 */
void INT_Excep_ICU_IRQ13(void)
{
    if (intFunc[PIN_IO53] != NULL)
    {
        intFunc[PIN_IO53]();
    } else if (intFunc[PIN_IO11] != NULL){
        intFunc[PIN_IO11]();
    }
}

/**
 * IRQ14 interrupt handler.
 */
void INT_Excep_ICU_IRQ14(void)
{
    if (intFunc[PIN_IO22] != NULL)
    {
        intFunc[PIN_IO22]();
    } else if (intFunc[PIN_IO12] != NULL){
        intFunc[PIN_IO12]();
    }
}

/**
 * IRQ15 interrupt handler.
 */
void INT_Excep_ICU_IRQ15(void)
{
    if (intFunc[PIN_IO52] != NULL)
    {
        intFunc[PIN_IO52]();
    }
}

#endif /*GRSAKURA*/

#if defined(EICRA) && defined(ISC20)
ISR(INT2_vect) {
  if(intFunc[EXTERNAL_INT_2])
    intFunc[EXTERNAL_INT_2]();
}
#endif

#endif

/*
ISR(TWI_vect) {
  if(twiIntFunc)
    twiIntFunc();
}
*/

