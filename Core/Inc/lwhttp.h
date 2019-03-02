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
	LwHHTP_PARSER_INIT = 0,
	LwHHTP_PARSER_READY = 1,
	LwHHTP_PARSER_DO_INITIAL = 2,
	LwHHTP_PARSER_DO_HEADER = 3,
	LwHHTP_PARSER_DO_BODY = 4,
	LwHHTP_PARSER_END = 5,
	LwHHTP_PARSER_ERROR = 6,
} lwhttp_parser_state_t;

/**
 * LwHTTP token description.
 * type		type (initial, header, body)
 * start	start position in JSON data string
 * end		end position in JSON data string
 * note         HTTP/1.1 Sec4.2 HTTP Message - Message headers
 */
typedef struct {
	char* field_name;
	uint16_t field_name_len;
	char* field_value;
	uint16_t field_value_len;
} lwhttp_message_header_t;

typedef struct {
        /* Parser internals */
	char* data;
	uint16_t data_len;
        
        /* HTTP/1.1 Sec5.1.1 Request - Method */
        char* method;
	uint16_t method_len;

        /* HTTP/1.1 Sec5.1.2 Request - Request-URI */
        char* request_uri;
	uint16_t request_uri_len;

        /* HTTP/1.1 Sec3.1 Protocol parameters - HTTP Version */
        char* http_version;
        uint16_t http_version_len;
} lwhttp_request_line_t;

typedef struct {
        /* Parser internals */
	char* data;
	uint16_t data_len;
        
        /* HTTP/1.1 Sec3.1 Protocol parameters - HTTP Version */
        char* http_version;
        uint16_t http_version_len;
        
        /* HTTP/1.1 Sec6.1.1 Response - Status Code and Reason Phrase */
        char* status_code;
	uint16_t status_code_len;

        /* HTTP/1.1 Sec6.1.1 Response - Status Code and Reason Phrase */
        char* reason_phrase;
	uint16_t reason_phrase_len;
} lwhttp_status_line_t;

typedef struct {
        /* Parser internals */
        lwhttp_parser_state_t state;
	char* data;
	uint16_t data_len;
	
        /* HTTP/1.1 Sec6.1 Response - Status line */
        lwhttp_status_line_t status_line;

        /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
        lwhttp_message_header_t* message_headers;
	uint16_t message_headers_len;

        /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
        char* message_body;
        uint16_t message_body_len;
} lwhttp_response_t;

typedef struct {
        /* Parser internals */
        lwhttp_builder_state_t state;
	char* data;
	uint16_t data_len;
	
        /* HTTP/1.1 Sec5.1 Request - Request line */
        lwhttp_request_line_t request_line;

        /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
        lwhttp_message_header_t* message_headers;
	uint16_t message_headers_len;

        /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
        char* message_body;
        uint16_t message_body_len;
} lwhttp_request_t;

/**
 * LwHTTP Parse function.
 * src          Source for HTTP response to parse
 * src_len      Length of HTTP response to parse
 * tok          Pointer to tokens where data will be stored
 * tok_len      Number of available tokens to populate
 * return       Number of parsed tokens
 */

/* LwHTTP Response Parse functions */
uint16_t lwhttp_response_parser_init(lwhttp_response_t* response);
uint16_t lwhttp_response_parser_write(lwhttp_response_t* response, char* src, uint16_t len);
uint16_t lwhttp_response_parser_run(lwhttp_response_t* response);
uint16_t lwhttp_response_parser_free(lwhttp_response_t* response);

/* LwHTTP Response functions */
uint16_t lwhttp_response_get(lwhttp_response_t* response, char** dest, uint16_t* len);
uint16_t lwhttp_response_get_status_line(lwhttp_response_t* response, char** dest, uint16_t* len);
uint16_t lwhttp_response_get_status_code(lwhttp_response_t* response, uint16_t* status_code);
uint16_t lwhttp_response_get_message_header(lwhttp_response_t* response, char* field_name, lwhttp_message_header_t* header_ptr);
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
