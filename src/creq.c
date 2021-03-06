/**
 * @file creq.c
 * @brief Implementation for functions defined in creq.h
 * @author CSharperMantle
 */

// for portability consideration, try to make hacks to use %zu format for size_t
#if defined(__MINGW32__) || defined(__MINGW64__)
#define __USE_MINGW_ANSI_STDIO 1
#endif // defined(__MINGW32__) || defined (__MINGW64__)

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>
#include "creq.h"
#include "cvector.h"

/*
 * RFC 7320
 * Whitespace Rules
 *   OWS            = *( SP / HTAB )
 *                  ; optional whitespace
 *   RWS            = 1*( SP / HTAB )
 *                  ; required whitespace
 *   BWS            = OWS
 *                  ; "bad" whitespace
 */

/*
 * RFC 7230
 * request-line = method SP request-target SP HTTP-version CRLF
 * 
 * $(method) /target/path HTTP/1.1 line_ending(\r, \n || \r\n)
 */
CREQ_PRIVATE(const char *)
_creq_FMT_REQUEST_LINE = "%s %s %s%s";

/*
 * RFC 7230
 * HTTP-version  = HTTP-name "/" DIGIT "." DIGIT
 * HTTP-name     = %x48.54.54.50 ; "HTTP", case-sensitive
 * 
 * HTTP/$(major).$(minor)
 */
CREQ_PRIVATE(const char *)
_creq_FMT_HTTP_VERSION = "HTTP/%d.%d";

/*
 * RFC 7230
 * header-field   = field-name ":" OWS field-value OWS
 * 
 * Header-Head: Header-Value line_ending
 */
CREQ_PRIVATE(const char *)
_creq_FMT_HEADER = "%s: %s%s";

/*
 * RFC 7230
 * HTTP-message   = start-line
 *                  *( header-field CRLF )
 *                  CRLF
 *                  [ message-body ]
 * 
 * REQUEST_LINE(with line_ending)
 * HEADER(with line_ending)
 * line_ending
 * BODY
 */
CREQ_PRIVATE(const char *)
_creq_FMT_FULL_REQUEST = "%s%s%s%s";

/*
 * RFC 7320
 * status-line = HTTP-version SP status-code SP reason-phrase CRLF
 */
CREQ_PRIVATE(const char *)
_creq_FMT_STATUS_LINE = "%s %d %s%s";

/*
 * RFC 7230
 * HTTP-message   = start-line
 *                  *( header-field CRLF )
 *                  CRLF
 *                  [ message-body ]
 * 
 * STATUS_LINE(with line_ending)
 * HEADER(with line_ending)
 * line_ending
 * BODY
 */
CREQ_PRIVATE(const char *)
_creq_FMT_FULL_RESPONSE = "%s%s%s%s";

CREQ_PRIVATE(void *)
_creq_malloc_n_init(size_t size)
{
    void *pNewSpace = malloc(size);
    memset(pNewSpace, 0, size);
    return pNewSpace;
}

/// @attention Don't forget to free the pointer returned!
CREQ_PRIVATE(char *)
_creq_malloc_strcpy(const char *src)
{
    size_t fullLen = strlen(src) + 1;
    char *dest = (char *)malloc(sizeof(char) * fullLen);
    strcpy(dest, src);
    return dest;
}

CREQ_PRIVATE(const char *)
_creq_get_line_ending_str(creq_Config_t *conf, creq_ConfigType_t confType)
{
    if (conf == NULL)
    {
        return NULL;
    }
    creq_LineEnding_t ending = LE_CRLF;
    if (confType == CONF_REQUEST)
    {
        ending = conf->data.request_config.line_ending;
    }
    else if (confType == CONF_RESPONSE)
    {
        ending = conf->data.response_config.line_ending;
    }
    switch (ending)
    {
    case LE_CR:
        return "\r";
    case LE_LF:
        return "\n";
    case LE_CRLF:
        // fall through
    default:
        return "\r\n";
    }
}

CREQ_PRIVATE(const char *)
_creq_get_http_method_str(creq_HttpMethod_t meth)
{
    switch (meth)
    {
    case METH_GET:
        return "GET";
    case METH_HEAD:
        return "HEAD";
    case METH_POST:
        return "POST";
    case METH_PUT:
        return "PUT";
    case METH_DELETE:
        return "DELETE";
    case METH_CONNECT:
        return "CONNECT";
    case METH_OPTIONS:
        return "OPTIONS";
    case METH_TRACE:
        return "TRACE";
    default:
        return NULL;
    }
}

/// @attention Don't forget to free the returned string! THE STR IS MALLOC'ED!
CREQ_PRIVATE(char *)
_creq_get_http_version_str(int major, int minor)
{
    int len = snprintf((char *)NULL, 0U, _creq_FMT_HTTP_VERSION, major, minor);
    char *str = (char *)_creq_malloc_n_init(sizeof(char) * (len + 1));
    snprintf(str, len + 1, _creq_FMT_HTTP_VERSION, major, minor);
    return str;
}

CREQ_PUBLIC(creq_HeaderField_t *)
creq_HeaderField_create(char *header, char *value)
{
    if (header == NULL || value == NULL)
    {
        return NULL;
    }
    creq_HeaderField_t *pNewHeader = (creq_HeaderField_t *)_creq_malloc_n_init(sizeof(struct creq_HeaderField));
    pNewHeader->field_name = _creq_malloc_strcpy(header);
    pNewHeader->is_field_name_literal = false;
    pNewHeader->field_value = _creq_malloc_strcpy(value);
    pNewHeader->is_field_value_literal = false;
    return pNewHeader;
}

CREQ_PUBLIC(creq_HeaderField_t *)
creq_HeaderField_create_literal(const char *header_s, const char *value_s)
{
    if (header_s == NULL || value_s == NULL)
    {
        return NULL;
    }
    creq_HeaderField_t *pNewHeader = (creq_HeaderField_t *)_creq_malloc_n_init(sizeof(struct creq_HeaderField));
    pNewHeader->field_name = (char *)header_s;
    pNewHeader->is_field_name_literal = true;
    pNewHeader->field_value = (char *)value_s;
    pNewHeader->is_field_value_literal = true;
    return pNewHeader;
}

CREQ_PUBLIC(creq_status_t)
creq_HeaderField_free(creq_HeaderField_t **ptrToFieldPtr)
{
    if (ptrToFieldPtr == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    creq_HeaderField_t *pField = *ptrToFieldPtr;
    if (!pField->is_field_name_literal)
    {
        CREQ_GUARDED_FREE(pField->field_name);
        pField->field_name = NULL;
    }
    if (!pField->is_field_value_literal)
    {
        CREQ_GUARDED_FREE(pField->field_value);
        pField->field_value = NULL;
    }
    CREQ_GUARDED_FREE(pField);
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_Request_t *)
creq_Request_create(creq_Config_t *conf)
{
    creq_Request_t *pRequest = (creq_Request_t *)_creq_malloc_n_init(sizeof(struct creq_Request));

    if (conf != NULL && conf->config_type == CONF_REQUEST)
    {
        pRequest->config = *conf;
    }
    else
    {
        creq_Config_t config;
        config.config_type = CONF_REQUEST;
        config.data.request_config.line_ending = LE_CRLF;
        pRequest->config = config;
    }
    pRequest->method = _METH_UNKNOWN;
    pRequest->is_request_target_literal = false;
    pRequest->request_target = NULL;
    pRequest->http_version.major = 0;
    pRequest->http_version.minor = 0;
    pRequest->header_vector = NULL;
    pRequest->is_message_body_literal = false;
    pRequest->message_body = NULL;

    return pRequest;
}

CREQ_PUBLIC(creq_status_t)
creq_Request_free(creq_Request_t *req)
{
    if (req != NULL)
    {
        // free pointer members
        if (!req->is_request_target_literal)
            CREQ_GUARDED_FREE(req->request_target);
        if (!req->is_message_body_literal)
            CREQ_GUARDED_FREE(req->message_body);
        creq_HeaderField_t *pHeader = NULL;
        size_t szNowSize = cvector_size(req->header_vector);
        while (szNowSize > 0)
        {
            pHeader = req->header_vector[szNowSize - 1];
            cvector_pop_back(req->header_vector);
            creq_HeaderField_free(&pHeader);
            szNowSize = cvector_size(req->header_vector);
        }
        cvector_free(req->header_vector);
        // finally
        free(req);
        return CREQ_STATUS_SUCC;
    }
    return CREQ_STATUS_FAILED;
}

CREQ_PUBLIC(creq_status_t)
creq_Request_set_http_method(creq_Request_t *req, creq_HttpMethod_t method)
{
    if (req == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    req->method = method;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_HttpMethod_t)
creq_Request_get_http_method(creq_Request_t *req)
{
    if (req == NULL)
    {
        return _METH_UNKNOWN;
    }
    return req->method;
}

CREQ_PUBLIC(creq_status_t)
creq_Request_set_target(creq_Request_t *req, char *requestTarget, bool is_literal)
{
    if (req == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    if (!req->is_request_target_literal)
        CREQ_GUARDED_FREE(req->request_target); // if it is a malloc'ed string then free it.
    if (requestTarget == NULL)
    {
        req->request_target = NULL;
        return CREQ_STATUS_SUCC;
    }

    if (is_literal)
    {
        req->request_target = (char *)requestTarget; // this is a immediate literal value, store it.
        req->is_request_target_literal = true;
    }
    else
    {

        char *pReqTargetCopy = _creq_malloc_strcpy(requestTarget);
        req->request_target = pReqTargetCopy;
        req->is_request_target_literal = false; // this is not a immediate literal.
    }

    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(char *)
creq_Request_get_target(creq_Request_t *req)
{
    if (req == NULL)
    {
        return NULL;
    }
    return req->request_target;
}

CREQ_PUBLIC(creq_status_t)
creq_Request_set_http_version(creq_Request_t *req, int major, int minor)
{
    if (req == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    req->http_version.major = major;
    req->http_version.minor = minor;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_HttpVersion_t)
creq_Request_get_http_version(creq_Request_t *req)
{
    creq_HttpVersion_t obj;
    obj.major = -1;
    obj.minor = -1;
    if (req == NULL)
    {
        return obj;
    }
    obj.major = req->http_version.major;
    obj.minor = req->http_version.minor;
    return obj;
}

CREQ_PUBLIC(creq_status_t)
creq_Request_add_header(creq_Request_t *req, char *header, char *value, bool is_literal)
{
    if (req == NULL || header == NULL || value == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    if (is_literal)
    {
        creq_HeaderField_t *pNewHeader = creq_HeaderField_create_literal(header, value);
        cvector_push_back(req->header_vector, pNewHeader);
    }
    else
    {
        creq_HeaderField_t *pNewHeader = creq_HeaderField_create(header, value);
        cvector_push_back(req->header_vector, pNewHeader);
    }
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_HeaderField_t *)
creq_Request_search_for_header(creq_Request_t *req, char *header)
{
    if (req == NULL || req->header_vector == NULL || header == NULL)
    {
        return NULL;
    }
    for (int i = 0; i < cvector_size(req->header_vector); ++i)
    {
        if (!strcmp(req->header_vector[i]->field_name, header))
        {
            return req->header_vector[i];
        }
    }
    return NULL;
}

CREQ_PUBLIC(int)
creq_Request_search_for_header_index(creq_Request_t *req, char *header)
{
    if (req == NULL || req->header_vector == NULL || header == NULL)
    {
        return -1;
    }
    for (int i = 0; i < cvector_size(req->header_vector); ++i)
    {
        if (!strcmp(req->header_vector[i]->field_name, header))
        {
            return i;
        }
    }
    return -1;
}

CREQ_PUBLIC(creq_status_t)
creq_Request_remove_header(creq_Request_t *req, char *header)
{
    if (req == NULL || header == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    int idx = creq_Request_search_for_header_index(req, header);
    if (idx >= 0)
    {
        creq_HeaderField_t *pNode = req->header_vector[idx];
        creq_HeaderField_free(&pNode);
        cvector_erase(req->header_vector, idx);
        return CREQ_STATUS_SUCC;
    }
    return CREQ_STATUS_FAILED;
}

CREQ_PUBLIC(creq_status_t)
creq_Request_remove_header_direct(creq_Request_t *req, creq_HeaderField_t *header)
{
    if (req == NULL || header == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    for (int i = 0; i < cvector_size(req->header_vector); ++i)
    {
        if (req->header_vector[i] == header)
        {
            creq_HeaderField_t *pNode = req->header_vector[i];
            creq_HeaderField_free(&pNode);
            cvector_erase(req->header_vector, i);
            return CREQ_STATUS_SUCC;
        }
    }

    return CREQ_STATUS_FAILED;
}

CREQ_PUBLIC(creq_status_t)
creq_Request_set_message_body(creq_Request_t *req, char *msg, bool is_literal)
{
    if (req == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    if (is_literal)
    {
        if (!req->is_message_body_literal)
            CREQ_GUARDED_FREE(req->message_body);
        if (msg == NULL)
        {
            req->message_body = NULL;
            return CREQ_STATUS_SUCC;
        }
        req->message_body = (char *)msg;
        req->is_message_body_literal = true;
    }
    else
    {
        if (!req->is_message_body_literal)
            CREQ_GUARDED_FREE(req->message_body);
        if (msg == NULL)
        {
            req->message_body = NULL;
            return CREQ_STATUS_SUCC;
        }
        char *pMsgCopy = _creq_malloc_strcpy(msg);
        req->message_body = pMsgCopy;
        req->is_message_body_literal = false;
    }
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_status_t)
creq_Request_set_message_body_content_len(creq_Request_t *req, char *msg, bool is_literal)
{
    if (creq_Request_set_message_body(req, msg, is_literal) == CREQ_STATUS_FAILED)
    {
        return CREQ_STATUS_FAILED;
    }
    creq_Request_remove_header(req, "Content-Length");
    size_t content_len = strlen(req->message_body);
    char *content_len_s = NULL;
    int content_len_s_len = snprintf(NULL, 0, "%zu", content_len);
    content_len_s = (char *)_creq_malloc_n_init(sizeof(char) * (content_len_s_len + 1));
    snprintf(content_len_s, content_len_s_len + 1, "%zu", content_len);
    creq_Request_add_header(req, "Content-Length", content_len_s, false); // content_len_s is never a literal.
    CREQ_GUARDED_FREE(content_len_s);
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(char *)
creq_Request_get_message_body(creq_Request_t *req)
{
    if (req == NULL)
    {
        return NULL;
    }
    return req->message_body;
}

CREQ_PUBLIC(char *)
creq_Request_stringify(creq_Request_t *req)
{
    if (req == NULL)
    {
        return NULL;
    }
    char *request_line_s = NULL, *headers_s = NULL, *body_s = NULL;

    const char *line_ending_s = _creq_get_line_ending_str(&req->config, CONF_REQUEST);
    const char *http_meth_s = _creq_get_http_method_str(req->method);
    char *http_version_s = _creq_get_http_version_str(req->http_version.major, req->http_version.minor);

    int request_line_len =
        snprintf(NULL, 0, _creq_FMT_REQUEST_LINE, http_meth_s, req->request_target, http_version_s, line_ending_s);
    request_line_s = (char *)_creq_malloc_n_init(sizeof(char) * (request_line_len + 1));
    snprintf(request_line_s, request_line_len + 1, _creq_FMT_REQUEST_LINE, http_meth_s, req->request_target,
             http_version_s, line_ending_s);
    CREQ_GUARDED_FREE(http_version_s);
    // we now have a request_line_s string. Others are garbage now, except for line_ending_s

    size_t header_list_len = cvector_size(req->header_vector);
    if (header_list_len != 0)
    {
        size_t idx = 0;
        int str_len = 0;
        cvector_VECTOR(creq_HeaderField_t *) hv = req->header_vector;

        for (idx = 0; idx < header_list_len; idx++)
        {
            str_len += snprintf(NULL, 0, _creq_FMT_HEADER, hv[idx]->field_name, hv[idx]->field_value, line_ending_s);
        }
        headers_s = (char *)_creq_malloc_n_init(sizeof(char) * (str_len + 1));
        for (idx = 0; idx < header_list_len; idx++)
        {
            char *str;
            int len = snprintf(NULL, 0, _creq_FMT_HEADER, hv[idx]->field_name, hv[idx]->field_value, line_ending_s);
            str = (char *)_creq_malloc_n_init(sizeof(char) * (len + 1));
            snprintf(str, len + 1, _creq_FMT_HEADER, hv[idx]->field_name, hv[idx]->field_value, line_ending_s);
            strcat(headers_s, str);
            CREQ_GUARDED_FREE(str);
        }
    }

    body_s = req->message_body;

    char *full_req_s = NULL;
    int full_req_len =
        snprintf(NULL, 0, _creq_FMT_FULL_REQUEST, request_line_s, headers_s == NULL ? line_ending_s : headers_s,
                 line_ending_s, body_s == NULL ? "" : body_s);
    full_req_s = (char *)_creq_malloc_n_init(sizeof(char) * (full_req_len + 1));
    snprintf(full_req_s, full_req_len + 1, _creq_FMT_FULL_REQUEST, request_line_s,
             headers_s == NULL ? line_ending_s : headers_s, line_ending_s, body_s == NULL ? "" : body_s);

    CREQ_GUARDED_FREE(request_line_s);
    CREQ_GUARDED_FREE(headers_s);

    return full_req_s;
}

CREQ_PUBLIC(creq_Response_t *)
creq_Response_create(creq_Config_t *conf)
{
    creq_Response_t *pResponse = (creq_Response_t *)_creq_malloc_n_init(sizeof(struct creq_Response));

    if (conf != NULL && conf->config_type == CONF_RESPONSE)
    {
        pResponse->config = *conf;
    }
    else
    {
        creq_Config_t config;
        config.config_type = CONF_RESPONSE;
        config.data.response_config.line_ending = LE_CRLF;
        pResponse->config = config;
    }

    pResponse->http_version.major = 0;
    pResponse->http_version.minor = 0;
    pResponse->status_code = 0;
    pResponse->reason_phrase = NULL;
    pResponse->is_reason_phrase_literal = false;
    pResponse->message_body = NULL;
    pResponse->is_message_body_literal = false;
    pResponse->header_vector = NULL;

    return pResponse;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_free(creq_Response_t *resp)
{
    if (resp != NULL)
    {
        // free pointer members
        if (!resp->is_reason_phrase_literal)
            CREQ_GUARDED_FREE(resp->reason_phrase);
        if (!resp->is_message_body_literal)
            CREQ_GUARDED_FREE(resp->message_body);
        creq_HeaderField_t *pHeader = NULL;
        size_t szNowSize = cvector_size(resp->header_vector);
        while (szNowSize > 0)
        {
            pHeader = resp->header_vector[szNowSize - 1];
            cvector_pop_back(resp->header_vector);
            creq_HeaderField_free(&pHeader);
            szNowSize = cvector_size(resp->header_vector);
        }
        cvector_free(resp->header_vector);
        // finally
        free(resp);
        return CREQ_STATUS_SUCC;
    }
    return CREQ_STATUS_FAILED;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_set_http_version(creq_Response_t *resp, int major, int minor)
{
    if (resp == NULL)
        return CREQ_STATUS_FAILED;

    resp->http_version.major = major;
    resp->http_version.minor = minor;

    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_HttpVersion_t)
creq_Response_get_http_version(creq_Response_t *resp)
{
    creq_HttpVersion_t obj;
    obj.major = -1;
    obj.minor = -1;
    if (resp == NULL)
    {
        return obj;
    }
    obj.major = resp->http_version.major;
    obj.minor = resp->http_version.minor;
    return obj;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_set_status_code(creq_Response_t *resp, int status)
{
    if (resp == NULL)
        return CREQ_STATUS_FAILED;
    resp->status_code = status;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_set_reason_phrase(creq_Response_t *resp, char *reason)
{
    if (resp == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    if (!resp->is_reason_phrase_literal)
        CREQ_GUARDED_FREE(resp->reason_phrase);
    if (reason == NULL)
    {
        resp->reason_phrase = NULL;
        return CREQ_STATUS_SUCC;
    }
    char *pReasonCopy = _creq_malloc_strcpy(reason);
    resp->reason_phrase = pReasonCopy;
    resp->is_reason_phrase_literal = false;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_set_reason_phrase_literal(creq_Response_t *resp, const char *reason_s)
{
    if (resp == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    if (!resp->is_reason_phrase_literal)
        CREQ_GUARDED_FREE(resp->reason_phrase);
    if (reason_s == NULL)
    {
        resp->reason_phrase = NULL;
        return CREQ_STATUS_SUCC;
    }
    resp->reason_phrase = (char *)reason_s;
    resp->is_reason_phrase_literal = true;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(int)
creq_Response_get_status_code(creq_Response_t *resp)
{
    if (resp == NULL)
        return 0;
    return resp->status_code;
}

CREQ_PUBLIC(char *)
creq_Response_get_reason_phrase(creq_Response_t *resp)
{
    if (resp == NULL)
        return NULL;
    return resp->reason_phrase;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_add_header(creq_Response_t *resp, char *header, char *value)
{
    if (resp == NULL || header == NULL || value == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    creq_HeaderField_t *pNewHeader = creq_HeaderField_create(header, value);
    cvector_push_back(resp->header_vector, pNewHeader);
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_add_header_literal(creq_Response_t *resp, const char *header_s, const char *value_s)
{
    if (resp == NULL || header_s == NULL || value_s == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    creq_HeaderField_t *pNewHeader = creq_HeaderField_create_literal(header_s, value_s);
    cvector_push_back(resp->header_vector, pNewHeader);
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_HeaderField_t *)
creq_Response_search_for_header(creq_Response_t *resp, char *header)
{
    if (resp == NULL || resp->header_vector == NULL || header == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < cvector_size(resp->header_vector); ++i)
    {
        if (!strcmp(resp->header_vector[i]->field_name, header))
        {
            return resp->header_vector[i];
        }
    }
    return NULL;
}

CREQ_PUBLIC(int)
creq_Response_search_for_header_index(creq_Response_t *resp, char *header)
{
    if (resp == NULL || resp->header_vector == NULL || header == NULL)
    {
        return -1;
    }
    for (int i = 0; i < cvector_size(resp->header_vector); ++i)
    {
        if (!strcmp(resp->header_vector[i]->field_name, header))
        {
            return i;
        }
    }
    return -1;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_remove_header(creq_Response_t *resp, char *header)
{
    if (resp == NULL || header == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    int idx = creq_Response_search_for_header_index(resp, header);
    if (idx >= 0)
    {
        creq_HeaderField_t *pNode = resp->header_vector[idx];
        creq_HeaderField_free(&pNode);
        cvector_erase(resp->header_vector, idx);
        return CREQ_STATUS_SUCC;
    }
    return CREQ_STATUS_FAILED;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_remove_header_direct(creq_Response_t *resp, creq_HeaderField_t *header)
{
    if (resp == NULL || header == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    for (int i = 0; i < cvector_size(resp->header_vector); ++i)
    {
        if (resp->header_vector[i] == header)
        {
            creq_HeaderField_t *pNode = resp->header_vector[i];
            creq_HeaderField_free(&pNode);
            cvector_erase(resp->header_vector, i);
            return CREQ_STATUS_SUCC;
        }
    }

    return CREQ_STATUS_FAILED;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_set_message_body(creq_Response_t *resp, char *msg)
{
    if (resp == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    if (!resp->is_message_body_literal)
        CREQ_GUARDED_FREE(resp->message_body);
    if (msg == NULL)
    {
        resp->message_body = NULL;
        return CREQ_STATUS_SUCC;
    }
    char *pMsgCopy = _creq_malloc_strcpy(msg);
    resp->message_body = pMsgCopy;
    resp->is_message_body_literal = false;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_set_message_body_content_len(creq_Response_t *resp, char *msg)
{
    if (creq_Response_set_message_body(resp, msg) == CREQ_STATUS_FAILED)
    {
        return CREQ_STATUS_FAILED;
    }
    creq_Response_remove_header(resp, "Content-Length");
    size_t content_len = strlen(resp->message_body);
    char *content_len_s = NULL;
    int content_len_s_len = snprintf(NULL, 0, "%zu", content_len);
    content_len_s = (char *)_creq_malloc_n_init(sizeof(char) * (content_len_s_len + 1));
    snprintf(content_len_s, content_len_s_len + 1, "%zu", content_len);
    creq_Response_add_header(resp, "Content-Length", content_len_s);
    CREQ_GUARDED_FREE(content_len_s);
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_set_message_body_literal(creq_Response_t *resp, const char *msg_s)
{
    if (resp == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    if (!resp->is_message_body_literal)
        CREQ_GUARDED_FREE(resp->message_body);
    if (msg_s == NULL)
    {
        resp->message_body = NULL;
        return CREQ_STATUS_SUCC;
    }
    resp->message_body = (char *)msg_s;
    resp->is_message_body_literal = true;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_status_t)
creq_Response_set_message_body_literal_content_len(creq_Response_t *resp, const char *msg_s)
{
    if (creq_Response_set_message_body_literal(resp, msg_s) == CREQ_STATUS_FAILED)
    {
        return CREQ_STATUS_FAILED;
    }
    creq_Response_remove_header(resp, "Content-Length");
    size_t content_len = strlen(resp->message_body);
    char *content_len_s = NULL;
    int content_len_s_len = snprintf(NULL, 0, "%zu", content_len);
    content_len_s = (char *)_creq_malloc_n_init(sizeof(char) * (content_len_s_len + 1));
    snprintf(content_len_s, content_len_s_len + 1, "%zu", content_len);
    creq_Response_add_header(resp, "Content-Length", content_len_s);
    CREQ_GUARDED_FREE(content_len_s);
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(char *)
creq_Response_get_message_body(creq_Response_t *resp)
{
    if (resp == NULL)
        return NULL;
    return resp->message_body;
}

CREQ_PUBLIC(char *)
creq_Response_stringify(creq_Response_t *resp)
{
    if (resp == NULL)
        return NULL;
    char *status_line_s = NULL, *headers_s = NULL, *body_s = NULL;

    const char *line_ending_s = _creq_get_line_ending_str(&resp->config, CONF_RESPONSE);
    char *http_version_s = _creq_get_http_version_str(resp->http_version.major, resp->http_version.minor);

    int status_line_len =
        snprintf(NULL, 0, _creq_FMT_STATUS_LINE, http_version_s, resp->status_code, resp->reason_phrase, line_ending_s);
    status_line_s = (char *)_creq_malloc_n_init(sizeof(char) * (status_line_len + 1));
    snprintf(status_line_s, status_line_len + 1, _creq_FMT_STATUS_LINE, http_version_s, resp->status_code,
             resp->reason_phrase, line_ending_s);
    CREQ_GUARDED_FREE(http_version_s);

    size_t header_list_len = cvector_size(resp->header_vector);
    if (header_list_len != 0)
    {
        size_t idx = 0;
        int str_len = 0;
        cvector_VECTOR(creq_HeaderField_t *) hv = resp->header_vector;

        for (idx = 0; idx < header_list_len; idx++)
        {
            str_len += snprintf(NULL, 0, _creq_FMT_HEADER, hv[idx]->field_name, hv[idx]->field_value, line_ending_s);
        }
        headers_s = (char *)_creq_malloc_n_init(sizeof(char) * (str_len + 1));
        for (idx = 0; idx < header_list_len; idx++)
        {
            char *str;
            int len = snprintf(NULL, 0, _creq_FMT_HEADER, hv[idx]->field_name, hv[idx]->field_value, line_ending_s);
            str = (char *)_creq_malloc_n_init(sizeof(char) * (len + 1));
            snprintf(str, len + 1, _creq_FMT_HEADER, hv[idx]->field_name, hv[idx]->field_value, line_ending_s);
            strcat(headers_s, str);
            CREQ_GUARDED_FREE(str);
        }
    }

    body_s = resp->message_body;

    char *full_resp_s = NULL;
    int full_resp_len =
        snprintf(NULL, 0, _creq_FMT_FULL_RESPONSE, status_line_s, headers_s == NULL ? line_ending_s : headers_s,
                 line_ending_s, body_s == NULL ? "" : body_s);
    full_resp_s = (char *)_creq_malloc_n_init(sizeof(char) * (full_resp_len + 1));
    snprintf(full_resp_s, full_resp_len + 1, _creq_FMT_FULL_RESPONSE, status_line_s,
             headers_s == NULL ? line_ending_s : headers_s, line_ending_s, body_s == NULL ? "" : body_s);

    CREQ_GUARDED_FREE(status_line_s);
    CREQ_GUARDED_FREE(headers_s);

    return full_resp_s;
}
