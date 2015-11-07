gcc -I/usr/share/R/include/ -c -ggdb simple.c
gcc -o simple simple.o  -L/usr/lib/R/lib -lR
LD_LIBRARY_PATH=/usr/lib/R/lib R_HOME=/usr/lib/R ./simple
