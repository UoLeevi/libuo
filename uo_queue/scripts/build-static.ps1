gcc -c src/uo_queue.c `
    -o lib/uo_queue.o `
    -g
ar rcs lib/libuo_queue.a `
    lib/uo_queue.o
    