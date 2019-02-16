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
	LwHHTP_BUILDER_ADDED_HEADER = 1,
	LwHHTP_BUILDER_ADDED_BODY = 2,
	LwHHTP_BUILDER_ADDED_END = 3,
	LwHHTP_BUILDER_ERROR = 4,
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
 */
typedef struct {
	char* key;
	uint16_t key_len;
	char* value;
	uint16_t value_len;
} lwhttp_header_token_t;

/**
 * LwHTTP token description.
 * buf          Buffer to store builder data
 * size         Builder buffer size
 * len          Builder used buffer length
 */
typedef struct {
	char* data;
	char* data_ptr;
	uint16_t size;
	uint16_t len;
        lwhttp_builder_state_t state;
} lwhttp_builder_t;

/**
 * LwHTTP token description.
 * buf          Buffer to store builder data
 * size         Builder buffer size
 * len          Builder used buffer length
 */
typedef struct {
        lwhttp_parser_state_t state;

	char* buff;
	uint16_t buff_idx;
        uint16_t buff_size;
	
        char* initial;
	uint16_t initial_len;

        lwhttp_header_token_t* headers;
	uint16_t headers_len;
	uint16_t headers_size;

        char* body;
        uint16_t body_len;
} lwhttp_parser_t;

/**
 * LwHTTP Parse function.
 * src          Source for HTTP response to parse
 * src_len      Length of HTTP response to parse
 * tok          Pointer to tokens where data will be stored
 * tok_len      Number of available tokens to populate
 * return       Number of parsed tokens
 */
uint16_t lwhttp_parser_init(lwhttp_parser_t* parser, char* buff, uint16_t buff_size, lwhttp_header_token_t* header_tokens, uint16_t num_tokens);
uint16_t lwhttp_parser_write(lwhttp_parser_t* parser, char* src, uint16_t len);
uint16_t lwhttp_parser_run(lwhttp_parser_t* parser);
uint16_t lwhttp_parser_get_status(lwhttp_parser_t* parser);
uint16_t lwhttp_parser_get_header(lwhttp_parser_t* parser, char* key, lwhttp_header_token_t** header_token);
uint16_t lwhttp_parser_get_body(lwhttp_parser_t* parser, char** dest, uint16_t* len);

uint16_t lwhttp_builder_init(lwhttp_builder_t* builder, char* dest, uint16_t size, lwhttp_method_t method, const char* url);
uint16_t lwhttp_builder_add_header(lwhttp_builder_t* builder, const char* header, const char* value);
uint16_t lwhttp_builder_add_body(lwhttp_builder_t* builder, const char* body, uint16_t len);
uint16_t lwhttp_builder_end(lwhttp_builder_t* builder);
uint16_t lwhttp_builder_get_data(lwhttp_builder_t* builder, char** dest, uint16_t* len);



#ifdef __cplusplus
}
#endif

#endif /* __LWHTTP_H_ */
