# ikea-skatgas-upgrade

## Motivation

IKEA Skatgås is a great tea light but unfortunately it doesn't have a timer to turn off after 4 or 6 hours for the rest of the day. The project aims to fix this missing feature.

The tea light has a MicrOne ME2188 step-up converter that creates 3V3 from the 1V2 of the NiMh battery. On the 3V3 rail there is a proprietary SOIC-8 IC that does the LED flickering effect and also detects undervoltage at 0.7V the keep the LED turned off even if battery voltage comes back to normal until it's physically turned off.

IKEA could have been implementing the timer function together with a 32.768kHz external oscillator in this proprietary IC but intentionally didn't do so.

The step-up converter and the IKEA IC are always on even when the physical switch is turned off. When turned off the circuit's current consumption is 45uA when turned on (but without active LED) it's about 150uA.

The Skatgås has a 350mAh battery and the goal is to run a tealight for 2 weeks (with 4 active hours a day) with a single charge.

## My solution

- Use an MSP430FR2311 supplied from the 3V3 rail of the ME2188
- The MCU GND is coming from the main switch which is a switched GND (TP1). Switch OFF == MCU is not running.
- When the MCU is running it drives the inhibit pin of the IKEA IC (lifted pin2) to low, which enables the IKEA IC. Otherwise if MCU is off or LED needs to be turned off the inhibit pin is driven to High-Z, which sends the IKEA IC to sleep with about 45uA current consumption only.
- The original LED series resistor from IKEA is 110Ohm, I changed it to 47Ohm.
- During the 4+20 hours the MSP430 is in deep sleep (LPM3) with less than 1uA current consumption.
- During startup (after turning on the main switch) my MSP430 plays a pattern by turning on and off the inhibit pin. 4 blinks means 4 active hours a day, 6 blinks means 6 active hours a day.
- The 2 timing mode can be switched by turning off during this startup pattern.

## History

- Original idea was an MSP430G2210 but that MCU lacks the RTC and 32.768kHz external LSE oscillator support. Without a precise external oscillator the internal VLO needs calibration and even after a calibration it drifts several minutes a day.

## Measurements

| | OG Skatgås | IKEA IC disconnected | MSP430G2210 + 470Ohm |
|-|------------|----------------------|----------------------|
| 1.2V active | 10.2mA | | 5mA |
| 1.2V inactive | | 29.4uA | 32uA |
| 0.9V active | 13.5 mA | | 6.5mA |
| 0.9V inactive | 13.5 mA | 44.2uA | 45uA |
| 0.7V (off) | 300 uA |
| 0.9V (locked off) | 220 uA |
| 1.2V (locked off) | 150 uA |
| 1.2V turned off | 30uA |  
| 0.9V turned off | 45uA | 

MSP430G2210 current consumption @ 3.3V is 215uA when active and 0.7uA in LPM3.

## Battery Life Estimation

The Skatgås uses a 1.2 V / 350 mAh NiMH battery. The design target is to operate for **14 days** with the LED active for **4 hours per day**.

### Available Energy

```
Battery capacity = 350 mAh
Target runtime   = 14 days

Maximum daily budget:
350 mAh / 14 = 25 mAh/day
```

### Daily Consumption

Assuming the candle is active for 4 hours and sleeping for the remaining 20 hours:

**Worst case (0.9 V battery):**

```
Active:
6.5 mA × 4 h = 26.0 mAh

Sleep:
45 µA × 20 h = 0.9 mAh

Total:
26.9 mAh/day
```

**Typical (fresh battery at 1.2 V):**

```
Active:
5.0 mA × 4 h = 20.0 mAh

Sleep:
30 µA × 20 h = 0.6 mAh

Total:
20.6 mAh/day
```

### Estimated Runtime

| Condition | Daily consumption | Estimated runtime |
|-----------|------------------:|------------------:|
| Typical (1.2 V) | 20.6 mAh/day | **≈17 days** |
| Worst case (0.9 V) | 26.9 mAh/day | **≈13 days** |

In practice the battery spends most of its discharge cycle close to **1.2 V**, only reaching **0.9 V** near the end of discharge. Therefore the expected runtime is between **13 and 17 days**, making the **2-week target realistic**.

## Dependency

The project depends on the opensource toolchain for MSP430 from TI:
```
https://www.ti.com/tool/MSP430-GCC-OPENSOURCE#downloads
```

## Build and run

Build the project with `./build.sh`, select device in this file.

Connect to the debugger from the terminal:
```bash
mspdebug rf2500
```

After connection it should look like this and it will open a debugger prompt:

```
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

The in the interactive prompt:

```
erase
prog main.elf
run
exit
```

## Schematic

## Images