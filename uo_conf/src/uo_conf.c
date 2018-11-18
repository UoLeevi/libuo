#include "uo_conf.h"
#include "uo_err.h"
#include "uo_hashtbl.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static uint64_t uo_conf_hash(
    const void *key)
{
    const char *k = key;
    uint64_t h = 0;

    for (size_t i = 0; i < sizeof h && k[i]; ++i)
        h ^= k[i] << i; 

    return h;
}

static bool uo_conf_equals(
    const void *key0,
    const void *key1)
{
    return strcmp(key0, key1) == 0;
}

uo_conf *uo_conf_create(
	char *filename)
{
    uo_conf *conf = calloc(1, sizeof *conf);
    uo_hashtbl *hashtbl = conf->hashtbl = uo_hashtbl_create(0x100, uo_conf_hash, uo_conf_equals);

    struct stat sb;
    if (stat(filename, &sb) == -1)
        uo_err_goto(err_free, "Unable to open configuration file '%s'.", filename);

    FILE *fp = fopen(filename, "rb");
    if (!fp)
        uo_err_goto(err_free, "Unable to open configuration file '%s'.", filename);

    char *buf = malloc(sb.st_size + 1);
    buf[sb.st_size] = '\0';

    if (fread(buf, sizeof *buf, sb.st_size, fp) != sb.st_size || ferror(fp))
        uo_err_goto(err_close, "Unable to read configuration file '%s'.", filename);

    fclose(fp);

    char *key = strtok(buf, "\r\n\t ");
        
    while (key)
    {
        char *value = strtok(NULL, "\r\n");

        uo_hashtbl_insert(hashtbl, key, value);
        
        key = strtok(NULL, "\r\n\t ");
    }

    return conf;

err_close:
    fclose(fp);
    free(buf);

err_free:
    free(hashtbl);
    free(conf);

    return NULL;

}

char *uo_conf_get(
	uo_conf *conf,
	char *key)
{
    return uo_hashtbl_find(conf->hashtbl, key);
}

void uo_conf_destroy(
	uo_conf *conf) 
{
    free(conf->hashtbl);
    free(conf);
}
