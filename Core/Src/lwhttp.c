#include "lwhttp.h"
#include <stdlib.h>
#include <stdio.h>

char* LwHTTP_methods[LwHHTP_MAX] = {"POST", "GET"};

uint16_t lwhttp_response_parser_init(lwhttp_response_t* response)
{
  uint16_t ret = 0;
  if (response != NULL)
  {
    /* Parser internals */
    response->state = LwHHTP_PARSER_INIT;
    response->data = NULL;
    response->data_len = 0;
    
    /* HTTP/1.1 Sec6.1 Response - Status line */
    response->status_line.data = NULL;
    response->status_line.data_len = 0;
    response->status_line.http_version = NULL;
    response->status_line.http_version_len = 0;
    response->status_line.status_code = NULL;
    response->status_line.status_code_len = 0;
    response->status_line.reason_phrase = NULL;
    response->status_line.reason_phrase_len = 0;

    /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
    response->message_headers = NULL;
    response->message_headers_len = 0;

    /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
    response->message_body = NULL;
    response->message_body_len = 0;        
  }
  return ret;
}

uint16_t lwhttp_response_parser_write(lwhttp_response_t* response, char* src, uint16_t len)
{
  uint16_t ret = 0;
  uint16_t idx = 0;
  if ((response != NULL) && (src != NULL) && (len > 0))
  {
    /* allocate enough space for response */
    response->data = realloc(response->data, sizeof(char) * (response->data_len + len));

    /* fill the buffer */
    for (idx = 0; idx < len; idx++)
    {
      response->data[response->data_len++] = *src++;
    }
    
    response->state = LwHHTP_PARSER_READY;
  }
  return ret;
}

uint16_t lwhttp_response_parser_run(lwhttp_response_t* response)
{
  uint16_t ret = 0;
  uint16_t parser_idx = 0;
  uint16_t length_before_body = 0;
  char* start = NULL;
  char* end = NULL;
  char* token = "\r\n";
  
  uint8_t empty_line_found = 0;
  
  if (response != NULL)
  {
    start = response->data;
    end = response->data;
  
    for (start = response->data; end != NULL; parser_idx++)
    {
      /* Response body */
      if (1 == empty_line_found)
      {
        response->message_body = start;
        response->message_body_len = (uint16_t)(&response->data[response->data_len] - start); 
        
        /* Quit loop intentionally */
        end = NULL;
        
        response->state = LwHHTP_PARSER_END;
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
            response->status_line.status_code = start;
            response->status_line.status_code_len = (uint16_t)(end-start);
            
            response->state = LwHHTP_PARSER_DO_INITIAL;
          }
          /* header or body line */
          else
          {
            /* look for empty line, an empty line indicates next line is response body start */
            if ((end - start) <= 0)
            {
              empty_line_found = 1;
              response->state = LwHHTP_PARSER_DO_BODY;
              ret = 1;
            }
            /* Header */
            else
            {
              /* Increase header counter */
              response->message_headers_len++;

#if 0
              /* If header tokens were provided */
              if ((0 < response->headers_size) && (response->headers != NULL))
              {
                
                /* if header tokens are available */
                if (response->headers_len <= response->headers_size)
                {
                  /* TODO: Parse headers in tokens */
                }
              }
#endif
              
              response->state = LwHHTP_PARSER_DO_HEADER;
            }
          }
          
          /* Skip token for next iteration (EOL) */
          start = end + strlen(token);
        }
        else
        {
          response->state = LwHHTP_PARSER_ERROR;
        }
      }
    }
  }
  
  return ret;
}

uint16_t lwhttp_response_get(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((response != NULL) && (dest != NULL) && (len != NULL))
  {
    if (response->data != NULL)
    {
      *dest = response->data;
      *len = response->data_len;
      ret = 1;
    }
    else
    {
      *dest = NULL;
      *len = 0;
      ret = 0;
    }
  }
  return ret;
}

uint16_t lwhttp_response_get_status_line(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((response != NULL) && (dest != NULL) && (len != NULL))
  {
    if (response->status_line.data != NULL)
    {
      *dest = response->status_line.data;
      *len = response->status_line.data_len;
      ret = 1;
    }
    else
    {
      *dest = NULL;
      *len = 0;
      ret = 0;
    }
  }
  return ret;
}

uint16_t lwhttp_response_get_status_code(lwhttp_response_t* response, uint16_t* status_code)
{
  uint16_t ret = 0;
  return ret;
}

uint16_t lwhttp_response_get_message_header(lwhttp_response_t* response, char* field_name, lwhttp_message_header_t* header_ptr)
{
  uint16_t ret = 0;
  return ret;
}

uint16_t lwhttp_response_get_message_body(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((response != NULL) && (dest != NULL) && (len != NULL))
  {
    if (response->message_body != NULL)
    {
      *dest = response->message_body;
      *len = response->message_body_len;
      ret = 1;
    }
    else
    {
      *dest = NULL;
      *len = 0;
      ret = 0;
    }
  }
  return ret;
}

uint16_t lwhttp_response_parser_free(lwhttp_response_t* response)
{
  uint16_t ret = 0;
  if (response != NULL)
  {
    free(response->data);
    ret = lwhttp_response_parser_init(response);
  }
  return ret;
}

uint16_t lwhttp_request_parser_init(lwhttp_request_t* request)
{
  if (request != NULL)
  {
    /* Parser internals */
    request->state = LwHHTP_BUILDER_INIT;
    request->data = NULL;
    request->data_len = 0;
    
    /* HTTP/1.1 Sec5.1 Request - Request line */
    request->request_line.data = NULL;
    request->request_line.data_len = 0;
    request->request_line.method = NULL;
    request->request_line.method_len = 0;
    request->request_line.request_uri = NULL;
    request->request_line.request_uri_len = 0;
    request->request_line.http_version = NULL;
    request->request_line.http_version_len = 0;

    /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
    request->message_headers = NULL;
    request->message_headers_len = 0;

    /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
    request->message_body = NULL;
    request->message_body_len = 0;
  }
}

uint16_t lwhttp_request_put_request_line(lwhttp_request_t* request, lwhttp_method_t method, const char* request_uri)
{
  char temp_buff[128];
  uint16_t ret = 0;
  uint16_t len = 0;
  uint16_t idx = 0;
  
  if ((request != NULL) && ((method >= 0) && (method < LwHHTP_MAX)) && (request_uri != NULL))
  {
    if (request->state == LwHHTP_BUILDER_INIT)
    {
      /* Format request line + Empty line */
      snprintf(&temp_buff[0], sizeof(temp_buff), "%s %s HTTP/1.0\r\n", LwHTTP_methods[method], request_uri);
      len = strlen(&temp_buff[0]);
      
      /* Allocate memory + NULL Terminator */
      request->data = realloc(request->data, sizeof(char) * (len + 1));
      
      /* Pointers to request line */
      request->request_line.data = request->data;
      request->request_line.data_len = len;
      
      /* Copy data */
      for (idx = 0; idx < len; idx++)
      {
          request->request_line.data[idx] = temp_buff[idx];
          request->data_len++;
      }
      
      /* Add NULL Terminator */
      request->data[request->data_len] = '\0';
      
      request->state = LwHHTP_BUILDER_ADDED_REQUEST;

      ret = 1;
    }
  }
  
  return ret;
}

uint16_t lwhttp_request_put_message_header(lwhttp_request_t* request, const char* header, const char* value)
{
  char temp_buff[128];
  uint16_t ret = 0;
  uint16_t len = 0;
  uint16_t idx = 0;
  
  char* message_header;
  uint16_t message_header_len;
  
  if ((request != NULL) && (header != NULL) && (value != NULL))
  {
    if ((request->state >= LwHHTP_BUILDER_ADDED_REQUEST) && (request->state <= LwHHTP_BUILDER_ADDED_HEADER))
    {      
      /* Format message header + Empty line */
      snprintf(&temp_buff[0], sizeof(temp_buff), "%s: %s\r\n\r\n", header, value);
      len = strlen(&temp_buff[0]);
      
      /* Allocate memory */
      request->data = realloc(request->data, sizeof(char) * (request->data_len + len));

      /* Pointers to message header, overwrite NULL terminator */
      message_header = &request->data[request->data_len];
      message_header_len = len;
      
      /* Copy data */
      for (idx = 0; idx < len; idx++)
      {
          message_header[idx] = temp_buff[idx];
          request->data_len++;
      }
      
      /* Add NULL Terminator */
      request->data[request->data_len] = '\0';

      request->state = LwHHTP_BUILDER_ADDED_HEADER;
      ret = 1;
    } 
  }
  
  return ret;
}

uint16_t lwhttp_request_put_message_body(lwhttp_request_t* request, const char* body, uint16_t len)
{
  uint16_t ret = 0;
  uint16_t idx = 0;
  
  if ((request != NULL) && (body != NULL) && (len > 0))
  {
    if ((request->state >= LwHHTP_BUILDER_ADDED_REQUEST) && (request->state <= LwHHTP_BUILDER_ADDED_HEADER))
    {
      /* Allocate memory */
      request->data = realloc(request->data, sizeof(char) * (request->data_len + len + 2));
      
      /* Add Empty line, overwrite NULL terminator */
      request->data[request->data_len++] = '\r';
      request->data[request->data_len++] = '\n';
      
      /* Pointers to message body */
      request->message_body = &request->data[request->data_len];
      request->message_body_len = len;      

      /* Copy data */
      for (idx = 0; idx < len; idx++)
      {
          request->message_body[idx] = body[idx];
          request->data_len++;
      }
      
      /* Add NULL Terminator */
      request->data[request->data_len] = '\0';

      request->state = LwHHTP_BUILDER_ADDED_BODY;
      ret = 1;
    } 
  }
  
  return ret;
}

uint16_t lwhttp_request_get(lwhttp_request_t* request, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((request != NULL) && (dest != NULL) && (len != NULL))
  {
    if (request->data != NULL)
    {
      *dest = request->data;
      *len = request->data_len;
      ret = 1;
    }
    else
    {
      *dest = NULL;
      *len = 0;
      ret = 0;
    }
  }
  return ret;
}

uint16_t lwhttp_request_parser_free(lwhttp_request_t* request)
{
  uint16_t ret = 0;
  if (request != NULL)
  {
    free(request->data);
    ret = lwhttp_request_parser_init(request);
  }
  return ret;
}

