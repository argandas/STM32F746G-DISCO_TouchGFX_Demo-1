#ifndef __LWHTTP_H_
#define __LWHTTP_H_

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LwHHTP_POST = 0,
	LwHHTP_GET = 1,
	LwHHTP_MAX,
} lwhttp_method_t;

typedef enum {
	LwHHTP_BUILDER_INIT = 0,
	LwHHTP_BUILDER_ADDED_REQUEST = 1,
	LwHHTP_BUILDER_ADDED_HEADER = 2,
	LwHHTP_BUILDER_ADDED_BODY = 3,
	LwHHTP_BUILDER_ERROR,
} lwhttp_builder_state_t;

typedef enum {
	LwHHTP_RESPONSE_PARSER_INIT = 0,
	LwHHTP_RESPONSE_PARSER_READY = 1,
	LwHHTP_RESPONSE_PARSER_STATUS_LINE = 2,
	LwHHTP_RESPONSE_PARSER_MESSAGE_HEADER = 3,
	LwHHTP_RESPONSE_PARSER_MESSAGE_BODY = 4,
	LwHHTP_RESPONSE_PARSER_END = 5,
	LwHHTP_RESPONSE_PARSER_ERROR = 6,
} lwhttp_parser_state_t;

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

typedef struct {
	lwhttp_token_t buffer; /* Buffer */
        lwhttp_token_t field1;
        lwhttp_token_t field2;
        lwhttp_token_t field3;
} lwhttp_start_line_t;

typedef struct {
        lwhttp_parser_state_t state;             /* Parser internals */
	lwhttp_token_t buffer;                   /* Buffer */
        lwhttp_status_line_t status_line;        /* HTTP/1.1 Sec6.1 Response - Status line */
        lwhttp_message_headers_t message_headers; /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
        lwhttp_token_t message_body;             /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
} lwhttp_response_t;

typedef struct {
        lwhttp_builder_state_t state;            /* Parser internals */
	lwhttp_token_t buffer;                   /* Buffer */
        lwhttp_request_line_t request_line;      /* HTTP/1.1 Sec5.1 Request - Request line */
        lwhttp_message_headers_t message_headers; /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
        lwhttp_token_t message_body;             /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
} lwhttp_request_t;

/* LwHTTP Response Parse functions */
uint16_t lwhttp_response_parser_init(lwhttp_response_t* response);
uint16_t lwhttp_response_parser_write(lwhttp_response_t* response, char* src, uint16_t len);
uint16_t lwhttp_response_parser_run(lwhttp_response_t* response);
uint16_t lwhttp_response_parser_free(lwhttp_response_t* response);

/* LwHTTP Response functions */
uint16_t lwhttp_response_get(lwhttp_response_t* response, char** dest, uint16_t* len);
uint16_t lwhttp_response_get_status_line(lwhttp_response_t* response, char** dest, uint16_t* len);
uint16_t lwhttp_response_get_status_code(lwhttp_response_t* response, char** dest, uint16_t* len);
uint16_t lwhttp_response_get_message_header_by_name(lwhttp_response_t* response, const char* field_name, lwhttp_message_header_t** dest);
uint16_t lwhttp_response_get_message_header_by_index(lwhttp_response_t* response, uint8_t index, lwhttp_message_header_t** dest);
uint16_t lwhttp_response_get_message_body(lwhttp_response_t* response, char** dest, uint16_t* len);

/* LwHTTP Request Parse functions */
uint16_t lwhttp_request_parser_init(lwhttp_request_t* request);
uint16_t lwhttp_request_parser_free(lwhttp_request_t* request);

/* LwHTTP Request functions */
uint16_t lwhttp_request_get(lwhttp_request_t* request, char** dest, uint16_t* len);
uint16_t lwhttp_request_put_request_line(lwhttp_request_t* request, lwhttp_method_t method, const char* request_uri);
uint16_t lwhttp_request_put_message_header(lwhttp_request_t* request, const char* header, const char* value);
uint16_t lwhttp_request_put_message_body(lwhttp_request_t* request, const char* body, uint16_t len);



#ifdef __cplusplus
}
#endif

#endif /* __LWHTTP_H_ */
