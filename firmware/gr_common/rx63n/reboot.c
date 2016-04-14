/*
  reboot.c - Reboot the system
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

#include "rx63n/reboot.h"
#include "rx63n/iodefine.h"
#include "rx63n/specific_instructions.h"

void system_reboot(reboot_mode mode)
{
    sei();
    switch (mode) {
    case REBOOT_USERAPP:
        //SYSTEM.PRCR.WORD = 0xa502;
        SYSTEM.SWRR = 0xa501;
        break;
    case REBOOT_FIRMWARE:
        SYSTEM.RSTSR0.BYTE = 0x00;
        SYSTEM.RSTSR1.BYTE = 0x00;
        SYSTEM.RSTSR2.BYTE = 0x00;
        WDT.WDTCR.WORD = 0x3310;
        WDT.WDTRCR.BYTE = 0x80;
        WDT.WDTRR = 0x00;
        WDT.WDTRR = 0xff;
        break;
    }
    for (;;);
}
