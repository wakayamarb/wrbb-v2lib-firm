/*
  TwoWire.cpp - TWI/I2C library for Wiring & Arduino
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

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
  Modified 2014 by Yuuki Okamiya for software I2C GR-SAKURA and add Wire1,2,4,5,6
  Modified 2014 by Yuuki Okamiya for add setFrequency for Hardware Wire
  Modified 10 Jun 2014 by Nozomu Fujita : i2c_rxBuffer[][] i2c_txBuffer[][] を static に変更
  Modified 10 Jun 2014 by Nozomu Fujita : Wire への TwoWire::setFrequency(int) 対応
  Modified 10 Jun 2014 by Nozomu Fujita : Wire3、Wire7 対応
*/

#ifndef GRSAKURA
extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
  #include "twi.h"
}

#include "Wire.h"
#else
#include "Wire.h"
#include "rx63n/iodefine.h"
#endif

#ifndef GRSAKURA
// Initialize Class Variables //////////////////////////////////////////////////

uint8_t TwoWire::rxBuffer[BUFFER_LENGTH];
uint8_t TwoWire::rxBufferIndex = 0;
uint8_t TwoWire::rxBufferLength = 0;

uint8_t TwoWire::txAddress = 0;
uint8_t TwoWire::txBuffer[BUFFER_LENGTH];
uint8_t TwoWire::txBufferIndex = 0;
uint8_t TwoWire::txBufferLength = 0;

uint8_t TwoWire::transmitting = 0;
void (*TwoWire::user_onRequest)(void);
void (*TwoWire::user_onReceive)(int);

#else
// Initialize Class Variables //////////////////////////////////////////////////
static uint8_t i2c_rxBuffer[8][BUFFER_LENGTH];
static uint8_t i2c_txBuffer[8][BUFFER_LENGTH];

SoftI2cMaster rtc(A4, A5);
static const uint8_t g_sci_i2c_channel_table[8] = {0, 0, 6, 2, 3, 5, 8, 1};
static bool g_sci_i2c_channel_inRepStart[8] = { false };
#endif

#ifndef GRSAKURA
// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire()
{
}
#else
// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire(uint8_t channel)
{
    rxBufferIndex = 0;
    rxBufferLength = 0;

    txAddress = 0;
    txBufferIndex = 0;
    txBufferLength = 0;

    transmitting = 0;
    wire_channel = channel;
    wire_frequency = 100000;
}
#endif

// Public Methods //////////////////////////////////////////////////////////////


void TwoWire::begin(void)
{
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;

#ifndef GRSAKURA
  twi_init();
#else
  if(wire_channel == 0){ // if Software I2C
      rtc.setFrequency(wire_frequency);
  } else { // or not Software I2C
      twi_rx_init(g_sci_i2c_channel_table[wire_channel], wire_frequency);
  }
}
#endif

void TwoWire::begin(uint8_t address)
{
#ifndef GRSAKURA
/* no implementation for slave
  twi_setAddress(address);
  twi_attachSlaveTxEvent(onRequestService);
  twi_attachSlaveRxEvent(onReceiveService);
  begin();
*/
#endif
}

void TwoWire::begin(int address)
{
#ifndef GRSAKURA
/* no implementation for slave
  begin((uint8_t)address);
*/
#endif
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop)
{
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  // perform blocking read into buffer
#ifndef GRSAKURA
  uint8_t read = twi_readFrom(address, rxBuffer, quantity, sendStop);
#else
  uint8_t read = 0;
  address <<= 1;
  address++; // set 1 to bit 1 for read flag
  if (wire_channel == 0) {
      bool di = isNoInterrupts();
      noInterrupts();
      if (g_sci_i2c_channel_inRepStart[wire_channel] != true) {
          rtc.start(address);
      } else {
          rtc.restart(address);
      }
      for (int i = 0; i < quantity; i++) {
          i2c_rxBuffer[wire_channel][i] = rtc.read(i == (quantity - 1));
          read++;
      }
      if (sendStop == true) {
          rtc.stop();
          g_sci_i2c_channel_inRepStart[wire_channel] = false;
      } else {
          g_sci_i2c_channel_inRepStart[wire_channel] = true;
      }
      if (!di) {
          interrupts();
      }
  } else {
      if (g_sci_i2c_channel_inRepStart[wire_channel] != true) {
          twi_rx_start(g_sci_i2c_channel_table[wire_channel], address);
      } else {
          twi_rx_restart(g_sci_i2c_channel_table[wire_channel], address);
      }
      for (int i = 0; i < quantity; i++) {
          i2c_rxBuffer[wire_channel][i] = twi_rx_read(
                  g_sci_i2c_channel_table[wire_channel], i == (quantity - 1));
          read++;
      }
      if (sendStop == true) {
          twi_rx_stop(g_sci_i2c_channel_table[wire_channel]);
          g_sci_i2c_channel_inRepStart[wire_channel] = false;
      } else {
          g_sci_i2c_channel_inRepStart[wire_channel] = true;
      }
  }
#endif
  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = read;

  return read;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TwoWire::beginTransmission(uint8_t address)
{
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
}

void TwoWire::beginTransmission(int address)
{
  beginTransmission((uint8_t)address);
}

//
//  Originally, 'endTransmission' was an f(void) function.
//  It has been modified to take one parameter indicating
//  whether or not a STOP should be performed on the bus.
//  Calling endTransmission(false) allows a sketch to
//  perform a repeated start.
//
//  WARNING: Nothing in the library keeps track of whether
//  the bus tenure has been properly ended with a STOP. Itno
//  is very possible to leave the bus in a hung state if
//  no call to endTransmission(true) is made. Some I2C
//  devices will behave oddly if they do not see a STOP.
//
uint8_t TwoWire::endTransmission(uint8_t sendStop)
{
  // transmit buffer (blocking)
//  int8_t ret = twi_writeTo(txAddress, txBuffer, txBufferLength, 1, sendStop);
    uint8_t address = txAddress;
    address <<= 1;

    if (wire_channel == 0) {
        bool di = isNoInterrupts();
        noInterrupts();

        if (g_sci_i2c_channel_inRepStart[wire_channel] != true) {
            rtc.start(address);
        } else {
            rtc.restart(address);
        }
        for (int i = 0; i < txBufferLength; i++) {
            rtc.write(i2c_txBuffer[wire_channel][i]);
        }
        if (sendStop == true) {
            rtc.stop();
            g_sci_i2c_channel_inRepStart[wire_channel] = false;
        } else {
            g_sci_i2c_channel_inRepStart[wire_channel] = true;
        }
        if (!di) {
            interrupts();
        }

    } else {
        if (g_sci_i2c_channel_inRepStart[wire_channel] != true) {
            twi_rx_start(g_sci_i2c_channel_table[wire_channel], address);
        } else {
            twi_rx_restart(g_sci_i2c_channel_table[wire_channel], address);
        }

        for (int i = 0; i < txBufferLength; i++) {
            twi_rx_write(g_sci_i2c_channel_table[wire_channel],
                    i2c_txBuffer[wire_channel][i]);
        }
        if (sendStop == true) {
            twi_rx_stop(g_sci_i2c_channel_table[wire_channel]);
            g_sci_i2c_channel_inRepStart[wire_channel] = false;
        } else {
            g_sci_i2c_channel_inRepStart[wire_channel] = true;
        }
    }

  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  // indicate that we are done transmitting
  transmitting = 0;
  //  return ret;
  return 0;
}

//  This provides backwards compatibility with the original
//  definition, and expected behaviour, of endTransmission
//
uint8_t TwoWire::endTransmission(void)
{
  return endTransmission(true);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(uint8_t data)
{
  if(transmitting){
  // in master transmitter mode
    // don't bother if buffer is full
    if(txBufferLength >= BUFFER_LENGTH){
      setWriteError();
      return 0;
    }
    // put byte in tx buffer
    i2c_txBuffer[wire_channel][txBufferIndex] = data;
    ++txBufferIndex;
    // update amount in buffer
    txBufferLength = txBufferIndex;
  }else{
  // in slave send mode
    // reply to master
/*no implementation for slave
    twi_transmit(&data, 1);
*/
  }
  return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
  if(transmitting){
  // in master transmitter mode
    for(size_t i = 0; i < quantity; ++i){
      write(data[i]);
    }
  }else{
  // in slave send mode
    // reply to master
/*no implementation for slave
    twi_transmit(data, quantity);
*/
  }
  return quantity;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void)
{
  return rxBufferLength - rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void)
{
  int value = -1;

  // get each successive byte on each call
  if(rxBufferIndex < rxBufferLength){
    value = i2c_rxBuffer[wire_channel][rxBufferIndex];
    ++rxBufferIndex;
  }

  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void)
{
  int value = -1;

  if(rxBufferIndex < rxBufferLength){
    value = i2c_rxBuffer[wire_channel][rxBufferIndex];
  }

  return value;
}

void TwoWire::flush(void)
{
  // XXX: to be implemented.
}

void TwoWire::setFrequency(int freq){
    wire_frequency = freq;
    if(wire_channel == 0){
        rtc.setFrequency(wire_frequency);
    } else {
        twi_rx_setFrequency(g_sci_i2c_channel_table[wire_channel], wire_frequency);
    }



}


/* no implement about slave for GR-SAKURA
 *
// behind the scenes function that is called when data is received
void TwoWire::onReceiveService(uint8_t* inBytes, int numBytes)
{
  // don't bother if user hasn't registered a callback
  if(!user_onReceive){
    return;
  }
  // don't bother if rx buffer is in use by a master requestFrom() op
  // i know this drops data, but it allows for slight stupidity
  // meaning, they may not have read all the master requestFrom() data yet
  if(rxBufferIndex < rxBufferLength){
    return;
  }
  // copy twi rx buffer into local read buffer
  // this enables new reads to happen in parallel
  for(uint8_t i = 0; i < numBytes; ++i){
    rxBuffer[i] = inBytes[i];
  }
  // set rx iterator vars
  rxBufferIndex = 0;
  rxBufferLength = numBytes;
  // alert user program
  user_onReceive(numBytes);
}
*/

/* no implement about slave for GR-SAKURA
 *
// behind the scenes function that is called when data is requested
void TwoWire::onRequestService(void)
{
  // don't bother if user hasn't registered a callback
  if(!user_onRequest){
    return;
  }
  // reset tx buffer iterator vars
  // !!! this will kill any pending pre-master sendTo() activity
  txBufferIndex = 0;
  txBufferLength = 0;
  // alert user program
  user_onRequest();
}
*/

/* no implement about slave for GR-SAKURA
 *
// sets function called on slave write
void TwoWire::onReceive( void (*function)(int) )
{
  user_onReceive = function;
}
*/

/* no implement about slave for GR-SAKURA
 *
// sets function called on slave read
void TwoWire::onRequest( void (*function)(void) )
{
  user_onRequest = function;
}
*/
// Preinstantiate Objects //////////////////////////////////////////////////////

TwoWire Wire = TwoWire(0);
TwoWire Wire1 = TwoWire(1);
TwoWire Wire2 = TwoWire(2);
TwoWire Wire3 = TwoWire(3);
TwoWire Wire4 = TwoWire(4);
TwoWire Wire5 = TwoWire(5);
TwoWire Wire6 = TwoWire(6);
TwoWire Wire7 = TwoWire(7);

