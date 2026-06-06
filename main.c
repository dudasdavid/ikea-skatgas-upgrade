#include <msp430.h>

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Watchdog stop

    P1DIR |= BIT0;             // P1.0 output

    while(1)
    {
        P1OUT ^= BIT0;         // Toggle LED

        volatile unsigned long i;
        for(i = 0; i < 50000; i++);
    }
}