# ikea-skatgas-upgrade

## Motivation

IKEA Skatgås is a great tea light but unfortunately it doesn't have a timer to turn off after 4 or 6 hours for the rest of the day. The project aims to fix this missing feature.

The tea light has a MicrOne ME2188 step-up converter that creates 3V3 from the 1V2 of the NiMh battery. On the 3V3 rail there is a proprietary SOIC-8 IC that does the LED flickering effect and also detects undervoltage at 0.7V the keep the LED turned off even if battery voltage comes back to normal until it's physically turned off.

It's clear that IKEA could have been implementing the timer function in this proprietary IC but intentionally didn't do so.

## My solution

- Use an MSP430FR2311 supplied from the 3V3 rail of the ME2188
- Original idea was an MSP430G2210 but that MCU lacks the RTC and 32.768kHz external LSE oscillator support. Without a precise external oscillator the internal VLO needs calibration and even after a calibration it drifts several minutes a day.
- There are 2 options for the startup at the moment:  
  1) The MCU GND is coming from the main switch --> switch OFF == MCU is not running. The MCU starts as soon as the main switch is turned on. The only disadvantage is that we cannot play the charging feedback flickering of the Skatgas.
  2) We run the MCU all the time from the system GND and monitor the main switch with a GPIO and wake up on a change of main switch. Disadvantage is partially the same, if the custom circuit is in its own off period the Skatgas charging feedback won't be visible. To fix this charging should be also monitored, which I found as an overkill for this project.
- The original resistor from IKEA is 110Ohm in the ground path, I changed it to 150Ohm
- Since LED max current at 3V3 with the 150 Ohm series resistor is only 4.2mA and MSP430 GPIOs are rated to 6mA I decided not to use any additional FET for switching the LED cathode.
- During the 4+20 hours the MSP430 is in deep sleep (LPM3)
- Nomnal quiescent current of the ME2188 is 13uA, I measured overall 8-9uA after decreasing battery voltage below 0.7V as the combined budget for the ME2188 and the unknown IKEA IC.
- I expect that my additional circuit with the MSP430 in LPM3 will only add maximum 1uA quiescent consumption.

## Measurements

- IKEA LED at 3.3V with 150 Ohm resistor --> 4.2mA

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