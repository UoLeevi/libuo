#include "uo_tcpserv_conf.h"
#include "uo_err.h"
#include "uo_sock.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

uo_tcpserv_conf *uo_tcpserv_conf_create() 
{
    uo_tcpserv_conf *conf = calloc(1, sizeof *conf);

    struct stat sb;
    if (stat(UO_TCPSERV_CONF_FILE, &sb) == -1)
        uo_err_exit("Unable to read tcpserv configuration.");

    FILE *fp = fopen(UO_TCPSERV_CONF_FILE, "rb");
    if (!fp)
        uo_err_exit("Unable to read tcpserv configuration.");

    char *buf = malloc(sb.st_size + 1);
    buf[sb.st_size] = '\0';

    if (fread(buf, sizeof *buf, sb.st_size, fp) != sb.st_size || ferror(fp))
        uo_err_exit("Unable to read tcpserv configuration.");

    fclose(fp);

    const char *delim = "\r\n\t ";
    char *token = strtok(buf, delim);
        
    do 
    {
        if (strcmp(token, "key") == 0)
        {
            char *key_path = strtok(NULL, delim);
            conf->keys = realloc(conf->keys, sizeof *conf->keys * (conf->keys_len + 1));
            uo_tcpserv_key *key = conf->keys + conf->keys_len;

            key->name_len = strlen(key_path);
            key->name = malloc(key->name_len);
            memcpy(key->name, key_path, key->name_len + 1);
            key->name[key->name_len] = '\0';

            if (stat(key_path, &sb) == -1)
                uo_err_exit("Unable to read tcpserv key file.");

            fp = fopen(key_path, "rb");
            if (!fp)
                uo_err_exit("Unable to read tcpserv key file.");

            key->key = malloc(sb.st_size + 1);
            key->key[sb.st_size] = '\0';

            if (fread(key->key, sizeof(char), sb.st_size, fp) != sb.st_size || ferror(fp))
                uo_err_exit("Unable to read tcpserv key file.");

            key->key_len = sb.st_size;
            
            while (isspace(key->key[key->key_len - 1]))
                key->key[--key->key_len] = '\0';

            fclose(fp);

            conf->keys_len++;

            continue;
        }


        if (strcmp(token, "port") == 0)
        {
            char *endptr;
            char *p = strtok(NULL, delim);
            conf->port = htons(strtoul(p, &endptr, 10));

            continue;
        }

        uo_err_exit("An unrecognized token was encountered while parsing tcpserv configuration.");

        
    } while (token = strtok(NULL, delim));

    free(buf);

    if (!conf->port) 
        uo_err_exit("Error occured while parsing tcpserv configuration.");

    return conf;
}

void uo_tcpserv_conf_destroy(
    uo_tcpserv_conf *conf) 
{
    for (size_t i = 0; i < conf->keys_len; ++i)
    {
        uo_tcpserv_key *key = conf->keys + i;
        free(key->key);
        free(key->name);
    }

    free(conf->keys);
    free(conf);
}
