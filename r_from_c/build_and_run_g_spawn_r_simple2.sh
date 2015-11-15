gcc -Wall -o g_spawn_r_simple2 g_spawn_r_simple2.c $(pkg-config --cflags --libs glib-2.0)
gcc -I/usr/share/R/include/ -c -ggdb simple2.c
gcc -o simple2 simple2.o  -L/usr/lib/R/lib -lR
./g_spawn_r_simple2
