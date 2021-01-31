#ifdef NDEBUG
#undef NDEBUG
#endif /* NDEBUG */

#include <stdlib.h>
#include <stdbool.h>
#include "creq.h"
#include "unity.h"

void test_creq_Request_BasicOperations()
{
    creq_Request_t *req = NULL;
    creq_Config_t req_conf;
    req_conf.config_type = CONF_REQUEST;
    req_conf.data.request_config.line_ending = LE_CRLF;

    req = creq_Request_create(&req_conf);

    TEST_ASSERT_EQUAL_INT(CONF_REQUEST, req->config.config_type);
    TEST_ASSERT_EQUAL_INT(LE_CRLF, req->config.data.request_config.line_ending);

    creq_Request_set_http_method(req, METH_GET);
    TEST_ASSERT_EQUAL_INT(METH_GET, req->method);

    creq_Request_set_http_version(req, 1, 1);
    TEST_ASSERT_EQUAL_INT(1, req->http_version.major);
    TEST_ASSERT_EQUAL_INT(1, req->http_version.minor);

    creq_Request_set_target(req, "www.my-site.com", true);
    creq_Request_add_header(req, "Host", "www.example.com", true);
    creq_Request_add_header(req, "User-Agent", "creq/0.1.7", true);
    creq_Request_add_header(req, "Connection", "close", true);
    TEST_ASSERT_EQUAL_STRING("www.example.com", creq_Request_search_for_header(req, "Host")->field_value);
    TEST_ASSERT_TRUE(creq_Request_search_for_header(req, "Host")->is_field_value_literal);

    creq_Request_set_message_body(req, "", true);
    TEST_ASSERT_EQUAL_STRING("", creq_Request_get_message_body(req));
    TEST_ASSERT_TRUE(req->is_message_body_literal);

    creq_Request_free(req);
}

void test_creq_Request_HeaderModification()
{
    creq_Request_t *req = NULL;
    creq_Config_t req_conf;
    req_conf.config_type = CONF_REQUEST;
    req_conf.data.request_config.line_ending = LE_CRLF;
    
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target(req, "www.my-site.com", true);

    creq_Request_add_header(req, "Host", "www.my-site.com", true);

    creq_Request_add_header(req, "Bogus", "placeholder", true);
    creq_Request_remove_header(req, "Bogus");
    TEST_ASSERT_EQUAL_INT(-1, creq_Request_search_for_header_index(req, "Bogus"));
    TEST_ASSERT_EQUAL(NULL, creq_Request_search_for_header(req, "Bogus"));

    creq_Request_set_message_body(req, "user=CSharperMantle&mood=happy", true);
    creq_Request_free(req);
}

void test_creq_Request_ContentLenCalculation()
{
    creq_Request_t *req = NULL;
    creq_Config_t req_conf;
    req_conf.config_type = CONF_REQUEST;
    req_conf.data.request_config.line_ending = LE_CRLF;

    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);

    creq_Request_set_target(req, "www.my-site.com", true);
    creq_Request_add_header(req, "Host", "www.my-site.com", true);
    creq_Request_add_header(req, "User-Agent", "creq/0.1.7", true);
    creq_Request_add_header(req, "Connection", "close", true);

    creq_Request_set_message_body_content_len(req, "user=CSharperMantle&mood=happy", true);
    TEST_ASSERT_NOT_NULL(creq_Request_search_for_header(req, "Content-Length")->field_value);
    TEST_ASSERT_EQUAL_STRING("30", creq_Request_search_for_header(req, "Content-Length")->field_value);

    creq_Request_free(req);
}

void test_creq_Request_ContentLenReplacement()
{
    creq_Request_t *req = NULL;
    creq_Config_t req_conf;
    req_conf.config_type = CONF_REQUEST;
    req_conf.data.request_config.line_ending = LE_CRLF;
    
    req = creq_Request_create(&req_conf);
    creq_Request_set_http_method(req, METH_POST);
    creq_Request_set_http_version(req, 1, 1);
    creq_Request_set_target(req, "www.my-site.com", true);
    creq_Request_add_header(req, "Host", "www.my-site.com", true);
    creq_Request_add_header(req, "User-Agent", "creq/0.1.7", true);
    creq_Request_add_header(req, "Connection", "close", true);

    creq_Request_add_header(req, "Content-Length", "15", true);
    creq_Request_set_message_body_content_len(req, "user=CSharperMantle&status=🚗", true);
    TEST_ASSERT_NOT_NULL(creq_Request_search_for_header(req, "Content-Length")->field_value);
    TEST_ASSERT_EQUAL_STRING("31", creq_Request_search_for_header(req, "Content-Length")->field_value);

    creq_Request_free(req);
}

void setUp()
{
    // empty body; placeholder
}

void tearDown()
{
    // empty body; placeholder
}

int main(int argc, char *argv[])
{
    UNITY_BEGIN();
    RUN_TEST(test_creq_Request_BasicOperations);
    RUN_TEST(test_creq_Request_HeaderModification);
    RUN_TEST(test_creq_Request_ContentLenCalculation);
    RUN_TEST(test_creq_Request_ContentLenReplacement);
    
    return UNITY_END();
}