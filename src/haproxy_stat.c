#include "haproxy_stat.h"

static char* line_metric_header;

static int haproxy_connect(char* socket_path);
static int haproxy_cmd(char* socket, char* cmd);
static int haproxy_recv(char** ret_data);

static int haproxy_update_info(char* socket);
static int haproxy_update_stat(char* socket);

static char* haproxy_info_value(char* name);
static char* haproxy_metric_value(char* pxname, char* svname, char* metric);

static void haproxy_parse_info(char* s);
static void haproxy_parse_metrics(char* header);
static void haproxy_parse_stat(char* s);
static void haproxy_parse_stat_line(char* stat);

int haproxy_init() {
    line_metric_header = NULL;
    stimeout.tv_sec = 30;
    stimeout.tv_usec = 0;
    haproxy_info = ht_new();
    haproxy_socket_fd = 0;
    haproxy_stats = NULL;
    haproxy_metrics = arr_new(INIT_NUM_METRICS);
    stat_timestamp = time(NULL) - 2 * CACHE_TTL;
    info_timestamp = time(NULL) - 2 * CACHE_TTL;

    return HAPROXY_OK;
}

int haproxy_uninit() {
    zbx_free(line_metric_header);
    arr_free(haproxy_metrics);
    ht_del_hash_table(haproxy_info);
    free_haproxy_servers(haproxy_stats);
    close(haproxy_socket_fd);
    return HAPROXY_OK;
}

static int haproxy_connect(char* socket_path) {
    struct sockaddr_un haproxy_stat_addr;
    size_t addr_length;

    if ((haproxy_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        return HAPROXY_FAIL;
    }

    memset(&haproxy_stat_addr, 0, sizeof(struct sockaddr_un));
    haproxy_stat_addr.sun_family = AF_UNIX;
    zbx_strlcpy(haproxy_stat_addr.sun_path, socket_path, strlen(socket_path) + 1);
    addr_length = sizeof(haproxy_stat_addr.sun_family) + strlen(haproxy_stat_addr.sun_path);

    for (int i = 0; i < MAX_RETRIES; i++) {
        if (connect(haproxy_socket_fd, (struct sockaddr*)&haproxy_stat_addr,
                    addr_length) != -1) {
            // socket input/output timeout
            if (setsockopt(haproxy_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&stimeout, sizeof(stimeout)) < 0) {
                zabbix_log(LOG_LEVEL_WARNING, "Cannot set SO_RCVTIMEO socket timeout: %ld seconds", stimeout.tv_sec);
            }
            if (setsockopt(haproxy_socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&stimeout, sizeof(stimeout)) < 0) {
                zabbix_log(LOG_LEVEL_WARNING, "Cannot set SO_SNDTIMEO socket timeout: %ld seconds", stimeout.tv_sec);
            }
            return HAPROXY_OK;
        }
    }
    return HAPROXY_FAIL;
}

static int haproxy_cmd(char* socket, char* cmd) {
    do {
        if (send(haproxy_socket_fd, cmd, strlen(cmd), MSG_NOSIGNAL) > 0) {
            return HAPROXY_OK;
        }
    } while (haproxy_connect(socket) == HAPROXY_OK);

    close(haproxy_socket_fd);
    return HAPROXY_FAIL;
}

char* haproxy_discovery(char* socket) {
    int data_size = 4096;

    haproxy_update_stat(socket);
    haproxy_server_t* sv = haproxy_stats;

    char* data = (char*)zbx_calloc(NULL, data_size, sizeof(char));
    strcat(data, "{\"data\":[");

    char* pxname = NULL;
    char* svname = NULL;
    char* buf = NULL;

    while (sv != NULL) {
        pxname = sv->stat;
        svname = sv->stat + sv->offsets[1];
        buf = zbx_dsprintf(buf, "{\"{#PXNAME}\":\"%s\",\"{#SVNAME}\":\"%s\"}", pxname, svname);
        //check if we have enough of free memory to append the content of the next buffer
        if ((strlen(data) + strlen(buf) + 4) > data_size) {
            //increment in one page size
            data_size = data_size + 4096;
            data = (char*)zbx_realloc(data, data_size * sizeof(char));
        }
        strcat(data, buf);
        if (sv->next != NULL) {
            strcat(data, ",");
        }
        sv = sv->next;
        zbx_free(buf);
    }
    strcat(data, "]}");

    return data;
}

static int haproxy_recv(char** ret_data) {
    assert(ret_data != NULL);

    int data_size = 4096, bytes_read, total_bytes_read = 0;
    char buffer[4096];

    memset(buffer, 0, sizeof(buffer));
    char* data = (char*)zbx_calloc(NULL, data_size, sizeof(char));

    while ((bytes_read = recv(haproxy_socket_fd, buffer, sizeof(buffer), 0)) > 0) {
        //check if we have enough of free memory to append the content of the next buffer
        if ((total_bytes_read + bytes_read + 2) > data_size) {
            //increment in one page size
            data_size = data_size + 4096;

            data = (char*)zbx_realloc(data, data_size * sizeof(char));
        }

        memcpy(data + total_bytes_read, buffer, bytes_read);
        total_bytes_read += bytes_read;
    }
    if (data != NULL) {
        data[total_bytes_read] = '\0';
    }
    if (bytes_read == -1) {
        zbx_free(data);
        return HAPROXY_FAIL;
    }

    *ret_data = data;

    return HAPROXY_OK;
}

static int haproxy_update_info(char* socket) {
    char* recv_buffer = NULL;
    if (haproxy_cmd(socket, "show info\n") == HAPROXY_FAIL) goto get_info_fail;
    if (haproxy_recv(&recv_buffer) == HAPROXY_FAIL) goto get_info_fail;
    haproxy_parse_info(recv_buffer);
    zbx_free(recv_buffer);
    return HAPROXY_OK;

get_info_fail:
    return HAPROXY_FAIL;
}

static int haproxy_update_stat(char* socket) {
    char* recv_buffer = NULL;
    if (haproxy_cmd(socket, "show stat\n") == HAPROXY_FAIL) goto get_stat_fail;
    if (haproxy_recv(&recv_buffer) == HAPROXY_FAIL) goto get_stat_fail;
    haproxy_parse_stat(recv_buffer);
    zbx_free(recv_buffer);
    return HAPROXY_OK;

get_stat_fail:
    return HAPROXY_FAIL;
}

static void haproxy_parse_info(char* s) {
    char* d = NULL;
    char key[255], value[255];
    int n = 0;

    char* line = (char*)strtok(s, "\n");

    while (line != NULL) {
        memset(key, 0, 255);
        memset(value, 0, 255);

        d = strchr(line, ':');
        // key length
        n = (d - line) / sizeof(char);
        memcpy(key, line, n);
        // value length
        n = strlen(line) - n - 2;
        memcpy(value, d + 2 * sizeof(char), n);

        ht_insert(haproxy_info, key, value);

        line = (char*)strtok(NULL, "\n");
    }
}

static void haproxy_parse_stat_line(char* stat) {
    int i = 0;

    haproxy_server_t* item = new_haproxy_server(stat, haproxy_metrics->size);

    item->offsets[i++] = 0;
    char* d = item->stat;

    while ((d = strchr(d, ',')) != NULL) {
        *d = '\0';
        item->offsets[i++] = (d - item->stat + 1) / sizeof(char);
        d++;
    }

    haproxy_stats = update_haproxy_servers(haproxy_stats, item);
}

static void haproxy_parse_metrics(char* header) {

    if (line_metric_header == NULL) {
        line_metric_header = zbx_strdup(NULL, header);
    } else if (strcmp(header, line_metric_header) == 0) {
        return;
    }

    char* d = header + 2;
    char* metric = d;

    arr_flush(haproxy_metrics);
    while ((d = strchr(d, ',')) != NULL) {
        *d = '\0';
        arr_push(haproxy_metrics, metric);
        metric = ++d;
    }
}

static void haproxy_parse_stat(char* s) {
    char* line = (char*)strtok(s, "\n");

    while (line != NULL) {
        if (line[0] == '#') {
            haproxy_parse_metrics(line);
        } else {
            haproxy_parse_stat_line(line);
        }
        line = (char*)strtok(NULL, "\n");
    }
}

char* haproxy_request_stat(char* socket, char* pxname, char* svname, char* metric) {
    if (time(NULL) - stat_timestamp > CACHE_TTL) {
        haproxy_update_stat(socket);
        stat_timestamp = time(NULL);
    }

    char* result = haproxy_metric_value(pxname, svname, metric);
    if (result == NULL) return HAPROXY_NO_DATA;

    return result;
}

char* haproxy_request_info(char* socket, char* key) {
    if (time(NULL) - info_timestamp > CACHE_TTL) {
        haproxy_update_info(socket);
        info_timestamp = time(NULL);
    }

    char* result = haproxy_info_value(key);
    if (result == NULL) return HAPROXY_NO_DATA;

    return result;
}

static char* haproxy_info_value(char* key) {
    return ht_search(haproxy_info, key);
}

static char* haproxy_metric_value(char* pxname, char* svname, char* metric) {
    int metric_id = arr_indexOf(haproxy_metrics, metric);

    if (metric_id < 0) return NULL;

    haproxy_server_t* item = get_haproxy_server(haproxy_stats, pxname, svname);

    if (item == NULL) {
        return NULL;
    }

    return item->stat + item->offsets[metric_id];
}
