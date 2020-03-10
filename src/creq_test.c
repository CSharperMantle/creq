#ifdef NDEBUG
#undef NDEBUG
#endif /* NDEBUG */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "creq.h"

int main(int argc, char *argv[])
{
    printf("%s\r\n", "---NOW PRINTING A GET REQUEST");
    creq_Request_t *req = NULL;
    creq_Config_t req_conf;
    req_conf.config_type = CONF_REQUEST;
    req_conf.data.request_config.line_ending = LE_CRLF;
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_GET);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target_literal(req, "www.baidu.com");
    creq_Request_add_header_literal(req, "Host", "www.baidu.com");
    creq_Request_add_header_literal(req, "User-Agent", "creq/0.1.5.1");
    creq_Request_add_header_literal(req, "Connection", "close");
    creq_Request_set_message_body_literal(req, "");
    char *req_str = creq_Request_stringify(req);
    printf("%s\r\n", req_str);
    creq_Request_free(req);
    if (req_str != NULL)
        free(req_str);

    printf("%s\r\n", "---NOW PRINTING A POST REQUEST");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target_literal(req, "www.my-site.com");
    creq_Request_add_header_literal(req, "Host", "www.my-site.com");
    creq_Request_add_header_literal(req, "User-Agent", "creq/0.1.5.1");
    creq_Request_add_header_literal(req, "Connection", "close");
    creq_Request_add_header_literal(req, "Bogus", "placeholder");
    creq_Request_remove_header(req, "Bogus");
    creq_Request_set_message_body_literal(req, "user=CSharperMantle&mood=happy");
    req_str = creq_Request_stringify(req);
    printf("%s\r\n", req_str);
    creq_Request_free(req);
    if (req_str != NULL)
        free(req_str);

    printf("%s\r\n", "---NOW PRINTING A POST REQUEST WITH AUTOMATICALLY CALCULATED Content-Length");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target_literal(req, "www.my-site.com");
    creq_Request_add_header_literal(req, "Host", "www.my-site.com");
    creq_Request_add_header_literal(req, "User-Agent", "creq/0.1.5.1");
    creq_Request_add_header_literal(req, "Connection", "close");
    creq_Request_add_header_literal(req, "Bogus", "placeholder");
    creq_Request_remove_header(req, "Bogus");
    creq_Request_set_message_body_literal_content_len(req, "user=CSharperMantle&mood=happy");
    req_str = creq_Request_stringify(req);
    printf("%s\r\n", req_str);
    creq_Request_free(req);
    if (req_str != NULL)
        free(req_str);

    printf("%s\r\n", "---NOW PRINTING A POST REQUEST WITH A PRE-DEFINED Content-Length WHICH NEED TO BE UPDATED");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target_literal(req, "www.my-site.com");
    creq_Request_add_header_literal(req, "Host", "www.my-site.com");
    creq_Request_add_header_literal(req, "User-Agent", "creq/0.1.5.1");
    creq_Request_add_header_literal(req, "Content-Length", "15");
    creq_Request_add_header_literal(req, "Connection", "close");
    creq_Request_set_message_body_literal_content_len(req, "user=CSharperMantle&status=🚗");
    req_str = creq_Request_stringify(req);
    printf("%s\r\n", req_str);
    creq_Request_free(req);
    if (req_str != NULL)
        free(req_str);

    printf("%s\r\n", "---NOW PRINTING A 200 RESPONSE");
    creq_Response_t *resp = NULL;
    creq_Config_t resp_conf;
    resp_conf.config_type = CONF_RESPONSE;
    resp_conf.data.response_config.line_ending = LE_CRLF;
    char *resp_str = NULL;

    resp = creq_Response_create(&resp_conf);
    creq_Response_set_http_version(resp, 1, 1);
    creq_Response_set_status_code(resp, 200);
    creq_Response_set_reason_phrase(resp, "OK");
    creq_Response_add_header_literal(resp, "Content-Type", "text/plain; charset=utf-8");
    creq_Response_add_header_literal(resp, "Connection", "close");
    creq_Response_add_header_literal(resp, "X-Generated-By", "creq/0.1.5.1");
    creq_Response_add_header_literal(resp, "Bogus", "placeholder");
    creq_Response_remove_header(resp, "Bogus");
    creq_Response_set_message_body_literal(resp, "Hello world!");
    resp_str = creq_Response_stringify(resp);
    printf("%s\r\n", resp_str);
    creq_Response_free(resp);
    if (resp_str != NULL)
        free(resp_str);

    printf("%s\r\n", "---NOW PRINTING A 200 RESPONSE WITH CALCULATED Content-Length");
    resp = creq_Response_create(&resp_conf);
    creq_Response_set_http_version(resp, 1, 1);
    creq_Response_set_status_code(resp, 200);
    creq_Response_set_reason_phrase(resp, "OK");
    creq_Response_add_header_literal(resp, "Content-Type", "text/html; charset=utf-8");
    creq_Response_add_header_literal(resp, "Connection", "close");
    creq_Response_add_header_literal(resp, "X-Generated-By", "creq/0.1.5.1");
    creq_Response_add_header_literal(resp, "Bogus", "placeholder");
    creq_Response_remove_header(resp, "Bogus");
    creq_Response_set_message_body_literal_content_len(
        resp, "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; "
              "charset=utf-8\"><title>Example Output</title></head><body><div>This is an example "
              "output.</div></body></html>");
    resp_str = creq_Response_stringify(resp);
    printf("%s\r\n", resp_str);
    creq_Response_free(resp);
    if (resp_str != NULL)
        free(resp_str);

    return 0;
}
