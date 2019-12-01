/**
 * @file creq.c
 * @brief Implementation for functions defined in creq.h
 * @author CSharperMantle
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "creq.h"

// $(method) /target/path HTTP/1.1 line_ending(\r, \n || \r\n)
CREQ_PRIVATE(const char *) _creq_FMT_REQUEST_STATUS_LINE = "%s %s %s%s";

// HTTP/$(major).$(minor)
CREQ_PRIVATE(const char *) _creq_FMT_HTTP_VERSION = "HTTP/%d.%d";

// Header-Head: Header-Value line_ending
CREQ_PRIVATE(const char *) _creq_FMT_HEADER = "%s: %s%s";

// REQUEST_STATUS_LINE(with ending)
// HEADER(with ending)
// BODY
// line_ending
CREQ_PRIVATE(const char *) _creq_FMT_FULL_REQUEST = "%s%s%s%s";

CREQ_PRIVATE(void *) _creq_malloc_n_init(size_t size)
{
    void *pNewSpace = malloc(size);
    memset(pNewSpace, 0, size);
    return pNewSpace;
}

/// @attention Don't forget to free the pointer returned!
CREQ_PRIVATE(char *) _creq_malloc_strcpy(const char *src)
{
    size_t fullLen = strlen(src) + 1;
    char *dest = (char *)malloc(sizeof(char) * fullLen);
    strcpy(dest, src);
    return dest;
}

CREQ_PRIVATE(const char *) _creq_get_line_ending_str(creq_Config_t *conf, creq_ConfigType_t confType)
{
    if (conf == NULL)
    {
        return NULL;
    }
    creq_LineEnding_t ending = LE_CRLF;
    if (confType == CONF_REQUEST)
    {
        ending = conf->data.request_config.line_ending;
    } else if (confType == CONF_RESPONSE)
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

CREQ_PRIVATE(const char *) _creq_get_http_method_str(creq_HttpMethod_t meth)
{
    switch (meth)
    {
    case GET:
        return "GET";
    case HEAD:
        return "HEAD";
    case POST:
        return "POST";
    case PUT:
        return "PUT";
    case DELETE:
        return "DELETE";
    case CONNECT:
        return "CONNECT";
    case OPTIONS:
        return "OPTIONS";
    case TRACE:
        return "TRACE";
    default:
        return NULL;
    }
}

/// @attention Don't forget to free the returned string! THE STR IS MALLOC'ED!
CREQ_PRIVATE(char *) _creq_get_http_version_str(int major, int minor)
{
    int len = snprintf((char *)NULL, 0U, _creq_FMT_HTTP_VERSION, major, minor);
    char *str = (char *)_creq_malloc_n_init(sizeof(char) * (len + 1));
    snprintf(str, len + 1, _creq_FMT_HTTP_VERSION, major, minor);
    return str;
}

CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_create(const char *header, const char *value)
{
    if (header == NULL || value == NULL) // guarded
    {
        return NULL;
    }
    creq_HeaderLListNode_t *pNode = (creq_HeaderLListNode_t *)_creq_malloc_n_init(sizeof(struct creq_HeaderLListNode));

    creq_HeaderField_t *pField = (creq_HeaderField_t *)_creq_malloc_n_init(sizeof(struct creq_HeaderField));
    pField->field_name = _creq_malloc_strcpy(header);
    pField->field_value = _creq_malloc_strcpy(value);

    pNode->data = pField;
    return pNode;
}

CREQ_PUBLIC(creq_status_t) creq_HeaderLListNode_free(creq_HeaderLListNode_t *node)
{
    if (node == NULL || node->data == NULL || node->data->field_name == NULL || node->data->field_value == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    creq_HeaderField_t *pField = node->data;

    CREQ_GUARDED_FREE(pField->field_name);
    CREQ_GUARDED_FREE(pField->field_value);

    CREQ_GUARDED_FREE(pField);
    node->data = NULL;
    node->next = NULL;
    node->prev = NULL;
    free(node);
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_search_for_header(creq_HeaderLListNode_t *head, const char *header)
{
    if (header == NULL)
    {
        return NULL;
    }
    creq_HeaderLListNode_t *pCursor = head;
    ///@todo Assumes that every node object has a non-NULL data field. Do more checks.
    ///@bug Maybe a bug here.
    while (pCursor != NULL && strcmp(pCursor->data->field_name, header))
    {
        pCursor = pCursor->next;
    }
    return pCursor;
}

CREQ_PUBLIC(creq_status_t) creq_HeaderLListNode_add_header(creq_HeaderLListNode_t **head, const char *header, const char *value)
{
    if (head == NULL || header == NULL || value == NULL) // guarded
        return CREQ_STATUS_FAILED;

    creq_HeaderLListNode_t *pNewNode = creq_HeaderLListNode_create(header, value);
    if (*head == NULL) // first element?
    {
        *head = pNewNode;
        return CREQ_STATUS_SUCC;
    }
    // not the first element
    creq_HeaderLListNode_t *pNodeCursor = *head;
    while (pNodeCursor->next != NULL) // find the last element
    {
        pNodeCursor = pNodeCursor->next;
    }
    pNodeCursor->next = pNewNode;
    pNewNode->prev = pNodeCursor;

    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_status_t) creq_HeaderLListNode_delist_header_direct(creq_HeaderLListNode_t **head, creq_HeaderLListNode_t *node)
{
    if (head == NULL || node == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    if (node->prev != NULL)
    {
        node->prev->next = node->next;
    }
    else
    {
        *head = node->next;
    }
    if (node->next != NULL)
    {
        node->next->prev = node->prev;
    }
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_delist_header(creq_HeaderLListNode_t **head, const char *header)
{
    if (head == NULL || header == NULL)
    {
        return NULL;
    }
    // look for a node
    creq_HeaderLListNode_t *pNodeToDelete = creq_HeaderLListNode_search_for_header(*head, header);
    if (pNodeToDelete == NULL)
    {
        return NULL;
    }
    // node found!
    creq_HeaderLListNode_delist_header_direct(head, pNodeToDelete);
    return pNodeToDelete;
}

CREQ_PUBLIC(creq_Request_t *) creq_Request_create(creq_Config_t *conf)
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
    pRequest->method = _UNKNOWN;
    pRequest->request_target = NULL;
    pRequest->http_version.major = 0;
    pRequest->http_version.minor = 0;
    pRequest->list_head = NULL;
    pRequest->message_body = NULL;

    return pRequest;
}

CREQ_PUBLIC(creq_status_t) creq_Request_free(creq_Request_t *req)
{
    if (req != NULL)
    {
        // free pointer members
        CREQ_GUARDED_FREE(req->request_target);
        CREQ_GUARDED_FREE(req->message_body);
        // free linked list
        creq_HeaderLListNode_t *pNodeCursor = req->list_head;
        if (pNodeCursor != NULL)
        {
            while (pNodeCursor->next != NULL) // find the last element
            {
                pNodeCursor = pNodeCursor->next;
            }
            while (pNodeCursor != NULL)
            {
                creq_HeaderLListNode_delist_header_direct(&req->list_head, pNodeCursor);
                creq_HeaderLListNode_t *pPrev = pNodeCursor->prev;
                creq_HeaderLListNode_free(pNodeCursor);
                pNodeCursor = pPrev; ///@todo Test required!
            }
        }
        // finally
        free(req);
        return CREQ_STATUS_SUCC;
    }
    return CREQ_STATUS_FAILED;
}

CREQ_PUBLIC(creq_status_t) creq_Request_set_http_method(creq_Request_t *req, creq_HttpMethod_t method)
{
    if (req == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    req->method = method;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_HttpMethod_t) creq_Request_get_http_method(creq_Request_t *req)
{
    if (req == NULL)
    {
        return _UNKNOWN;
    }
    return req->method;
}

CREQ_PUBLIC(creq_status_t) creq_Request_set_target(creq_Request_t *req, const char *requestTarget)
{
    if (req == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    CREQ_GUARDED_FREE(req->request_target); // has it been set previously? check it first, free it if necessary...
    if (requestTarget == NULL) // user requests to clear
    {
        req->request_target = NULL;
        return CREQ_STATUS_SUCC;
    }
    // ...and then do what we are supposed to do :D
    char *pReqTargetCopy = _creq_malloc_strcpy(requestTarget);
    req->request_target = pReqTargetCopy;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(char *) creq_Request_get_target(creq_Request_t *req)
{
    if (req == NULL)
    {
        return NULL;
    }
    return req->request_target;
}

CREQ_PUBLIC(creq_status_t) creq_Request_set_http_version(creq_Request_t *req, int major, int minor)
{
    if (req == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    req->http_version.major = major;
    req->http_version.minor = minor;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(creq_HttpVersion_t) creq_Request_get_http_version(creq_Request_t *req)
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

CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body(creq_Request_t *req, const char *msg)
{
    if (req == NULL)
    {
        return CREQ_STATUS_FAILED;
    }
    CREQ_GUARDED_FREE(req->message_body);
    if (msg == NULL)
    {
        req->message_body = NULL;
        return CREQ_STATUS_SUCC;
    }
    char *pMsgCopy = _creq_malloc_strcpy(msg);
    req->message_body = pMsgCopy;
    return CREQ_STATUS_SUCC;
}

CREQ_PUBLIC(char *) creq_Request_get_message_body(creq_Request_t *req)
{
    if (req == NULL)
    {
        return NULL;
    }
    return req->message_body;
}

CREQ_PUBLIC(char *) creq_Request_stringify(creq_Request_t *req)
{
    if (req == NULL)
    {
        return NULL;
    }
    char *status_line_s = NULL, *headers_s = NULL, *body_s = NULL;

    const char *line_ending_s = _creq_get_line_ending_str(&req->config, CONF_REQUEST);
    const char *http_meth_s = _creq_get_http_method_str(req->method);
    char *http_version_s = _creq_get_http_version_str(req->http_version.major, req->http_version.minor);

    int status_line_len = snprintf(NULL, 0, _creq_FMT_REQUEST_STATUS_LINE, http_meth_s, req->request_target, http_version_s, line_ending_s);
    status_line_s = (char *)_creq_malloc_n_init(sizeof(char) * (status_line_len + 1));
    snprintf(status_line_s, status_line_len + 1, _creq_FMT_REQUEST_STATUS_LINE, http_meth_s, req->request_target, http_version_s, line_ending_s);
    CREQ_GUARDED_FREE(http_version_s);

    // we now have a status_line_s string. Others are garbage now, except for line_ending_s

    // walk the linked list
    size_t header_list_len = 0;
    creq_HeaderLListNode_t *cursor = req->list_head;
    while (cursor != NULL)
    {
        header_list_len ++;
        cursor = cursor->next;
    }
    if (header_list_len != 0)
    {
        size_t idx = 0;
        int str_len = 0;
        creq_HeaderField_t *fields[header_list_len];
        cursor = req->list_head;
        while (cursor != NULL)
        {
            fields[idx] = cursor->data;
            idx ++;
            cursor = cursor->next;
        }

        for (idx = 0; idx < header_list_len; idx ++)
        {
            str_len += snprintf(NULL, 0, _creq_FMT_HEADER, fields[idx]->field_name, fields[idx]->field_value, line_ending_s);
        }
        headers_s = (char *)_creq_malloc_n_init(sizeof(char) * (str_len + 1));
        for (idx = 0; idx < header_list_len; idx ++)
        {
            char *str;
            int len = snprintf(NULL, 0, _creq_FMT_HEADER, fields[idx]->field_name, fields[idx]->field_value, line_ending_s);
            str = (char *)_creq_malloc_n_init(sizeof(char) * (len + 1));
            snprintf(str, len + 1, _creq_FMT_HEADER, fields[idx]->field_name, fields[idx]->field_value, line_ending_s);
            strcat(headers_s, str);
            CREQ_GUARDED_FREE(str);
        }
    }

    body_s = req->message_body;

    char *full_req_s = NULL;
    int full_len = snprintf(NULL, 0, _creq_FMT_FULL_REQUEST,
                                status_line_s,
                                headers_s == NULL ? line_ending_s : headers_s,
                                body_s == NULL ? "" : body_s,
                                line_ending_s);
    full_req_s = (char *)_creq_malloc_n_init(sizeof(char) * (full_len + 1));
    snprintf(full_req_s, full_len + 1, _creq_FMT_FULL_REQUEST,
             status_line_s,
             headers_s == NULL ? line_ending_s : headers_s,
             body_s == NULL ? "" : body_s,
             line_ending_s);

    CREQ_GUARDED_FREE(status_line_s);
    CREQ_GUARDED_FREE(headers_s);

    /// @todo DEBUG REQUIRED
    return full_req_s;
}
