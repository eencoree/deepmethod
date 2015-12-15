gcc -Wall -o hello_glib hello_glib.c $(pkg-config --cflags --libs glib-2.0)
./hello_glib
