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

typedef enum creq_LineEnding_e
{
    LE_CR,
    LE_LF,
    LE_CRLF
} creq_LineEnding_t;

typedef enum creq_HttpMethod_e
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    _UNKNOWN
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
    char *field_value;
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
    // space
    creq_HttpVersion_t http_version;
    // space
    // line ending

    // > header field
    creq_HeaderLListNode_t *list_head;
    // line ending after each header
    // line ending again in the end

    char *message_body;
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
    // line ending

    // > header field
    creq_HeaderLListNode_t *list_head;
    // line ending after each header
    // line ending again in the end

    char *message_body;
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
CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_create(const char *header, const char *value);

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
 * @attention Always return the first one when there are multiple occurrences.
 */
CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_search_for_header(creq_HeaderLListNode_t *head, const char *header);

/**
 * @brief Adds a new item to the tail of the given list.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_HeaderLListNode_add_header(creq_HeaderLListNode_t **head, const char *header, const char *value);

/**
 * @brief Moves a header node out of the list.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure directly operates on the given node pointer.
 */
CREQ_PUBLIC(creq_status_t) creq_HeaderLListNode_delist_header_direct(creq_HeaderLListNode_t **head, creq_HeaderLListNode_t *node);

/**
 * @brief Moves a header from the headers list which has the given header out of the list. Alternative to creq_HeaderLListNode_delist_header_direct.
 *  @see creq_HeaderLListNode_delist_header_direct
 * @return A pointer to the removed header for further process.
 *  @retval NULL Header not found.
 * @attention Always delete the first header found in the list. To delete multiple header-value pairs, call this procedure iteratively until NULL is returned.
 */
CREQ_PUBLIC(creq_HeaderLListNode_t *) creq_HeaderLListNode_delist_header(creq_HeaderLListNode_t **head, const char *header);

/**
 * @brief Creates a new creq_Request object.
 *  @see creq_Request
 * @return A pointer to the newly created creq_Request object.
 *  @retval NULL Fails to create a new object.
 * @attention Always use creq_Request_free when done.
 *  @see creq_Request_free
 */
CREQ_PUBLIC(creq_Request_t *) creq_Request_create();

/**
 * @brief Frees a previously-created creq_Request object.
 *  @see creq_Request
 *  @see creq_create_request
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
CREQ_PUBLIC(creq_status_t) creq_Request_set_target(creq_Request_t *req, const char *requestTarget);

/**
 * @brief Get the creq_Request object's target
 * @return A pointer to the target string.
 *  @retval NULL No target specified or invalid argument given.
 * @attention The returned pointer points to the internal object. DO NOT MODIFY IT.
 */
CREQ_PUBLIC(char *) creq_Request_get_target(creq_Request_t *req);

/**
 * @brief Set the http version of the creq_Request object.
 *  @see creq_Request::http_version
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_http_version(creq_Request_t *req, int major, int minor);

/**
 * @brief Get the http version of the creq_Request object.
 * @retval The object which contains the version.
 */
CREQ_PUBLIC(creq_HttpVersion_t) creq_Request_get_http_version(creq_Request_t *req);

/**
 * @brief Set the creq_Request object's message body to the given string.
 *  @see creq_Request::message_body
 * @param requestTarget The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores a copy of the given string.
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body(creq_Request_t *req, const char *msg);

/**
 * @brief Get the creq_Request object's message body.
 * @return The pointer to the message body string.
 *  @retval NULL Message body not defined or bad argument given.
 * @attention The returned pointer points to the internal object. DO NOT MODIFY IT.
 */
CREQ_PUBLIC(char *) creq_Request_get_message_body(creq_Request_t *req);

/**
 * @brief Create the full request text using the given creq_Request object.
 * @return A pointer to the newly created request string.
 *  @retval NULL Some fields undefined or invalid.
 * @attention This procedure will return a NEWLY MALLOC'ED string. Creq will not store it. It's the caller's responsibility to deal with it and free it.
 */
CREQ_PUBLIC(char *) creq_Request_stringify(creq_Request_t *req);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // CREQ_H_INCLUDED
