gcc -Wall -o g_spawn_ls_example g_spawn_ls_example.c $(pkg-config --cflags --libs glib-2.0)
./g_spawn_ls_example
