#include <msp430.h>

#define LED_PIN BIT2

static void toggle_led(void)
{
    if (P1DIR & LED_PIN)
        P1DIR &= ~LED_PIN;      // OFF = Hi-Z
    else {
        P1OUT &= ~LED_PIN;
        P1DIR |= LED_PIN;       // ON = sink
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    P1SEL &= ~LED_PIN;
#ifdef P1SEL2
    P1SEL2 &= ~LED_PIN;
#endif

    P1OUT &= ~LED_PIN;
    P1DIR |= LED_PIN;           // start ON

    TACCR0 = 62500;
    TACCTL0 = CCIE;
    TACTL = TASSEL_2 | ID_3 | MC_1 | TACLR;

    __enable_interrupt();

    while (1)
    {
        // no sleep yet
    }
}

void __attribute__((interrupt(TIMERA0_VECTOR))) Timer_A_ISR(void)
{
    toggle_led();
}