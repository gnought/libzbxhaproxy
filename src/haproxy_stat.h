/*
 * File:   haproxy_stat.h
 * Author: kinky
 *
 * Created on October 20, 2019, 10:40 PM
 */
#if __APPLE__
#define MSG_NOSIGNAL 0x2000 /* don't raise SIGPIPE */
#endif

#ifndef HAPROXY_STAT_H
#define HAPROXY_STAT_H

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "log.h"
#include "array.h"
#include "hash_table.h"
#include "haproxy_servers.h"

#define CACHE_TTL 30
#define HAPROXY_NO_DATA ""
#define HAPROXY_OK 0
#define HAPROXY_FAIL -1
#define INIT_NUM_METRICS 100
#define MAX_RETRIES 2

int haproxy_socket_fd;

time_t stat_timestamp;
time_t info_timestamp;

ht_hash_table* haproxy_info;
haproxy_servers_t haproxy_stats;
Array* haproxy_metrics;

static struct timeval stimeout;

int haproxy_init();
int haproxy_uninit();

char* haproxy_discovery(char* socket);
char* haproxy_request_info(char* socket, char* key);
char* haproxy_request_stat(char* socket, char* pxname, char* svname, char* metric);

#endif /* HAPROXY_STAT_H */
