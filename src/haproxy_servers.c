#include "haproxy_servers.h"

static haproxy_server_t* get_prev_haproxy_server(haproxy_servers_t servers, char* pxname, char* svname);
static int check_haproxy_server_name(haproxy_server_t* server, char* pxname, char* svname);

/*
 * compare haproxy `server` name with `pxname`,`svname`
 * note: haproxy server name have 2 part:
 *      `pxname` - proxy name
 *      `svname` - server name
 * return:
 *       1 - is equal
 *       0 - is not equal
 */
static int check_haproxy_server_name(haproxy_server_t* server, char* pxname, char* svname) {
    char* sv_pxname = server->stat;
    char* sv_svname = server->stat + server->offsets[1];

    return (strcmp(sv_pxname, pxname) == 0 && strcmp(sv_svname, svname) == 0);
}

void free_haproxy_servers(haproxy_servers_t servers) {
    haproxy_server_t* cur = servers;
    haproxy_server_t* next = NULL;

    while (cur != NULL) {
        next = cur->next;
        zbx_free(cur);
        cur = next;
    }
}

/*
 * get haproxy server by name - `pxname` and `svname`
 * return:
 *      pointer to haproxy server
 *      NULL - server not found
 */
haproxy_server_t* get_haproxy_server(haproxy_servers_t servers, char* pxname, char* svname) {
    haproxy_server_t* sv = servers;

    while (sv != NULL) {
        if (check_haproxy_server_name(sv, pxname, svname)) {
            return sv;
        }
        sv = sv->next;
    }

    return NULL;
}

/*
 * get previous server for server with name `pxname`,`svname`
 * note: used only for update_haproxy_servers
 * return:
 *      pointer to previous server
 *      NULL - server list is empty
 */

static haproxy_server_t* get_prev_haproxy_server(haproxy_servers_t list, char* pxname, char* svname) {
    haproxy_server_t* sv = list;
    haproxy_server_t* prev_sv = list;

    while (sv != NULL) {
        if (check_haproxy_server_name(sv, pxname, svname)) {
            return prev_sv;
        }
        prev_sv = sv;
        sv = sv->next;
    }

    return prev_sv;
}

/*
 * create new haproxy server with `stat` line
 * return:
 *      pointer to new server
 *      NULL - memory allocation error
 */
haproxy_server_t* new_haproxy_server(char* stat, int metrics_num) {
    haproxy_server_t* sv = (haproxy_server_t*)zbx_malloc(NULL, sizeof(haproxy_server_t));

    sv->next = NULL;
    memset(sv->stat, 0, MAX_SIZE_STAT_LINE);
    sv->num_offsets = 0;
    zbx_strlcpy(sv->stat, stat, MAX_SIZE_STAT_LINE);

    return sv;
}

/*
 * update haproxy server list `servers` with `server`
 * return:
 *      pointer to haproxy server list
 */
haproxy_servers_t update_haproxy_servers(haproxy_servers_t servers, haproxy_server_t* server) {
    if (servers == NULL)
        // `server` is first in list
        return server;

    char* pxname = server->stat;
    char* svname = server->stat + server->offsets[1];
    haproxy_server_t* prev = get_prev_haproxy_server(servers, pxname, svname);
    if (prev->next != NULL) {
        // replace exists server in list
        server->next = prev->next->next;
        zbx_free(prev->next);
        prev->next = server;
    } else {
        // insert new server in list
        prev->next = server;
    }

    return servers;
}
