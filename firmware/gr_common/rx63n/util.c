/*
  util.c - 
  Copyright (c) 2014 Nozomu Fujita.  All right reserved.

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

#include "rx63n/util.h"
#include "rx63n/iodefine.h"
#include "rx63n/specific_instructions.h"
#include "rx63n/interrupt_handlers.h"
#include "Arduino.h"
#include "utilities.h"

static void _startOrStopModule(MstpId m, bool start);

void startModule(MstpId module)
{
    _startOrStopModule(module, true);
}

void stopModule(MstpId module)
{
    _startOrStopModule(module, false);
}

static void _startOrStopModule(MstpId id, bool start)
{
    static const struct {
        int id:9;
        uint8_t reg:2;
        uint8_t bit:5;
    } t[] = {
        {MstpIdEXDMAC,  0, 29},
        {MstpIdEXDMAC0, 0, 29},
        {MstpIdEXDMAC1, 0, 29},
        {MstpIdDMAC,    0, 28},
        {MstpIdDMAC0,   0, 28},
        {MstpIdDMAC1,   0, 28},
        {MstpIdDMAC2,   0, 28},
        {MstpIdDMAC3,   0, 28},
        {MstpIdDTC,     0, 28},
        {MstpIdA27,     0, 27},
        {MstpIdA24,     0, 24},
        {MstpIdAD,      0, 23},
        {MstpIdDA,      0, 19},
        {MstpIdS12AD,   0, 17},
        {MstpIdCMT0,    0, 15},
        {MstpIdCMT1,    0, 15},
        {MstpIdCMT2,    0, 14},
        {MstpIdCMT3,    0, 14},
        {MstpIdTPU0,    0, 13},
        {MstpIdTPU1,    0, 13},
        {MstpIdTPU2,    0, 13},
        {MstpIdTPU3,    0, 13},
        {MstpIdTPU4,    0, 13},
        {MstpIdTPU5,    0, 13},
        {MstpIdTPU6,    0, 12},
        {MstpIdTPU7,    0, 12},
        {MstpIdTPU8,    0, 12},
        {MstpIdTPU9,    0, 12},
        {MstpIdTPU10,   0, 12},
        {MstpIdTPU11,   0, 12},
        {MstpIdPPG0,    0, 11},
        {MstpIdPPG1,    0, 10},
        {MstpIdMTU,     0,  9},
        {MstpIdMTU0,    0,  9},
        {MstpIdMTU1,    0,  9},
        {MstpIdMTU2,    0,  9},
        {MstpIdMTU3,    0,  9},
        {MstpIdMTU4,    0,  9},
        {MstpIdMTU5,    0,  9},
        {MstpIdTMR0,    0,  5},
        {MstpIdTMR1,    0,  5},
        {MstpIdTMR01,   0,  5},
        {MstpIdTMR2,    0,  4},
        {MstpIdTMR3,    0,  4},
        {MstpIdTMR23,   0,  4},
        {MstpIdSCI0,    1, 31},
        {MstpIdSMCI0,   1, 31},
        {MstpIdSCI1,    1, 30},
        {MstpIdSMCI1,   1, 30},
        {MstpIdSCI2,    1, 29},
        {MstpIdSMCI2,   1, 29},
        {MstpIdSCI3,    1, 28},
        {MstpIdSMCI3,   1, 28},
        {MstpIdSCI4,    1, 27},
        {MstpIdSMCI4,   1, 27},
        {MstpIdSCI5,    1, 26},
        {MstpIdSMCI5,   1, 26},
        {MstpIdSCI6,    1, 25},
        {MstpIdSMCI6,   1, 25},
        {MstpIdSCI7,    1, 24},
        {MstpIdSMCI7,   1, 24},
        {MstpIdCRC,     1, 23},
        {MstpIdPDC,     1, 22},
        {MstpIdRIIC0,   1, 21},
        {MstpIdRIIC1,   1, 20},
        {MstpIdUSB0,    1, 19},
        {MstpIdUSB1,    1, 18},
        {MstpIdRSPI0,   1, 17},
        {MstpIdRSPI1,   1, 16},
        {MstpIdEDMAC,   1, 15},
        {MstpIdTEMPS,   1,  8},
        {MstpIdSCI12,   1,  4},
        {MstpIdSMCI12,  1,  4},
        {MstpIdCAN2,    1,  2},
        {MstpIdCAN1,    1,  1},
        {MstpIdCAN0,    1,  0},
        {MstpIdSCI8,    2, 27},
        {MstpIdSMCI8,   2, 27},
        {MstpIdSCI9,    2, 26},
        {MstpIdSMCI9,   2, 26},
        {MstpIdSCI10,   2, 25},
        {MstpIdSMCI10,  2, 25},
        {MstpIdSCI11,   2, 24},
        {MstpIdSMCI11,  2, 24},
        {MstpIdRSPI2,   2, 22},
        {MstpIdMCK,     2, 19},
        {MstpIdIEB,     2, 18},
        {MstpIdRIIC2,   2, 17},
        {MstpIdRIIC3,   2, 16},
        {MstpIdRAM1,    2,  1},
        {MstpIdRAM0,    2,  0},
        {MstpIdDEU,     3, 31},
    };
    static uint8_t f[(NumOfMstpId + __CHAR_BIT__ - 1) / __CHAR_BIT__] = {0};

    if (id >= 0 && id < NumOfMstpId) {
        int reg;
        volatile uint32_t* mstpcr = NULL;
        int bit;
        int i;
        for (i = 0; i < (int)(sizeof(t) / sizeof(*t)); i++) {
            if (t[i].id == id) {
                reg = t[i].reg;
                mstpcr = (volatile uint32_t*)((uint32_t*)&SYSTEM.MSTPCRA.LONG + reg);
                bit = t[i].bit;
                break;
            }
        }
        if (mstpcr != NULL) {
            int c = 0;
            int j;
            for (j = 0; j < (int)(sizeof(t) / sizeof(*t)); j++) {
                if (t[j].id != id && (t[j].reg == reg && t[j].bit == bit)) {
                    if ((f[t[j].id / __CHAR_BIT__] & (1 << (t[j].id % __CHAR_BIT__))) != 0) {
                        c++;
                    }
                }
            }
            if (start) {
                if ((f[id / __CHAR_BIT__] & (1 << (id % __CHAR_BIT__))) == 0) {
                    f[id / __CHAR_BIT__] |= (1 << (id % __CHAR_BIT__));
                    if (c == 0) {
                        pushi();
                        cli();
                        *mstpcr &= ~(1U << bit);
                        popi();
                    }
                }
            } else {
                if ((f[id / __CHAR_BIT__] & (1 << (id % __CHAR_BIT__))) != 0) {
                    f[id / __CHAR_BIT__] &= ~(1 << (id % __CHAR_BIT__));
                    if (c == 0) {
                        pushi();
                        cli();
                        *mstpcr |= (1U << bit);
                        popi();
                    }
                }
            }
        }
    }
}

void assignPinFunction(int pin, int psel, int isel, int asel)
{
    typedef union {
        unsigned char BYTE;
        struct {
            unsigned char PSEL:5;
            unsigned char :1;
            unsigned char ISEL:1;
            unsigned char ASEL:1;
        } BIT;
    } PFS;
    if (pin >= 0 && pin < NUM_DIGITAL_PINS) {
        int port = digitalPinToPort(pin);
        int bit = digitalPinToBit(pin);
        volatile PFS* pfs = (volatile PFS*)&MPC.P00PFS.BYTE + (8 * (port) + (bit));
        if (psel >= 0) {
            pfs->BIT.PSEL = psel;
        }
        if (isel == 0 || isel == 1) {
            pfs->BIT.ISEL = isel;
        }
        if (asel == 0 || asel == 1) {
            pfs->BIT.ASEL = asel;
        }
    }
}

void assignPinPSEL(int pin, int psel)
{
    assignPinFunction(pin, psel, -1, -1);
}

void assignPinISEL(int pin, int isel)
{
    assignPinFunction(pin, -1, isel, -1);
}

void assignPinASEL(int pin, int asel)
{
    assignPinFunction(pin, -1, -1, asel);
}


static volatile PinMode sPinMode[NUM_DIGITAL_PINS] = {
    PinModeUnused,
};

void setPinMode(int pin, PinMode mode)
{
    if (pin >= 0 && pin < NUM_DIGITAL_PINS) {
        if (getPinMode(pin) != mode) {
            resetPinMode(pin);
            switch (mode) {
            case PinModeUnused:
                if (setPinModeUnused != NULL) {
                    setPinModeUnused(pin);
                }
                goto change;
            case PinModeInput:
                if (setPinModeInput != NULL) {
                    setPinModeInput(pin);
                }
                goto change;
            case PinModeOutput:
                if (setPinModeOutput != NULL) {
                    setPinModeOutput(pin);
                }
                goto change;
            case PinModeInputPullUp:
                if (setPinModeInputPullUp != NULL) {
                    setPinModeInputPullUp(pin);
                }
                goto change;
            case PinModeOutputHigh:
                if (setPinModeOutputHigh != NULL) {
                    setPinModeOutputHigh(pin);
                }
                goto change;
            case PinModeOutputOpenDrain:
                if (setPinModeOutputOpenDrain != NULL) {
                    setPinModeOutputOpenDrain(pin);
                }
                goto change;
            case PinModeAnalogRead:
                if (setPinModeAnalogRead != NULL) {
                    setPinModeAnalogRead(pin);
                }
                goto change;
            case PinModeAnalogWrite:
                if (setPinModeAnalogWrite != NULL) {
                    setPinModeAnalogWrite(pin);
                }
                goto change;
            case PinModeTone:
                if (setPinModeTone != NULL) {
                    setPinModeTone(pin);
                }
                goto change;
            case PinModeServo:
                if (setPinModeServo != NULL) {
                    setPinModeServo(pin);
                }
                goto change;
            case PinModeDac:
                if (setPinModeDac != NULL) {
                    setPinModeDac(pin);
                }
                goto change;
            case PinModeOther:
                if (setPinModeOther != NULL) {
                    setPinModeOther(pin);
                }
              change:
                changePinMode(pin, mode);
                break;
            default:
                break;
            }
        }
    }
}

void setPinModeUnused(int pin)
{
    setPinModeInput(pin);
}

PinMode getPinMode(int pin)
{
    return (pin >= 0 && pin < NUM_DIGITAL_PINS) ? sPinMode[pin] : PinModeError;
}

void changePinMode(int pin, PinMode mode)
{
    if (pin >= 0 && pin < NUM_DIGITAL_PINS) {
        sPinMode[pin] = mode;
    }
}

void resetPinMode(int pin)
{
    if (pin >= 0 && pin < NUM_DIGITAL_PINS) {
        PinMode mode = getPinMode(pin);
        switch (mode) {
        case PinModeUnused:
            break;
        case PinModeInput:
            if (resetPinModeInput != NULL) {
                resetPinModeInput(pin);
            }
            goto change;
        case PinModeOutput:
            if (resetPinModeOutput != NULL) {
                resetPinModeOutput(pin);
            }
            goto change;
        case PinModeInputPullUp:
            if (resetPinModeInputPullUp != NULL) {
                resetPinModeInputPullUp(pin);
            }
            goto change;
        case PinModeOutputHigh:
            if (resetPinModeOutputHigh != NULL) {
                resetPinModeOutputHigh(pin);
            }
            goto change;
        case PinModeOutputOpenDrain:
            if (resetPinModeOutputOpenDrain != NULL) {
                resetPinModeOutputOpenDrain(pin);
            }
            goto change;
        case PinModeAnalogRead:
            if (resetPinModeAnalogRead != NULL) {
                resetPinModeAnalogRead(pin);
            }
            goto change;
        case PinModeAnalogWrite:
            if (resetPinModeAnalogWrite != NULL) {
                resetPinModeAnalogWrite(pin);
            }
            goto change;
        case PinModeTone:
            if (resetPinModeTone != NULL) {
                resetPinModeTone(pin);
            }
            goto change;
        case PinModeServo:
            if (resetPinModeServo != NULL) {
                resetPinModeServo(pin);
            }
            goto change;
        case PinModeDac:
            if (resetPinModeDac != NULL) {
                resetPinModeDac(pin);
            }
            goto change;
        case PinModeOther:
            if (resetPinModeOther != NULL) {
                resetPinModeOther(pin);
            }
          change:
            changePinMode(pin, PinModeUnused);
            break;
        default:
            break;
        }
    }
}

void setPinModeHardwarePWM(int pin, int period, int term, unsigned long length)
{
    if (pin >= 0 && pin < NUM_DIGITAL_PINS) {
        int port = digitalPinToPort(pin);
        if (port == NOT_A_PIN) {
            return;
        }
        int bit = digitalPinToBit(pin);
        int tpsc = 0b10; // PCLK/16
        switch (pin) {
        case PIN_IO0:
            startModule(MstpIdTPU3);
            assignPinFunction(pin, 0b00011, 0, 0);
            BSET(portModeRegister(port), bit);
            TPUA.TSTR.BIT.CST3 = 0;
            TPU3.TCR.BIT.TPSC = tpsc;
            TPU3.TCR.BIT.CCLR = 0b010;
            TPU3.TCR.BIT.CKEG = 0b01;
            TPU3.TMDR.BIT.MD = 0b0011;
            changePinModeHardwarePWM(pin, period, term, length);
            TPUA.TSTR.BIT.CST3 = 1;
            break;
        case PIN_IO1:
            startModule(MstpIdMTU1);
            assignPinFunction(pin, 0b00001, 0, 0);
            BSET(portModeRegister(port), bit);
            MTU.TSTR.BIT.CST1 = 0;
            MTU1.TCR.BIT.TPSC = tpsc;
            MTU1.TCR.BIT.CCLR = 0b001;
            MTU1.TCR.BIT.CKEG = 0b01;
            MTU1.TMDR.BIT.MD = 0b0010;
            changePinModeHardwarePWM(pin, period, term, length);
            MTU.TSTR.BIT.CST1 = 1;
            break;
        case PIN_IO2:
            startModule(MstpIdTPU3);
            assignPinFunction(pin, 0b00011, 0, 0);
            BSET(portModeRegister(port), bit);
            TPUA.TSTR.BIT.CST3 = 0;
            TPU3.TCR.BIT.TPSC = tpsc;
            TPU3.TCR.BIT.CCLR = 0b010;
            TPU3.TCR.BIT.CKEG = 0b01;
            TPU3.TMDR.BIT.MD = 0b0011;
            changePinModeHardwarePWM(pin, period, term, length);
            TPUA.TSTR.BIT.CST3 = 1;
            break;
        case PIN_IO3:
            startModule(MstpIdTPU3);
            assignPinFunction(pin, 0b00011, 0, 0);
            BSET(portModeRegister(port), bit);
            TPUA.TSTR.BIT.CST3 = 0;
            TPU3.TCR.BIT.TPSC = tpsc;
            TPU3.TCR.BIT.CCLR = 0b010;
            TPU3.TCR.BIT.CKEG = 0b01;
            TPU3.TMDR.BIT.MD = 0b0011;
            changePinModeHardwarePWM(pin, period, term, length);
            TPUA.TSTR.BIT.CST3 = 1;
            break;
        case PIN_IO4:
            startModule(MstpIdMTU4);
            assignPinFunction(pin, 0b00001, 0, 0);
            BSET(portModeRegister(port), bit);
            MTU.TSTR.BIT.CST4 = 0;
            MTU.TOER.BIT.OE4A = 1;
            MTU4.TCR.BIT.TPSC = tpsc;
            MTU4.TCR.BIT.CCLR = 0b001;
            MTU4.TCR.BIT.CKEG = 0b01;
            MTU4.TMDR.BIT.MD = 0b0010;
            changePinModeHardwarePWM(pin, period, term, length);
            MTU.TSTR.BIT.CST4 = 1;
            break;
        case PIN_IO5:
            // TIOCA4
            startModule(MstpIdTPU4);
            assignPinFunction(pin, 0b00011, 0, 0);
            BSET(portModeRegister(port), bit);
            TPUA.TSTR.BIT.CST4 = 0;
            TPU4.TCR.BIT.TPSC = tpsc;
            TPU4.TCR.BIT.CCLR = 0b001;
            TPU4.TCR.BIT.CKEG = 0b01;
            TPU4.TMDR.BIT.MD = 0b0010;
            changePinModeHardwarePWM(pin, period, term, length);
            TPUA.TSTR.BIT.CST4 = 1;
            break;
        case PIN_IO6:
            startModule(MstpIdTPU0);
            assignPinFunction(pin, 0b00011, 0, 0);
            BSET(portModeRegister(port), bit);
            TPUA.TSTR.BIT.CST0 = 0;
            TPU0.TCR.BIT.TPSC = tpsc;
            TPU0.TCR.BIT.CCLR = 0b001;
            TPU0.TCR.BIT.CKEG = 0b01;
            TPU0.TMDR.BIT.MD = 0b0011;
            changePinModeHardwarePWM(pin, period, term, length);
            TPUA.TSTR.BIT.CST0 = 1;
            break;
        case PIN_IO7:
            startModule(MstpIdTPU0);
            assignPinFunction(pin, 0b00011, 0, 0);
            BSET(portModeRegister(port), bit);
            TPUA.TSTR.BIT.CST0 = 0;
            TPU0.TCR.BIT.TPSC = tpsc;
            TPU0.TCR.BIT.CCLR = 0b001;
            TPU0.TCR.BIT.CKEG = 0b01;
            TPU0.TMDR.BIT.MD = 0b0011;
            changePinModeHardwarePWM(pin, period, term, length);
            TPUA.TSTR.BIT.CST0 = 1;
            break;
        case PIN_IO11:
            startModule(MstpIdMTU3);
            assignPinFunction(pin, 0b00001, 0, 0);
            BSET(portModeRegister(port), bit);
            MTU.TSTR.BIT.CST3 = 0;
            MTU3.TCR.BIT.TPSC = tpsc;
            MTU3.TCR.BIT.CCLR = 0b101;
            MTU3.TCR.BIT.CKEG = 0b01;
            MTU3.TMDR.BIT.MD = 0b0010;
            changePinModeHardwarePWM(pin, period, term, length);
            MTU.TSTR.BIT.CST3 = 1;
            break;
        }
    }
}

void changePinModeHardwarePWM(int pin, int period, int term, unsigned long length)
{
    if (pin >= 0 && pin < NUM_DIGITAL_PINS) {
        int port = digitalPinToPort(pin);
        if (port == NOT_A_PIN) {
            return;
        }
        switch (pin) {
        case PIN_IO0:
            if (term <= 0) {
                TPU3.TIORH.BIT.IOA = 0b0001;
                TPU3.TGRA = 0;
            } else {
                TPU3.TIORH.BIT.IOA = 0b0101;
                TPU3.TGRA = term - 1;
            }
            TPU3.TIORH.BIT.IOB = 0b0110;
            TPU3.TGRB = period - 1;
            break;
        case PIN_IO1:
            MTU1.TIOR.BIT.IOB = 0b0001;
            if (term <= 0) {
                MTU1.TGRB = 0;
                MTU1.TIOR.BIT.IOA = 0b0001;
            } else if (term < period) {
                MTU1.TGRB = term - 1;
                MTU1.TIOR.BIT.IOA = 0b0010;
            } else {
                MTU1.TGRB = period - 1;
                MTU1.TIOR.BIT.IOA = 0b0110;
            }
            MTU1.TGRA = period - 1;
            break;
        case PIN_IO2:
            if (term <= 0) {
                TPU3.TIORL.BIT.IOC = 0b0001;
                TPU3.TGRC = 0;
            } else {
                TPU3.TIORL.BIT.IOC = 0b0101;
                TPU3.TGRC = term - 1;
            }
            TPU3.TIORH.BIT.IOB = 0b0110;
            TPU3.TGRB = period - 1;
            break;
        case PIN_IO3:
            if (term <= 0) {
                TPU3.TIORL.BIT.IOD = 0b0001;
                TPU3.TGRD = 0;
            } else {
                TPU3.TIORL.BIT.IOD = 0b0101;
                TPU3.TGRD = term - 1;
            }
            TPU3.TIORH.BIT.IOB = 0b0110;
            TPU3.TGRB = period - 1;
            break;
        case PIN_IO4:
            MTU4.TIORH.BIT.IOB = 0b0001;
            if (term <= 0) {
                MTU4.TGRB = 0;
                MTU4.TIORH.BIT.IOA = 0b0001;
            } else if (term < period) {
                MTU4.TGRB = term - 1;
                MTU4.TIORH.BIT.IOA = 0b0010;
            } else {
                MTU4.TGRB = period - 1;
                MTU4.TIORH.BIT.IOA = 0b0110;
            }
            MTU4.TGRA = period - 1;
            break;
        case PIN_IO5:
            // TIOCA4
            TPU4.TIOR.BIT.IOB = 0b0101;
            if (term <= 0) {
                TPU4.TGRB = 0;
                TPU4.TIOR.BIT.IOA = 0b0001;
            } else {
                TPU4.TGRB = term - 1;
                TPU4.TIOR.BIT.IOA = 0b0110;
            }
            TPU4.TGRA = period - 1;
            break;
        case PIN_IO6:
            if (term <= 0) {
                TPU0.TIORL.BIT.IOC = 0b0001;
                TPU0.TGRC = 0;
            } else {
                TPU0.TIORL.BIT.IOC = 0b0101;
                TPU0.TGRC = term - 1;
            }
            TPU0.TIORH.BIT.IOA = 0b0110;
            TPU0.TGRA = period - 1;
            break;
        case PIN_IO7:
            if (term <= 0) {
                TPU0.TIORL.BIT.IOD = 0b0001;
                TPU0.TGRD = 0;
            } else {
                TPU0.TIORL.BIT.IOD = 0b0101;
                TPU0.TGRD = term - 1;
            }
            TPU0.TIORH.BIT.IOA = 0b0110;
            TPU0.TGRA = period - 1;
            break;
        case PIN_IO11:
            MTU3.TIORL.BIT.IOD = 0b0001;
            if (term <= 0) {
                MTU3.TGRD = 0;
                MTU3.TIORL.BIT.IOC = 0b0001;
            } else if (term < period) {
                MTU3.TGRD = term - 1;
                MTU3.TIORL.BIT.IOC = 0b0010;
            } else {
                MTU3.TGRD = period - 1;
                MTU3.TIORL.BIT.IOC = 0b0110;
            }
            MTU3.TGRC = period - 1;
            break;
        }
    }
}

void resetPinModeHardwarePWM(int pin)
{
    if (pin >= 0 && pin < NUM_DIGITAL_PINS) {
        int port = digitalPinToPort(pin);
        if (port == NOT_A_PIN) {
            return;
        }
        switch (pin) {
        case PIN_IO0:
            setPinMode(pin, PinModeInput);
            //stopModule(MstpIdTPU3);
            break;
        case PIN_IO1:
            setPinMode(pin, PinModeInput);
            //stopModule(MstpIdMTU1);
            break;
        case PIN_IO2:
            setPinMode(pin, PinModeInput);
            //stopModule(MstpIdTPU3);
            break;
        case PIN_IO3:
            setPinMode(pin, PinModeInput);
            //stopModule(MstpIdTPU3);
            break;
        case PIN_IO4:
            setPinMode(pin, PinModeInput);
            //stopModule(MstpIdMTU4);
            break;
        case PIN_IO5:
            setPinMode(pin, PinModeInput);
            //stopModule(MstpIdTPU4);
            break;
        case PIN_IO6:
            setPinMode(pin, PinModeInput);
            //stopModule(MstpIdTPU0);
            break;
        case PIN_IO7:
            setPinMode(pin, PinModeInput);
            //stopModule(MstpIdTPU0);
            break;
        case PIN_IO11:
            setPinMode(pin, PinModeInput);
            //stopModule(MstpIdMTU1);
            break;
        }
    }
}

typedef struct {
    bool valid;
    int8_t pin;
    uint16_t count;
    uint16_t period;
    uint16_t term;
    uint32_t length;
    volatile uint8_t* out;
    uint8_t bit;
} SoftwarePwm;

static SoftwarePwm softwarePwmTable[MaxSoftwarePwmChannels];

void setPinModeSoftwarePWM(int pin, int period, int term, unsigned long length)
{
    if (pin >= 0 && pin < NUM_DIGITAL_PINS) {
        SoftwarePwm* p;
        for (p = &softwarePwmTable[0]; p < &softwarePwmTable[MaxSoftwarePwmChannels]; p++) {
            if (!p->valid || p->pin == pin) {
                setPinModeOutput(pin);
                p->valid = false;
                p->pin = pin;
                p->count = 0;
                p->period = period;
                p->term = term;
                p->length = length;
                p->out = portOutputRegister(digitalPinToPort(pin));
                p->bit = digitalPinToBit(pin);
                p->valid = true;
                break;
            }
        }
    }
}

void changePinModeSoftwarePWM(int pin, int period, int term, unsigned long length)
{
    if (pin >= 0 && pin < NUM_DIGITAL_PINS) {
        volatile SoftwarePwm* p;
        for (p = &softwarePwmTable[0]; p < &softwarePwmTable[MaxSoftwarePwmChannels]; p++) {
            if (p->pin == pin) {
                p->valid = false;
                p->term = term;
                p->period = period;
                p->length = length;
                p->valid = true;
                break;
            }
        }
    }
}

void resetPinModeSoftwarePWM(int pin)
{
    SoftwarePwm* p;
    for (p = &softwarePwmTable[0]; p < &softwarePwmTable[MaxSoftwarePwmChannels]; p++) {
        if (p->valid && p->pin == pin) {
            p->pin = -1;
            p->valid = false;
            digitalWrite(pin, LOW);
            break;
        }
    }
}

void INT_Excep_TPU2_TGI2A()
{
    SoftwarePwm* p;
    for (p = &softwarePwmTable[0]; p < &softwarePwmTable[MaxSoftwarePwmChannels]; p++) {
        if (p->valid) {
            if (p->count == p->term) {
                BCLR(p->out, p->bit);
            } else if (p->count == 0) {
                BSET(p->out, p->bit);
            }
            if (p->length > 0 && --p->length == 0) {
                p->valid = false;
            }
            if (++p->count >= p->period) {
                p->count = 0;
            }
        }
    }
}
