/**
 * @file creq.c
 * @brief Implementation for functions defined in creq.h
 * @author CSharperMantle
 */

#include <stdlib.h>
#include <string.h>
#include "creq.h"


static void *_creq_malloc_n_init(size_t size)
{
    void *pNewSpace = malloc(size);
    memset(pNewSpace, 0, size);
    return pNewSpace;
}

///@attention Don't forget to free the pointer returned!
static char *_creq_malloc_strcpy(const char *src)
{
    size_t fullLen = strlen(src) + 1;
    char *dest = (char *)malloc(sizeof(char) * fullLen);
    strcpy(dest, src);
    return dest;
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

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_HeaderLListNode_free(creq_HeaderLListNode_t *node)
{
    if (node == NULL || node->data == NULL || node->data->field_name == NULL || node->data->field_value == NULL)
    {
        return CREQ_STATUS_CODE_FAILED;
    }
    creq_HeaderField_t *pField = node->data;

    CREQ_GUARDED_FREE(pField->field_name);
    CREQ_GUARDED_FREE(pField->field_value);

    CREQ_GUARDED_FREE(pField);
    node->data = NULL;
    node->next = NULL;
    node->prev = NULL;
    free(node);
    return CREQ_STATUS_CODE_SUCC;
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

CREQ_PUBLIC(creq_Request_t *) creq_Request_create()
{
    creq_Request_t *pRequest = (creq_Request_t *)_creq_malloc_n_init(sizeof(struct creq_Request));

    creq_Config_t config;
    config.config_type = REQUEST;
    config.data.request_config.line_ending = CRLF;
    pRequest->config = config;
    pRequest->method = 0;
    pRequest->request_target = NULL;
    pRequest->http_version = 0;
    pRequest->list_head = NULL;
    pRequest->message_body = NULL;

    return pRequest;
}

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_Request_free(creq_Request_t *req)
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
                creq_Request_delist_header_direct(req, pNodeCursor);
                creq_HeaderLListNode_t *pPrev = pNodeCursor->prev;
                creq_HeaderLListNode_free(pNodeCursor);
                pNodeCursor = pPrev; ///@todo Test required!
            }
        }
        // finally
        free(req);
        return CREQ_STATUS_CODE_SUCC;
    }
    return CREQ_STATUS_CODE_FAILED;
}

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_Request_set_http_method(creq_Request_t *req, creq_HttpMethod_t method)
{
    if (req == NULL)
    {
        return CREQ_STATUS_CODE_FAILED;
    }
    req->method = method;
    return CREQ_STATUS_CODE_SUCC;
}

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_Request_set_target(creq_Request_t *req, const char *requestTarget)
{
    if (req == NULL || requestTarget == NULL)
    {
        return CREQ_STATUS_CODE_FAILED;
    }
    CREQ_GUARDED_FREE(req->request_target); // has it been set previously? check it first, free it if necessary...
    // ...and then do what we are supposed to do :D
    char *pReqTargetCopy = _creq_malloc_strcpy(requestTarget);
    req->request_target = pReqTargetCopy;
    return CREQ_STATUS_CODE_SUCC;
}

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_Request_set_http_version(creq_Request_t *req, int major, int minor)
{
    ///@todo To be implemented.
    return CREQ_STATUS_CODE_FAILED;
}

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_Request_add_header(creq_Request_t *req, const char *header, const char *value)
{
    if (req == NULL || header == NULL || value == NULL) // guarded
        return CREQ_STATUS_CODE_FAILED;

    creq_HeaderLListNode_t *pNewNode = creq_HeaderLListNode_create(header, value);
    if (req->list_head == NULL) // first element?
    {
        req->list_head = pNewNode;
        return CREQ_STATUS_CODE_SUCC;
    }
    // not the first element
    creq_HeaderLListNode_t *pNodeCursor = req->list_head;
    while (pNodeCursor->next != NULL) // find the last element
    {
        pNodeCursor = pNodeCursor->next;
    }
    pNodeCursor->next = pNewNode;
    pNewNode->prev = pNodeCursor;

    return CREQ_STATUS_CODE_SUCC;
}

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_Request_delist_header_direct(creq_Request_t *req, creq_HeaderLListNode_t *node)
{
    if (req == NULL || node == NULL)
    {
        return CREQ_STATUS_CODE_FAILED;
    }
    if (node->prev != NULL)
    {
        node->prev->next = node->next;
    }
    else
    {
        req->list_head = node->next;
    }
    if (node->next != NULL)
    {
        node->next->prev = node->prev;
    }
    return CREQ_STATUS_CODE_SUCC;
}

CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_Request_delist_header(creq_Request_t *req, const char *header)
{
    if (req == NULL || header == NULL)
    {
        return NULL;
    }
    // look for a node
    creq_HeaderLListNode_t *pNodeToDelete = creq_HeaderLListNode_search_for_header(req->list_head, header);
    if (pNodeToDelete == NULL)
    {
        return NULL;
    }
    // node found!
    creq_Request_delist_header_direct(req, pNodeToDelete);
    return pNodeToDelete;
}
