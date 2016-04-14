/*
  MsTimer2.h - Using timer2 with 1ms resolution
  Javier Valencia <javiervalencia80@gmail.com>

  History:
    29/May/09 - V0.5 added support for Atmega1280 (thanks to Manuel Negri)
    19/Mar/09 - V0.4 added support for ATmega328P (thanks to Jerome Despatis)
    11/Jun/08 - V0.3
        changes to allow working with different CPU frequencies
        added support for ATMega128 (using timer2)
        compatible with ATMega48/88/168/8
    10/May/08 - V0.2 added some security tests and volatile keywords
    9/May/08 - V0.1 released working on ATMEGA168 only


  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/*
 *  Modified 14 May 2014 by Yuuki Okamiya, for GR-SAKURA.
 *  Modified 17 Jun 2014 by Nozomu Fujita : TPU1.TGRA に設定する値を修正
 */

#include <MsTimer2.h>
#if defined (GRSAKURA)
#include "rx63n/iodefine.h"
#include "rx63n/interrupt_handlers.h"
#endif

unsigned long MsTimer2::msecs;
void (*MsTimer2::func)();
volatile unsigned long MsTimer2::count;
volatile char MsTimer2::overflowing;
volatile unsigned int MsTimer2::tcnt2;

void MsTimer2::set(unsigned long ms, void (*f)()) {
    float prescaler = 0.0;

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__)
    TIMSK2 &= ~(1<<TOIE2);
    TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
    TCCR2B &= ~(1<<WGM22);
    ASSR &= ~(1<<AS2);
    TIMSK2 &= ~(1<<OCIE2A);

    if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {    // prescaler set to 64
        TCCR2B |= (1<<CS22);
        TCCR2B &= ~((1<<CS21) | (1<<CS20));
        prescaler = 64.0;
    } else if (F_CPU < 1000000UL) { // prescaler set to 8
        TCCR2B |= (1<<CS21);
        TCCR2B &= ~((1<<CS22) | (1<<CS20));
        prescaler = 8.0;
    } else { // F_CPU > 16Mhz, prescaler set to 128
        TCCR2B |= ((1<<CS22) | (1<<CS20));
        TCCR2B &= ~(1<<CS21);
        prescaler = 128.0;
    }
#elif defined (__AVR_ATmega8__)
    TIMSK &= ~(1<<TOIE2);
    TCCR2 &= ~((1<<WGM21) | (1<<WGM20));
    TIMSK &= ~(1<<OCIE2);
    ASSR &= ~(1<<AS2);

    if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {    // prescaler set to 64
        TCCR2 |= (1<<CS22);
        TCCR2 &= ~((1<<CS21) | (1<<CS20));
        prescaler = 64.0;
    } else if (F_CPU < 1000000UL) { // prescaler set to 8
        TCCR2 |= (1<<CS21);
        TCCR2 &= ~((1<<CS22) | (1<<CS20));
        prescaler = 8.0;
    } else { // F_CPU > 16Mhz, prescaler set to 128
        TCCR2 |= ((1<<CS22) && (1<<CS20));
        TCCR2 &= ~(1<<CS21);
        prescaler = 128.0;
    }
#elif defined (__AVR_ATmega128__)
    TIMSK &= ~(1<<TOIE2);
    TCCR2 &= ~((1<<WGM21) | (1<<WGM20));
    TIMSK &= ~(1<<OCIE2);

    if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {    // prescaler set to 64
        TCCR2 |= ((1<<CS21) | (1<<CS20));
        TCCR2 &= ~(1<<CS22);
        prescaler = 64.0;
    } else if (F_CPU < 1000000UL) { // prescaler set to 8
        TCCR2 |= (1<<CS21);
        TCCR2 &= ~((1<<CS22) | (1<<CS20));
        prescaler = 8.0;
    } else { // F_CPU > 16Mhz, prescaler set to 256
        TCCR2 |= (1<<CS22);
        TCCR2 &= ~((1<<CS21) | (1<<CS20));
        prescaler = 256.0;
    }
#elif defined (GRSAKURA)
//    SYSTEM.PRCR.WORD = 0xA503;
    // Cancel any module stops for the TPUB channels and re-lock.
    SYSTEM.MSTPCRA.BIT.MSTPA13 = 0U;
//    SYSTEM.PRCR.WORD = 0xA500;

    // Stop the timer.
    TPUA.TSTR.BIT.CST1 = 0U;
    // Set the counter to run at the desired frequency.
    TPU1.TCR.BIT.TPSC = 0b011;
    prescaler = 64.0;

    // Set TGRA compare match to clear TCNT.
    TPU1.TCR.BIT.CCLR = 0b001;
    // Set the count to occur on rising edge of PCLK.
    TPU1.TCR.BIT.CKEG = 0b01;
    // Set Normal.
    TPU1.TMDR.BIT.MD = 0b0000;

    // Set the count to occur on rising edge of PCLK.
    TPU1.TSR.BIT.TGFA = 0U;
    /* Set TGI6A interrupt priority level to 5*/
//    IPR(TPU1,TGI1A) = 0x5;
    IPR(TPU1,TGI1A) = 0x4;
    /* Enable TGI6A interrupts */
    IEN(TPU1,TGI1A) = 0x1;
    /* Clear TGI6A interrupt flag */
    IR(TPU1,TGI1A) = 0x0;
    // Enable the module interrupt for the ms timer.
    TPU1.TIER.BIT.TGIEA = 1U;

#endif
#if defined (GRSAKURA)
    tcnt2 = (int)((float)F_CPU * 0.001 / prescaler);
#else
    tcnt2 = 256 - (int)((float)F_CPU * 0.001 / prescaler);
#endif

    if (ms == 0)
        msecs = 1;
    else
        msecs = ms;

    func = f;
}

void MsTimer2::start() {
    count = 0;
    overflowing = 0;
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__)
    TCNT2 = tcnt2;
    TIMSK2 |= (1<<TOIE2);
#elif defined (__AVR_ATmega128__)
    TCNT2 = tcnt2;
    TIMSK |= (1<<TOIE2);
#elif defined (__AVR_ATmega8__)
    TCNT2 = tcnt2;
    TIMSK |= (1<<TOIE2);
#elif defined (GRSAKURA)
    // Set the period.
    TPU1.TGRA = tcnt2 - 1;
    // Start the timer.
    TPUA.TSTR.BIT.CST1 = 1U;

#endif
}

void MsTimer2::stop() {
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__)
    TIMSK2 &= ~(1<<TOIE2);
#elif defined (__AVR_ATmega128__)
    TIMSK &= ~(1<<TOIE2);
#elif defined (__AVR_ATmega8__)
    TIMSK &= ~(1<<TOIE2);
#elif defined (GRSAKURA)
    // Stop the timer.
    TPUA.TSTR.BIT.CST1 = 0U;
#endif
}

void MsTimer2::_overflow() {
    count += 1;

    if (count >= msecs && !overflowing) {
        overflowing = 1;
        count = 0;
        (*func)();
        overflowing = 0;
    }
}

#ifndef GRSAKURA
ISR(TIMER2_OVF_vect) {
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__)
    TCNT2 = MsTimer2::tcnt2;
#elif defined (__AVR_ATmega128__)
    TCNT2 = MsTimer2::tcnt2;
#elif defined (__AVR_ATmega8__)
    TCNT2 = MsTimer2::tcnt2;
#endif
    MsTimer2::_overflow();
}
#else
// INTERRUPT HANDLERS *********************************************************/
// Note that these are declared in interrupts_handlers.h but defined here for
// clarity.

// TPU4 TGI4A
void INT_Excep_TPU1_TGI1A(void){
    MsTimer2::_overflow();
}
#endif
