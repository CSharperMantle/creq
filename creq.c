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

    free(pField->field_name);
    pField->field_name = NULL;
    free(pField->field_value);
    pField->field_value = NULL;

    free(pField);
    pField = NULL;
    node->data = NULL;
    node->next = NULL;
    node->prev = NULL;
    free(node);
    return CREQ_STATUS_CODE_SUCC;
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
    pRequest->first_header = NULL;
    pRequest->message_body = NULL;

    return pRequest;
}

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_Request_free(creq_Request_t *req)
{
    if (req != NULL)
    {
        // free pointer members
        req->request_target = NULL;
        req->message_body = NULL;
        // free linked list
        creq_HeaderLListNode_t *pNodeCursor = req->first_header;
        if (pNodeCursor != NULL)
        {
            while (pNodeCursor->next != NULL) // find the last element
            {
                pNodeCursor = pNodeCursor->next;
            }
            while (pNodeCursor != NULL)
            {
                if (pNodeCursor->prev != NULL)
                {
                    pNodeCursor->prev->next = pNodeCursor->next;
                }
                else
                {
                    req->first_header = pNodeCursor->next;
                }
                if (pNodeCursor->next != NULL)
                {
                    pNodeCursor->next->prev = pNodeCursor->prev;
                }
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

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_Request_add_header(creq_Request_t *req, const char *header, const char *value)
{
    if (req == NULL || header == NULL || value == NULL) // guarded
        return CREQ_STATUS_CODE_FAILED;

    creq_HeaderLListNode_t *pNewNode = creq_HeaderLListNode_create(header, value);
    if (req->first_header == NULL) // first element?
    {
        req->first_header = pNewNode;
        return CREQ_STATUS_CODE_SUCC;
    }
    // not the first element
    creq_HeaderLListNode_t *pNodeCursor = req->first_header;
    while (pNodeCursor->next != NULL) // find the last element
    {
        pNodeCursor = pNodeCursor->next;
    }
    pNodeCursor->next = pNewNode;
    pNewNode->prev = pNodeCursor;

    return CREQ_STATUS_CODE_SUCC;
}
