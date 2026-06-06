#!/bin/bash

set -e

TOOL=/Users/david.dudas/Desktop/Development/Toolchain/msp430-gcc-9.3.1.11_macos
SUPPORT=/Users/david.dudas/Desktop/Development/Toolchain/msp430-gcc-support-files

$TOOL/bin/msp430-elf-gcc \
    -mmcu=msp430g2210 \
    -I$SUPPORT/include \
    -L$SUPPORT/include \
    -Os \
    -g \
    main.c \
    -o main.elf

echo ""
echo "Build successful"
echo ""
$TOOL/bin/msp430-elf-size main.elf