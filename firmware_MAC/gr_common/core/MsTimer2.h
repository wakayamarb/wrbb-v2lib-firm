#ifndef MsTimer2_h
#define MsTimer2_h

#if defined (GRSAKURA)
#define F_CPU 48000000 //PCLK is 48MHz, CPU is operating at 96MHz
#else
#include <avr/interrupt.h>
#endif

namespace MsTimer2 {
    extern unsigned long msecs;
    extern void (*func)();
    extern volatile unsigned long count;
    extern volatile char overflowing;
    extern volatile unsigned int tcnt2;

    void set(unsigned long ms, void (*f)());
    void start();
    void stop();
    void _overflow();
}

#endif
