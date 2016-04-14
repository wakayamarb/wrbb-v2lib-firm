/*
  wiring_analog.c - analog input and output
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

  Modified 28 September 2010 by Mark Sproul

  $Id: wiring.c 248 2007-02-03 15:36:30Z mellis $
*/

#include "wiring_private.h"
#include "pins_arduino.h"
#ifdef GRSAKURA
#include "Arduino.h"
#include "utilities.h"
#include "rx63n/interrupt_handlers.h"
#include "rx63n/util.h"
#endif/*GRSAKURA*/

uint8_t analog_reference = DEFAULT;

void analogReference(uint8_t mode)
{
	// can't actually set the register here because the default setting
	// will connect AVCC and the AREF pin, which would cause a short if
	// there's something connected to AREF.
	analog_reference = mode;
}

#ifdef GRSAKURA
static uint8_t analog_read_clock = 0b00; // PCLK/8

void analogReadClock(uint8_t clock)
{
	analog_read_clock = clock;
}
#endif/*GRSAKURA*/

#ifdef GRSAKURA
void setPinModeAnalogRead(int pin)
{
	int an0 = pin - PIN_AN000;
	if (an0 >= 0 && an0 <= 7) {
		BCLR(&PORT4.PDR.BYTE, an0);
		BSET(&PORT4.PMR.BYTE, an0);
	} else if (an0 >= 8 && an0 <= 13) {
		BCLR(&PORTD.PDR.BYTE, an0 - 8);
		BSET(&PORTD.PMR.BYTE, an0 - 8);
	}
	if (an0 >= 0 && an0 <= 7) {
		assignPinFunction(PIN_IO14 + an0, 0, 0, 1);
	} else if (an0 >= 8 && an0 <= 13) {
		assignPinFunction(PIN_IO36 + an0 - 8, 0, 0, 1);
	}
}

void resetPinModeAnalogRead(int pin)
{
	(void)pin;
}
#endif/*GRSAKURA*/

int analogRead(uint8_t pin)
{
#ifndef GRSAKURA
	uint8_t low, high;

#if defined(analogPinToChannel)
#if defined(__AVR_ATmega32U4__)
	if (pin >= 18) pin -= 18; // allow for channel or pin numbers
#endif
	pin = analogPinToChannel(pin);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	if (pin >= 54) pin -= 54; // allow for channel or pin numbers
#elif defined(__AVR_ATmega32U4__)
	if (pin >= 18) pin -= 18; // allow for channel or pin numbers
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
	if (pin >= 24) pin -= 24; // allow for channel or pin numbers
#else
	if (pin >= 14) pin -= 14; // allow for channel or pin numbers
#endif

#if defined(ADCSRB) && defined(MUX5)
	// the MUX5 bit of ADCSRB selects whether we're reading from channels
	// 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
	ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
#endif
  
	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
#if defined(ADMUX)
	ADMUX = (analog_reference << 6) | (pin & 0x07);
#endif

	// without a delay, we seem to read from the wrong channel
	//delay(1);

#if defined(ADCSRA) && defined(ADCL)
	// start the conversion
	sbi(ADCSRA, ADSC);

	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low  = ADCL;
	high = ADCH;
#else
	// we dont have an ADC, return 0
	low  = 0;
	high = 0;
#endif

	// combine the two bytes
	return (high << 8) | low;
#else /*GRSAKURA*/
	volatile uint16_t* adcdr = NULL;

	startModule(MstpIdS12AD);
    if (pin < 14) pin += 14; // allow for channel or pin numbers

	if (pin >= PIN_AN000 && pin <= PIN_AN013) {
		int an0 = pin - PIN_AN000;
		setPinMode(pin, PinModeAnalogRead);
		S12AD.ADEXICR.BIT.TSS = 0;
		S12AD.ADEXICR.BIT.OCS = 0;
		S12AD.ADANS0.WORD = 1 << an0;
		S12AD.ADANS1.WORD = 0;
		adcdr = (volatile uint16_t*)&S12AD.ADDR0 + an0;
	} else if (pin == PIN_ANINT) {
		S12AD.ADEXICR.BIT.TSS = 1;
		S12AD.ADEXICR.BIT.OCS = 0;
		S12AD.ADEXICR.BIT.TSSAD = 0;
		S12AD.ADANS0.WORD = 0;
		S12AD.ADANS1.WORD = 0;
		adcdr = &S12AD.ADOCDR;
	} else if (pin == PIN_ANTMP) {
		S12AD.ADEXICR.BIT.TSS = 0;
		S12AD.ADEXICR.BIT.OCS = 1;
		S12AD.ADEXICR.BIT.OCSAD = 0;
		S12AD.ADANS0.WORD = 0;
		S12AD.ADANS1.WORD = 0;
		adcdr = &S12AD.ADTSDR;
	}

	S12AD.ADCSR.BYTE = 0x00;
	S12AD.ADCSR.BIT.CKS = analog_read_clock;
	S12AD.ADADC.BIT.ADC = 0b00;
	S12AD.ADCER.BIT.ADRFMT = 0;
	S12AD.ADCER.BIT.ACE = 0;

	S12AD.ADCSR.BIT.ADST = 1;
	while (S12AD.ADCSR.BIT.ADST) {
		;
	}

	int val = *adcdr & 0x0fff;
	switch (analog_reference) {
	case DEFAULT:
		val = val * (1024 * 33) / (4096 * 50);
		break;
	case INTERNAL:
		val = val * (1024 * 33) / (4096 * 11);
		if (val > 1023) {
			val = 1023;
		}
		break;
	case EXTERNAL:
		val = val * 1024 / 4096;
		break;
	case RAW12BIT:
		break;
	}
	return val;
#endif/*GRSAKURA*/
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite(uint8_t pin, int val)
{
#ifndef GRSAKURA
	// We need to make sure the PWM output is enabled for those pins
	// that support it, as we turn it off when digitally reading or
	// writing with them.  Also, make sure the pin is in output mode
	// for consistenty with Wiring, which doesn't require a pinMode
	// call for the analog output pins.
	pinMode(pin, OUTPUT);
	if (val == 0)
	{
		digitalWrite(pin, LOW);
	}
	else if (val == 255)
	{
		digitalWrite(pin, HIGH);
	}
	else
	{
		switch(digitalPinToTimer(pin))
		{
			// XXX fix needed for atmega8
			#if defined(TCCR0) && defined(COM00) && !defined(__AVR_ATmega8__)
			case TIMER0A:
				// connect pwm to pin on timer 0
				sbi(TCCR0, COM00);
				OCR0 = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR0A) && defined(COM0A1)
			case TIMER0A:
				// connect pwm to pin on timer 0, channel A
				sbi(TCCR0A, COM0A1);
				OCR0A = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR0A) && defined(COM0B1)
			case TIMER0B:
				// connect pwm to pin on timer 0, channel B
				sbi(TCCR0A, COM0B1);
				OCR0B = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR1A) && defined(COM1A1)
			case TIMER1A:
				// connect pwm to pin on timer 1, channel A
				sbi(TCCR1A, COM1A1);
				OCR1A = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR1A) && defined(COM1B1)
			case TIMER1B:
				// connect pwm to pin on timer 1, channel B
				sbi(TCCR1A, COM1B1);
				OCR1B = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR2) && defined(COM21)
			case TIMER2:
				// connect pwm to pin on timer 2
				sbi(TCCR2, COM21);
				OCR2 = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR2A) && defined(COM2A1)
			case TIMER2A:
				// connect pwm to pin on timer 2, channel A
				sbi(TCCR2A, COM2A1);
				OCR2A = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR2A) && defined(COM2B1)
			case TIMER2B:
				// connect pwm to pin on timer 2, channel B
				sbi(TCCR2A, COM2B1);
				OCR2B = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR3A) && defined(COM3A1)
			case TIMER3A:
				// connect pwm to pin on timer 3, channel A
				sbi(TCCR3A, COM3A1);
				OCR3A = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR3A) && defined(COM3B1)
			case TIMER3B:
				// connect pwm to pin on timer 3, channel B
				sbi(TCCR3A, COM3B1);
				OCR3B = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR3A) && defined(COM3C1)
			case TIMER3C:
				// connect pwm to pin on timer 3, channel C
				sbi(TCCR3A, COM3C1);
				OCR3C = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR4A)
			case TIMER4A:
				//connect pwm to pin on timer 4, channel A
				sbi(TCCR4A, COM4A1);
				#if defined(COM4A0)		// only used on 32U4
				cbi(TCCR4A, COM4A0);
				#endif
				OCR4A = val;	// set pwm duty
				break;
			#endif
			
			#if defined(TCCR4A) && defined(COM4B1)
			case TIMER4B:
				// connect pwm to pin on timer 4, channel B
				sbi(TCCR4A, COM4B1);
				OCR4B = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR4A) && defined(COM4C1)
			case TIMER4C:
				// connect pwm to pin on timer 4, channel C
				sbi(TCCR4A, COM4C1);
				OCR4C = val; // set pwm duty
				break;
			#endif
				
			#if defined(TCCR4C) && defined(COM4D1)
			case TIMER4D:				
				// connect pwm to pin on timer 4, channel D
				sbi(TCCR4C, COM4D1);
				#if defined(COM4D0)		// only used on 32U4
				cbi(TCCR4C, COM4D0);
				#endif
				OCR4D = val;	// set pwm duty
				break;
			#endif

							
			#if defined(TCCR5A) && defined(COM5A1)
			case TIMER5A:
				// connect pwm to pin on timer 5, channel A
				sbi(TCCR5A, COM5A1);
				OCR5A = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR5A) && defined(COM5B1)
			case TIMER5B:
				// connect pwm to pin on timer 5, channel B
				sbi(TCCR5A, COM5B1);
				OCR5B = val; // set pwm duty
				break;
			#endif

			#if defined(TCCR5A) && defined(COM5C1)
			case TIMER5C:
				// connect pwm to pin on timer 5, channel C
				sbi(TCCR5A, COM5C1);
				OCR5C = val; // set pwm duty
				break;
			#endif

			case NOT_ON_TIMER:
			default:
				if (val < 128) {
					digitalWrite(pin, LOW);
				} else {
					digitalWrite(pin, HIGH);
				}
		}
	}
#else /*GRSAKURA*/
	bool hardPwm = isHardwarePWMPin(pin);
	int period;
	int term;
	if (hardPwm) {
		period = 255 * HardwarePWMFreq / SoftwarePWMFreq;
		term = val * HardwarePWMFreq / SoftwarePWMFreq;
	} else {
		period = 255;
		term = val;
	}
	if (getPinMode(pin) != PinModeAnalogWrite) {
		setPinMode(pin, PinModeAnalogWrite);
		if (hardPwm) {
			setPinModeHardwarePWM(pin, period, term, 0);
		} else {
			setPinModeSoftwarePWM(pin, period, term, 0);
		}
	} else {
		if (hardPwm) {
			changePinModeHardwarePWM(pin, period, term, 0);
		} else {
			changePinModeSoftwarePWM(pin, period, term, 0);
		}
	}
#endif/*GRSAKURA*/
}

#ifdef GRSAKURA
/*
void setPinModeAnalogWrite(int pin)
{
}
*/

void resetPinModeAnalogWrite(int pin)
{
	if (isHardwarePWMPin(pin)) {
		resetPinModeHardwarePWM(pin);
	} else {
		resetPinModeSoftwarePWM(pin);
	}
}
#endif/*GRSAKURA*/

#ifdef GRSAKURA
void analogWriteDAC(int port, int val)
{
	int pin = PIN_IO53;
	if (getPinMode(pin) != PinModeDac) {
		setPinMode(pin, PinModeDac);
	}
	DA.DADR0 = 0;
	DA.DADR1 = val;
	DA.DACR.BIT.DAE = 0;
	DA.DACR.BIT.DAOE0 = 0;
	DA.DACR.BIT.DAOE1 = 1;
	DA.DADPR.BIT.DPSEL = 0;
	DA.DAADSCR.BIT.DAADST = 0;
}

void setPinModeDac(int pin)
{
	if (pin == PIN_IO53) {
		int port = digitalPinToPort(pin);
		int bit = digitalPinToBit(pin);
		startModule(MstpIdDA);
		BCLR(portModeRegister(port), bit);
		BCLR(portDirectionRegister(port), bit);
		assignPinFunction(pin, 0, 0, 1);
	}
}

void resetPinModeDac(int pin)
{
	if (pin == PIN_IO53) {
		stopModule(MstpIdDA);
		pinMode(pin, INPUT);
	}
}
#endif/*GRSAKURA*/
