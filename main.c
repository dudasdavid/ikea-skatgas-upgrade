#include <msp430.h>

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Watchdog stop

    volatile unsigned long i;

    while(1)
    {
        // LED ON: drive low
        P1DIR |= BIT2;     // P1.2 output
        P1OUT &= ~BIT2;    // drive low

        for(i = 0; i < 50000; i++);

        // LED OFF: high impedance
        P1DIR &= ~BIT2;    // input = Hi-Z

        for(i = 0; i < 50000; i++);

    }
}