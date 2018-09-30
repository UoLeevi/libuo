gcc -c src/uo_cb.c `
    -I ../uo_queue/include `
    -I ../uo_hashtbl/include `
    -o lib/uo_cb.o `
    -g
ar rcs lib/libuo_cb.a `
    lib/uo_cb.o `
    ../uo_queue/lib/uo_queue.o `
    ../uo_hashtbl/lib/uo_hashtbl.o
