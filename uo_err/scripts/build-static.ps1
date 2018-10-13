gcc -c src/uo_hashtbl.c `
    -o lib/uo_hashtbl.o `
    -g
ar rcs lib/libuo_hashtbl.a `
    lib/uo_hashtbl.o