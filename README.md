# ikea-skatgas-upgrade

## Motivation

IKEA Skatgås is a great tea light but unfortunately it doesn't have a timer to turn off after 4 or 6 hours for the rest of the day. The project aims to fix this missing feature.

The tea light has a MicrOne ME2188 step-up converter that creates 3V3 from the 1V2 of the NiMh battery. On the 3V3 rail there is a proprietary SOIC-8 IC that does the LED flickering and also detects undervoltage at 0.7V the keep the LED turned off even if battery voltage comes back to normal until it's physically turned off.

It's clear that IKEA could have been implementing the timer function in this IC but intentionally didn't do it.

## My solution

- Use an MSP430G2210 supplied from the 3V3 rail of the ME2188
- After startup turn on the LED by applying ground on the cathode of the LED with a 2N7002 and a series resistor of 110Ohm
- The original resistor from IKEA is 110Ohm in the ground path, in my own circuit I'll probably increase the resistance a bit to save some battery
- For 4 hours the 2N7002 is turned on then it's turned off for 20 hours
- During the 4+20 hours the MSP430 is in deep sleep
- Nomnal quiescent current of the ME2188 is 13uA, I measured overall 8-9uA after decreasing battery voltage below 0.7V as the combined budget for the ME2188 and the unknown IKEA IC.
- I expect that my additional circuit with the MSP430 in LPM3 will only add maximum 4.5uA quiescent consumption when the LED is turned on and only 1uA when it's turned off. (1uA for the MSP430 and 3.3uA for the 1MOhm pull-down resistor at the 2N7002.)

## Dependency

The project depends on the opensource toolchain for MSP430 from TI:
```
https://www.ti.com/tool/MSP430-GCC-OPENSOURCE#downloads
```

## Schematic

## Images