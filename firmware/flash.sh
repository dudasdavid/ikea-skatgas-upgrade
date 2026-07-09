#!/bin/bash

set -e

FLASHER_DIR=/Users/david.dudas/ti/MSPFlasher_1.3.20
FLASHER=$FLASHER_DIR/MSP430Flasher
IMAGE=main.hex

if [ ! -f "$IMAGE" ]; then
    ./build.sh
fi

DYLD_LIBRARY_PATH=$FLASHER_DIR \
"$FLASHER" \
    -n MSP430FR2311 \
    -w "$IMAGE" \
    -v \
    -z "[VCC,RESET]"
