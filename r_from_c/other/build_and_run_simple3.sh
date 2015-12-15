#!/bin/bash

gcc -I/usr/share/R/include/ -c -ggdb simple3.c
gcc -o simple3 simple3.o  -L/usr/lib/R/lib -lR
LD_LIBRARY_PATH=/usr/lib/R/lib R_HOME=/usr/lib/R ./simple3
