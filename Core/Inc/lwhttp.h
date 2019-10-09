#ifndef __LWHTTP_H_
#define __LWHTTP_H_

#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <portmacro.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DBG_LWHTTP_ENABLED   1


#define errOK          ( pdPASS )            /** No error, everything OK. */
#define errNOK         ( pdFAIL )            /** Unknown error.           */
#define errMEM         ( ( BaseType_t ) -1  ) /** Out of memory error.     */
#define errBUF         ( ( BaseType_t ) -2  ) /** Buffer error.            */
#define errTimeout     ( ( BaseType_t ) -3  ) /** Timeout.                 */
#define errNotFound    ( ( BaseType_t ) -4  ) /** Routing problem.         */
#define errARG         ( ( BaseType_t ) -5  ) /** Illegal argument.        */
#define errParser      ( ( BaseType_t ) -6  ) /** Failed to parse.         */
#define errParseReady  ( ( BaseType_t ) -7  ) /** Failed to parse.         */
#define errParseStart  ( ( BaseType_t ) -8  ) /** Failed to parse.         */
#define errParseHeader ( ( BaseType_t ) -9  ) /** Failed to parse.         */
#define errParseBody   ( ( BaseType_t ) -10 ) /** Failed to parse.         */
#define errBuild       ( ( BaseType_t ) -11 ) /** Failed to build.         */
#define errBuildReady  ( ( BaseType_t ) -12 ) /** Failed to build.         */
#define errBuildStart  ( ( BaseType_t ) -13 ) /** Failed to build.         */
#define errBuildHeader ( ( BaseType_t ) -14 ) /** Failed to build.         */
#define errBuildBody   ( ( BaseType_t ) -15 ) /** Failed to build.         */

typedef enum {
    LwHHTP_MESSAGE_TYPE_INVALID = 0,
    LwHHTP_MESSAGE_TYPE_REQUEST = 1,
    LwHHTP_MESSAGE_TYPE_RESPONSE = 2,
    LwHHTP_MESSAGE_TYPE_MAX,
} lwhttp_message_type_t;

typedef enum {
    LwHHTP_POST = 0,
    LwHHTP_GET = 1,
    LwHHTP_MAX,
} lwhttp_method_t;

typedef enum {
    LwHHTP_BUILDER_INIT = 0,
    LwHHTP_BUILDER_IN_PROGRESS = 1,
    LwHHTP_BUILDER_ADDED_START_LINE = 2,
    LwHHTP_BUILDER_ADDED_MESSAGE_HEADER = 3,
    LwHHTP_BUILDER_ADDED_MESSAGE_BODY = 4,
    LwHHTP_BUILDER_FINISHED = LwHHTP_BUILDER_ADDED_MESSAGE_BODY,
    LwHHTP_BUILDER_ERROR = 6,
} lwhttp_message_builder_state_t;

typedef enum {
    LwHHTP_PARSER_INIT = 0,
    LwHHTP_PARSER_READY = 1,
    LwHHTP_PARSER_START_LINE = 2,
    LwHHTP_PARSER_MESSAGE_HEADER = 3,
    LwHHTP_PARSER_MESSAGE_BODY = 4,
    LwHHTP_PARSER_END = 5,
    LwHHTP_PARSER_ERROR = 6,
} lwhttp_message_parser_state_t;

typedef struct {
    char* data;
    uint16_t len;
} lwhttp_token_t;

typedef struct {
    lwhttp_token_t buffer;        /* Buffer */
    lwhttp_token_t field_name;
    lwhttp_token_t field_value;
} lwhttp_message_header_t;

typedef struct {
    lwhttp_message_header_t* headers;
    uint16_t count;
} lwhttp_message_headers_t;

typedef struct {
    lwhttp_token_t buffer;        /* Buffer */
    lwhttp_token_t method;        /* HTTP/1.1 Sec5.1.1 Request - Method */
    lwhttp_token_t request_uri;   /* HTTP/1.1 Sec5.1.2 Request - Request-URI */
    lwhttp_token_t http_version;  /* HTTP/1.1 Sec3.1 Protocol parameters - HTTP Version */
} lwhttp_request_line_t;

typedef struct {
    lwhttp_token_t buffer;        /* Buffer */
    lwhttp_token_t http_version;  /* HTTP/1.1 Sec3.1 Protocol parameters - HTTP Version */
    lwhttp_token_t status_code;   /* HTTP/1.1 Sec6.1.1 Response - Status Code and Reason Phrase */
    lwhttp_token_t reason_phrase; /* HTTP/1.1 Sec6.1.1 Response - Status Code and Reason Phrase */
} lwhttp_status_line_t;

typedef union {
    lwhttp_request_line_t request_line; /* HTTP/1.1 Sec5.1 Request - Request line */
    lwhttp_status_line_t status_line;   /* HTTP/1.1 Sec6.1 Response - Status line */
} lwhttp_start_line_t;

typedef struct {
    lwhttp_message_parser_state_t parser_state;   /* Parser internals */
    lwhttp_message_builder_state_t builder_state; /* Builder internals */
    lwhttp_token_t buffer;                        /* Buffer */
    lwhttp_message_type_t message_type;           /* HTTP/1.1 Sec4.1 Message Types */
    lwhttp_start_line_t start_line;               /* HTTP/1.1 Sec4.1 Message Types - Start-Line */
    lwhttp_message_headers_t message_headers;     /* HTTP/1.1 Sec4.2 HTTP Message - Message-Headers */
    lwhttp_token_t message_body;                  /* HTTP/1.1 Sec4.3 HTTP Message - Message-Body */
} lwhttp_message_t;

typedef lwhttp_message_t lwhttp_response_t;
typedef lwhttp_message_t lwhttp_request_t;

/* LwHTTP Response Specific functions */
BaseType_t lwhttp_response_init(lwhttp_response_t* response);
BaseType_t lwhttp_response_free(lwhttp_response_t* response);
BaseType_t lwhttp_response_parse(lwhttp_response_t* response);
BaseType_t lwhttp_response_get(lwhttp_response_t* response, char** dest, uint16_t* len);
BaseType_t lwhttp_response_put(lwhttp_response_t* response, const char* src, uint16_t len);
BaseType_t lwhttp_response_put_eol(lwhttp_response_t* response);
BaseType_t lwhttp_response_get_status_line(lwhttp_response_t* response, char** dest, uint16_t* len);
BaseType_t lwhttp_response_put_status_line(lwhttp_response_t* response, const char* status_code, const char* reason_phrase);
BaseType_t lwhttp_response_get_message_header(lwhttp_response_t* response, const char* field_name, lwhttp_message_header_t** dest);
BaseType_t lwhttp_response_put_message_header(lwhttp_response_t* response, const char* header, const char* value);
BaseType_t lwhttp_response_get_message_body(lwhttp_response_t* response, char** dest, uint16_t* len);
BaseType_t lwhttp_response_put_message_body(lwhttp_response_t* response, const char* body, uint16_t len);
BaseType_t lwhttp_response_get_status_code(lwhttp_response_t* response, char** dest, uint16_t* len);

/* LwHTTP Request Specific functions */
BaseType_t lwhttp_request_init(lwhttp_request_t* request);
BaseType_t lwhttp_request_free(lwhttp_request_t* request);
BaseType_t lwhttp_request_parse(lwhttp_request_t* request);
BaseType_t lwhttp_request_get(lwhttp_request_t* request, char** dest, uint16_t* len);
BaseType_t lwhttp_request_put(lwhttp_request_t* request, const char* src, uint16_t len);
BaseType_t lwhttp_request_put_eol(lwhttp_request_t* request);
BaseType_t lwhttp_request_get_request_line(lwhttp_request_t* request, char** dest, uint16_t* len);
BaseType_t lwhttp_request_put_request_line(lwhttp_request_t* request, lwhttp_method_t method, const char* request_uri);
BaseType_t lwhttp_request_get_message_header(lwhttp_request_t* request, const char* field_name, lwhttp_message_header_t** dest);
BaseType_t lwhttp_request_put_message_header(lwhttp_request_t* request, const char* header, const char* value);
BaseType_t lwhttp_request_get_message_body(lwhttp_request_t* request, char** dest, uint16_t* len);
BaseType_t lwhttp_request_put_message_body(lwhttp_request_t* request, const char* body, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __LWHTTP_H_ */
