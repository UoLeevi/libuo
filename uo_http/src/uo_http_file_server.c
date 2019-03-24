#include "uo_http_file_server.h"
#include "uo_refcount.h"
#include "uo_util.h"
#include "uo_buf.h"
#include "uo_macro.h"
#include "uo_mem.h"

#include <errno.h>
#include <time.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <pthread.h>
#include <sys/stat.h>

// TODO
// create mechanism to make cache entries expire after some time has passed and file has not been accessed

typedef struct uo_http_file_cache_entry
{
    uo_refcount *refcount;
    char *filename;
    char *uri;
    const char *filetype;
    char *data;
    time_t mtime;
    size_t size;
    pthread_mutex_t mtx;
} uo_http_file_cache_entry;

uo_http_file_server *uo_http_file_server_create(
    const char *dirname,
    size_t cache_size)
{
    struct stat sb;
    if (stat(dirname, &sb) == -1 || !S_ISDIR(sb.st_mode))
        return false;

    uo_http_file_server *http_file_server = calloc(1, sizeof *http_file_server);
    uo_strhashtbl_create_at(&http_file_server->cache, 0);
    pthread_mutex_t *mtx = http_file_server->mtx = malloc(sizeof *mtx);
    pthread_mutex_init(mtx, NULL);
    http_file_server->cache_space = cache_size;
    http_file_server->dirname = strdup(dirname);
    return http_file_server;
}

static uo_http_file_cache_entry *uo_http_file_cache_entry_create(
    const char *uri,
    const char *filename)
{
    struct stat sb;
    if (stat(filename, &sb) == -1 || !S_ISREG(sb.st_mode))
        return NULL;

    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return NULL;

    char *p = malloc(sb.st_size + 1);

    if (fread(p, sizeof *p, sb.st_size, fp) != sb.st_size || ferror(fp))
    {
        free(p);
        fclose(fp);
        return NULL;
    }

    fclose(fp);

    uo_http_file_cache_entry *cache_entry = calloc(1, sizeof *cache_entry);
    cache_entry->refcount = uo_refcount_create(p, free);
    cache_entry->data = p;
    cache_entry->filename = strdup(filename);
    cache_entry->uri = strdup(uri);
    cache_entry->mtime = sb.st_mtime;
    cache_entry->size = sb.st_size;
    p[sb.st_size] = '\0';

    const char *file_extension = strrchr(filename, '.');
    size_t file_extension_len;

    if (file_extension && (file_extension_len = strlen(file_extension)) > 1)
    {
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types
        switch (tolower(file_extension[1]))
        {
            case 'h': cache_entry->filetype = "text/html; charset=utf-8";              break;
            case 'j': cache_entry->filetype = "application/javascript; charset=utf-8"; break;
            case 'c': cache_entry->filetype = "text/css; charset=utf-8";               break;
            case 's': cache_entry->filetype = "image/svg+xml; charset=utf-8";          break;
            default:  cache_entry->filetype = "application/octet-stream";              break;
        }
    }
    else
        cache_entry->filetype = "application/octet-stream";

    pthread_mutex_init(&cache_entry->mtx, NULL);

    return cache_entry;
}

static void uo_http_file_cache_entry_destroy(
    uo_http_file_cache_entry *cache_entry)
{
    free(cache_entry->filename);
    free(cache_entry->uri);
    uo_refcount_dec(cache_entry->refcount);
    pthread_mutex_unlock(&cache_entry->mtx);
    pthread_mutex_destroy(&cache_entry->mtx);
    free(cache_entry);
}

static bool uo_http_file_cache_entry_try_refresh(
    uo_http_file_cache_entry *cache_entry,
    struct stat *sb_new)
{
    FILE *fp = fopen(cache_entry->filename, "rb");
    if (!fp)
        return false;

    cache_entry->mtime = sb_new->st_mtime;
    size_t size = cache_entry->size = sb_new->st_size;
    
    uo_refcount_dec(cache_entry->refcount);

    char *p = cache_entry->data = malloc(size + 1);
    cache_entry->refcount = uo_refcount_create(p, free);

    if (fread(p, sizeof *p, size, fp) != size || ferror(fp))
    {
        fclose(fp);
        return false;
    }

    fclose(fp);
    p[size] = '\0';

    return true;
}

static uo_http_file_cache_entry *uo_http_file_server_get_cache_entry(
    uo_http_file_server *http_file_server,
    const char *uri)
{
    pthread_mutex_lock(http_file_server->mtx);
    uo_http_file_cache_entry *cache_entry = uo_strhashtbl_get(&http_file_server->cache, uri);
    pthread_mutex_unlock(http_file_server->mtx);

    if (cache_entry)
        pthread_mutex_lock(&cache_entry->mtx);

    return cache_entry;
}

static bool uo_http_file_server_try_set_cache_entry(
    uo_http_file_server *http_file_server,
    uo_http_file_cache_entry *cache_entry)
{
    pthread_mutex_lock(http_file_server->mtx);

    uo_http_file_cache_entry *dup_cache_entry = uo_strhashtbl_remove(&http_file_server->cache, cache_entry->uri);

    if (dup_cache_entry)
    {
        pthread_mutex_lock(&dup_cache_entry->mtx);
        http_file_server->cache_space += dup_cache_entry->size;
        uo_http_file_cache_entry_destroy(dup_cache_entry);
    }

    if (http_file_server->cache_space < cache_entry->size)
    {
        pthread_mutex_unlock(http_file_server->mtx);
        return false;
    }

    uo_strhashtbl_set(&http_file_server->cache, cache_entry->uri, cache_entry);
    http_file_server->cache_space -= cache_entry->size;
    pthread_mutex_unlock(http_file_server->mtx);
    return true;
}

static uo_http_file_cache_entry *uo_http_file_server_remove_cache_entry(
    uo_http_file_server *http_file_server,
    const char *uri)
{
    pthread_mutex_lock(http_file_server->mtx);
    uo_http_file_cache_entry *cache_entry = uo_strhashtbl_remove(&http_file_server->cache, uri);
    http_file_server->cache_space += cache_entry->size;
    pthread_mutex_unlock(http_file_server->mtx);
    return cache_entry;
}

void uo_http_file_server_destroy(
    uo_http_file_server *http_file_server)
{
    uo_strkvp_linklist *head = uo_strhashtbl_list(&http_file_server->cache);

    while (head != uo_strkvp_linklist_next(head)) 
    {
        uo_strkvp_linklist *strkvp_linklist = uo_strkvp_linklist_next(head);
        uo_linklist_unlink(&strkvp_linklist->link);
        uo_http_file_cache_entry *cache_entry = strkvp_linklist->item.value;
        pthread_mutex_lock(&cache_entry->mtx);
        uo_http_file_cache_entry_destroy(cache_entry);
    }

    uo_strhashtbl_destroy_at(&http_file_server->cache);
    pthread_mutex_destroy(http_file_server->mtx);
    free(http_file_server->mtx);
    free((void *)http_file_server->dirname);
    free(http_file_server);
}

void uo_http_file_server_serve(
    uo_http_file_server *http_file_server,
    uo_http_conn *http_conn)
{
    uo_http_req *http_req = &http_conn->http_req;
    uo_http_res *http_res = &http_conn->http_res;

    const char *uri = http_req->uri;
    uo_http_file_cache_entry *cache_entry = uo_http_file_server_get_cache_entry(http_file_server, uri);

    struct stat sb;

    if (cache_entry)
    {
        if (stat(cache_entry->filename, &sb) == -1)
        {
            // the file is no longer valid
            uo_http_file_server_remove_cache_entry(http_file_server, uri);
            uo_http_file_cache_entry_destroy(cache_entry);
            uo_http_res_set_status_line(http_res, UO_HTTP_404, UO_HTTP_VER_1_1);
            uo_http_res_set_content(http_res, "404 Not Found", "text/plain", UO_STRLEN("404 Not Found"));
        }
        else if (sb.st_mtime != cache_entry->mtime)
        {
            // the file has been modified
            if (uo_http_file_cache_entry_try_refresh(cache_entry, &sb))
            {
                uo_http_res_set_content_ref(http_res, cache_entry->data, cache_entry->filetype, cache_entry->size, cache_entry->refcount);
                pthread_mutex_unlock(&cache_entry->mtx);
            }
            else
            {
                // error while reloading the file
                uo_http_file_server_remove_cache_entry(http_file_server, uri);
                uo_http_file_cache_entry_destroy(cache_entry);
                uo_http_res_set_status_line(http_res, UO_HTTP_404, UO_HTTP_VER_1_1);
                uo_http_res_set_content(http_res, "404 Not Found", "text/plain", UO_STRLEN("404 Not Found"));
            }
        }
        else
        {
            uo_http_res_set_status_line(http_res, UO_HTTP_200, UO_HTTP_VER_1_1);
            uo_http_res_set_content_ref(http_res, cache_entry->data, cache_entry->filetype, cache_entry->size, cache_entry->refcount);
            pthread_mutex_unlock(&cache_entry->mtx);
        }

        return;
    }

    // no cache entry was found so create one

    uo_buf filename_buf = uo_buf_alloc(0x100);
    const char *dirname = http_file_server->dirname;
    uo_buf_memcpy_append(&filename_buf, dirname, strlen(dirname));

    char *p = uo_buf_get_ptr(filename_buf);
    size_t uri_len = strlen(uri);

    uo_buf_memcpy_append(&filename_buf, uri, uri_len);
    p = uo_uri_decode(p, uri, uri_len);

    if (uri[uri_len - 1] == '/')
        uo_buf_memcpy_append(&filename_buf, "index.html", 11);
    else
        *p = '\0';

    cache_entry = uo_http_file_cache_entry_create(uri, filename_buf);
    uo_buf_free(filename_buf);

    if (cache_entry)
    {
        uo_http_res_set_status_line(http_res, UO_HTTP_200, UO_HTTP_VER_1_1);
        uo_http_res_set_content_ref(http_res, cache_entry->data, cache_entry->filetype, cache_entry->size, cache_entry->refcount);

        if (!uo_http_file_server_try_set_cache_entry(http_file_server, cache_entry))
        {
            pthread_mutex_lock(&cache_entry->mtx);
            uo_http_file_cache_entry_destroy(cache_entry);
        }
    }
    else
    {
        uo_http_res_set_status_line(http_res, UO_HTTP_404, UO_HTTP_VER_1_1);
        uo_http_res_set_content(http_res, "404 Not Found", "text/plain", UO_STRLEN("404 Not Found"));
    }

}
