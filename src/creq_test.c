#ifdef NDEBUG
#undef NDEBUG
#endif /* NDEBUG */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "creq.h"

static void _creqtest_print_status(char *str);

static void _creqtest_print_good(char *str);

static void _creqtest_print_bad(char *str);

#define _creqtest_assert(expr)                                                                                         \
    {                                                                                                                  \
        assert(expr);                                                                                                  \
        _creqtest_print_good("Assertion (" #expr ") succeeded");                                                       \
    }

int main(int argc, char *argv[])
{
    _creqtest_print_status("creq_Request part");
    _creqtest_print_status("Now testing request basics");
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
    creq_Request_set_target(req, "www.my-site.com", true);
    creq_Request_add_header(req, "Host", "www.example.com", true);
    creq_Request_add_header(req, "User-Agent", "creq/0.1.7", true);
    creq_Request_add_header(req, "Connection", "close", true);
    _creqtest_assert(!strcmp(creq_Request_search_for_header(req, "Host")->field_value, "www.example.com"));
    _creqtest_assert(creq_Request_search_for_header(req, "Host")->is_field_value_literal == true);
    creq_Request_set_message_body(req, "", true);
    _creqtest_assert(!strcmp(creq_Request_get_message_body(req), ""));
    _creqtest_assert(req->is_message_body_literal == true);
    creq_Request_free(req);

    _creqtest_print_status("Now testing header modification");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target(req, "www.my-site.com", true);
    creq_Request_add_header(req, "Host", "www.my-site.com", true);
    creq_Request_add_header(req, "User-Agent", "creq/0.1.7", true);
    creq_Request_add_header(req, "Connection", "close", true);
    creq_Request_add_header(req, "Bogus", "placeholder", true);
    creq_Request_remove_header(req, "Bogus");
    _creqtest_assert(creq_Request_search_for_header_index(req, "Bogus") == -1);
    _creqtest_assert(creq_Request_search_for_header(req, "Bogus") == NULL);
    creq_Request_set_message_body(req, "user=CSharperMantle&mood=happy", true);
    creq_Request_free(req);

    _creqtest_print_status("Now testing automatic Content-Length calculation (no replacement)");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target(req, "www.my-site.com", true);
    creq_Request_add_header(req, "Host", "www.my-site.com", true);
    creq_Request_add_header(req, "User-Agent", "creq/0.1.7", true);
    creq_Request_add_header(req, "Connection", "close", true);
    creq_Request_set_message_body_content_len(req, "user=CSharperMantle&mood=happy", true);
    _creqtest_assert(creq_Request_search_for_header(req, "Content-Length")->field_value != NULL);
    _creqtest_assert(!strcmp(creq_Request_search_for_header(req, "Content-Length")->field_value, "30"));
    creq_Request_free(req);

    _creqtest_print_status("Now testing automatic Content-Length calculation (replacement)");
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target(req, "www.my-site.com", true);
    creq_Request_add_header(req, "Host", "www.my-site.com", true);
    creq_Request_add_header(req, "User-Agent", "creq/0.1.7", true);
    creq_Request_add_header(req, "Content-Length", "15", true);
    creq_Request_add_header(req, "Connection", "close", true);
    creq_Request_set_message_body_content_len(req, "user=CSharperMantle&status=🚗", true);
    _creqtest_assert(creq_Request_search_for_header(req, "Content-Length")->field_value != NULL);
    _creqtest_assert(!strcmp(creq_Request_search_for_header(req, "Content-Length")->field_value, "31"));
    creq_Request_free(req);

    _creqtest_print_status("creq_Response part");
    _creqtest_print_status("Now testing Response basics and header modification");
    creq_Response_t *resp = NULL;
    creq_Config_t resp_conf;
    resp_conf.config_type = CONF_RESPONSE;
    resp_conf.data.response_config.line_ending = LE_CRLF;
    resp = creq_Response_create(&resp_conf);
    _creqtest_assert(resp->config.config_type == CONF_RESPONSE);
    _creqtest_assert(resp->config.data.response_config.line_ending == LE_CRLF);
    creq_Response_set_http_version(resp, 1, 1);
    creq_Response_set_status_code(resp, 200);
    _creqtest_assert(resp->status_code == 200);
    creq_Response_set_reason_phrase(resp, "OK");
    _creqtest_assert(!strcmp(resp->reason_phrase, "OK"));
    creq_Response_add_header_literal(resp, "Content-Type", "text/plain; charset=utf-8");
    creq_Response_add_header_literal(resp, "Connection", "close");
    creq_Response_add_header_literal(resp, "X-Generated-By", "creq/0.1.5.1");
    creq_Response_add_header_literal(resp, "Bogus", "placeholder");
    creq_Response_remove_header(resp, "Bogus");
    _creqtest_assert(creq_Response_search_for_header_index(resp, "Bogus") == -1);
    _creqtest_assert(creq_Response_search_for_header(resp, "Bogus") == NULL);
    creq_Response_set_message_body_literal(resp, "Hello world!");
    creq_Response_free(resp);

    _creqtest_print_status("Now testing automatic Content-Length calculation (no replacement)");
    resp = creq_Response_create(&resp_conf);
    creq_Response_set_http_version(resp, 1, 1);
    creq_Response_set_status_code(resp, 200);
    creq_Response_set_reason_phrase(resp, "OK");
    creq_Response_add_header_literal(resp, "Content-Type", "text/html; charset=utf-8");
    creq_Response_add_header_literal(resp, "Connection", "close");
    creq_Response_add_header_literal(resp, "X-Generated-By", "creq/0.1.5.1");
    creq_Response_set_message_body_literal_content_len(
        resp, "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Example Output</title></head><body><div>This "
              "is an example output.</div></body></html>");
    _creqtest_assert(creq_Response_search_for_header(resp, "Content-Length")->field_value != NULL);
    _creqtest_assert(!strcmp(creq_Response_search_for_header(resp, "Content-Length")->field_value, "142"));
    creq_Response_free(resp);

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
