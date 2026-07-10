#include <msp430.h>

#define SLEEP_PIN BIT5
#define TEST_PIN BIT2

#define XT1_PIN_MASK (BIT6 | BIT7)
#define XT1_STARTUP_ATTEMPTS 200U
#define XT1_SETTLE_CYCLES 10000U
#define RTC_TICKS_5S 159U
#define RTC_TICKS_10S 319U
#define AWAKE_05S_CYCLES 500000UL

static volatile unsigned char stored_mode __attribute__((section(".persistent"))) = 4;

static void fram_write_enable(void)
{
    SYSCFG0 = FRWPPW;
}

static void fram_write_protect(void)
{
    SYSCFG0 = FRWPPW | PFWP;
}

static void write_stored_mode(unsigned char mode)
{
    fram_write_enable();
    stored_mode = mode;
    fram_write_protect();
}

static unsigned char normalize_mode(unsigned char mode)
{
    if ((mode != 4U) && (mode != 6U))
    {
        mode = 4U;
        write_stored_mode(mode);
    }

    return mode;
}

static unsigned char opposite_mode(unsigned char mode)
{
    return (mode == 4U) ? 6U : 4U;
}

static void awake(void)
{
    P1OUT &= ~SLEEP_PIN;
    P1REN &= ~SLEEP_PIN;
    P1DIR |= SLEEP_PIN;
}

static void sleep(void)
{
    /* Hi-Z needed for OFF cycles since IKEA IC has an internal pullup */
    P1OUT &= ~SLEEP_PIN;  // Preload low for next activation
    P1REN &= ~SLEEP_PIN;  // Disable internal pull-up/down
    P1DIR &= ~SLEEP_PIN;  // Input = High-Z
}

static void delay_05s(void)
{
    __delay_cycles(AWAKE_05S_CYCLES);
}

static void startup_pattern(unsigned char count)
{
    unsigned char n;

    for (n = 0; n < count; n++)
    {
        awake();
        delay_05s();

        sleep();
        delay_05s();
    }
}

static void init_gpio(void)
{
    P1OUT = 0x00;
    P1DIR = 0xFF;
    P1REN = 0x00;
    P1SEL0 = 0x00;
    P1SEL1 = 0x00;

    sleep();

    P2OUT = 0x00;
    P2DIR = (unsigned char)~XT1_PIN_MASK;
    P2REN = 0x00;
    P2SEL0 = 0x00;
    P2SEL1 = XT1_PIN_MASK;

    PJOUT = 0x0000;
    PJDIR = 0xFFFF;
    PJREN = 0x0000;
    PJSEL0 = 0x0000;
    PJSEL1 = 0x0000;

    PM5CTL0 &= ~LOCKLPM5;
}

static unsigned char init_xt1(void)
{
    unsigned int attempts = XT1_STARTUP_ATTEMPTS;

    CSCTL4 = SELA__XT1CLK;
    CSCTL6 = XT1DRIVE_3 | XT1BYPASS_0 | XT1AGCOFF_0 | XT1AUTOOFF_0;
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
    CSCTL6 = XT1DRIVE_0 | XT1BYPASS_0 | XT1AGCOFF_0 | XT1AUTOOFF_0;
    return 1;
}

static void init_rtc(void)
{
    RTCCTL = RTCSS__DISABLED;
    RTCMOD = RTC_TICKS_5S;
    RTCCTL = RTCSS__XT1CLK | RTCPS__1024 | RTCIE | RTCSR;
}

static void set_rtc_period(unsigned int ticks)
{
    RTCCTL |= RTCSR;
    RTCMOD = ticks;
}

int main(void)
{
    unsigned char mode;

    WDTCTL = WDTPW | WDTHOLD;

    init_gpio();

    mode = normalize_mode(stored_mode);
    write_stored_mode(opposite_mode(mode));

    awake();
    startup_pattern(mode);

    write_stored_mode(mode);

    awake();
    (void)init_xt1();
    init_rtc();

    while (1)
    {
        __bis_SR_register(LPM3_bits | GIE);
        __no_operation();
    }
}

void __attribute__((interrupt(RTC_VECTOR))) RTC_ISR(void)
{
    switch (RTCIV)
    {
    case RTCIV__RTCIFG:
        if (P1DIR & SLEEP_PIN)
        {
            // currently ON
            sleep();
            set_rtc_period(RTC_TICKS_10S);
        }
        else
        {
            // currently OFF
            awake();
            set_rtc_period(RTC_TICKS_5S);
        }
        break;
    default:
        break;
    }
}
