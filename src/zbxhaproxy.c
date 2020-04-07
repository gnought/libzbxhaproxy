#include "zbxhaproxy.h"

int zbx_module_api_version(void) {
#ifdef ZBX_MODULE_API_VERSION
    return ZBX_MODULE_API_VERSION;
#else
    return ZBX_MODULE_API_VERSION_ONE;
#endif
}

void zbx_module_item_timeout(int timeout) {
    item_timeout = timeout;
}

ZBX_METRIC* zbx_module_item_list(void) {
    return keys;
}

int zbx_module_init(void) {
    zabbix_log(LOG_LEVEL_INFORMATION, "starting agent module %s", HAPROXY_MOD_NAME);
    haproxy_init();

    return SYSINFO_RET_OK;
}

int zbx_module_uninit(void) {
    haproxy_uninit();
    return SYSINFO_RET_OK;
}

static int zbxhaproxy_info(AGENT_REQUEST* request, AGENT_RESULT* result) {
    if (request->nparam != 2) {
        SET_MSG_RESULT(result, strdup("Invalid number of parameters. "
                                      "Usage haproxy.info[socket, key]"));
        return SYSINFO_RET_FAIL;
    }

    char* socket = get_rparam(request, 0);
    char* key = get_rparam(request, 1);

    char* value = haproxy_request_info(socket, key);
    SET_STR_RESULT(result, strdup(value));

    return SYSINFO_RET_OK;
}

static int zbxhaproxy_stat(AGENT_REQUEST* request, AGENT_RESULT* result) {
    if (request->nparam != 4) {
        SET_MSG_RESULT(result, strdup("Invalid number of parameters. "
                                      "Usage haproxy.stat[socket, pxname, svname, key]"));
        return SYSINFO_RET_FAIL;
    }

    char* socket = get_rparam(request, 0);
    char* pxname = get_rparam(request, 1);
    char* svname = get_rparam(request, 2);
    char* key = get_rparam(request, 3);

    char* value = haproxy_request_stat(socket, pxname, svname, key);
    SET_STR_RESULT(result, strdup(value));

    return SYSINFO_RET_OK;
}

static int zbxhaproxy_discovery(AGENT_REQUEST* request, AGENT_RESULT* result) {
    zabbix_log(LOG_LEVEL_DEBUG, "module %s have discovery", HAPROXY_MOD_NAME);

    if (request->nparam != 1) {
        SET_MSG_RESULT(result, strdup("Invalid number of parameters. "
                                      "Usage haproxy.discovery[socket]"));
        return SYSINFO_RET_FAIL;
    }

    char* socket = get_rparam(request, 0);

    char* data = haproxy_discovery(socket);
    zabbix_log(LOG_LEVEL_DEBUG, "module %s response: %s", HAPROXY_MOD_NAME, data);
    SET_STR_RESULT(result, data);
    return SYSINFO_RET_OK;
}

static int zbxhaproxy_test(AGENT_REQUEST* request, AGENT_RESULT* result) {
    SET_STR_RESULT(result, strdup("Hello"));
    return SYSINFO_RET_OK;
}
