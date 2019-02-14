#include "lwhttp.h"

char* LwHTTP_methods[LwHHTP_MAX] = {"POST", "GET"};

uint16_t lwhttp_parse(char* src, uint16_t src_len, lwhttp_tok_t* tok, uint16_t tok_len)
{
  return 0;
}

uint16_t lwhttp_new_builder(lwhttp_builder_t* builder, char* dest, uint16_t size)
{
  uint16_t ret = 0;
  if ((dest != NULL) && (size > 0))
  {
    builder->data = dest;
    builder->data_ptr = dest;
    builder->len = 0;
    builder->size = size;
    builder->state = LwHHTP_BUILDER_INIT;
    
    ret = 1;
  }
  
  return ret;
}

uint16_t lwhttp_add_request(lwhttp_builder_t* builder, lwhttp_method_t method, const char* url)
{
  uint16_t ret = 0;
  if ((builder != NULL) && ((method >= 0) && (method < LwHHTP_MAX)) && (url != NULL))
  {
    if (builder->state == LwHHTP_BUILDER_INIT)
    {
      strcpy(builder->data_ptr, LwHTTP_methods[method]);
      strcat(builder->data_ptr, " ");
      strcat(builder->data_ptr, url);
      strcat(builder->data_ptr, " HTTP/1.0\r\n");
      
      builder->len += strlen(builder->data_ptr);
      
      builder->state = LwHHTP_BUILDER_ADDED_REQUEST;
      ret = 1;
    }    
  }
  
  return ret;
}

uint16_t lwhttp_add_header(lwhttp_builder_t* builder, const char* header, const char* value)
{
  uint16_t ret = 0;
  if ((builder != NULL) && (header != NULL) && (value != NULL))
  {
    if ((builder->state >= LwHHTP_BUILDER_ADDED_REQUEST) && (builder->state <= LwHHTP_BUILDER_ADDED_HEADER))
    {
      strcat(builder->data_ptr, header);
      strcat(builder->data_ptr, ": ");
      strcat(builder->data_ptr, value);
      strcat(builder->data_ptr, "\r\n");
      
      builder->len = strlen(builder->data_ptr);
      
      builder->state = LwHHTP_BUILDER_ADDED_HEADER;
      ret = 1;
    } 
  }
  
  return ret;
}

uint16_t lwhttp_add_body(lwhttp_builder_t* builder, const char* body, uint16_t len)
{
  uint16_t ret = 0;
  if ((builder != NULL) && (body != NULL) && (len > 0))
  {
    if ((builder->state >= LwHHTP_BUILDER_ADDED_REQUEST) && (builder->state <= LwHHTP_BUILDER_ADDED_HEADER))
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

uint16_t lwhttp_end(lwhttp_builder_t* builder)
{
  uint16_t ret = 0;
  if (builder != NULL)
  {
    if ((builder->state >= LwHHTP_BUILDER_ADDED_REQUEST) && (builder->state <= LwHHTP_BUILDER_ADDED_BODY))
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
