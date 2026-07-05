# ikea-skatgas-upgrade

## Motivation

IKEA Skatgås is a lovely rechargeable tea light, but it does not include a timer
that turns the LED off after 4 or 6 hours and keeps it off for the rest of the
day. This project adds that missing feature.

The original circuit uses a MicrOne ME2188 step-up converter to generate 3.3 V
from the 1.2 V NiMH battery. On the 3.3 V rail there is a proprietary SOIC-8 IC
that creates the LED flicker effect. It also detects undervoltage at around
0.7 V and keeps the LED off until the candle is physically switched off, even if
the battery voltage later recovers.

The timer could have been implemented in the proprietary IC, together with a
32.768 kHz external oscillator, but the stock candle does not include that
feature.

The step-up converter and the IKEA IC remain powered even when the physical
switch is off. In the off state the circuit draws about 45 µA. When switched on,
but with the LED inactive, it draws about 150 µA.

The Skatgås contains a 350 mAh battery. The goal is to run the tea light for two
weeks on a single charge, assuming 4 active hours per day.

## My Solution

- Use an MSP430FR2311 powered from the ME2188 3.3 V rail.
- Connect the MCU ground through the main switch, which switches ground at TP1.
  When the switch is off, the MCU is not running.
- Let the MCU control the IKEA IC inhibit pin, with pin 2 lifted from the board.
  Driving the inhibit pin low enables the IKEA IC. Leaving it in High-Z disables
  the IKEA IC and reduces the current draw to about 45 µA.
- Replace the original 110 Ohm LED series resistor with a 470 Ohm resistor.
- Keep the MSP430 in deep sleep, LPM3, during the 4+20 hour timing cycle. In this
  state it draws less than 1 µA.
- On startup, play a short blink pattern by toggling the inhibit pin. Four blinks
  indicate 4 active hours per day; six blinks indicate 6 active hours per day.
- Switch between the two timing modes by turning the candle off during the
  startup pattern.

## History

The original prototype used an MSP430G2210, but that MCU does not have an RTC or
support for a 32.768 kHz external low-speed oscillator. Without a precise
external clock, the internal VLO needs calibration and can still drift by several
minutes per day.

## Measurements

| | Original Skatgås | IKEA IC disconnected | MSP430G2210 + 470 Ohm |
|-|------------------|----------------------|-----------------------|
| 1.2 V active | 10.2 mA | | 5 mA |
| 1.2 V inactive | | 29.4 µA | 32 µA |
| 0.9 V active | 13.5 mA | | 6.5 mA |
| 0.9 V inactive | 13.5 mA | 44.2 µA | 45 µA |
| 0.7 V off | 300 µA | | |
| 0.9 V locked off | 220 µA | | |
| 1.2 V locked off | 150 µA | | |
| 1.2 V switched off | 30 µA | | |
| 0.9 V switched off | 45 µA | | |

The MSP430G2210 draws about 215 µA at 3.3 V when active and about 0.7 µA in LPM3.

## Battery Life Estimate

The Skatgås uses a 1.2 V, 350 mAh NiMH battery. The design target is **14 days**
of runtime with the LED active for **4 hours per day**.

### Available Capacity

```text
Battery capacity = 350 mAh
Target runtime   = 14 days

Maximum daily budget:
350 mAh / 14 = 25 mAh/day
```

### Daily Consumption

Assuming the candle is active for 4 hours and asleep for the remaining 20 hours:

**Worst case, at 0.9 V battery voltage:**

```text
Active:
6.5 mA x 4 h = 26.0 mAh

Sleep:
45 µA x 20 h = 0.9 mAh

Total:
26.9 mAh/day
```

**Typical case, with a fresh 1.2 V battery:**

```text
Active:
5.0 mA x 4 h = 20.0 mAh

Sleep:
30 µA x 20 h = 0.6 mAh

Total:
20.6 mAh/day
```

### Estimated Runtime

| Condition | Daily consumption | Estimated runtime |
|-----------|------------------:|------------------:|
| Typical, 1.2 V | 20.6 mAh/day | **~17 days** |
| Worst case, 0.9 V | 26.9 mAh/day | **~13 days** |

In practice, the battery spends most of its discharge cycle close to 1.2 V and
only reaches 0.9 V near the end. The expected runtime is therefore between
13 and 17 days, which makes the 2-week target realistic.

## Dependencies

The firmware depends on TI's open-source MSP430 GCC toolchain:

```text
https://www.ti.com/tool/MSP430-GCC-OPENSOURCE#downloads
```

## Build and Flash

Build the firmware from the `firmware` directory:

```bash
./build.sh
```

Select the target device in `build.sh`.

Connect to the debugger from the terminal:

```bash
mspdebug rf2500
```

After connection, `mspdebug` opens an interactive prompt. A successful connection
looks similar to this:

```text
MSPDebug version 0.26 - debugging tool for MSP430 MCUs
Copyright (C) 2009-2017 Daniel Beer <dlbeer@gmail.com>
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
Chip info database from MSP430.dll v3.15.0.1 Copyright (C) 2013 TI, Inc.

Initializing FET...
FET protocol version is 30394216
Set Vcc: 3000 mV
Configured for Spy-Bi-Wire
fet: FET returned error code 4 (Could not find device or device not supported)
fet: command C_IDENT1 failed
Using Olimex identification procedure
Device ID: 0xf201
  Code start address: 0xf800
  Code size         : 2048 byte = 2 kb
  RAM  start address: 0x200
  RAM  end   address: 0x27f
  RAM  size         : 128 byte = 0 kb
Device: F20x1_G2x0x_G2x1x
Number of breakpoints: 2
fet: FET returned NAK
warning: device does not support power profiling
Chip ID data:
  ver_id:         01f2
  ver_sub_id:     0000
  revision:       60
  fab:            70
  self:           0000
  config:         01
  fuses:          00
Device: F20x1_G2x0x_G2x1x
```

Then run the following commands in the interactive prompt:

```text
erase
prog main.elf
run
exit
```

## Schematic

## Images
