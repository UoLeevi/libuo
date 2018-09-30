gcc -c src/uo_http_res.c `
    -o lib/uo_http_res.o `
    -g
gcc -c src/uo_httpc.c `
    -I ../uo_cb/include `
    -I ../uo_sock/include `
    -o lib/uo_httpc.o `
    -g
ar rcs lib/libuo_httpc.a `
    lib/uo_http_res.o `
    lib/uo_httpc.o `
    ../uo_sock/lib/uo_sock.o `
    ../uo_cb/lib/uo_cb.o
