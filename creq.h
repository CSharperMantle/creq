#ifndef CREQ_H_INCLUDED
#define CREQ_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

// BEGINNING OF COPIED CODE these code is copied from cJSON by DaveGamble.
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
#else /* !__WINDOWS__ */
#define CREQ_CDECL
#define CREQ_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(CREQ_API_VISIBILITY)
#define CREQ_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define CREQ_PUBLIC(type) type
#endif
#endif
// END OF COPIED CODE

#include <stdio.h>

enum
{
    CR,
    LF,
    CRLF
} creq_LineEnding_e;

enum
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
} creq_HttpMethod_e; //TODO/**<  */

typedef struct creq_Config
{
    union {
        struct {
            int line_ending;

        } RequestConfig;
        struct {
            int line_ending;

        } ResponseConfig;
    } data;
    enum { REQUEST, RESPONSE } config_type;
} creq_Config_t;



typedef struct creq_HeaderField
{
    char *field_name;
    char *field_value;
} creq_HeaderField_t;

// As defined in RFC7230 Section 3
typedef struct creq_Request
{
    creq_Config_t config;

    // > request-line, Section 3.1.1
    int method;
    // space
    char *request_target;
    // space
    // > mask value, for example (0101)10 for HTTP/1.1
    int http_version;
    // space
    // line ending

    // > header field
    struct {
        creq_HeaderField_t *first_header;
        int len;
    } HeaderFields; //TODO: better algorithm
    // line ending after each header
    // line ending again in the end

    char *message_body;
} creq_Request_t;

typedef struct creq_Response
{
    creq_Config_t config;

    // > status-line, Section 3.1.2
    // > mask value, for example (0101)10 for HTTP/1.1
    int http_version;
    // space
    int status_code;
    // space
    char *reason_phrase;
    // line ending

    // > header field
    struct {
        creq_HeaderField_t *first_header;
        int len;
    } HeaderFields; //TODO: better algorithm
    // line ending after each header
    // line ending again in the end

    char *message_body;
} creq_Response_t;


#ifdef __cplusplus
}
#endif // __cplusplus
#endif // CREQ_H_INCLUDED
