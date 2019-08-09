/**
 * @file creq.c
 * @brief Implementation for functions defined in creq.h
 * @author CSharperMantle
 */

#include <stdlib.h>
#include <string.h>
#include "creq.h"


void *_creq_malloc_n_init(size_t size)
{
    void *pNewSpace = malloc(size);
    memset(pNewSpace, 0, size);
    return pNewSpace;
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
        creq_LinkedListNode_t *pNodeCursor = req->first_header;
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
                creq_LinkedListNode_t *pPrev = pNodeCursor->prev;
                free(pNodeCursor->data);
                free(pNodeCursor);
                pNodeCursor = pPrev; ///@todo Test required!
            }
        }
        // finally
        free(req);
        return 0;
    }
    return 1;
}

CREQ_PUBLIC(CREQ_STATUS_CODE) creq_Request_add_header(creq_Request_t *req, const char *header, const char *value)
{
    if (req == NULL || header == NULL || value == NULL) // guarded
        return 1;

    creq_HeaderField_t *pNewHeader = (creq_HeaderField_t *)_creq_malloc_n_init(sizeof(struct creq_HeaderField));
    pNewHeader->field_name = header;
    pNewHeader->field_value = value;
    ///@todo For some reason, I don't know how strings are proceeded in C. Need to pay more attention to string copying.

    creq_LinkedListNode_t *pNewNode = (creq_LinkedListNode_t *)_creq_malloc_n_init(sizeof(struct creq_LinkedListNode));
    pNewNode->data = pNewHeader;
    if (req->first_header == NULL) // first element?
    {
        req->first_header = pNewNode;
        return 0;
    }
    // not the first element
    creq_LinkedListNode_t *pNodeCursor = req->first_header;
    while (pNodeCursor->next != NULL) // find the last element
    {
        pNodeCursor = pNodeCursor->next;
    }
    pNodeCursor->next = pNewNode;
    pNewNode->prev = pNodeCursor;

    return 0;
}
