#include "uo_conf.h"
#include "uo_err.h"
#include "uo_strhashtbl.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

uo_conf *uo_conf_create(
    char *filename)
{
    uo_conf *conf = calloc(1, sizeof *conf);
    uo_strhashtbl *strhashtbl = conf->strhashtbl = uo_strhashtbl_create(0x10);

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

        uo_strhashtbl_insert(strhashtbl, key, value);
        
        key = strtok(NULL, "\r\n\t ");
    }

    return conf;

err_close:
    fclose(fp);
    free(buf);

err_free:
    free(strhashtbl);
    free(conf);

    return NULL;

}

char *uo_conf_get(
    uo_conf *conf,
    const char *key)
{
    return uo_strhashtbl_find(conf->strhashtbl, key);
}

void uo_conf_destroy(
    uo_conf *conf) 
{
    free(conf->strhashtbl);
    free(conf);
}
