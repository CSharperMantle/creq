#include <stdlib.h>
#include <stdio.h>
#include "creq.h"


int main(int argc, char **argv)
{
    creq_Request_t *req = NULL;
    creq_Config_t conf;
    conf.config_type = CONF_REQUEST;
    conf.data.request_config.line_ending = LE_CRLF;
    req = creq_Request_create(&conf);
    creq_Request_set_http_method(req, GET);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target(req, "www.baidu.com");
    creq_HeaderLListNode_add_header(&req->list_head, "Host", "www.baidu.com");
    creq_HeaderLListNode_add_header(&req->list_head, "User-agent", "creq/0.1.0");
    creq_HeaderLListNode_add_header(&req->list_head, "Connection", "close");
    creq_Request_set_message_body(req, "");
    printf("%s\n", creq_Request_stringify(req));
    creq_Request_free(req);
    return 0;
}
