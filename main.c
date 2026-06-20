#include <msp430.h>

#define LED_PIN BIT2

#define TICKS_5S   7500    // VLO ~12kHz / 8 = ~1500 Hz, 5s  = 7500
#define TICKS_10S  15000   // 10s = 15000

static void led_on(void)
{
    P1OUT &= ~LED_PIN;
    P1DIR |= LED_PIN;       // ON = sink
}

static void led_off(void)
{
    P1DIR &= ~LED_PIN;      // OFF = Hi-Z
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    // clear alternate function selection for all pins
    P1SEL = 0x00;
#ifdef P1SEL2
    P1SEL2 = 0x00;
#endif

    // clear all outputs and set all pins to output low to decrease power consumption
    P1OUT = 0x00;
    P1DIR = 0xFF;      // all P1 pins output low

#ifdef P2DIR
    P2OUT = 0x00;
    P2DIR = 0xFF;
#endif

    led_on();               // start ON

    BCSCTL3 |= LFXT1S_2;    // ACLK = VLO ~12 kHz

    TACCR0 = TICKS_5S;      // first interrupt after 5s
    TACCTL0 = CCIE;
    TACTL = TASSEL_1 | ID_3 | MC_1 | TACLR;   // ACLK / 8, up mode

    __enable_interrupt();

    while (1)
    {
        __bis_SR_register(LPM3_bits | GIE);
    }
}

void __attribute__((interrupt(TIMERA0_VECTOR))) Timer_A_ISR(void)
{
    if (P1DIR & LED_PIN)
    {
        led_off();
        TACCR0 = TICKS_10S;     // stay OFF for 10s
    }
    else
    {
        led_on();
        TACCR0 = TICKS_5S;      // stay ON for 5s
    }

    TACTL |= TACLR;             // restart timer period cleanly
}