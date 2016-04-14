/*
SoftwareSerial.h (formerly NewSoftSerial.h) - 
Multi-instance software serial library for Arduino/Wiring
-- Interrupt-driven receive and other improvements by ladyada
   (http://ladyada.net)
-- Tuning, circular buffer, derivation from class Print/Stream,
   multi-instance support, porting to 8MHz processors,
   various optimizations, PROGMEM delay tables, inverse logic and 
   direct port writing by Mikal Hart (http://www.arduiniana.org)
-- Pin change interrupt macros by Paul Stoffregen (http://www.pjrc.com)
-- 20MHz processor support by Garrett Mace (http://www.macetech.com)
-- ATmega1280/2560 support by Brett Hagman (http://www.roguerobotics.com/)

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

The latest version of this library can always be found at
http://arduiniana.org.

Modified 11 August 2014 by Nozomu Fujita for GR-SAKURA
*/

#ifndef SoftwareSerial_h
#define SoftwareSerial_h

#include <inttypes.h>
#include <Stream.h>
#ifdef GRSAKURA
#include "HardwareSerial.h"
#endif/*GRSAKURA*/

/******************************************************************************
* Definitions
******************************************************************************/

#define _SS_MAX_RX_BUFF 64 // RX buffer size
#ifdef GRSAKURA
#define _SS_MAX_TX_BUFF 64 // TX buffer size
#endif/*GRSAKURA*/
#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

class SoftwareSerial : public Stream
{
private:
  // per object data
  uint8_t _receivePin;
  uint8_t _receiveBitMask;
  volatile uint8_t *_receivePortRegister;
#ifndef GRSAKURA
  uint8_t _transmitBitMask;
#else /*GRSAKURA*/
  uint8_t _transmitBit;
#endif/*GRSAKURA*/
  volatile uint8_t *_transmitPortRegister;

#ifndef GRSAKURA
  uint16_t _rx_delay_centering;
  uint16_t _rx_delay_intrabit;
  uint16_t _rx_delay_stopbit;
  uint16_t _tx_delay;
#endif/*GRSAKURA*/

  uint16_t _buffer_overflow:1;
  uint16_t _inverse_logic:1;

  // static data
#ifndef GRSAKURA
  static char _receive_buffer[_SS_MAX_RX_BUFF]; 
  static volatile uint8_t _receive_buffer_tail;
  static volatile uint8_t _receive_buffer_head;
#else /*GRSAKURA*/
  uint8_t _format_data_bits;
  enum {
    _format_parity_none,
    _format_parity_even,
    _format_parity_odd
  } _format_parity;
  uint8_t _format_stop_bits;

  uint8_t _receive_bit;
  int8_t  _transmit_bit_pos;
  uint8_t _transmit_bit_length_count;
  uint8_t _receive_byte;
  uint8_t _receive_buffer[_SS_MAX_RX_BUFF];
  volatile uint8_t _receive_buffer_tail;
  volatile uint8_t _receive_buffer_head;

  uint8_t _transmit_bit;
  int8_t  _receive_bit_pos;
  uint8_t _receive_bit_length_count;
  uint8_t _transmit_byte;
  uint8_t _transmit_buffer[_SS_MAX_TX_BUFF];
  volatile uint8_t _transmit_buffer_tail;
  volatile uint8_t _transmit_buffer_head;
#endif/*GRSAKURA*/
  static SoftwareSerial *active_object;

  // private methods
  void recv();
#ifdef GRSAKURA
  void send();
#endif/*GRSAKURA*/
  uint8_t rx_pin_read();
  void tx_pin_write(uint8_t pin_state);
  void setTX(uint8_t transmitPin);
  void setRX(uint8_t receivePin);

#ifndef GRSAKURA
  // private static method for timing
  static inline void tunedDelay(uint16_t delay);
#endif/*GRSAKURA*/

public:
  // public methods
  SoftwareSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);
  virtual ~SoftwareSerial();
#ifndef GRSAKURA
  void begin(long speed);
#else /*GRSAKURA*/
  void begin(uint32_t speed, uint8_t config = SERIAL_8N1);
#endif/*GRSAKURA*/
  bool listen();
  void end();
  bool isListening() { return this == active_object; }
  bool overflow() { bool ret = _buffer_overflow; _buffer_overflow = false; return ret; }
  int peek();

  virtual size_t write(uint8_t byte);
  virtual int read();
  virtual int available();
  virtual void flush();
  
  using Print::write;

  // public only for easy access by interrupt handlers
  static inline void handle_interrupt();
};

// Arduino 0012 workaround
#undef int
#undef char
#undef long
#undef byte
#undef float
#undef abs
#undef round

#endif
