#!/bin/bash
#
# USAGE: $ ./build.sh 5.5.0

particle usb dfu
particle compile tracker ../code --target $1 --saveTo tracker-prize-wheel@$1.bin
particle flash --usb tracker-prize-wheel@$1.bin

# All done, ding!
tput bel
