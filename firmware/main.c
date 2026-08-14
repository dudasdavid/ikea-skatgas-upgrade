#include <msp430.h>

#define IR_PIN BIT2

#define XT1_PIN_MASK (BIT6 | BIT7)
#define XT1_STARTUP_ATTEMPTS 200U
#define XT1_SETTLE_CYCLES 10000U
#define RTC_TICKS_30MIN 57599U
#define AWAKE_05S_CYCLES 500000UL
#define STARTUP_DELAY_CYCLES 1600000UL
#define IR_VALUE_CYCLES 902UL
#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

#define CHUNKS_PER_HOUR 2U
#define ON_CHUNKS_4H (4U * CHUNKS_PER_HOUR)
#define OFF_CHUNKS_20H (20U * CHUNKS_PER_HOUR)
#define ON_CHUNKS_6H (6U * CHUNKS_PER_HOUR)
#define OFF_CHUNKS_18H (18U * CHUNKS_PER_HOUR)

static volatile unsigned char stored_mode __attribute__((section(".persistent"))) = 4;
static unsigned int on_chunks;
static unsigned int off_chunks;
static unsigned int elapsed_chunks;
static unsigned char is_on;
static const unsigned char off_pattern[] =
{
    0U, 1U, 0U, 1U, 0U, 1U, 0U, 1U,
    0U, 1U, 0U, 1U, 0U, 1U, 0U, 0U,
    1U, 0U, 1U, 1U, 0U, 0U, 1U, 0U,
    1U, 0U
};
static const unsigned char on_pattern[] =
{
    0U, 1U, 0U, 1U, 0U, 1U, 0U, 1U,
    0U, 1U, 0U, 1U, 0U, 1U, 0U, 0U,
    1U, 0U, 1U, 0U, 1U, 0U, 1U, 0U,
    1U, 1U, 0U
};

static void output_ir_pattern(const unsigned char *values, unsigned int count);

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

static void configure_mode_timing(unsigned char mode)
{
    if (mode == 6U)
    {
        on_chunks = ON_CHUNKS_6H;
        off_chunks = OFF_CHUNKS_18H;
    }
    else
    {
        on_chunks = ON_CHUNKS_4H;
        off_chunks = OFF_CHUNKS_20H;
    }
}

static void awake(void)
{
    output_ir_pattern(on_pattern, ARRAY_LENGTH(on_pattern));
    is_on = 1U;
}

static void sleep(void)
{
    output_ir_pattern(off_pattern, ARRAY_LENGTH(off_pattern));
    is_on = 0U;
}

static void delay_05s(void)
{
    __delay_cycles(AWAKE_05S_CYCLES);
}

static void output_ir_pattern(const unsigned char *values, unsigned int count)
{
    unsigned int n;

    for (n = 0; n < count; n++)
    {
        if (values[n] == 0U)
        {
            P1OUT |= IR_PIN;
        }
        else
        {
            P1OUT &= ~IR_PIN;
        }

        __delay_cycles(IR_VALUE_CYCLES);
    }

    P1OUT &= ~IR_PIN;
}

static void startup_pattern(unsigned char count)
{
    unsigned char n;

    for (n = 0; n < count; n++)
    {
        sleep();
        delay_05s();

        awake();
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

    P1OUT &= ~IR_PIN;
    P1DIR |= IR_PIN;

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

    do
    {
        CSCTL7 &= ~XT1OFFG;
        SFRIFG1 &= ~OFIFG;
        __delay_cycles(XT1_SETTLE_CYCLES);
    }
    while ((SFRIFG1 & OFIFG) && --attempts);

    if (SFRIFG1 & OFIFG)
    {
        return 0;
    }

    CSCTL6 = XT1DRIVE_0 | XT1BYPASS_0 | XT1AGCOFF_0 | XT1AUTOOFF_0;
    return 1;
}

static void init_rtc(void)
{
    RTCCTL = RTCSS__DISABLED;
    RTCMOD = RTC_TICKS_30MIN;
    RTCCTL = RTCSS__XT1CLK | RTCPS__1024 | RTCIE | RTCSR;
}

int main(void)
{
    unsigned char mode;

    WDTCTL = WDTPW | WDTHOLD;

    init_gpio();
    __delay_cycles(STARTUP_DELAY_CYCLES);

    mode = normalize_mode(stored_mode);
    configure_mode_timing(mode);
    write_stored_mode(opposite_mode(mode));

    awake();
    delay_05s();

    startup_pattern(mode);

    write_stored_mode(mode);

    elapsed_chunks = 0;
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
        elapsed_chunks++;

        if (is_on)
        {
            // currently ON
            if (elapsed_chunks >= on_chunks)
            {
                elapsed_chunks = 0;
                sleep();
            }
        }
        else
        {
            // currently OFF
            if (elapsed_chunks >= off_chunks)
            {
                elapsed_chunks = 0;
                awake();
            }
        }
        break;
    default:
        break;
    }
}
