#ifndef __LWHTTP_H_
#define __LWHTTP_H_

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * LwHTTP type identifier. Basic types are:
 * 	o Initial line (request)
 * 	o Header
 * 	o Body
 */
typedef enum {
	LwHHTP_UNDEFINED = 0,
	LwHHTP_INITIAL = 1,
	LwHHTP_HEADER = 2,
	LwHHTP_BODY = 3,
} lwhttp_type_t;

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
	LwHHTP_BUILDER_ADDED_END = 4,
	LwHHTP_BUILDER_ERROR = 5,
} lwhttp_builder_state_t;

/**
 * LwHTTP token description.
 * type		type (initial, header, body)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
	lwhttp_type_t type;
	uint16_t start;
	uint16_t end;
	uint16_t size;
} lwhttp_tok_t;

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
 * LwHTTP Parse function.
 * src          Source for HTTP response to parse
 * src_len      Length of HTTP response to parse
 * tok          Pointer to tokens where data will be stored
 * tok_len      Number of available tokens to populate
 * return       Number of parsed tokens
 */
uint16_t lwhttp_parse(char* src, uint16_t src_len, lwhttp_tok_t* tok, uint16_t tok_len);

uint16_t lwhttp_new_builder(lwhttp_builder_t* builder, char* dest, uint16_t size);

uint16_t lwhttp_add_request(lwhttp_builder_t* builder, lwhttp_method_t method, const char* url);

uint16_t lwhttp_add_header(lwhttp_builder_t* builder, const char* header, const char* value);

uint16_t lwhttp_add_body(lwhttp_builder_t* builder, const char* body, uint16_t len);

uint16_t lwhttp_end(lwhttp_builder_t* builder);



#ifdef __cplusplus
}
#endif

#endif /* __LWHTTP_H_ */
