gcc -o helper helper.c
gcc -o spawn spawn.c $(pkg-config --cflags --libs gtk+-2.0)
./spawn
