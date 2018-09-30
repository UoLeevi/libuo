gcc -c src/uo_sock.c `
    -o lib/uo_sock.o `
    -g
ar rcs lib/libuo_sock.a `
    lib/uo_sock.o
    