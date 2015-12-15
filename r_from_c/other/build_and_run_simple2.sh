#!/bin/bash

gcc -I/usr/share/R/include/ -c -ggdb simple2.c
gcc -o simple2 simple2.o  -L/usr/lib/R/lib -lR
LD_LIBRARY_PATH=/usr/lib/R/lib R_HOME=/usr/lib/R ./simple2
