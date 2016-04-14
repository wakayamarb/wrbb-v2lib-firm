/*
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

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
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  Modified 3 December 2013 by Matthijs Kooijman
  Modified 13 July 2014 by Nozomu Fujita for GR-SAKURA
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

#include "HardwareSerial.h"
#include "HardwareSerial_private.h"

#ifdef GRSAKURA
#include "utilities.h"
#include "usb_hal.h"
#include "usb_cdc.h"
#include "rx63n/util.h"
/// For USB receive /////////////////////////////
/// The definitions are copied from usb_hal.c ///
#include "rx63n/iodefine.h"
/*NOTE USB0 is defined in iodefine.h file*/
#define USBIO USB0
#define PID_BUF     1
/////////////////////////////////////////////

struct SciInterruptRegistersTableStruct {
  uint8_t _txier;
  uint8_t _txien;
  uint8_t _rxier;
  uint8_t _rxien;
  uint8_t _txir;
  uint8_t _rxir;
  uint8_t _txipr;
  uint8_t _rxipr;
};

static const SciInterruptRegistersTableStruct SciInterruptRegistersTable[] = {
  {IER_SCI0_TXI0, 7, IER_SCI0_RXI0, 6, IR_SCI0_TXI0, IR_SCI0_RXI0, IPR_SCI0_TXI0, IPR_SCI0_RXI0},
  {IER_SCI6_TXI6, 1, IER_SCI6_RXI6, 0, IR_SCI6_TXI6, IR_SCI6_RXI6, IPR_SCI6_TXI6, IPR_SCI6_RXI6},
  {IER_SCI2_TXI2, 5, IER_SCI2_RXI2, 4, IR_SCI2_TXI2, IR_SCI2_RXI2, IPR_SCI2_TXI2, IPR_SCI2_RXI2},
  {IER_SCI3_TXI3, 0, IER_SCI3_RXI3, 7, IR_SCI3_TXI3, IR_SCI3_RXI3, IPR_SCI3_TXI3, IPR_SCI3_RXI3},
  {IER_SCI5_TXI5, 6, IER_SCI5_RXI5, 5, IR_SCI5_TXI5, IR_SCI5_RXI5, IPR_SCI5_TXI5, IPR_SCI5_RXI5},
  {IER_SCI8_TXI8, 7, IER_SCI8_RXI8, 6, IR_SCI8_TXI8, IR_SCI8_RXI8, IPR_SCI8_TXI8, IPR_SCI8_RXI8},
  {IER_SCI1_TXI1, 2, IER_SCI1_RXI1, 1, IR_SCI1_TXI1, IR_SCI1_RXI1, IPR_SCI1_TXI1, IPR_SCI1_RXI1},
};
#endif/*GRSAKURA*/

// this next line disables the entire HardwareSerial.cpp, 
// this is so I can support Attiny series and any other chip without a uart
#if defined(HAVE_HWSERIAL0) || defined(HAVE_HWSERIAL1) || defined(HAVE_HWSERIAL2) || defined(HAVE_HWSERIAL3) || defined(HAVE_HWSERIAL4) || defined(HAVE_HWSERIAL5) || defined(HAVE_HWSERIAL6) || defined(HAVE_HWSERIAL7)

// SerialEvent functions are weak, so when the user doesn't define them,
// the linker just sets their address to 0 (which is checked below).
// The Serialx_available is just a wrapper around Serialx.available(),
// but we can refer to it weakly so we don't pull in the entire
// HardwareSerial instance if the user doesn't also refer to it.
#if defined(HAVE_HWSERIAL0)
  void serialEvent() __attribute__((weak));
  bool Serial0_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL1)
  void serialEvent1() __attribute__((weak));
  bool Serial1_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL2)
  void serialEvent2() __attribute__((weak));
  bool Serial2_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL3)
  void serialEvent3() __attribute__((weak));
  bool Serial3_available() __attribute__((weak));
#endif

#ifdef GRSAKURA
#if defined(HAVE_HWSERIAL4)
  void serialEvent4() __attribute__((weak));
  bool Serial4_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL5)
  void serialEvent5() __attribute__((weak));
  bool Serial5_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL6)
  void serialEvent6() __attribute__((weak));
  bool Serial6_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL7)
  void serialEvent7() __attribute__((weak));
  bool Serial7_available() __attribute__((weak));
#endif
#endif/*GRSAKURA*/

void serialEventRun(void)
{
#if defined(HAVE_HWSERIAL0)
  if (Serial0_available && serialEvent && Serial0_available()) serialEvent();
#endif
#if defined(HAVE_HWSERIAL1)
  if (Serial1_available && serialEvent1 && Serial1_available()) serialEvent1();
#endif
#if defined(HAVE_HWSERIAL2)
  if (Serial2_available && serialEvent2 && Serial2_available()) serialEvent2();
#endif
#if defined(HAVE_HWSERIAL3)
  if (Serial3_available && serialEvent3 && Serial3_available()) serialEvent3();
#endif
#ifdef GRSAKURA
#if defined(HAVE_HWSERIAL4)
  if (Serial4_available && serialEvent4 && Serial4_available()) serialEvent4();
#endif
#if defined(HAVE_HWSERIAL5)
  if (Serial5_available && serialEvent5 && Serial5_available()) serialEvent5();
#endif
#if defined(HAVE_HWSERIAL6)
  if (Serial6_available && serialEvent6 && Serial6_available()) serialEvent6();
#endif
#if defined(HAVE_HWSERIAL7)
  if (Serial7_available && serialEvent7 && Serial7_available()) serialEvent7();
#endif
#endif/*GRSAKURA*/
}

// Actual interrupt handlers //////////////////////////////////////////////////////////////

void HardwareSerial::_tx_udr_empty_irq(void)
{
#ifndef GRSAKURA
  // If interrupts are enabled, there must be more data in the output
  // buffer. Send the next byte
  unsigned char c = _tx_buffer[_tx_buffer_tail];
  _tx_buffer_tail = (_tx_buffer_tail + 1) % SERIAL_BUFFER_SIZE;

  *_udr = c;

  // clear the TXC bit -- "can be cleared by writing a one to its bit
  // location". This makes sure flush() won't return until the bytes
  // actually got written
  sbi(*_ucsra, TXC0);

  if (_tx_buffer_head == _tx_buffer_tail) {
    // Buffer empty, so disable interrupts
    cbi(*_ucsrb, UDRIE0);
  }
#else /*GRSAKURA*/
  if (_tx_buffer_head != _tx_buffer_tail) {
    _sci->TDR = _tx_buffer[_tx_buffer_tail];
    _tx_buffer_tail = (_tx_buffer_tail + 1) % SERIAL_BUFFER_SIZE;
  } else {
    _sending = false;
  }
#endif/*GRSAKURA*/
}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(unsigned long baud, byte config)
{
#ifndef GRSAKURA
  // Try u2x mode first
  uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
  *_ucsra = 1 << U2X0;

  // hardcoded exception for 57600 for compatibility with the bootloader
  // shipped with the Duemilanove and previous boards and the firmware
  // on the 8U2 on the Uno and Mega 2560. Also, The baud_setting cannot
  // be > 4095, so switch back to non-u2x mode if the baud rate is too
  // low.
  if (((F_CPU == 16000000UL) && (baud == 57600)) || (baud_setting >4095))
  {
    *_ucsra = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  _written = false;

  //set the data bits, parity, and stop bits
#if defined(__AVR_ATmega8__)
  config |= 0x80; // select UCSRC register (shared with UBRRH)
#endif
  *_ucsrc = config;
  
  sbi(*_ucsrb, RXEN0);
  sbi(*_ucsrb, TXEN0);
  sbi(*_ucsrb, RXCIE0);
  cbi(*_ucsrb, UDRIE0);
#else /*GRSAKURA*/
  switch (_serial_channel) {
#if defined(HAVE_HWSERIAL0)
  case 0:
        {
        USB_ERR err = USBCDC_Init();
        if (err == USB_ERR_OK) {
            bool isConnected = false;
            const unsigned long TimeOut = 3000;
            unsigned long start = millis();
            while ((millis() - start) < TimeOut) {
                if (USBCDC_IsConnected()) {
                    isConnected = true;
                    break;
                }
            }
            if (isConnected) {
//                USBCDC_Read_Async(SERIAL_BUFFER_SIZE, g_Buffer, CBDoneRead);
            } else {
                USBCDC_Cancel();
            }
        }
    }
        break;
#endif
#if defined(HAVE_HWSERIAL1)
  case 1:
#endif
#if defined(HAVE_HWSERIAL2)
  case 2:
#endif
#if defined(HAVE_HWSERIAL3)
  case 3:
#endif
#if defined(HAVE_HWSERIAL4)
  case 4:
#endif
#if defined(HAVE_HWSERIAL5)
  case 5:
#endif
#if defined(HAVE_HWSERIAL6)
  case 6:
#endif
#if defined(HAVE_HWSERIAL7)
  case 7:
#endif
#if defined(HAVE_HWSERIAL1) || defined(HAVE_HWSERIAL2) || defined(HAVE_HWSERIAL3) || defined(HAVE_HWSERIAL4) || defined(HAVE_HWSERIAL5) || defined(HAVE_HWSERIAL6) || defined(HAVE_HWSERIAL7)
    {
      pinMode(_txpin, OUTPUT);
      digitalWrite(_txpin, HIGH);
      pinMode(_rxpin, INPUT);

      startModule(_module);

      _sci->SCR.BIT.CKE = 0;
      _sci->SCR.BIT.TEIE = 0;
      _sci->SCR.BIT.MPIE = 0;
      _sci->SCR.BIT.RE = 0;
      _sci->SCR.BIT.TE = 0;
      _sci->SCR.BIT.RIE = 0;
      _sci->SCR.BIT.TIE = 0;
      _sci->SCMR.BIT.SMIF = 0;
      _sci->SCMR.BIT.SINV = 0;
      _sci->SCMR.BIT.SDIR = 0;
      _sci->SMR.BIT.MP = 0;
      _sci->SMR.BIT.CM = 0;

      {
        const uint8_t SERIAL_DM = 0b00000111;
        const uint8_t SERIAL_D8 = 0b00000110;
        const uint8_t SERIAL_D7 = 0b00000100;
    //  const uint8_t SERIAL_D6 = 0b00000010;
    //  const uint8_t SERIAL_D5 = 0b00000000;
        const uint8_t SERIAL_PM = 0b00110000;
        const uint8_t SERIAL_PN = 0b00000000;
        const uint8_t SERIAL_PE = 0b00100000;
        const uint8_t SERIAL_PO = 0b00110000;
        const uint8_t SERIAL_SM = 0b00001000;
        const uint8_t SERIAL_S1 = 0b00000000;
        const uint8_t SERIAL_S2 = 0b00001000;

        _sci->SMR.BIT.CHR = 0;
        _sci->SMR.BIT.PE = 0;
        _sci->SMR.BIT.PM = 0;
        _sci->SMR.BIT.STOP = 0;
        switch (config & SERIAL_DM) {
        case SERIAL_D8:
          _sci->SMR.BIT.CHR = 0;
          break;
        case SERIAL_D7:
          _sci->SMR.BIT.CHR = 1;
          break;
        default:
          break;
        }
        switch (config & SERIAL_PM) {
        case SERIAL_PN:
          _sci->SMR.BIT.PE = 0;
          break;
        case SERIAL_PE:
          _sci->SMR.BIT.PE = 1;
          _sci->SMR.BIT.PM = 0;
          break;
        case SERIAL_PO:
          _sci->SMR.BIT.PE = 1;
          _sci->SMR.BIT.PM = 1;
          break;
        default:
          break;
        }
        switch (config & SERIAL_SM) {
        case SERIAL_S1:
          _sci->SMR.BIT.STOP = 0;
          break;
        case SERIAL_S2:
          _sci->SMR.BIT.STOP = 1;
          break;
        default:
          break;
        }
      }

      if (baud <= (PCLK / ((16 << 7) * 256)) || baud > (PCLK / 16)) {
        baud = 9600;
      }
      {
        int n;
        int abcs;
        int N;
        for (int i = 0; i < 8; i++) {
          n = i / 2;
          abcs = (i & 1) ? 0 : 1;
          N = (2 * PCLK / ((16 << i) * baud) + 1) / 2 - 1;
          if (N >= 0 && N <= 255) {
            _sci->SMR.BIT.CKS = n;
            _sci->SEMR.BIT.ABCS = abcs;
            _sci->BRR = N;
            break;
          }
        }
      }

      {
        st_sci0_ssr ssr;
        ssr.BYTE = _sci->SSR.BYTE;
        ssr.BIT.ORER = 0;
        ssr.BIT.FER = 0;
        ssr.BIT.PER = 0;
        ssr.BIT.b7_6 = 0b11;
        _sci->SSR.BYTE = ssr.BYTE;
      }

      {
        const SciInterruptRegistersTableStruct* t = &SciInterruptRegistersTable[_serial_channel - 1];
        ICU.IPR[t->_txipr].BIT.IPR = 2;
        ICU.IPR[t->_rxipr].BIT.IPR = 2;

        BSET(&ICU.IER[t->_txier].BYTE, t->_txien);
        BSET(&ICU.IER[t->_rxier].BYTE, t->_rxien);

        ICU.IR[t->_txir].BIT.IR = 0x0;
        ICU.IR[t->_rxir].BIT.IR = 0x0;

        _sci->SCR.BIT.TIE = 1;
        _sci->SCR.BIT.RIE = 1;
        _sci->SCR.BIT.TE = 1;
        _sci->SCR.BIT.RE = 1;
      }

      {
        int txPort = digitalPinToPort(_txpin);
        int rxPort = digitalPinToPort(_rxpin);
        int txBit = digitalPinToBit(_txpin);
        int rxBit = digitalPinToBit(_rxpin);

        assignPinFunction(_txpin, 0b01010, 0, 0);
        assignPinFunction(_rxpin, 0b01010, 0, 0);

        BSET(portModeRegister(txPort), txBit);
        BSET(portModeRegister(rxPort), rxBit);
      }
      _begin = true;
    }
    break;
#endif
  default:
    break;
  }
#endif/*GRSAKURA*/
}

void HardwareSerial::end()
{
#ifndef GRSAKURA
  // wait for transmission of outgoing data
  while (_tx_buffer_head != _tx_buffer_tail)
    ;

  cbi(*_ucsrb, RXEN0);
  cbi(*_ucsrb, TXEN0);
  cbi(*_ucsrb, RXCIE0);
  cbi(*_ucsrb, UDRIE0);
  
  // clear any received data
  _rx_buffer_head = _rx_buffer_tail;
#else /*GRSAKURA*/
  switch (_serial_channel) {
#if defined(HAVE_HWSERIAL0)
  case 0:
    {
      USBCDC_Cancel();
    }
    break;
#endif
#if defined(HAVE_HWSERIAL1)
  case 1:
#endif
#if defined(HAVE_HWSERIAL2)
  case 2:
#endif
#if defined(HAVE_HWSERIAL3)
  case 3:
#endif
#if defined(HAVE_HWSERIAL4)
  case 4:
#endif
#if defined(HAVE_HWSERIAL5)
  case 5:
#endif
#if defined(HAVE_HWSERIAL6)
  case 6:
#endif
#if defined(HAVE_HWSERIAL7)
  case 7:
#endif
#if defined(HAVE_HWSERIAL1) || defined(HAVE_HWSERIAL2) || defined(HAVE_HWSERIAL3) || defined(HAVE_HWSERIAL4) || defined(HAVE_HWSERIAL5) || defined(HAVE_HWSERIAL6) || defined(HAVE_HWSERIAL7)
    {
      flush();
      {
        const SciInterruptRegistersTableStruct* t = &SciInterruptRegistersTable[_serial_channel - 1];
        ICU.IR[t->_txir].BIT.IR = 0x0;
        ICU.IR[t->_rxir].BIT.IR = 0x0;

        BCLR(&ICU.IER[t->_txier].BYTE, t->_txien);
        BCLR(&ICU.IER[t->_rxier].BYTE, t->_rxien);

        ICU.IPR[t->_txipr].BIT.IPR = 0;
        ICU.IPR[t->_rxipr].BIT.IPR = 0;
      }

      stopModule(_module);
      _begin = false;
    }
    break;
#endif
  default:
    break;
  }
#endif/*GRSAKURA*/
}

int HardwareSerial::available(void)
{
  return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail) % SERIAL_BUFFER_SIZE;
}

int HardwareSerial::peek(void)
{
  if (_rx_buffer_head == _rx_buffer_tail) {
    return -1;
  } else {
    return _rx_buffer[_rx_buffer_tail];
  }
}

int HardwareSerial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer_head == _rx_buffer_tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer[_rx_buffer_tail];
#if SERIAL_BUFFER_SIZE < 256
	_rx_buffer_tail = (uint8_t)(_rx_buffer_tail + 1) % SERIAL_BUFFER_SIZE;
#else
	_rx_buffer_tail = (uint32_t)(_rx_buffer_tail + 1) % SERIAL_BUFFER_SIZE;
#endif
    return c;
  }
}

void HardwareSerial::flush()
{
#ifndef GRSAKURA
  // If we have never written a byte, no need to flush. This special
  // case is needed since there is no way to force the TXC (transmit
  // complete) bit to 1 during initialization
  if (!_written)
    return;

  while (bit_is_set(*_ucsrb, UDRIE0) || bit_is_clear(*_ucsra, TXC0)) {
    if (bit_is_clear(SREG, SREG_I) && bit_is_set(*_ucsrb, UDRIE0))
	// Interrupts are globally disabled, but the DR empty
	// interrupt should be enabled, so poll the DR empty flag to
	// prevent deadlock
	if (bit_is_set(*_ucsra, UDRE0))
	  _tx_udr_empty_irq();
  }
  // If we get here, nothing is queued anymore (DRIE is disabled) and
  // the hardware finished tranmission (TXC is set).
#else /*GRSAKURA*/
  while (_tx_buffer_head != _tx_buffer_tail) {
    ;
  }
  while (_sending) {
    ;
  }
#endif/*GRSAKURA*/
}

size_t HardwareSerial::write(uint8_t c)
{
#ifndef GRSAKURA
  // If the buffer and the data register is empty, just write the byte
  // to the data register and be done. This shortcut helps
  // significantly improve the effective datarate at high (>
  // 500kbit/s) bitrates, where interrupt overhead becomes a slowdown.
  if (_tx_buffer_head == _tx_buffer_tail && bit_is_set(*_ucsra, UDRE0)) {
    *_udr = c;
    sbi(*_ucsra, TXC0);
    return 1;
  }
  uint8_t i = (_tx_buffer_head + 1) % SERIAL_BUFFER_SIZE;
	
  // If the output buffer is full, there's nothing for it other than to 
  // wait for the interrupt handler to empty it a bit
  while (i == _tx_buffer_tail) {
    if (bit_is_clear(SREG, SREG_I)) {
      // Interrupts are disabled, so we'll have to poll the data
      // register empty flag ourselves. If it is set, pretend an
      // interrupt has happened and call the handler to free up
      // space for us.
      if(bit_is_set(*_ucsra, UDRE0))
	_tx_udr_empty_irq();
    } else {
      // nop, the interrupt handler will free up space for us
    }
  }

  _tx_buffer[_tx_buffer_head] = c;
  _tx_buffer_head = i;
	
  sbi(*_ucsrb, UDRIE0);
  _written = true;
  
  return 1;
#else /*GRSAKURA*/
  switch (_serial_channel) {
#if defined(HAVE_HWSERIAL0)
  case 0:
    {
      USBCDC_PutChar(c);
      return 1;
    }
#endif
#if defined(HAVE_HWSERIAL1)
  case 1:
#endif
#if defined(HAVE_HWSERIAL2)
  case 2:
#endif
#if defined(HAVE_HWSERIAL3)
  case 3:
#endif
#if defined(HAVE_HWSERIAL4)
  case 4:
#endif
#if defined(HAVE_HWSERIAL5)
  case 5:
#endif
#if defined(HAVE_HWSERIAL6)
  case 6:
#endif
#if defined(HAVE_HWSERIAL7)
  case 7:
#endif
#if defined(HAVE_HWSERIAL1) || defined(HAVE_HWSERIAL2) || defined(HAVE_HWSERIAL3) || defined(HAVE_HWSERIAL4) || defined(HAVE_HWSERIAL5) || defined(HAVE_HWSERIAL6) || defined(HAVE_HWSERIAL7)
    {
      unsigned int i = (_tx_buffer_head + 1) % SERIAL_BUFFER_SIZE;
      // If the output buffer is full, there's nothing for it other than to
      // wait for the interrupt handler to empty it a bit
      // ???: return 0 here instead?
      if (_begin) {
        while (i == _tx_buffer_tail) {
          ;
        }
        _tx_buffer[_tx_buffer_head] = c;
        _tx_buffer_head = i;

        if (!_sending) {
          bool di = isNoInterrupts();
          noInterrupts();
          _sending = true;
          _tx_udr_empty_irq();
          if (!di) {
            interrupts();
          }
        }
      } else {
        if (i != _tx_buffer_tail) {
          _tx_buffer[_tx_buffer_head] = c;
          _tx_buffer_head = i;
        }
      }
      return 1;
    }
#endif
  default:
    return 0;
  }
#endif/*GRSAKURA*/
}


#endif // whole file

#ifdef GRSAKURA
#include "rx63n/interrupt_handlers.h"

#ifdef HAVE_HWSERIAL0
extern "C"{
void ReadBulkOUTPacket(void)
{
    uint16_t DataLength = 0;

    /*Read data using D1FIFO*/
    /*NOTE: This probably will have already been selected if using BRDY interrupt.*/
    do{
    USBIO.D1FIFOSEL.BIT.CURPIPE = PIPE_BULK_OUT;
    }while(USBIO.D1FIFOSEL.BIT.CURPIPE != PIPE_BULK_OUT);

    /*Set PID to BUF*/
    USBIO.PIPE1CTR.BIT.PID = PID_BUF;

    /*Wait for buffer to be ready*/
    while(USBIO.D1FIFOCTR.BIT.FRDY == 0){;}

    /*Set Read Count Mode - so DTLN count will decrement as data read from buffer*/
    USBIO.D1FIFOSEL.BIT.RCNT = 1;

    /*Read length of data */
    DataLength = USBIO.D1FIFOCTR.BIT.DTLN;

    while(DataLength != 0){
        /*Read from the FIFO*/
        uint16_t Data = USBIO.D1FIFO;
        if(DataLength >= 2){
            /*Save first byte*/
            Serial._store_char((uint8_t)Data);
            /*Save second byte*/
            Serial._store_char((uint8_t)(Data>>8));
            DataLength-=2;
        } else {
            Serial._store_char((uint8_t)Data);
            DataLength--;
        }
    }

}
} // extern C

HardwareSerial Serial(0, NULL, MstpIdINVALID, INVALID_IO, INVALID_IO);
#endif/*HAVE_HWSERIAL0*/

#ifdef HAVE_HWSERIAL1
void INT_Excep_SCI0_RXI0()
{
  Serial1._rx_complete_irq();
}

void INT_Excep_SCI0_TXI0()
{
  Serial1._tx_udr_empty_irq();
}

bool Serial1_available() {
  return Serial1.available();
}

HardwareSerial Serial1(1, &SCI0, MstpIdSCI0, PIN_IO1, PIN_IO0);
#endif/*HAVE_HWSERIAL1*/

#ifdef HAVE_HWSERIAL2
void INT_Excep_SCI6_RXI6()
{
  Serial2._rx_complete_irq();
}

void INT_Excep_SCI6_TXI6()
{
  Serial2._tx_udr_empty_irq();
}

bool Serial2_available() {
  return Serial2.available();
}

HardwareSerial Serial2(2, &SCI6, MstpIdSCI6, PIN_IO6, PIN_IO7);
#endif/*HAVE_HWSERIAL2*/

#ifdef HAVE_HWSERIAL3
void INT_Excep_SCI2_RXI2()
{
  Serial3._rx_complete_irq();
}

void INT_Excep_SCI2_TXI2()
{
  Serial3._tx_udr_empty_irq();
}

bool Serial3_available() {
  return Serial3.available();
}

HardwareSerial Serial3(3, &SCI2, MstpIdSCI2, PIN_IO24, PIN_IO26);
#endif/*HAVE_HWSERIAL3*/

#ifdef HAVE_HWSERIAL4
void INT_Excep_SCI3_RXI3()
{
  Serial4._rx_complete_irq();
}

void INT_Excep_SCI3_TXI3()
{
  Serial4._tx_udr_empty_irq();
}

bool Serial4_available() {
  return Serial4.available();
}

HardwareSerial Serial4(4, &SCI3, MstpIdSCI3, PIN_IO3, PIN_IO5);
#endif/*HAVE_HWSERIAL4*/

#ifdef HAVE_HWSERIAL5
void INT_Excep_SCI5_RXI5()
{
  Serial5._rx_complete_irq();
}

void INT_Excep_SCI5_TXI5()
{
  Serial5._tx_udr_empty_irq();
}

bool Serial5_available() {
  return Serial5.available();
}

HardwareSerial Serial5(5, &SCI5, MstpIdSCI5, PIN_IO9, PIN_IO8);
#endif/*HAVE_HWSERIAL5*/

#ifdef HAVE_HWSERIAL6
void INT_Excep_SCI8_RXI8()
{
  Serial6._rx_complete_irq();
}

void INT_Excep_SCI8_TXI8()
{
  Serial6._tx_udr_empty_irq();
}

bool Serial6_available() {
  return Serial6.available();
}

HardwareSerial Serial6(6, &SCI8, MstpIdSCI8, PIN_IO12, PIN_IO11);
#endif/*HAVE_HWSERIAL6*/

#ifdef HAVE_HWSERIAL7
void INT_Excep_SCI1_RXI1()
{
  Serial7._rx_complete_irq();
}

void INT_Excep_SCI1_TXI1()
{
  Serial7._tx_udr_empty_irq();
}

bool Serial7_available() {
  return Serial7.available();
}

HardwareSerial Serial7(7, &SCI1, MstpIdSCI1, PIN_IO58, PIN_IO60);
#endif/*HAVE_HWSERIAL7*/

#endif/*GRSAKURA*/
