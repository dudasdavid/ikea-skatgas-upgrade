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
    P1DIR |= LED_PIN;       // ON = sink

    TACCR0 = 62500;
    TACTL = TASSEL_2 | ID_3 | MC_1 | TACLR;   // SMCLK / 8, up mode

    unsigned int div = 0;

    while (1)
    {
        if (TACCTL0 & CCIFG)
        {
            TACCTL0 &= ~CCIFG;

            if (++div >= 3)
            {
                div = 0;
                toggle_led();
            }
        }
    }
}