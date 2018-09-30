gcc -c src/uo_stack.c `
    -o lib/uo_stack.o `
    -g
ar rcs lib/libuo_stack.a `
    lib/uo_stack.o
    