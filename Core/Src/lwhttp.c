#include "lwhttp.h"
#include <stdlib.h>
#include <stdio.h>

static const char* LwHTTP_eol_string = "\r\n";
static const char* LwHTTP_version_v1_0_string = "HTTP/1.0";
static const char* LwHTTP_method_string[LwHHTP_MAX] = {"POST", "GET"};
static const char* LwHTTP_start_line_fmt = "%s %s %s%s";
static const char* LwHTTP_message_header_fmt = "%s: %s%s";

/* LwHTTP General functions */
static uint16_t lwhttp_message_init(lwhttp_message_t* message, lwhttp_message_type_t type);
static uint16_t lwhttp_message_free(lwhttp_message_t* message);
static uint16_t lwhttp_message_parse(lwhttp_message_t* message);
static uint16_t lwhttp_message_get(lwhttp_message_t* message, char** dest, uint16_t* len);
static uint16_t lwhttp_message_put(lwhttp_message_t* message, const char* src, uint16_t len);
static uint16_t lwhttp_message_get_start_line(lwhttp_message_t* message, char** dest, uint16_t* len);
static uint16_t lwhttp_message_put_start_line(lwhttp_message_t* message, const char* field1, const char* field2, const char* field3);
static uint16_t lwhttp_message_get_message_header(lwhttp_message_t* message, const char* field_name, lwhttp_message_header_t** dest);
static uint16_t lwhttp_message_put_message_header(lwhttp_message_t* message, const char* header, const char* value);
static uint16_t lwhttp_message_get_message_body(lwhttp_message_t* message, char** dest, uint16_t* len);
static uint16_t lwhttp_message_put_message_body(lwhttp_message_t* message, const char* body, uint16_t len);

/* LwHTTP Auxiliar functions */
static void lwhttp_token_set(lwhttp_token_t* token, const char* src, uint16_t len);
static uint16_t lwhttp_token_get(lwhttp_token_t* token, char** dest, uint16_t* len);
static void lwhttp_token_reset(lwhttp_token_t* token);
static void lwhttp_parse_message_header(lwhttp_message_header_t* message_header);
static void lwhttp_parse_start_line(lwhttp_start_line_t* start_line, lwhttp_message_type_t type);

static void lwhttp_token_set(lwhttp_token_t* token, const char* src, uint16_t len)
{
  if (token != NULL)
  {
    token->data = (char*)src;
    token->len = len;
  }
}

static uint16_t lwhttp_token_get(lwhttp_token_t* token, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((token != NULL) && (dest != NULL) && (len != NULL))
  {
    *dest = token->data;
    *len = token->len;
    ret = 1;
  }
  return ret;
}

static void lwhttp_token_reset(lwhttp_token_t* token)
{
  lwhttp_token_set(token, NULL, 0);
}

uint16_t lwhttp_response_init(lwhttp_response_t* response)
{
  return lwhttp_message_init((lwhttp_message_t*)response, LwHHTP_MESSAGE_TYPE_RESPONSE);
}

uint16_t lwhttp_request_init(lwhttp_request_t* request)
{
  return lwhttp_message_init((lwhttp_message_t*)request, LwHHTP_MESSAGE_TYPE_REQUEST);
}

static uint16_t lwhttp_message_init(lwhttp_message_t* message, lwhttp_message_type_t type)
{
  uint16_t ret = 0;
  if ((message != NULL) && ((type >= LwHHTP_MESSAGE_TYPE_REQUEST) && (type <= LwHHTP_MESSAGE_TYPE_RESPONSE)))
  {
    /* Parser/Builder internals */
    message->builder_state = LwHHTP_BUILDER_INIT;
    message->parser_state = LwHHTP_PARSER_INIT;
    
    /* General */
    message->message_type = type;
    lwhttp_token_reset(&message->buffer);
    
    /* HTTP/1.1 Sec6.1 Response - Status line */
    if (LwHHTP_MESSAGE_TYPE_RESPONSE == message->message_type)
    {
      lwhttp_token_reset(&message->start_line.status_line.buffer);
      lwhttp_token_reset(&message->start_line.status_line.http_version);
      lwhttp_token_reset(&message->start_line.status_line.status_code);
      lwhttp_token_reset(&message->start_line.status_line.reason_phrase);
    }
    else /* LwHHTP_MESSAGE_TYPE_REQUEST */
    {
      lwhttp_token_reset(&message->start_line.request_line.buffer);
      lwhttp_token_reset(&message->start_line.request_line.method);
      lwhttp_token_reset(&message->start_line.request_line.request_uri);
      lwhttp_token_reset(&message->start_line.request_line.http_version);
    }
    
    ret = lwhttp_message_free(message);
  }
  return ret;
}

uint16_t lwhttp_response_free(lwhttp_response_t* response)
{
  return lwhttp_message_free((lwhttp_message_t*) response);
}

uint16_t lwhttp_request_free(lwhttp_request_t* request)
{
  return lwhttp_message_free((lwhttp_message_t*) request);
}

static uint16_t lwhttp_message_free(lwhttp_message_t* message)
{
  uint16_t ret = 0;
  if (message != NULL)
  {
    /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
    free(message->message_headers.headers);
    message->message_headers.headers = NULL;
    message->message_headers.count = 0;

    /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
    free(message->buffer.data);
    lwhttp_token_reset(&message->message_body);  
    
    ret = 1;
  }
  return ret;
}

uint16_t lwhttp_response_parse(lwhttp_response_t* response)
{
  return lwhttp_message_parse((lwhttp_message_t*) response);
}

uint16_t lwhttp_request_parse(lwhttp_request_t* request)
{
  return lwhttp_message_parse((lwhttp_message_t*) request);
}

static uint16_t lwhttp_message_parse(lwhttp_message_t* message)
{
  uint16_t ret = 0;
  char* start = NULL;
  char* end = NULL;
    
  if (message != NULL)
  {    
    if (LwHHTP_PARSER_READY == message->parser_state)
    {
      message->parser_state = LwHHTP_PARSER_START_LINE;
    }
    else
    {
      message->parser_state = LwHHTP_PARSER_ERROR;
    }
    
    for (start = message->buffer.data; message->parser_state <= LwHHTP_PARSER_END; start = (char*)(end + strlen(LwHTTP_eol_string)))
    {
      /* Get EOL */
      end = strstr(start, LwHTTP_eol_string);
      if ((end == NULL) && (message->parser_state != LwHHTP_PARSER_MESSAGE_BODY))
      {
        message->parser_state = LwHHTP_PARSER_ERROR;
      }
      
      switch(message->parser_state){
      case (LwHHTP_PARSER_START_LINE):
            /* Fill start line buffer */
            if(LwHHTP_MESSAGE_TYPE_RESPONSE == message->message_type)
            {
              lwhttp_token_set(&message->start_line.status_line.buffer, start, (uint16_t)(end-start));
            }
            else /* LwHHTP_MESSAGE_TYPE_REQUEST */
            {
              lwhttp_token_set(&message->start_line.request_line.buffer, start, (uint16_t)(end-start));
            }
            
            /* Parse status line */
            lwhttp_parse_start_line(&message->start_line, message->message_type);
            
            message->parser_state = LwHHTP_PARSER_MESSAGE_HEADER;
        break;
        
      case (LwHHTP_PARSER_MESSAGE_HEADER):
        if ((end - start) <= 0)
        {
          /* Empty line found, next line is body */
          message->parser_state = LwHHTP_PARSER_MESSAGE_BODY;
        }
        else
        {
          /* Allocate message header */
          message->message_headers.count++;
          message->message_headers.headers = realloc(message->message_headers.headers, sizeof(lwhttp_message_header_t) * message->message_headers.count);
          
          /* Fill message header buffer */
          message->message_headers.headers[message->message_headers.count-1].buffer.data = start;
          message->message_headers.headers[message->message_headers.count-1].buffer.len = (uint16_t)(end-start);

          /* Parse message header */
          lwhttp_parse_message_header(&message->message_headers.headers[message->message_headers.count-1]);
          
          message->parser_state = LwHHTP_PARSER_MESSAGE_HEADER;
        }
        break;
        
      case (LwHHTP_PARSER_MESSAGE_BODY):
        message->message_body.data = start;
        message->message_body.len = (uint16_t)(&message->buffer.data[message->buffer.len] - start);
        message->parser_state = LwHHTP_PARSER_END;
        break;
        
      default:
        break;
        /* Do nothing */
      }
    }
    
    if (message->parser_state == LwHHTP_PARSER_END)
    {
      ret = 1;
    }
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

static void lwhttp_parse_start_line(lwhttp_start_line_t* start_line, lwhttp_message_type_t type)
{
  char* start_ptr = NULL;
  char* end_ptr = NULL;
  uint16_t len = 0; 
  
  lwhttp_token_t* buffer;
  lwhttp_token_t* field1;
  lwhttp_token_t* field2;
  lwhttp_token_t* field3;
  
  if (start_line != NULL)
  {
    if(LwHHTP_MESSAGE_TYPE_RESPONSE == type)
    {
      buffer = &start_line->status_line.buffer;
      field1 = &start_line->status_line.http_version;
      field2 = &start_line->status_line.status_code;
      field3 = &start_line->status_line.reason_phrase;
    }
    else /* LwHHTP_MESSAGE_TYPE_REQUEST */
    {
      buffer = &start_line->request_line.buffer;
      field1 = &start_line->request_line.method;
      field2 = &start_line->request_line.request_uri;
      field3 = &start_line->request_line.http_version;      
    }
    
    if ((buffer->data != NULL) && (buffer->len > 0))
    {
      start_ptr = buffer->data;
      end_ptr = strstr(start_ptr, " ");
      
      /* If string found inside boundaries */
      if ((end_ptr != NULL) && ((end_ptr - buffer->data) <= buffer->len) && (end_ptr >= start_ptr))
      {
        lwhttp_token_set(field1, start_ptr, (uint16_t)(end_ptr - start_ptr));
      }
      else
      {
        lwhttp_token_reset(field1);
      }
      
      start_ptr = (char*)(end_ptr + 1);
      end_ptr = strstr(start_ptr, " ");
      
      /* If string found inside boundaries */
      if ((end_ptr != NULL) && ((end_ptr - buffer->data) <= buffer->len) && (end_ptr >= start_ptr))
      {
        lwhttp_token_set(field2, start_ptr, (uint16_t)(end_ptr - start_ptr));
      }
      else
      {
        lwhttp_token_reset(field2);
      }
      
      start_ptr = (char*)(end_ptr + 1);         
      end_ptr = &buffer->data[buffer->len];
      
      /* If string found inside boundaries */
      if ((end_ptr != NULL) && ((end_ptr - buffer->data) <= buffer->len) && (end_ptr >= start_ptr))
      {
        lwhttp_token_set(field3, start_ptr, (uint16_t)(end_ptr - start_ptr));
      }
      else
      {
        lwhttp_token_reset(field3);
      }
    }
  }        
}

uint16_t lwhttp_response_get(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  return lwhttp_message_get((lwhttp_message_t*) response, dest, len);
}

uint16_t lwhttp_request_get(lwhttp_request_t* request, char** dest, uint16_t* len)
{
  return lwhttp_message_get((lwhttp_message_t*) request, dest, len);
}

static uint16_t lwhttp_message_get(lwhttp_message_t* message, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((message != NULL) && (dest != NULL) && (len != NULL))
  {
    ret = lwhttp_token_get(&message->buffer, dest, len);
  }
  return ret;
}

uint16_t lwhttp_response_put(lwhttp_response_t* response, const char* src, uint16_t len)
{
  return lwhttp_message_put((lwhttp_message_t*) response, src, len);
}

uint16_t lwhttp_request_put(lwhttp_request_t* request, const char* src, uint16_t len)
{
  return lwhttp_message_put((lwhttp_message_t*) request, src, len);
}

static uint16_t lwhttp_message_put(lwhttp_message_t* message, const char* src, uint16_t len)
{
  uint16_t ret = 0;
  uint16_t idx = 0;
  if ((message != NULL) && (src != NULL) && (len > 0))
  {
    /* allocate enough space for message */
    message->buffer.data = realloc(message->buffer.data, sizeof(char) * (message->buffer.len + len));

    /* fill the buffer */
    for (idx = 0; idx < len; idx++)
    {
      message->buffer.data[message->buffer.len++] = *src++;
    }
    
    message->builder_state = LwHHTP_BUILDER_READY;
    message->parser_state = LwHHTP_PARSER_READY;
  }
  return ret;
}

uint16_t lwhttp_response_put_eol(lwhttp_response_t* response)
{
  return lwhttp_message_put((lwhttp_message_t*) response, LwHTTP_eol_string, strlen(LwHTTP_eol_string));
}

uint16_t lwhttp_request_put_eol(lwhttp_request_t* request)
{
  return lwhttp_message_put((lwhttp_message_t*) request, LwHTTP_eol_string, strlen(LwHTTP_eol_string));
}

uint16_t lwhttp_response_get_status_line(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  return lwhttp_message_get_start_line((lwhttp_message_t*) response, dest, len);
}

uint16_t lwhttp_request_get_request_line(lwhttp_request_t* request, char** dest, uint16_t* len)
{
  return lwhttp_message_get_start_line((lwhttp_message_t*) request, dest, len);
}

static uint16_t lwhttp_message_get_start_line(lwhttp_message_t* message, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if (message != NULL)
  {
    if(LwHHTP_MESSAGE_TYPE_RESPONSE == message->message_type)
    {
      ret = lwhttp_token_get(&message->start_line.status_line.buffer, dest, len);
    }
    else /* LwHHTP_MESSAGE_TYPE_REQUEST */
    {
      ret = lwhttp_token_get(&message->start_line.request_line.buffer, dest, len);
    }
  }
  return ret;
}

uint16_t lwhttp_response_get_status_code(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if (response != NULL)
  {
    ret = lwhttp_token_get(&response->start_line.status_line.status_code, dest, len);
  }
  return ret;
}

uint16_t lwhttp_response_put_status_line(lwhttp_response_t* response, const char* status_code, const char* reason_phrase)
{
  return lwhttp_message_put_start_line((lwhttp_message_t*) response, LwHTTP_version_v1_0_string, status_code, reason_phrase);
}

uint16_t lwhttp_request_put_request_line(lwhttp_request_t* request, lwhttp_method_t method, const char* request_uri)
{
  return lwhttp_message_put_start_line((lwhttp_message_t*) request, LwHTTP_method_string[method], request_uri, LwHTTP_version_v1_0_string);
}

static uint16_t lwhttp_message_put_start_line(lwhttp_message_t* message, const char* field1, const char* field2, const char* field3)
{
  char temp_buff[128];
  uint16_t ret = 0;
  uint16_t len = 0;
  
  if ((message != NULL) && (field1 != NULL) && (field2 != NULL) && (field3 != NULL))
  {
    if (message->builder_state == LwHHTP_BUILDER_INIT)
    {
      /* Format Start-Line + EOL */
      snprintf(&temp_buff[0], sizeof(temp_buff), LwHTTP_start_line_fmt, field1, field2, field3, LwHTTP_eol_string);
      len = strlen(&temp_buff[0]);
      
      ret = lwhttp_message_put(message, &temp_buff[0], len);
      
      message->builder_state = LwHHTP_BUILDER_ADDED_START_LINE;
    }
  }
  
  return ret;
}

uint16_t lwhttp_response_get_message_header(lwhttp_response_t* response, const char* field_name, lwhttp_message_header_t** dest)
{
  return lwhttp_message_get_message_header((lwhttp_message_t*) response, field_name, dest);
}

uint16_t lwhttp_request_get_message_header(lwhttp_request_t* request, const char* field_name, lwhttp_message_header_t** dest)
{
  return lwhttp_message_get_message_header((lwhttp_message_t*) request, field_name, dest);
}

static uint16_t lwhttp_message_get_message_header(lwhttp_message_t* message, const char* field_name, lwhttp_message_header_t** dest)
{
  uint8_t idx = 0;
  uint16_t ret = 0;

  if ((message != NULL) && (field_name != NULL)&& (dest != NULL))
  {
    *dest = NULL;
    
    for (idx = 0; (idx < message->message_headers.count) && (*dest == NULL) && (message->message_headers.count > 0); idx++)
    {
      if (0 == strncmp(message->message_headers.headers[idx].field_name.data, field_name, strlen(field_name)))
      {
        *dest = &message->message_headers.headers[idx];
        ret = 1;
        break;
      }
    }
  }
  
  return ret;
}

uint16_t lwhttp_response_put_message_header(lwhttp_response_t* response, const char* header, const char* value)
{
  return lwhttp_message_put_message_header((lwhttp_message_t*) response, header, value);
}

uint16_t lwhttp_request_put_message_header(lwhttp_request_t* request, const char* header, const char* value)
{
  return lwhttp_message_put_message_header((lwhttp_message_t*) request, header, value);
}

static uint16_t lwhttp_message_put_message_header(lwhttp_message_t* message, const char* header, const char* value)
{
  char temp_buff[128];
  uint16_t ret = 0;
  uint16_t len = 0;
  uint16_t idx = 0;
  
  if ((message != NULL) && (header != NULL) && (value != NULL))
  {
    if ((message->builder_state >= LwHHTP_BUILDER_ADDED_START_LINE) && (message->builder_state <= LwHHTP_BUILDER_ADDED_MESSAGE_HEADER))
    {      
      /* Format message header + EOL */
      snprintf(&temp_buff[0], sizeof(temp_buff), LwHTTP_message_header_fmt, header, value, LwHTTP_eol_string);
      len = strlen(&temp_buff[0]);
      
      ret = lwhttp_message_put(message, &temp_buff[0], len);

      message->builder_state = LwHHTP_BUILDER_ADDED_MESSAGE_HEADER;
      
      ret = 1;
    } 
  }
  
  return ret;
}

uint16_t lwhttp_response_get_message_body(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  return lwhttp_message_get_message_body((lwhttp_message_t*) response, dest, len);
}

uint16_t lwhttp_request_get_message_body(lwhttp_request_t* request, char** dest, uint16_t* len)
{
  return lwhttp_message_get_message_body((lwhttp_message_t*) request, dest, len);
}

static uint16_t lwhttp_message_get_message_body(lwhttp_message_t* message, char** dest, uint16_t* len)
{
  uint16_t ret = 0;
  if ((message != NULL) && (dest != NULL) && (len != NULL))
  {
    lwhttp_token_get(&message->message_body, dest, len);
    ret = 1;
  }
  return ret;
}

uint16_t lwhttp_response_put_message_body(lwhttp_response_t* response, const char* body, uint16_t len)
{
  return lwhttp_message_put_message_body((lwhttp_message_t*) response, body, len);
}

uint16_t lwhttp_request_put_message_body(lwhttp_request_t* request, const char* body, uint16_t len)
{
  return lwhttp_message_put_message_body((lwhttp_message_t*) request, body, len);
}

static uint16_t lwhttp_message_put_message_body(lwhttp_message_t* message, const char* body, uint16_t len)
{
  uint16_t ret = 0;
  uint16_t idx = 0;
  
  if ((message != NULL) && (body != NULL) && (len > 0))
  {
    if ((message->builder_state >= LwHHTP_BUILDER_ADDED_START_LINE) && (message->builder_state <= LwHHTP_BUILDER_ADDED_MESSAGE_HEADER))
    {
      /* Add Empty Line */
      ret = lwhttp_message_put(message, LwHTTP_eol_string, strlen(LwHTTP_eol_string));
      
      /* Add Body */
      ret = lwhttp_message_put(message, body, len);
      message->builder_state = LwHHTP_BUILDER_FINISHED;
      
      ret = 1;
    } 
  }
  
  return ret;
}
