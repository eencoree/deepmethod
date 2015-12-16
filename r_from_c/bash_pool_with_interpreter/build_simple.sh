gcc -g -Wall -o simple simple.c  $(pkg-config --cflags --libs glib-2.0)
