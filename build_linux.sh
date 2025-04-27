#!/bin/bash

#
# build_linux.sh
#
# Caleb Barger
# 04/22/2025
#

set -e
gcc linux_vkcalc.c -g -lm -o vkcalc
