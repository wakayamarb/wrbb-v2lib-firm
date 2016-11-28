/*
  wiring.c - Partial implementation of the Wiring API for the ATmega8.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

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

  $Id$
*/
/*
  Modified 15 July 2014 by Nozomu Fujita for GR-SAKURA
*/

#include "wiring_private.h"
#ifdef GRSAKURA
#include <stdint.h>
#include "rx63n/interrupt_handlers.h"
#include "rx63n/util.h"
#endif /*GRSAKURA*/

#ifndef GRSAKURA
// the prescaler is set so that timer0 ticks every 64 clock cycles, and the
// the overflow handler is called every 256 ticks.
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))

// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)
#else  /*GRSAKURA*/
#define TicksForMillis (PCLK / 8 / 1000)
#endif /*GRSAKURA*/

#ifndef GRSAKURA
volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
ISR(TIM0_OVF_vect)
#else
ISR(TIMER0_OVF_vect)
#endif
{
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}
#else /*GRSAKURA*/
volatile unsigned long timer0_millis = 0;

void INT_Excep_CMT0_CMI0(void)
{
	timer0_millis++;
}

static inline unsigned long timerTicks()
{
	noInterrupts();
	unsigned long ms = timer0_millis;
	unsigned long cmcnt = CMT0.CMCNT;
	int ir = IR(CMT0, CMI0);
	interrupts();
	if (ir && cmcnt < TicksForMillis / 2) {
		ms++;
	}
	return TicksForMillis * ms + cmcnt;
}

static void delayTicks(unsigned long ticks)
{
	if (!isNoInterrupts()) {
		unsigned long s = timerTicks();
		for (;;) {
			unsigned long l = timerTicks();
			unsigned long d = l - s;
			if (ticks <= d) {
				break;
			}
			ticks -= d;
			s = l;
		}
	} else {
		unsigned short s = CMT0.CMCNT;
		for (;;) {
			unsigned short l = CMT0.CMCNT;
			unsigned short d = l - s;
			if (l < s) {
				d += TicksForMillis;
			}
			if (ticks <= d) {
				break;
			}
			ticks -= d;
			s = l;
		}
	}
}
#endif/*GRSAKURA*/

unsigned long millis()
{
#ifndef GRSAKURA
	unsigned long m;
	uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer0_millis)
	cli();
	m = timer0_millis;
	SREG = oldSREG;

	return m;
#else /*GRSAKURA*/
	return timer0_millis;
#endif/*GRSAKURA*/
}

unsigned long micros() {
#ifndef GRSAKURA
	unsigned long m;
	uint8_t oldSREG = SREG, t;
	
	cli();
	m = timer0_overflow_count;
#if defined(TCNT0)
	t = TCNT0;
#elif defined(TCNT0L)
	t = TCNT0L;
#else
	#error TIMER 0 not defined
#endif

  
#ifdef TIFR0
	if ((TIFR0 & _BV(TOV0)) && (t < 255))
		m++;
#else
	if ((TIFR & _BV(TOV0)) && (t < 255))
		m++;
#endif

	SREG = oldSREG;
	
	return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
#else /*GRSAKURA*/
	bool di = isNoInterrupts();
	noInterrupts();
	unsigned long ms = timer0_millis;
	unsigned long cmcnt = CMT0.CMCNT;
	int ir = IR(CMT0, CMI0);
	if (!di) {
		interrupts();
	}
	if (ir && cmcnt < TicksForMillis / 2) {
		ms++;
	}
	return 1000 * ms + cmcnt / (TicksForMillis / 1000);
#endif/*GRSAKURA*/
}

void delay(unsigned long ms)
{
#ifndef GRSAKURA
	uint16_t start = (uint16_t)micros();

	while (ms > 0) {
		yield();
		if (((uint16_t)micros() - start) >= 1000) {
			ms--;
			start += 1000;
		}
	}
#else /*GRSAKURA*/
	while (ms > 0) {
		const unsigned long msmax = UINT32_MAX / TicksForMillis;
		unsigned long ticks;
		if (ms >= msmax) {
			ticks = msmax * TicksForMillis;
			ms -= msmax;
		} else {
			ticks = ms * TicksForMillis;
			ms = 0;
		}
		delayTicks(ticks);
	}
#endif/*GRSAKURA*/
}

/* Delay for the given number of microseconds.  Assumes a 8 or 16 MHz clock. */
void delayMicroseconds(unsigned int us)
{
#ifndef GRSAKURA
	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);
#if F_CPU >= 20000000L
	// for the 20 MHz clock on rare Arduino boards

	// for a one-microsecond delay, simply wait 2 cycle and return. The overhead
	// of the function call yields a delay of exactly a one microsecond.
	__asm__ __volatile__ (
		"nop" "\n\t"
		"nop"); //just waiting 2 cycle
	if (--us == 0)
		return;

	// the following loop takes a 1/5 of a microsecond (4 cycles)
	// per iteration, so execute it five times for each microsecond of
	// delay requested.
	us = (us<<2) + us; // x5 us

	// account for the time taken in the preceeding commands.
	us -= 2;

#elif F_CPU >= 16000000L
	// for the 16 MHz clock on most Arduino boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call yields a delay of approximately 1 1/8 us.
	if (--us == 0)
		return;

	// the following loop takes a quarter of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2;

	// account for the time taken in the preceeding commands.
	us -= 2;
#else
	// for the 8 MHz internal clock on the ATmega168

	// for a one- or two-microsecond delay, simply return.  the overhead of
	// the function calls takes more than two microseconds.  can't just
	// subtract two, since us is unsigned; we'd overflow.
	if (--us == 0)
		return;
	if (--us == 0)
		return;

	// the following loop takes half of a microsecond (4 cycles)
	// per iteration, so execute it twice for each microsecond of
	// delay requested.
	us <<= 1;
    
	// partially compensate for the time taken by the preceeding commands.
	// we can't subtract any more than this or we'd overflow w/ small delays.
	us--;
#endif

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);
#else /*GRSAKURA*/
	while (us > 0) {
		const unsigned long TicksForMicros = TicksForMillis / 1000;
		const unsigned long usmax = UINT32_MAX / TicksForMicros;
		unsigned long ticks;
		if (us >= usmax) {
			ticks = usmax * TicksForMicros;
			us -= usmax;
		} else {
			ticks = us * TicksForMicros;
			us = 0;
		}
		delayTicks(ticks);
	}
#endif/*GRSAKURA*/
}

void init()
{
#ifndef GRSAKURA
	// this needs to be called before setup() or some functions won't
	// work there
	sei();
	
	// on the ATmega168, timer 0 is also used for fast hardware pwm
	// (using phase-correct PWM would mean that timer 0 overflowed half as often
	// resulting in different millis() behavior on the ATmega8 and ATmega168)
#if defined(TCCR0A) && defined(WGM01)
	sbi(TCCR0A, WGM01);
	sbi(TCCR0A, WGM00);
#endif  

	// set timer 0 prescale factor to 64
#if defined(__AVR_ATmega128__)
	// CPU specific: different values for the ATmega128
	sbi(TCCR0, CS02);
#elif defined(TCCR0) && defined(CS01) && defined(CS00)
	// this combination is for the standard atmega8
	sbi(TCCR0, CS01);
	sbi(TCCR0, CS00);
#elif defined(TCCR0B) && defined(CS01) && defined(CS00)
	// this combination is for the standard 168/328/1280/2560
	sbi(TCCR0B, CS01);
	sbi(TCCR0B, CS00);
#elif defined(TCCR0A) && defined(CS01) && defined(CS00)
	// this combination is for the __AVR_ATmega645__ series
	sbi(TCCR0A, CS01);
	sbi(TCCR0A, CS00);
#else
	#error Timer 0 prescale factor 64 not set correctly
#endif

	// enable timer 0 overflow interrupt
#if defined(TIMSK) && defined(TOIE0)
	sbi(TIMSK, TOIE0);
#elif defined(TIMSK0) && defined(TOIE0)
	sbi(TIMSK0, TOIE0);
#else
	#error	Timer 0 overflow interrupt not set correctly
#endif

	// timers 1 and 2 are used for phase-correct hardware pwm
	// this is better for motors as it ensures an even waveform
	// note, however, that fast pwm mode can achieve a frequency of up
	// 8 MHz (with a 16 MHz clock) at 50% duty cycle

#if defined(TCCR1B) && defined(CS11) && defined(CS10)
	TCCR1B = 0;

	// set timer 1 prescale factor to 64
	sbi(TCCR1B, CS11);
#if F_CPU >= 8000000L
	sbi(TCCR1B, CS10);
#endif
#elif defined(TCCR1) && defined(CS11) && defined(CS10)
	sbi(TCCR1, CS11);
#if F_CPU >= 8000000L
	sbi(TCCR1, CS10);
#endif
#endif
	// put timer 1 in 8-bit phase correct pwm mode
#if defined(TCCR1A) && defined(WGM10)
	sbi(TCCR1A, WGM10);
#elif defined(TCCR1)
	#warning this needs to be finished
#endif

	// set timer 2 prescale factor to 64
#if defined(TCCR2) && defined(CS22)
	sbi(TCCR2, CS22);
#elif defined(TCCR2B) && defined(CS22)
	sbi(TCCR2B, CS22);
#else
	#warning Timer 2 not finished (may not be present on this CPU)
#endif

	// configure timer 2 for phase correct pwm (8-bit)
#if defined(TCCR2) && defined(WGM20)
	sbi(TCCR2, WGM20);
#elif defined(TCCR2A) && defined(WGM20)
	sbi(TCCR2A, WGM20);
#else
	#warning Timer 2 not finished (may not be present on this CPU)
#endif

#if defined(TCCR3B) && defined(CS31) && defined(WGM30)
	sbi(TCCR3B, CS31);		// set timer 3 prescale factor to 64
	sbi(TCCR3B, CS30);
	sbi(TCCR3A, WGM30);		// put timer 3 in 8-bit phase correct pwm mode
#endif

#if defined(TCCR4A) && defined(TCCR4B) && defined(TCCR4D) /* beginning of timer4 block for 32U4 and similar */
	sbi(TCCR4B, CS42);		// set timer4 prescale factor to 64
	sbi(TCCR4B, CS41);
	sbi(TCCR4B, CS40);
	sbi(TCCR4D, WGM40);		// put timer 4 in phase- and frequency-correct PWM mode	
	sbi(TCCR4A, PWM4A);		// enable PWM mode for comparator OCR4A
	sbi(TCCR4C, PWM4D);		// enable PWM mode for comparator OCR4D
#else /* beginning of timer4 block for ATMEGA1280 and ATMEGA2560 */
#if defined(TCCR4B) && defined(CS41) && defined(WGM40)
	sbi(TCCR4B, CS41);		// set timer 4 prescale factor to 64
	sbi(TCCR4B, CS40);
	sbi(TCCR4A, WGM40);		// put timer 4 in 8-bit phase correct pwm mode
#endif
#endif /* end timer4 block for ATMEGA1280/2560 and similar */	

#if defined(TCCR5B) && defined(CS51) && defined(WGM50)
	sbi(TCCR5B, CS51);		// set timer 5 prescale factor to 64
	sbi(TCCR5B, CS50);
	sbi(TCCR5A, WGM50);		// put timer 5 in 8-bit phase correct pwm mode
#endif

#if defined(ADCSRA)
	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// XXX: this will not work properly for other clock speeds, and
	// this code should use F_CPU to determine the prescale factor.
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);

	// enable a2d conversions
	sbi(ADCSRA, ADEN);
#endif

	// the bootloader connects pins 0 and 1 to the USART; disconnect them
	// here so they can be used as normal digital i/o; they will be
	// reconnected in Serial.begin()
#if defined(UCSRB)
	UCSRB = 0;
#elif defined(UCSR0B)
	UCSR0B = 0;
#endif
#else /*GRSAKURA*/
	startModule(MstpIdCMT0);

	CMT.CMSTR0.BIT.STR0 = 0;

	CMT0.CMCNT = 0;
	CMT0.CMCOR = TicksForMillis - 1;

	IPR(CMT0, CMI0) = 1;
	IEN(CMT0, CMI0) = 1;
	IR(CMT0, CMI0) = 0;

	struct st_cmt0_cmcr cmcr;
	cmcr.WORD = 0;
	cmcr.BIT.CKS = 0b00;	// PCLK/8
	cmcr.BIT.CMIE = 1;	// compare match interrupt enable
	cmcr.BIT.b7 = 1;
	CMT0.CMCR.WORD = cmcr.WORD;

	CMT.CMSTR0.BIT.STR0 = 1;

	startModule(MstpIdTPU2);
	TPUA.TSTR.BIT.CST2 = 0;
	IPR(TPU2, TGI2A) = 0x5;
	IEN(TPU2, TGI2A) = 0x1;
	IR(TPU2, TGI2A) = 0x0;
	TPU2.TCR.BIT.TPSC = 0b011;
	TPU2.TCR.BIT.CCLR = 0b001;
	TPU2.TCR.BIT.CKEG = 0b01;
	TPU2.TMDR.BIT.MD = 0b0000;
	TPU2.TSR.BIT.TGFA = 0;
	TPU2.TIER.BIT.TGIEA = 1;
	TPU2.TGRA = 6 - 1;
	TPUA.TSTR.BIT.CST2 = 1;

#if defined(__T4__)
/*CMT1_CMI1 T4Ether 10ms int use*/
    startModule(MstpIdCMT1);
    CMT.CMSTR0.BIT.STR1 = 0;
    CMT1.CMCNT = 0;
    IPR(CMT1, CMI1) = 2;
    IEN(CMT1, CMI1) = 1;
    IR(CMT1, CMI1) = 0;
    /* ---- Initialize CMT1 ---- */             //Ether 10ms int. use
    CMT1.CMCR.WORD = 0x0000;
    CMT1.CMCR.BIT.CKS = 3;                      //clock PCLK/512 = 48MHz/512 = 93,750Hz
    CMT1.CMCR.BIT.CMIE = 1;                     //compair match int. en.
    CMT1.CMCNT = 0x0000;                        //sys.int.use.cnt.
    CMT1.CMCOR = (PCLK/512/100);                //user cnt.set       10msINT=1s/100
    CMT.CMSTR0.BIT.STR1 = 1;
    /* ==== RMII Pins setting ==== */
#if ETH_MODE_SEL == ETH_RMII_MODE
    /*
    Pin Functions : Port
    --------------------
    ET_LINKSTA    : PA5(no use)
    ET_MDIO       : PA3
    ET_MDC        : PA4
    RMII_RXD1     : PB0
    RMII_RXD0     : PB1
    REF50CK       : PB2
    RMII_RX_ER    : PB3
    RMII_TXD_EN   : PB4
    RMII_TXD0     : PB5
    RMII_TXD1     : PB6
    RMII_CRS_DV   : PB7
    ET_WOL        : NC
    ET_EXOUT      : NC
    */

    /* Clear PDR and PMR */
    PORTA.PDR.BIT.B5 = 0;
    PORTA.PDR.BIT.B3 = 0;
    PORTA.PDR.BIT.B4 = 0;
    PORTB.PDR.BIT.B0 = 0;
    PORTB.PDR.BIT.B1 = 0;
    PORTB.PDR.BIT.B2 = 0;
    PORTB.PDR.BIT.B3 = 0;
    PORTB.PDR.BIT.B4 = 0;
    PORTB.PDR.BIT.B5 = 0;
    PORTB.PDR.BIT.B6 = 0;
    PORTB.PDR.BIT.B7 = 0;

    PORTA.PMR.BIT.B5 = 0;
    PORTA.PMR.BIT.B3 = 0;
    PORTA.PMR.BIT.B4 = 0;
    PORTB.PMR.BIT.B0 = 0;
    PORTB.PMR.BIT.B1 = 0;
    PORTB.PMR.BIT.B2 = 0;
    PORTB.PMR.BIT.B3 = 0;
    PORTB.PMR.BIT.B4 = 0;
    PORTB.PMR.BIT.B5 = 0;
    PORTB.PMR.BIT.B6 = 0;
    PORTB.PMR.BIT.B7 = 0;

    /* Write protect off */
    MPC.PWPR.BYTE = 0x00;       /* PWPR.PFSWE write protect off */
    MPC.PWPR.BYTE = 0x40;       /* PFS register write protect off */

    /* Select pin function */
    MPC.PA5PFS.BYTE = 0x11;
    MPC.PA3PFS.BYTE = 0x11;
    MPC.PA4PFS.BYTE = 0x11;
    MPC.PB0PFS.BYTE = 0x12;
    MPC.PB1PFS.BYTE = 0x12;
    MPC.PB2PFS.BYTE = 0x12;
    MPC.PB3PFS.BYTE = 0x12;
    MPC.PB4PFS.BYTE = 0x12;
    MPC.PB5PFS.BYTE = 0x12;
    MPC.PB6PFS.BYTE = 0x12;
    MPC.PB7PFS.BYTE = 0x12;

    /* Write protect on */
//    MPC.PWPR.BYTE = 0x80;       /* PFS register write protect on */

    /* Select ethernet mode */
    MPC.PFENET.BIT.PHYMODE = 0; /* RMII mode */

    /* Switch to the selected input/output function */
    PORTA.PMR.BIT.B5 = 1;
    PORTA.PMR.BIT.B3 = 1;
    PORTA.PMR.BIT.B4 = 1;
    PORTB.PMR.BIT.B0 = 1;
    PORTB.PMR.BIT.B1 = 1;
    PORTB.PMR.BIT.B2 = 1;
    PORTB.PMR.BIT.B3 = 1;
    PORTB.PMR.BIT.B4 = 1;
    PORTB.PMR.BIT.B5 = 1;
    PORTB.PMR.BIT.B6 = 1;
    PORTB.PMR.BIT.B7 = 1;
#endif  /* ETH_MODE_SEL */

#endif  //#if defined(__T4__)

#endif/*GRSAKURA*/
}
