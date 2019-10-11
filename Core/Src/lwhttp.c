#include "lwhttp.h"
#include <stdlib.h>
#include <stdio.h>

#if (DBG_LWHTTP_ENABLED == 1 )
  extern void cli_dbg(const char* label, const char* fn, const char* fmt, ...);
  #define DBG_LWHTTP(...) cli_dbg((char*)"LwHTTP",  __FUNCTION__, __VA_ARGS__)
#else
  #define DBG_LWHTTP(...) (void)0
#endif

static const char* LwHTTP_eol_string = "\r\n";
static const char* LwHTTP_version_v1_0_string = "HTTP/1.0";
static const char* LwHTTP_method_string[LwHHTP_MAX] = {"POST", "GET"};
static const char* LwHTTP_start_line_fmt = "%s %s %s%s";
static const char* LwHTTP_message_header_fmt = "%s: %s%s";

/* LwHTTP General functions */
static BaseType_t lwhttp_message_init(lwhttp_message_t* message, lwhttp_message_type_t type);
static BaseType_t lwhttp_message_free(lwhttp_message_t* message);
static BaseType_t lwhttp_message_parse(lwhttp_message_t* message);
static BaseType_t lwhttp_message_get(lwhttp_message_t* message, char** dest, uint16_t* len);
static BaseType_t lwhttp_message_put(lwhttp_message_t* message, const char* src, uint16_t len);
static BaseType_t lwhttp_message_put_ext(lwhttp_message_t* message, const char* src, uint16_t len);
static BaseType_t lwhttp_message_put_int(lwhttp_message_t* message, const char* src, uint16_t len);
static BaseType_t lwhttp_message_get_start_line(lwhttp_message_t* message, char** dest, uint16_t* len);
static BaseType_t lwhttp_message_put_start_line(lwhttp_message_t* message, const char* field1, const char* field2, const char* field3);
static BaseType_t lwhttp_message_get_message_header(lwhttp_message_t* message, const char* field_name, lwhttp_message_header_t** dest);
static BaseType_t lwhttp_message_put_message_header(lwhttp_message_t* message, const char* header, const char* value);
static BaseType_t lwhttp_message_get_message_body(lwhttp_message_t* message, char** dest, uint16_t* len);
static BaseType_t lwhttp_message_put_message_body(lwhttp_message_t* message, const char* body, uint16_t len);
static BaseType_t lwhttp_message_put_eol(lwhttp_request_t* message);
static BaseType_t lwhttp_message_put_nul(lwhttp_request_t* message);

/* LwHTTP Auxiliar functions */
static BaseType_t lwhttp_token_set(lwhttp_token_t* token, const char* src, uint16_t len);
static BaseType_t lwhttp_token_get(lwhttp_token_t* token, char** dest, uint16_t* len);
static BaseType_t lwhttp_token_reset(lwhttp_token_t* token);
static BaseType_t lwhttp_parse_message_header(lwhttp_message_header_t* message_header);
static BaseType_t lwhttp_parse_start_line(lwhttp_start_line_t* start_line, lwhttp_message_type_t type);
static void lwhttp_builder_set_state(lwhttp_message_t* message, lwhttp_message_builder_state_t state);
static void lwhttp_parser_set_state(lwhttp_message_t* message, lwhttp_message_parser_state_t state);

static void lwhttp_builder_set_state(lwhttp_message_t* message, lwhttp_message_builder_state_t state)
{
#if (DBG_LWHTTP_TRANSITIONS_ENABLED == 1 )
  DBG_LWHTTP("Transition from (%d) to (%d)\r\n", message->builder_state, state);
#endif
  message->builder_state = state;
}

static void lwhttp_parser_set_state(lwhttp_message_t* message, lwhttp_message_parser_state_t state)
{
#if (DBG_LWHTTP_TRANSITIONS_ENABLED == 1 )
  DBG_LWHTTP("Transition from (%d) to (%d)\r\n", message->parser_state, state);
#endif
  message->parser_state = state;
}

static BaseType_t lwhttp_token_set(lwhttp_token_t* token, const char* src, uint16_t len)
{
  BaseType_t xReturn = errNOK;

  if (token != NULL)
  {
    token->data = (char*)src;
    token->len = len;
    xReturn = errOK;
  }
  else
  {
    xReturn = errARG;
  }

  return xReturn;
}

static BaseType_t lwhttp_token_get(lwhttp_token_t* token, char** dest, uint16_t* len)
{
  BaseType_t xReturn = errNOK;

  if ((token != NULL) && (dest != NULL) && (len != NULL))
  {
    *dest = token->data;
    *len = token->len;
    xReturn = errOK;
  }
  else
  {
    xReturn = errARG;
  }

  return xReturn;
}

static BaseType_t lwhttp_token_reset(lwhttp_token_t* token)
{
  return lwhttp_token_set(token, NULL, 0);
}

BaseType_t lwhttp_response_init(lwhttp_response_t* response)
{
  return lwhttp_message_init((lwhttp_message_t*)response, LwHHTP_MESSAGE_TYPE_RESPONSE);
}

BaseType_t lwhttp_request_init(lwhttp_request_t* request)
{
  return lwhttp_message_init((lwhttp_message_t*)request, LwHHTP_MESSAGE_TYPE_REQUEST);
}

static BaseType_t lwhttp_message_init(lwhttp_message_t* message, lwhttp_message_type_t type)
{
  BaseType_t xReturn = errNOK;
  if ((message != NULL) && ((type >= LwHHTP_MESSAGE_TYPE_REQUEST) && (type <= LwHHTP_MESSAGE_TYPE_RESPONSE)))
  {

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

    /* Parser/Builder internals */
    lwhttp_builder_set_state(message, LwHHTP_BUILDER_INIT);
    lwhttp_parser_set_state(message, LwHHTP_PARSER_INIT);
    
    lwhttp_message_free(message);

    xReturn = errOK;
  }

  return xReturn;
}

BaseType_t lwhttp_response_free(lwhttp_response_t* response)
{
  return lwhttp_message_free((lwhttp_message_t*) response);
}

BaseType_t lwhttp_request_free(lwhttp_request_t* request)
{
  return lwhttp_message_free((lwhttp_message_t*) request);
}

static BaseType_t lwhttp_message_free(lwhttp_message_t* message)
{
  BaseType_t xReturn = errNOK;
  if (message != NULL)
  {
    /* HTTP/1.1 Sec4.2 HTTP Message - Message headers */
    free(message->message_headers.headers);
    message->message_headers.headers = NULL;
    message->message_headers.count = 0;

    /* HTTP/1.1 Sec4.3 HTTP Message - Message body */
    free(message->buffer.data);
    xReturn = lwhttp_token_reset(&message->message_body);  
  }
  return xReturn;
}

BaseType_t lwhttp_response_parse(lwhttp_response_t* response)
{
  return lwhttp_message_parse((lwhttp_message_t*) response);
}

BaseType_t lwhttp_request_parse(lwhttp_request_t* request)
{
  return lwhttp_message_parse((lwhttp_message_t*) request);
}

static BaseType_t lwhttp_message_parse(lwhttp_message_t* message)
{
  BaseType_t xReturn = errOK;
  char* start = NULL;
  char* end = NULL;
    
  if (message != NULL)
  {    
    if (LwHHTP_PARSER_READY == message->parser_state)
    {
      lwhttp_parser_set_state(message, LwHHTP_PARSER_START_LINE);
    }
    else
    {
      lwhttp_parser_set_state(message, LwHHTP_PARSER_ERROR);
      xReturn = errParseReady;
    }
    
    for (start = message->buffer.data; message->parser_state < LwHHTP_PARSER_END; start = (char*)(end + strlen(LwHTTP_eol_string)))
    {
      /* Get EOL */
      end = strstr(start, LwHTTP_eol_string);
      if ((end == NULL) && (message->parser_state != LwHHTP_PARSER_MESSAGE_BODY))
      {
        lwhttp_parser_set_state(message, LwHHTP_PARSER_ERROR);
        xReturn = errParser;
      }
      
      switch(message->parser_state){
      case (LwHHTP_PARSER_START_LINE):
        {
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
          xReturn = lwhttp_parse_start_line(&message->start_line, message->message_type);
          if(xReturn != errOK)
          {
            xReturn = errParseStart;
            lwhttp_parser_set_state(message, LwHHTP_PARSER_ERROR);
          }
          else
          {
            /* Parse next header */
            lwhttp_parser_set_state(message, LwHHTP_PARSER_MESSAGE_HEADER);
          }
        }
        break;
        
      case (LwHHTP_PARSER_MESSAGE_HEADER):
        {
          if ((end - start) <= 0)
          {
            /* Empty line found, next line is body */
            lwhttp_parser_set_state(message, LwHHTP_PARSER_MESSAGE_BODY);
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
            xReturn = lwhttp_parse_message_header(&message->message_headers.headers[message->message_headers.count-1]);
            if(xReturn != errOK)
            {
              xReturn = errParseHeader;
              lwhttp_parser_set_state(message, LwHHTP_PARSER_ERROR);
            }
            else
            {
              /* Parse next header */
              lwhttp_parser_set_state(message, LwHHTP_PARSER_MESSAGE_HEADER);
            }
          }
        }
        break;
        
      case (LwHHTP_PARSER_MESSAGE_BODY):
        message->message_body.data = start;
        message->message_body.len = (uint16_t)(&message->buffer.data[message->buffer.len] - start);
        if (message->message_body.data != NULL)
        {
          lwhttp_parser_set_state(message, LwHHTP_PARSER_END);
        }
        else
        {
          lwhttp_parser_set_state(message, LwHHTP_PARSER_ERROR);
          xReturn = errParseBody;
        }
        break;
        
      default:
        break;
        /* Do nothing */
      }

#if (DBG_LWHTTP_ENABLED == 1)
      if (errOK != xReturn)
      {
        DBG_LWHTTP("xRet = %d, pState = %d, bState = %d\r\n", xReturn, message->parser_state, message->builder_state);
      }
#endif
    }
    
    if (message->parser_state == LwHHTP_PARSER_END)
    {
      xReturn = errOK;
    }
  }
  else
  {
    xReturn = errARG;
  }

#if (DBG_LWHTTP_ENABLED == 1)
  if (xReturn != errOK)
  {
    DBG_LWHTTP("xRet = %d, pState = %d, bState = %d\r\n", xReturn, message->parser_state, message->builder_state);
  }
#endif 
  
  return xReturn;
}

static BaseType_t lwhttp_parse_message_header(lwhttp_message_header_t* message_header)
{
  BaseType_t xReturn = errNOK;

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
        xReturn = errBUF;
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
        xReturn = errBUF;
      }

      if ((message_header->field_name.data != NULL) && (message_header->field_value.data != NULL))
      {
        xReturn = errOK;
      }
    }
    else
    {
      xReturn = errARG;
    }
  }
  else
  {
    xReturn = errARG;
  }

  return xReturn;
}

static BaseType_t lwhttp_parse_start_line(lwhttp_start_line_t* start_line, lwhttp_message_type_t type)
{
  BaseType_t xReturn = errNOK;

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

      if ((field1->data != NULL) && (field2->data != NULL) && (field3->data != NULL))
      {
        xReturn = errOK;
      }
    }
    else
    {
      xReturn = errBUF;
    }
  }   
  else
  {
    xReturn = errARG;
  }

  return xReturn;     
}

BaseType_t lwhttp_response_get(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  return lwhttp_message_get((lwhttp_message_t*) response, dest, len);
}

BaseType_t lwhttp_request_get(lwhttp_request_t* request, char** dest, uint16_t* len)
{
  return lwhttp_message_get((lwhttp_message_t*) request, dest, len);
}

static BaseType_t lwhttp_message_get(lwhttp_message_t* message, char** dest, uint16_t* len)
{
  BaseType_t xReturn = errNOK;
  
  xReturn = lwhttp_token_get(&message->buffer, dest, len);
  if (xReturn == errOK)
  {
    if ((*dest == NULL) || (*len == 0))
    {
      xReturn = errNotFound;
    }
  }

  return xReturn;
}

BaseType_t lwhttp_response_put(lwhttp_response_t* response, const char* src, uint16_t len)
{
  return lwhttp_message_put_ext((lwhttp_message_t*) response, src, len);
}

BaseType_t lwhttp_request_put(lwhttp_request_t* request, const char* src, uint16_t len)
{
  return lwhttp_message_put_ext((lwhttp_message_t*) request, src, len);
}

static BaseType_t lwhttp_message_put_ext(lwhttp_message_t* message, const char* src, uint16_t len)
{
  BaseType_t xReturn = errNOK;

  if (message != NULL)
  {
    if ((message->builder_state == LwHHTP_BUILDER_INIT) || (message->builder_state == LwHHTP_BUILDER_IN_PROGRESS))
    {
      if ((message->builder_state == LwHHTP_BUILDER_IN_PROGRESS) && (src == NULL) && (len == 0))
      {
        lwhttp_message_put_nul(message);
        lwhttp_parser_set_state(message, LwHHTP_PARSER_READY);
        lwhttp_builder_set_state(message, LwHHTP_BUILDER_FINISHED);
        xReturn = errOK;
      }
      else if ((src != NULL) && (len > 0))
      {
        lwhttp_builder_set_state(message, LwHHTP_BUILDER_IN_PROGRESS);
        xReturn = lwhttp_message_put(message, src, len);
      }
      else
      {
        xReturn = errARG;
      }
    }
    else
    {
      xReturn = errBuild;
    }
  }
  else
  {
    xReturn = errARG;
  }

#if (DBG_LWHTTP_ENABLED == 1)
  if (xReturn != errOK)
  {
    DBG_LWHTTP("src = %ld, len = %d\r\n", (uint32_t)src, len);
    DBG_LWHTTP("xRet = %d, pState = %d, bState = %d\r\n", xReturn, message->parser_state, message->builder_state);
  }
#endif 

  return xReturn;
}

static BaseType_t lwhttp_message_put_int(lwhttp_message_t* message, const char* src, uint16_t len)
{
  BaseType_t xReturn = errNOK;

  if ((message != NULL) && (src != NULL) && (len > 0))
  {
    if ((message->builder_state == LwHHTP_BUILDER_INIT) 
      || ((message->builder_state >= LwHHTP_BUILDER_ADDED_START_LINE) && (message->builder_state < LwHHTP_BUILDER_FINISHED)))
    {
      xReturn = lwhttp_message_put(message, src, len);
    }
    else
    {
      xReturn = errBuild;
    }
  }
  else
  {
    xReturn = errARG;
  }

#if (DBG_LWHTTP_ENABLED == 1)
  if (xReturn != errOK)
  {
    DBG_LWHTTP("xRet = %d, pState = %d, bState = %d\r\n", xReturn, message->parser_state, message->builder_state);
  }
#endif 

  return xReturn;
}

static BaseType_t lwhttp_message_put(lwhttp_message_t* message, const char* src, uint16_t len)
{
  BaseType_t xReturn = errNOK;
  uint16_t usIndex;

  if ((message != NULL) && (src != NULL) && (len > 0))
  {
    /* allocate enough space for message */
    message->buffer.data = realloc(message->buffer.data, sizeof(char) * (message->buffer.len + len));

    if( message->buffer.data != NULL )
    {
      /* fill the buffer */
      for (usIndex = 0; usIndex < len; usIndex++)
      {
        message->buffer.data[message->buffer.len++] = *src++;
      }

      xReturn = errOK;
    }
    else
    {
      /* Failed to realloc buffer */
      xReturn = errMEM;
    }
  }
  else
  {
    xReturn = errARG;
  }

  return xReturn;
}

BaseType_t lwhttp_response_get_status_line(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  return lwhttp_message_get_start_line((lwhttp_message_t*) response, dest, len);
}

BaseType_t lwhttp_request_get_request_line(lwhttp_request_t* request, char** dest, uint16_t* len)
{
  return lwhttp_message_get_start_line((lwhttp_message_t*) request, dest, len);
}

static BaseType_t lwhttp_message_get_start_line(lwhttp_message_t* message, char** dest, uint16_t* len)
{
  BaseType_t xReturn = errNOK;
  if (message != NULL)
  {
    if(LwHHTP_MESSAGE_TYPE_RESPONSE == message->message_type)
    {
      xReturn = lwhttp_token_get(&message->start_line.status_line.buffer, dest, len);
    }
    else /* LwHHTP_MESSAGE_TYPE_REQUEST */
    {
      xReturn = lwhttp_token_get(&message->start_line.request_line.buffer, dest, len);
    }

    if (xReturn != errOK)
    {
      xReturn = errNotFound;
    }
  }
  else
  {
    xReturn = errARG;
  }
  return xReturn;
}

BaseType_t lwhttp_response_get_status_code(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  BaseType_t xReturn = errNOK;
  if (response != NULL)
  {
    xReturn = lwhttp_token_get(&response->start_line.status_line.status_code, dest, len);
  }
  else
  {
    xReturn = errARG;
  }
  return xReturn;
}

BaseType_t lwhttp_response_put_status_line(lwhttp_response_t* response, const char* status_code, const char* reason_phrase)
{
  return lwhttp_message_put_start_line((lwhttp_message_t*) response, LwHTTP_version_v1_0_string, status_code, reason_phrase);
}

BaseType_t lwhttp_request_put_request_line(lwhttp_request_t* request, lwhttp_method_t method, const char* request_uri)
{
  return lwhttp_message_put_start_line((lwhttp_message_t*) request, LwHTTP_method_string[method], request_uri, LwHTTP_version_v1_0_string);
}

static BaseType_t lwhttp_message_put_start_line(lwhttp_message_t* message, const char* field1, const char* field2, const char* field3)
{
  BaseType_t xReturn = errNOK;

  char temp_buff[128];
  uint16_t usLength = 0;
  
  if ((message != NULL) && (field1 != NULL) && (field2 != NULL) && (field3 != NULL))
  {
    if (message->builder_state == LwHHTP_BUILDER_INIT)
    {
      /* Format Start-Line + EOL */
      snprintf(&temp_buff[0], sizeof(temp_buff), LwHTTP_start_line_fmt, field1, field2, field3, LwHTTP_eol_string);
      usLength = strlen(&temp_buff[0]);

      xReturn = lwhttp_message_put_int(message, &temp_buff[0], usLength);
      if (xReturn == errOK)
      {
        lwhttp_builder_set_state(message, LwHHTP_BUILDER_ADDED_START_LINE);
      }
      else
      {
        xReturn = errBuildStart;
      }
    }
    else
    {
      xReturn = errBuildReady;
    }
  }
  else
  {
    xReturn = errARG;
  }
  
  return xReturn;
}

BaseType_t lwhttp_response_get_message_header(lwhttp_response_t* response, const char* field_name, lwhttp_message_header_t** dest)
{
  return lwhttp_message_get_message_header((lwhttp_message_t*) response, field_name, dest);
}

BaseType_t lwhttp_request_get_message_header(lwhttp_request_t* request, const char* field_name, lwhttp_message_header_t** dest)
{
  return lwhttp_message_get_message_header((lwhttp_message_t*) request, field_name, dest);
}

static BaseType_t lwhttp_message_get_message_header(lwhttp_message_t* message, const char* field_name, lwhttp_message_header_t** dest)
{
  BaseType_t xReturn = errNOK;
  uint16_t usIndex = 0;

  if ((message != NULL) && (field_name != NULL)&& (dest != NULL))
  {
    *dest = NULL;
    
    for (usIndex = 0; (usIndex < message->message_headers.count) && (*dest == NULL) && (message->message_headers.count > 0); usIndex++)
    {
      if (0 == strncmp(message->message_headers.headers[usIndex].field_name.data, field_name, strlen(field_name)))
      {
        *dest = &message->message_headers.headers[usIndex];
        xReturn = errOK;
        break;
      }
    }

    if(*dest == NULL)
    {
      xReturn = errNotFound;
    }
  }
  else
  {
    xReturn = errARG;
  }
  
  return xReturn;
}

BaseType_t lwhttp_response_put_message_header(lwhttp_response_t* response, const char* header, const char* value)
{
  return lwhttp_message_put_message_header((lwhttp_message_t*) response, header, value);
}

BaseType_t lwhttp_request_put_message_header(lwhttp_request_t* request, const char* header, const char* value)
{
  return lwhttp_message_put_message_header((lwhttp_message_t*) request, header, value);
}

static BaseType_t lwhttp_message_put_message_header(lwhttp_message_t* message, const char* header, const char* value)
{
  BaseType_t xReturn = errNOK;
  char cTempBuff[128];
  uint16_t usLength = 0;

  if ((message != NULL) && (header != NULL) && (value != NULL))
  {
    if ((message->builder_state >= LwHHTP_BUILDER_ADDED_START_LINE) && (message->builder_state <= LwHHTP_BUILDER_ADDED_MESSAGE_HEADER))
    {      
      /* Format message header + EOL */
      snprintf(&cTempBuff[0], sizeof(cTempBuff), LwHTTP_message_header_fmt, header, value, LwHTTP_eol_string);
      usLength = strlen(&cTempBuff[0]);
      
      xReturn = lwhttp_message_put_int(message, &cTempBuff[0], usLength);

      if (xReturn == errOK)
      {
        lwhttp_builder_set_state(message, LwHHTP_BUILDER_ADDED_MESSAGE_HEADER);
      }
    }
    else
    {
      xReturn = errBuildHeader;
    }
  }
  else
  {
    xReturn = errARG;
  }
  
  return xReturn;
}

BaseType_t lwhttp_response_get_message_body(lwhttp_response_t* response, char** dest, uint16_t* len)
{
  return lwhttp_message_get_message_body((lwhttp_message_t*) response, dest, len);
}

BaseType_t lwhttp_request_get_message_body(lwhttp_request_t* request, char** dest, uint16_t* len)
{
  return lwhttp_message_get_message_body((lwhttp_message_t*) request, dest, len);
}

static BaseType_t lwhttp_message_get_message_body(lwhttp_message_t* message, char** dest, uint16_t* len)
{
  BaseType_t xReturn = errNOK;
  if ((message != NULL) && (dest != NULL) && (len != NULL))
  {
    xReturn = lwhttp_token_get(&message->message_body, dest, len);
  }
  else
  {
    xReturn = errARG;
  }
  return xReturn;
}

BaseType_t lwhttp_response_put_message_body(lwhttp_response_t* response, const char* body, uint16_t len)
{
  return lwhttp_message_put_message_body((lwhttp_message_t*) response, body, len);
}

BaseType_t lwhttp_request_put_message_body(lwhttp_request_t* request, const char* body, uint16_t len)
{
  return lwhttp_message_put_message_body((lwhttp_message_t*) request, body, len);
}

static BaseType_t lwhttp_message_put_message_body(lwhttp_message_t* message, const char* body, uint16_t len)
{
  BaseType_t xReturn = errNOK;
  
  if ((message != NULL) && (body != NULL) && (len > 0))
  {
    if ((message->builder_state >= LwHHTP_BUILDER_ADDED_START_LINE) && (message->builder_state <= LwHHTP_BUILDER_ADDED_MESSAGE_HEADER))
    {
      /* Add Empty Line */
      lwhttp_message_put_eol(message);
      
      /* Add Body */

      xReturn = lwhttp_message_put_int(message, body, len);

      if (xReturn == errOK)
      {
        /* NULL terminate the buff */
        lwhttp_message_put_nul(message);
        lwhttp_builder_set_state(message, LwHHTP_BUILDER_FINISHED);
        lwhttp_parser_set_state(message, LwHHTP_PARSER_READY);
      }
    }
    else
    {
      xReturn = errBuildBody;
    }
  }
  else
  {
    xReturn = errARG;
  }
  
  return xReturn;
}

BaseType_t lwhttp_response_put_eol(lwhttp_request_t* response)
{
  return lwhttp_message_put_eol((lwhttp_request_t*) response);
}

BaseType_t lwhttp_request_put_eol(lwhttp_request_t* request)
{
  return lwhttp_message_put_eol((lwhttp_request_t*) request);
}

static BaseType_t lwhttp_message_put_eol(lwhttp_request_t* message)
{
  return lwhttp_message_put_int(message, LwHTTP_eol_string, strlen(LwHTTP_eol_string));
}

static BaseType_t lwhttp_message_put_nul(lwhttp_request_t* message)
{
  BaseType_t xReturn = errNOK;
  xReturn = lwhttp_message_put(message, "\0", 1);
  if (xReturn == errOK)
  {
    /* Decrease Buffer Length */
   // message->buffer.len--;
  }
  return xReturn;
}