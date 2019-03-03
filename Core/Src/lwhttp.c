#include "lwhttp.h"
#include <stdlib.h>
#include <stdio.h>

static const char* LwHTTP_eol_string = "\r\n";
static const char* LwHTTP_version_v1_0_string = "HTTP/1.0";
static const char* LwHTTP_method_string[LwHHTP_MAX] = {"POST", "GET"};
static const char* LwHTTP_request_line_fmt = "%s %s %s%s";
static const char* LwHTTP_message_header_fmt = "%s: %s%s";

static void lwhttp_token_reset(lwhttp_token_t* token);

static void lwhttp_parse_message_header(lwhttp_message_header_t* message_header);
static void lwhttp_parse_status_line(lwhttp_status_line_t* status_line);
static void lwhttp_parse_request_line(lwhttp_request_line_t* request_line);
static void lwhttp_parse_start_line(lwhttp_start_line_t* start_line);

static void lwhttp_token_reset(lwhttp_token_t* token)
{
  if (token != NULL)
  {
    token->data = NULL;
    token->len = 0;
  }
}

uint16_t lwhttp_response_parser_init(lwhttp_response_t* response)
{
  uint16_t ret = 0;
  if (response != NULL)
  {
    /* Parser internals */
    response->state = LwHHTP_RESPONSE_PARSER_INIT;
    
    /* Buffer */
    lwhttp_token_reset(&response->buffer);
    
    /* HTTP/1.1 Sec6.1 Response - Status line */
    lwhttp_token_reset(&response->status_line.buffer);
    lwhttp_token_reset(&response->status_line.http_version);
    lwhttp_token_reset(&response->status_line.status_code);
    lwhttp_token_reset(&response->status_line.reason_phrase);

    /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
    response->message_headers.headers = NULL;
    response->message_headers.count = 0;

    /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
    lwhttp_token_reset(&response->message_body);  
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
    response->buffer.data = realloc(response->buffer.data, sizeof(char) * (response->buffer.len + len));

    /* fill the buffer */
    for (idx = 0; idx < len; idx++)
    {
      response->buffer.data[response->buffer.len++] = *src++;
    }
    
    response->state = LwHHTP_RESPONSE_PARSER_READY;
  }
  return ret;
}

static void lwhttp_parse_message_header(lwhttp_message_header_t* message_header)
{
  char* start_ptr = NULL;
  char* end_ptr = NULL;
  uint16_t len = 0; 
  
  if (message_header != NULL)
  {
    if ((message_header->buffer.data != NULL) && (message_header->buffer.len > 0))
    {
      start_ptr = message_header->buffer.data;
      end_ptr = strstr(start_ptr, ": ");
      
      /* If string found inside boundaries */
      if ((end_ptr != NULL) && ((end_ptr - message_header->buffer.data) <= message_header->buffer.len) && (end_ptr >= start_ptr))
      {
        message_header->field_name.data = start_ptr;
        message_header->field_name.len = (uint16_t)(end_ptr - start_ptr);
      }
      else
      {
        message_header->field_name.data = NULL;
        message_header->field_name.len = 0;
      }
      
      start_ptr = (char*)(end_ptr + 2);         
      end_ptr = &message_header->buffer.data[message_header->buffer.len];
      
      /* If string found inside boundaries */
      if ((end_ptr != NULL) && ((end_ptr - message_header->buffer.data) <= message_header->buffer.len) && (end_ptr >= start_ptr))
      {
        /* HTTP/1.1 Sec6.1.1 Response - Status Code and Reason Phrase */
        message_header->field_value.data = start_ptr;
        message_header->field_value.len = (uint16_t)(end_ptr - start_ptr);
      }
      else
      {
        message_header->field_value.data = NULL;
        message_header->field_value.len = 0;
      }
      
    }
  }
}

static void lwhttp_parse_status_line(lwhttp_status_line_t* status_line)
{
  lwhttp_parse_start_line((lwhttp_start_line_t*)status_line);
}

static void lwhttp_parse_request_line(lwhttp_request_line_t* request_line)
{
  lwhttp_parse_start_line((lwhttp_start_line_t*)request_line);
}

static void lwhttp_parse_start_line(lwhttp_start_line_t* start_line)
{
  char* start_ptr = NULL;
  char* end_ptr = NULL;
  uint16_t len = 0; 
  
  if (start_line != NULL)
  {
    if ((start_line->buffer.data != NULL) && (start_line->buffer.len > 0))
    {
      start_ptr = start_line->buffer.data;
      end_ptr = strstr(start_ptr, " ");
      
      /* If string found inside boundaries */
      if ((end_ptr != NULL) && ((end_ptr - start_line->buffer.data) <= start_line->buffer.len) && (end_ptr >= start_ptr))
      {
        start_line->field1.data = start_ptr;
        start_line->field1.len = (uint16_t)(end_ptr - start_ptr);
      }
      else
      {
        start_line->field1.data = NULL;
        start_line->field1.len = 0;
      }
      
      start_ptr = (char*)(end_ptr + 1);
      end_ptr = strstr(start_ptr, " ");
      
      /* If string found inside boundaries */
      if ((end_ptr != NULL) && ((end_ptr - start_line->buffer.data) <= start_line->buffer.len) && (end_ptr >= start_ptr))
      {
        /* HTTP/1.1 Sec6.1.1 Response - Status Code and Reason Phrase */
        start_line->field2.data = start_ptr;
        start_line->field2.len = (uint16_t)(end_ptr - start_ptr);
      }
      else
      {
        start_line->field2.data = NULL;
        start_line->field2.len = 0;
      }
      
      start_ptr = (char*)(end_ptr + 1);         
      end_ptr = &start_line->buffer.data[start_line->buffer.len];
      
      /* If string found inside boundaries */
      if ((end_ptr != NULL) && ((end_ptr - start_line->buffer.data) <= start_line->buffer.len) && (end_ptr >= start_ptr))
      {
        /* HTTP/1.1 Sec6.1.1 Response - Status Code and Reason Phrase */
        start_line->field3.data = start_ptr;
        start_line->field3.len = (uint16_t)(end_ptr - start_ptr);
      }
      else
      {
        start_line->field3.data = NULL;
        start_line->field3.len = 0;
      }
    }
  }        
}

uint16_t lwhttp_response_parser_run(lwhttp_response_t* response)
{
  uint16_t ret = 0;
  char* start = NULL;
  char* end = NULL;
    
  if (response != NULL)
  {    
    if (LwHHTP_RESPONSE_PARSER_READY == response->state)
    {
      response->state = LwHHTP_RESPONSE_PARSER_STATUS_LINE;
    }
    else
    {
      response->state = LwHHTP_RESPONSE_PARSER_ERROR;
    }
    
    for (start = response->buffer.data; response->state <= LwHHTP_RESPONSE_PARSER_END; start = (char*)(end + strlen(LwHTTP_eol_string)))
    {
      /* Get EOL */
      end = strstr(start, LwHTTP_eol_string);
      if ((end == NULL) && (response->state != LwHHTP_RESPONSE_PARSER_MESSAGE_BODY))
      {
        response->state = LwHHTP_RESPONSE_PARSER_ERROR;
      }
      
      switch(response->state){
      case (LwHHTP_RESPONSE_PARSER_STATUS_LINE):
            /* Fill status line buffer */
            response->status_line.buffer.data = start;
            response->status_line.buffer.len = (uint16_t)(end-start);
            
            /* Parse status line */
            lwhttp_parse_status_line(&response->status_line);
            
            response->state = LwHHTP_RESPONSE_PARSER_MESSAGE_HEADER;
        break;
        
      case (LwHHTP_RESPONSE_PARSER_MESSAGE_HEADER):
        if ((end - start) <= 0)
        {
          /* Empty line found, next line is body */
          response->state = LwHHTP_RESPONSE_PARSER_MESSAGE_BODY;
        }
        else
        {
          /* Allocate message header */
          response->message_headers.count++;
          response->message_headers.headers = realloc(response->message_headers.headers, sizeof(lwhttp_message_header_t) * response->message_headers.count);
          
          /* Fill message header buffer */
          response->message_headers.headers[response->message_headers.count-1].buffer.data = start;
          response->message_headers.headers[response->message_headers.count-1].buffer.len = (uint16_t)(end-start);

          /* Parse message header */
          lwhttp_parse_message_header(&response->message_headers.headers[response->message_headers.count-1]);
          
          response->state = LwHHTP_RESPONSE_PARSER_MESSAGE_HEADER;
        }
        break;
        
      case (LwHHTP_RESPONSE_PARSER_MESSAGE_BODY):
        response->message_body.data = start;
        response->message_body.len = (uint16_t)(&response->buffer.data[response->buffer.len] - start);
        response->state = LwHHTP_RESPONSE_PARSER_END;
        break;
        
      default:
        break;
        /* Do nothing */
      }
    }
    
    if (response->state == LwHHTP_RESPONSE_PARSER_END)
    {
      ret = 1;
    }
  }
  
  return ret;
}

uint16_t lwhttp_response_get(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((response != NULL) && (dest != NULL) && (len != NULL))
  {
    if (response->buffer.data != NULL)
    {
      *dest = response->buffer.data;
      *len = response->buffer.len;
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
    if (response->status_line.buffer.data != NULL)
    {
      *dest = response->status_line.buffer.data;
      *len = response->status_line.buffer.len;
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

uint16_t lwhttp_response_get_status_code(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((response != NULL) && (dest != NULL) && (len != NULL))
  {
    if (response->status_line.status_code.data != NULL)
    {
      *dest = response->status_line.status_code.data;
      *len = response->status_line.status_code.len;
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

uint16_t lwhttp_response_get_message_header_by_name(lwhttp_response_t* response, const char* field_name, lwhttp_message_header_t** dest)
{
  uint8_t idx = 0;
  uint16_t ret = 0;

  if ((response != NULL) && (field_name != NULL)&& (dest != NULL))
  {
    *dest = NULL;
    
    for (idx = 0; (idx < response->message_headers.count) && (*dest == NULL) && (response->message_headers.count > 0); idx++)
    {
      /* If Status is OK and Body has response from Thingspeak server */
      if (0 == strncmp(response->message_headers.headers[idx].field_name.data, field_name, strlen(field_name)))
      {
        *dest = &response->message_headers.headers[idx];
        ret = 1;
        break;
      }
    }
  }
  
  return ret;
}

uint16_t lwhttp_response_get_message_header_by_index(lwhttp_response_t* response, uint8_t index, lwhttp_message_header_t** dest)
{
  uint8_t idx = 0;
  uint16_t ret = 0;

  if ((response != NULL) && (dest != NULL))
  {
    *dest = NULL;
    
    if ((index <= response->message_headers.count) && (index >= 0))
    {
      *dest = &response->message_headers.headers[idx];
      ret = 1;   
    }
  }
  
  return ret;
}

uint16_t lwhttp_response_get_message_body(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((response != NULL) && (dest != NULL) && (len != NULL))
  {
    if (response->message_body.data != NULL)
    {
      *dest = response->message_body.data;
      *len = response->message_body.len;
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
    free(response->buffer.data);
    free(response->message_headers.headers);
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
    
    /* Buffer */
    lwhttp_token_reset(&request->buffer);  

    /* HTTP/1.1 Sec5.1 Request - Request line */
    lwhttp_token_reset(&request->request_line.buffer);  
    lwhttp_token_reset(&request->request_line.method);  
    lwhttp_token_reset(&request->request_line.request_uri);  
    lwhttp_token_reset(&request->request_line.http_version);  

    /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
    request->message_headers.headers = NULL;
    request->message_headers.count = 0;

    /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
    lwhttp_token_reset(&request->message_body);
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
      snprintf(&temp_buff[0], sizeof(temp_buff), LwHTTP_request_line_fmt, LwHTTP_method_string[method], request_uri, LwHTTP_version_v1_0_string, LwHTTP_eol_string);
      len = strlen(&temp_buff[0]);
      
      /* Allocate memory + NULL Terminator */
      request->buffer.data = realloc(request->buffer.data, sizeof(char) * (len + 1));
      
      /* Pointers to request line */
      request->request_line.buffer.data = request->buffer.data;
      request->request_line.buffer.len = len;
      
      /* Copy data */
      for (idx = 0; idx < len; idx++)
      {
          request->request_line.buffer.data[idx] = temp_buff[idx];
          request->buffer.len++;
      }
      
      /* Parse request line */
      lwhttp_parse_request_line(&request->request_line);
      
      /* Add NULL Terminator */
      request->buffer.data[request->buffer.len] = '\0';
      
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
  
  if ((request != NULL) && (header != NULL) && (value != NULL))
  {
    if ((request->state >= LwHHTP_BUILDER_ADDED_REQUEST) && (request->state <= LwHHTP_BUILDER_ADDED_HEADER))
    {      
      /* Format message header + Empty line */
      snprintf(&temp_buff[0], sizeof(temp_buff), LwHTTP_message_header_fmt, header, value, LwHTTP_eol_string);
      len = strlen(&temp_buff[0]);
      
      /* Allocate memory */
      request->buffer.data = realloc(request->buffer.data, sizeof(char) * (request->buffer.len + len));

      /* Alloc Message header */
      request->message_headers.count++;
      request->message_headers.headers = realloc(request->message_headers.headers, sizeof(lwhttp_message_header_t) * request->message_headers.count);
      
      /* Pointers to message header, overwrite NULL terminator */
      request->message_headers.headers[request->message_headers.count - 1].buffer.data = &request->buffer.data[request->buffer.len];
      request->message_headers.headers[request->message_headers.count - 1].buffer.len = len;
      
      /* Copy data */
      for (idx = 0; idx < len; idx++)
      {
          request->message_headers.headers[request->message_headers.count - 1].buffer.data[idx] = temp_buff[idx];
          request->buffer.len++;
      }
      
      /* Parse message header */
      lwhttp_parse_message_header(&request->message_headers.headers[request->message_headers.count-1]);
      
      /* Add NULL Terminator */
      request->buffer.data[request->buffer.len] = '\0';

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
      request->buffer.data = realloc(request->buffer.data, sizeof(char) * (request->buffer.len + len + 2));
      
      /* Add Empty line, overwrite NULL terminator */
      strcpy(&request->buffer.data[request->buffer.len], LwHTTP_eol_string);
      request->buffer.len += strlen(LwHTTP_eol_string);
      
      /* Pointers to message body */
      request->message_body.data = &request->buffer.data[request->buffer.len];
      request->message_body.len = len;      

      /* Copy data */
      for (idx = 0; idx < len; idx++)
      {
          request->message_body.data[idx] = body[idx];
          request->buffer.len++;
      }
      
      /* Add NULL Terminator */
      request->buffer.data[request->buffer.len] = '\0';

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
    if (request->buffer.data != NULL)
    {
      *dest = request->buffer.data;
      *len = request->buffer.len;
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
    free(request->buffer.data);
    free(request->message_headers.headers);
    ret = lwhttp_request_parser_init(request);
  }
  return ret;
}

