#include "creq.h"
#include "unity.h"

void test_creq_Response_BasicOperations()
{
    creq_Response_t *resp = NULL;
    creq_Config_t resp_conf;
    resp_conf.config_type = CONF_RESPONSE;
    resp_conf.data.response_config.line_ending = LE_CRLF;

    resp = creq_Response_create(&resp_conf);

    TEST_ASSERT_EQUAL_INT(CONF_RESPONSE, resp->config.config_type);
    TEST_ASSERT_EQUAL_INT(LE_CRLF, resp->config.data.response_config.line_ending);

    creq_Response_set_http_version(resp, 1, 1);
    creq_Response_set_status_code(resp, 200);
    TEST_ASSERT_EQUAL_INT(200, resp->status_code);

    creq_Response_set_reason_phrase(resp, "OK");
    TEST_ASSERT_EQUAL_STRING("OK", resp->reason_phrase);

    creq_Response_add_header_literal(resp, "Connection", "close");
    creq_Response_add_header_literal(resp, "X-Generated-By", "creq/0.1.5.1");
    creq_Response_add_header_literal(resp, "Bogus", "placeholder");
    creq_Response_remove_header(resp, "Bogus");
    TEST_ASSERT_EQUAL_INT(-1, creq_Response_search_for_header_index(resp, "Bogus"));
    TEST_ASSERT_NULL(creq_Response_search_for_header(resp, "Bogus"));

    creq_Response_set_message_body_literal(resp, "Hello world!");
    creq_Response_free(resp);
}

void test_creq_Response_ContentLenCalculation()
{
    creq_Response_t *resp = NULL;
    creq_Config_t resp_conf;
    resp_conf.config_type = CONF_RESPONSE;
    resp_conf.data.response_config.line_ending = LE_CRLF;

    resp = creq_Response_create(&resp_conf);
    creq_Response_set_http_version(resp, 1, 1);
    creq_Response_set_status_code(resp, 200);
    creq_Response_set_reason_phrase(resp, "OK");
    
    creq_Response_add_header_literal(resp, "X-Generated-By", "creq/0.1.5.1");
    creq_Response_set_message_body_literal_content_len(
        resp, "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Example Output</title></head><body><div>This "
              "is an example output.</div></body></html>");
    TEST_ASSERT_NOT_NULL(creq_Response_search_for_header(resp, "Content-Length")->field_value);
    TEST_ASSERT_EQUAL_STRING("142", creq_Response_search_for_header(resp, "Content-Length")->field_value);
    
    creq_Response_free(resp);
}

void setUp()
{
    // placeholder
}

void tearDown()
{
    // placeholder
}

int main(int argc, char *argv[])
{
    UNITY_BEGIN();

    return UNITY_END();
}