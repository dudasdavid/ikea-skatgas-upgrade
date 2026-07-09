#include <msp430.h>

#define SLEEP_PIN BIT5
#define TEST_PIN BIT2

#define XT1_PIN_MASK (BIT6 | BIT7)
#define XT1_STARTUP_ATTEMPTS 200U
#define XT1_SETTLE_CYCLES 10000U
#define RTC_TICKS_5S 159U

static void sleep_pin_on(void)
{
    P1OUT &= ~SLEEP_PIN;
    P1DIR |= SLEEP_PIN;
}

static void sleep_pin_off(void)
{
    P1DIR &= ~SLEEP_PIN;
}

static void sleep_pin_toggle(void)
{
    if (P1DIR & SLEEP_PIN)
    {
        sleep_pin_off();
    }
    else
    {
        sleep_pin_on();
    }
}

static void init_gpio(void)
{
    P1SEL0 &= ~(SLEEP_PIN | TEST_PIN);
    P1SEL1 &= ~(SLEEP_PIN | TEST_PIN);
    P1REN &= ~(SLEEP_PIN | TEST_PIN);

    P1OUT &= ~TEST_PIN;
    P1DIR |= TEST_PIN;
    sleep_pin_off();

    P2SEL0 &= ~XT1_PIN_MASK;
    P2SEL1 |= XT1_PIN_MASK;
    P2REN &= ~XT1_PIN_MASK;

    PM5CTL0 &= ~LOCKLPM5;
}

static unsigned char init_xt1(void)
{
    unsigned int attempts = XT1_STARTUP_ATTEMPTS;

    CSCTL4 = SELA__XT1CLK;
    CSCTL6 = XT1DRIVE_3;
    P1OUT |= TEST_PIN;

    do
    {
        CSCTL7 &= ~XT1OFFG;
        SFRIFG1 &= ~OFIFG;
        __delay_cycles(XT1_SETTLE_CYCLES);
    }
    while ((SFRIFG1 & OFIFG) && --attempts);

    if (SFRIFG1 & OFIFG)
    {
        P1OUT |= TEST_PIN;
        return 0;
    }

    P1OUT &= ~TEST_PIN;
    CSCTL6 = XT1DRIVE_0;
    return 1;
}

static void init_rtc(void)
{
    RTCCTL = RTCSS__DISABLED;
    RTCMOD = RTC_TICKS_5S;
    RTCCTL = RTCSS__XT1CLK | RTCPS__1024 | RTCIE | RTCSR;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    init_gpio();
    (void)init_xt1();
    init_rtc();
    __enable_interrupt();

    while (1)
    {
    }
}

void __attribute__((interrupt(RTC_VECTOR))) RTC_ISR(void)
{
    switch (RTCIV)
    {
    case RTCIV__RTCIFG:
        sleep_pin_toggle();
        break;
    default:
        break;
    }
}
