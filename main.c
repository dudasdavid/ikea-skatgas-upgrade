#include <msp430.h>

#define LED_PIN BIT2

static void toggle_led(void)
{
    if (P1DIR & LED_PIN)
    {
        P1DIR &= ~LED_PIN;      // OFF = Hi-Z
    }
    else
    {
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
    TACTL = TASSEL_2 | ID_3 | MC_1 | TACLR;   // SMCLK / 8, up mode

    __enable_interrupt();

    while (1)
    {
        __bis_SR_register(LPM0_bits | GIE);   // sleep until Timer_A interrupt
    }
}

void __attribute__((interrupt(TIMERA0_VECTOR))) Timer_A_ISR(void)
{
    static unsigned int div = 0;

    if (++div >= 3)
    {
        div = 0;
        toggle_led();
    }
}