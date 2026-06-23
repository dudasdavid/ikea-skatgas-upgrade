#include <msp430.h>

#define LED_PIN BIT2

// VLO ~12 kHz, ACLK/8 ~1500 Hz
// 60000 / 1500 ≈ 40 seconds
#define TICKS_10S   14563U// should be 15000U but VLO is not exactly 12 kHz, so this is based on measurements
#define TICKS_40S   58252U

// 4 h  = 14400 s  / 40 s = 360 chunks
// 20 h = 72000 s  / 40 s = 1800 chunks
#define ON_CHUNKS   360U
#define OFF_CHUNKS  1800U

static void led_on(void)
{
    P1OUT &= ~LED_PIN;
    P1DIR |= LED_PIN;       // ON = sink
}

static void led_off(void)
{
    P1DIR &= ~LED_PIN;      // OFF = Hi-Z
}

static void delay_1s(void)
{
    __delay_cycles(300000UL);   // 1s at 1 MHz
}

static void startup_pattern(void)
{
    unsigned char n;

    for (n = 0; n < 2; n++)
    {
        led_on();
        delay_1s();

        led_off();
        delay_1s();
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    // Set DCO to 1 MHz, ACLK = VLO ~12 kHz
    DCOCTL = 0;
    BCSCTL1 = XT2OFF | RSEL2 | RSEL1;   // rough range
    DCOCTL = DCO1 | DCO0;               // rough ~1 MHz-ish
    BCSCTL3 |= LFXT1S_2;    // ACLK = VLO

    P1SEL = 0x00;
#ifdef P1SEL2
    P1SEL2 = 0x00;
#endif

    P1OUT = 0x00;
    P1DIR = 0xFF;           // unused pins output low

    startup_pattern();      // play a startup pattern to indicate the device is alive

    led_on();               // start with 4h ON phase

    TACCR0 = TICKS_40S;
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
    static unsigned int chunks = 0;

    chunks++;

    if (P1DIR & LED_PIN)
    {
        // currently ON
        if (chunks >= ON_CHUNKS)
        {
            chunks = 0;
            led_off();
        }
    }
    else
    {
        // currently OFF
        if (chunks >= OFF_CHUNKS)
        {
            chunks = 0;
            led_on();
        }
    }
}