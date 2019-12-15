/**
 * @file creq.h
 * @brief Main header for creq project.
 * @author CSharperMantle
 */


#ifndef CREQ_H_INCLUDED
#define CREQ_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:
CREQ_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
CREQ_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
CREQ_IMPORT_SYMBOLS - Define this if you want to dllimport symbol
For Unix builds that support visibility attribute, you can define similar behavior by
setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS
then using the CREQ_API_VISIBILITY flag to "export" the same symbols the way CREQ_EXPORT_SYMBOLS does
*/

#define CREQ_CDECL __cdecl
#define CREQ_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(CREQ_HIDE_SYMBOLS) && !defined(CREQ_IMPORT_SYMBOLS) && !defined(CREQ_EXPORT_SYMBOLS)
#define CREQ_EXPORT_SYMBOLS
#endif

#if defined(CREQ_HIDE_SYMBOLS)
#define CREQ_PUBLIC(type)   type CREQ_STDCALL
#elif defined(CREQ_EXPORT_SYMBOLS)
#define CREQ_PUBLIC(type)   __declspec(dllexport) type CREQ_STDCALL
#elif defined(CREQ_IMPORT_SYMBOLS)
#define CREQ_PUBLIC(type)   __declspec(dllimport) type CREQ_STDCALL
#endif
#define CREQ_PRIVATE(type) static type
#else /* !__WINDOWS__ */
#define CREQ_CDECL
#define CREQ_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(CREQ_API_VISIBILITY)
#define CREQ_PUBLIC(type)   __attribute__((visibility("default"))) type
#define CREQ_PRIVATE(type) __attribute__((visibility("hidden"))) type
#else
#define CREQ_PUBLIC(type) type
#define CREQ_PRIVATE(type) static type
#endif
#endif

typedef int creq_status_t;
#define CREQ_STATUS_SUCC 0
#define CREQ_STATUS_FAILED 1

#define CREQ_GUARDED_FREE(ptr) if (ptr != NULL) { free(ptr); ptr = NULL; }

#include <stdbool.h>
#include <wchar.h>

typedef enum creq_LineEnding_e
{
    LE_CR,
    LE_LF,
    LE_CRLF
} creq_LineEnding_t;

typedef enum creq_HttpMethod_e
{
    METH_GET,
    METH_HEAD,
    METH_POST,
    METH_PUT,
    METH_DELETE,
    METH_CONNECT,
    METH_OPTIONS,
    METH_TRACE,
    _METH_UNKNOWN
} creq_HttpMethod_t;

typedef enum creq_ConfigType_e
{
    CONF_REQUEST,
    CONF_RESPONSE
} creq_ConfigType_t;

/**
 * @brief Represents a single header-value pair used in request and response.
 */
typedef struct creq_HeaderField
{
    char *field_name;
    bool is_field_name_literal;
    char *field_value;
    bool is_field_value_literal;
} creq_HeaderField_t;

/**
 * @brief A self-defined node used in bidirectional linked list
 */
typedef struct creq_HeaderLListNode
{
    struct creq_HeaderLListNode *prev;
    creq_HeaderField_t *data;
    struct creq_HeaderLListNode *next;
} creq_HeaderLListNode_t;

/**
 * @brief Configuration used in both request and response.
 */
typedef struct creq_Config
{
    union {
        struct {
            creq_LineEnding_t line_ending;

        } request_config;
        struct {
            creq_LineEnding_t line_ending;

        } response_config;
    } data;
    creq_ConfigType_t config_type;
} creq_Config_t;

typedef struct creq_HttpVersion
{
    unsigned major;
    unsigned minor;
} creq_HttpVersion_t;

/**
 * @brief Inner request struct for response generating.
 * @see RFC7230 Section 3
 */
typedef struct creq_Request
{
    creq_Config_t config;

    // > request-line, Section 3.1.1
    creq_HttpMethod_t method;
    // space
    char *request_target;
    bool is_request_target_literal;
    // space
    creq_HttpVersion_t http_version;
    // space
    // line ending

    // > header field
    creq_HeaderLListNode_t *list_head;
    // line ending after each header
    // line ending again in the end

    char *message_body;
    bool is_message_body_literal;
} creq_Request_t;

/**
 * @brief Inner response struct for response generating.
 * @see RFC7230 Section 3
 */
typedef struct creq_Response
{
    creq_Config_t config;

    // > status-line, Section 3.1.2
    // > mask value, for example (0101)10 for HTTP/1.1
    creq_HttpVersion_t http_version;
    // space
    int status_code;
    // space
    char *reason_phrase;
    bool is_reason_phrase_literal;
    // line ending

    // > header field
    creq_HeaderLListNode_t *list_head;
    // line ending after each header
    // line ending again in the end

    char *message_body;
    bool is_message_body_literal;
} creq_Response_t;

/**
 * @brief Creates a new header list node which contains a header-value pair.
 *  @see creq_HeaderLListNode
 *  @see creq_HeaderField
 * @return A pointer to the newly created node.
 *  @retval NULL Fails to create a new object.
 * @attention DO NOT directly free the node unless it's not inserted into the list.
 * @attention For internal use only.
 */
CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_create(char *header, char *value);

/**
 * @brief Creates a new header list node which contains a header-value pair with literal header and value.
 *  @see creq_HeaderLListNode
 *  @see creq_HeaderField
 * @return A pointer to the newly created node.
 *  @retval NULL Fails to create a new object.
 * @attention DO NOT directly free the node unless it's not inserted into the list.
 * @attention For internal use only.
 * @attention Behaviour will be undefined if non-literals are given.
 */
CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_create_literal(const char *header_s, const char *value_s);

/**
 * @brief Frees a previously-created HeaderLListNode.
 *  @see creq_HeaderLListNode_create
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure only frees the node, the data field, and the strings, set the prev and next pointers to NULL, and delete the object directly.
 * @attention For internal use only.
 * @see creq_HeaderLListNode
 * @see creq_HeaderField
 */
CREQ_PUBLIC(creq_status_t) creq_HeaderLListNode_free(creq_HeaderLListNode_t *node);

/**
 * @brief Search for a header-value pair in the headers list which contains the given header.
 * @return A pointer to the header found.
 *  @retval NULL Header not found.
 * @attention Always return the first one when there are multiple occurrences. To search for multiple header-value pairs, call this procedure iteratively with the last-found header as the head until NULL is returned.
 */
CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_search_for_header(creq_HeaderLListNode_t *head, const char *header);

/**
 * @brief Adds a new item to the tail of the given list.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_HeaderLListNode_add_header(creq_HeaderLListNode_t **head, char *header, char *value);

/**
 * @brief Adds a new item with literal header and value to the tail of the given list.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This function works with literal header and values. If non-literals are given, the result is undefined.
 */
CREQ_PUBLIC(creq_status_t) creq_HeaderLListNode_add_header_literal(creq_HeaderLListNode_t **head, const char *header_s, const char *value_s);

/**
 * @brief Moves a header node out of the list.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure directly operates on the given node pointer.
 * @attention For internal use only.
 */
CREQ_PUBLIC(creq_status_t) creq_HeaderLListNode_delist_header_direct(creq_HeaderLListNode_t **head, creq_HeaderLListNode_t *node);

/**
 * @brief Moves a header from the headers list which has the given header out of the list. Alternative to creq_HeaderLListNode_delist_header_direct.
 *  @see creq_HeaderLListNode_delist_header_direct
 * @return A pointer to the removed header for further process.
 *  @retval NULL Header not found.
 * @attention Always delete the first header found in the list. To delete multiple header-value pairs, call this procedure iteratively until NULL is returned.
 */
CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_delist_header(creq_HeaderLListNode_t **head, char *header);

/**
 * @brief Creates a new creq_Request object.
 *  @see creq_Request
 * @return A pointer to the newly created creq_Request object.
 *  @retval NULL Fails to create a new object.
 * @attention Always use creq_Request_free when done.
 *  @see creq_Request_free
 */
CREQ_PUBLIC(creq_Request_t *) creq_Request_create(creq_Config_t *conf);

/**
 * @brief Frees a previously-created creq_Request object.
 *  @see creq_Request
 *  @see creq_Request_create
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure frees all the resources used in the given object, including all the items in the headers list.
 * @attention So make sure they are malloc'ed!
 */
CREQ_PUBLIC(creq_status_t) creq_Request_free(creq_Request_t *req);

/**
 * @brief Set the creq_Request object's http method.
 *  @see creq_Request::method
 *  @see creq_HttpMethod_e
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_http_method(creq_Request_t *req, creq_HttpMethod_t method);

/**
 * @brief Get the creq_Request object's http method.
 */
CREQ_PUBLIC(creq_HttpMethod_t) creq_Request_get_http_method(creq_Request_t *req);

/**
 * @brief Set the creq_Request object's target to the given string.
 *  @see creq_Request::request_target
 * @param requestTarget The pointer to the new target. NULL will clear the target.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores a copy of the given string.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_target(creq_Request_t *req, char *requestTarget);

/**
 * @brief Set the creq_Request object's target to the given literal.
 *  @see creq_Request::request_target
 * @param requestTarget The pointer to the new target. NULL will clear the target.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores the pointer to the literal directly. Behaviour will be undefined if non-literals are given.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_target_literal(creq_Request_t *req, const char *requestTarget_s);

/**
 * @brief Get the creq_Request object's target
 * @return A pointer to the target string.
 *  @retval NULL No target specified or invalid argument given.
 * @attention The returned pointer points to the internal object. DO NOT MODIFY IT.
 */
CREQ_PUBLIC(char *) creq_Request_get_target(creq_Request_t *req);

/**
 * @brief Set the creq_Request object's http version.
 *  @see creq_Request::http_version
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_http_version(creq_Request_t *req, int major, int minor);

/**
 * @brief Get the creq_Request object's http version.
 * @retval The object which contains the version.
 */
CREQ_PUBLIC(creq_HttpVersion_t) creq_Request_get_http_version(creq_Request_t *req);

/**
 * @brief Adds a new item to the tail of the headers list of the creq_Request object.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_add_header(creq_Request_t *req, char *header, char *value);

/**
 * @brief Adds a new item with literal header and value to the tail of the headers list of the creq_Request object.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This function works with literal header and values. If non-literals are given, the result is undefined.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_add_header_literal(creq_Request_t *req, const char *header_s, const char *value_s);

/**
 * @brief Searches for a header-value pair in the headers list of the creq_Request object which contains the given header.
 * @return A pointer to the header found.
 *  @retval NULL Header not found.
 * @attention Always return the first one when there are multiple occurrences. To search for multiple header-value pairs, call this procedure iteratively with the last-found header as the head until NULL is returned.
 */
CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_Request_search_for_header(creq_Request_t *req, char *header);

/**
 * @brief Moves the header with the given header name out of the creq_Request object's headers list and frees it.
 *  @see creq_HeaderLListNode_delist_header
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention Always delete the first header found in the list.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_remove_header(creq_Request_t *req, char *header);

/**
 * @brief Moves a header node out of the list of the give creq_Request object and frees it.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure directly operates on the given node pointer. Often used with creq_Request_search_for_header .
 * @see creq_Request_search_for_header
 * @attention Behaviour will be undefined if the given header is not in the list.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_remove_header_direct(creq_Request_t *req, creq_HeaderLListNode_t *node);

/**
 * @brief Set the creq_Request object's message body to the given string.
 *  @see creq_Request::message_body
 * @param msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores a copy of the given string.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body(creq_Request_t *req, char *msg);

/**
 * @brief Set the creq_Request object's message body to the given literal.
 *  @see creq_Request::message_body
 * @param msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores the pointer to the literal directly. Behaviour will be undefined if non-literals are given.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body_literal(creq_Request_t *req, const char *msg);

/**
 * @brief Set the creq_Request object's message body to the given string and update the Content-Length header.
 *  @see creq_Request_set_message_body
 * @param msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body_content_len(creq_Request_t *req, char *msg);

/**
 * @brief Set the creq_Request object's message body to the given literal and update the Content-Length header.
 *  @see creq_Request_set_message_body_literal
 * @param msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body_literal_content_len(creq_Request_t *req, const char *msg);

/**
 * @brief Get the creq_Request object's message body.
 * @return The pointer to the message body string.
 *  @retval NULL Message body not set or bad argument given.
 * @attention The returned pointer points to the internal object. DO NOT MODIFY IT.
 */
CREQ_PUBLIC(char *) creq_Request_get_message_body(creq_Request_t *req);

/**
 * @brief Create the full request text using the given creq_Request object.
 * @return A pointer to the newly created request string.
 *  @retval NULL Some of the fields unset or invalid.
 * @attention This procedure will return a NEWLY MALLOC'ED string. Creq will not store it. It's the caller's responsibility to deal with it and free it.
 */
CREQ_PUBLIC(char *) creq_Request_stringify(creq_Request_t *req);

/**
 * @brief Creates a new creq_Response object.
 *  @see creq_Response
 * @return A pointer to the newly created creq_Response object.
 *  @retval NULL Fails to create a new object.
 * @attention Always use creq_Response_free when done.
 *  @see creq_Response_free
 */
CREQ_PUBLIC(creq_Response_t *) creq_Response_create(creq_Config_t *conf);

/**
 * @brief Frees a previously-created creq_Response object.
 *  @see creq_Response
 *  @see creq_Response_create
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure frees all the resources used in the given object, including all the items in the headers list. So make sure they are malloc'ed!
 */
CREQ_PUBLIC(creq_status_t) creq_Response_free(creq_Response_t *resp);

/**
 * @brief Set the creq_Response object's http version.
 *  @see creq_Response::http_version
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_http_version(creq_Response_t *resp, int major, int minor);

/**
 * @brief Get the creq_Response object's http version.
 * @retval The object which contains the version.
 */
CREQ_PUBLIC(creq_HttpVersion_t) creq_Response_get_http_version(creq_Response_t *resp);

/**
 * @brief Set the creq_Response object's status code field to a given value.
 *  @see creq_Response::status_code
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_status_code(creq_Response_t *resp, int status);

/**
 * @brief Set the creq_Response object's reason phrase to a given value.
 *  @see creq_Response::reason_phrase
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores a copy of the given string.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_reason_phrase(creq_Response_t *resp, char *reason);

/**
 * @brief Set the creq_Response object's reason phrase to a given literal.
 *  @see creq_Response::reason_phrase
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores the pointer to the literal directly. Behaviour will be undefined if non-literals are given.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_reason_phrase_literal(creq_Response_t *resp, const char *reason_s);

/**
 * @brief Get the creq_Response object's status code.
 * @return The previously set status code or 0.
 *  @retval 0 The status code has not been set yet.
 */
CREQ_PUBLIC(int) creq_Response_get_status_code(creq_Response_t *resp);

/**
 * @brief Get the creq_Response object's reason phrase.
 * @return A pointer to the reason phrase.
 *  @retval NULL Reason phrase not set or bad argument given.
 */
CREQ_PUBLIC(char *) creq_Response_get_reason_phrase(creq_Response_t *resp);

/**
 * @brief Adds a new item to the tail of the headers list of the creq_Response object.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_add_header(creq_Response_t *resp, char *header, char *value);

/**
 * @brief Adds a new item with literal header and value to the tail of the headers list of the creq_Response object.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This function works with literal header and values. If non-literals are given, the result is undefined.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_add_header_literal(creq_Response_t *resp, const char *header_s, const char *value_s);

/**
 * @brief Searches for a header-value pair in the headers list of the creq_Response object which contains the given header.
 * @return A pointer to the header found.
 *  @retval NULL Header not found.
 * @attention Always return the first one when there are multiple occurrences. To search for multiple header-value pairs, call this procedure iteratively with the last-found header as the head until NULL is returned.
 */
CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_Response_search_for_header(creq_Response_t *resp, char *header);

/**
 * @brief Moves the header with the given header name out of the creq_Response object's headers list and frees it.
 *  @see creq_HeaderLListNode_delist_header
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention Always delete the first header found in the list.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_remove_header(creq_Response_t *resp, char *header);

/**
 * @brief Moves a header node out of the list of the give creq_Response object and frees it.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure directly operates on the given node pointer. Often used with creq_Response_search_for_header .
 * @see creq_Response_search_for_header
 * @attention Behaviour will be undefined if the given header is not in the list.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_remove_header_direct(creq_Response_t *resp, creq_HeaderLListNode_t *node);

/**
 * @brief Set the creq_Response object's message body to the given string.
 *  @see creq_Response::message_body
 * @param msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores a copy of the given string.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_message_body(creq_Response_t *resp, char *msg);

/**
 * @brief Set the creq_Response object's message body to the given literal.
 *  @see creq_Response::message_body
 * @param msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores the pointer to the literal directly. Behaviour will be undefined if non-literals are given.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_message_body_literal(creq_Response_t *resp, const char *msg_s);

/**
 * @brief Set the creq_Response object's message body to the given string and update the Content-Length header.
 *  @see creq_Response_set_message_body
 * @param msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_message_body_content_len(creq_Response_t *resp, char *msg);

/**
 * @brief Set the creq_Response object's message body to the given literal and update the Content-Length header.
 *  @see creq_Response_set_message_body_literal
 * @param msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_message_body_literal_content_len(creq_Response_t *resp, const char *msg);

/**
 * @brief Get the creq_Response object's message body.
 * @return The pointer to the message body string.
 *  @retval NULL Message body not set or bad argument given.
 * @attention The returned pointer points to the internal object. DO NOT MODIFY IT.
 */
CREQ_PUBLIC(char *) creq_Response_get_message_body(creq_Response_t *resp);

/**
 * @brief Create the full request text using the given creq_Response object.
 * @return A pointer to the newly created request string.
 *  @retval NULL Some of the fields unset or invalid.
 * @attention This procedure will return a NEWLY MALLOC'ED string. Creq will not store it. It's the caller's responsibility to deal with it and free it.
 */
CREQ_PUBLIC(char *) creq_Response_stringify(creq_Response_t *resp);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // CREQ_H_INCLUDED
