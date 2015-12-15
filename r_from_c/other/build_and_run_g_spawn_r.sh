gcc -Wall -o g_spawn_r g_spawn_r.c $(pkg-config --cflags --libs glib-2.0)
./g_spawn_r
