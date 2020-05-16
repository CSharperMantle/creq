/**
 * @file creq.h
 * @brief Main header for creq project.
 * @author CSharperMantle
 */

#ifndef CREQ_H_INCLUDED
#define CREQ_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
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
#define CREQ_PUBLIC(type) type CREQ_STDCALL
#elif defined(CREQ_EXPORT_SYMBOLS)
#define CREQ_PUBLIC(type) __declspec(dllexport) type CREQ_STDCALL
#elif defined(CREQ_IMPORT_SYMBOLS)
#define CREQ_PUBLIC(type) __declspec(dllimport) type CREQ_STDCALL
#endif
#define CREQ_PRIVATE(type) static type
#else /* !__WINDOWS__ */
#define CREQ_CDECL
#define CREQ_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(CREQ_API_VISIBILITY)
#define CREQ_PUBLIC(type) __attribute__((visibility("default"))) type
#define CREQ_PRIVATE(type) __attribute__((visibility("hidden"))) type
#else
#define CREQ_PUBLIC(type) type
#define CREQ_PRIVATE(type) static type
#endif
#endif

/**
 * @brief Type for return values used when a function needs an error-indicating returning.
 * @see CREQ_STATUS_SUCC
 * @see CREQ_STATUS_FAILED
 */
typedef int creq_status_t;

/**
 * @brief Value stands for a successful execution of a function.
 */
#define CREQ_STATUS_SUCC 0

/**
 * @brief Value stands for an unsuccessful execution of a function.
 * @note When a function return this value, check the calling function's docs for detailed error handling.
 */
#define CREQ_STATUS_FAILED 1

/**
 * @brief Function-like marco for easy access to freeing a pointer with NULL-checks.
 * @attention This marco is intended for internal-uses only. Use this marco iff the pointer is malloced.
 */
#define CREQ_GUARDED_FREE(ptr)                                                                                         \
if (ptr != NULL)                                                                                                   \
{                                                                                                                  \
    free(ptr);                                                                                                     \
    ptr = NULL;                                                                                                    \
}

#define CVECTOR_LOGARITHMIC_GROWTH

#include "cvector.h"
#include <stdbool.h>
#include <wchar.h>

/**
 * @brief Line ending styles used in request/response generation.
 * @see creq_Config_t
 */
typedef enum creq_LineEnding_e
{
    LE_CR,
    LE_LF,
    LE_CRLF
} creq_LineEnding_t;

/**
 * @brief Pre-defined HTTP methods.
 * @note These values only cover methods defined in RFC7321 Section 4. Users can add more methods as they wish.
 * @see RFC7321 Section 4
 */
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

/**
 * @brief Config types used in object creation.
 * @see creq_Config_t
 * @see creq_Request_t
 * @see creq_Response_t
 */
typedef enum creq_ConfigType_e
{
    CONF_REQUEST,
    CONF_RESPONSE
} creq_ConfigType_t;

/**
 * @brief Represents a single header-value pair used in request and response.
 * @attention Manually editing these fields is not encouraged. Poorly-set values may cause use-after-free situation and/or crashes.
 */
typedef struct creq_HeaderField
{
    char *field_name;
    bool is_field_name_literal;
    char *field_value;
    bool is_field_value_literal;
} creq_HeaderField_t;

/**
 * @brief Configuration used in both request and response.
 */
typedef struct creq_Config
{
    union
    {
        struct
        {
            creq_LineEnding_t line_ending;

        } request_config;
        struct
        {
            creq_LineEnding_t line_ending;

        } response_config;
    } data;
    creq_ConfigType_t config_type;
} creq_Config_t;

/**
 * @brief HTTP version struct.
 */
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
    cvector_VECTOR(creq_HeaderField_t *) header_vector;
    // line ending after each header
    // line ending again in the end

    char *message_body;
    bool is_message_body_literal;

    /// @todo for future verification apis, not used for now
    bool is_verified;
} creq_Request_t;

/**
 * @brief Inner response struct for response generating.
 * @see RFC7230 Section 3
 */
typedef struct creq_Response
{
    creq_Config_t config;

    // > status-line, Section 3.1.2
    creq_HttpVersion_t http_version;
    // space
    int status_code;
    // space
    char *reason_phrase;
    bool is_reason_phrase_literal;
    // line ending

    // > header field
    cvector_VECTOR(creq_HeaderField_t *) header_vector;
    // line ending after each header
    // line ending again in the end

    char *message_body;
    bool is_message_body_literal;

    /// @todo for future verification apis, not used for now
    bool is_verified;
} creq_Response_t;

/**
 * @brief Creates a new header-value pair.
 * @return A pointer to the newly created node.
 *  @retval NULL Fails to create a new object.
 * @attention For internal use only.
 * @see creq_HeaderField_t
 */
CREQ_PUBLIC(creq_HeaderField_t *) creq_HeaderField_create(char *header, char *value);

/**
 * @brief Creates a new header-value pair with literal header and value.
 * @return A pointer to the newly created node.
 *  @retval NULL Fails to create a new object.
 * @attention For internal use only.
 * @attention Behaviour will be undefined if non-literals are given.
 * @see creq_HeaderField_t
 */
CREQ_PUBLIC(creq_HeaderField_t *) creq_HeaderField_create_literal(const char *header_s, const char *value_s);

/**
 * @brief Frees a previously-created creq_HeaderField.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure only frees the object, the data field and the strings.
 * @attention For internal use only.
 * @see creq_HeaderField_t
 */
CREQ_PUBLIC(creq_status_t) creq_HeaderField_free(creq_HeaderField_t **ptrToFieldPtr);

/**
 * @brief Creates a new creq_Request object.
 * @return A pointer to the newly created creq_Request object.
 *  @retval NULL Fails to create a new object.
 * @attention Always use creq_Request_free when done.
 * @see creq_Request_free
 * @see creq_Request_t
 */
CREQ_PUBLIC(creq_Request_t *) creq_Request_create(creq_Config_t *conf);

/**
 * @brief Frees a previously-created creq_Request object.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure frees all the resources used in the given object, including all the items in the headers list.
 * @attention So make sure they are malloc'ed!
 * @see creq_Request_t
 * @see creq_Request_create
 */
CREQ_PUBLIC(creq_status_t) creq_Request_free(creq_Request_t *req);

/**
 * @brief Set the creq_Request object's http method.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @see creq_Request_t::method
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_http_method(creq_Request_t *req, creq_HttpMethod_t method);

/**
 * @brief Get the creq_Request object's http method.
 * @retval The object which contains the method.
 */
CREQ_PUBLIC(creq_HttpMethod_t) creq_Request_get_http_method(creq_Request_t *req);

/**
 * @brief Set the creq_Request object's target to the given string.
 * @param[in] requestTarget The pointer to the new target. NULL will clear the target.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores a copy of the given string.
 * @see creq_Request_t::request_target
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_target(creq_Request_t *req, char *requestTarget);

/**
 * @brief Set the creq_Request object's target to the given literal.
 * @param[in] requestTarget_s The pointer to the new target. NULL will clear the target.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores the pointer to the literal directly. Behaviour will be undefined if non-literals are given.
 * @see creq_Request_t::request_target
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
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @see creq_Request_t::http_version
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
 * @attention Always return the first one when there are multiple occurrences.
 */
CREQ_PUBLIC(creq_HeaderField_t *) creq_Request_search_for_header(creq_Request_t *req, char *header);

/**
 * @brief Get a header-value pair's index in the list.
 * @return The index.
 *  @retval -1 Header not found.
 * @attention Always return the first one when there are multiple occurrences.
 */
CREQ_PUBLIC(int) creq_Request_search_for_header_index(creq_Request_t *req, char *header);

/**
 * @brief Moves the header with the given header name out of the creq_Request object's headers list and frees it.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention Always delete the first header found in the list.
 * @see creq_HeaderField_free
 */
CREQ_PUBLIC(creq_status_t) creq_Request_remove_header(creq_Request_t *req, char *header);

/**
 * @brief Moves a header node out of the list of the give creq_Request object and frees it.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure directly operates on the given pointer. Often used with creq_Request_search_for_header.
 * @see creq_Request_search_for_header
 */
CREQ_PUBLIC(creq_status_t) creq_Request_remove_header_direct(creq_Request_t *req, creq_HeaderField_t *node);

/**
 * @brief Set the creq_Request object's message body to the given string.
 * @param[in] msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores a copy of the given string.
 * @see creq_Request_t::message_body
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body(creq_Request_t *req, char *msg);

/**
 * @brief Set the creq_Request object's message body to the given literal.
 * @param[in] msg_s The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores the pointer to the literal directly. Behaviour will be undefined if non-literals are given.
 * @see creq_Request_t::message_body
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body_literal(creq_Request_t *req, const char *msg_s);

/**
 * @brief Set the creq_Request object's message body to the given string and update the Content-Length header.
 * @param[in] msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @see creq_Request_set_message_body
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body_content_len(creq_Request_t *req, char *msg);

/**
 * @brief Set the creq_Request object's message body to the given literal and update the Content-Length header.
 * @param[in] msg_s The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @see creq_Request_set_message_body_literal
 */
CREQ_PUBLIC(creq_status_t) creq_Request_set_message_body_literal_content_len(creq_Request_t *req, const char *msg_s);

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
 * @return A pointer to the newly created creq_Response object.
 *  @retval NULL Fails to create a new object.
 * @attention Always use creq_Response_free when done.
 * @see creq_Response_t
 * @see creq_Response_free
 */
CREQ_PUBLIC(creq_Response_t *) creq_Response_create(creq_Config_t *conf);

/**
 * @brief Frees a previously-created creq_Response object.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure frees all the resources used in the given object, including all the items in the headers list. So make sure they are malloc'ed!
 * @see creq_Response_t
 * @see creq_Response_create
 */
CREQ_PUBLIC(creq_status_t) creq_Response_free(creq_Response_t *resp);

/**
 * @brief Set the creq_Response object's http version.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @see creq_Response_t::http_version
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_http_version(creq_Response_t *resp, int major, int minor);

/**
 * @brief Get the creq_Response object's http version.
 * @retval The object which contains the version.
 */
CREQ_PUBLIC(creq_HttpVersion_t) creq_Response_get_http_version(creq_Response_t *resp);

/**
 * @brief Set the creq_Response object's status code field to a given value.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @see creq_Response_t::status_code
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_status_code(creq_Response_t *resp, int status);

/**
 * @brief Set the creq_Response object's reason phrase to a given value.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores a copy of the given string.
 * @see creq_Response_t::reason_phrase
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_reason_phrase(creq_Response_t *resp, char *reason);

/**
 * @brief Set the creq_Response object's reason phrase to a given literal.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores the pointer to the literal directly. Behaviour will be undefined if non-literals are given.
 * @see creq_Response_t::reason_phrase
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
 * @attention Always return the first one when there are multiple occurrences.
 */
CREQ_PUBLIC(creq_HeaderField_t *) creq_Response_search_for_header(creq_Response_t *resp, char *header);

/**
 * @brief Get a header-value pair's index in the list.
 * @return The index.
 *  @retval -1 Header not found.
 * @attention Always return the first one when there are multiple occurrences.
 */
CREQ_PUBLIC(int) creq_Response_search_for_header_index(creq_Response_t *resp, char *header);

/**
 * @brief Moves the header with the given header name out of the creq_Response object's headers list and frees it.
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
 */
CREQ_PUBLIC(creq_status_t) creq_Response_remove_header_direct(creq_Response_t *resp, creq_HeaderField_t *node);

/**
 * @brief Set the creq_Response object's message body to the given string.
 * @param[in] msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores a copy of the given string.
 * @see creq_Response_t::message_body
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_message_body(creq_Response_t *resp, char *msg);

/**
 * @brief Set the creq_Response object's message body to the given literal.
 * @param[in] msg_s The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @attention This procedure stores the pointer to the literal directly. Behaviour will be undefined if non-literals are given.
 * @see creq_Response_t::message_body
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_message_body_literal(creq_Response_t *resp, const char *msg_s);

/**
 * @brief Set the creq_Response object's message body to the given string and update the Content-Length header.
 * @param[in] msg The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @see creq_Response_set_message_body
 */
CREQ_PUBLIC(creq_status_t) creq_Response_set_message_body_content_len(creq_Response_t *resp, char *msg);

/**
 * @brief Set the creq_Response object's message body to the given literal and update the Content-Length header.
 * @param[in] msg_s The pointer to the new message. NULL will clear the message.
 * @return Indicates if the procedure is finished properly.
 *  @retval CREQ_STATUS_SUCC Procedure finishes successfully.
 *  @retval CREQ_STATUS_FAILED Procedure fails.
 * @see creq_Response_set_message_body_literal
 */
CREQ_PUBLIC(creq_status_t)
creq_Response_set_message_body_literal_content_len(creq_Response_t *resp, const char *msg_s);

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
