# ikea-skatgas-upgrade

## Motivation

IKEA Skatgås is a great rechargeable tea light, but it does not include a timer
that turns the LED off after 4 or 6 hours and keeps it off for the rest of the
day. This project aims to add this missing feature.

The original circuit uses a propriatery SOIC-8 ASIC (or OTP MCU) and a MicrOne ME2188
step-up converter to generate 3.3V from the 1.2V NiMH battery. The proprietary IC is on
the 3.3V rail and it creates the LED candle flickering effect.
It also detects charger connection (5V) and undervoltage at around
0.8V, it's keeping the LED off until the device is physically switched off, even if
the battery voltage later recovers.
Switch off doesn't turn off the boost converter or the proprietary IC, it's monitored instead
by the 2nd pin of the ASIC. Active low means ON and floating is OFF. In the OFF state
the circuit draws about 45 µA. When switched ON, but with the LED inactive, it draws about 150 µA.

The timer function could have been implemented in the proprietary IC, together with a
32.768 kHz external oscillator, but the stock candle does not include that
feature for whatever reason.

The Skatgås contains a 350 mAh battery. And the project's goal is to run the tea light for two
weeks on a single charge - assuming 4 active hours per day.

## My Solution

- Use an MSP430FR2311 powered from the ME2188 3.3V rail.
- Connect the MCU ground through the main switch, which switches the ground at TP1.
  When the switch is OFF, TP1 is floating the MCU is not running. When the switch is ON
  TP1 is connected to the GND.
- Let the MCU control the IKEA IC sleep/inhibit pin (2nd pin). This pin needs to be
  lifted from the board. Driving the sleep pin with active low enables the IKEA IC.
  Leaving it floating or driving with High-Z disables the IKEA IC and reduces the
  current consumption to about 45 µA from 150 µA.
- Replace the original 110 Ohm LED series resistor with a 1.6 kOhm resistor. This reduces
  the current consumption of the LED from 10 mA to 2.8 mA from the 1.2V battery.
- Keep the MSP430 in deep sleep (LPM3), during the 4+20 (or 6+18) hours timing cycle. In this
  state the MSP430 alone draws less than 1 µA at 3.3 V.
- At startup, play a short blink pattern by toggling the sleep pin. Four blinks
  indicate 4 active hours per day and six blinks indicate 6 active hours per day.
- Switch between the two timing modes by turning the candle off during the
  startup pattern. The actual selected mode is stored in the FRAM persistently.

## History

The original prototype used an MSP430G2210, but that MCU does not have an RTC or
support for a 32.768 kHz external low-speed oscillator. Without a precise
external clock, the internal VLO needs calibration and can still drift by several
minutes per day which is an unacceptable compromise.

## Measurements

| | Original Skatgås | IKEA IC disconnected | MSP430G2210 w/ 470 Ohm | MSP430FR2311 w/ 1.6kOhm |
|-|-|-|-|-|
| 1.2 V active | 10.2 mA | | 5 mA | 2.8 mA
| 1.2 V inactive | | 29.4 µA | 32 µA | 34 µA
| 0.9 V active | 13.5 mA | | 6.5 mA | 3.3 mA
| 0.9 V inactive | 13.5 mA | 44.2 µA | 45 µA | 41.5 µA
| 0.7 V off | 300 µA | | |
| 0.9 V locked off | 220 µA | | |
| 1.2 V locked off | 150 µA | | |
| 1.2 V switched off | 30 µA | | |
| 0.9 V switched off | 45 µA | | |

The MSP430G2210 current consumption was about 215 µA at 3.3V when active and about 0.7 µA in LPM3. The MSP430FR2311 with XT1 and RTC is only 25 nA in LPM3.

## Battery Life Estimate

The Skatgås uses a 1.2V, 350 mAh NiMH battery. The design target is **14 days**
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

**Worst case, at 0.9V battery voltage:**

```text
Active:
3.3 mA x 4 h = 13.2 mAh

Sleep:
45 µA x 20 h = 0.9 mAh

Total:
14.1 mAh/day
```

**Typical case, with a fresh 1.2V battery:**

```text
Active:
2.8 mA x 4 h = 11.2 mAh

Sleep:
35 µA x 20 h = 0.7 mAh

Total:
11.9 mAh/day
```

### Estimated Runtime

| Condition | Daily consumption | Estimated runtime |
|-----------|------------------:|------------------:|
| Typical, 1.2V | 11.9 mAh/day | **~29 days** |
| Worst case, 0.9V | 14.1 mAh/day | **~24 days** |

In practice, the battery spends most of its discharge cycle close to 1.2 V and
only reaches 0.9 V near the end. The expected runtime is therefore between
24 and 29 days, which outperfroms the 2-week target.

## Dependencies

The firmware depends on TI's open-source MSP430 GCC toolchain:

```text
https://www.ti.com/tool/MSP430-GCC-OPENSOURCE#downloads
```

And the MSP430-FLASHER:
```text
https://www.ti.com/tool/MSP430-FLASHER#downloads
```

## Build and Flash

Build the firmware from the `firmware` directory with:

```bash
./build.sh
```

Select the target device in `build.sh` (e.g MSP430G2210 or MSP430FR2311). The build script generates `.elf` and `.hex` outputs.

### Flash with MSP430 Flasher

On macOS, MSP430 Flasher may need `DYLD_LIBRARY_PATH` set so it can find TI's
debug library this needs to be specified in the flasher script:

```bash
./flash.sh
```

### Flash with MSPDebug

MSPDebug flashing was only working for the MSP430G2210 target. Connect to the debugger from the terminal. For MSP430G2210 targets, use the rf2500 driver:

```bash
mspdebug rf2500
```

For MSP430FR2311 targets, use the eZ-FET driver (although this was not working well for me):

```bash
mspdebug ezfet
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
prog main.elf
verify main.elf
reset
run
exit
```

## Schematic

## Images
