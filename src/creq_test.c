#ifdef NDEBUG
#undef NDEBUG
#endif /* NDEBUG */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "creq.h"

void _creqtest_print_status(char *str);

void _creqtest_print_good(char *str);

void _creqtest_print_bad(char *str);

#define _creqtest_assert(expr) { assert(expr); _creqtest_print_good("assert (" #expr ") finished"); }

int main(int argc, char *argv[])
{
    _creqtest_print_status("Now testing Request basics");
    creq_Request_t *req = NULL;
    creq_Config_t req_conf;
    req_conf.config_type = CONF_REQUEST;
    req_conf.data.request_config.line_ending = LE_CRLF;
    req = creq_Request_create(&req_conf);
    _creqtest_assert(req->config.config_type == CONF_REQUEST);
    _creqtest_assert(req->config.data.request_config.line_ending == LE_CRLF);
    creq_Request_set_http_method(req, METH_GET);
    _creqtest_assert(req->method == METH_GET);
    creq_Request_set_http_version(req, 1, 1);
    _creqtest_assert(req->http_version.major == 1);
    _creqtest_assert(req->http_version.minor == 1);
    creq_Request_set_target_literal(req, "www.example.com");
    creq_Request_add_header_literal(req, "Host", "www.example.com");
    creq_Request_add_header_literal(req, "User-Agent", "creq/0.1.5.1");
    creq_Request_add_header_literal(req, "Connection", "close");
    _creqtest_assert(!strcmp(creq_Request_search_for_header(req, "Host")->field_value, "www.example.com"));
    creq_Request_set_message_body_literal(req, "");
    _creqtest_assert(!strcmp(creq_Request_get_message_body(req), ""));
    _creqtest_assert(req->is_message_body_literal == true);
    creq_Request_free(req);

    _creqtest_print_status("Now testing header modification");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target_literal(req, "www.my-site.com");
    creq_Request_add_header_literal(req, "Host", "www.my-site.com");
    creq_Request_add_header_literal(req, "User-Agent", "creq/0.1.5.1");
    creq_Request_add_header_literal(req, "Connection", "close");
    creq_Request_add_header_literal(req, "Bogus", "placeholder");
    creq_Request_remove_header(req, "Bogus");
    _creqtest_assert(creq_Request_search_for_header_index(req, "Bogus") == -1);
    _creqtest_assert(creq_Request_search_for_header(req, "Bogus") == NULL);
    creq_Request_set_message_body_literal(req, "user=CSharperMantle&mood=happy");
    creq_Request_free(req);

    _creqtest_print_status("Now testing automatic Content-Length calculation (no replacement)");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target_literal(req, "www.my-site.com");
    creq_Request_add_header_literal(req, "Host", "www.my-site.com");
    creq_Request_add_header_literal(req, "User-Agent", "creq/0.1.5.1");
    creq_Request_add_header_literal(req, "Connection", "close");
    creq_Request_set_message_body_literal_content_len(req, "user=CSharperMantle&mood=happy");
    _creqtest_assert(creq_Request_search_for_header(req, "Content-Length")->field_value != NULL);
    _creqtest_assert(!strcmp(creq_Request_search_for_header(req, "Content-Length")->field_value, "30"));
    creq_Request_free(req);

    _creqtest_print_status("Now testing automatic Content-Length calculation (replacement)");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target_literal(req, "www.my-site.com");
    creq_Request_add_header_literal(req, "Host", "www.my-site.com");
    creq_Request_add_header_literal(req, "User-Agent", "creq/0.1.5.1");
    creq_Request_add_header_literal(req, "Content-Length", "15");
    creq_Request_add_header_literal(req, "Connection", "close");
    creq_Request_set_message_body_literal_content_len(req, "user=CSharperMantle&status=🚗");
    _creqtest_assert(creq_Request_search_for_header(req, "Content-Length")->field_value != NULL);
    _creqtest_assert(!strcmp(creq_Request_search_for_header(req, "Content-Length")->field_value, "31"));
    creq_Request_free(req);

    _creqtest_print_status("NOW PRINTING A 200 RESPONSE");
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

    _creqtest_print_status("NOW PRINTING A 200 RESPONSE WITH CALCULATED Content-Length");
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

void _creqtest_print_status(char *str)
{
    printf("[*] %s\r\n", str);
}

void _creqtest_print_good(char *str)
{
    printf("[+] %s\r\n", str);
}

void _creqtest_print_bad(char *str)
{
    printf("[-] %s\r\n", str);
}
