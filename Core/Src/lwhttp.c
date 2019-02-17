#include "lwhttp.h"

char* LwHTTP_methods[LwHHTP_MAX] = {"POST", "GET"};

uint16_t lwhttp_parser_init(lwhttp_parser_t* parser, lwhttp_header_token_t* header_tokens, uint16_t num_tokens)
{
  uint16_t ret = 0;
  if (parser != NULL)
  {    
    parser->data = NULL;
    parser->data_len = 0;
    
    parser->initial = NULL;
    parser->initial_len = 0;

    /* Header tokens are optional and will be handled when parsing the payload */
    parser->headers = header_tokens;
    parser->headers_len = 0;
    parser->headers_size = num_tokens;

    parser->body = NULL;
    parser->body_len = 0;
        
    parser->state = LwHHTP_PARSER_INIT;
  }
  return ret;
}

uint16_t lwhttp_parser_write(lwhttp_parser_t* parser, char* src, uint16_t len)
{
  uint16_t ret = 0;
  uint16_t idx = 0;
  if ((parser != NULL) && (src != NULL) && (len > 0))
  {
    /* allocate enough space for response */
    parser->data = realloc(parser->data, sizeof(char) * (parser->data_len + len));

    /* fill the buffer */
    for (idx = 0; idx < len; idx++)
    {
      parser->data[parser->data_len++] = *src++;
    }
    
    parser->state = LwHHTP_PARSER_READY;
  }
  return ret;
}

uint16_t lwhttp_parser_run(lwhttp_parser_t* parser)
{
  uint16_t ret = 0;
  uint16_t parser_idx = 0;
  uint16_t length_before_body = 0;
  char* start = NULL;
  char* end = NULL;
  char* token = "\r\n";
  
  uint8_t empty_line_found = 0;
  
  if (parser != NULL)
  {
    start = parser->data;
    end = parser->data;
  
    for (start = parser->data; end != NULL; parser_idx++)
    {
      /* Response body */
      if (1 == empty_line_found)
      {
        parser->body = start;
        parser->body_len = (uint16_t)(&parser->data[parser->data_len] - start); 
        
        /* Quit loop intentionally */
        end = NULL;
        
        parser->state = LwHHTP_PARSER_END;
      }
      else
      {
        /* Get next line */
        end = strstr(start, token);

        if (end != NULL)
        {
          /* request response line */
          if (parser_idx == 0)
          {
            parser->initial = start;
            parser->initial_len = (uint16_t)(end-start);
            
            parser->state = LwHHTP_PARSER_DO_INITIAL;
          }
          /* header or body line */
          else
          {
            /* look for empty line, an empty line indicates next line is response body start */
            if ((end - start) <= 0)
            {
              empty_line_found = 1;
              parser->state = LwHHTP_PARSER_DO_BODY;
              ret = 1;
            }
            /* Header */
            else
            {
              /* Increase header counter */
              parser->headers_len++;

              /* If header tokens were provided */
              if ((0 < parser->headers_size) && (parser->headers != NULL))
              {
                
                /* if header tokens are available */
                if (parser->headers_len <= parser->headers_size)
                {
                  /* TODO: Parse headers in tokens */
                }
              }
              parser->state = LwHHTP_PARSER_DO_HEADER;
            }
          }
          
          /* Skip token for next iteration (EOL) */
          start = end + strlen(token);
        }
        else
        {
          parser->state = LwHHTP_PARSER_ERROR;
        }
      }
    }
  }
  
  return ret;
}

uint16_t lwhttp_parser_get_status(lwhttp_parser_t* parser)
{
  uint16_t ret = 0;
  return ret;
}

uint16_t lwhttp_parser_get_header(lwhttp_parser_t* parser, char* key, lwhttp_header_token_t** header_token)
{
  uint16_t ret = 0;
  return ret;
}

uint16_t lwhttp_parser_get_body(lwhttp_parser_t* parser, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((parser != NULL) && (dest != NULL) && (len != NULL))
  {
    *dest = parser->body;
    *len = parser->body_len;
    ret = 1;
  }
  return ret;
}

uint16_t lwhttp_response_parser_free(lwhttp_parser_t* parser)
{
  uint16_t ret = 0;
  if (parser != NULL)
  {
    free(parser->data);
    ret = 1;
  }
  return ret;
}


uint16_t lwhttp_builder_init(lwhttp_builder_t* builder, char* dest, uint16_t size, lwhttp_method_t method, const char* url)
{
  uint16_t ret = 0;
  if ((builder != NULL) && (dest != NULL) && (size > 0) && ((method >= 0) && (method < LwHHTP_MAX)) && (url != NULL))
  {
    /* Clean buffer */
    memset(dest, 0x00, size);
    
    builder->data = dest;
    builder->len = 0;
    builder->size = size;

    strcpy(builder->data, LwHTTP_methods[method]);
    strcat(builder->data, " ");
    strcat(builder->data, url);
    strcat(builder->data, " HTTP/1.0\r\n");
    
    builder->len += strlen(builder->data);
    
    builder->state = LwHHTP_BUILDER_INIT;

    ret = 1;
  }
  
  return ret;
}

uint16_t lwhttp_builder_add_header(lwhttp_builder_t* builder, const char* header, const char* value)
{
  uint16_t ret = 0;
  char* str_ptr = NULL; 
  if ((builder != NULL) && (header != NULL) && (value != NULL))
  {
    if ((builder->state >= LwHHTP_BUILDER_INIT) && (builder->state <= LwHHTP_BUILDER_ADDED_HEADER))
    {
      str_ptr = builder->data_ptr;
      
      if (builder->state == LwHHTP_BUILDER_ADDED_HEADER)
      {
        /* Overwrite EOL chars from last line */
        str_ptr -= 2;
      }
      
      strcat(str_ptr, header);
      strcat(str_ptr, ": ");
      strcat(str_ptr, value);
      strcat(str_ptr, "\r\n");
      
      builder->len = strlen(str_ptr);
      
      builder->state = LwHHTP_BUILDER_ADDED_HEADER;
      ret = 1;
    } 
  }
  
  return ret;
}

uint16_t lwhttp_builder_add_body(lwhttp_builder_t* builder, const char* body, uint16_t len)
{
  uint16_t ret = 0;
  if ((builder != NULL) && (body != NULL) && (len > 0))
  {
    if ((builder->state >= LwHHTP_BUILDER_INIT) && (builder->state <= LwHHTP_BUILDER_ADDED_HEADER))
    {
      strcat(builder->data_ptr, "\r\n");
      strncat(builder->data_ptr, body, len);
      strcat(builder->data_ptr, "\r\n");
      
      builder->len = strlen(builder->data_ptr);
      
      builder->state = LwHHTP_BUILDER_ADDED_BODY;
      ret = 1;
    } 
  }
  
  return ret;
}

uint16_t lwhttp_builder_end(lwhttp_builder_t* builder)
{
  uint16_t ret = 0;
  if (builder != NULL)
  {
    if ((builder->state >= LwHHTP_BUILDER_INIT) && (builder->state <= LwHHTP_BUILDER_ADDED_BODY))
    {
      strcat(builder->data_ptr, "\r\n");      
      builder->len = strlen(builder->data_ptr);
      builder->data_ptr[builder->len] = '\0';
      builder->state = LwHHTP_BUILDER_ADDED_END;
      ret = 1;
    } 
  }
  
  return ret;
}

uint16_t lwhttp_builder_get_data(lwhttp_builder_t* builder, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((builder != NULL) && (dest != NULL) && (len != NULL))
  {
    *dest = builder->data;
    *len = builder->len;
    ret = 1;
  }
  return ret;
}

