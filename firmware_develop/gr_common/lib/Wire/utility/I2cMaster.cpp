/* Arduino I2cMaster Library
 * Copyright (C) 2010 by William Greiman
 *
 * This file is part of the Arduino I2cMaster Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino I2cMaster Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *   Modified for GR-SAKURA 3 May 2014 by Yuuki Okamiya.
 *   Modified 10 Jun 2014 by Nozomu Fujita : SoftI2cMaster::setFrequency(int) 追加
 *   Modified 30 Jul 2014 by Yuuki Okamiya
 *   Modified 2  Jul 2015 by Yuuki Okamiya : Adjustment
 */
#include <Arduino.h>
#include "I2cMaster.h"
//==============================================================================
// WARNING don't change SoftI2cMaster unless you verify the change with a scope
//------------------------------------------------------------------------------
/**
 * Initialize SCL/SDA pins and set the bus high.
 *
 * \param[in] sdaPin The software SDA pin number.
 *
 * \param[in] sclPin The software SCL pin number.
 */
SoftI2cMaster::SoftI2cMaster(uint8_t sdaPin, uint8_t sclPin) {
  sdaPin_ = sdaPin;
  pinMode(sdaPin_, OUTPUT_OPENDRAIN);
  digitalWrite(sdaPin_, HIGH);
  sclPin_ = sclPin;
  pinMode(sclPin_, OUTPUT_OPENDRAIN);
  digitalWrite(sclPin_, HIGH);
}
//------------------------------------------------------------------------------
/** Read a byte and send Ack if more reads follow else Nak to terminate read.
 *
 * \param[in] last Set true to terminate the read else false.
 *
 * \return The byte read from the I2C bus.
 */
//#define delayMicroseconds delayMicroseconds
uint8_t SoftI2cMaster::read(uint8_t last) {
  uint8_t b = 0;
  // make sure pull-up enabled
  pinMode(sdaPin_, INPUT_PULLUP);
  // read byte
  delayMicroseconds(10); //TODO
  for (uint8_t i = 0; i < 8; i++) {
    // don't change this loop unless you verify the change with a scope
    b <<= 1;
    delayMicroseconds(3); //TODO
    digitalWrite(sclPin_, HIGH);
    delayMicroseconds(15); //TODO
    if (digitalRead(sdaPin_)) b |= 1;
    digitalWrite(sclPin_, LOW);
  }
  // send Ack or Nak
  pinMode(sdaPin_, OUTPUT_OPENDRAIN);
  digitalWrite(sdaPin_, last);
  delayMicroseconds(delayClock);
  digitalWrite(sclPin_, HIGH);
  delayMicroseconds(delayClock);
  digitalWrite(sclPin_, LOW);
  digitalWrite(sdaPin_, LOW);
  return b;
}
//------------------------------------------------------------------------------
/** Issue a restart condition.
 *
 * \param[in] addressRW I2C address with read/write bit.
 *
 * \return The value true, 1, for success or false, 0, for failure.
 */
bool SoftI2cMaster::restart(uint8_t addressRW) {
  digitalWrite(sdaPin_, HIGH);
  digitalWrite(sclPin_, HIGH);
  delayMicroseconds(delayClock);
  return start(addressRW);
}
//------------------------------------------------------------------------------
/** Issue a start condition.
 *
 * \param[in] addressRW I2C address with read/write bit.
 *
 * \return The value true, 1, for success or false, 0, for failure.
 */
bool SoftI2cMaster::start(uint8_t addressRW) {
  digitalWrite(sdaPin_, LOW);
  delayMicroseconds(delayClock);
  digitalWrite(sclPin_, LOW);
  delayMicroseconds(delayClock);
  return write(addressRW);
}
//------------------------------------------------------------------------------
  /**  Issue a stop condition. */
void SoftI2cMaster::stop(void) {
  digitalWrite(sdaPin_, LOW);
  delayMicroseconds(delayClock);
  digitalWrite(sclPin_, HIGH);
  delayMicroseconds(delayClock);
  digitalWrite(sdaPin_, HIGH);
  delayMicroseconds(delayClock);
}
//------------------------------------------------------------------------------
/**
 * Write a byte.
 *
 * \param[in] data The byte to send.
 *
 * \return The value true, 1, if the slave returned an Ack or false for Nak.
 */
bool SoftI2cMaster::write(uint8_t data) {
  // write byte

  for (uint8_t m = 0x80; m != 0; m >>= 1) {
    // don't change this loop unless you verify the change with a scope
    delayMicroseconds(3); //TODO
    delayMicroseconds(3); //TODO
    digitalWrite(sdaPin_, !((m & data) == 0));
    delayMicroseconds(3); //TODO
    digitalWrite(sclPin_, HIGH);
    delayMicroseconds(3); //TODO
    digitalWrite(sclPin_, LOW);
  }

  // get Ack or Nak
  pinMode(sdaPin_, INPUT_PULLUP);
  delayMicroseconds(3); //TODO
  digitalWrite(sclPin_, HIGH);
  delayMicroseconds(3); //TODO
  uint8_t rtn = digitalRead(sdaPin_);
  digitalWrite(sclPin_, LOW);
  pinMode(sdaPin_, OUTPUT_OPENDRAIN);
  digitalWrite(sdaPin_, HIGH);
  delayMicroseconds(20); //TODO
  return rtn == 0;
}
//------------------------------------------------------------------------------
/**
 * I2C の周波数(待ち時間)を設定する
 *
 * 引数: freq は周波数(Hz)
 *
 * 戻値: なし
 */
void SoftI2cMaster::setFrequency(int freq)
{
    // TODO to implement
    if (freq > 0) {
//        delayClock = (1000 + (2 * freq) - 1) / (2 * freq);
        delayClock = 10;
    }
}
