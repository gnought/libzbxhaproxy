/*
 * File:   haproxy_servers.h
 * Author: kinky
 *
 * Created on October 26, 2019, 4:29 PM
 */

#ifndef HAPROXY_STAT_LIST_H
#define HAPROXY_STAT_LIST_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "common.h"

typedef struct haproxy_server_c {
    // full stat line (character ',' replaced by '\0')
    char* stat;
    // metric value offset in the stat line (0 - pxname, 1 - svname ...))
    // get metric value by (stat + offsets[metric_index]])
    // offset[0] is 0 always
    unsigned int* offsets;
    // next haproxy server pointer
    struct haproxy_server_c* next;
} haproxy_server_t;

// haproxy servers stored as simple one-direction list
typedef haproxy_server_t* haproxy_servers_t;
//
// typedef enum {
//     frontend,
//     backend,
//     server,
//     listener
// } haproxy_server_type_t;

void free_haproxy_servers(haproxy_servers_t servers);
haproxy_server_t* get_haproxy_server(haproxy_servers_t servers, char* pxname, char* svname);
haproxy_server_t* new_haproxy_server(char* stat, int metrics_num);
haproxy_servers_t update_haproxy_servers(haproxy_servers_t servers, haproxy_server_t* server);

#endif /* HAPROXY_STAT_LIST_H */
