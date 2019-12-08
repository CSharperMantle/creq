#include <stdlib.h>
#include <stdio.h>
#include "creq.h"


int main()
{
    printf("%s\r\n", "NOW PRINTING A GET REQUEST");

    creq_Request_t *req = NULL;
    creq_Config_t req_conf;
    req_conf.config_type = CONF_REQUEST;
    req_conf.data.request_config.line_ending = LE_CRLF;
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_GET);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target(req, "www.baidu.com");
    creq_HeaderLListNode_add_header(&req->list_head, "Host", "www.baidu.com");
    creq_HeaderLListNode_add_header(&req->list_head, "User-Agent", "creq/0.1.1");
    creq_HeaderLListNode_add_header(&req->list_head, "Connection", "close");
    creq_Request_set_message_body(req, "");
    char *req_str = creq_Request_stringify(req);
    printf("%s\r\n", req_str);
    creq_Request_free(req);
    if (req_str != NULL)
        free(req_str);

    printf("%s\r\n", "NOW PRINTING A POST REQUEST");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target(req, "www.my-site.com");
    creq_HeaderLListNode_add_header(&req->list_head, "Host", "www.my-site.com");
    creq_HeaderLListNode_add_header(&req->list_head, "User-Agent", "creq/0.1.1");
    creq_HeaderLListNode_add_header(&req->list_head, "Connection", "close");
    creq_Request_set_message_body(req, "user=CSharperMantle&mood=happy");
    req_str = creq_Request_stringify(req);
    printf("%s\r\n", req_str);
    creq_Request_free(req);
    if (req_str != NULL)
        free(req_str);
    
    printf("%s\r\n", "NOW PRINTING A 200 RESPONSE");
    creq_Response_t *resp = NULL;
    creq_Config_t resp_conf;
    resp_conf.config_type = CONF_RESPONSE;
    resp_conf.data.response_config.line_ending = LE_CRLF;
    char *resp_str = NULL;

    resp = creq_Response_create(&resp_conf);
    creq_Response_set_http_version(resp, 1, 1);
    creq_Response_set_status_code(resp, 200);
    creq_Response_set_reason_phrase(resp, "OK");
    creq_HeaderLListNode_add_header(&resp->list_head, "Content-Type", "text/plain; charset=utf-8");
    creq_HeaderLListNode_add_header(&resp->list_head, "Connection", "close");
    creq_HeaderLListNode_add_header(&resp->list_head, "X-Generated-By", "creq/0.1.1");
    creq_Response_set_message_body(resp, "Hello world!");
    resp_str = creq_Response_stringify(resp);
    printf("%s\r\n", resp_str);
    creq_Response_free(resp);
    if (resp_str != NULL)
        free(resp_str);

    return 0;
}
